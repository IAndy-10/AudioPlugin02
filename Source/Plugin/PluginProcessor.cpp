#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "saw.h"
#include <fstream>

std::unique_ptr<plugin01::FMSaw> osc;

static juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"gain", 1}, "Gain", -60.0f, 0.0f, -60.0f));

  parameter_list.push_back(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID{"waveform", 1}, "Waveform", juce::StringArray{"Saw"},
      0));

  return {parameter_list.begin(), parameter_list.end()};
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "Parameters", parameters()) {
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginAudioProcessor::getNumPrograms() { return 1; }

int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int) {
  osc = std::make_unique<plugin01::FMSaw>((float)sampleRate);

  float initFreq = (float)juce::MidiMessage::getMidiNoteInHertz(60.0f);

  osc->frequency(initFreq);
}

void AudioPluginAudioProcessor::releaseResources() {}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
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

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  juce::ignoreUnused(midiMessages);
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = getTotalNumInputChannels(); i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  float gainDb = apvts.getRawParameterValue("gain")->load();
  int waveformChoice = (int)apvts.getRawParameterValue("waveform")->load();

  float gainAmp = juce::Decibels::decibelsToGain(gainDb);

  // Get pointers to the audio channels
  float *leftChannel = buffer.getWritePointer(0);
  float *rightChannel =
      totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

  for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
    float outSample = 0.0f;

    if (waveformChoice == 0) {
      outSample = osc->process() * gainAmp;
    }

    leftChannel[sample] = outSample;
    if (rightChannel)
      rightChannel[sample] = outSample;
  }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}