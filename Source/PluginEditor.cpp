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
    // Input Gain Slider
    inputGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (inputGainSlider);

    inputGainLabel.setText ("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (inputGainLabel);

    // Output Gain Slider
    outputGainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (outputGainSlider);

    outputGainLabel.setText ("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (outputGainLabel);

    // Attach sliders to parameters
    inputGainAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "inputGain", inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "outputGain", outputGainSlider);

    setSize (400, 300);
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
    g.drawFittedText ("Audio Clipper", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);

    g.setFont (14.0f);
    g.drawFittedText ("Soft Clipper (tanh)", getLocalBounds().removeFromTop(80).removeFromBottom(20), juce::Justification::centred, 1);
}

void AudioClipperAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop (80); // Space for title

    auto sliderArea = bounds.reduced (20);
    auto sliderWidth = sliderArea.getWidth() / 2;

    // Input Gain
    auto inputArea = sliderArea.removeFromLeft (sliderWidth);
    inputGainLabel.setBounds (inputArea.removeFromTop (30));
    inputGainSlider.setBounds (inputArea);

    // Output Gain
    auto outputArea = sliderArea;
    outputGainLabel.setBounds (outputArea.removeFromTop (30));
    outputGainSlider.setBounds (outputArea);
}
