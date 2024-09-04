#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    juce::Slider thresholdKnob;

    juce::Slider attackKnob;
    juce::Slider sustainKnob;

    juce::Slider attackTimeSlider;
    juce::Slider bodyTimeSlider;
    juce::Slider sustainTimeSlider;

    juce::Slider attackCurveSlider;
    juce::Slider bodyCurveSlider;
    juce::Slider sustainCurveSlider;

    // Labels for knobs
    juce::Label thresholdLabel;

    juce::Label attackLabel;
    juce::Label sustainLabel;
    
    juce::Label attackTimeLabel;
    juce::Label bodyTimeLabel;
    juce::Label sustainTimeLabel;
    
    juce::Label attackCurveLabel;
    juce::Label bodyCurveLabel;
    juce::Label sustainCurveLabel;

    // Slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bodyTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainTimeAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackCurveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bodyCurveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainCurveAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
