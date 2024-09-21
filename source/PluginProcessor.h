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

    int getLatencySamples() override;
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
    // the threshold to detect a transient
    static constexpr auto thresholdParamID = "threshold";


    // gain values, allowing to increase / decrease two parts of the sound
    static constexpr auto attackParamID = "attack";
    static constexpr auto releaseParamID = "release";


    // for setting the slope of the attack phase - the duration of time over which the "attack" gain is reached
    static constexpr auto attackTimeParamID = "attackTimeMs";

    // for setting the duration of the body of the sound, which contains both the first boosted transient,
    // and an untouched portion of the sound (before the sustain phase is entered)
    static constexpr auto sustainTimeParamID = "sustainTimeMs"; 

    // for setting the duration of the sustain phase - the duration of time over which the "sustain" gain is reached
    static constexpr auto releaseTimeParamID = "releaseTimeMs";

    // for setting the slope behaviour
    static constexpr auto attackCurveParamID = "attackCurve";

    static constexpr auto sustainCurveParamID = "sustainCurve";

    static constexpr auto releaseCurveParamID = "releaseCurve";

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
