#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SlidingWindowEnergy.h"

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
                std::make_unique<juce::AudioParameterFloat> (thresholdParamID, "Threshold", -20.0f, 0.0f, -10.0f),
                
                std::make_unique<juce::AudioParameterFloat> (attackTimeParamID, "Attack Time (ms)", 1.0f, 20.0f, 5.0f),
                std::make_unique<juce::AudioParameterFloat> (bodyTimeParamID, "Sustain Time (ms)", 10.0f, 150.0f, 50.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainTimeParamID, "Release Time (ms)", 10.0f, 200.0f, 100.0f),
                
                std::make_unique<juce::AudioParameterFloat> (attackCurveParamID, "Attack Curve", 1.0f, 10.0f, 3.0f),
                std::make_unique<juce::AudioParameterFloat> (bodyCurveParamID, "Body Curve", 1.0f, 10.0f, 3.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainCurveParamID, "Sustain Curve", 1.0f, 10.0f, 3.0f)
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

int body_samples = 0;
int sustain_samples = 0;
double sample_rate = 0;
std::unique_ptr<SlidingWindowEnergy> window;
int windowSizeInSamples = 0;
float cached_gain = 1;
float prev_gain = 1;

enum class State
{
    IDLE,       // Waiting for a transient
    BODY,  // A transient is detected, apply gain envelope to the body of the sound
    SUSTAIN     // After the body of the sound has passed, enter the sustain phase and apply the sustain envelope
};

State current_state = State::IDLE;

void PluginProcessor::prepareToPlay (double sampleRate, [[maybe_unused]] int samplesPerBlock)
{
    body_samples = 0;
    sustain_samples = 0;
    sample_rate = sampleRate;
    cached_gain = 1;
    prev_gain = 1;

    windowSizeInSamples = (int)(0.005 * sampleRate);  // 5ms window
    window = std::make_unique<SlidingWindowEnergy>(windowSizeInSamples);
}

void PluginProcessor::releaseResources()
{
}


void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    (void)midiMessages;

    // Get the parameter values
    float threshold = *parameters.getRawParameterValue(thresholdParamID);

    float attack = *parameters.getRawParameterValue (attackParamID);
    float sustain = *parameters.getRawParameterValue (sustainParamID);

    float attackTimeMs = *parameters.getRawParameterValue (attackTimeParamID);
    float bodyTimeMs = *parameters.getRawParameterValue (bodyTimeParamID);
    float sustainTimeMs = *parameters.getRawParameterValue(sustainTimeParamID);

    int attackSampleCount = static_cast<int>(attackTimeMs * 0.001f * sample_rate);
    int bodySampleCount = static_cast<int>(bodyTimeMs * 0.001f * sample_rate) - attackSampleCount;
    int sustainSampleCount = static_cast<int>(sustainTimeMs * 0.001f * sample_rate);


    float attackCurve = *parameters.getRawParameterValue(attackCurveParamID);
    float bodyCurve = *parameters.getRawParameterValue(bodyCurveParamID);
    float sustainCurve = *parameters.getRawParameterValue(sustainCurveParamID);

    float normalizationFactorAttack = 1.0f - std::exp(-attackCurve);
    float normalizationFactorBody = 1.0f - std::exp(-bodyCurve);
    float normalizationFactorSustain = 1.0f - std::exp(-sustainCurve);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float inputSample = channelData[i];

            // When receiving a new sample:
            window->addSample(inputSample);  // Add the latest sample to the sliding window

            // Calculate the energy in the window:
            float energy = window->calculateEnergy();
            float dB = 10.0f * std::log10(energy + 1e-6f);  // Convert to dB

            // Compare dB level with your threshold to detect transient
            if (dB > threshold) {
                // Transient detected

                if (current_state == State::IDLE)
                {
                    prev_gain = 1;
                }
                else 
                {
                    prev_gain = cached_gain;
                }

                current_state = State::BODY;
            }

            float gain = 1;
            switch (current_state)
            {
                case State::IDLE:
                    gain = 1;
                    break;

                case State::BODY:
                    {
                        // Calculate the attack curve with normalization
                        if (body_samples <= attackSampleCount)
                        {
                            // Attack phase: progress from 1.0 to the attack gain
                            float progress = static_cast<float>(body_samples) / attackSampleCount;
                            float curve = (1.0f - std::exp(-attackCurve * progress)) / normalizationFactorAttack;
                            gain = prev_gain + (attack - prev_gain) * curve;
                        }
                        else
                        {
                            // Body phase: progress back from the attack gain to 1.0
                            float bodyProgress = static_cast<float>(body_samples - attackSampleCount) / bodySampleCount;
                            float curve = (1.0f - std::exp(-bodyCurve * bodyProgress)) / normalizationFactorBody;
                            gain = 1.0f + (attack - 1.0f) * (1.0f - curve);
                        }
                    
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
                        float sustainProgress = static_cast<float>(sustain_samples) / sustainSampleCount;
                        float curve = (1.0f - std::exp(-sustainCurve * sustainProgress)) / normalizationFactorSustain;
                        gain = 1.0f + (sustain - 1.0f) * curve;

                        sustain_samples++;
                        if (sustain_samples >= sustainSampleCount) {
                            current_state = State::IDLE;
                            sustain_samples = 0;
                        }
                    }
                    break;
            }

            // Apply the gain (with some tanh saturation)
            channelData[i] = tanh(channelData[i] * gain);

            cached_gain = gain;            
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
