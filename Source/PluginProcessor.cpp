/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GRMeter.h"

using namespace juce;

//==============================================================================
OneCompAudioProcessor::OneCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr)
#endif
{
    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "threshold", // parameter ID
        "Threshold", // parameter name
        NormalisableRange<float>(-60.0f, 0.0f, 1.0f, 1.0f), // min, max, interval, skewFactor
        10.0f // default value
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "ratio",
        "Ratio",
        NormalisableRange<float>(1.0f, 25.0f, 0.1f, 0.7f), // min, max, interval, skewFactor
        21.5f // default value
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "attack",
        "Attack",
        NormalisableRange<float>(0.1f, 150.0f, 0.05f, 0.5f), // min, max, interval, skewFactor
        2.0f // default value
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "release",
        "Release",
        NormalisableRange<float>(1.5f, 2000.0f, 0.1f, 0.5f), // min, max, interval, skewFactor
        120.0f // default value
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        NormalisableRange<float>(-30.0f, 30.0f), // min, max, interval, skewFactor
        0.0f // default value
    ));

    parameters.state = juce::ValueTree("savedParams");

    
}

OneCompAudioProcessor::~OneCompAudioProcessor()
{
}

//==============================================================================
const juce::String OneCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OneCompAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool OneCompAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool OneCompAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double OneCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OneCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int OneCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OneCompAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String OneCompAudioProcessor::getProgramName(int index)
{
    return {};
}

void OneCompAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void OneCompAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    compressor.prepare(spec);
    //compressor.setThreshold(-20.f);
    //compressor.setRatio(2.f);
    //compressor.setAttack(10.f);
    //compressor.setRelease(100.f);

}

void OneCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OneCompAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void OneCompAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Assuming mono processing for simplicity. You might need to adjust for stereo or multi-channel.
    auto inputLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples()); // Measure input RMS level
    float inputLevelDb = inputLevel > 0.0f ? juce::Decibels::gainToDecibels(inputLevel) : -100.0f;
    lastInputLevel.store(inputLevelDb, std::memory_order_relaxed);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Retrieve parameter values
    float threshold = *parameters.getRawParameterValue("threshold");
    float ratio = *parameters.getRawParameterValue("ratio");
    float attackTime = *parameters.getRawParameterValue("attack");
    float releaseTime = *parameters.getRawParameterValue("release");
    float makeupGain = *parameters.getRawParameterValue("gain");

    // Update the compressor settings from parameters
    compressor.setThreshold(*parameters.getRawParameterValue("threshold"));
    compressor.setRatio(*parameters.getRawParameterValue("ratio"));
    compressor.setAttack(*parameters.getRawParameterValue("attack"));
    compressor.setRelease(*parameters.getRawParameterValue("release"));

    // Prepare the DSP context
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process the block with the updated compressor settings
    compressor.process(context);

    // Measure output RMS level after compression
    auto outputLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    float outputLevelDb = outputLevel > 0.0f ? juce::Decibels::gainToDecibels(outputLevel) : -100.0f;

    // Calculate gain reduction in dB. If outputLevel is 0, avoid division by zero.
    float gainReductionDb = outputLevel > 0 ? juce::Decibels::gainToDecibels(inputLevel / outputLevel) : 0.f;
    // Since we're interested in reduction, we take the negative of the calculated value.
    // This assumes inputLevel >= outputLevel. Adjust the logic if your scenario could differ.
    lastGainReduction.store(-gainReductionDb, std::memory_order_release);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Placeholder for actual compression logic
            // For demonstration, let's just apply makeup gain
            float inputSample = channelData[sample];
            float inputGain = juce::Decibels::decibelsToGain(makeupGain);
            channelData[sample] = inputSample * inputGain;

            // TODO: Implement actual compression logic here
            // This would involve checking the level of each sample against the threshold,
            // applying gain reduction based on the ratio, and smoothing the gain changes
            // with the attack and release parameters.
        }

        // ..do something to the data...
        auto outputLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        float outputLevelDb = outputLevel > 0.0f ? juce::Decibels::gainToDecibels(outputLevel) : -100.0f;

        lastOutputLevel.store(outputLevelDb, std::memory_order_relaxed);

     
    }
}

//==============================================================================
bool OneCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OneCompAudioProcessor::createEditor()
{
    return new OneCompAudioProcessorEditor(*this);
}

//==============================================================================
void OneCompAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // Convert the current state of your parameters into a memory block
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OneCompAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    // Create an XML element from the binary data
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            // Restore the state from the XML element
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OneCompAudioProcessor();
}

float OneCompAudioProcessor::getGainReduction() const {
    return lastGainReduction.load(std::memory_order_relaxed); // or std::memory_order_acquire
}

float OneCompAudioProcessor::getInputLevel() const {
    return lastInputLevel.load(std::memory_order_relaxed);
}

float OneCompAudioProcessor::getOutputLevel() const {
    return lastOutputLevel.load(std::memory_order_relaxed);
}