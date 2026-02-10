#pragma once

#include "noise.h"
#include "pulse.h"
#include "saw.h"
#include "KarplusStrong.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>

namespace ID {
    inline const juce::ParameterID gain         {"gain", 1};
    inline const juce::ParameterID frequency    {"frequency", 1};
    inline const juce::ParameterID filterType   {"filterType", 1};
    inline const juce::ParameterID pulseWidth   {"pulseWidth", 1};
    inline const juce::ParameterID noiseColor   {"noiseColor", 1};
    inline const juce::ParameterID noiseFadeIn  {"noiseFadeIn", 1};
    inline const juce::ParameterID pluck        {"pluck", 1};
    inline const juce::ParameterID waveformType  {"waveformType", 1};
    inline const juce::ParameterID pulseFilter  {"pulseFilter", 1};
}

class WebViewPluginAudioProcessor : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener 
{
public:
    WebViewPluginAudioProcessor();
    ~WebViewPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    // Required by Listener interface
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    // Helper for APVTS initialization
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Renamed to 'state' to match PluginProcessor.cpp usage
    juce::AudioProcessorValueTreeState state;

private:
    std::unique_ptr<plugin01::FMSaw> osc;
    std::unique_ptr<plugin01::FMPulse> osc2;
    std::unique_ptr<plugin01::NoiseGenerator> noise;
    std::unique_ptr<plugin01::KarplusStrong> karplus;
    
    float lastPluckValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebViewPluginAudioProcessor)
};