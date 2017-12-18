/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
DirectivityShaperAudioProcessorEditor::DirectivityShaperAudioProcessorEditor (DirectivityShaperAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts), fv(20.0f, 20000.0f, -50.0f, 10.0f, 10.0f)
{
    setSize (900, 710);
    setLookAndFeel (&globalLaF);
    
    for (int b = 0; b <numberOfBands; ++b)
    {
        weights[b][0] = 1.0f;
        weights[b][1] = 7.7777777777777779e-01;
        weights[b][2] = 4.6666666666666667e-01;
        weights[b][3] = 2.1212121212121213e-01;
        weights[b][4] = 7.0707070707070704e-02;
        weights[b][5] = 1.6317016317016316e-02;
        weights[b][6] = 2.3310023310023310e-03;
        weights[b][7] = 1.5540015540015540e-04;
    }
    
    addAndMakeVisible(&title);
    title.setTitle(String("Directivity"),String("Shaper"));
    title.setFont(globalLaF.robotoBold,globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    
    Colour colours[4] = {
        Colours::cadetblue,
        Colours::mediumpurple,
        Colours::greenyellow,
        Colours::orangered,
    };
    
    
    addAndMakeVisible(&cbNormalization);
    cbNormalization.addItem("basic decode", 1);
    cbNormalization.addItem("on-axis", 2);
    cbNormalization.addItem("constant energy", 3);
    cbNormalizationAttachment = new ComboBoxAttachment(valueTreeState, "normalization", cbNormalization);
    
    
    addAndMakeVisible(&fv);
    fv.setSampleRate(48000); //TODO
    for (int i = 0; i < numberOfBands; ++i)
        fv.addCoefficients(&processor.filter[i].coefficients, colours[i], &slFilterFrequency[i], &slFilterGain[i], &slFilterQ[i], &processor.probeGains[i]);
    fv.setParallel(true);
    
    addAndMakeVisible(&xyPad);
    for (int i = 0; i < numberOfBands; ++i)
        xyPad.addElement(slShape[i], slOrder[i], colours[i]);
    
    addAndMakeVisible(&dv);
    for (int i = 0; i < numberOfBands; ++i)
        dv.addElement(weights[i], colours[i]);
    
    
    
    cbOrderSettingAttachment = new ComboBoxAttachment(valueTreeState, "orderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer());
    
    addAndMakeVisible(&slMasterYaw);
    slMasterYaw.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slMasterYaw.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
    slMasterYaw.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
    slMasterYaw.setReverse(true);
    slMasterYaw.setRotaryParameters(M_PI, 3*M_PI, false);
    slMasterYaw.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    slMasterYawAttachment = new SliderAttachment(valueTreeState, "masterYaw", slMasterYaw);
    
    addAndMakeVisible(&slMasterPitch);
    slMasterPitch.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slMasterPitch.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
    slMasterPitch.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
    slMasterPitch.setReverse(true);
    slMasterPitch.setRotaryParameters(M_PI, 3*M_PI, false);
    slMasterPitch.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    slMasterPitchAttachment = new SliderAttachment(valueTreeState, "masterPitch", slMasterPitch);
    
    addAndMakeVisible(&slMasterRoll);
    slMasterRoll.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slMasterRoll.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
    slMasterRoll.setColour(Slider::rotarySliderOutlineColourId, Colours::black);
    slMasterRoll.setRotaryParameters(M_PI, 3*M_PI, false);
    slMasterRoll.setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
    slMasterRollAttachment = new SliderAttachment(valueTreeState, "masterRoll", slMasterRoll);
    
    addAndMakeVisible(&tbMasterToggle);
    tbMasterToggle.setButtonText("Lock Directions");
    tbMasterToggleAttachment = new ButtonAttachment(valueTreeState, "masterToggle", tbMasterToggle);
    
    
    for (int i = 0; i < numberOfBands; ++i)
    {
        addAndMakeVisible(&cbFilterType[i]);
        cbFilterType[i].addItem("All-pass", 1);
        cbFilterType[i].addItem("Low-pass", 2);
        cbFilterType[i].addItem("Band-pass", 3);
        cbFilterType[i].addItem("High-pass", 4);
        cbFilterType[i].setJustificationType(Justification::centred);
        cbFilterTypeAttachment[i] = new ComboBoxAttachment(valueTreeState, "filterType" + String(i), cbFilterType[i]);
        
        addAndMakeVisible(&slFilterFrequency[i]);
        slFilterFrequency[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slFilterFrequency[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slFilterFrequency[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterFrequency[i].setTextValueSuffix(" Hz");
        slFilterFrequencyAttachment[i] = new SliderAttachment(valueTreeState, "filterFrequency" + String(i), slFilterFrequency[i]);
        
        addAndMakeVisible(&slFilterQ[i]);
        slFilterQ[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slFilterQ[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slFilterQ[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterQAttachment[i] = new SliderAttachment(valueTreeState, "filterQ" + String(i), slFilterQ[i]);
        
        addAndMakeVisible(&slFilterGain[i]);
        slFilterGain[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slFilterGain[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slFilterGain[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterGain[i].setTextValueSuffix(" dB");
        slFilterGainAttachment[i] = new SliderAttachment(valueTreeState, "filterGain" + String(i), slFilterGain[i]);
        
        addAndMakeVisible(&slOrder[i]);
        slOrder[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slOrder[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slOrder[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slOrderAttachment[i] = new SliderAttachment(valueTreeState, "order" + String(i), slOrder[i]);
        
        addAndMakeVisible(&slShape[i]);
        slShape[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slShape[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slShape[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slShapeAttachment[i] = new SliderAttachment(valueTreeState, "shape" + String(i), slShape[i]);
        
        addAndMakeVisible(&slYaw[i]);
        slYaw[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slYaw[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slYaw[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slYaw[i].setReverse(true);
        slYaw[i].setRotaryParameters(M_PI, 3*M_PI, false);
        slYaw[i].setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
        slYawAttachment[i] = new SliderAttachment(valueTreeState, "yaw" + String(i), slYaw[i]);
        
        addAndMakeVisible(&slPitch[i]);
        slPitch[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slPitch[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slPitch[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slPitch[i].setReverse(true);
        slPitch[i].setRotaryParameters(M_PI, 3*M_PI, false);
        slPitch[i].setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
        slPitchAttachment[i] = new SliderAttachment(valueTreeState, "pitch" + String(i), slPitch[i]);
    }
    
    addAndMakeVisible(&lbYaw);
    lbYaw.setText("Yaw");
    lbYaw.setJustification(Justification::centred);
    
    addAndMakeVisible(&lbPitch);
    lbPitch.setText("Pitch");
    lbPitch.setJustification(Justification::centred);
    
    addAndMakeVisible(&lbOrder);
    lbOrder.setText("Order");
    lbOrder.setJustification(Justification::right);
    
    addAndMakeVisible(&lbShape);
    lbShape.setText("Shape");
    lbShape.setJustification(Justification::right);
    
    addAndMakeVisible(&gcSettings);
    gcSettings.setText("Probe Settings and Normalization");
    
    addAndMakeVisible(&gcFilterBands);
    gcFilterBands.setText("Filter Bands and Probe Response");
    
    addAndMakeVisible(&gcOrderAndShape);
    gcOrderAndShape.setText("Order and Shape");
    
    addAndMakeVisible(&gcPanning);
    gcPanning.setText("Spatial Panning");
    
    
    
    addAndMakeVisible(&lbProbeYaw);
    lbProbeYaw.setText("Yaw");
    lbProbeYaw.setJustification(Justification::centred);
    
    addAndMakeVisible(&lbProbePitch);
    lbProbePitch.setText("Pitch");
    lbProbePitch.setJustification(Justification::centred);
    
    addAndMakeVisible(&lbProbeRoll);
    lbProbeRoll.setText("Roll");
    lbProbeRoll.setJustification(Justification::centred);
    
    addAndMakeVisible(&lbNormalization);
    lbNormalization.setText("Normalization");
    lbNormalization.setJustification(Justification::right);
    
    
    
    addAndMakeVisible(&sphere);
    for (int i = 0; i < numberOfBands; ++i)
    {
        sphereElements[i].setColour(colours[i]);
        sphereElements[i].setSliders(&slYaw[i],&slPitch[i]);
        sphere.addElement(&sphereElements[i]);
    }
    
    masterElement.setColour(Colours::black);
    masterElement.setSliders(&slMasterYaw,&slMasterPitch);
    sphere.addElement(&masterElement);
    
    startTimer(10);
}

DirectivityShaperAudioProcessorEditor::~DirectivityShaperAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DirectivityShaperAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (globalLaF.ClBackground);
}

void DirectivityShaperAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;
    const int rotSliderSpacing = 10;
    Rectangle<int> area (getLocalBounds());
    
    Rectangle<int> footerArea (area.removeFromBottom (footerHeight));
    footer.setBounds(footerArea);
    
    area.removeFromLeft(leftRightMargin);
    area.removeFromRight(leftRightMargin);
    Rectangle<int> headerArea = area.removeFromTop(headerHeight);
    title.setBounds (headerArea);
    area.removeFromTop(10);
    
    { // left side
        Rectangle<int> leftSide(area.removeFromLeft(545));
        { // upper row
            Rectangle<int> filterArea(leftSide.removeFromTop(250));
            gcFilterBands.setBounds(filterArea);
            filterArea.removeFromTop(25);
            
            Rectangle<int> cbArea (filterArea.removeFromBottom(50));
            for (int i = 0; i < numberOfBands; ++i)
            {
                slFilterFrequency[i].setBounds(cbArea.removeFromLeft(40));
                slFilterGain[i].setBounds(cbArea.removeFromLeft(40));
                slFilterQ[i].setBounds(cbArea.removeFromLeft(40));
                cbArea.removeFromLeft(20);
            }
            
            cbArea = filterArea.removeFromBottom(15);
            cbArea.removeFromLeft(20);
            for (int i = 0; i < numberOfBands; ++i)
            {
                cbFilterType[i].setBounds(cbArea.removeFromLeft(100));
                cbArea.removeFromLeft(40);
            }
            
            fv.setBounds(filterArea);
        }
        
        leftSide.removeFromTop(25); // spacing
        
        { // lower row
            gcOrderAndShape.setBounds(leftSide);
            leftSide.removeFromTop(25);
            
            
            Rectangle<int> row(leftSide.removeFromTop(250));
            xyPad.setBounds(row.removeFromLeft(250));
            dv.setBounds(row.removeFromRight(250));
            
            leftSide.removeFromTop(15); //spacing
            row = leftSide.removeFromTop(50);
            
            lbOrder.setBounds(row.removeFromLeft(50));
            for (int i = 0; i < numberOfBands; ++i)
            {
                row.removeFromLeft(4); // spacing
                slOrder[i].setBounds(row.removeFromLeft(40));
            }
            
            row.removeFromLeft(55); // spacing
            
            lbShape.setBounds(row.removeFromLeft(50));
            for (int i = 0; i < numberOfBands; ++i)
            {
                row.removeFromLeft(4);
                slShape[i].setBounds(row.removeFromLeft(40));
            }
        }
        DBG("remaining height: " << leftSide.getHeight());
    }
    
    
    area.removeFromLeft(20);
    DBG(area.getWidth());
    
    {
        Rectangle<int> rightSide(area);
        {
            Rectangle<int> panningArea(rightSide.removeFromTop(375));
            gcPanning.setBounds(panningArea);
            panningArea.removeFromTop(25);
            
            
            Rectangle<int> sliderRow = panningArea.removeFromBottom(50);
            lbPitch.setBounds(sliderRow.removeFromLeft(47));
            for (int i = 0; i < numberOfBands; ++i)
            {
                sliderRow.removeFromLeft(4);
                slPitch[i].setBounds(sliderRow.removeFromLeft(40));
            }
            
            sliderRow = panningArea.removeFromBottom(50);
            lbYaw.setBounds(sliderRow.removeFromLeft(47));
            for (int i = 0; i < numberOfBands; ++i)
            {
                sliderRow.removeFromLeft(4);
                slYaw[i].setBounds(sliderRow.removeFromLeft(40));
            }
            sphere.setBounds(panningArea);
            
            rightSide.removeFromTop(25);
            
            {
                gcSettings.setBounds(rightSide);
                rightSide.removeFromTop(25);
                
                Rectangle<int> sliderRow(rightSide.removeFromTop(55));
                
                slMasterYaw.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                slMasterPitch.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                slMasterRoll.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                tbMasterToggle.setBounds(sliderRow);
                
                sliderRow = rightSide.removeFromTop(15);
                lbProbeYaw.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                lbProbePitch.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                lbProbeRoll.setBounds(sliderRow.removeFromLeft(40));
                
                rightSide.removeFromTop(15);
                
                sliderRow = rightSide.removeFromTop(20);
                lbNormalization.setBounds(sliderRow.removeFromLeft(80));
                sliderRow.removeFromLeft(10);
                cbNormalization.setBounds(sliderRow.removeFromLeft(90));
            }
        }
    }
    
    
    
}

void DirectivityShaperAudioProcessorEditor::timerCallback()
{
    
    if (processor.maxPossibleOrder != maxPossibleOrder)
    {
        maxPossibleOrder = processor.maxPossibleOrder;
        title.getOutputWidgetPtr()->updateOrderCb(maxPossibleOrder);
    }
    
    if (processor.ambisonicOrder != ambisonicOrder)
    {
        ambisonicOrder = processor.ambisonicOrder;
        xyPad.setMaxOrder(ambisonicOrder);
    }
    for (int b = 0; b <numberOfBands; ++b)
    {
        for (int i = 0; i < 8; ++i)
            weights[b][i] = processor.weights[b][i];
    }
    
    if (processor.updateFv)
    {
        processor.updateFv = false;
        fv.repaint();
    }
    
    sphere.repaint();
    xyPad.repaint();
    dv.repaint();
}
