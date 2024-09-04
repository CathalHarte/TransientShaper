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
        processorRef.parameters, PluginProcessor::sustainParamID, sustainKnob);


    attackTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackTimeParamID, attackTimeSlider);

    bodyTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::bodyTimeParamID, bodyTimeSlider);

    sustainTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::sustainTimeParamID, sustainTimeSlider);


    attackCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::attackCurveParamID, attackCurveSlider);

    bodyCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::bodyCurveParamID, bodyCurveSlider);

    sustainCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, PluginProcessor::sustainCurveParamID, sustainCurveSlider);


    // Set up slider properties
    thresholdKnob.setSliderStyle(juce::Slider::Rotary);
    thresholdKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(thresholdKnob);


    attackKnob.setSliderStyle(juce::Slider::Rotary);
    attackKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackKnob);

    sustainKnob.setSliderStyle(juce::Slider::Rotary);
    sustainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(sustainKnob);


    attackTimeSlider.setSliderStyle(juce::Slider::Rotary);
    attackTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackTimeSlider);

    bodyTimeSlider.setSliderStyle(juce::Slider::Rotary);
    bodyTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(bodyTimeSlider);

    sustainTimeSlider.setSliderStyle(juce::Slider::Rotary);
    sustainTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(sustainTimeSlider);


    attackCurveSlider.setSliderStyle(juce::Slider::Rotary);
    attackCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(attackCurveSlider);

    bodyCurveSlider.setSliderStyle(juce::Slider::Rotary);
    bodyCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(bodyCurveSlider);

    sustainCurveSlider.setSliderStyle(juce::Slider::Rotary);
    sustainCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(sustainCurveSlider);


    // Initialize and configure labels
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.attachToComponent(&thresholdKnob, false);  // Attach above the knob
    thresholdLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(thresholdLabel);

    
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackKnob, false);  // Attach above the knob
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.attachToComponent(&sustainKnob, false);  // Attach above the knob
    sustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainLabel);


    attackTimeLabel.setText("Attack Time", juce::dontSendNotification);
    attackTimeLabel.attachToComponent(&attackTimeSlider, false);
    attackTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackTimeLabel);

    bodyTimeLabel.setText("Body Time", juce::dontSendNotification);
    bodyTimeLabel.attachToComponent(&bodyTimeSlider, false);
    bodyTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bodyTimeLabel);

    sustainTimeLabel.setText("Sustain Time", juce::dontSendNotification);
    sustainTimeLabel.attachToComponent(&sustainTimeSlider, false);
    sustainTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainTimeLabel);


    attackCurveLabel.setText("Attack Curve", juce::dontSendNotification);
    attackCurveLabel.attachToComponent(&attackCurveSlider, false);
    attackCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackCurveLabel);

    bodyCurveLabel.setText("Body Curve", juce::dontSendNotification);
    bodyCurveLabel.attachToComponent(&bodyCurveSlider, false);
    bodyCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bodyCurveLabel);

    sustainCurveLabel.setText("Sustain Curve", juce::dontSendNotification);
    sustainCurveLabel.attachToComponent(&sustainCurveSlider, false);
    sustainCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainCurveLabel);


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
    sustainKnob.setBounds (200, 200, 100, 100);
    
    attackTimeSlider.setBounds(50, 350, 100, 100);
    bodyTimeSlider.setBounds(200, 350, 100, 100);
    sustainTimeSlider.setBounds(350, 350, 100, 100);

    attackCurveSlider.setBounds(50, 500, 100, 100);
    bodyCurveSlider.setBounds(200, 500, 100, 100);
    sustainCurveSlider.setBounds(350, 500, 100, 100);
}
