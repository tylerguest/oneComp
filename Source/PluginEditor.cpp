#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GRMeter.h"

OneCompAudioProcessorEditor::OneCompAudioProcessorEditor(OneCompAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), gainReductionMeter(p),
    thresholdKnob(juce::ImageCache::getFromMemory(BinaryData::oneCompThresholdButton_png, 
        BinaryData::oneCompThresholdButton_pngSize)),
    gainKnob(juce::ImageCache::getFromMemory(BinaryData::oneCompGainButton_png,
        BinaryData::oneCompGainButton_pngSize)),
    attackKnob(juce::ImageCache::getFromMemory(BinaryData::oneCompSmallButton_png,
        BinaryData::oneCompSmallButton_pngSize)),
    ratioKnob(juce::ImageCache::getFromMemory(BinaryData::oneCompSmallButton_png,
        BinaryData::oneCompSmallButton_pngSize)),
    releaseKnob(juce::ImageCache::getFromMemory(BinaryData::oneCompSmallButton_png,
        BinaryData::oneCompSmallButton_pngSize)),
    inputKnob(juce::ImageCache::getFromMemory(BinaryData::oneCompInputButton_png,
        BinaryData::oneCompInputButton_pngSize))

{
    background = juce::ImageCache::getFromMemory(BinaryData::oneCompBG_png, BinaryData::oneCompBG_pngSize);

    addAndMakeVisible(&thresholdKnob);
    addAndMakeVisible(&gainKnob);
    addAndMakeVisible(&attackKnob);
    addAndMakeVisible(&ratioKnob);
    addAndMakeVisible(&releaseKnob);
    addAndMakeVisible(&inputKnob);

    ratioSlider.setSliderStyle(juce::Slider::LinearBar);
    ratioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    ratioSlider.setRange(1.0f, 50.0f, 2.5f);

    attackSlider.setSliderStyle(juce::Slider::LinearBar);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    attackSlider.setRange(0.5f, 500.0f, 16.0f);

    releaseSlider.setSliderStyle(juce::Slider::LinearBar);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    releaseSlider.setRange(5.0f, 5000.0f, 160.0f);

    gainSlider.setSliderStyle(juce::Slider::LinearBar);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    gainSlider.setRange(-30.0f, 30.0f, 0.0f);

    thresholdLabel.setText("Threshold: 0 dB", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);

    ratioLabel.setText("Ratio: 0 dB", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);

    attackLabel.setText("Attack: 0 dB", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);

    releaseLabel.setText("Release: 0 dB", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);

    gainLabel.setText("Gain: 0 dB", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);

    initializeKnob(thresholdKnob, "threshold");
    initializeKnob(gainKnob, "gain");
    initializeKnob(attackKnob, "attack");
    initializeKnob(ratioKnob, "ratio");
    initializeKnob(releaseKnob, "release");
    initializeKnob(inputKnob, "input");

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "threshold", thresholdKnob);
    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ratio", ratioKnob);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "attack", attackKnob);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release", releaseKnob);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainKnob);
    inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "input", inputKnob);

    inputLabel.setText("00.0", juce::dontSendNotification);
    inputLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    inputLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    inputLabel.setBounds(75, 825, 250, 80); 
    addAndMakeVisible(inputLabel);
    
    gainReductionLabel.setText("00.0", juce::dontSendNotification);
    gainReductionLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    gainReductionLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    gainReductionLabel.setBounds(315, 825, 250, 80);
    addAndMakeVisible(gainReductionLabel);

    outputLabel.setText("00.0", juce::dontSendNotification);
    outputLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    outputLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    outputLabel.setBounds(550, 825, 250, 80);
    addAndMakeVisible(outputLabel);

    menuBar = std::make_unique<juce::MenuBarComponent>(this);
    addAndMakeVisible(menuBar.get());

    setSize(background.getWidth(), background.getHeight());

    startTimer(100);
}

OneCompAudioProcessorEditor::~OneCompAudioProcessorEditor()
{   
    stopTimer();
}

void OneCompAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawImageAt(background, 0, 0);
}

void OneCompAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10); 

    int sliderHeight = 50;
    int labelHeight = 20;
    int gap = 10; 

    thresholdSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); 
    thresholdLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); 

    ratioSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); 
    ratioLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); 

    attackSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); 
    attackLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap);

    releaseSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); 
    releaseLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); 

    gainSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); 
    gainLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap);

    auto knobBounds = thresholdKnob.getImageBounds();

    int knobX = 50; 
    int knobY = 80;

    thresholdKnob.setBounds(knobX, knobY, knobBounds.getWidth(), knobBounds.getHeight());

    int gainKnobX = knobX + 400; 
    int gainKnobY = knobY; 
    gainKnob.setBounds(gainKnobX, gainKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    int attackKnobX = knobX - 50; 
    int attackKnobY = knobY + 350; 
    attackKnob.setBounds(attackKnobX, attackKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    int ratioKnobX = knobX + 200; 
    int ratioKnobY = knobY + 250; 
    ratioKnob.setBounds(ratioKnobX, ratioKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    int releaseKnobX = knobX + 450; 
    int releaseKnobY = knobY + 350; 
    releaseKnob.setBounds(releaseKnobX, releaseKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    int inputKnobX = knobX + 200; 
    int inputKnobY = knobY - 145; 
    inputKnob.setBounds(inputKnobX, inputKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    auto bounds = getLocalBounds();
    auto menuBarHeight = juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight();

    menuBar->setBounds(bounds.removeFromTop(menuBarHeight));
}

void OneCompAudioProcessorEditor::timerCallback()
{
    auto inputDb = audioProcessor.getInputLevel();
    inputLabel.setText(juce::String(inputDb, 1), juce::dontSendNotification);

    auto grDb = audioProcessor.getGainReduction();
    gainReductionLabel.setText(juce::String(grDb, 1), juce::dontSendNotification);

    auto outputDb = audioProcessor.getOutputLevel();
    outputLabel.setText(juce::String(outputDb, 1), juce::dontSendNotification);
}

void OneCompAudioProcessorEditor::initializeKnob(ImageKnob& knob, const juce::String& parameterId)
{
    addAndMakeVisible(knob);
    auto& processor = static_cast<OneCompAudioProcessor&>(audioProcessor);
    auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, parameterId, knob);
}

juce::StringArray OneCompAudioProcessorEditor::getMenuBarNames()
{
    return { "File", "Edit", "Help" };
}

juce::PopupMenu OneCompAudioProcessorEditor::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;
    if (menuName == "File")
    {
        menu.addItem(1, "Open");
        menu.addItem(2, "Save");
        menu.addSeparator();
        menu.addItem(3, "Exit");
    }
    else if (menuName == "Edit")
    {
        menu.addItem(4, "Undo");
        menu.addItem(5, "Redo");
    }
    else if (menuName == "Help")
    {
        menu.addItem(6, "About");
    }
    return menu;
}

void OneCompAudioProcessorEditor::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    switch (menuItemID)
    {
    case 1: /* Open */ break;
    case 2: /* Save */ break;
    case 3: /* Exit */ break;
    }
}