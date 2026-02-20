#include "PluginProcessor.h"
#include "ParameterLayout.h"
#include "PluginEditor.h"
#include <fstream>

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
      apvts(*this, nullptr, "Parameters", createParameterLayout()) {
  // Initialize audio format manager
  audioFormatManager.registerBasicFormats();

  // Initialize transport source
  transportSource = std::make_unique<juce::AudioTransportSource>();
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
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  currentSampleRate = sampleRate;

  // Prepare the transport source
  transportSource->prepareToPlay(samplesPerBlock, sampleRate);
}

void AudioPluginAudioProcessor::releaseResources() {
  transportSource->releaseResources();
}
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

  float gainDb =
      apvts.getRawParameterValue(PluginParamIDs::gain.getParamID())->load();

  bool shouldPlayAudio =
      apvts.getRawParameterValue(PluginParamIDs::playing.getParamID())->load() >
      0.5f;

  if (!shouldPlayAudio) {
    buffer.clear();
    return;
  }

  float gainAmp = juce::Decibels::decibelsToGain(gainDb);

  // Get audio data from the transport source
  juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
  transportSource->getNextAudioBlock(info);

  // Apply gain to the output
  buffer.applyGain(gainAmp);
}

// =============================================================================
void AudioPluginAudioProcessor::loadAudioFile(const juce::File &audioFile) {
  if (!audioFile.existsAsFile()) {
    juce::Logger::getCurrentLogger()->writeToLog("Audio file not found: " +
                                                 audioFile.getFullPathName());
    return;
  }

  auto reader = audioFormatManager.createReaderFor(audioFile);
  if (reader != nullptr) {
    readerSource =
        std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    // Must set source before (re-)preparing the transport
    transportSource->setSource(readerSource.get(),
                               0, // read-ahead buffer size (0 = synchronous)
                               nullptr, // time-slice thread
                               reader->sampleRate,
                               2); // num channels
    // If audio is already running, re-prepare with current settings
    if (currentSampleRate > 0.0)
      transportSource->prepareToPlay(512, currentSampleRate);
    juce::Logger::getCurrentLogger()->writeToLog("Audio file loaded: " +
                                                 audioFile.getFullPathName());
  } else {
    juce::Logger::getCurrentLogger()->writeToLog("Failed to read audio file: " +
                                                 audioFile.getFullPathName());
  }
}

void AudioPluginAudioProcessor::loadAudioFromMemory(const void *data,
                                                    int sizeInBytes) {
  // JUCE 7+ requires unique_ptr ownership transfer for createReaderFor
  auto stream = std::make_unique<juce::MemoryInputStream>(
      data, (size_t)sizeInBytes, false);
  auto reader = audioFormatManager.createReaderFor(std::move(stream));

  if (reader != nullptr) {
    readerSource =
        std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transportSource->setSource(readerSource.get(), 0, nullptr,
                               reader->sampleRate, 2);
    if (currentSampleRate > 0.0)
      transportSource->prepareToPlay(512, currentSampleRate);
    juce::Logger::getCurrentLogger()->writeToLog(
        "Audio loaded from binary data - OK");
  } else {
    // stream is already deleted by unique_ptr if createReaderFor failed
    juce::Logger::getCurrentLogger()->writeToLog(
        "Failed to decode audio from binary data");
  }
}

void AudioPluginAudioProcessor::playAudio() {
  if (readerSource != nullptr) {
    transportSource->start();
    isPlaying = true;
    juce::Logger::getCurrentLogger()->writeToLog("Playing audio");
  } else {
    juce::Logger::getCurrentLogger()->writeToLog("No audio file loaded");
  }
}

void AudioPluginAudioProcessor::stopAudio() {
  transportSource->stop();
  transportSource->setPosition(0.0);
  isPlaying = false;
  juce::Logger::getCurrentLogger()->writeToLog("Stopped audio");
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