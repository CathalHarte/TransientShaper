#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    // Would be great to get this up and running, edit the look
    // addAndMakeVisible (inspectButton);

    attackKnob.setSliderStyle (juce::Slider::Rotary);
    attackKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible (&attackKnob);

    sustainKnob.setSliderStyle (juce::Slider::Rotary);
    sustainKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible (&sustainKnob);

    saturationRoutingButton.setButtonText ("Saturation Before Shaper");
    addAndMakeVisible (&saturationRoutingButton);

    clipperButton.setButtonText ("Clipper");
    addAndMakeVisible (&clipperButton);

    // this chunk of code instantiates and opens the melatonin inspector
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre(100, 50));

    attackKnob.setBounds (50, 50, 100, 100);
    sustainKnob.setBounds (200, 50, 100, 100);
    saturationRoutingButton.setBounds (50, 200, 200, 30);
    clipperButton.setBounds (50, 250, 100, 30);
}
