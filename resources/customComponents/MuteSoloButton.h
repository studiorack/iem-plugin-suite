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


//==============================================================================
/*
*/
class MuteSoloButton    : public ToggleButton
{
public:
    enum Type
    {
        mute,
        solo
    };
    
    MuteSoloButton()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }
    ~MuteSoloButton()
    {
    }

    void setType (Type newType)
    {
        type = newType;
    }
    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds().reduced(1, 1);
        const bool state = getToggleState();
        
        
        Colour col;
        switch (type) { //set colour
            case mute:
                col = juce::Colours::red;
                break;
            case solo:
                col = juce::Colours::yellow;
                break;
                
            default:
                col = juce::Colours::blue;
                break;
        }
        
        
        Path path;
        path.addRoundedRectangle(bounds,bounds.getWidth()/5.0f);
        
        g.setColour(col.withMultipliedAlpha(state ? 1.0f : 0.8));
        g.strokePath(path,PathStrokeType(1.0f));
        
        g.setColour(col.withMultipliedAlpha(state ? 1.0f : 0.5f));
        g.fillPath(path);
        g.setFont(bounds.getHeight()-1);
        g.setColour(state ? Colours::black : col);
        g.drawFittedText(type == solo ? "S" : "M",bounds, juce::Justification::centred, 1);
        
    }

    void resized() override
    {
    }

private:
    Type type;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MuteSoloButton)
};


