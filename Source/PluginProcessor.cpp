/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


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
        -60.0f,      // minimum value
        0.0f,        // maximum value
        0.0f         // default value
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "ratio",
        "Ratio",
        1.0f,
        50.0f,
        2.5f
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "attack",
        "Attack",
        .5f,
        500.0f,
        16.0f
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "release",
        "Release",
        5.0f,
        5000.0f,
        160.0f
    ));

    parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        -30.0f,
        30.0f,
        0.0f
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
    compressor.setThreshold(-20.f);
    compressor.setRatio(2.f);
    compressor.setAttack(10.f);
    compressor.setRelease(100.f);

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

    // Assuming 'compressor' is a member of type juce::dsp::Compressor<float> or similar
    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setAttack(attackTime);
    compressor.setRelease(releaseTime);

    // Prepare the DSP context
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process the block with the updated compressor settings
    compressor.process(context);

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
}

void OneCompAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OneCompAudioProcessor();
}