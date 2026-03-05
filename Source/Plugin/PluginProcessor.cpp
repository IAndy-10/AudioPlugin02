#include "PluginProcessor.h"
#include "ParameterLayout.h"
#include "ParameterIDs.h"
#include "PluginEditor.h"

using namespace PluginParamIDs;

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input",  juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

const juce::String AudioPluginAudioProcessor::getName() const { return JucePlugin_Name; }
bool AudioPluginAudioProcessor::acceptsMidi()  const { return true;  }
bool AudioPluginAudioProcessor::producesMidi() const { return false; }
bool AudioPluginAudioProcessor::isMidiEffect() const { return false; }
double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 10.0; }
int AudioPluginAudioProcessor::getNumPrograms() { return 1; }
int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }
void AudioPluginAudioProcessor::setCurrentProgram(int) {}
const juce::String AudioPluginAudioProcessor::getProgramName(int) { return {}; }
void AudioPluginAudioProcessor::changeProgramName(int, const juce::String&) {}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}

void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(samplesPerBlock);

    // Prepare DSP modules
    inputFilter.prepare(sampleRate);
    predelayModule.prepare(sampleRate);
    earlyReflections.prepare(sampleRate);
    fdnReverb.prepare(sampleRate);
    chorusModule.prepare(sampleRate);
    stereoWidener.setWidth(1.0f);

    // Prepare smoothed values (10ms smoothing time)
    auto initSmooth = [&](juce::SmoothedValue<float>& sv, float initial) {
        sv.reset(sampleRate, 0.01);
        sv.setCurrentAndTargetValue(initial);
    };

    initSmooth(smoothDecay,         apvts.getRawParameterValue(decay.getParamID())->load());
    initSmooth(smoothDiffusion,     apvts.getRawParameterValue(diffusion.getParamID())->load());
    initSmooth(smoothSize,          apvts.getRawParameterValue(size.getParamID())->load());
    initSmooth(smoothDamping,       apvts.getRawParameterValue(damping.getParamID())->load());
    initSmooth(smoothFeedback,      apvts.getRawParameterValue(feedback.getParamID())->load());
    initSmooth(smoothPredelay,      apvts.getRawParameterValue(PluginParamIDs::predelay.getParamID())->load());
    initSmooth(smoothStereo,        apvts.getRawParameterValue(stereo.getParamID())->load());
    initSmooth(smoothDryWet,        apvts.getRawParameterValue(dryWet.getParamID())->load());
    initSmooth(smoothChorusAmount,  apvts.getRawParameterValue(chorusAmount.getParamID())->load());
    initSmooth(smoothChorusRate,    apvts.getRawParameterValue(chorusRate.getParamID())->load());
    initSmooth(smoothCrossoverFreq, apvts.getRawParameterValue(crossoverFreq.getParamID())->load());
    initSmooth(smoothReflectGain,   apvts.getRawParameterValue(reflectGain.getParamID())->load());
    initSmooth(smoothDiffuseGain,   apvts.getRawParameterValue(diffuseGain.getParamID())->load());
    initSmooth(smoothErAmount,      apvts.getRawParameterValue(erAmount.getParamID())->load());
    initSmooth(smoothErRate,        apvts.getRawParameterValue(erRate.getParamID())->load());
    initSmooth(smoothLoCutFreq,     apvts.getRawParameterValue(loCutFreq.getParamID())->load());
    initSmooth(smoothHiCutFreq,     apvts.getRawParameterValue(hiCutFreq.getParamID())->load());

    updateDSPParameters();
}

void AudioPluginAudioProcessor::releaseResources() {}

