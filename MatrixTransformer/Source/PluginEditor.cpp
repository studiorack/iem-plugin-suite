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
MatrixTransformerAudioProcessorEditor::MatrixTransformerAudioProcessorEditor (MatrixTransformerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits(500, 200, 800, 500); // use this to create a resizeable GUI
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("Matrix"),String("Transformer"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbInputChannelsSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
    cbOutputChannelsSettingAttachment = new ComboBoxAttachment(valueTreeState, "outputChannelsSetting", *title.getOutputWidgetPtr()->getChannelsCbPointer());
    
    addAndMakeVisible(btLoadFile);
    btLoadFile.setButtonText("Load preset");
    btLoadFile.addListener(this);
    
    addAndMakeVisible(edOutput);
    edOutput.setMultiLine(true);
    edOutput.setReadOnly(true);
    edOutput.setTabKeyUsedAsCharacter(true);
    edOutput.clear();
    edOutput.setText(processor.getMessageForEditor());
    
    // start timer after everything is set up properly
    startTimer(20);
}

MatrixTransformerAudioProcessorEditor::~MatrixTransformerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MatrixTransformerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MatrixTransformerAudioProcessorEditor::resized()
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
    

    
    Rectangle<int> sliderRow = area.removeFromRight(120);
    btLoadFile.setBounds(sliderRow.removeFromTop(30));
    
    area.removeFromRight(10);
    edOutput.setBounds(area);
}

void MatrixTransformerAudioProcessorEditor::timerCallback()
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
}

void MatrixTransformerAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &btLoadFile)
    {
        loadPresetFile();
    }
}

void MatrixTransformerAudioProcessorEditor::buttonStateChanged(juce::Button *button)
{

}

void MatrixTransformerAudioProcessorEditor::loadPresetFile()
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

