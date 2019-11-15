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
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), footer (p.getOSCParameterInterface()), lv(processor.points, processor.triangles, processor.normals, processor.imaginaryFlags), lspList(processor.getLoudspeakersValueTree(), lv, grid, processor.undoManager, processor), grid(processor.points, processor.imaginaryFlags, processor.energyDistribution, processor.rEVector)
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits(1000, 600, 1200, 900); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("AllRA"),String("Decoder"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================


    // create the connection between title component's comboBoxes and parameters
    cbNormalizationSettingAttachment.reset (new ComboBoxAttachment (valueTreeState, "useSN3D", *title.getInputWidgetPtr()->getNormCbPointer()));
    cbOrderSettingAttachment.reset (new ComboBoxAttachment(valueTreeState, "inputOrderSetting", *title.getInputWidgetPtr()->getOrderCbPointer()));

    addAndMakeVisible (cbDecoderOrder);
    cbDecoderOrder.setJustificationType (Justification::centred);
    cbDecoderOrder.addSectionHeading ("Decoder order");
    for (int n = 1; n <= 7; ++n)
        cbDecoderOrder.addItem (getOrderString(n), n);
    cbDecoderOrderAttachment.reset (new ComboBoxAttachment (valueTreeState, "decoderOrder", cbDecoderOrder));

    addAndMakeVisible (lbDecoderOrder);
    lbDecoderOrder.setText ("Decoder Order", Justification::left);

    addAndMakeVisible (cbDecoderWeights);
    cbDecoderWeights.setJustificationType (Justification::centred);
    cbDecoderWeights.addItemList (p.weightsStrings, 1);
    cbDecoderWeightsAttachment.reset (new ComboBoxAttachment (valueTreeState, "weights", cbDecoderWeights));

    addAndMakeVisible (lbDecoderWeights);
    lbDecoderWeights.setText ("Weights", Justification::left);

    addAndMakeVisible(gcLayout);
    gcLayout.setText("Loudspeaker Layout");

    addAndMakeVisible(gcDecoder);
    gcDecoder.setText("Calculate Decoder");

    addAndMakeVisible(gcExport);
    gcExport.setText("Export Decoder/Layout");

    addAndMakeVisible(tbExportDecoder);
    tbExportDecoderAttachment.reset (new ButtonAttachment(valueTreeState, "exportDecoder", tbExportDecoder));
    tbExportDecoder.setButtonText("Export Decoder");
    tbExportDecoder.setColour(ToggleButton::tickColourId, Colours::orange);

    addAndMakeVisible(tbExportLayout);
    tbExportLayoutAttachment.reset (new ButtonAttachment(valueTreeState, "exportLayout", tbExportLayout));
    tbExportLayout.setButtonText("Export Layout");
    tbExportLayout.setColour(ToggleButton::tickColourId, Colours::limegreen);


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
    tbAddSpeakers.setTooltip ("Adds a new loudspeaker with random position. \n Alt+click: adds an imaginary loudspeaker to the nadir position.");
    tbAddSpeakers.addListener(this);

    addAndMakeVisible(tbJson);
    tbJson.setButtonText("EXPORT");
    tbJson.setColour(TextButton::buttonColourId, Colours::orange);
    tbJson.setTooltip ("Stores the decoder and/or loudspeaker layout to a configuration file.");
    tbJson.addListener(this);

    addAndMakeVisible(tbImport);
    tbImport.setButtonText("IMPORT");
    tbImport.setColour(TextButton::buttonColourId, Colours::orange);
    tbImport.setTooltip ("Imports loudspeakers from a configuration file.");
    tbImport.addListener(this);

    addAndMakeVisible(tbUndo);
    tbUndo.setButtonText("UNDO");
    tbUndo.setColour(TextButton::buttonColourId, Colours::orangered);
    tbUndo.onClick = [this] () { processor.undo(); };

    addAndMakeVisible(tbRedo);
    tbRedo.setButtonText("REDO");
    tbRedo.setColour(TextButton::buttonColourId, Colours::orangered);
    tbRedo.onClick = [this] () { processor.redo(); };

    addAndMakeVisible(tbRotate);
    tbRotate.setButtonText("ROTATE");
    tbRotate.setColour(TextButton::buttonColourId, Colours::cornflowerblue);
    tbRotate.setTooltip ("Rotates all loudspeakers by a desired amount around the z-axis.");
    tbRotate.onClick = [this] () { openRotateWindow(); };

    addAndMakeVisible(lv);

    addAndMakeVisible(lspList);

    updateChannelCount();

    // start timer after everything is set up properly
    startTimer(50);

    tooltipWin.setLookAndFeel (&globalLaF);
    tooltipWin.setMillisecondsBeforeTipAppears (500);
    tooltipWin.setOpaque (false);
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
    // the removeFrom...() methods are quite handy to create scalable areas
    // best practice would be the use of flexBoxes...
    // the following is medium level practice ;-)

    Rectangle<int> rightArea = area.removeFromRight(420);
    Rectangle<int> bottomRight = rightArea.removeFromBottom(100);

    rightArea.removeFromBottom(25);

    gcLayout.setBounds(rightArea);
    rightArea.removeFromTop(25);

    Rectangle<int> ctrlsAndDisplay (rightArea.removeFromBottom(80));
    Rectangle<int> lspCtrlArea (ctrlsAndDisplay.removeFromTop(20));
    ctrlsAndDisplay.removeFromTop(5);
    tbAddSpeakers.setBounds(lspCtrlArea.removeFromLeft(120));
    lspCtrlArea.removeFromLeft(5);
    tbRotate.setBounds(lspCtrlArea.removeFromLeft(55));
    lspCtrlArea.removeFromLeft(5);
    tbUndo.setBounds(lspCtrlArea.removeFromLeft(55));
    lspCtrlArea.removeFromLeft(5);
    tbRedo.setBounds(lspCtrlArea.removeFromLeft(55));
    lspCtrlArea.removeFromLeft(5);

    tbImport.setBounds(lspCtrlArea.removeFromRight(80));
    messageDisplay.setBounds(ctrlsAndDisplay);



    rightArea.removeFromBottom(5);
    lspList.setBounds(rightArea);

    Rectangle<int> decoderArea = bottomRight.removeFromLeft(150);
    bottomRight.removeFromLeft(20);
    Rectangle<int> exportArea = bottomRight;


    gcDecoder.setBounds (decoderArea);
    decoderArea.removeFromTop (25);
    auto decoderCtrlRow = decoderArea.removeFromTop (20);
    lbDecoderOrder.setBounds (decoderCtrlRow.removeFromLeft (80));
    cbDecoderOrder.setBounds (decoderCtrlRow.removeFromLeft (55));;
    decoderArea.removeFromTop (5);
    decoderCtrlRow = decoderArea.removeFromTop (20);
    lbDecoderWeights.setBounds (decoderCtrlRow.removeFromLeft (55));
    cbDecoderWeights.setBounds (decoderCtrlRow.removeFromLeft (80));;
    decoderArea.removeFromTop (5);
    tbCalculateDecoder.setBounds (decoderArea.removeFromTop (20));


    gcExport.setBounds(exportArea);
    exportArea.removeFromTop(25);
    Rectangle<int> toggleArea (exportArea.removeFromLeft(120));
    tbExportDecoder.setBounds(toggleArea.removeFromTop(20));
    tbExportLayout.setBounds(toggleArea.removeFromTop(20));
    exportArea.removeFromLeft(20);
    exportArea.removeFromTop(10);
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
    title.setMaxSize (processor.getMaxSize());
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

    if (processor.updateChannelCount.get())
    {
        processor.updateChannelCount = false;
        updateChannelCount();
    }
}



void AllRADecoderAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &tbAddSpeakers)
    {
        const auto& modifiers = ModifierKeys::getCurrentModifiers();
        if (modifiers.isAltDown())
            processor.addImaginaryLoudspeakerBelow();
        else
            processor.addRandomPoint();
    }
    else if (button == &tbCalculateDecoder)
    {
        processor.calculateDecoder();
    }
    else if (button == &tbJson)
    {
        FileChooser myChooser ("Save configuration...",
                               processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                               "*.json");
        if (myChooser.browseForFileToSave (true))
        {
            File configFile (myChooser.getResult());
            processor.setLastDir(configFile.getParentDirectory());
            processor.saveConfigurationToFile (configFile);
        }
    }
    else if (button == &tbImport)
    {
        FileChooser myChooser ("Load configuration...",
                               processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                               "*.json");
        if (myChooser.browseForFileToOpen())
        {
            File configFile (myChooser.getResult());
            processor.setLastDir(configFile.getParentDirectory());
            processor.loadConfiguration (configFile);
        }
    }
}

void AllRADecoderAudioProcessorEditor::updateChannelCount ()
{
    ReferenceCountedDecoder::Ptr currentDecoder = processor.getCurrentDecoder();
    if (currentDecoder != nullptr)
    {
        const int order = currentDecoder->getOrder();
        title.getInputWidgetPtr()->setMaxOrder(order);

        const int nCh = currentDecoder->getNumOutputChannels();
        title.getOutputWidgetPtr()->setSizeIfUnselectable(nCh);
    }
    else
    {
        title.getInputWidgetPtr()->setMaxOrder(0);
        title.getOutputWidgetPtr()->setSizeIfUnselectable(0);
    }

};

void AllRADecoderAudioProcessorEditor::buttonStateChanged (Button* button) {};

void AllRADecoderAudioProcessorEditor::openRotateWindow()
{
    auto* rotateWindow = new RotateWindow (processor);
    rotateWindow->setSize (120, 35);

    CallOutBox& myBox = CallOutBox::launchAsynchronously (rotateWindow, tbRotate.getScreenBounds(), nullptr);
    myBox.setLookAndFeel(&globalLaF);
}
