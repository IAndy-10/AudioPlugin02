#include "PluginEditor.h"

//==============================================================================
WebViewPluginAudioProcessorEditor::WebViewPluginAudioProcessorEditor(
    WebViewPluginAudioProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p),
      webComponent(
          juce::WebBrowserComponent::Options()
              .withBackend(
                  juce::WebBrowserComponent::Options::Backend::defaultBackend)
              .withNativeIntegrationEnabled()
              .withResourceProvider(
                  [this](const juce::String &url) { return getResource(url); })
              .withNativeFunction(
                  "setGain",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      float gain = (float)args[0];
                      onGainChanged(gain);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setFrequency",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      float midiNote = (float)args[0];
                      onFrequencyChanged(midiNote);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setFilterType",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      int type = (int)args[0];
                      onFilterTypeChanged(type);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setPulseWidth",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      float width = (float)args[0];
                      onPulseWidthChanged(width);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setNoiseColor",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      int color = (int)args[0];
                      onNoiseColorChanged(color);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setNoiseFadeIn",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      float fadeInMs = (float)args[0];
                      onNoiseFadeInChanged(fadeInMs);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "triggerPluck",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    onPluckTriggered();
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setWaveformType",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      int type = (int)args[0];
                      onWaveformTypeChanged(type);
                    }
                    complete(juce::var());
                  })
              .withNativeFunction(
                  "setPulseFilter",
                  [this](const juce::Array<juce::var> &args,
                         std::function<void(juce::var)> complete) {
                    if (args.size() > 0) {
                      float amount = (float)args[0];
                      onPulseFilterChanged(amount);
                    }
                    complete(juce::var());
                  })) {
  addAndMakeVisible(webComponent);

  // Load the web interface
  webComponent.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

  setSize(1000, 800);

  // Start timer for updating UI with current parameter values
  startTimerHz(30);
}

WebViewPluginAudioProcessorEditor::~WebViewPluginAudioProcessorEditor() {
  stopTimer();
}

//==============================================================================
void WebViewPluginAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void WebViewPluginAudioProcessorEditor::resized() {
  webComponent.setBounds(getLocalBounds());
}

//==============================================================================
void WebViewPluginAudioProcessorEditor::timerCallback() {
  // Get current parameter values
  auto gain =
      processorRef.state.getRawParameterValue(ID::gain.getParamID())->load();
  auto frequency =
      processorRef.state.getRawParameterValue(ID::frequency.getParamID())
          ->load();
  auto filterType =
      (int)processorRef.state.getRawParameterValue(ID::filterType.getParamID())
          ->load();
  auto pulseWidth =
      processorRef.state.getRawParameterValue(ID::pulseWidth.getParamID())
          ->load();
  auto noiseColor =
      (int)processorRef.state.getRawParameterValue(ID::noiseColor.getParamID())
          ->load();
  auto noiseFadeIn =
      processorRef.state.getRawParameterValue(ID::noiseFadeIn.getParamID())
          ->load();
  auto waveformType = (int)processorRef.state
                          .getRawParameterValue(ID::waveformType.getParamID())
                          ->load();
  auto pulseFilter =
      processorRef.state.getRawParameterValue(ID::pulseFilter.getParamID())
          ->load();

  // Send updates to JavaScript
  juce::String script =
      "if (typeof updateFromPlugin === 'function') {"
      "    updateFromPlugin(" +
      juce::String(gain) + ", " + juce::String(frequency) + ", " +
      juce::String(filterType) + ", " + juce::String(pulseWidth) + ", " +
      juce::String(noiseColor) + ", " + juce::String(noiseFadeIn) + ", " +
      juce::String(waveformType) + ", " + juce::String(pulseFilter) +
      ");"
      "}";

  webComponent.emitEventIfBrowserIsVisible("updateParameters", script);
}

//==============================================================================
void WebViewPluginAudioProcessorEditor::onGainChanged(float gain) {
  // Gain is already in 0-1 range
  if (auto *param = processorRef.state.getParameter(ID::gain.getParamID()))
    param->setValueNotifyingHost(gain);
}

void WebViewPluginAudioProcessorEditor::onFrequencyChanged(float midiNote) {
  // MIDI note needs to be normalized to 0-1 range (36-96 MIDI notes)
  float normalized = juce::jmap(midiNote, 36.0f, 96.0f, 0.0f, 1.0f);
  
  if (auto *param = processorRef.state.getParameter(ID::frequency.getParamID()))
    param->setValueNotifyingHost(normalized);
}

void WebViewPluginAudioProcessorEditor::onFilterTypeChanged(int type) {
  // Normalize to 0-1 range (0, 1, or 2 becomes 0.0, 0.5, or 1.0)
  float normalized = type / 2.0f;

  if (auto *param =
          processorRef.state.getParameter(ID::filterType.getParamID()))
    param->setValueNotifyingHost(normalized);
}

