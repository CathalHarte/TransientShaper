#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Attach sliders and buttons to APVTS parameters
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackParamID, attackKnob);

    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::sustainParamID, sustainKnob);


    // Set up slider and button properties
    attackKnob.setSliderStyle (juce::Slider::Rotary);
    attackKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible (attackKnob);

    sustainKnob.setSliderStyle (juce::Slider::Rotary);
    sustainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible (sustainKnob);

    attackTimeSlider.setSliderStyle(juce::Slider::Rotary);
    attackTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackTimeSlider);

    sustainTimeSlider.setSliderStyle(juce::Slider::Rotary);
    sustainTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(sustainTimeSlider);

    releaseTimeSlider.setSliderStyle(juce::Slider::Rotary);
    releaseTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(releaseTimeSlider);

    // Attach sliders to parameters
    attackTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackTimeParamID, attackTimeSlider);

    sustainTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::bodyTimeParamID, sustainTimeSlider);

    releaseTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::sustainTimeParamID, releaseTimeSlider);


    // Set the size of the editor
    setSize (600, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // Fill background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    // Layout positions of child components
    attackKnob.setBounds (50, 50, 100, 100);
    sustainKnob.setBounds (200, 50, 100, 100);
    attackTimeSlider.setBounds(50, 200, 100, 100);
    sustainTimeSlider.setBounds(200, 200, 100, 100);
    releaseTimeSlider.setBounds(350, 200, 100, 100);
}
