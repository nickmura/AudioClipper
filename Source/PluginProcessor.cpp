/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AudioClipperAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("inputGain", 1),
        "Input Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f),
        0.0f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("outputGain", 1),
        "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f),
        0.0f));

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
    inputGainParam  = parameters.getRawParameterValue ("inputGain");
    outputGainParam = parameters.getRawParameterValue ("outputGain");

    jassert (inputGainParam != nullptr);
    jassert (outputGainParam != nullptr);
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void AudioClipperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // TEMPORARY BYPASS TEST - just pass audio straight through
    // If you hear audio now, the problem is in the processing code
    return;

    // Get parameter values and convert from dB to linear gain
    float inputGainDb  = (inputGainParam != nullptr) ? inputGainParam->load() : 0.0f;
    float outputGainDb = (outputGainParam != nullptr) ? outputGainParam->load() : 0.0f;
    float inputGain    = juce::Decibels::decibelsToGain (inputGainDb);
    float outputGain   = juce::Decibels::decibelsToGain (outputGainDb);

    // Process each channel
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // Process each sample
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Apply input gain
            float input = channelData[sample] * inputGain;

            // Apply tanh soft clipping
            float clipped = std::tanh (input);

            // Apply output gain
            channelData[sample] = clipped * outputGain;
        }
    }
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
