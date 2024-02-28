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
class OneCompAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    OneCompAudioProcessorEditor (OneCompAudioProcessor&);
    ~OneCompAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // Timer callback
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    OneCompAudioProcessor& audioProcessor;

    juce::Slider thresholdSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    juce::Label thresholdLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OneCompAudioProcessorEditor)
};
