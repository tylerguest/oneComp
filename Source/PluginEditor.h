/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GRMeter.h"

//==============================================================================
/**
*/
class OneCompAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    OneCompAudioProcessorEditor(OneCompAudioProcessor&);
    ~OneCompAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    // Timer callback
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    OneCompAudioProcessor& audioProcessor;
    GainReductionMeter gainReductionMeter; // Add this line
    juce::Label gainReductionLabel;

    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider gainSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneCompAudioProcessorEditor)
};
