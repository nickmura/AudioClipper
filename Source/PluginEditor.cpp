/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// LevelMeter implementation
void AudioClipperAudioProcessorEditor::LevelMeter::setLevel (float newLevel)
{
    // Smooth decay: keep the higher of new level or 90% of current display
    displayLevel = std::max (newLevel, displayLevel * 0.9f);
}

void AudioClipperAudioProcessorEditor::LevelMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (juce::Colours::black);
    g.fillRect (bounds);

    // Border
    g.setColour (juce::Colours::grey);
    g.drawRect (bounds, 1.0f);

    // Calculate meter height
    float meterLevel = juce::jlimit (0.0f, 1.0f, displayLevel);
    float meterHeight = bounds.getHeight() * meterLevel;

    // Draw meter from bottom up with gradient coloring
    auto meterRect = bounds.removeFromBottom (meterHeight).reduced (2.0f, 0.0f);

    if (meterHeight > 0)
    {
        // Color based on level: green -> yellow -> red
        juce::Colour meterColour;
        if (displayLevel < 0.6f)
            meterColour = juce::Colours::limegreen;
        else if (displayLevel < 0.9f)
            meterColour = juce::Colours::yellow;
        else
            meterColour = juce::Colours::red;

        g.setColour (meterColour);
        g.fillRect (meterRect);
    }
}

//==============================================================================
// TransferCurveDisplay implementation
void AudioClipperAudioProcessorEditor::TransferCurveDisplay::updateCurve (int clipTypeIndex)
{
    if (clipTypeIndex == currentClipType)
        return;

    currentClipType = clipTypeIndex;

    // Pre-compute the transfer curve
    // Input range: -3 to +3 (allowing visualization of saturation beyond unity)
    for (size_t i = 0; i < curveData.size(); ++i)
    {
        float input = (static_cast<float> (i) / (curveData.size() - 1)) * 6.0f - 3.0f; // -3 to +3

        float output;
        switch (clipTypeIndex)
        {
            case 0: // Tanh
                output = std::tanh (input);
                break;
            case 1: // Arctan
                output = (2.0f / juce::MathConstants<float>::pi) * std::atan (input);
                break;
            case 2: // Cubic
            {
                float x = juce::jlimit (-1.5f, 1.5f, input);
                output = x - (x * x * x) / 3.0f;
                break;
            }
            case 3: // Hard Clip
            default:
                output = juce::jlimit (-1.0f, 1.0f, input);
                break;
        }

        curveData[i] = output;
    }

    repaint();
}

void AudioClipperAudioProcessorEditor::TransferCurveDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (juce::Colour (0xff1a1a1a));
    g.fillRect (bounds);

    // Border
    g.setColour (juce::Colours::grey);
    g.drawRect (bounds, 1.0f);

    auto plotBounds = bounds.reduced (4.0f);

    // Draw grid lines
    g.setColour (juce::Colours::darkgrey.withAlpha (0.5f));

    // Vertical center line (x = 0)
    float centerX = plotBounds.getCentreX();
    g.drawLine (centerX, plotBounds.getY(), centerX, plotBounds.getBottom(), 0.5f);

    // Horizontal center line (y = 0)
    float centerY = plotBounds.getCentreY();
    g.drawLine (plotBounds.getX(), centerY, plotBounds.getRight(), centerY, 0.5f);

    // Draw linear reference line (diagonal, y = x clamped to output range)
    g.setColour (juce::Colours::grey.withAlpha (0.4f));
    juce::Path linearPath;
    bool linearStarted = false;

    for (size_t i = 0; i < curveData.size(); ++i)
    {
        float normX = static_cast<float> (i) / (curveData.size() - 1);
        float input = normX * 6.0f - 3.0f; // -3 to +3
        float linearY = juce::jlimit (-1.0f, 1.0f, input);

        float x = plotBounds.getX() + normX * plotBounds.getWidth();
        float y = plotBounds.getCentreY() - (linearY * plotBounds.getHeight() * 0.5f);

        if (!linearStarted)
        {
            linearPath.startNewSubPath (x, y);
            linearStarted = true;
        }
        else
        {
            linearPath.lineTo (x, y);
        }
    }
    g.strokePath (linearPath, juce::PathStrokeType (1.0f));

    // Draw the transfer curve
    g.setColour (juce::Colours::cyan);
    juce::Path curvePath;
    bool curveStarted = false;

    for (size_t i = 0; i < curveData.size(); ++i)
    {
        float normX = static_cast<float> (i) / (curveData.size() - 1);
        float x = plotBounds.getX() + normX * plotBounds.getWidth();
        // Output range is -1 to +1, map to plot coordinates (inverted y)
        float y = plotBounds.getCentreY() - (curveData[i] * plotBounds.getHeight() * 0.5f);

        if (!curveStarted)
        {
            curvePath.startNewSubPath (x, y);
            curveStarted = true;
        }
        else
        {
            curvePath.lineTo (x, y);
        }
    }
    g.strokePath (curvePath, juce::PathStrokeType (2.0f));
}

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

    // Ceiling Slider
    ceilingSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    ceilingSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    ceilingSlider.setTextValueSuffix (" dB");
    addAndMakeVisible (ceilingSlider);

    ceilingLabel.setText ("Ceiling", juce::dontSendNotification);
    ceilingLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (ceilingLabel);

    // Oversampling ComboBox
    oversamplingLabel.setText ("Oversampling", juce::dontSendNotification);
    oversamplingLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (oversamplingLabel);

    oversamplingCombo.addItemList (juce::StringArray { "Off", "2x", "4x" }, 1);
    addAndMakeVisible (oversamplingCombo);

    // Toggle buttons
    addAndMakeVisible (stereoLinkButton);
    addAndMakeVisible (midSideModeButton);
    addAndMakeVisible (autoGainButton);

    // Attach controls to parameters
    inputGainAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "inputGain", inputGainSlider);
    outputGainAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "outputGain", outputGainSlider);
    thresholdAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "threshold", thresholdSlider);
    mixAttachment          = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "mix", mixSlider);
    ceilingAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.parameters, "ceiling", ceilingSlider);
    clipTypeAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.parameters, "clipType", clipTypeCombo);
    oversamplingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.parameters, "oversampling", oversamplingCombo);
    stereoLinkAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.parameters, "stereoLink", stereoLinkButton);
    midSideModeAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.parameters, "midSideMode", midSideModeButton);
    autoGainAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.parameters, "autoGain", autoGainButton);

    // Visualization components
    addAndMakeVisible (inputMeterL);
    addAndMakeVisible (inputMeterR);
    addAndMakeVisible (outputMeterL);
    addAndMakeVisible (outputMeterR);
    addAndMakeVisible (transferCurveDisplay);

    // Initialize transfer curve with current clip type
    int initialClipType = static_cast<int> (audioProcessor.parameters.getRawParameterValue ("clipType")->load());
    transferCurveDisplay.updateCurve (initialClipType);

    // Start timer for meter updates (~60 Hz)
    startTimerHz (60);

    setSize (500, 520);
}

