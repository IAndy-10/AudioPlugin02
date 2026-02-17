#pragma once
#include "ParameterIDs.h"

inline juce::AudioProcessorValueTreeState::ParameterLayout
createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      PluginParamIDs::gain, "Gain", -60.0f, 0.0f, -60.0f));

  params.push_back(std::make_unique<juce::AudioParameterChoice>(
      PluginParamIDs::waveform, "Waveform", juce::StringArray{"Saw", "Noise"}, 0));

  return {params.begin(), params.end()};
}
