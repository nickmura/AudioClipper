/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class AudioClipperAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioClipperAudioProcessorEditor (AudioClipperAudioProcessor&);
    ~AudioClipperAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioClipperAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider thresholdSlider;
    juce::Slider mixSlider;

    // Labels
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label thresholdLabel;
    juce::Label mixLabel;
    juce::Label clipTypeLabel;
    juce::Label oversamplingLabel;

    // ComboBoxes
    juce::ComboBox clipTypeCombo;
    juce::ComboBox oversamplingCombo;

    // Slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // ComboBox attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> clipTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversamplingAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClipperAudioProcessorEditor)
};
