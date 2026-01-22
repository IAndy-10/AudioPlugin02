#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SimpleGainAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    SimpleGainAudioProcessorEditor (SimpleGainAudioProcessor&);
    ~SimpleGainAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    
    SimpleGainAudioProcessor& audioProcessor;
    juce::WebBrowserComponent webView;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleGainAudioProcessorEditor)
};