#pragma once

#include <juce_audio_devices/juce_audio_devices.h>

#include <atomic>

/**
    Real-time audio callback for the test bench.

    Copies the selected input channel (the guitar) to every output channel,
    and tracks the input peak level so the UI can draw a meter.
    Nothing here allocates or locks: it is safe to run on the audio thread.
*/
class PassThroughEngine final : public juce::AudioIODeviceCallback
{
public:
    /** Which input channel feeds the outputs (0 = first active input). */
    void setInputChannel (int channel) noexcept   { inputChannel.store (channel); }
    int getInputChannel() const noexcept          { return inputChannel.load(); }

    /** Output gain, linear. */
    void setGain (float newGain) noexcept         { gain.store (newGain); }

    /** Returns the peak input level since the last call, then resets it. */
    float popInputPeak() noexcept                 { return inputPeak.exchange (0.0f); }

    /** True if at least one sample hit full scale since the last call. */
    bool popClipped() noexcept                    { return clipped.exchange (false); }

    // juce::AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext (const float* const* inputs, int numInputs,
                                           float* const* outputs, int numOutputs,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext&) override;
    void audioDeviceAboutToStart (juce::AudioIODevice*) override {}
    void audioDeviceStopped() override {}

private:
    std::atomic<int>   inputChannel { 0 };
    std::atomic<float> gain         { 1.0f };
    std::atomic<float> inputPeak    { 0.0f };
    std::atomic<bool>  clipped      { false };
};
