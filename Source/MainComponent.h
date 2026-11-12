#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "MidiMonitor.h"
#include "PassThroughEngine.h"

/**
    Step 1 test bench:
      - choose the audio interface (Focusrite Scarlett) and its settings,
      - hear the guitar go straight through (no effects yet),
      - read the round-trip latency,
      - see what each footswitch of the MIDI controller sends.
*/
class MainComponent final : public juce::Component,
                            private juce::ChangeListener,
                            private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void timerCallback() override;

    void refreshDeviceInfo();
    void refreshInputChoices();
    void enableAllMidiInputs();

    juce::AudioDeviceManager deviceManager;
    PassThroughEngine engine;

    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;
    juce::Viewport selectorViewport;

    juce::Label inputLabel { {}, "Entree guitare :" };
    juce::ComboBox inputChoice;
    juce::Label gainLabel { {}, "Volume :" };
    juce::Slider gainSlider;
    juce::Label latencyLabel, deviceLabel;

    MidiMonitor midiMonitor;

    float displayedLevel = 0.0f;
    int clipHoldFrames = 0;
    juce::Rectangle<int> meterArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
