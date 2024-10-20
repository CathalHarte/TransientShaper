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
                std::make_unique<juce::AudioParameterFloat> (releaseParamID, "Sustain", 0.0f, 2.0f, 1.0f), // 1.0f is neutral
                
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
int refractoryPeriodSamples = 500;  // Refractory period in samples (adjust based on needs)
int refractoryCounter = 0;  // Initialize the refractory counter

enum class State
{
    IDLE,       // Waiting for a transient
    SUSTAIN,  // A transient is detected, apply gain envelope to the sustain portion of the sound
    RELEASE     // After the body of the sound has passed, enter the release phase and apply the release envelope
};

// Define variables for Fuzz Face distortion parameters
float c1, c2, r1, r2, r3;
float a1, a2, b1, b2;
float s1, s2;

State current_state = State::IDLE;
void PluginProcessor::prepareToPlay (double sampleRate, [[maybe_unused]] int samplesPerBlock)
{


    // Initialize parameters for the Fuzz Face distortion
    sample_rate = sampleRate;
    c1 = 0.01e-6f;  // 0.01uF capacitor
    c2 = 2.2e-6f;   // 2.2uF capacitor
    r1 = 33e3f;     // 33k resistor
    r2 = 8.2e3f;    // 8.2k resistor
    r3 = 470.0f;    // 470 ohm resistor

    // Pre-calculate constants
    const float T = 1.0f / static_cast<float>(sampleRate);
    a1 = T / (2.0f * c1);
    a2 = T / (2.0f * c2);
    b1 = (2.0f * r1 * c1 - T) / (2.0f * r1 * c1 + T);
    b2 = (2.0f * r3 * c2 - T) / (2.0f * r3 * c2 + T);

    // Initialize state variables
    s1 = s2 = 0.0f;
}

void PluginProcessor::releaseResources()
{
    // Reset state variables
    s1 = s2 = 0.0f;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    (void)midiMessages;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];

            // First stage (input transistor)
            float u1 = (input - r1 * (a1 * input + s1)) / (1 + r1 * a1);
            float v1 = a1 * u1 + s1;
            s1 = 2 * v1 - s1;

            // Clipping stage
            float clip = std::tanh(v1 / 0.025f) * 0.025f;

            // Second stage (output transistor)
            float u2 = (clip - r3 * (a2 * clip + s2)) / (1 + r3 * a2);
            float v2 = a2 * u2 + s2;
            s2 = 2 * v2 - s2;

            // Output
            channelData[sample] = v2 * 5.0f;  // Amplify the output
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
