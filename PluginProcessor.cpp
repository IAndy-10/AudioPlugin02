#include "PluginProcessor.h"
#include "PluginEditor.h"

std::unique_ptr<plugin01::FMSaw> osc;
std::unique_ptr<plugin01::FMPulse> osc2;
std::unique_ptr<plugin01::NoiseGenerator> noise;

//==============================================================================
// 1. Parameters

juce::AudioProcessorValueTreeState::ParameterLayout
WebViewPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(ID::gain, "Gain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ID::frequency, "MIDI Note", juce::NormalisableRange<float>(36.0f, 96.0f, 1.0f), 60.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ID::filterType, "Filter Type", juce::StringArray{ "Low Pass", "High Pass", "Band Pass" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ID::pulseWidth, "Pulse Width", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ID::noiseColor, "Noise Color", juce::StringArray{ "White", "Pink", "Brown" }, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ID::noiseFadeIn, "Noise Fade In", juce::NormalisableRange<float>(0.0f, 1000.0f, 1.0f), 200.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ID::pluck, "Pluck", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ID::waveformType, "Waveform", juce::StringArray{ "Pulse", "Noise", "Karplus" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ID::pulseFilter, "Pulse Filter", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    return layout;
}

WebViewPluginAudioProcessor::WebViewPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Parameters", createParameterLayout())
{
}

WebViewPluginAudioProcessor::~WebViewPluginAudioProcessor() = default;

//==============================================================================
// 2. Prepare to Play

void WebViewPluginAudioProcessor::prepareToPlay(double sampleRate,
                                                int samplesPerBlock) {
  osc = std::make_unique<plugin01::FMSaw>((float)sampleRate);
  osc2 = std::make_unique<plugin01::FMPulse>((float)sampleRate);
  noise = std::make_unique<plugin01::NoiseGenerator>((float)sampleRate);
  karplus = std::make_unique<plugin01::KarplusStrong>((float)sampleRate);

  float initFreq = juce::MidiMessage::getMidiNoteInHertz(60.0f);
  
  osc->frequency(initFreq);
  osc2->frequency(initFreq);
  noise->frequency(initFreq);
  karplus->frequency(initFreq);
}

//==============================================================================
// 3. Release Resources

void WebViewPluginAudioProcessor::releaseResources() {
  // Release oscillator resources
}

//==============================================================================
// 4. Process Block

void WebViewPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // 1. Clear unused channels
    for (auto i = getTotalNumInputChannels(); i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 2. Fetch parameters using the correct ID strings
    float gainDb       = state.getRawParameterValue(ID::gain.getParamID())->load();
    float freqMidi     = state.getRawParameterValue(ID::frequency.getParamID())->load();
    int waveformChoice = (int)state.getRawParameterValue(ID::waveformType.getParamID())->load();
    float pw           = state.getRawParameterValue(ID::pulseWidth.getParamID())->load();
    int noiseColor     = (int)state.getRawParameterValue(ID::noiseColor.getParamID())->load();
    float noiseFadeIn  = state.getRawParameterValue(ID::noiseFadeIn.getParamID())->load();
    float pulseFilt    = state.getRawParameterValue(ID::pulseFilter.getParamID())->load();

    // 3. Convert units
    float gainAmp = juce::Decibels::decibelsToGain(gainDb);
    float freqHz  = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(freqMidi));

    if (osc != nullptr && osc2 != nullptr && noise != nullptr && karplus != nullptr) {
        // Set oscillator parameters
        osc->frequency(freqHz);
        osc->filter(pulseFilt);

        osc2->frequency(freqHz);
        osc2->filter(pulseFilt);
        osc2->pulseWidth(pw);

        noise->frequency(freqHz);
        noise->color(noiseColor);
        noise->fadeIn(noiseFadeIn);

        // Set Karplus frequency (only when Karplus is selected for stability)
        if (waveformChoice == 3) {
            karplus->frequency(freqHz);
        }

        // Pluck detection using float parameter with threshold
        float pluckValue = state.getRawParameterValue(ID::pluck.getParamID())->load();
        
        // Trigger on rising edge (value goes from <0.5 to >=0.5)
        if (pluckValue >= 0.5f && lastPluckValue < 0.5f) {
            DBG("***** PLUCK TRIGGERED! Value: " << pluckValue << " *****");
            karplus->pluck();
        }
        lastPluckValue = pluckValue;

        auto *leftChannel = buffer.getWritePointer(0);
        auto *rightChannel = (totalNumOutputChannels > 1) ? buffer.getWritePointer(1) : nullptr;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float outSample = 0.0f;

            // Select waveform based on choice
            if (waveformChoice == 0) {
                outSample = osc->process() * gainAmp;
            } else if (waveformChoice == 1) {
                outSample = osc2->process() * gainAmp;
            } else if (waveformChoice == 2) {
                outSample = noise->process() * gainAmp;
            } else if (waveformChoice == 3) {
                outSample = karplus->operator()() * gainAmp;
            }

            leftChannel[sample] = outSample;
            if (rightChannel)
                rightChannel[sample] = outSample;
        }
    }
}

//==============================================================================
// 5. Bus Layout Support

bool WebViewPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

//==============================================================================
// 6. Editor & Plugin Info

bool WebViewPluginAudioProcessor::hasEditor() const { 
  return true; 
}

juce::AudioProcessorEditor *WebViewPluginAudioProcessor::createEditor() {
  return new WebViewPluginAudioProcessorEditor(*this);
}

const juce::String WebViewPluginAudioProcessor::getName() const {
  return "WebViewPlugin";
}

bool WebViewPluginAudioProcessor::acceptsMidi() const { 
  return false; 
}

bool WebViewPluginAudioProcessor::producesMidi() const { 
  return false; 
}

bool WebViewPluginAudioProcessor::isMidiEffect() const { 
  return false; 
}

double WebViewPluginAudioProcessor::getTailLengthSeconds() const { 
  return 0.0; 
}

//==============================================================================
// 7. Program Management

int WebViewPluginAudioProcessor::getNumPrograms() { 
  return 1; 
}

int WebViewPluginAudioProcessor::getCurrentProgram() { 
  return 0; 
}

void WebViewPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String WebViewPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void WebViewPluginAudioProcessor::changeProgramName(int index, 
                                                     const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
// 8. Parameter Listener

void WebViewPluginAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue) {
    // Leave empty or add logic for specific parameter reactions here
    juce::ignoreUnused(parameterID, newValue);
}

//==============================================================================
// 9. State Management

void WebViewPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto stateTree = state.copyState();
  std::unique_ptr<juce::XmlElement> xml(stateTree.createXml());
  copyXmlToBinary(*xml, destData);
}

void WebViewPluginAudioProcessor::setStateInformation(const void *data, 
                                                      int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

  if (xmlState != nullptr)
    if (xmlState->hasTagName(state.state.getType()))
      state.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// 10. Plugin Factory

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new WebViewPluginAudioProcessor();
}