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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class LevelMeter    : public Component
{
    
public:
    LevelMeter()
    {

    }

    ~LevelMeter()
    {
    }

    void setGainReductionMeter (bool isGainReductionMeter) {
        isGRmeter = isGainReductionMeter;
        repaint();
    }
    
    float decibelsToY (float dB) {
        return offset - scale * std::tanh(dB / minLevel * -2.0f);
    }
    
    void paint (Graphics& g) override
    {
        int height = getHeight();
        Path bg;
        Rectangle<int> meterArea(0, 0, getWidth(), getHeight());
        meterArea.reduce(2,2);
        int width = meterArea.getWidth();
        int xPos = meterArea.getX();
        bg.addRoundedRectangle(meterArea, 2);
        
        g.setColour(Colour(0xFF212121));
        g.strokePath(bg, PathStrokeType(2.f));
        
        g.setColour(Colour(0XFF272727));
        g.setColour(Colours::black);
        g.fillPath(bg);
        
        
        Rectangle<int> lvlRect;
        g.setColour (levelColour);
        if (isGRmeter) {
            lvlRect = Rectangle<int>(Point<int>(meterArea.getX(), offset), Point<int>(meterArea.getRight(), decibelsToY(level)));
        }
        else {
            lvlRect = Rectangle<int>(Point<int>(meterArea.getX(), height), Point<int>(meterArea.getRight(), decibelsToY(level)));
        }
        
        g.fillRect(lvlRect);
        
    
        g.setColour(Colours::white);
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 0)));
        g.setFont(9.0f);
        
        int lastTextDrawPos = -1;
        drawLevelMark(g, xPos, width, 0, "0");
        drawLevelMark(g, xPos, width, -3, "3");
        drawLevelMark(g, xPos, width, -6, "6");

        
        for (float dB = -10.0f; dB >= minLevel; dB -= 5.0f) {
            lastTextDrawPos = drawLevelMark (g, xPos, width, dB, String ((int) -dB), lastTextDrawPos);
        }

        
 
        
    }

    int inline drawLevelMark(Graphics& g, int x, int width, const int level, const String& label, int lastTextDrawPos = -1)
    {
        float yPos = decibelsToY(level);
        x = x + 1.0f;
        width = width - 2.0f;
        
        g.drawLine (x, yPos, x + 2, yPos);
        g.drawLine (x + width - 2, yPos, x + width, yPos);
        if (yPos-4 > lastTextDrawPos)
        {
            g.drawText (label, x + 2, yPos - 4, width - 4, 9, Justification::centred, false);
            return yPos + 5;
        }
        return lastTextDrawPos;
    }
    
    void setColour (Colour newColour)
    {
        levelColour = newColour;
        repaint();
    }
    
    void setLevel (float newLevel)
    {
        level = newLevel;
        repaint();
    }
    
    void setMinLevel ( float newMinLevel)
    {
        minLevel = newMinLevel;
        repaint();
    }
    
    void resized() override
    {
        offset = 0.1*getHeight();
        scale = 0.9*getHeight();
    }

private:
    
    Colour levelColour = Colour(Colours::green);
    
    bool isGRmeter = false;
    float minLevel = -60.0f;
    float level = 0.0f;
    float scale = 0.0f;
    float offset = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
