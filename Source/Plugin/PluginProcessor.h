#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor {
public:
  //==============================================================================
  AudioPluginAudioProcessor();
  ~AudioPluginAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  using AudioProcessor::processBlock;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //==============================================================================
  // Audio playback methods
  void loadAudioFile(const juce::File &audioFile);
  void loadAudioFromMemory(const void *data, int sizeInBytes);
  void playAudio();
  void stopAudio();
  bool isAudioPlaying() const { return isPlaying; }
  juce::AudioProcessorValueTreeState apvts;

private:
  // Audio file playback members
  juce::AudioFormatManager audioFormatManager;
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  std::unique_ptr<juce::AudioTransportSource> transportSource;

  // Playback state
  bool isPlaying = false;
  double currentSampleRate = 44100.0;
  bool audioFileLoaded = false;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};