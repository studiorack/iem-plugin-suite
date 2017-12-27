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
    setResizeLimits(500, 300, 800, 500);
    setLookAndFeel (&globalLaF);
    
    addAndMakeVisible(&title);
    title.setTitle(String("Plugin"),String("Template"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    
    cbNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer());
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "orderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
    
    
    startTimer(20);
}

PluginTemplateAudioProcessorEditor::~PluginTemplateAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void PluginTemplateAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void PluginTemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom(footerHeight));
    footer.setBounds(footerArea);

    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    area.removeFromBottom(5);
    
    
    
    Rectangle<int> sliderRow = area.removeFromTop(50);
    slParam1.setBounds(sliderRow.removeFromLeft(150));
    slParam2.setBounds(sliderRow.removeFromRight(150));
    sliderRow.reduce(20, 10);
    cbOrderSetting.setBounds(sliderRow);
    
}

void PluginTemplateAudioProcessorEditor::timerCallback()
{
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    
}
