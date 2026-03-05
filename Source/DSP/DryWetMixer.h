#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

// Mixes a dry (input) buffer with a wet (processed) buffer.
// mix = 0.0 → all dry; mix = 1.0 → all wet.
class DryWetMixer {
public:
    void setMix(float mixAmount) { mix = mixAmount; }

    // Call before processing: saves the dry signal.
    void saveDry(const juce::AudioBuffer<float>& input) {
        int channels = input.getNumChannels();
        int samples  = input.getNumSamples();

        dryBuffer.setSize(channels, samples, false, false, true);
        for (int ch = 0; ch < channels; ++ch)
            dryBuffer.copyFrom(ch, 0, input, ch, 0, samples);
    }

    // Call after processing: blends dry and wet into output.
    void mixToOutput(juce::AudioBuffer<float>& wetOutput) {
        int channels = juce::jmin(wetOutput.getNumChannels(), dryBuffer.getNumChannels());
        int samples  = juce::jmin(wetOutput.getNumSamples(), dryBuffer.getNumSamples());

        float dryGain = 1.0f - mix;
        float wetGain = mix;

        for (int ch = 0; ch < channels; ++ch) {
            auto* wet = wetOutput.getWritePointer(ch);
            auto* dry = dryBuffer.getReadPointer(ch);
            for (int i = 0; i < samples; ++i)
                wet[i] = dryGain * dry[i] + wetGain * wet[i];
        }
    }

private:
    float mix = 0.5f;
    juce::AudioBuffer<float> dryBuffer;
};
