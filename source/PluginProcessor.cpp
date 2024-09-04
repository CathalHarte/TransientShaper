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
                std::make_unique<juce::AudioParameterFloat> (bodyTimeParamID, "Sustain Time (ms)", 10.0f, 150.0f, 50.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainTimeParamID, "Release Time (ms)", 10.0f, 200.0f, 100.0f)
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

float prev_val = 0.0f;
int body_samples = 0;
int sustain_samples = 0;

enum class State
{
    IDLE,       // Waiting for a transient
    BODY,  // A transient is detected, apply gain envelope to the body of the sound
    SUSTAIN     // After the body of the sound has passed, enter the sustain phase and apply the sustain envelope
};

State current_state = State::IDLE;

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const double sampleRate = getSampleRate();

    // Get the parameter values
    float attack = *parameters.getRawParameterValue (attackParamID);
    float sustain = *parameters.getRawParameterValue (sustainParamID);

    // Get time-related parameter values
    float attackTimeMs = *parameters.getRawParameterValue (attackTimeParamID);
    float bodyTimeMs = *parameters.getRawParameterValue (bodyTimeParamID);
    float sustainTimeMs = *parameters.getRawParameterValue(sustainTimeParamID);

    int attackSampleCount = static_cast<int>(attackTimeMs * 0.001f * sampleRate);
    int bodySampleCount = static_cast<int>(bodyTimeMs * 0.001f * sampleRate);
    int sustainSampleCount = static_cast<int>(sustainTimeMs * 0.001f * sampleRate);

    // Attack and release curve parameters
    const float attackCurve = 5.0f; // Adjust for desired attack curve
    const float releaseCurve = 2.0f; // Adjust for desired release curve

    float normalizationFactorAttack = 1.0f - std::exp(-attackCurve);
    float normalizationFactorRelease = 1.0f - std::exp(-releaseCurve);

    // Define a minimum threshold for transient detection
    const float minThreshold = 0.2f; // Adjust this value based on your needs

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float inputSample = channelData[i];

            // Check if a transient is detected to move to the TRANSIENT state
            if (std::abs(inputSample - prev_val) > minThreshold) {
                current_state = State::BODY;
            }

            switch (current_state)
            {
                case State::IDLE:
                    break;

                case State::BODY:
                    {
                        float gain;
                        // Calculate the attack curve with normalization
                        if (body_samples <= attackSampleCount)
                        {
                            // Attack phase: progress from 1.0 to the attack gain
                            float progress = static_cast<float>(body_samples) / attackSampleCount;
                            float curve = (1.0f - std::exp(-attackCurve * progress)) / normalizationFactorAttack;
                            gain = 1.0f + (attack - 1.0f) * curve;
                        }
                        else
                        {
                            // Decay phase: progress back from the attack gain to 1.0
                            float decayProgress = static_cast<float>(body_samples - attackSampleCount) / bodySampleCount;
                            float curve = (1.0f - std::exp(-attackCurve * decayProgress)) / normalizationFactorRelease;
                            gain = 1.0f + (attack - 1.0f) * (1.0f - curve);
                        }

                        // Apply the gain
                        channelData[i] *= gain;
                    
                        body_samples++;
                        if (body_samples >= bodySampleCount)
                        {
                            current_state = State::SUSTAIN;
                            body_samples = 0;
                        }
                    }
                    break;

                case State::SUSTAIN:
                    {
                        // Calculate the release curve
                        float progress = static_cast<float>(sustain_samples) / sustainSampleCount;
                        float curve = std::exp(-releaseCurve * progress);
                        float gain = sustain + (1.0f - sustain) * curve;

                        // Apply the gain
                        channelData[i] *= gain;

                        sustain_samples++;
                        if (sustain_samples >= sustainSampleCount) {
                            current_state = State::IDLE;
                            sustain_samples = 0;
                        }
                    }
                    break;
            }

            prev_val = inputSample;            
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
