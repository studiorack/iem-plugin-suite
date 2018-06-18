/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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
DistanceCompensatorAudioProcessorEditor::DistanceCompensatorAudioProcessorEditor (DistanceCompensatorAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    setLookAndFeel (&globalLaF);
    
    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("Distance"), String ("Compensator"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================
    
    
    // create the connection between title component's comboBoxes and parameters
    cbInputChannelsSettingAttachment = new ComboBoxAttachment (valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());
    
    addAndMakeVisible (lbSpeedOfSound);
    lbSpeedOfSound.setEditable (true);
    lbSpeedOfSound.setJustificationType(Justification::centred);
    lbSpeedOfSoundAttachment = new LabelAttachment (valueTreeState, "speedOfSound", lbSpeedOfSound);
    
    addAndMakeVisible (slbSpeedOfSound);
    slbSpeedOfSound.setText ("Speed of sound");
    slbSpeedOfSound.setJustification(Justification::left);
    
    addAndMakeVisible (lbDistanceExponent);
    lbDistanceExponent.setEditable (true);
    lbDistanceExponent.setJustificationType(Justification::centred);
    lbDistanceExponentAttachment = new LabelAttachment (valueTreeState, "distanceExponent", lbDistanceExponent);
    
    addAndMakeVisible (slbDistanceExponent);
    slbDistanceExponent.setText ("Distance-Gain exponent");
    slbDistanceExponent.setJustification (Justification::left);
    
    addAndMakeVisible (lbReferenceX);
    lbReferenceX.setEditable (true);
    lbReferenceX.setJustificationType(Justification::centred);
    lbReferenceXAttachment = new LabelAttachment (valueTreeState, "referenceX", lbReferenceX);
    
    addAndMakeVisible (lbReferenceY);
    lbReferenceY.setEditable (true);
    lbReferenceY.setJustificationType(Justification::centred);
    lbReferenceYAttachment = new LabelAttachment (valueTreeState, "referenceY", lbReferenceY);
    
    addAndMakeVisible (lbReferenceZ);
    lbReferenceZ.setEditable (true);
    lbReferenceZ.setJustificationType(Justification::centred);
    lbReferenceZAttachment = new LabelAttachment (valueTreeState, "referenceZ", lbReferenceZ);
    
    
    addAndMakeVisible (slbReference);
    slbReference.setText ("Reference listener position", true);
    slbReference.setJustification (Justification::left);
    
    addAndMakeVisible (slbReferenceX);
    slbReferenceX.setText ("x");
    slbReferenceX.setJustification (Justification::centred);
    
    addAndMakeVisible (slbReferenceY);
    slbReferenceY.setText ("y");
    slbReferenceY.setJustification (Justification::centred);
    
    addAndMakeVisible (slbReferenceZ);
    slbReferenceZ.setText ("z");
    slbReferenceZ.setJustification (Justification::centred);
    
    addAndMakeVisible (gcLayout);
    gcLayout.setText ("From loudspeaker layout");
    
    addAndMakeVisible(btLoadFile);
    btLoadFile.setButtonText("LOAD LAYOUT");
    btLoadFile.addListener(this);
    btLoadFile.setColour(TextButton::buttonColourId, Colours::orange);
    
    addAndMakeVisible(btReference);
    btReference.setButtonText("UPDATE REFERENCE");
    btReference.addListener(this);
    btReference.setColour(TextButton::buttonColourId, Colours::cornflowerblue);
    
    addAndMakeVisible (gcCompensation);
    gcCompensation.setText ("Settings");
    
    addAndMakeVisible(tbEnableGains);
    tbEnableGainsAttachment = new ButtonAttachment(valueTreeState, "enableGains", tbEnableGains);
    tbEnableGains.setButtonText("Gain compensation");
    tbEnableGains.setColour(ToggleButton::tickColourId, Colours::limegreen);
    
    addAndMakeVisible(tbEnableDelays);
    tbEnableDelaysAttachment = new ButtonAttachment(valueTreeState, "enableDelays", tbEnableDelays);
    tbEnableDelays.setButtonText("Delay compensation");
    tbEnableDelays.setColour(ToggleButton::tickColourId, Colours::orange);
    
    addAndMakeVisible (gcDistances);
    gcDistances.setText ("Loudspeaker Distances");
    
    for (int i = 0; i < 64; ++i)
    {
        {
            auto handle = tbEnableCompensation.add (new RoundButton());
            addAndMakeVisible (handle);
            handle->setColour (ToggleButton::tickColourId, Colours::cornflowerblue);
            handle->setButtonText ("C");
            handle->setTooltip("Enable compensation and \n factoring in the distance in gain/delay-calculation.");
            tbEnableCompensationAttachment.add (new ButtonAttachment (valueTreeState, "enableCompensation" + String(i), *handle));
        }
        
        
        auto handle = slDistance.add (new Label());
        addAndMakeVisible (handle);
        handle->setJustificationType(Justification::centred);
        handle->setEditable (true);
        handle->setExplicitFocusOrder(i + 1);
        slDistanceAttachment.add(new LabelAttachment (valueTreeState, "distance" + String(i), *handle));
        
        auto lbHandle = lbDistance.add(new SimpleLabel());
        addAndMakeVisible(lbHandle);
        lbHandle->setText(String(i + 1), true, Justification::right);
    }
    
    
    setResizeLimits (540, 620, 540, 620); // use this to create a resizable GUI
    toolTipWin.setMillisecondsBeforeTipAppears (500);
    // start timer after everything is set up properly
    startTimer (20);
}

DistanceCompensatorAudioProcessorEditor::~DistanceCompensatorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DistanceCompensatorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void DistanceCompensatorAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds (footerArea);
    
    area.removeFromLeft (leftRightMargin);
    area.removeFromRight (leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop (10);
    area.removeFromBottom (5);
    // =========== END: header and footer =================
    
    Rectangle<int> controls = area.removeFromTop (120);
    auto settingsArea = controls.removeFromLeft(170);
    gcCompensation.setBounds(settingsArea);
    settingsArea.removeFromTop(25);
    tbEnableGains.setBounds(settingsArea.removeFromTop(20));
    settingsArea.removeFromTop (3);
    
    auto rowDistanceGain (settingsArea.removeFromTop (18));
    rowDistanceGain.removeFromLeft(20);
    lbDistanceExponent.setBounds (rowDistanceGain.removeFromLeft (25));
    rowDistanceGain.removeFromLeft(5);
    slbDistanceExponent.setBounds (rowDistanceGain);
    settingsArea.removeFromTop (5);
    
    tbEnableDelays.setBounds(settingsArea.removeFromTop(20));
    settingsArea.removeFromTop (5);
    auto rowSpeed (settingsArea.removeFromTop (18));
    rowSpeed.removeFromLeft(20);
    lbSpeedOfSound.setBounds (rowSpeed.removeFromLeft (50));
    rowSpeed.removeFromLeft(5);
    slbSpeedOfSound.setBounds (rowSpeed);
    
    
    controls.removeFromLeft(20);
    
    gcLayout.setBounds (controls);
    controls.removeFromTop (25);
    auto buttonArea = controls.removeFromTop(21).removeFromRight(130);
    btLoadFile.setBounds(buttonArea);
    
    controls.removeFromTop (15);
    
    auto referenceArea = controls.removeFromTop (60);
    slbReference.setBounds (referenceArea.removeFromTop (18));
    referenceArea.removeFromTop (5);
    
    auto buttonAreaReference (referenceArea.removeFromRight(130));
    referenceArea.removeFromRight(5);
    btReference.setBounds(buttonAreaReference.removeFromTop(21));
    
    auto labelArea (referenceArea.removeFromTop(18));
    lbReferenceX.setBounds(labelArea.removeFromLeft (50));
    labelArea.removeFromLeft(5);
    lbReferenceY.setBounds(labelArea.removeFromLeft (50));
    labelArea.removeFromLeft(5);
    lbReferenceZ.setBounds(labelArea.removeFromLeft (50));
    labelArea = referenceArea.removeFromTop (12);
    slbReferenceX.setBounds(labelArea.removeFromLeft (50));
    labelArea.removeFromLeft(5);
    slbReferenceY.setBounds(labelArea.removeFromLeft (50));
    labelArea.removeFromLeft(5);
    slbReferenceZ.setBounds(labelArea.removeFromLeft (50));
    
    
    area.removeFromTop (10);
    gcDistances.setBounds (area.removeFromTop(25));
    
    Rectangle<int> sliderCol;
    
    for (int i = 0; i < 64; ++i)
    {
        if (i % 16 == 0)
            sliderCol = area.removeFromLeft(100);
        else if (i % 8 == 0)
            sliderCol.removeFromTop(15);
        
        auto sliderRow = sliderCol.removeFromTop(18);
        lbDistance.getUnchecked(i)->setBounds (sliderRow.removeFromLeft (20));
        sliderRow.removeFromLeft (8);
        tbEnableCompensation.getUnchecked(i)->setBounds (sliderRow.removeFromLeft (18));
        sliderRow.removeFromLeft (2);
        slDistance.getUnchecked (i)->setBounds (sliderRow);
        
        sliderCol.removeFromTop(2);
        
        if ((i - 1) % 16 == 0)
            area.removeFromLeft(20);
        
    }
}

void DistanceCompensatorAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize (maxInSize, maxOutSize);
    title.setMaxSize (maxInSize, maxOutSize);
    // ==========================================
    
    // insert stuff you want to do be done at every timer callback
}


void DistanceCompensatorAudioProcessorEditor::buttonClicked(Button* button)
{
    if (button == &btLoadFile)
    {
        FileChooser myChooser ("Load loudspeaker layout...",
                               processor.getLastDir().exists() ? processor.getLastDir() : File::getSpecialLocation (File::userHomeDirectory),
                               "*.json");
        if (myChooser.browseForFileToOpen())
        {
            File configFile (myChooser.getResult());
            processor.setLastDir(configFile.getParentDirectory());
            processor.loadConfiguration (configFile);
        }
    }
    else if (button == &btReference)
    {
        processor.updateParameters();
    }
}

void DistanceCompensatorAudioProcessorEditor::buttonStateChanged(juce::Button *button)
{
    
}
