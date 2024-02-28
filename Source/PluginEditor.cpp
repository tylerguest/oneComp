/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OneCompAudioProcessorEditor::OneCompAudioProcessorEditor (OneCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Initialize your slider here
    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    thresholdSlider.setRange(-60.0f, 0.0f, 1.0f);
    addAndMakeVisible(&thresholdSlider);

    thresholdLabel.setText("Threshold: 0 dB", juce::dontSendNotification);
    // thresholdLabel.attachToComponent(&thresholdSlider, false); // set to true to place the label above slider
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    // Now create the SliderAttachment
    thresholdAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "threshold", thresholdSlider));

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    startTimer(100);

}

OneCompAudioProcessorEditor::~OneCompAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void OneCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
   // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);


}

void OneCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    thresholdSlider.setBounds(0, 0, getWidth(), 50);

    // Example bounds setting
    auto area = getLocalBounds();
    auto sliderHeight = 50;
    auto labelHeight = 20;

    thresholdSlider.setBounds(area.removeFromTop(sliderHeight));
    thresholdLabel.setBounds(area.removeFromTop(labelHeight)); // Position the label just below the slider
}

void OneCompAudioProcessorEditor::timerCallback()
{
    auto thresholdValue = audioProcessor.parameters.getRawParameterValue("threshold")->load();
    thresholdLabel.setText("Threshold: " + juce::String(thresholdValue, 2) + " dB", juce::dontSendNotification);
}