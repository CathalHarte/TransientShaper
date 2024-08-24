#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters (*this, nullptr, "Parameters",
            {
                std::make_unique<juce::AudioParameterFloat> (attackParamID, "Attack", 0.0f, 2.0f, 1.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainParamID, "Sustain", 0.0f, 2.0f, 1.0f)
            })
{
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
    return false;
}

bool PluginProcessor::producesMidi() const
{
    return false;
}

bool PluginProcessor::isMidiEffect() const
{
    return false;
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    float attack = *parameters.getRawParameterValue (attackParamID);
    float sustain = *parameters.getRawParameterValue (sustainParamID);

    const float attackTime = 0.01f; // Attack time in seconds
    const float releaseTime = 0.1f; // Release time in seconds
    const float sampleRate = getSampleRate();

    // Convert attack and release times to coefficients
    float attackCoeff = std::exp(-1.0f / (attackTime * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (releaseTime * sampleRate));

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();

        float envelope = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            // Calculate envelope
            float rectifiedSample = std::abs(channelData[i]);

            if (rectifiedSample > envelope)
                envelope = attackCoeff * (envelope - rectifiedSample) + rectifiedSample;
            else
                envelope = releaseCoeff * (envelope - rectifiedSample) + rectifiedSample;

            // Apply attack shaping
            if (rectifiedSample > envelope * 1.1f) // 1.1 is a threshold multiplier, can be adjusted
                channelData[i] *= attack;

            // Apply sustain shaping
            if (rectifiedSample < envelope * 0.9f) // Sustain handling, adjust the ratio as needed
                channelData[i] *= sustain;
        }
    }
}

float envelopeFollower(const float* input, int numSamples, float attackCoeff, float releaseCoeff)
{
    float envelope = 0.0f;
    
    for (int i = 0; i < numSamples; ++i)
    {
        float rectifiedSample = std::abs(input[i]); // Full-wave rectification
        
        if (rectifiedSample > envelope)
            envelope = attackCoeff * (envelope - rectifiedSample) + rectifiedSample;
        else
            envelope = releaseCoeff * (envelope - rectifiedSample) + rectifiedSample;
    }
    
    return envelope;
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save plugin state
    std::unique_ptr<juce::XmlElement> xml (parameters.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore plugin state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            parameters.state = juce::ValueTree::fromXml (*xmlState);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
