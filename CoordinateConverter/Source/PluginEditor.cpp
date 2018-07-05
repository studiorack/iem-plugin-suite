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
CoordinateConverterAudioProcessorEditor::CoordinateConverterAudioProcessorEditor (CoordinateConverterAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts),
      panner (*valueTreeState.getParameter("azimuth"), valueTreeState.getParameterRange("azimuth"), *valueTreeState.getParameter("elevation"), valueTreeState.getParameterRange("elevation")),
      xyzPanner (*valueTreeState.getParameter("xPos"), valueTreeState.getParameterRange("xPos"),
              *valueTreeState.getParameter("yPos"), valueTreeState.getParameterRange("yPos"),
              *valueTreeState.getParameter("zPos"), valueTreeState.getParameterRange("zPos"))
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (450, 550, 800, 650); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("Coordinate"), String ("Converter"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================


    addAndMakeVisible (gcSpherical);
    gcSpherical.setText ("Spherical Coordinates");

    addAndMakeVisible (gcCartesian);
    gcCartesian.setText ("Cartesian Coordinates");

    addAndMakeVisible (gcRange);
    gcRange.setText ("Normalization Settings");

    addAndMakeVisible (sphere);
    panner.setColour (Colours::white);
    sphere.addElement (&panner);


    // ============== BEGIN: SPHERICAL COORDINATES ============

    addAndMakeVisible (slAzimuth);
    slAzimuthAttachment = new SliderAttachment (valueTreeState, "azimuth", slAzimuth);
    slAzimuth.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slAzimuth.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slAzimuth.setReverse (true);
    slAzimuth.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[0]);
    slAzimuth.setRotaryParameters (M_PI, 3*M_PI, false);
    slAzimuth.setTooltip ("Azimuth angle");

    addAndMakeVisible (slElevation);
    slElevationAttachment = new SliderAttachment(valueTreeState, "elevation", slElevation);
    slElevation.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slElevation.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slElevation.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[1]);
    slElevation.setRotaryParameters (0.5 * M_PI, 2.5 * M_PI, false);
    slElevation.setTooltip( "Elevation angle");

    addAndMakeVisible (slRadius);
    slRadiusAttachment = new SliderAttachment (valueTreeState, "radius", slRadius);
    slRadius.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slRadius.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slRadius.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slRadius.setReverse(false);
    slRadius.setTooltip("Radius (normalized)");

    addAndMakeVisible (lbAzimuth);
    lbAzimuth.setText ("Azimuth");

    addAndMakeVisible (lbElevation);
    lbElevation.setText ("Elevation");

    addAndMakeVisible (lbRadius);
    lbRadius.setText ("Radius");

    // ============== END: SPHERICAL COORDINATES ============

    // ============== BEGIN: CARTESIAN COORDINATES ============


    addAndMakeVisible (&xyPlane);
    xyPlane.addElement (&xyzPanner);
    xyPlane.useAutoScale (true);
    xyPlane.setDimensions ({2.0f, 2.0f, 2.0f});

    addAndMakeVisible (&zyPlane);
    zyPlane.setPlane (PositionPlane::Planes::zy);
    zyPlane.addElement (&xyzPanner);
    zyPlane.useAutoScale (true);
    zyPlane.setDimensions ({2.0f, 2.0f, 2.0f});

    addAndMakeVisible (slXPos);
    slXPosAttachment = new SliderAttachment(valueTreeState, "xPos", slXPos);
    slXPos.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slXPos.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slXPos.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slXPos.setTooltip ("x coordinate (normalized)");
    //slXPos.addListener(this);

    addAndMakeVisible (slYPos);
    slYPosAttachment = new SliderAttachment(valueTreeState, "yPos", slYPos);
    slYPos.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slYPos.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slYPos.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slYPos.setTooltip ("y coordinate (normalized)");
    //slSourceY.addListener(this);

    addAndMakeVisible (slZPos);
    slZPosAttachment = new SliderAttachment(valueTreeState, "zPos", slZPos);
    slZPos.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slZPos.setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
    slZPos.setColour (Slider::rotarySliderOutlineColourId, globalLaF.ClWidgetColours[2]);
    slZPos.setTooltip ("z coordinate (normalized)");
    //slSourceZ.addListener(this);


    addAndMakeVisible (lbXPos);
    lbXPos.setText ("x");

    addAndMakeVisible (lbYPos);
    lbYPos.setText ("y");

    addAndMakeVisible (lbZPos);
    lbZPos.setText ("z");

    // ============== END: CARTESIAN COORDINATES ============

    // ============== BEGIN: RANGE SETTINGS ============

    addAndMakeVisible (slRadiusRange);
    slRadiusRange.setJustificationType(Justification::centred);
    slRadiusRange.setEditable (true);
    slRadiusRangeAttachment = new LabelAttachment (valueTreeState, "radiusRange", slRadiusRange);

    addAndMakeVisible (lbRadiusRange);
    lbRadiusRange.setText ("Radius");

    addAndMakeVisible (slXRange);
    slXRange.setJustificationType(Justification::centred);
    slXRange.setEditable (true);
    slXRangeAttachment = new LabelAttachment (valueTreeState, "xRange", slXRange);

    addAndMakeVisible (lbXRange);
    lbXRange.setText ("+/- X");

    addAndMakeVisible (slYRange);
    slYRange.setJustificationType(Justification::centred);
    slYRange.setEditable (true);
    slYRangeAttachment = new LabelAttachment (valueTreeState, "yRange", slYRange);

    addAndMakeVisible (lbYRange);
    lbYRange.setText ("+/- Y");

    addAndMakeVisible (slZRange);
    slZRange.setJustificationType(Justification::centred);
    slZRange.setEditable (true);
    slZRangeAttachment = new LabelAttachment (valueTreeState, "zRange", slZRange);

    addAndMakeVisible (lbZRange);
    lbZRange.setText ("+/- Z");


    // ============== END: RANGE SETTINGS ============

    // start timer after everything is set up properly
    startTimer (20);
}

