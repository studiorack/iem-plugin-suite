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
EnergyVisualizerAudioProcessorEditor::EnergyVisualizerAudioProcessorEditor (EnergyVisualizerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (680, 400);
    setLookAndFeel (&globalLaF);
    
    
    addAndMakeVisible(&title);
    title.setTitle(String("Energy"),String("Visualizer"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    
    cbNormalizationAtachement = new ComboBoxAttachment(valueTreeState,"useSN3D", *title.getInputWidgetPtr()->getNormCbPointer());
    cbOrderAtachement = new ComboBoxAttachment(valueTreeState,"orderSetting", *title.getInputWidgetPtr()->getOrderCbPointer());
    
    
    
    
    addAndMakeVisible(&slPeakLevel);
    slPeakLevelAttachment = new SliderAttachment(valueTreeState, "peakLevel", slPeakLevel);
    slPeakLevel.setSliderStyle(Slider::LinearVertical);
    slPeakLevel.setTextBoxStyle(Slider::TextBoxBelow, false, 100, 12);
    slPeakLevel.setTextValueSuffix(" dB");
    slPeakLevel.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slPeakLevel.setReverse(false);
    slPeakLevel.addListener(this);
    
    addAndMakeVisible(&lbPeakLevel);
    lbPeakLevel.setText("Peak level");
    
    addAndMakeVisible(&visualizer);
    visualizer.setRmsDataPtr(&p.rms);
    
    addAndMakeVisible(&colormap);
    
    startTimer(20);
}

EnergyVisualizerAudioProcessorEditor::~EnergyVisualizerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void EnergyVisualizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void EnergyVisualizerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);

    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop    (headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    
    Rectangle<int> sliderCol = area.removeFromRight(50);
    sliderCol.reduce(0, 80);
    lbPeakLevel.setBounds(sliderCol.removeFromBottom(12));
    slPeakLevel.setBounds(sliderCol);
    
    area.removeFromRight(5);
    sliderCol = area.removeFromRight(25);
    sliderCol.reduce(0,40);
    colormap.setBounds(sliderCol);
    
    area.removeFromRight(5);
    visualizer.setBounds(area);
    
}
void EnergyVisualizerAudioProcessorEditor::sliderValueChanged (Slider *slider)
{
    colormap.setMaxLevel((float) slider->getValue());
}

void EnergyVisualizerAudioProcessorEditor::timerCallback()
{
    if (processor.maxPossibleOrder != maxPossibleOrder)
    {
        maxPossibleOrder = processor.maxPossibleOrder;
        title.getInputWidgetPtr()->updateOrderCb(maxPossibleOrder);
    }
}
