#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Attach sliders and buttons to APVTS parameters
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::thresholdParamID, thresholdKnob);


    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackParamID, attackKnob);

    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::releaseParamID, releaseKnob);


    attackTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackTimeParamID, attackTimeSlider);

    sustainTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::sustainTimeParamID, sustainTimeSlider);

    releaseTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::releaseTimeParamID, releaseTimeSlider);


    attackCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackCurveParamID, attackCurveSlider);

    sustainCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::sustainCurveParamID, sustainCurveSlider);

    releaseCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::releaseCurveParamID, releaseCurveSlider);


    // Set up slider properties
    thresholdKnob.setSliderStyle(juce::Slider::Rotary);
    thresholdKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(thresholdKnob);


    attackKnob.setSliderStyle(juce::Slider::Rotary);
    attackKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackKnob);

    releaseKnob.setSliderStyle(juce::Slider::Rotary);
    releaseKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(releaseKnob);


    attackTimeSlider.setSliderStyle(juce::Slider::Rotary);
    attackTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackTimeSlider);

    sustainTimeSlider.setSliderStyle(juce::Slider::Rotary);
    sustainTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(sustainTimeSlider);

    releaseTimeSlider.setSliderStyle(juce::Slider::Rotary);
    releaseTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(releaseTimeSlider);


    attackCurveSlider.setSliderStyle(juce::Slider::Rotary);
    attackCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackCurveSlider);

    sustainCurveSlider.setSliderStyle(juce::Slider::Rotary);
    sustainCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(sustainCurveSlider);

    releaseCurveSlider.setSliderStyle(juce::Slider::Rotary);
    releaseCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(releaseCurveSlider);


    // Initialize and configure labels
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.attachToComponent(&thresholdKnob, false);  // Attach above the knob
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackKnob, false);  // Attach above the knob
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    sustainLabel.setText("Release", juce::dontSendNotification);
    sustainLabel.attachToComponent(&releaseKnob, false);  // Attach above the knob
    sustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainLabel);


    attackTimeLabel.setText("Attack Time", juce::dontSendNotification);
    attackTimeLabel.attachToComponent(&attackTimeSlider, false);
    attackTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackTimeLabel);

    sustainTimeLabel.setText("Sustain Time", juce::dontSendNotification);
    sustainTimeLabel.attachToComponent(&sustainTimeSlider, false);
    sustainTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainTimeLabel);

    releaseTimeLabel.setText("Release Time", juce::dontSendNotification);
    releaseTimeLabel.attachToComponent(&releaseTimeSlider, false);
    releaseTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseTimeLabel);


    attackCurveLabel.setText("Attack Curve", juce::dontSendNotification);
    attackCurveLabel.attachToComponent(&attackCurveSlider, false);
    attackCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackCurveLabel);

    sustainCurveLabel.setText("Sustain Curve", juce::dontSendNotification);
    sustainCurveLabel.attachToComponent(&sustainCurveSlider, false);
    sustainCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainCurveLabel);

    releaseCurveLabel.setText("Release Curve", juce::dontSendNotification);
    releaseCurveLabel.attachToComponent(&releaseCurveSlider, false);
    releaseCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseCurveLabel);


    // Set the size of the editor
    setSize(500, 700);
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
    thresholdKnob.setBounds (50, 50, 100, 100);

    attackKnob.setBounds (50, 200, 100, 100);
    releaseKnob.setBounds (200, 200, 100, 100);
    
    attackTimeSlider.setBounds(50, 350, 100, 100);
    sustainTimeSlider.setBounds(200, 350, 100, 100);
    releaseTimeSlider.setBounds(350, 350, 100, 100);

    attackCurveSlider.setBounds(50, 500, 100, 100);
    sustainCurveSlider.setBounds(200, 500, 100, 100);
    releaseCurveSlider.setBounds(350, 500, 100, 100);
}
