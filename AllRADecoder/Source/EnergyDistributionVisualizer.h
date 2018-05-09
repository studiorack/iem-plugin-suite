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

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../resources/customComponents/HammerAitovGrid.h"
#pragma once


class  EnergyDistributionVisualizer :  public Component
{
public:
    EnergyDistributionVisualizer(std::vector<R3>& pts, BigInteger& imagFlags, Image& energyImageFromProcessor, Image& rEImageFromProcessor) : Component(), extPoints(pts), imaginaryFlags(imagFlags), energyImage(energyImageFromProcessor), rEImage(rEImageFromProcessor)
    {
        setBufferedToImage(true);

        showrEVector = false;
        addAndMakeVisible(imgComp);
        imgComp.setImage(energyImage);
        imgComp.setImagePlacement(RectanglePlacement::stretchToFit);

        addAndMakeVisible(background);
        background.addMouseListener(this, false); // could this be risky?
    };
    ~EnergyDistributionVisualizer() {};


    void resized () override
    {
        imgComp.setBounds(getLocalBounds().reduced(10, 20));
        background.setBounds(getLocalBounds());
    }

    void paintOverChildren (Graphics& g) override
    {
        const Rectangle<float> bounds = getLocalBounds().toFloat().reduced(10.0f, 20.0f);
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        const float wh = bounds.getWidth() * 0.5f;
        const float hh = bounds.getHeight() * 0.5f;

        for (int i = 0; i < extPoints.size(); ++i)
        {
            R3 point = extPoints[i];
            g.setColour(activePoint == point.lspNum ? Colours::lawngreen : point.isImaginary ? Colours::orange : Colours::cornflowerblue);
            float x, y;
            float azimuth = degreesToRadians(point.azimuth);
            float elevation = degreesToRadians(point.elevation);
            HammerAitov::sphericalToXY(azimuth, elevation, x, y);

            Rectangle<float> rect (centreX + x*wh - 5.0f, centreY - y*hh - 5.0f, 10.0f, 10.0f);
            g.fillRoundedRectangle(rect, 5.0f);
        }

        g.setColour(Colours::white);
        g.setFont (getLookAndFeel().getTypefaceForFont(Font(12.0f)));
        g.setFont (12.f);

        String displayText = showrEVector ? "acos-rE source width (double-click to change)" : "energy fluctuations (double-click to change)";
        g.drawText(displayText, getLocalBounds().removeFromBottom(12), Justification::centred);
    };

    void mouseDoubleClick (const MouseEvent &event) override
    {
        showrEVector = !showrEVector;
        if (showrEVector)
            imgComp.setImage(rEImage);
        else
            imgComp.setImage(energyImage);
        imgComp.repaint();
        repaint();
    }

    void setActiveSpeakerIndex (int newIdx)
    {
        activePoint = newIdx;
        repaint();
    }

private:
    std::vector<R3>& extPoints;
    BigInteger& imaginaryFlags;
    int activePoint = -1;
    ImageComponent imgComp;
    Image& energyImage, rEImage;
    bool showrEVector;

    HammerAitovGrid background;

};
