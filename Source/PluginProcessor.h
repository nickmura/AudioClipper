/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
*/
class AudioClipperAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioClipperAudioProcessor();
    ~AudioClipperAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    // Clipping function type
    using ClipFunction = float (*)(float);

    static float clipTanh (float x);
    static float clipArctan (float x);
    static float clipCubic (float x);
    static float clipHard (float x);

    static constexpr std::array<ClipFunction, 4> clipFunctions = {
        clipTanh, clipArctan, clipCubic, clipHard
    };

    // Parameter pointers
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    std::atomic<float>* clipTypeParam = nullptr;
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* oversamplingParam = nullptr;
    std::atomic<float>* stereoLinkParam = nullptr;
    std::atomic<float>* midSideModeParam = nullptr;
    std::atomic<float>* autoGainParam = nullptr;
    std::atomic<float>* ceilingParam = nullptr;

    // DSP components
    std::array<std::unique_ptr<juce::dsp::Oversampling<float>>, 3> oversamplers;
    juce::dsp::DryWetMixer<float> dryWetMixer { 512 };

    // DC blocking filters (one per channel)
    std::array<juce::dsp::IIR::Filter<float>, 2> dcBlockFilters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClipperAudioProcessor)
};
