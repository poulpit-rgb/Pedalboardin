#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
    Listens to every MIDI input (e.g. the Hotone Ampero Control) and shows
    each incoming message in plain French, newest first.

    This is how you find out which message each footswitch sends, before
    assigning it to virtual pedals in step 3.
*/
class MidiMonitor final : public juce::Component,
                          public juce::MidiInputCallback,
                          private juce::AsyncUpdater
{
public:
    MidiMonitor();

    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage&) override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Human-readable description of a MIDI message (public for tests). */
    static juce::String describe (const juce::MidiMessage&);

private:
    void handleAsyncUpdate() override;

    struct Entry { juce::String device, text; };

    juce::CriticalSection pendingLock;
    std::vector<Entry> pending;      // filled by the MIDI thread
    juce::StringArray lines;         // shown on screen (message thread only)

    juce::Label title, lastEvent;
    juce::TextEditor log;
    juce::TextButton clearButton { "Effacer" };

    static constexpr int maxLines = 200;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMonitor)
};