AudioClipperAudioProcessorEditor::~AudioClipperAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void AudioClipperAudioProcessorEditor::timerCallback()
{
    // Update level meters with values from processor
    inputMeterL.setLevel (audioProcessor.getInputPeakL());
    inputMeterR.setLevel (audioProcessor.getInputPeakR());
    outputMeterL.setLevel (audioProcessor.getOutputPeakL());
    outputMeterR.setLevel (audioProcessor.getOutputPeakR());

    // Repaint meters
    inputMeterL.repaint();
    inputMeterR.repaint();
    outputMeterL.repaint();
    outputMeterR.repaint();

    // Update transfer curve if clip type changed
    int currentClipType = static_cast<int> (audioProcessor.parameters.getRawParameterValue ("clipType")->load());
    transferCurveDisplay.updateCurve (currentClipType);
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

    // Knobs area (5 knobs in a row)
    auto knobArea = bounds.removeFromTop (180);
    int knobWidth = knobArea.getWidth() / 5;

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

    // Ceiling
    auto ceilingArea = knobArea.removeFromLeft (knobWidth);
    ceilingLabel.setBounds (ceilingArea.removeFromTop (25));
    ceilingSlider.setBounds (ceilingArea.reduced (5));

    // Mix
    auto mixArea = knobArea;
    mixLabel.setBounds (mixArea.removeFromTop (25));
    mixSlider.setBounds (mixArea.reduced (5));

    // Toggle buttons row
    auto toggleRow = bounds.removeFromTop (35);
    int toggleWidth = toggleRow.getWidth() / 3;
    stereoLinkButton.setBounds (toggleRow.removeFromLeft (toggleWidth).reduced (10, 5));
    midSideModeButton.setBounds (toggleRow.removeFromLeft (toggleWidth).reduced (10, 5));
    autoGainButton.setBounds (toggleRow.reduced (10, 5));

    // Oversampling row
    auto osRow = bounds.removeFromTop (40);
    oversamplingLabel.setBounds (osRow.removeFromLeft (100).reduced (5));
    oversamplingCombo.setBounds (osRow.reduced (5, 5).withWidth (100));

    // Some spacing before visualization
    bounds.removeFromTop (5);

    // Visualization area (100px high)
    auto vizArea = bounds.removeFromTop (100);
    int meterWidth = 15;
    int meterSpacing = 3;

    // Input meters (left side)
    auto inputMeterArea = vizArea.removeFromLeft (meterWidth * 2 + meterSpacing + 10);
    inputMeterArea.removeFromLeft (5); // left padding
    inputMeterL.setBounds (inputMeterArea.removeFromLeft (meterWidth).reduced (0, 5));
    inputMeterArea.removeFromLeft (meterSpacing);
    inputMeterR.setBounds (inputMeterArea.removeFromLeft (meterWidth).reduced (0, 5));

    // Output meters (right side)
    auto outputMeterArea = vizArea.removeFromRight (meterWidth * 2 + meterSpacing + 10);
    outputMeterArea.removeFromRight (5); // right padding
    outputMeterR.setBounds (outputMeterArea.removeFromRight (meterWidth).reduced (0, 5));
    outputMeterArea.removeFromRight (meterSpacing);
    outputMeterL.setBounds (outputMeterArea.removeFromRight (meterWidth).reduced (0, 5));

    // Transfer curve display (center)
    transferCurveDisplay.setBounds (vizArea.reduced (10, 5));
}
