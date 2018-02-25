/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich, Franz Zotter
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
AllRADecoderAudioProcessorEditor::AllRADecoderAudioProcessorEditor (AllRADecoderAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), lv(processor.points, processor.triangles, processor.normals, processor.imaginaryFlags), lspList(processor.getLoudspeakersValueTree(), lv, grid, processor.undoManager), grid(processor.points, processor.imaginaryFlags, processor.energyDistribution)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits(1000, 600, 1200, 900); // use this to create a resizeable GUI
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("AllRA"),String("Decoder"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbNormalizationSettingAttachment = new ComboBoxAttachment(valueTreeState, "useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputOrderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    
    addAndMakeVisible(cbDecoderOrder);
    cbDecoderOrder.addSectionHeading("Decoder order");
    for (int n = 1; n <= 7; ++n)
        cbDecoderOrder.addItem(getOrderString(n), n);
    cbDecoderOrderAttachment = new ComboBoxAttachment(valueTreeState, "decoderOrder", cbDecoderOrder);
    
    addAndMakeVisible(gcLayout);
    gcLayout.setText("Loudspeaker Layout");
    
    addAndMakeVisible(gcDecoder);
    gcDecoder.setText("Calculate Decoder");
    
    addAndMakeVisible(gcExport);
    gcExport.setText("Export Decoder/Layout");
    
    addAndMakeVisible(lbDecoderOrder);
    lbDecoderOrder.setText("Decoder Order");
    
    addAndMakeVisible(messageDisplay);
    messageDisplay.setMessage(processor.messageToEditor);
    
    addAndMakeVisible(grid);
    
    addAndMakeVisible(tbCalculateDecoder);
    tbCalculateDecoder.setButtonText("CALCULATE DECODER");
    tbCalculateDecoder.setColour(TextButton::buttonColourId, Colours::cornflowerblue);
    tbCalculateDecoder.addListener(this);
    
    addAndMakeVisible(tbAddSpeakers);
    tbAddSpeakers.setButtonText("ADD LOUDSPEAKER");
    tbAddSpeakers.setColour(TextButton::buttonColourId, Colours::limegreen);
    tbAddSpeakers.addListener(this);
    
    addAndMakeVisible(tbJson);
    tbJson.setButtonText("EXPORT");
    tbJson.setColour(TextButton::buttonColourId, Colours::orange);
    tbJson.addListener(this);
    
    addAndMakeVisible(tbUndo);
    tbUndo.setButtonText("UNDO");
    tbUndo.setColour(TextButton::buttonColourId, Colours::orangered);
    tbUndo.onClick = [this] () { processor.undo(); };
    
    addAndMakeVisible(tbRedo);
    tbRedo.setButtonText("REDO");
    tbRedo.setColour(TextButton::buttonColourId, Colours::orangered);
    tbRedo.onClick = [this] () { processor.redo(); };
    
    addAndMakeVisible(lv);
    
    addAndMakeVisible(lspList);
    
    // start timer after everything is set up properly
    startTimer(50);
}

AllRADecoderAudioProcessorEditor::~AllRADecoderAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void AllRADecoderAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void AllRADecoderAudioProcessorEditor::resized()
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
    
    Rectangle<int> rightArea = area.removeFromRight(400);
    Rectangle<int> bottomRight = rightArea.removeFromBottom(100);
    
    rightArea.removeFromBottom(25);
    
    gcLayout.setBounds(rightArea);
    rightArea.removeFromTop(25);
    
    Rectangle<int> ctrlsAndDisplay (rightArea.removeFromBottom(45));
    Rectangle<int> lspCtrlArea (ctrlsAndDisplay.removeFromLeft(120));
    ctrlsAndDisplay.removeFromLeft(10);
    messageDisplay.setBounds(ctrlsAndDisplay);
    
    tbAddSpeakers.setBounds(lspCtrlArea.removeFromTop(20));
    lspCtrlArea.removeFromTop(5);
    tbUndo.setBounds(lspCtrlArea.removeFromLeft(55));
    tbRedo.setBounds(lspCtrlArea.removeFromRight(55));
    
    rightArea.removeFromBottom(5);
    lspList.setBounds(rightArea);

    Rectangle<int> decoderArea = bottomRight.removeFromLeft(130);
    bottomRight.removeFromLeft(20);
    Rectangle<int> exportArea = bottomRight;
    
    
    gcDecoder.setBounds(decoderArea);
    decoderArea.removeFromTop(25);
    Rectangle<int> decoderCtrlRow = decoderArea.removeFromTop(20);
    lbDecoderOrder.setBounds(decoderCtrlRow.removeFromLeft(80));
    cbDecoderOrder.setBounds(decoderCtrlRow.removeFromLeft(50));;
    decoderArea.removeFromTop(5);
    tbCalculateDecoder.setBounds(decoderArea.removeFromTop(20));
    
    
    gcExport.setBounds(exportArea);
    exportArea.removeFromTop(25);
    tbJson.setBounds(exportArea.removeFromTop(20).removeFromLeft(80));

    area.removeFromRight(20);
    Rectangle<int> leftArea = area;
    
    grid.setBounds(leftArea.removeFromBottom(200));
    leftArea.removeFromBottom(10);
    lv.setBounds(leftArea);

}

void AllRADecoderAudioProcessorEditor::timerCallback()
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
        grid.repaint();
    }
    
    if (processor.updateTable.get())
    {
        processor.updateTable = false;
        lspList.updateContent();
    }
    
    if (processor.updateMessage.get())
    {
        processor.updateMessage = false;
        messageDisplay.setMessage(processor.messageToEditor);
    }
}



void AllRADecoderAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &tbAddSpeakers)
    {
        processor.addRandomPoint();
    }
    else if (button == &tbCalculateDecoder)
    {
        processor.calculateDecoder();
    }
    else if (button == &tbJson)
    {
        FileChooser myChooser ("Please select the preset you want to load...",
                               processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                               "*.json");
        if (myChooser.browseForFileToSave (true))
        {
            File presetFile (myChooser.getResult());
            processor.setLastDir(presetFile.getParentDirectory());
            processor.saveConfigurationToFile (presetFile);
        }
        
    }
}

void AllRADecoderAudioProcessorEditor::buttonStateChanged (Button* button) {};