void WebViewPluginAudioProcessorEditor::onPulseWidthChanged(float width) {
  // Width is already in 0-1 range
  if (auto *param =
          processorRef.state.getParameter(ID::pulseWidth.getParamID()))
    param->setValueNotifyingHost(width);
}

void WebViewPluginAudioProcessorEditor::onNoiseColorChanged(int color) {
  // Normalize to 0-1 range (0, 1, or 2 becomes 0.0, 0.5, or 1.0)
  float normalized = color / 2.0f;

  if (auto *param =
          processorRef.state.getParameter(ID::noiseColor.getParamID()))
    param->setValueNotifyingHost(normalized);
}

void WebViewPluginAudioProcessorEditor::onNoiseFadeInChanged(float fadeInMs) {
  // Normalize fade-in time to 0-1 range (0-1000ms)
  float normalized = juce::jmap(fadeInMs, 0.0f, 1000.0f, 0.0f, 1.0f);
  
  if (auto *param =
          processorRef.state.getParameter(ID::noiseFadeIn.getParamID()))
    param->setValueNotifyingHost(normalized);
}

void WebViewPluginAudioProcessorEditor::onPluckTriggered() {
  // Trigger the pluck by setting the parameter to 1.0
  // The processor will reset it back to 0.0 after detecting the trigger
  if (auto *param = processorRef.state.getParameter(ID::pluck.getParamID()))
    param->setValueNotifyingHost(1.0f);
}

void WebViewPluginAudioProcessorEditor::onWaveformTypeChanged(int type) {
  // Normalize to 0-1 range based on number of waveform choices (0, 1, 2 for 3 types)
  float normalized = type / 2.0f;

  if (auto *param =
          processorRef.state.getParameter(ID::waveformType.getParamID()))
    param->setValueNotifyingHost(normalized);
}

void WebViewPluginAudioProcessorEditor::onPulseFilterChanged(float amount) {
  // Amount is already in 0-1 range
  if (auto *param =
          processorRef.state.getParameter(ID::pulseFilter.getParamID()))
    param->setValueNotifyingHost(amount);
}

//==============================================================================
std::vector<std::byte>
WebViewPluginAudioProcessorEditor::streamToVector(juce::InputStream &stream) {
  std::vector<std::byte> result((size_t)stream.getTotalLength());
  stream.setPosition(0);
  [[maybe_unused]] const auto bytesRead =
      stream.read(result.data(), result.size());
  jassert(bytesRead == (ssize_t)result.size());
  return result;
}

juce::String WebViewPluginAudioProcessorEditor::loadResourceFile(
    const juce::String &filename) {
  // Load actual files from Resources directory
  juce::File resourcesDir =
      juce::File::getSpecialLocation(juce::File::currentExecutableFile)
          .getParentDirectory()
          .getChildFile("Resources");

  // If running from build directory, try relative path
  if (!resourcesDir.exists()) {
    resourcesDir =
        juce::File::getCurrentWorkingDirectory().getChildFile("Resources");
  }

  // Try source directory (for development)
  if (!resourcesDir.exists()) {
    resourcesDir =
        juce::File(__FILE__).getParentDirectory().getChildFile("Resources");
  }

  juce::File resourceFile = resourcesDir.getChildFile(filename);

  if (resourceFile.existsAsFile()) {
    return resourceFile.loadFileAsString();
  }

  // Fallback: return empty string if file not found
  DBG("Resource file not found: " + filename);
  return {};
}

std::optional<juce::WebBrowserComponent::Resource>
WebViewPluginAudioProcessorEditor::getResource(const juce::String &url) {
  // Handle root URL
  juce::String urlToRetrieve =
      url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

  // Map of MIME types
  static const std::unordered_map<juce::String, const char *> mimeTypes = {
      {"html", "text/html"},
      {"css", "text/css"},
      {"js", "text/javascript"},
      {"json", "application/json"}};

  // Get file extension
  auto extension =
      urlToRetrieve.fromLastOccurrenceOf(".", false, false).toLowerCase();

  // Determine MIME type
  const char *mimeType = "text/plain";
  if (auto it = mimeTypes.find(extension); it != mimeTypes.end())
    mimeType = it->second;

  // Load the requested resource
  juce::String content = loadResourceFile(urlToRetrieve);

  if (content.isNotEmpty()) {
    juce::MemoryInputStream stream(content.toRawUTF8(),
                                   content.getNumBytesAsUTF8(), false);
    return juce::WebBrowserComponent::Resource{streamToVector(stream),
                                               mimeType};
  }

  return std::nullopt;
}