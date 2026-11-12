#include "MidiMonitor.h"

MidiMonitor::MidiMonitor()
{
    title.setText ("Messages MIDI recus", juce::dontSendNotification);
    title.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    addAndMakeVisible (title);

    lastEvent.setText ("Appuie sur un switch de ton pedalier...", juce::dontSendNotification);
    lastEvent.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    lastEvent.setJustificationType (juce::Justification::centred);
    lastEvent.setColour (juce::Label::textColourId, juce::Colour (0xffffb000));
    addAndMakeVisible (lastEvent);

    log.setMultiLine (true);
    log.setReadOnly (true);
    log.setCaretVisible (false);
    log.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    addAndMakeVisible (log);

    clearButton.onClick = [this]
    {
        lines.clear();
        log.clear();
    };
    addAndMakeVisible (clearButton);
}

void MidiMonitor::handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (message.isActiveSense() || message.isMidiClock())
        return; // noise, not useful here

    {
        const juce::ScopedLock sl (pendingLock);
        pending.push_back ({ source != nullptr ? source->getName() : juce::String ("?"),
                             describe (message) });
    }
    triggerAsyncUpdate();
}

void MidiMonitor::handleAsyncUpdate()
{
    std::vector<Entry> incoming;
    {
        const juce::ScopedLock sl (pendingLock);
        incoming.swap (pending);
    }

    if (incoming.empty())
        return;

    const auto time = juce::Time::getCurrentTime().toString (false, true, true, true);

    for (const auto& e : incoming)
        lines.insert (0, time + "  [" + e.device + "]  " + e.text);

    lines.removeRange (maxLines, lines.size());
    lastEvent.setText (incoming.back().text, juce::dontSendNotification);
    log.setText (lines.joinIntoString ("\n"), false);
}

juce::String MidiMonitor::describe (const juce::MidiMessage& m)
{
    const auto channel = "canal " + juce::String (m.getChannel());

    if (m.isController())
        return "CC " + juce::String (m.getControllerNumber())
             + " = " + juce::String (m.getControllerValue()) + "  (" + channel + ")";

    if (m.isProgramChange())
        return "Program Change " + juce::String (m.getProgramChangeNumber()) + "  (" + channel + ")";

    if (m.isNoteOn())
        return "Note ON " + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 4)
             + " vel " + juce::String (m.getVelocity()) + "  (" + channel + ")";

    if (m.isNoteOff())
        return "Note OFF " + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 4)
             + "  (" + channel + ")";

    if (m.isPitchWheel())
        return "Pitch bend " + juce::String (m.getPitchWheelValue()) + "  (" + channel + ")";

    if (m.isSysEx())
        return "SysEx (" + juce::String (m.getSysExDataSize()) + " octets)";

    return m.getDescription();
}

void MidiMonitor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff23262b));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);
}

void MidiMonitor::resized()
{
    auto area = getLocalBounds().reduced (12);

    auto header = area.removeFromTop (28);
    clearButton.setBounds (header.removeFromRight (90));
    title.setBounds (header);

    area.removeFromTop (6);
    lastEvent.setBounds (area.removeFromTop (48));
    area.removeFromTop (6);
    log.setBounds (area);
}
