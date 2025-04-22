#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class GainReductionMeter : public juce::Component, private juce::Timer {
public:
    GainReductionMeter(OneCompAudioProcessor& p) : audioProcessor(p) {
        startTimerHz(30); 
        reductionLabel.setText("-∞ dB", juce::dontSendNotification); 
        reductionLabel.setFont(juce::Font(15.0f)); 
        reductionLabel.setColour(juce::Label::textColourId, juce::Colours::white); 
        reductionLabel.setJustificationType(juce::Justification::centredRight); 
        addAndMakeVisible(reductionLabel);
    }

    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat();
        float reduction = audioProcessor.getGainReduction();
        float width = juce::jmap(reduction, 0.f, -20.f, 0.f, area.getWidth());

        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::green);
        g.fillRect(meterRect);
        juce::Rectangle<float> meterRect = area.removeFromRight(width);
    }

    void timerCallback() override {
        float reductionDb = audioProcessor.getGainReduction();
        reductionLabel.setText(juce::String(reductionDb, 2) + " dB", juce::dontSendNotification);
        repaint();
    }

private:
    OneCompAudioProcessor& audioProcessor;
    juce::Label reductionLabel;
};
