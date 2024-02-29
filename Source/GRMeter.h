/*
  ==============================================================================

    GRMeter.h
    Created: 28 Feb 2024 8:40:57pm
    Author:  oktylerguest

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class GainReductionMeter : public juce::Component, private juce::Timer {
public:
    GainReductionMeter(OneCompAudioProcessor& p) : audioProcessor(p) {
        startTimerHz(30); // Update 30 times per second
        reductionLabel.setText("-∞ dB", juce::dontSendNotification); // Initial text
        reductionLabel.setFont(juce::Font(15.0f)); // Set the font size
        reductionLabel.setColour(juce::Label::textColourId, juce::Colours::white); // Set the text color
        reductionLabel.setJustificationType(juce::Justification::centredRight); // Right-aligned text
        addAndMakeVisible(reductionLabel);


    }

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat();
        float reduction = audioProcessor.getGainReduction();

        // Simple linear conversion, adjust according to your meter's design
        float width = juce::jmap(reduction, 0.f, -20.f, 0.f, area.getWidth());

        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::green);

        // Calculate the rectangle for the gain reduction meter
        // The rectangle starts from the right edge minus the calculated width
        juce::Rectangle<float> meterRect = area.removeFromRight(width);

        g.fillRect(meterRect);
    }

    void timerCallback() override {
        // Fetch the dynamic gain reduction value from the processor
        float reductionDb = audioProcessor.getGainReduction();

        // Update the label with the dynamic value
        reductionLabel.setText(juce::String(reductionDb, 2) + " dB", juce::dontSendNotification);

        // Request a repaint to update the visual representation
        repaint();

    }

private:
    OneCompAudioProcessor& audioProcessor;
    juce::Label reductionLabel; // Label to display gain reduction in dB
};
