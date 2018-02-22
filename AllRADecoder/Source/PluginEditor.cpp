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
    
    
    addAndMakeVisible(grid);
    
    addAndMakeVisible(tbCalculateDecoder);
    tbCalculateDecoder.setButtonText("calculate Decoder");
    tbCalculateDecoder.addListener(this);
    
    addAndMakeVisible(tbAddSpeakers);
    tbAddSpeakers.setButtonText("add loudspeaker");
    tbAddSpeakers.addListener(this);
    
    addAndMakeVisible(tbJson);
    tbJson.setButtonText("print JSON");
    tbJson.addListener(this);
    
    addAndMakeVisible(tbUndo);
    tbUndo.setButtonText("undo");
    tbUndo.onClick = [this] () { processor.undo(); };
    
    addAndMakeVisible(tbRedo);
    tbRedo.setButtonText("redo");
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
    Rectangle<int> sliderRow = area.removeFromTop(50);

    tbAddSpeakers.setBounds(20, 80, 100, 20);
    tbCalculateDecoder.setBounds(150, 80, 100, 20);
    tbUndo.setBounds(280, 80, 100, 20);
    tbRedo.setBounds(410, 80, 100, 20);
    tbJson.setBounds(540, 80, 100, 20);
    cbDecoderOrder.setBounds(670, 80, 100, 20);
    
    Rectangle<int> leftArea = area.removeFromLeft(area.getWidth() / 2);
    lv.setBounds(leftArea);
    Rectangle<int> rightTopArea = area.removeFromTop(200);
    
    lspList.setBounds(rightTopArea);
    grid.setBounds(area);
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
