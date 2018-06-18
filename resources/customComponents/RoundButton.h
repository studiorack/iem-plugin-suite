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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class RoundButton    : public ToggleButton
{
public:
    RoundButton()
    {
    }

    ~RoundButton()
    {
    }

    void paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();

        const auto fontSize = jmin (15.0f, bounds.getHeight() * 0.75f);

        const float boxSize = bounds.getWidth() * 0.8f;
        Rectangle<float> buttonArea((bounds.getWidth() - boxSize) * 0.5f, (bounds.getHeight() - boxSize) * 0.5f, boxSize, boxSize);

        const bool isButtonDown = isMouseButtonDown();
        const bool isMouseOverButton = isMouseOver();

        const bool ticked = getToggleState();

        if (isButtonDown)
            buttonArea.reduce(0.8f, 0.8f);
        else if (isMouseOverButton)
            buttonArea.reduce(0.4f, 0.4f);

        g.setColour(findColour(ToggleButton::tickColourId).withMultipliedAlpha(ticked ? 1.0f : isMouseOverButton ? 0.7f : 0.5f) );

        g.drawEllipse(buttonArea, 1.0f);

        buttonArea.reduce(1.5f, 1.5f);
        g.setColour(findColour(ToggleButton::tickColourId).withMultipliedAlpha(ticked ? 1.0f : isMouseOverButton ? 0.5f : 0.2f));

        g.fillEllipse(buttonArea);

        
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(13.0f, 1)));
        g.setFont (buttonArea.getHeight());
        g.setColour (findColour (getToggleState() ? TextButton::textColourOnId
                                 : TextButton::textColourOffId)
                     .withMultipliedAlpha (isEnabled() ? 1.0f : 0.5f));

        g.setColour(ticked ? findColour(ResizableWindow::backgroundColourId) :  findColour(ToggleButton::tickColourId).withMultipliedAlpha(isMouseOverButton ? 0.7f : 0.5f) );

        g.drawText(getButtonText(), getLocalBounds(), Justification::centred);
    }

    void resized() override
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoundButton)
};
