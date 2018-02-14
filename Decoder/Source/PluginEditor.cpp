/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at
 
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
PluginTemplateAudioProcessorEditor::PluginTemplateAudioProcessorEditor (PluginTemplateAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), lv(processor.points, processor.triangles, processor.normals)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits(600, 400, 1200, 900); // use this to create a resizeable GUI
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("Decoder"),String("Ding"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputOrderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    
    
    addAndMakeVisible(tbPrintJSON);
    tbPrintJSON.setButtonText("print JSON");
    tbPrintJSON.addListener(this);
    
    addAndMakeVisible(tbAddSpeakers);
    tbAddSpeakers.setButtonText("add speakers");
    tbAddSpeakers.addListener(this);
    
    addAndMakeVisible(lv);
    
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

    tbAddSpeakers.setBounds(20, 80, 100, 20);
    tbPrintJSON.setBounds(150, 80, 100, 20);
    
    lv.setBounds(area);
}

void PluginTemplateAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize(maxInSize, maxOutSize);
    title.setMaxSize(maxInSize, maxOutSize);
    // ==========================================
    
    
    if (processor.updateLoudspeakerVisualization.get())
    {
        processor.updateLoudspeakerVisualization = false;
        lv.updateVerticesAndIndices();
    }
    // insert stuff you want to do be done at every timer callback
}



void PluginTemplateAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &tbAddSpeakers)
    {
        processor.addRandomPoint();
    }
    else if (button == &tbPrintJSON)
    {
        DBG(JSON::toString(processor.lsps));
    }
}

void PluginTemplateAudioProcessorEditor::buttonStateChanged (Button* button) {};
