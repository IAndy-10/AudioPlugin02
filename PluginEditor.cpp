#include "PluginProcessor.h"
#include "PluginEditor.h"

SimpleGainAudioProcessorEditor::SimpleGainAudioProcessorEditor (SimpleGainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
    
    addAndMakeVisible (webView);
    
    // Load the HTML file from binary data
    juce::String htmlContent = BinaryData::interface_html;
    webView.goToURL ("data:text/html;charset=utf-8," + juce::URL::addEscapeChars (htmlContent, false));
    
    // Start timer to update UI
    startTimer (50);
}

SimpleGainAudioProcessorEditor::~SimpleGainAudioProcessorEditor()
{
    stopTimer();
}

void SimpleGainAudioProcessorEditor::timerCallback()
{
    // Get current gain value and send to web UI
    auto gain = audioProcessor.apvts.getRawParameterValue("gain")->load();
    auto gainPercent = static_cast<int>(gain * 100);
    
    juce::String js = "if(window.updateGainFromPlugin) { window.updateGainFromPlugin(" + 
                      juce::String(gainPercent) + "); }";
    webView.evaluateJavascript (js);
}

void SimpleGainAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SimpleGainAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
}