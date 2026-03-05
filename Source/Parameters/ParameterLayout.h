#pragma once
#include "ParameterIDs.h"

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    using namespace PluginParamIDs;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ---- Input Filter ----
    params.push_back(std::make_unique<juce::AudioParameterBool>(loCutEnabled, "Lo Cut", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(hiCutEnabled, "Hi Cut", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        loCutFreq, "Lo Cut Freq",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.5f), 80.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        hiCutFreq, "Hi Cut Freq",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.5f), 8000.0f));

    // ---- Early Reflections ----
    params.push_back(std::make_unique<juce::AudioParameterBool>(erEnabled, "ER Spin", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        erAmount, "ER Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        erRate, "ER Rate",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        erShape, "ER Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    // ---- FDN Core ----
    params.push_back(std::make_unique<juce::AudioParameterInt>(reverbMode, "Mode", 0, 1, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        crossoverFreq, "Crossover Freq",
        juce::NormalisableRange<float>(200.0f, 8000.0f, 1.0f, 0.5f), 3000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        diffusion, "Diffusion",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        scale, "Scale",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        decay, "Decay",
        juce::NormalisableRange<float>(100.0f, 10000.0f, 1.0f, 0.5f), 1500.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        damping, "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        feedback, "Feedback",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.75f));

    // ---- Chorus ----
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        chorusAmount, "Chorus Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        chorusRate, "Chorus Rate",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 1.5f));

    // ---- Output ----
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        reflectGain, "Reflect Gain",
        juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        diffuseGain, "Diffuse Gain",
        juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        dryWet, "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));

    // ---- Utility Row ----
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        predelay, "Predelay",
        juce::NormalisableRange<float>(0.0f, 500.0f, 0.1f, 0.5f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>(smooth,  "Smooth",  0, 3, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        size, "Size",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(freeze,  "Freeze",  false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(flatCut, "Flat/Cut", 0, 1, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        stereo, "Stereo",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>(density, "Density", 0, 3, 1));

    return { params.begin(), params.end() };
}
