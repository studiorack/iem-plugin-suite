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
MultiEQAudioProcessorEditor::MultiEQAudioProcessorEditor (MultiEQAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor (p), valueTreeState (vts), footer (p.getOSCReceiver())
{
    // ============== BEGIN: essentials ======================
    // set GUI size and lookAndFeel
    //setSize(500, 300); // use this to create a fixed-size GUI
    setResizeLimits (880, 330, 1000, 800); // use this to create a resizable GUI
    setLookAndFeel (&globalLaF);

    // make title and footer visible, and set the PluginName
    addAndMakeVisible (&title);
    title.setTitle (String ("Multi"), String ("EQ"));
    title.setFont (globalLaF.robotoBold, globalLaF.robotoLight);
    addAndMakeVisible (&footer);
    // ============= END: essentials ========================


    // create the connection between title component's comboBoxes and parameters
    cbNumInputChannelsAttachment = new ComboBoxAttachment (valueTreeState, "inputChannelsSetting", *title.getInputWidgetPtr()->getChannelsCbPointer());

    tooltips.setOpaque (false);

    const Colour colours[numFilterBands] = { Colours::cadetblue, // make sure you have enough colours in here
        Colours::mediumpurple,
        Colours::cornflowerblue,
        Colours::greenyellow,
        Colours::orangered,
        Colours::red };


    addAndMakeVisible (fv);
    for (int f = 0; f < numFilterBands; ++f)
        fv.addCoefficients (&processor.dummyFilter[f].coefficients, colours[f], &slFilterFrequency[f], &slFilterGain[f], &slFilterQ[f]);



    for (int i = 0; i < numFilterBands; ++i)
    {
        addAndMakeVisible (&cbFilterType[i]);
        cbFilterType[i].addItem ("High-Pass", 1);
        cbFilterType[i].addItem ("Low-shelf", 2);
        cbFilterType[i].addItem ("Peak", 3);
        cbFilterType[i].addItem ("High-shelf", 4);
        cbFilterType[i].addItem ("Low-pass", 5);
        cbFilterType[i].setJustificationType (Justification::centred);
        cbFilterTypeAttachment[i] = new ComboBoxAttachment (valueTreeState, "filterType" + String(i), cbFilterType[i]);

        addAndMakeVisible (&slFilterFrequency[i]);
        slFilterFrequency[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slFilterFrequency[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slFilterFrequency[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterFrequencyAttachment[i] = new SliderAttachment (valueTreeState, "filterFrequency" + String(i), slFilterFrequency[i]);

        addAndMakeVisible(&slFilterQ[i]);
        slFilterQ[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slFilterQ[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slFilterQ[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterQAttachment[i] = new SliderAttachment (valueTreeState, "filterQ" + String(i), slFilterQ[i]);

        addAndMakeVisible(&slFilterGain[i]);
        slFilterGain[i].setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        slFilterGain[i].setTextBoxStyle (Slider::TextBoxBelow, false, 50, 15);
        slFilterGain[i].setColour (Slider::rotarySliderOutlineColourId, colours[i]);
        slFilterGainAttachment[i] = new SliderAttachment (valueTreeState, "filterGain" + String(i), slFilterGain[i]);
    }








    // start timer after everything is set up properly
    startTimer (20);
}

MultiEQAudioProcessorEditor::~MultiEQAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void MultiEQAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (globalLaF.ClBackground);
}

void MultiEQAudioProcessorEditor::resized()
{
    DBG ("GUI resized to " << getLocalBounds().getWidth() << "x" << getLocalBounds().getHeight());
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


    // try to not use explicit coordinates to position your GUI components
    // the removeFrom...() methods are quite handy to create scalable areas
    // best practice would be the use of flexBoxes...
    // the following is medium level practice ;-)
    Rectangle<int> filterArea = area;
    { // upper row

        Rectangle<int> cbArea (filterArea.removeFromBottom (50));
        for (int i = 0; i < numFilterBands; ++i)
        {
            slFilterFrequency[i].setBounds(cbArea.removeFromLeft (40));
            slFilterGain[i].setBounds(cbArea.removeFromLeft  (40));
            slFilterQ[i].setBounds(cbArea.removeFromLeft(40));
            cbArea.removeFromLeft(20);
        }

        cbArea = filterArea.removeFromBottom(15);
        cbArea.removeFromLeft(20);
        for (int i = 0; i < numFilterBands; ++i)
        {
            cbFilterType[i].setBounds(cbArea.removeFromLeft(100));
            cbArea.removeFromLeft(40);
        }

        fv.setBounds(filterArea);
    }

}

void MultiEQAudioProcessorEditor::timerCallback()
{
    // === update titleBar widgets according to available input/output channel counts
    int maxInSize, maxOutSize;
    processor.getMaxSize (maxInSize, maxOutSize);
    title.setMaxSize (maxInSize, maxOutSize);
    // ==========================================

    if (processor.repaintFV.get())
    {
        processor.repaintFV = false;
        fv.repaint();
    }

    // insert stuff you want to do be done at every timer callback
}
