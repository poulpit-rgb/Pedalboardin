#include "PassThroughEngine.h"

#include <algorithm>
#include <cmath>

void PassThroughEngine::audioDeviceIOCallbackWithContext (const float* const* inputs, int numInputs,
                                                          float* const* outputs, int numOutputs,
                                                          int numSamples,
                                                          const juce::AudioIODeviceCallbackContext&)
{
    const int channel = inputChannel.load();
    const float* in = (channel >= 0 && channel < numInputs) ? inputs[channel] : nullptr;

    if (in == nullptr)
    {
        for (int ch = 0; ch < numOutputs; ++ch)
            if (outputs[ch] != nullptr)
                juce::FloatVectorOperations::clear (outputs[ch], numSamples);
        return;
    }

    // Input metering
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        peak = std::max (peak, std::abs (in[i]));

    if (peak >= 0.999f)
        clipped.store (true);

    float previous = inputPeak.load();
    while (peak > previous && ! inputPeak.compare_exchange_weak (previous, peak)) {}

    // Mono guitar in -> every output channel
    const float g = gain.load();
    for (int ch = 0; ch < numOutputs; ++ch)
        if (outputs[ch] != nullptr)
            juce::FloatVectorOperations::copyWithMultiply (outputs[ch], in, g, numSamples);
}
