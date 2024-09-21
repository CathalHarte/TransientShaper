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
    juce::Slider releaseKnob;

    juce::Slider attackTimeSlider;
    juce::Slider sustainTimeSlider;
    juce::Slider releaseTimeSlider;

    juce::Slider attackCurveSlider;
    juce::Slider sustainCurveSlider;
    juce::Slider releaseCurveSlider;

    // Labels for knobs
    juce::Label thresholdLabel;

    juce::Label attackLabel;
    juce::Label sustainLabel;
    
    juce::Label attackTimeLabel;
    juce::Label sustainTimeLabel;
    juce::Label releaseTimeLabel;
    
    juce::Label attackCurveLabel;
    juce::Label sustainCurveLabel;
    juce::Label releaseCurveLabel;

    // Slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseTimeAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackCurveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainCurveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseCurveAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
