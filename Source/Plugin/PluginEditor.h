#pragma once

#include "PluginProcessor.h"
#include "WebviewBridge.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;
  void timerCallback() override;

private:
  AudioPluginAudioProcessor &processorRef;

  std::unique_ptr<WebViewBridge> webView;
  
  // Handle parameter changes from JavaScript
  void onParameterChangedFromJS(const juce::String& paramName, float value);
  
  // Send parameter updates to JavaScript
  void sendGainToJS(float gainValue);
  
  float lastGainValue = -60.0f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};