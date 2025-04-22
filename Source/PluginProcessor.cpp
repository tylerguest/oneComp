#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GRMeter.h"

using namespace juce;

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
        "threshold", 
        "Threshold", 
        NormalisableRange<float>(-60.0f, 0.0f, 1.0f, 1.0f), 
        10.0f 
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "ratio",
        "Ratio",
        NormalisableRange<float>(1.0f, 25.0f, 0.1f, 0.7f), 
        21.5f 
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "attack",
        "Attack",
        NormalisableRange<float>(0.1f, 150.0f, 0.05f, 0.5f), 
        2.0f 
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "release",
        "Release",
        NormalisableRange<float>(1.5f, 2000.0f, 0.1f, 0.5f), 
        120.0f 
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        NormalisableRange<float>(-30.0f, 30.0f), 
        0.0f 
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "input",
        "Input",
        NormalisableRange<float>(-30.0f, 30.0f), 
        0.0f 
    ));

    parameters.state = juce::ValueTree("savedParams");

    
}

OneCompAudioProcessor::~OneCompAudioProcessor()
{
    
}


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
    return 1;   
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

void OneCompAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    compressor.prepare(spec);
}

void OneCompAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OneCompAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float inputGain = *parameters.getRawParameterValue("input");
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Apply input gain directly to the sample
            channelData[sample] *= juce::Decibels::decibelsToGain(inputGain);
        }
    }

    auto inputLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples()); 
    float inputLevelDb = inputLevel > 0.0f ? juce::Decibels::gainToDecibels(inputLevel) : -100.0f;
    lastInputLevel.store(inputLevelDb, std::memory_order_relaxed);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    float threshold = *parameters.getRawParameterValue("threshold");
    float ratio = *parameters.getRawParameterValue("ratio");
    float attackTime = *parameters.getRawParameterValue("attack");
    float releaseTime = *parameters.getRawParameterValue("release");
    float makeupGain = *parameters.getRawParameterValue("gain");

    compressor.setThreshold(*parameters.getRawParameterValue("threshold"));
    compressor.setRatio(*parameters.getRawParameterValue("ratio"));
    compressor.setAttack(*parameters.getRawParameterValue("attack"));
    compressor.setRelease(*parameters.getRawParameterValue("release"));

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    compressor.process(context);

    auto outputLevel = buffer.getRMSLevel(0, 0, buffer.getNumSamples());

    float gainReductionDb = outputLevel > 0 ? juce::Decibels::gainToDecibels(inputLevel / outputLevel) : 0.f;

    lastGainReduction.store(-gainReductionDb, std::memory_order_release);

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            channelData[sample] *= juce::Decibels::decibelsToGain(makeupGain);
        }
    }

    auto outputLevel2 = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    float outputLevelDb = outputLevel2 > 0.0f ? juce::Decibels::gainToDecibels(outputLevel2) : -100.0f;
    lastOutputLevel.store(outputLevelDb, std::memory_order_release);

}

bool OneCompAudioProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* OneCompAudioProcessor::createEditor()
{
    return new OneCompAudioProcessorEditor(*this);
}

void OneCompAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OneCompAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OneCompAudioProcessor();
}

float OneCompAudioProcessor::getGainReduction() const {
    return lastGainReduction.load(std::memory_order_relaxed); 
}

float OneCompAudioProcessor::getInputLevel() const {
    return lastInputLevel.load(std::memory_order_relaxed);
}

float OneCompAudioProcessor::getOutputLevel() const {
    return lastOutputLevel.load(std::memory_order_relaxed);
}