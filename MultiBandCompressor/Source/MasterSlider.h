/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Markus Huber
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


//==============================================================================
/**
    This is a simple wrapper for a Slider that controls other components,
    e.g. other Sliders, by forwarding mouse events. It's intended to hold
    pointers to the sliders it is controlling.
*/
class MasterSlider : public Slider
{
public:
    MasterSlider()
    {
    }

    ~MasterSlider()
    {
    }


    void mouseDrag (const MouseEvent& e) override
    {
        Slider::mouseDrag (e);
        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseDrag (e);
        }
    }


    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) override
    {
        Slider::mouseWheelMove (e, wheel);
        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseWheelMove (e, wheel);
        }
    }


    void  mouseUp (const MouseEvent & e) override
    {
        Slider::mouseUp (e);
        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseUp (e);
        }
    }


    void mouseDown (const MouseEvent& e) override
    {
        Slider::mouseDown (e);
        for (int i = 0; i < elements.size(); ++i)
        {
            if (elements[i] != nullptr)
                elements[i]->mouseDown (e);
        }
    }


    void addSlave (Component& newComponentToControl)
    {
        elements.add (&newComponentToControl);
    }


private:
    Array<Component*> elements;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterSlider)
};
