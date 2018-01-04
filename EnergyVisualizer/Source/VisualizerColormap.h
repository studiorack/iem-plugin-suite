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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class VisualizerColormap    : public Component
{
public:
    VisualizerColormap()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~VisualizerColormap()
    {
    }

    void paint (Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */
        
        Colour colormapData[8];
        colormapData[0] = Colours::skyblue.withMultipliedAlpha(0.0f);
        colormapData[1] = Colours::skyblue.withMultipliedAlpha(0.2f);
        colormapData[2] = Colours::skyblue.withMultipliedAlpha(0.3f);
        colormapData[3] = Colour::fromFloatRGBA(0.167f, 0.620f, 0.077f, 6.0f);
        colormapData[4] = Colour::fromFloatRGBA(0.167f, 0.620f, 0.077f, 7.0f);
        colormapData[5] = Colour::fromFloatRGBA(0.8f, 0.620f, 0.077f, 0.8f);
        colormapData[6] = Colour::fromFloatRGBA(0.8f, 0.620f, 0.077f, 1.0f);
        colormapData[7] = Colours::red;
        
        
        
        
        Rectangle<int> colormapArea(getLocalBounds());
        colormapArea.removeFromTop(12);
        colormapArea.removeFromBottom(6);
        ColourGradient gradient;
        gradient.point1 = colormapArea.getTopLeft().toFloat();
        gradient.point2 = colormapArea.getBottomLeft().toFloat();
        
        for (int i=0; i<8; ++i)
        {
            gradient.addColour(1.0f - i*1.f/7, colormapData[i]);
        }
        
        Path path;
        path.addRectangle(colormapArea);
        g.setGradientFill(gradient);
        g.fillPath (path);

        g.setColour (Colours::white);
        int width = colormapArea.getWidth();
        
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 1)));
        g.drawText("dB", 0, 0, width, 12, Justification::centred);
        
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 0)));
        g.setFont(12.0f);
        
        float yStep = (float) colormapArea.getHeight() / 7;
        
        g.drawText(String(maxLevel,1), 0, 12, width, 12, Justification::centred);
        for (int i=1; i<8; ++i)
        {
            g.drawText(String(maxLevel - 5 * i,1), 0, 6 + yStep * i, width, 12, Justification::centred);
        }
        
    }

    void setMaxLevel(float newMaxLevel)
    {
        maxLevel = newMaxLevel;
        repaint();
    }
    
    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    float maxLevel = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualizerColormap)
};
