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
                std::make_unique<juce::AudioParameterFloat> (attackParamID, "Attack", 0.0f, 2.0f, 1.0f), // 1.0f is neutral
                std::make_unique<juce::AudioParameterFloat> (sustainParamID, "Sustain", 0.0f, 2.0f, 1.0f), // 1.0f is neutral
                std::make_unique<juce::AudioParameterFloat> (attackTimeParamID, "Attack Time (ms)", 1.0f, 20.0f, 5.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainTimeParamID, "Sustain Time (ms)", 10.0f, 150.0f, 50.0f),
                std::make_unique<juce::AudioParameterFloat> (releaseTimeParamID, "Release Time (ms)", 10.0f, 200.0f, 100.0f)
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

    // Get the parameter values
    float attack = *parameters.getRawParameterValue (attackParamID);
    float sustain = *parameters.getRawParameterValue (sustainParamID);

    // Get time-related parameter values
    float attackTimeMs = *parameters.getRawParameterValue (attackTimeParamID);
    float sustainTimeMs = *parameters.getRawParameterValue (sustainTimeParamID);
    float releaseTimeMs = *parameters.getRawParameterValue (releaseTimeParamID);

    const float sampleRate = getSampleRate();

    // Convert times to coefficients
    const float attackCoeff = std::exp(-1.0f / (attackTimeMs * 0.001f * sampleRate));
    const float sustainCoeff = std::exp(-1.0f / (sustainTimeMs * 0.001f * sampleRate));
    const float releaseCoeff = std::exp(-1.0f / (releaseTimeMs * 0.001f * sampleRate));

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float inputSample = channelData[i];
            float rectifiedSample = std::abs(inputSample);

            // Envelope follower for transient detection
            if (rectifiedSample > envelope)
            {
                envelope = attackCoeff * (envelope - rectifiedSample) + rectifiedSample;
                inTransient = true; // We are in the transient phase
                inSustain = false;  // Reset sustain flag during the transient phase
            }
            else
            {
                envelope = releaseCoeff * (envelope - rectifiedSample) + rectifiedSample;
                inTransient = false;

                // Once the transient phase ends, we enter the sustain phase
                if (!inSustain)
                {
                    inSustain = true;
                }
            }

            // Apply attack shaping during the transient phase
            if (inTransient)
            {
                gain = juce::jmap(attack, 0.5f, 1.5f, 0.7f, 1.3f); // Adjust gain based on attack control
            }
            else if (inSustain)
            {
                // Apply sustain shaping during the tail phase
                // Increase gain reduction strength for sustain control
                if (sustain < 1.0f)
                {
                    gain = 1.0f - (1.0f - sustain) * sustainCoeff; // Aggressive tail reduction when sustain < 1.0
                }
                else
                {
                    gain = juce::jmap(sustain, 1.0f, 2.0f, 1.0f, 1.2f) * sustainCoeff; // Subtle enhancement when sustain > 1.0
                }
            }

            // Apply gain smoothly with a limiter to avoid distortion
            channelData[i] *= juce::jlimit(0.0f, 2.0f, gain);
        }
    }
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