CoordinateConverterAudioProcessorEditor::~CoordinateConverterAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void CoordinateConverterAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void CoordinateConverterAudioProcessorEditor::resized()
{
    // ============ BEGIN: header and footer ============
    const int leftRightMargin = 30;
    const int headerHeight = 60;
    const int footerHeight = 25;

    const int rotSliderHeight = 55;
    const int rotSliderSpacing = 17;
    const int rotSliderWidth = 40;

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

    // ===== RANGE SETTINGS
    auto settingsArea = area.removeFromBottom (60);
    gcRange.setBounds (settingsArea);
    settingsArea.removeFromTop (25);

    {
        auto sliderRow = settingsArea.removeFromTop (18);
        lbRadiusRange.setBounds (sliderRow.removeFromLeft (40));
        sliderRow.removeFromLeft (5);
        slRadiusRange.setBounds (sliderRow.removeFromLeft (50));

        sliderRow.removeFromLeft (5);

        lbXRange.setBounds (sliderRow.removeFromLeft (35));
        sliderRow.removeFromLeft (5);
        slXRange.setBounds (sliderRow.removeFromLeft (50));

        sliderRow.removeFromLeft (5);

        lbYRange.setBounds (sliderRow.removeFromLeft (35));
        sliderRow.removeFromLeft (5);
        slYRange.setBounds (sliderRow.removeFromLeft (50));

        sliderRow.removeFromLeft (5);

        lbZRange.setBounds (sliderRow.removeFromLeft (35));
        sliderRow.removeFromLeft (5);
        slZRange.setBounds (sliderRow.removeFromLeft (50));
    }

    area.removeFromBottom (10);
    
    // ===== SPHERICAL

    const int sphericalWidth = 220;
    auto sphericalArea = area.removeFromLeft (sphericalWidth);
    gcSpherical.setBounds (sphericalArea);
    sphericalArea.removeFromTop (25);

    auto sliderArea = sphericalArea.removeFromBottom (rotSliderHeight + 12 + 5); // slider + spacing + label
    const int w = sphericalArea.getWidth();
    const int leftMargin = 0.5f * (w - 3 * rotSliderWidth - 2 * rotSliderSpacing);
    auto sliderRow = sliderArea.removeFromTop (rotSliderHeight);
    sliderRow.removeFromLeft (leftMargin);
    slAzimuth.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft(rotSliderSpacing);
    slElevation.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing);
    slRadius.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sliderArea.removeFromBottom (5);

    sliderRow = sliderArea.removeFromTop (12);
    sliderRow.removeFromLeft (leftMargin);
    lbAzimuth.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
    sliderRow.removeFromLeft (rotSliderSpacing - 5);
    lbElevation.setBounds (sliderRow.removeFromLeft (rotSliderWidth + 10));
    sliderRow.removeFromLeft (rotSliderSpacing - 5);
    lbRadius.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

    sphere.setBounds (sphericalArea);


    area.removeFromLeft (20);

    // ==== CARTESIAN

    auto cartesianArea = area;
    gcCartesian.setBounds (cartesianArea);
    cartesianArea.removeFromTop (25);

    {
        auto sliderArea = cartesianArea.removeFromBottom (rotSliderHeight + 12 + 5); // slider + spacing + label
        const int w = cartesianArea.getWidth();
        const int leftMargin = 0.5f * (w - 3 * rotSliderWidth - 2 * rotSliderSpacing);

        sliderArea.removeFromLeft (leftMargin);

        auto sliderRow = sliderArea.removeFromTop (rotSliderHeight);
        slXPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        slYPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        slZPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        sliderRow = sliderArea.removeFromTop (12);
        lbXPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft(rotSliderSpacing);
        lbYPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));
        sliderRow.removeFromLeft (rotSliderSpacing);
        lbZPos.setBounds (sliderRow.removeFromLeft (rotSliderWidth));

        const int sphereHeight = sphere.getHeight();
        const int planeHeight = (sphereHeight - 11) / 2;

        auto planeArea = cartesianArea.removeFromLeft (planeHeight);
        xyPlane.setBounds (planeArea.removeFromTop (planeHeight));
        planeArea.removeFromTop (11);
        zyPlane.setBounds (planeArea.removeFromTop (planeHeight));

    }
}

void CoordinateConverterAudioProcessorEditor::timerCallback()
{
    if (processor.repaintPositionPlanes.get())
    {
        processor.repaintPositionPlanes = false;
        xyPlane.repaint();
        zyPlane.repaint();
    }

    if (processor.repaintSphere.get())
    {
        processor.repaintSphere = false;
        sphere.repaint();
    }

    if (processor.updatePlaneDimensions.get())
    {
        processor.repaintSphere = false;

    }

}
