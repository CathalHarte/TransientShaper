#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Declare the isBusesLayoutSupported function here
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // APVTS object to manage plugin parameters
    juce::AudioProcessorValueTreeState parameters;

    // Parameter IDs
    static constexpr auto attackParamID = "attack";
    static constexpr auto sustainParamID = "sustain";

    static constexpr auto attackTimeParamID = "attackTimeMs";
    static constexpr auto sustainTimeParamID = "sustainTimeMs";
    static constexpr auto releaseTimeParamID = "releaseTimeMs";

private:
    // Envelope-related variables
    float envelope = 0.0f;
    float gain = 1.0f;
    bool inTransient = false;
    bool inSustain = false;

    void applyTransientShaper (juce::AudioBuffer<float>& buffer, float attack, float sustain);
    void applySaturation (juce::AudioBuffer<float>& buffer);
    void applyClipper (juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
