/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//Plugin Design Essentials
#include "../../resources/lookAndFeel/IEM_LaF.h"
#include "../../resources/customComponents/TitleBar.h"

//Custom Components
#include "../../resources/customComponents/ReverseSlider.h"
#include "../../resources/customComponents/SimpleLabel.h"


typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class PluginTemplateAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor&, AudioProcessorValueTreeState&);
    ~PluginTemplateAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginTemplateAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    void timerCallback() override;
    LaF globalLaF;
    TitleBar<AudioChannelsIOWidget<2,false>, NoIOWidget> title;
    Footer footer;

    ReverseSlider slParam1, slParam2;
    ComboBox cbOrderSetting;
    
    ScopedPointer<SliderAttachment> slParam1Attachment, slParam2Attachment;
    ScopedPointer<ComboBoxAttachment> cbOrderSettingAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessorEditor)
};
