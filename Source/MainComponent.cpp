#include "MainComponent.h"

namespace
{
    const juce::Colour background   { 0xff16181c };
    const juce::Colour panel        { 0xff23262b };
    const juce::Colour accent       { 0xffffb000 };
    const juce::Colour meterGreen   { 0xff3ddc84 };
    const juce::Colour meterRed     { 0xffff4d4d };

    juce::String toMs (double samples, double sampleRate)
    {
        return sampleRate > 0.0 ? juce::String (1000.0 * samples / sampleRate, 1) + " ms" : "?";
    }
}

MainComponent::MainComponent()
{
    // 1 input (guitar) and 2 outputs (stereo) by default; the user can change it.
    const auto error = deviceManager.initialise (2, 2, nullptr, true);
    if (error.isNotEmpty())
        juce::Logger::writeToLog ("Audio init error: " + error);

    deviceManager.addAudioCallback (&engine);
    deviceManager.addChangeListener (this);
    deviceManager.addMidiInputDeviceCallback ({}, &midiMonitor); // all enabled MIDI inputs
    enableAllMidiInputs();

    deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent> (
        deviceManager,
        1, 6,      // inputs  (Scarlett 6i6 has up to 6)
        1, 6,      // outputs
        true,      // show MIDI inputs
        false,     // MIDI output
        false,     // stereo pairs
        false);    // advanced options hidden
    selectorViewport.setViewedComponent (deviceSelector.get(), false);
    selectorViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (selectorViewport);

    addAndMakeVisible (inputLabel);
    addAndMakeVisible (inputChoice);
    inputChoice.onChange = [this] { engine.setInputChannel (inputChoice.getSelectedItemIndex()); };

    addAndMakeVisible (gainLabel);
    gainSlider.setRange (0.0, 2.0, 0.01);
    gainSlider.setValue (1.0, juce::dontSendNotification);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    gainSlider.onValueChange = [this] { engine.setGain ((float) gainSlider.getValue()); };
    addAndMakeVisible (gainSlider);

    latencyLabel.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    latencyLabel.setColour (juce::Label::textColourId, accent);
    addAndMakeVisible (latencyLabel);
    addAndMakeVisible (deviceLabel);

    addAndMakeVisible (midiMonitor);

    refreshDeviceInfo();
    startTimerHz (30);
    setSize (1000, 680);
}

MainComponent::~MainComponent()
{
    deviceManager.removeMidiInputDeviceCallback ({}, &midiMonitor);
    deviceManager.removeAudioCallback (&engine);
    deviceManager.removeChangeListener (this);
}

void MainComponent::enableAllMidiInputs()
{
    for (const auto& input : juce::MidiInput::getAvailableDevices())
        deviceManager.setMidiInputDeviceEnabled (input.identifier, true);
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refreshDeviceInfo();
}

void MainComponent::refreshInputChoices()
{
    const int previous = inputChoice.getSelectedItemIndex();
    inputChoice.clear (juce::dontSendNotification);

    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        const auto names  = device->getInputChannelNames();
        const auto active = device->getActiveInputChannels();

        int item = 1;
        for (int i = 0; i < names.size(); ++i)
            if (active[i])
                inputChoice.addItem (names[i], item++);
    }

    if (inputChoice.getNumItems() > 0)
        inputChoice.setSelectedItemIndex (juce::jlimit (0, inputChoice.getNumItems() - 1, juce::jmax (0, previous)),
                                          juce::sendNotificationSync);
}

void MainComponent::refreshDeviceInfo()
{
    refreshInputChoices();

    auto* device = deviceManager.getCurrentAudioDevice();
    if (device == nullptr)
    {
        latencyLabel.setText ("Aucune carte son active", juce::dontSendNotification);
        deviceLabel.setText ({}, juce::dontSendNotification);
        return;
    }

    const double sr     = device->getCurrentSampleRate();
    const int    buffer = device->getCurrentBufferSizeSamples();
    const int    in     = device->getInputLatencyInSamples();
    const int    out    = device->getOutputLatencyInSamples();
    const int    total  = in + out + buffer; // the callback adds one buffer

    latencyLabel.setText ("Delai aller-retour estime : " + toMs (total, sr),
                          juce::dontSendNotification);

    deviceLabel.setText (device->getTypeName() + " / " + device->getName()
                           + "   |   " + juce::String (sr, 0) + " Hz, buffer " + juce::String (buffer)
                           + " (" + toMs (buffer, sr) + ")   |   entree " + toMs (in, sr)
                           + ", sortie " + toMs (out, sr),
                         juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    const float peak = engine.popInputPeak();
    displayedLevel = juce::jmax (peak, displayedLevel * 0.85f); // fast attack, smooth release

    if (engine.popClipped())
        clipHoldFrames = 45; // ~1.5 s
    else if (clipHoldFrames > 0)
        --clipHoldFrames;

    repaint (meterArea);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (background);

    g.setColour (panel);
    g.fillRoundedRectangle (selectorViewport.getBounds().expanded (6).toFloat(), 8.0f);

    // Input level meter (dB scale from -60 to 0)
    auto meter = meterArea.toFloat();
    g.setColour (juce::Colours::black);
    g.fillRoundedRectangle (meter, 4.0f);

    const float db = juce::Decibels::gainToDecibels (displayedLevel, -60.0f);
    const float proportion = juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f);
    g.setColour (clipHoldFrames > 0 ? meterRed : meterGreen);
    g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * proportion), 4.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (clipHoldFrames > 0 ? "SATURE ! baisse le gain sur la Scarlett"
                                   : "Niveau d'entree " + juce::String (db, 1) + " dB",
                meterArea.reduced (8, 0), juce::Justification::centredLeft);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (16);

    auto left = area.removeFromLeft (area.getWidth() / 2).reduced (6);
    area.removeFromLeft (10);

    selectorViewport.setBounds (left);
    deviceSelector->setSize (left.getWidth() - selectorViewport.getScrollBarThickness(),
                             juce::jmax (left.getHeight(), 520));

    auto right = area;
    latencyLabel.setBounds (right.removeFromTop (34));
    deviceLabel.setBounds (right.removeFromTop (40));
    right.removeFromTop (6);

    auto row = right.removeFromTop (28);
    inputLabel.setBounds (row.removeFromLeft (120));
    inputChoice.setBounds (row);
    right.removeFromTop (6);

    row = right.removeFromTop (28);
    gainLabel.setBounds (row.removeFromLeft (120));
    gainSlider.setBounds (row);
    right.removeFromTop (8);

    meterArea = right.removeFromTop (26);
    right.removeFromTop (12);

    midiMonitor.setBounds (right);
}
