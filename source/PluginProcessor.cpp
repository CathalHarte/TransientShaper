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
                std::make_unique<juce::AudioParameterFloat> (thresholdParamID, "Threshold", -30.0f, 0.0f, -10.0f),

                std::make_unique<juce::AudioParameterFloat> (attackParamID, "Attack", 0.0f, 2.0f, 1.0f), // 1.0f is neutral                
                std::make_unique<juce::AudioParameterFloat> (releaseParamID, "Release", 0.0f, 2.0f, 1.0f), // 1.0f is neutral
                
                std::make_unique<juce::AudioParameterFloat> (attackTimeParamID, "Attack Time (ms)", 1.0f, 20.0f, 5.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainTimeParamID, "Sustain Time (ms)", 1.0f, 150.0f, 50.0f),
                std::make_unique<juce::AudioParameterFloat> (releaseTimeParamID, "Release Time (ms)", 10.0f, 1000.0f, 100.0f),
                
                std::make_unique<juce::AudioParameterFloat> (attackCurveParamID, "Attack Curve", 1.0f, 10.0f, 3.0f),
                std::make_unique<juce::AudioParameterFloat> (sustainCurveParamID, "Sustain Curve", 1.0f, 10.0f, 3.0f),
                std::make_unique<juce::AudioParameterFloat> (releaseCurveParamID, "Release Curve", 1.0f, 20.0f, 3.0f)
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

int sustain_samples = 0;
int release_samples = 0;
double sample_rate = 0;
std::unique_ptr<SlidingWindowEnergy> window;
int windowSizeInSamples = 0;
float cached_gain = 1;
float prev_gain = 1;
int refractoryPeriodSamples;
int refractoryCounter = 0;
float prev_dB = 0;


enum class State
{
    IDLE,       // Waiting for a transient
    SUSTAIN,  // A transient is detected, apply gain envelope to the sustain portion of the sound
    RELEASE     // After the body of the sound has passed, enter the release phase and apply the release envelope
};

State current_state = State::IDLE;

void PluginProcessor::prepareToPlay (double sampleRate, [[maybe_unused]] int samplesPerBlock)
{
    sustain_samples = 0;
    release_samples = 0;
    sample_rate = sampleRate;
    cached_gain = 1;
    prev_gain = 1;
    prev_dB = 0;

    refractoryPeriodSamples = (int)(0.1 * sampleRate); // 16th note at 150 bpm is 100ms
    refractoryCounter = 0;

    windowSizeInSamples = (int)(0.005 * sampleRate);  // 2ms window
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
    float release = *parameters.getRawParameterValue (releaseParamID);

    float attackTimeMs = *parameters.getRawParameterValue (attackTimeParamID);
    float sustainTimeMs = *parameters.getRawParameterValue (sustainTimeParamID);
    float releaseTimeMs = *parameters.getRawParameterValue(releaseTimeParamID);

    int attackSampleCount = static_cast<int>(attackTimeMs * 0.001f * sample_rate);
    int sustainSampleCount = static_cast<int>(sustainTimeMs * 0.001f * sample_rate);
    int releaseSampleCount = static_cast<int>(releaseTimeMs * 0.001f * sample_rate);


    float attackCurve = *parameters.getRawParameterValue(attackCurveParamID);
    float sustainCurve = *parameters.getRawParameterValue(sustainCurveParamID);
    float releaseCurve = *parameters.getRawParameterValue(releaseCurveParamID);

    float normalizationFactorAttack = 1.0f - std::exp(-attackCurve);
    float normalizationFactorSustain = 1.0f - std::exp(-sustainCurve);
    float normalizationFactorRelease = 1.0f - std::exp(-releaseCurve);

    auto* channelData = buffer.getWritePointer(0);
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
        // if not in the refractory period (but having possibly just left)
        // look at the previous dB calculation, and make sure we are bigger than
        // that

        if (refractoryCounter <= 0 && dB > threshold) {
            // Transient detected

            refractoryCounter = refractoryPeriodSamples;

            if (current_state == State::IDLE)
            {
                prev_gain = 1;
            }
            else 
            {
                prev_gain = cached_gain;
            }

            current_state = State::SUSTAIN;
        }
        else if (refractoryCounter > 0){
            refractoryCounter--;
        }

        prev_dB = dB; // single channel save, but we have multiple channelssss

        float gain = 1;
        switch (current_state)
        {
            case State::IDLE:
                gain = 1;
                break;

            case State::SUSTAIN:
                {
                    // Calculate the attack curve with normalization
                    if (sustain_samples <= attackSampleCount)
                    {
                        // Attack phase: progress from 1.0 to the attack gain
                        float progress = static_cast<float>(sustain_samples) / attackSampleCount;
                        float curve = (1.0f - std::exp(-attackCurve * progress)) / normalizationFactorAttack;
                        gain = prev_gain + ((attack - prev_gain) * curve);
                    }
                    else
                    {
                        // progress back from the attack gain to 1.0
                        float sustainProgress = static_cast<float>(sustain_samples - attackSampleCount) / sustainSampleCount;
                        float curve = (1.0f - std::exp(-sustainCurve * sustainProgress)) / normalizationFactorSustain;
                        gain = 1.0f + ((attack - 1.0f) * (1.0f - curve));
                    }
                
                    sustain_samples++;
                    if (sustain_samples >= sustainSampleCount)
                    {
                        current_state = State::RELEASE;
                        prev_gain = gain;
                        sustain_samples = 0;
                    }
                }
                break;

            case State::RELEASE:
                {
                    // Calculate the release curve
                    float releaseProgress = static_cast<float>(release_samples) / releaseSampleCount;
                    float curve = (1.0f - std::exp(-releaseCurve * releaseProgress)) / normalizationFactorRelease;
                    gain = prev_gain + ((release - prev_gain) * curve);

                    release_samples++;
                    if (release_samples >= releaseSampleCount) {
                        current_state = State::IDLE;
                        release_samples = 0;
                    }
                }
                break;
        }

        // Apply the gain (with some tanh saturation)
        channelData[i] = tanh(channelData[i] * gain);

        cached_gain = gain;    
    }        

    // Optionally, you could copy the processed mono data to the other channels (for stereo output)
    for (int channel = 1; channel < buffer.getNumChannels(); ++channel) {
        float* allChannelData = buffer.getWritePointer(channel);
        std::memcpy(allChannelData, channelData, sizeof(float) * buffer.getNumSamples());
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
