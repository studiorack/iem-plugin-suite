/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://www.iem.at
 
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


#define RS_FLT_EPSILON 1.19209290E-07F
class ReverseSlider : public Slider
{
public:
    ReverseSlider () :
    Slider(),
    lastDistanceFromDragStart(0),
    reversed(false),
    isDual(false),
    scrollWheelEnabled(true)
    {}

    ~ ReverseSlider () {}

public:
    void setReverse (bool shouldBeReversed)
    {
        if (reversed != shouldBeReversed)
        {
            reversed = shouldBeReversed;
            repaint();
        }
    }
    void setDual (bool shouldBeDual)
    {
        if (isDual != shouldBeDual)
        {
            isDual = shouldBeDual;
            repaint();
        }
    }



    double proportionOfLengthToValue (double proportion) override
    {
        double ret = 0;
        if (reversed)
            ret = getMaximum() + getMinimum() - Slider::proportionOfLengthToValue(proportion);
            else
                ret = Slider::proportionOfLengthToValue(proportion);
                return ret;
    }

    double valueToProportionOfLength (double value) override
    {
        double ret = 0;
        if (reversed)
            ret = jlimit(0., 1., 1.0 - Slider::valueToProportionOfLength(value));
            else
                ret = Slider::valueToProportionOfLength(value);
                return ret;
    }

    void setScrollWheelEnabled(bool enabled) {
        scrollWheelEnabled = enabled;
        Slider::setScrollWheelEnabled(enabled);
    }
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel) override
    {
        if (isRotary() && !getRotaryParameters().stopAtEnd && scrollWheelEnabled)
        {
            const double delta = (std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? -wheel.deltaX : wheel.deltaY)* (wheel.isReversed ? -1.0f : 1.0f) * (reversed ? -1.0f : 1.0f);
            bool positiveDelta = delta >= 0.0;

            if (std::abs(getValue() - getMaximum()) < getInterval() || std::abs(getValue() - getMaximum()) < RS_FLT_EPSILON)
            {
                if (positiveDelta)
                    setValue(getMinimum());
            }
            else if (std::abs(getValue() - getMinimum()) < getInterval() || std::abs(getValue() - getMinimum()) < RS_FLT_EPSILON)
            {
                if (!positiveDelta)
                    setValue(getMaximum());
            }
        }
        Slider::mouseWheelMove(e, wheel);
    }
    void mouseDown (const MouseEvent& e) override
    {
        lastDistanceFromDragStart = 0;
        Slider::mouseDown(e);
    }
    void mouseDrag (const MouseEvent& e) override
    {
        if (isRotary() && !getRotaryParameters().stopAtEnd && scrollWheelEnabled)
        {
            int delta = 0;
            switch (getSliderStyle())
            {
                case RotaryVerticalDrag:
                    delta = - e.getDistanceFromDragStartY() - lastDistanceFromDragStart;
                    break;
                case RotaryHorizontalDrag:
                    delta = e.getDistanceFromDragStartX() - lastDistanceFromDragStart;
                    break;
                case RotaryHorizontalVerticalDrag:
                    delta = e.getDistanceFromDragStartX() - e.getDistanceFromDragStartY() - lastDistanceFromDragStart;
                    break;
                default:
                    break;
            }
            delta = delta * (reversed ? -1 : 1);

            if (std::abs(getValue() - getMaximum()) < getInterval() || std::abs(getValue() - getMaximum()) < RS_FLT_EPSILON)
            {
                if (delta > 0)
                {
                    setValue(getMinimum());
                    Slider::mouseDown(e); //hack
                }
            }
            else if (std::abs(getValue() - getMinimum()) < getInterval() || std::abs(getValue() - getMinimum()) < RS_FLT_EPSILON)
            {
                if (delta < 0)
                {
                    setValue(getMaximum());
                    Slider::mouseDown(e); //hack
                }
            }
        }

        switch (getSliderStyle())
        {
            case RotaryVerticalDrag:
                lastDistanceFromDragStart = - e.getDistanceFromDragStartY();
                break;
            case RotaryHorizontalDrag:
                lastDistanceFromDragStart = e.getDistanceFromDragStartX();
                break;
            case RotaryHorizontalVerticalDrag:
                lastDistanceFromDragStart = e.getDistanceFromDragStartX() - e.getDistanceFromDragStartY();
                break;
            default:
                break;
        }

        Slider::mouseDrag(e);
    }

private:
    int lastDistanceFromDragStart;
    bool reversed;
    bool isDual;
    bool scrollWheelEnabled;
};