void AudioPluginAudioProcessor::updateDSPParameters() {
    // Input filter
    inputFilter.setLoCutEnabled(apvts.getRawParameterValue(loCutEnabled.getParamID())->load() > 0.5f);
    inputFilter.setHiCutEnabled(apvts.getRawParameterValue(hiCutEnabled.getParamID())->load() > 0.5f);
    inputFilter.setLoCutFreq(apvts.getRawParameterValue(loCutFreq.getParamID())->load());
    inputFilter.setHiCutFreq(apvts.getRawParameterValue(hiCutFreq.getParamID())->load());

    // Early reflections
    earlyReflections.setEnabled(apvts.getRawParameterValue(erEnabled.getParamID())->load() > 0.5f);
    earlyReflections.setAmount(apvts.getRawParameterValue(erAmount.getParamID())->load());
    earlyReflections.setRate(apvts.getRawParameterValue(erRate.getParamID())->load());
    earlyReflections.setShape(apvts.getRawParameterValue(erShape.getParamID())->load());

    // FDN
    fdnReverb.setDecayMs(apvts.getRawParameterValue(decay.getParamID())->load());
    fdnReverb.setDiffusion(apvts.getRawParameterValue(diffusion.getParamID())->load());
    fdnReverb.setSize(apvts.getRawParameterValue(size.getParamID())->load());
    fdnReverb.setDamping(apvts.getRawParameterValue(damping.getParamID())->load());
    fdnReverb.setFeedback(apvts.getRawParameterValue(feedback.getParamID())->load());
    fdnReverb.setCrossoverFreq(apvts.getRawParameterValue(crossoverFreq.getParamID())->load());
    fdnReverb.setReverbMode(static_cast<int>(apvts.getRawParameterValue(reverbMode.getParamID())->load()));
    fdnReverb.setFrozen(apvts.getRawParameterValue(PluginParamIDs::freeze.getParamID())->load() > 0.5f);

    // Chorus
    chorusModule.setAmount(apvts.getRawParameterValue(chorusAmount.getParamID())->load());
    chorusModule.setRate(apvts.getRawParameterValue(chorusRate.getParamID())->load());

    // Predelay
    predelayModule.setDelayMs(apvts.getRawParameterValue(PluginParamIDs::predelay.getParamID())->load());

    // Widener
    stereoWidener.setWidth(apvts.getRawParameterValue(stereo.getParamID())->load());

    // Mixer
    dryWetMixer.setMix(apvts.getRawParameterValue(dryWet.getParamID())->load() / 100.0f);
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = getTotalNumInputChannels(); i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Save dry signal for mixing later
    dryWetMixer.saveDry(buffer);

    // --- Signal chain ---
    // 1. Input filter
    inputFilter.process(buffer);

    // 2. Predelay
    predelayModule.setDelayMs(apvts.getRawParameterValue(PluginParamIDs::predelay.getParamID())->load());
    predelayModule.process(buffer);

    // 3. Early reflections (additive)
    earlyReflections.setEnabled(apvts.getRawParameterValue(erEnabled.getParamID())->load() > 0.5f);
    earlyReflections.process(buffer);

    // 4. FDN reverb (replaces signal with reverb output)
    bool isFrozen = apvts.getRawParameterValue(PluginParamIDs::freeze.getParamID())->load() > 0.5f;
    fdnReverb.setFrozen(isFrozen);
    fdnReverb.setDecayMs(apvts.getRawParameterValue(decay.getParamID())->load());
    fdnReverb.setDiffusion(apvts.getRawParameterValue(diffusion.getParamID())->load());
    fdnReverb.process(buffer);

    // 5. Apply reflect and diffuse gains
    float rGain = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue(reflectGain.getParamID())->load());
    float dGain = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue(diffuseGain.getParamID())->load());
    buffer.applyGain(rGain * dGain);

    // 6. Chorus
    chorusModule.setAmount(apvts.getRawParameterValue(chorusAmount.getParamID())->load());
    chorusModule.process(buffer);

    // 7. Stereo widener
    stereoWidener.setWidth(apvts.getRawParameterValue(stereo.getParamID())->load());
    stereoWidener.process(buffer);

    // 8. Dry/wet mix
    dryWetMixer.setMix(apvts.getRawParameterValue(dryWet.getParamID())->load() / 100.0f);
    dryWetMixer.mixToOutput(buffer);
}

bool AudioPluginAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
    return new AudioPluginAudioProcessorEditor(*this);
}

void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}
