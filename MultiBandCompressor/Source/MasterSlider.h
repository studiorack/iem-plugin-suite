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

//#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/customComponents/ReverseSlider.h"


//==============================================================================
/**
    This is a simple wrapper for a Slider that controls other components,
    e.g. other Sliders, by forwarding mouse events. It's intended to hold
    pointers to the sliders it is controlling.
*/
template <typename slaveType>
class MasterSlider    : public ReverseSlider
{
public:
    MasterSlider() : ReverseSlider ()
    {
    }

    ~MasterSlider()
    {
    }


    void mouseDrag (const MouseEvent& e) override
    {
        ReverseSlider::mouseDrag (e);
      
        if (!elements.isEmpty())
        {
            checkIfUpdateIsValid ();
          
            if (!preventUpdates)
            {
                for (int i = 0; i < elements.size(); ++i)
                {
                    if (*elements[i] != nullptr)
                        (*elements[i])->mouseDrag (e);
                }
            }
        }
    }
  
  
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) override
    {
        ReverseSlider::mouseWheelMove (e, wheel);
      
        if (!elements.isEmpty())
        {
            checkIfUpdateIsValid ();
          
            if (!preventUpdates)
            {
                for (int i = 0; i < elements.size(); ++i)
                {
                    if (*elements[i] != nullptr)
                        (*elements[i])->mouseWheelMove (e, wheel);
                }
            }
        }
    }
  

    void  mouseUp (const MouseEvent & e) override
    {
        ReverseSlider::mouseUp (e);
    
        if (!elements.isEmpty())
        {
            for (int i = 0; i < elements.size(); ++i)
            {
                if (*elements[i] != nullptr)
                    (*elements[i])->mouseUp (e);
            }
        }
    }
  
  
    void mouseDown (const MouseEvent& e) override
    {
        ReverseSlider::mouseDown (e);
    
        if (!elements.isEmpty())
        {
            for (int i = 0; i < elements.size(); ++i)
            {
                if (*elements[i] != nullptr)
                    (*elements[i])->mouseDown (e);
            }
        }
    }
  
//    void mouseEnter (const MouseEvent & e) override
//    {
//        ReverseSlider::mouseEnter (e);
//
//        // calculate currently allowed value range, as they could possibly change over time
//        range = getRange();
//        if (!elements.isEmpty())
//        {
//            for (int i = 0; i < elements.size(); ++i)
//            {
//                if (*elements[i] != nullptr)
//                {
//                    range = range.getUnionWith ((*elements[i])->getRange());
//                }
//
//            }
//        }
//    }
  
  
    void addSlave (slaveType& newSlave)
    {
        elements.add (new  slaveType* (&newSlave));
    }


private:
    OwnedArray<slaveType*> elements;
//    Range<double> range;
    double overshoot;
    bool isMaximum;
    bool preventUpdates = false;
  
  
    void checkIfUpdateIsValid ()
    {
        if (!preventUpdates)
        {
            for (int i = 0; i < elements.size(); ++i)
            {
                if ((*elements[i])->getValue() >= (*elements[i])->getMaximum())
                {
                    preventUpdates = true;
                    isMaximum = true;
                    overshoot = getValue();
                    return;
                }
                else if ((*elements[i])->getValue() <= (*elements[i])->getMinimum())
                {
                    preventUpdates = true;
                    isMaximum = false;
                    overshoot = getValue();
                    return;
                }
            }
        }
        else
        {
            preventUpdates = isMaximum ? (getValue() >= overshoot ? true : false) : (getValue() <= overshoot ? true : false);
//
//                if (isMaximum)
//                {
//                    preventUpdates = getValue() >= overshoot ? true : false;
//                }
//                else
//                {
//                    preventUpdates = getValue() <= overshoot ? true : false;
//                }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterSlider)
};
