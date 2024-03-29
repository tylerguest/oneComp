/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class OneCompAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    OneCompAudioProcessor();
    ~OneCompAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Declare your parameter objects
    juce::AudioParameterFloat* threshold;
    juce::AudioParameterFloat* ratio;
    juce::AudioParameterFloat* attack;
    juce::AudioParameterFloat* release;
    juce::AudioParameterFloat* gain;

    juce::AudioProcessorValueTreeState parameters;

   
    float getGainReduction() const;
    float getInputLevel() const; // Getter for input level
    float getOutputLevel() const; // Getter for output level

private:
    juce::dsp::Compressor<float> compressor;

    std::atomic<float> lastGainReduction{ 0.0f };
    std::atomic<float> lastInputLevel{ 0.0f }; // Stores the last input level
    std::atomic<float> lastOutputLevel{ 0.0f }; // Stores the last output level
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneCompAudioProcessor)
};
