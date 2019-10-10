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
DirectivityShaperAudioProcessorEditor::DirectivityShaperAudioProcessorEditor (DirectivityShaperAudioProcessor& p, AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), processor (p), valueTreeState(vts), footer (p.getOSCParameterInterface()),
probeElement(*valueTreeState.getParameter("probeAzimuth"), valueTreeState.getParameterRange("probeAzimuth"),
              *valueTreeState.getParameter("probeElevation"), valueTreeState.getParameterRange("probeElevation")),
fv(20.0f, 20000.0f, -50.0f, 10.0f, 10.0f)
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


    addAndMakeVisible(&cbDirectivityNormalization);
    cbDirectivityNormalization.addItem("basic decode", 1);
    cbDirectivityNormalization.addItem("on-axis", 2);
    cbDirectivityNormalization.addItem("constant energy", 3);
    cbDirectivityNormalizationAttachment.reset (new ComboBoxAttachment (valueTreeState, "normalization", cbDirectivityNormalization));


    addAndMakeVisible(&fv);
    fv.setSampleRate(48000); //TODO
    for (int i = 0; i < numberOfBands; ++i)
        fv.addCoefficients(processor.filter[i].coefficients, colours[i], &slFilterFrequency[i], &slFilterGain[i], &slFilterQ[i], &processor.probeGains[i]);
    fv.setParallel(true);

    addAndMakeVisible(&xyPad);
    for (int i = 0; i < numberOfBands; ++i)
        xyPad.addElement(slShape[i], slOrder[i], colours[i]);

    addAndMakeVisible(&dv);
    for (int i = 0; i < numberOfBands; ++i)
        dv.addElement(weights[i], colours[i]);

    cbOrderSettingAttachment.reset (new ComboBoxAttachment (valueTreeState, "orderSetting", *title.getOutputWidgetPtr()->getOrderCbPointer()));
    cbNormalizationAttachment.reset (new ComboBoxAttachment (valueTreeState, "useSN3D", *title.getOutputWidgetPtr()->getNormCbPointer()));

    addAndMakeVisible(&slProbeAzimuth);
    slProbeAzimuth.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slProbeAzimuth.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
    slProbeAzimuth.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slProbeAzimuth.setReverse(true);
    slProbeAzimuth.setRotaryParameters(MathConstants<float>::pi, 3*MathConstants<float>::pi, false);
    slProbeAzimuthAttachment.reset (new SliderAttachment (valueTreeState, "probeAzimuth", slProbeAzimuth));

    addAndMakeVisible(&slProbeElevation);
    slProbeElevation.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slProbeElevation.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
    slProbeElevation.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slProbeElevation.setRotaryParameters(0.5 * MathConstants<float>::pi, 2.5 * MathConstants<float>::pi, false);
    slProbeElevationAttachment.reset (new SliderAttachment (valueTreeState, "probeElevation", slProbeElevation));

    addAndMakeVisible(&slProbeRoll);
    slProbeRoll.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    slProbeRoll.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
    slProbeRoll.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slProbeRoll.setRotaryParameters(MathConstants<float>::pi, 3*MathConstants<float>::pi, false);
    slProbeRollAttachment.reset (new SliderAttachment (valueTreeState, "probeRoll", slProbeRoll));

    addAndMakeVisible(&tbProbeLock);
    tbProbeLock.setButtonText("Lock Directions");
    tbProbeLockAttachment.reset (new ButtonAttachment (valueTreeState, "probeLock", tbProbeLock));


    for (int i = 0; i < numberOfBands; ++i)
    {
        addAndMakeVisible(&cbFilterType[i]);
        cbFilterType[i].addItem("All-pass", 1);
        cbFilterType[i].addItem("Low-pass", 2);
        cbFilterType[i].addItem("Band-pass", 3);
        cbFilterType[i].addItem("High-pass", 4);
        cbFilterType[i].setJustificationType(Justification::centred);
        cbFilterTypeAttachment[i].reset (new ComboBoxAttachment (valueTreeState, "filterType" + String(i), cbFilterType[i]));

        addAndMakeVisible(&slFilterFrequency[i]);
        slFilterFrequency[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slFilterFrequency[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slFilterFrequency[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterFrequencyAttachment[i].reset (new SliderAttachment (valueTreeState, "filterFrequency" + String(i), slFilterFrequency[i]));

        addAndMakeVisible(&slFilterQ[i]);
        slFilterQ[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slFilterQ[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slFilterQ[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterQAttachment[i].reset (new SliderAttachment (valueTreeState, "filterQ" + String(i), slFilterQ[i]));

        addAndMakeVisible(&slFilterGain[i]);
        slFilterGain[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slFilterGain[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slFilterGain[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterGainAttachment[i].reset (new SliderAttachment (valueTreeState, "filterGain" + String(i), slFilterGain[i]));

        addAndMakeVisible(&slOrder[i]);
        slOrder[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slOrder[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slOrder[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slOrderAttachment[i].reset (new SliderAttachment (valueTreeState, "order" + String(i), slOrder[i]));

        addAndMakeVisible(&slShape[i]);
        slShape[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slShape[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slShape[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slShapeAttachment[i].reset (new SliderAttachment (valueTreeState, "shape" + String(i), slShape[i]));

        addAndMakeVisible(&slAzimuth[i]);
        slAzimuth[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slAzimuth[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slAzimuth[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slAzimuth[i].setReverse(true);
        slAzimuth[i].setRotaryParameters(MathConstants<float>::pi, 3*MathConstants<float>::pi, false);
        slAzimuth[i].setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
        slAzimuthAttachment[i].reset (new SliderAttachment (valueTreeState, "azimuth" + String(i), slAzimuth[i]));

        addAndMakeVisible(&slElevation[i]);
        slElevation[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slElevation[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 15);
        slElevation[i].setColour(Slider::rotarySliderOutlineColourId, colours[i]);
        slElevation[i].setRotaryParameters(0.5 * MathConstants<float>::pi, 2.5 * MathConstants<float>::pi, false);
        slElevation[i].setTextValueSuffix(CharPointer_UTF8 (R"(°)"));
        slElevationAttachment[i].reset (new SliderAttachment (valueTreeState, "elevation" + String(i), slElevation[i]));
    }

    addAndMakeVisible(&lbAzimuth);
    lbAzimuth.setText("Azimuth");
    lbAzimuth.setJustification(Justification::centred);

    addAndMakeVisible(&lvElevation);
    lvElevation.setText("Elevation");
    lvElevation.setJustification(Justification::centred);

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


    addAndMakeVisible(&lbProbeAzimuth);
    lbProbeAzimuth.setText("Azimuth");
    lbProbeAzimuth.setJustification(Justification::centred);

    addAndMakeVisible(&lbProbeElevation);
    lbProbeElevation.setText("Elevation");
    lbProbeElevation.setJustification(Justification::centred);

    addAndMakeVisible(&lbProbeRoll);
    lbProbeRoll.setText("Roll");
    lbProbeRoll.setJustification(Justification::centred);

    addAndMakeVisible(&lbNormalization);
    lbNormalization.setText("Normalization");
    lbNormalization.setJustification(Justification::right);



    addAndMakeVisible(&sphere);
    for (int i = 0; i < numberOfBands; ++i)
    {
        sphereElements[i].reset (new SpherePanner::AzimuthElevationParameterElement(*valueTreeState.getParameter("azimuth" + String(i)), valueTreeState.getParameterRange("azimuth" + String(i)), *valueTreeState.getParameter("elevation" + String(i)), valueTreeState.getParameterRange("elevation" + String(i))));
        sphereElements[i]->setColour(colours[i]);
        sphere.addElement (sphereElements[i].get());
    }

    probeElement.setColour(Colours::black);
    probeElement.setTextColour(Colours::white);
    probeElement.setLabel("P");
    sphere.addElement(&probeElement);

    startTimer(30);
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
    }

    area.removeFromLeft(20);

    {
        Rectangle<int> rightSide(area);
        {
            Rectangle<int> panningArea(rightSide.removeFromTop(375));
            gcPanning.setBounds(panningArea);
            panningArea.removeFromTop(25);


            Rectangle<int> sliderRow = panningArea.removeFromBottom(50);
            lvElevation.setBounds(sliderRow.removeFromLeft(47));
            for (int i = 0; i < numberOfBands; ++i)
            {
                sliderRow.removeFromLeft(4);
                slElevation[i].setBounds(sliderRow.removeFromLeft(50));
            }

            sliderRow = panningArea.removeFromBottom(50);
            lbAzimuth.setBounds(sliderRow.removeFromLeft(47));
            for (int i = 0; i < numberOfBands; ++i)
            {
                sliderRow.removeFromLeft(4);
                slAzimuth[i].setBounds(sliderRow.removeFromLeft(50));
            }
            sphere.setBounds(panningArea);

            rightSide.removeFromTop(25);

            {
                gcSettings.setBounds(rightSide);
                rightSide.removeFromTop(25);

                Rectangle<int> sliderRow(rightSide.removeFromTop(55));

                slProbeAzimuth.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                slProbeElevation.setBounds(sliderRow.removeFromLeft(45));
                sliderRow.removeFromLeft(rotSliderSpacing);
                slProbeRoll.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                tbProbeLock.setBounds(sliderRow);

                sliderRow = rightSide.removeFromTop(15);
                lbProbeAzimuth.setBounds(sliderRow.removeFromLeft(40));
                sliderRow.removeFromLeft(rotSliderSpacing);
                lbProbeElevation.setBounds(sliderRow.removeFromLeft(45));
                sliderRow.removeFromLeft(rotSliderSpacing);
                lbProbeRoll.setBounds(sliderRow.removeFromLeft(40));

                rightSide.removeFromTop(15);

                sliderRow = rightSide.removeFromTop(20);
                lbNormalization.setBounds(sliderRow.removeFromLeft(80));
                sliderRow.removeFromLeft(10);
                cbDirectivityNormalization.setBounds(sliderRow.removeFromLeft(90));
            }
        }
    }
}

void DirectivityShaperAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    title.setMaxSize (processor.getMaxSize());
    // ==========================================

    const int processorAmbisonicOrder = processor.output.getOrder();
    if (processorAmbisonicOrder != ambisonicOrder)
    {
        ambisonicOrder = processorAmbisonicOrder;
        xyPad.setMaxOrder(ambisonicOrder);
    }

    for (int b = 0; b <numberOfBands; ++b)
    {
        for (int i = 0; i < 8; ++i)
            weights[b][i] = processor.weights[b][i];
    }

    if (processor.repaintFV.get())
    {
        processor.repaintFV = false;
        fv.setSampleRate (processor.getSampleRate());
    }


    if (processor.repaintSphere.get())
    {
        processor.repaintSphere = false;
        sphere.repaint();
    }

    if (processor.repaintXY.get())
    {
        processor.repaintXY = false;
        xyPad.repaint();
    }

    if (processor.repaintDV.get())
    {
        processor.repaintDV = false;
        dv.repaint();
    }

}
