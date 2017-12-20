/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
PluginTemplateAudioProcessorEditor::PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 300);
    setLookAndFeel (&globalLaF);
    
    
    addAndMakeVisible(&title);
    title.setTitle(String("Plugin"),String("Template"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    
    addAndMakeVisible(&cbOrderSetting);
    cbOrderSetting.setJustificationType(Justification::centred);
    cbOrderSetting.addSectionHeading("Order Setting");
    cbOrderSetting.addItem("Auto", 1);
    cbOrderSetting.addItem("0th", 2);
    cbOrderSetting.addItem("1st", 3);
    cbOrderSetting.addItem("2nd", 4);
    cbOrderSetting.addItem("3rd", 5);
    cbOrderSetting.addItem("4th", 6);
    cbOrderSetting.addItem("5th", 7);
    cbOrderSetting.addItem("6th", 8);
    cbOrderSetting.addItem("7th", 9);
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "orderSetting", cbOrderSetting);
    
    
    addAndMakeVisible(&slParam1);
    slParam1Attachment = new SliderAttachment(valueTreeState, "param1", slParam1);
    slParam1.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    
    addAndMakeVisible(&slParam2);
    slParam2Attachment = new SliderAttachment(valueTreeState, "param2", slParam2);
    slParam2.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slParam2.setReverse(true);
    
    //startTimer(100);
}

PluginTemplateAudioProcessorEditor::~PluginTemplateAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void PluginTemplateAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 0)));
    g.setFont (25.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void PluginTemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);

    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop    (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    
    
    Rectangle<int> sliderRow = area.removeFromTop(50);
    slParam1.setBounds(sliderRow.removeFromLeft(150));
    slParam2.setBounds(sliderRow.removeFromRight(150));
    sliderRow.reduce(20, 10);
    cbOrderSetting.setBounds(sliderRow);
    
}

void PluginTemplateAudioProcessorEditor::timerCallback()
{
    // timeCallback stuff
}
