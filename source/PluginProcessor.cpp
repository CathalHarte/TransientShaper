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
                       )
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
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // for (int channel = 0; channel < totalNumInputChannels; ++channel)
    // {
    //     auto* channelData = buffer.getWritePointer (channel);
    //     juce::ignoreUnused (channelData);
    //     // ..do something to the data...
    // }

    // So am I missing something, then? Multi channel processing?

    if (saturationBefore)
        applySaturation (buffer);

    applyTransientShaper (buffer);

    if (!saturationBefore)
        applySaturation (buffer);

    if (clipperEnabled)
        applyClipper (buffer);
}

void PluginProcessor::applyTransientShaper(juce::AudioBuffer<float>& buffer)
{
    const float transientThreshold = 0.1f; // Threshold for detecting transients
    const float transientBoost = 1.2f;     // Amount to boost transients
    const float attackTime = 0.01f;        // Attack time in seconds
    const float releaseTime = 0.1f;        // Release time in seconds
    const float sampleRate = getSampleRate(); // Get the sample rate

    // Convert attack and release times from seconds to samples
    int attackSamples = static_cast<int>(attackTime * sampleRate);
    int releaseSamples = static_cast<int>(releaseTime * sampleRate);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();

        float gain = 1.0f; // Initial gain (no effect)

        for (int i = 1; i < numSamples; ++i)
        {
            float currentSample = channelData[i];
            float previousSample = channelData[i - 1];
            
            // Detect transients based on sudden changes
            if (std::abs(currentSample - previousSample) > transientThreshold)
            {
                // Apply attack phase
                gain = std::min(gain + (transientBoost - gain) / attackSamples, transientBoost);
            }
            else
            {
                // Apply release phase
                gain = std::max(gain - (gain - 1.0f) / releaseSamples, 1.0f);
            }

            // Apply gain to the current sample
            channelData[i] *= gain;
        }
    }
}


void PluginProcessor::applySaturation(juce::AudioBuffer<float>& buffer)
{
    const float saturationAmount = 0.5f; // Adjust saturation amount as needed

    // Iterate through each channel
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            // Apply a simple saturation effect using a soft clipper
            float x = channelData[i];
            float saturatedValue = x / (1.0f + std::abs(x) * saturationAmount);
            channelData[i] = saturatedValue;
        }
    }
}


void PluginProcessor::applyClipper (juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (channelData[sample] > 1.0f)
                channelData[sample] = 1.0f;
            else if (channelData[sample] < -1.0f)
                channelData[sample] = -1.0f;
        }
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Store parameters here
    std::unique_ptr<juce::XmlElement> xml (new juce::XmlElement ("StateInfo"));
    xml->setAttribute ("attack", (double) attack);
    xml->setAttribute ("sustain", (double) sustain);
    xml->setAttribute ("saturationBefore", saturationBefore);
    xml->setAttribute ("clipperEnabled", clipperEnabled);
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore parameters here
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName ("StateInfo"))
        {
            attack = (float) xmlState->getDoubleAttribute ("attack", 0.5);
            sustain = (float) xmlState->getDoubleAttribute ("sustain", 0.5);
            saturationBefore = xmlState->getBoolAttribute ("saturationBefore", false);
            clipperEnabled = xmlState->getBoolAttribute ("clipperEnabled", false);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
