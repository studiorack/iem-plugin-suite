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
MatrixMultiplierAudioProcessorEditor::MatrixMultiplierAudioProcessorEditor (MatrixMultiplierAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), footer (p.getOSCParameterInterface())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    setResizeLimits(500, 200, 800, 500); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible(&title);
    title.setTitle(String("Matrix"),String("Multiplier"));
    title.setFont(globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================


    // create the connection between title component's comboBoxes and parameters
//    cbInputChannelsSettingAttachment = new ComboBoxAttachment(valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
//    cbOutputChannelsSettingAttachment = new ComboBoxAttachment(valueTreeState, "outputChannelsSetting", *title.getOutputWidgetPtr()->getChannelsCbPointer());

    addAndMakeVisible(btLoadFile);
    btLoadFile.setButtonText("Load configuration");
    btLoadFile.setColour(TextButton::buttonColourId, Colours::cornflowerblue);
    btLoadFile.addListener(this);

    addAndMakeVisible(edOutput);
    edOutput.setMultiLine(true);
    edOutput.setReadOnly(true);
    edOutput.setTabKeyUsedAsCharacter(true);
    edOutput.clear();
    edOutput.setText(processor.getMessageForEditor());
    edOutput.setColour(TextEditor::backgroundColourId, Colours::cornflowerblue.withMultipliedAlpha(0.2f));

    // start timer after everything is set up properly
    startTimer(20);
}

MatrixMultiplierAudioProcessorEditor::~MatrixMultiplierAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MatrixMultiplierAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MatrixMultiplierAudioProcessorEditor::resized()
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

void MatrixMultiplierAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    if (processor.messageChanged)
    {
        edOutput.clear();
        edOutput.setText(processor.getMessageForEditor());
        processor.messageChanged = false;
    }

    ReferenceCountedMatrix::Ptr currentMatrix = processor.getCurrentMatrix();
    if (currentMatrix != nullptr)
    {
        title.getOutputWidgetPtr()->setSizeIfUnselectable(currentMatrix->getNumOutputChannels());
        title.getInputWidgetPtr()->setSizeIfUnselectable(currentMatrix->getNumInputChannels());
    }
    else
    {
        title.getOutputWidgetPtr()->setSizeIfUnselectable (0);
        title.getInputWidgetPtr()->setSizeIfUnselectable (0);
    }
}

void MatrixMultiplierAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &btLoadFile)
    {
        loadConfigurationFile();
    }
}

void MatrixMultiplierAudioProcessorEditor::buttonStateChanged(juce::Button *button)
{

}

void MatrixMultiplierAudioProcessorEditor::loadConfigurationFile()
{
    FileChooser myChooser ("Please select the configuration you want to load...",
                           processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                           "*.json");
    if (myChooser.browseForFileToOpen())
    {
        File configurationFile (myChooser.getResult());
        processor.setLastDir(configurationFile.getParentDirectory());
        processor.loadConfiguration (configurationFile);

        edOutput.clear();
        edOutput.setText(processor.getMessageForEditor());
    }
}
