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



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClipperAudioProcessorEditor)
};
