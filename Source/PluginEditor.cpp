/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioClipperAudioProcessorEditor::AudioClipperAudioProcessorEditor (AudioClipperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Clip Type ComboBox
    clipTypeLabel.setText ("Clip Type", juce::dontSendNotification);
    clipTypeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (clipTypeLabel);

    clipTypeCombo.addItemList (juce::StringArray { "Tanh", "Arctan", "Cubic", "Hard Clip" }, 1);
    addAndMakeVisible (clipTypeCombo);

    // Input Gain Slider
    inputGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    inputGainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (inputGainSlider);

    inputGainLabel.setText ("Input", juce::dontSendNotification);
    inputGainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (inputGainLabel);

    // Threshold Slider
    thresholdSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    thresholdSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    thresholdSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (thresholdSlider);

    thresholdLabel.setText ("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (thresholdLabel);

    // Output Gain Slider
    outputGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    outputGainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (outputGainSlider);

    outputGainLabel.setText ("Output", juce::dontSendNotification);
    outputGainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (outputGainLabel);

    // Mix Slider
    mixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    mixSlider.setTextValueSuffix (" %");
    addAndMakeVisible (mixSlider);

    mixLabel.setText ("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (mixLabel);

    // Oversampling ComboBox
    oversamplingLabel.setText ("Oversampling", juce::dontSendNotification);
    oversamplingLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (oversamplingLabel);

    oversamplingCombo.addItemList (juce::StringArray { "Off", "2x", "4x" }, 1);
    addAndMakeVisible (oversamplingCombo);

    // Attach controls to parameters
    inputGainAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "inputGain", inputGainSlider);
    outputGainAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "outputGain", outputGainSlider);
    thresholdAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "threshold", thresholdSlider);
    mixAttachment          = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "mix", mixSlider);
    clipTypeAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.parameters, "clipType", clipTypeCombo);
    oversamplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.parameters, "oversampling", oversamplingCombo);

    setSize (500, 380);
}

AudioClipperAudioProcessorEditor::~AudioClipperAudioProcessorEditor()
{
}

//==============================================================================
void AudioClipperAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("Audio Clipper", getLocalBounds().removeFromTop (50), juce::Justification::centred, 1);
}

void AudioClipperAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Title area (top 50px)
    bounds.removeFromTop (50);

    // Clip type row
    auto clipTypeRow = bounds.removeFromTop (40);
    clipTypeLabel.setBounds (clipTypeRow.removeFromLeft (80).reduced (5));
    clipTypeCombo.setBounds (clipTypeRow.reduced (5, 5).withWidth (150));

    // Knobs area (4 knobs in a row)
    auto knobArea = bounds.removeFromTop (200);
    int knobWidth = knobArea.getWidth() / 4;

    // Input Gain
    auto inputArea = knobArea.removeFromLeft (knobWidth);
    inputGainLabel.setBounds (inputArea.removeFromTop (25));
    inputGainSlider.setBounds (inputArea.reduced (5));

    // Threshold
    auto threshArea = knobArea.removeFromLeft (knobWidth);
    thresholdLabel.setBounds (threshArea.removeFromTop (25));
    thresholdSlider.setBounds (threshArea.reduced (5));

    // Output Gain
    auto outputArea = knobArea.removeFromLeft (knobWidth);
    outputGainLabel.setBounds (outputArea.removeFromTop (25));
    outputGainSlider.setBounds (outputArea.reduced (5));

    // Mix
    auto mixArea = knobArea;
    mixLabel.setBounds (mixArea.removeFromTop (25));
    mixSlider.setBounds (mixArea.reduced (5));

    // Oversampling row
    auto osRow = bounds.removeFromTop (40);
    oversamplingLabel.setBounds (osRow.removeFromLeft (100).reduced (5));
    oversamplingCombo.setBounds (osRow.reduced (5, 5).withWidth (100));
}
