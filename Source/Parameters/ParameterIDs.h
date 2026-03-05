#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace PluginParamIDs {
    // Input Filter
    const juce::ParameterID loCutEnabled { "loCutEnabled", 1 };
    const juce::ParameterID hiCutEnabled { "hiCutEnabled", 1 };
    const juce::ParameterID loCutFreq    { "loCutFreq",    1 };
    const juce::ParameterID hiCutFreq    { "hiCutFreq",    1 };

    // Early Reflections
    const juce::ParameterID erEnabled { "erEnabled", 1 };
    const juce::ParameterID erAmount  { "erAmount",  1 };
    const juce::ParameterID erRate    { "erRate",    1 };
    const juce::ParameterID erShape   { "erShape",   1 };

    // FDN Reverb Core
    const juce::ParameterID reverbMode    { "reverbMode",    1 };
    const juce::ParameterID crossoverFreq { "crossoverFreq", 1 };
    const juce::ParameterID diffusion     { "diffusion",     1 };
    const juce::ParameterID scale         { "scale",         1 };
    const juce::ParameterID decay         { "decay",         1 };
    const juce::ParameterID damping       { "damping",       1 };
    const juce::ParameterID feedback      { "feedback",      1 };

    // Chorus
    const juce::ParameterID chorusAmount { "chorusAmount", 1 };
    const juce::ParameterID chorusRate   { "chorusRate",   1 };

    // Output / Reflect
    const juce::ParameterID reflectGain  { "reflectGain",  1 };
    const juce::ParameterID diffuseGain  { "diffuseGain",  1 };
    const juce::ParameterID dryWet       { "dryWet",       1 };

    // Bottom Utility Row
    const juce::ParameterID predelay { "predelay", 1 };
    const juce::ParameterID smooth   { "smooth",   1 };
    const juce::ParameterID size     { "size",     1 };
    const juce::ParameterID freeze   { "freeze",   1 };
    const juce::ParameterID flatCut  { "flatCut",  1 };
    const juce::ParameterID stereo   { "stereo",   1 };
    const juce::ParameterID density  { "density",  1 };
}
