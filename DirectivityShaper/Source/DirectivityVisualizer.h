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
using namespace dsp;
class DirectivityVisualizer    : public Component
{
    struct WeightsAndColour {
        float *weights;
        Colour colour;
    };

    const float deg2rad = MathConstants<float>::pi / 180.0f;
    const int degStep = 1;
    const int nLookUpSamples = 360;
    const int maxdB = 90;
    const float power = 3.0f;
    const int dBstep = 10;
    //#define scale 4
    const float scale = sqrt(4 * MathConstants<float>::pi) * decodeCorrection(7);
public:
    DirectivityVisualizer()
    {
        // 0th
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * (0.25f / (float) MathConstants<float>::pi); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 1st
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * (0.75f / MathConstants<float>::pi) * std::cos (phi); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 2nd
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * 2.0f*(5.0f /16.0f / MathConstants<float>::pi) * (3 * std::cos (phi) * std::cos (phi) - 1.0f); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 3rd
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * (7.0f / MathConstants<float>::pi) / 8.0f * (5 * pow(std::cos (phi), 3) - 3.0f * std::cos (phi));}, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 4th
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * 9.0f / 32.0f / MathConstants<float>::pi * (35 * pow(std::cos (phi),4) - 30* pow(std::cos (phi), 2) + 3.0f); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 5th
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * 8.0f / 256.0f * 11.0f / MathConstants<float>::pi * (63 * pow(std::cos (phi),5) - 70* pow(std::cos (phi),3) + 15.0f * std::cos (phi)); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 6th
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * 16.0f / 1024.0f * 13.0f / MathConstants<float>::pi * (231 *  pow(std::cos (phi),6) - 315 * pow(std::cos (phi), 4) + 105 * pow(std::cos (phi),2) - 5.0f); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        // 7th
        lookUpTables.add(new LookupTableTransform<float>([this] (float phi) { return scale * 16.0f / 1024.0f *15.0f /MathConstants<float>::pi * (429 * pow(std::cos (phi),7) - 693 * pow(std::cos (phi), 5) + 315* pow(std::cos (phi),3) - 35 * std::cos (phi)); }, -MathConstants<float>::pi, MathConstants<float>::pi, nLookUpSamples));

        for (int phi = -180; phi <= 180; phi += degStep)
        {
            pointsOnCircle.add(Point<float>(cos(deg2rad * phi), sin(deg2rad * phi)));
        }


        Path circle;
        circle.addEllipse(-1.0f, -1.0f, 2.0f, 2.0f);
        Path line;
        line.startNewSubPath(0.0f, -1.0f);
        line.lineTo(0.0f, 1.0f);


        grid.clear();
        for (int dB = 0; dB < maxdB; dB += dBstep)
            grid.addPath(circle, AffineTransform().scaled(dBToRadius(-dB)));

        subGrid.clear();
        for (int dB = dBstep/2; dB < maxdB; dB += dBstep)
            subGrid.addPath(circle, AffineTransform().scaled(dBToRadius(-dB)));

        subGrid.addPath(line);
        subGrid.addPath(line, AffineTransform().rotation(0.25f * MathConstants<float>::pi));
        subGrid.addPath(line, AffineTransform().rotation(0.5f * MathConstants<float>::pi));
        subGrid.addPath(line, AffineTransform().rotation(0.75f * MathConstants<float>::pi));

    }

    ~DirectivityVisualizer()
    {
    }

    float dBToRadius (float dB)
    {
        if (dB>0.0f) dB = 0.0f;
        //float radius = 1.0f - pow(abs(dB) / maxdB, power);
        float radius = (exp(power * dB / maxdB) - exp(-power)) / (1.0f - exp(-power));
        if (radius < 0.0f) radius = 0.0f;
        return radius;
    }

    void paint (Graphics& g) override
    {
        Rectangle<int> bounds = getLocalBounds();
        const int scale = plotArea.getWidth()/2;
        //const int height = bounds.getHeight();

        int centreX = bounds.getCentreX();
        int centreY = bounds.getCentreY();


        Path path;
        path = grid;
        path.applyTransform(transform);
        g.setColour (Colours::skyblue.withMultipliedAlpha(0.1f));
        g.fillPath(path);
        g.setColour (Colours::white);
        g.strokePath(path, PathStrokeType(1.0f));

        path = subGrid;
        path.applyTransform(transform);
        g.setColour (Colours::skyblue.withMultipliedAlpha(0.3f));
        g.strokePath(path, PathStrokeType(0.5f));

        g.setColour (Colours::white);
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 2)));
        g.setFont(12.0f);
        g.drawText("0 dB", centreX-10, centreY + scale * dBToRadius(0.0f) - 12, 20, 12, Justification::centred);
        g.drawText("-10", centreX-10, centreY + scale * dBToRadius(-10.0f), 20, 12, Justification::centred);
        g.drawText("-20", centreX-10, centreY + scale * dBToRadius(-20.0f), 20, 12, Justification::centred);


        int size = elements.size();
        for (int i = 0; i < size; ++i)
        {
            WeightsAndColour& handle(elements.getReference(i));
            g.setColour (handle.colour);

            path.clear();

            int idx=0;
            for (int phi = -180; phi <= 180; phi += degStep)
            {
                float gain = 0.0f;
                float phiInRad = (float) phi * deg2rad;
                for (int o = 0; o < 8; ++o) {
                    gain += handle.weights[o] * lookUpTables[o]->processSample(phiInRad);
                }
                Point<float> point = dBToRadius(Decibels::gainToDecibels(std::abs(gain), -1.0f * maxdB)) * pointsOnCircle[idx];
                if (phi == -180)
                    path.startNewSubPath(point);
                else
                    path.lineTo(point);
                ++idx;
            }

            path.closeSubPath();
            path.applyTransform(transform);
            g.strokePath(path, PathStrokeType(2.0f));
        }
    }

    void resized() override
    {
        Rectangle<int> bounds = getLocalBounds();
        Point<int> centre = bounds.getCentre();

        bounds.reduce(10,10);

        if (bounds.getWidth() > bounds.getHeight())
            bounds.setWidth(bounds.getHeight());
        else
            bounds.setHeight(bounds.getWidth());
        bounds.setCentre(centre);

        transform = AffineTransform::fromTargetPoints((float) centre.x, (float) centre.y, (float)  centre.x, bounds.getY(), bounds.getX(), centre.y);


        plotArea = bounds;
    }

    void addElement(float* weights, Colour colour)
    {
        elements.add({weights, colour});
    }

private:
    OwnedArray<LookupTableTransform<float>> lookUpTables;
    Path grid;
    Path subGrid;
    AffineTransform transform;
    Rectangle<int> plotArea;

    int maxOrder;

    Array<WeightsAndColour> elements;

    Array<Point<float>> pointsOnCircle;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectivityVisualizer)
};
