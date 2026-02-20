// PluginEditor.cpp
#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "PluginProcessor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {

  // 1. Initialize the component FIRST
  webView = std::make_unique<WebViewBridge>();

  // 2. Set up its properties
  addAndMakeVisible(webView.get());

  webView->setParameterCallback(
      [this](const juce::String &paramName, float value) {
        onParameterChangedFromJS(paramName, value);
      });

  // Set up audio playback callbacks
  webView->setPlayCallback([this]() {
    juce::File audioFile = juce::File::getCurrentWorkingDirectory()
                               .getChildFile("Source")
                               .getChildFile("Audio")
                               .getChildFile("this-is-a-test1.wav");

    processorRef.loadAudioFile(audioFile);
    processorRef.playAudio();
  });

  webView->setStopCallback([this]() { processorRef.stopAudio(); });

  webView->setPauseCallback([this]() { processorRef.stopAudio(); });

  webView->setResumeCallback([this]() { processorRef.playAudio(); });

  // 3. ONLY THEN call setSize (which triggers resized())
  setSize(750, 500);

  // 4. Load your HTML/initial values after the UI is ready
  // Robust method: Extract BinaryData to a temp folder to avoid bundle path
  // issues

  auto tempDir =
      juce::File::getSpecialLocation(juce::File::tempDirectory)
          .getChildFile("AudioPlugin02_Resources_" +
                        juce::String(juce::Time::currentTimeMillis()));

  tempDir.createDirectory();

  // Helper to write binary data to file - explicitly using 'BinaryData::'
  // namespace
  auto unpackFile = [&](const char *data, int size,
                        const juce::String &filename) {
    auto file = tempDir.getChildFile(filename);
    file.replaceWithData(data, size);
    return file;
  };

  auto htmlFile = unpackFile(BinaryData::index_html, BinaryData::index_htmlSize,
                             "index.html");

  webView->goToURL("file://" + htmlFile.getFullPathName());

  startTimerHz(60);
  sendGainToJS(processorRef.apvts
                   .getRawParameterValue(PluginParamIDs::gain.getParamID())
                   ->load());
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
  stopTimer();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
  // WebView will handle all rendering
  g.fillAll(juce::Colour(0xff1e1e1e));
}

//==============================================================================
void AudioPluginAudioProcessorEditor::resized() {
  // Only set bounds if the pointer actually exists!
  if (webView != nullptr) {
    webView->setBounds(getLocalBounds());
  }
}
//==============================================================================
void AudioPluginAudioProcessorEditor::timerCallback() {
  // Poll for parameter changes and send to JavaScript
  float currentGain =
      processorRef.apvts
          .getRawParameterValue(PluginParamIDs::gain.getParamID())
          ->load();

  if (std::abs(currentGain - lastGainValue) > 0.01f) {
    lastGainValue = currentGain;
    sendGainToJS(currentGain);
  }
}

//==============================================================================
void AudioPluginAudioProcessorEditor::onParameterChangedFromJS(
    const juce::String &paramName, float value) {
  // Update the parameter in the processor
  if (paramName == PluginParamIDs::gain.getParamID()) {
    auto *param =
        processorRef.apvts.getParameter(PluginParamIDs::gain.getParamID());

    if (param != nullptr) {
      // Normalize the value to 0-1 range for JUCE parameter
      float normalizedValue = juce::jmap(value, -60.0f, 0.0f, 0.0f, 1.0f);
      param->setValueNotifyingHost(normalizedValue);
      lastGainValue = value; // Update cached value to prevent feedback loop
    }
  }

  // --- Handle play/stop from JS ---
  if (paramName == PluginParamIDs::playing.getParamID()) {
    auto *param =
        processorRef.apvts.getParameter(PluginParamIDs::playing.getParamID());
    if (param != nullptr)
      param->setValueNotifyingHost(value); // sync APVTS (0.0 or 1.0)

    if (value > 0.5f) {
      // Load from embedded binary data (compiled in from
      // Source/Audio/this-is-a-test1.wav)
      processorRef.loadAudioFromMemory(BinaryData::thisisatest1_wav,
                                       BinaryData::thisisatest1_wavSize);
      processorRef.playAudio();
    } else {
      processorRef.stopAudio();
    }
  }
}

//==============================================================================
void AudioPluginAudioProcessorEditor::sendGainToJS(float gainDb) {
  // Convert gain from dB (-60 to 0) to percentage (0 to 100)
  float percentage = juce::jmap(gainDb, -60.0f, 0.0f, 0.0f, 100.0f);

  // Functions that comes from the bridge.ts file
  juce::String jsCode =
      "if (window.setParameterValue) { window.setParameterValue('gain', " +
      juce::String(percentage) + "); }";

  webView->evaluateJavascript(jsCode);
}
