/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GRMeter.h"

//==============================================================================
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
        BinaryData::oneCompSmallButton_pngSize))

{
    background = juce::ImageCache::getFromMemory(BinaryData::oneCompBG_png, BinaryData::oneCompBG_pngSize);



    // Initialize slider here
    //thresholdSlider.setSliderStyle(juce::Slider::Rotary);
    //thresholdSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    //thresholdSlider.setRange(-60.0f, 0.0f, 1.0f);
    //addAndMakeVisible(&thresholdSlider);

    addAndMakeVisible(&thresholdKnob);
    addAndMakeVisible(&gainKnob);
    addAndMakeVisible(&attackKnob);
    addAndMakeVisible(&ratioKnob);
    addAndMakeVisible(&releaseKnob);

    // Correctly cast audioProcessor to OneCompAudioProcessor& to access custom members like `parameters`
    //auto& processor = static_cast<OneCompAudioProcessor&>(audioProcessor);

    ratioSlider.setSliderStyle(juce::Slider::LinearBar);
    ratioSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    ratioSlider.setRange(1.0f, 50.0f, 2.5f);
    //addAndMakeVisible(&ratioSlider);

    attackSlider.setSliderStyle(juce::Slider::LinearBar);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    attackSlider.setRange(0.5f, 500.0f, 16.0f);
    //addAndMakeVisible(&attackSlider);

    releaseSlider.setSliderStyle(juce::Slider::LinearBar);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    releaseSlider.setRange(5.0f, 5000.0f, 160.0f);
    //addAndMakeVisible(&releaseSlider);

    gainSlider.setSliderStyle(juce::Slider::LinearBar);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    gainSlider.setRange(-30.0f, 30.0f, 0.0f);
    //addAndMakeVisible(&gainSlider);


    // Initialize slider label here
    thresholdLabel.setText("Threshold: 0 dB", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(thresholdLabel);

    ratioLabel.setText("Ratio: 0 dB", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(ratioLabel);

    attackLabel.setText("Attack: 0 dB", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(attackLabel);

    releaseLabel.setText("Release: 0 dB", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(releaseLabel);

    gainLabel.setText("Gain: 0 dB", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(gainLabel);

    // Gain reduction meter
    // addAndMakeVisible(gainReductionMeter);

    // Now create the SliderAttachment
    // thresholdAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "threshold", thresholdSlider));
    // ratioAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "ratio", ratioSlider));
    // attackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "attack", attackSlider));
    // releaseAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "release", releaseSlider));
    // gainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, "gain", gainSlider));

    // Initialize SliderAttachment objects
    // Initialize the knob and create its attachment
    initializeKnob(thresholdKnob, "threshold");
    initializeKnob(gainKnob, "gain");
    initializeKnob(attackKnob, "attack");
    initializeKnob(ratioKnob, "ratio");
    initializeKnob(releaseKnob, "release");

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "threshold", thresholdKnob);
    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ratio", ratioKnob);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "attack", attackKnob);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release", releaseKnob);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainKnob);

    // Position the input, GR, and output labels
    inputLabel.setText("00.0", juce::dontSendNotification);
    inputLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    inputLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    inputLabel.setBounds(75, 825, 250, 80); // These bounds are arbitrary; set them according to your GUI design
    addAndMakeVisible(inputLabel);

    
    gainReductionLabel.setText("00.0", juce::dontSendNotification);
    gainReductionLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    gainReductionLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    gainReductionLabel.setBounds(315, 825, 250, 80);
    addAndMakeVisible(gainReductionLabel);

    outputLabel.setText("00.0", juce::dontSendNotification);
    outputLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    outputLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    outputLabel.setBounds(550, 825, 250, 80); // (int x, int y, int w, int h)
    addAndMakeVisible(outputLabel);

    // Initialize the menu bar and add it to the component
    menuBar = std::make_unique<juce::MenuBarComponent>(this);
    addAndMakeVisible(menuBar.get());

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    gainReductionMeter.repaint();
    setSize(background.getWidth(), background.getHeight());

    startTimer(100);
}

OneCompAudioProcessorEditor::~OneCompAudioProcessorEditor()
{
    
    stopTimer();
}

//==============================================================================
void OneCompAudioProcessorEditor::paint(juce::Graphics& g)
{
    

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    g.drawImageAt(background, 0, 0);
}

void OneCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    // Define the starting area for your components
    auto area = getLocalBounds().reduced(10); // Apply some padding around the edges

    // Define the dimensions for your sliders and labels
    int sliderHeight = 50;
    int labelHeight = 20;
    int gap = 10; // Gap between the slider and its label, and between successive sets

    // Set bounds for the threshold slider and its label
    thresholdSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); // Add a gap between the slider and the label
    thresholdLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); // Add a gap between the label and the next slider

    // Set bounds for the ratio slider and its label
    ratioSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); // Add a gap between the slider and the label
    ratioLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); // Add a gap between the label and the next slider

    // Set bounds for the attack slider and its label
    attackSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); // Add a gap between the slider and the label
    attackLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); // Add a gap between the label and the next slider

    // Set bounds for the release slider and its label
    releaseSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); // Add a gap between the slider and the label
    releaseLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); // Add a gap between the label and the next slider

    // Set bounds for the gain slider and its label
    gainSlider.setBounds(area.removeFromTop(sliderHeight));
    area.removeFromTop(gap); // Add a gap between the slider and the label
    gainLabel.setBounds(area.removeFromTop(labelHeight));
    area.removeFromTop(gap); // Add a gap between the label and the next slider
    // If you add more components, continue adjusting 'area' as needed

    // Retrieve the image bounds from the ImageKnob instance
    auto knobBounds = thresholdKnob.getImageBounds();

    // Custom position
    int knobX = 50; // Change this to your desired X coordinate
    int knobY = 80; // Change this to your desired Y coordinate
    // Set the knob bounds
    thresholdKnob.setBounds(knobX, knobY, knobBounds.getWidth(), knobBounds.getHeight());

    // Inside OneCompAudioProcessorEditor::resized()
    int gainKnobX = knobX + 400; // Example: Place it 200 pixels to the right of the threshold knob
    int gainKnobY = knobY; // Keep it on the same vertical level
    gainKnob.setBounds(gainKnobX, gainKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    // Inside OneCompAudioProcessorEditor::resized()
    int attackKnobX = knobX - 50; // Example: Place it 200 pixels to the right of the threshold knob
    int attackKnobY = knobY + 350; // Keep it on the same vertical level
    attackKnob.setBounds(attackKnobX, attackKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    // Inside OneCompAudioProcessorEditor::resized()
    int ratioKnobX = knobX + 200; // Example: Place it 200 pixels to the right of the threshold knob
    int ratioKnobY = knobY + 250; // Keep it on the same vertical level
    ratioKnob.setBounds(ratioKnobX, ratioKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    // Inside OneCompAudioProcessorEditor::resized()
    int releaseKnobX = knobX + 450; // Example: Place it 200 pixels to the right of the threshold knob
    int releaseKnobY = knobY + 350; // Keep it on the same vertical level
    releaseKnob.setBounds(releaseKnobX, releaseKnobY, knobBounds.getWidth(), knobBounds.getHeight());

    auto bounds = getLocalBounds();
    auto menuBarHeight = juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight();

    // Assign space for the menu bar at the top
    menuBar->setBounds(bounds.removeFromTop(menuBarHeight));
}

void OneCompAudioProcessorEditor::timerCallback()
{
    auto thresholdValue = audioProcessor.parameters.getRawParameterValue("threshold")->load();
    thresholdLabel.setText("Threshold: " + juce::String(thresholdValue, 2), juce::dontSendNotification);

    auto ratioValue = audioProcessor.parameters.getRawParameterValue("ratio")->load();
    ratioLabel.setText("Ratio: " + juce::String(ratioValue, 2), juce::dontSendNotification);

    auto attackValue = audioProcessor.parameters.getRawParameterValue("attack")->load();
    attackLabel.setText("Attack: " + juce::String(attackValue, 2), juce::dontSendNotification);

    auto releaseValue = audioProcessor.parameters.getRawParameterValue("release")->load();
    releaseLabel.setText("Release: " + juce::String(releaseValue, 2), juce::dontSendNotification);

    auto gainValue = audioProcessor.parameters.getRawParameterValue("gain")->load();
    gainLabel.setText("Gain: " + juce::String(gainValue, 2), juce::dontSendNotification);

    auto formatDbValueForDisplay = [](float dbValue) -> juce::String {
        // Adjust values to represent analog 0 dB as -18 dBFS
        dbValue -= 18.0f;

        if (dbValue <= -100.0f || dbValue == 00.0f) { // Adjust the threshold as per your requirement
            return "-00.0"; // Using a Unicode character for the minus sign
        }
        else {
            juce::String formattedString = juce::String(dbValue, 1);
            // Ensure leading zero for single-digit negative values
            if (dbValue > -10.0f && dbValue < 0.0f) {
                formattedString = "-0" + juce::String(std::abs(dbValue), 1);
            }
            // Ensure three characters for positive single-digit values, including leading zeros
            else if (dbValue >= 0.0f && dbValue < 10.0f) {
                formattedString = "0" + juce::String(dbValue, 1);
            }
            return formattedString;
        }
    };



    // Input level
    auto inputDb = audioProcessor.getInputLevel();
    inputLabel.setText(formatDbValueForDisplay(inputDb), juce::dontSendNotification);

    // Gain reduction
    auto grDb = audioProcessor.getGainReduction();
    gainReductionLabel.setText(formatDbValueForDisplay(grDb + 18.0f), juce::dontSendNotification);

    // Output level
    auto outputDb = audioProcessor.getOutputLevel();
    outputLabel.setText(formatDbValueForDisplay(outputDb), juce::dontSendNotification);

}

void OneCompAudioProcessorEditor::initializeKnob(ImageKnob& knob, const juce::String& parameterId)
{
    addAndMakeVisible(knob);
    auto& processor = static_cast<OneCompAudioProcessor&>(audioProcessor);
    auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.parameters, parameterId, knob);
    // Depending on your design, you might keep these attachments in a vector if they need to be accessed later
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
    // Handle menu item selection
    switch (menuItemID)
    {
    case 1: /* Open */ break;
    case 2: /* Save */ break;
    case 3: /* Exit */ break;
        // Handle other cases
    }
}