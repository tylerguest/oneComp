
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GRMeter.h"

class ImageKnob : public juce::Slider {
public:
    ImageKnob(const juce::Image& knobImage) : image(knobImage) {
        setSliderStyle(SliderStyle::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(NoTextBox, false, 0, 0);
        setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
            juce::MathConstants<float>::pi * 2.75f, true);
    }

    juce::Rectangle<int> getImageBounds() const {
        return juce::Rectangle<int>(image.getWidth(), image.getHeight());
    }

    void ImageKnob::paint(juce::Graphics& g) {
        auto bounds = getLocalBounds().toFloat();
        auto rotationCentre = bounds.getCentre();
        auto angle = getRotaryAngle();

        juce::AffineTransform transform = juce::AffineTransform::translation(-image.getWidth() / 2.0f, -image.getHeight() / 2.0f) 
            .rotated(angle) 
            .translated(rotationCentre.getX(), rotationCentre.getY()); 

        g.drawImageTransformed(image, transform, false);
    }

private:
    juce::Image image;

    float getRotaryAngle() const {
        return juce::jmap<float>(getValue(), getMinimum(), getMaximum(),
            juce::MathConstants<float>::pi * 1.25f,
            juce::MathConstants<float>::pi * 2.75f);
    }
};

class OneCompAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::MenuBarModel
{


public:
    OneCompAudioProcessorEditor(OneCompAudioProcessor&);
    ~OneCompAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void initializeKnob(ImageKnob& knob, const juce::String& parameterId);

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

private:
    OneCompAudioProcessor& audioProcessor;
    GainReductionMeter gainReductionMeter; 

    juce::Image background;
    ImageKnob thresholdKnob;
    ImageKnob gainKnob;
    ImageKnob attackKnob;
    ImageKnob ratioKnob;
    ImageKnob releaseKnob;
    ImageKnob inputKnob;
   
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment;

    std::unique_ptr<juce::MenuBarComponent> menuBar;

    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label gainLabel;

    juce::Label inputLabel;
    juce::Label gainReductionLabel;
    juce::Label outputLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneCompAudioProcessorEditor)
};
