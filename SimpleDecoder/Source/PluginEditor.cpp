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
SimpleDecoderAudioProcessorEditor::SimpleDecoderAudioProcessorEditor (SimpleDecoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), fv(20, 20000, -20, 10, 5)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits(670, 280, 800, 700); // use this to create a resizeable GUI
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
    
    addAndMakeVisible(gcLfe);
    gcLfe.setText("LFE");
    
    addAndMakeVisible(gcConfiguration);
    gcConfiguration.setText("Decoder Configuration");
    
    
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
    cbLfeMode.setJustificationType(Justification::centred);
    cbLfeMode.addItem("none", 1);
    cbLfeMode.addItem("discrete", 2);
    cbLfeMode.addItem("virtual", 3);
    cbLfeModeAttachment = new ComboBoxAttachment(valueTreeState, "lfeMode", cbLfeMode);
    
    addAndMakeVisible(lbLfeMode);
    lbLfeMode.setText("LFE mode");
    
    addAndMakeVisible(lbLfeChannel);
    lbLfeChannel.setText("LFE Channel");
    
    addAndMakeVisible(slLfeChannel);
    slLfeChannelAttachment = new SliderAttachment(valueTreeState, "lfeChannel", slLfeChannel);
    slLfeChannel.setSliderStyle(Slider::IncDecButtons);
    slLfeChannel.setTextBoxStyle (Slider::TextBoxLeft, false, 200, 20);
    
    // ================= END: LFE mode =======================
    
    addAndMakeVisible(btLoadFile);
    btLoadFile.setButtonText("Load configuration");
    btLoadFile.addListener(this);
    btLoadFile.setColour(TextButton::buttonColourId, Colours::steelblue); //globalLaF.ClWidgetColours[0]);
    
    dcInfoBox.setErrorMessage(processor.getMessageForEditor());
    
    addAndMakeVisible(dcInfoBox);
    dcInfoBox.setDecoderConfig(processor.getCurrentDecoderConfig());
    
    addAndMakeVisible(fv);
    fv.setParallel(true);
    fv.addCoefficients(&processor.lowPassCoefficients, Colours::orangered, &slLowPassFrequency, &slLowPassGain);
    fv.addCoefficients(&processor.highPassCoefficients, Colours::cyan, &slHighPassFrequency);
    
    // start timer after everything is set up properly
    startTimer(20);
}

SimpleDecoderAudioProcessorEditor::~SimpleDecoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleDecoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void SimpleDecoderAudioProcessorEditor::resized()
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
    const int extraMargin = 6;
    
    const int width = 0.5f * (area.getWidth() - 10);
    
    
    Rectangle<int> leftSide (area.removeFromLeft(280));
    area.removeFromLeft(20);
    Rectangle<int> rightSide (area.removeFromRight(100));
    rightSide.removeFromTop(extraMargin);
    area.removeFromRight(20);
    
    { //====================== CONFIGURATION GROUP ==================================
        Rectangle<int> configArea(leftSide);
        Rectangle<int> buttonArea = configArea;
        buttonArea = buttonArea.removeFromRight(130).removeFromTop(21);
        btLoadFile.setBounds(buttonArea);
        configArea.removeFromTop(extraMargin);
        gcConfiguration.setBounds(configArea);
        configArea.removeFromTop(25);
        
        configArea.removeFromTop(5);
        
        dcInfoBox.setBounds(configArea);
    }
    

    { //====================== LFE GROUP ==================================
        Rectangle<int> lfeArea(rightSide);
        gcLfe.setBounds(lfeArea);
        lfeArea.removeFromTop(25);
        
        cbLfeMode.setBounds(lfeArea.removeFromTop(20));
        lbLfeMode.setBounds(lfeArea.removeFromTop(labelHeight));

        lfeArea.removeFromTop(10);

        slLfeChannel.setBounds(lfeArea.removeFromTop(20));
        lbLfeChannel.setBounds(lfeArea.removeFromTop(labelHeight));
    }
    
    { //====================== FILTER GROUP ==================================
        Rectangle<int> filterArea(area);
        filterArea.removeFromTop(extraMargin);
        gcFilter.setBounds(filterArea);
        filterArea.removeFromTop(25);
        
        const int rotSliderWidth = 50;
        
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

}

void SimpleDecoderAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================
    
    if (processor.messageChanged)
    {
        dcInfoBox.setErrorMessage(processor.getMessageForEditor());
        processor.messageChanged = false;
    }
    
    ReferenceCountedDecoder::Ptr currentDecoder = processor.getCurrentDecoderConfig();
    if (lastDecoder != currentDecoder)
    {
        lastDecoder = currentDecoder;
        if (lastDecoder != nullptr)
        {
            const int lfeMode = *valueTreeState.getRawParameterValue("lfeMode");
            int neededChannels = 0;
            if (lfeMode == 1)
                neededChannels = jmax(currentDecoder->getNumOutputChannels(), (int) *valueTreeState.getRawParameterValue("lfeChannel"));
            else
                neededChannels = currentDecoder->getNumOutputChannels();
            
            title.getInputWidgetPtr()->setMaxOrder(currentDecoder->getOrder());
            title.getOutputWidgetPtr()->setSizeIfUnselectable(neededChannels);
        }
        else
        {
            title.getInputWidgetPtr()->setMaxOrder(0);
            title.getOutputWidgetPtr()->setSizeIfUnselectable(0);   
        }
    }
   
    
    if (processor.updateFv)
    {
        fv.repaint();
        processor.updateFv = false;
    }
    
}

void SimpleDecoderAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &btLoadFile)
    {
        loadPresetFile();
    }
}

void SimpleDecoderAudioProcessorEditor::buttonStateChanged(juce::Button *button)
{
    
}

void SimpleDecoderAudioProcessorEditor::loadPresetFile()
{
    FileChooser myChooser ("Please select the preset you want to load...",
                           processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToOpen())
    {
        File presetFile (myChooser.getResult());
        processor.setLastDir(presetFile.getParentDirectory());
        processor.loadPreset (presetFile);
        
        dcInfoBox.setDecoderConfig(processor.getCurrentDecoderConfig());
    }
    
}
