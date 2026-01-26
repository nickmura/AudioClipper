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
class AudioClipperAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    AudioClipperAudioProcessorEditor (AudioClipperAudioProcessor&);
    ~AudioClipperAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    // Nested component: Level Meter
    class LevelMeter : public juce::Component
    {
    public:
        void setLevel (float newLevel);
        void paint (juce::Graphics& g) override;

    private:
        float displayLevel = 0.0f;
    };

    //==============================================================================
    // Nested component: Transfer Curve Display
    class TransferCurveDisplay : public juce::Component
    {
    public:
        void updateCurve (int clipTypeIndex);
        void paint (juce::Graphics& g) override;

    private:
        std::array<float, 128> curveData {};
        int currentClipType = -1;
    };

private:
    void timerCallback() override;
    AudioClipperAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider thresholdSlider;
    juce::Slider mixSlider;
    juce::Slider ceilingSlider;

    // Labels
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label thresholdLabel;
    juce::Label mixLabel;
    juce::Label clipTypeLabel;
    juce::Label oversamplingLabel;
    juce::Label ceilingLabel;

    // ComboBoxes
    juce::ComboBox clipTypeCombo;
    juce::ComboBox oversamplingCombo;

    // Toggle buttons
    juce::ToggleButton stereoLinkButton { "Stereo Link" };
    juce::ToggleButton midSideModeButton { "Mid-Side" };
    juce::ToggleButton autoGainButton { "Auto Gain" };

    // Slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;

    // ComboBox attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> clipTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversamplingAttachment;

    // Button attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoLinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> midSideModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoGainAttachment;

    // Visualization components
    LevelMeter inputMeterL, inputMeterR;
    LevelMeter outputMeterL, outputMeterR;
    TransferCurveDisplay transferCurveDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClipperAudioProcessorEditor)
};
