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
    
    
    void timerCallback() override;
    
private:
    // ====================== beging essentials ==================
    // stored references to the AudioProcessor and ValueTreeState holding all the parameters
    PluginTemplateAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    // lookAndFeel class with the IEM plug-in suite design
    LaF globalLaF;
    
    /* title and footer component
     title component can hold different widgets for in- and output:
        - NoIOWidget (if there's no need for an input or output widget)
        - AudioChannelsIOWidget<maxNumberOfChannels, isChoosable>
        - AmbisonicIOWidget
        - DirectivitiyIOWidget
     */
    TitleBar<AudioChannelsIOWidget<10,true>, AmbisonicIOWidget> title;
    Footer footer;
    // =============== end essentials ============
    
    // Attachments to create a connection between IOWidgets comboboxes
    // and the associated parameters
    ScopedPointer<ComboBoxAttachment> cbInputChannelsSettingAttachment;
    ScopedPointer<ComboBoxAttachment> cbOrderSettingAttachment;
    ScopedPointer<ComboBoxAttachment> cbNormalizationSettingAttachment;
    
    // Demo stuff
    ReverseSlider slParam1, slParam2;
    ScopedPointer<SliderAttachment> slParam1Attachment, slParam2Attachment;
    
    

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplateAudioProcessorEditor)
};
