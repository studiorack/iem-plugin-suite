/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
DecoderAudioProcessorEditor::DecoderAudioProcessorEditor (DecoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), fv(20, 20000, -20, 10, 5)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits(700, 280, 800, 700); // use this to create a resizeable GUI
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("Simple"),String("Decoder"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputOrderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    cbNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    //cbOutputChannelsSettingAttachment = new ComboBoxAttachment(valueTreeState, "outputChannelsSetting", *title.getOutputWidgetPtr()->getChannelsCbPointer());
    
    addAndMakeVisible(gcFilter);
    gcFilter.setText("Frequency Bands");
    
    addAndMakeVisible(gcSettings);
    gcSettings.setText("Settings");
    
    addAndMakeVisible(gcConfiguration);
    gcConfiguration.setText("Configuration");
    
    
    // ================= BEGIN: filter slider ================
    addAndMakeVisible(slLowPassFrequency);
    slLowPassFrequencyAttachment = new SliderAttachment(valueTreeState, "lowPassFrequency", slLowPassFrequency);
    slLowPassFrequency.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slLowPassFrequency.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slLowPassFrequency.setColour (Slider::rotarySliderOutlineColourId, Colours::orangered);
    addAndMakeVisible(lbLowPassFrequency);
    lbLowPassFrequency.setText("Frequency");
    
    addAndMakeVisible(slLowPassGain);
    slLowPassGainAttachment = new SliderAttachment(valueTreeState, "lowPassGain", slLowPassGain);
    slLowPassGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slLowPassGain.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slLowPassGain.setColour (Slider::rotarySliderOutlineColourId, Colours::orangered);
    addAndMakeVisible(lbLowPassGain);
    lbLowPassGain.setText("Gain");
    
    addAndMakeVisible(slHighPassFrequency);
    slHighPassFrequencyAttachment = new SliderAttachment(valueTreeState, "highPassFrequency", slHighPassFrequency);
    slHighPassFrequency.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slHighPassFrequency.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slHighPassFrequency.setColour (Slider::rotarySliderOutlineColourId, Colours::cyan);
    addAndMakeVisible(lbHighPassFrequency);
    lbHighPassFrequency.setText("Frequency");
    // ================= END: filter slider ==================
    
    // ================= BEGIN: LFE mode =====================
    addAndMakeVisible(cbLfeMode);
    cbLfeMode.addItem("none", 1);
    cbLfeMode.addItem("append", 2);
    cbLfeMode.addItem("virtual", 3);
    cbLfeModeAttachment = new ComboBoxAttachment(valueTreeState, "lfeMode", cbLfeMode);
    
    addAndMakeVisible(lbLfeMode);
    lbLfeMode.setText("LFE mode");
    // ================= END: LFE mode =======================
    
    addAndMakeVisible(btLoadFile);
    btLoadFile.setButtonText("Load preset");
    btLoadFile.addListener(this);
    
    addAndMakeVisible(edOutput);
    edOutput.setMultiLine(true);
    edOutput.setReadOnly(true);
    edOutput.setTabKeyUsedAsCharacter(true);
    edOutput.clear();
    edOutput.setText(processor.getMessageForEditor());
    
    addAndMakeVisible(fv);
    fv.setParallel(true);
    fv.addCoefficients(&processor.lowPassCoefficients, Colours::orangered, &slLowPassFrequency, &slLowPassGain);
    fv.addCoefficients(&processor.highPassCoefficients, Colours::cyan, &slHighPassFrequency);
    
    // start timer after everything is set up properly
    startTimer(20);
}

DecoderAudioProcessorEditor::~DecoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DecoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void DecoderAudioProcessorEditor::resized()
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
    
    
    //const int sliderHeight = 15;
    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 10;
    //const int sliderSpacing = 3;
    const int rotSliderWidth = 40;
    const int labelHeight = 20;

    { //====================== FILTER GROUP ==================================
        Rectangle<int> filterArea(area.removeFromLeft(240));
        gcFilter.setBounds(filterArea);
        
        const int rotSliderWidth = 60;
        
        filterArea.removeFromTop(25);
        
        Rectangle<int> sliderRow(filterArea.removeFromBottom(labelHeight));
        lbLowPassFrequency.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbLowPassGain.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        
        
        lbHighPassFrequency.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        
        sliderRow = filterArea.removeFromBottom(rotSliderHeight-10);
        
        
        slLowPassFrequency.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slLowPassGain.setBounds (sliderRow.removeFromLeft(rotSliderWidth));
        
        
        slHighPassFrequency.setBounds (sliderRow.removeFromRight(rotSliderWidth));
        
        fv.setBounds(filterArea);
    }
    
    area.removeFromLeft(20);
    
    Rectangle<int> loadArea (area);

    Rectangle<int> loadColumn (loadArea.removeFromRight(120));
    btLoadFile.setBounds(loadColumn.removeFromTop(30));
    
    loadArea.removeFromRight(10);
    edOutput.setBounds(loadArea);
    

    cbLfeMode.setBounds(200, 40, 100, 40);

}

void DecoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================
    
    if (processor.messageChanged)
    {
        edOutput.clear();
        edOutput.setText(processor.getMessageForEditor());
        processor.messageChanged = false;
    }
    
    ReferenceCountedDecoder::Ptr currentDecoder = processor.getCurrentDecoderConfig();
    if (currentDecoder != nullptr)
    {
        int lfeMode = *valueTreeState.getRawParameterValue("lfeMode");
        title.getOutputWidgetPtr()->setSizeIfUnselectable(currentDecoder->getNumOutputChannels() + lfeMode == 2 ? 1 : 0);
    }
    
    if (processor.updateFv)
    {
        fv.repaint();
        processor.updateFv = false;
    }
    
}

void DecoderAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &btLoadFile)
    {
        loadPresetFile();
    }
}

void DecoderAudioProcessorEditor::buttonStateChanged(juce::Button *button)
{
    
}

void DecoderAudioProcessorEditor::loadPresetFile()
{
    FileChooser myChooser ("Please select the preset you want to load...",
                           processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToOpen())
    {
        File presetFile (myChooser.getResult());
        processor.setLastDir(presetFile.getParentDirectory());
        processor.loadPreset (presetFile);
        
        edOutput.clear();
        edOutput.setText(processor.getMessageForEditor());
    }
}
