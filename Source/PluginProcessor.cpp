/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Static clipping functions
float AudioClipperAudioProcessor::clipTanh (float x)
{
    return std::tanh (x);
}

float AudioClipperAudioProcessor::clipArctan (float x)
{
    return (2.0f / juce::MathConstants<float>::pi) * std::atan (x);
}

float AudioClipperAudioProcessor::clipCubic (float x)
{
    // Soft saturation, clamp to prevent inversion outside [-1.5, 1.5]
    x = juce::jlimit (-1.5f, 1.5f, x);
    return x - (x * x * x) / 3.0f;
}

float AudioClipperAudioProcessor::clipHard (float x)
{
    return juce::jlimit (-1.0f, 1.0f, x);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AudioClipperAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("inputGain", 1),
        "Input Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
        0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("outputGain", 1),
        "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
        0.0f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("clipType", 1),
        "Clip Type",
        juce::StringArray { "Tanh", "Arctan", "Cubic", "Hard Clip" },
        0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("threshold", 1),
        "Threshold",
        juce::NormalisableRange<float> (-24.0f, 0.0f, 0.1f),
        0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("mix", 1),
        "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        100.0f));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("oversampling", 1),
        "Oversampling",
        juce::StringArray { "Off", "2x", "4x" },
        0));

    return layout;
}

AudioClipperAudioProcessor::AudioClipperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
       , parameters (*this, nullptr, juce::Identifier ("AudioClipper"), createParameterLayout())
{
    inputGainParam    = parameters.getRawParameterValue ("inputGain");
    outputGainParam   = parameters.getRawParameterValue ("outputGain");
    clipTypeParam     = parameters.getRawParameterValue ("clipType");
    thresholdParam    = parameters.getRawParameterValue ("threshold");
    mixParam          = parameters.getRawParameterValue ("mix");
    oversamplingParam = parameters.getRawParameterValue ("oversampling");

    jassert (inputGainParam != nullptr);
    jassert (outputGainParam != nullptr);
    jassert (clipTypeParam != nullptr);
    jassert (thresholdParam != nullptr);
    jassert (mixParam != nullptr);
    jassert (oversamplingParam != nullptr);

    // Initialize oversamplers (2 channels each)
    // Index 0: 1x (no oversampling - uses factor 0 which means 2^0 = 1x)
    oversamplers[0] = std::make_unique<juce::dsp::Oversampling<float>> (2, 0,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
    // Index 1: 2x (factor 1 = 2^1)
    oversamplers[1] = std::make_unique<juce::dsp::Oversampling<float>> (2, 1,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
    // Index 2: 4x (factor 2 = 2^2)
    oversamplers[2] = std::make_unique<juce::dsp::Oversampling<float>> (2, 2,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
}

AudioClipperAudioProcessor::~AudioClipperAudioProcessor()
{
}

//==============================================================================
const juce::String AudioClipperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioClipperAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AudioClipperAudioProcessor::producesMidi() const
{
    return false;
}

bool AudioClipperAudioProcessor::isMidiEffect() const
{
    return false;
}

double AudioClipperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioClipperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioClipperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioClipperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioClipperAudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioClipperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioClipperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare all oversamplers
    for (auto& os : oversamplers)
        os->initProcessing (static_cast<size_t> (samplesPerBlock));

    // Prepare dry/wet mixer
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumInputChannels());

    dryWetMixer.prepare (spec);
    dryWetMixer.setMixingRule (juce::dsp::DryWetMixingRule::linear);
}

void AudioClipperAudioProcessor::releaseResources()
{
    for (auto& os : oversamplers)
        os->reset();

    dryWetMixer.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioClipperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AudioClipperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any extra output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get parameter values
    const float inputGainDb   = inputGainParam->load();
    const float outputGainDb  = outputGainParam->load();
    const float thresholdDb   = thresholdParam->load();
    const float mixPercent    = mixParam->load();
    const int clipTypeIndex   = static_cast<int> (clipTypeParam->load());
    const int osIndex         = static_cast<int> (oversamplingParam->load());

    // Convert to linear values
    const float inputGain     = juce::Decibels::decibelsToGain (inputGainDb);
    const float outputGain    = juce::Decibels::decibelsToGain (outputGainDb);
    const float thresholdLin  = juce::Decibels::decibelsToGain (thresholdDb);
    const float invThreshold  = (thresholdLin > 0.0001f) ? 1.0f / thresholdLin : 1.0f;

    // Get the clipping function (avoids per-sample branching)
    ClipFunction clipFunc = clipFunctions[static_cast<size_t> (clipTypeIndex)];

    // Update dry/wet mix
    dryWetMixer.setWetMixProportion (mixPercent / 100.0f);
    dryWetMixer.setWetLatency (static_cast<float> (oversamplers[static_cast<size_t> (osIndex)]->getLatencyInSamples()));

    // Create audio block for DSP processing
    juce::dsp::AudioBlock<float> block (buffer);

    // Push dry samples for later mixing
    dryWetMixer.pushDrySamples (juce::dsp::AudioBlock<const float> (block));

    // Apply input gain
    block.multiplyBy (inputGain);

    // Upsample
    auto oversampledBlock = oversamplers[static_cast<size_t> (osIndex)]->processSamplesUp (block);

    // Process clipping on oversampled block
    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer (channel);

        for (size_t sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            // Apply threshold scaling, clip, then scale back
            float scaled = channelData[sample] * invThreshold;
            float clipped = clipFunc (scaled);
            channelData[sample] = clipped * thresholdLin;
        }
    }

    // Downsample back to original rate
    oversamplers[static_cast<size_t> (osIndex)]->processSamplesDown (block);

    // Apply output gain
    block.multiplyBy (outputGain);

    // Mix wet with dry
    dryWetMixer.mixWetSamples (block);
}

//==============================================================================
bool AudioClipperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioClipperAudioProcessor::createEditor()
{
    return new AudioClipperAudioProcessorEditor (*this);
}

//==============================================================================
void AudioClipperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioClipperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioClipperAudioProcessor();
}
