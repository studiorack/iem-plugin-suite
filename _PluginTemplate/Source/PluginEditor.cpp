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
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits(500, 300, 800, 500); // use this to create a resizeable GUI
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("Plugin"),String("Template"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbInputChannelsSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
    cbNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer());
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "outputOrderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
    
    addAndMakeVisible(slParam1);
    slParam1Attachment = new SliderAttachment(valueTreeState, "param1", slParam1);
    addAndMakeVisible(slParam2);
    slParam2Attachment = new SliderAttachment(valueTreeState, "param2", slParam2);
    
    
    // start timer after everything is set up properly
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
    // ============ BEGIN: header and footer ============
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
    // =========== END: header and footer =================
    
    
    // try to not use explicit coordinates to position your GUI components
    // the removeFrom...() methods are quite handy to create scaleable areas
    // best practice would be the use of flexBoxes...
    // the following is medium level practice ;-)
    Rectangle<int> sliderRow = area.removeFromTop(50);
    slParam1.setBounds(sliderRow.removeFromLeft(150));
    slParam2.setBounds(sliderRow.removeFromRight(150));
    
}

void PluginTemplateAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================
    
    // insert stuff you want to do be done at every timer callback
}
