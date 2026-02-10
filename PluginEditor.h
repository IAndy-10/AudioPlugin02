#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
/**
 * Audio Processor Editor with WebBrowserComponent
 * Provides an HTML/CSS/JavaScript based user interface
 */
class WebViewPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer {
public:
  explicit WebViewPluginAudioProcessorEditor(WebViewPluginAudioProcessor &);
  ~WebViewPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  //==============================================================================
  void timerCallback() override;

  // Get resources for the web browser (HTML, CSS, JS files)
  std::optional<juce::WebBrowserComponent::Resource>
  getResource(const juce::String &url);

  // Load file content as string
  juce::String loadResourceFile(const juce::String &filename);

  // Helper to convert stream to vector
  static std::vector<std::byte> streamToVector(juce::InputStream &stream);

  //==============================================================================
  // Native functions callable from JavaScript
  void setupNativeFunctions();

  // Callbacks for parameter changes from web UI
  void onGainChanged(float gain);
  void onFrequencyChanged(float midiNote);
  void onFilterTypeChanged(int type);
  void onPulseWidthChanged(float width);
  void onNoiseColorChanged(int color);
  void onNoiseFadeInChanged(float fadeInMs);
  void onPluckTriggered();
  void onWaveformTypeChanged(int type);
  void onPulseFilterChanged(float amount);

  //==============================================================================
  WebViewPluginAudioProcessor &processorRef;

  // Web browser component
  juce::WebBrowserComponent webComponent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
      WebViewPluginAudioProcessorEditor)
};