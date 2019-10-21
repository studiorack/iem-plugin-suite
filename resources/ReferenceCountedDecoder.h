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
#include "ReferenceCountedMatrix.h"
#include "ambisonicTools.h"
#include "MaxRE.h"
#include "inPhase.h"

class ReferenceCountedDecoder : public ReferenceCountedMatrix
{
public:
    typedef ReferenceCountedObjectPtr<ReferenceCountedDecoder> Ptr;

    enum Normalization
    {
        n3d,
        sn3d
    };

    enum Weights
    {
        none,
        maxrE,
        inPhase
    };

    struct Settings {
        Normalization expectedNormalization = sn3d;
        Weights weights = none;
        bool weightsAlreadyApplied = false;
        int subwooferChannel = -1;
    };


    ReferenceCountedDecoder (const String& nameToUse, const String& descriptionToUse, int rows, int columns)
    :   ReferenceCountedMatrix(nameToUse, descriptionToUse, rows, columns), order(isqrt(columns)-1)
    {}

    ~ReferenceCountedDecoder()
    {}

    virtual String getConstructorMessage() override
    {
        return "Decoder named '" + name + "' constructed. Size: " + String(matrix.getNumRows()) + "x" + String(matrix.getNumColumns());
    }

    virtual String getDeconstructorMessage() override
    {
        return "Decoder named '" + name + "' destroyed.";
    }

    const String getName()
    {
        return name;
    }

    const String getDescription()
    {
        return description;
    }

    void setSettings (const Settings newSettings)
    {
        settings = newSettings;
    }

    const Settings getSettings()
    {
        return settings;
    }

    const String getWeightsString()
    {
        switch(settings.weights)
        {
            case 1: return String("maxrE");
            case 2: return String("inPhase");
            default: return String("none");
        }
    }

    /**
     Applies the inverse weights to the decoder matrix, so it can be used with different orders. This method has to be called before the decoder processes audio input.
    */
    void removeAppliedWeights()
    {
        if (settings.weightsAlreadyApplied && settings.weights != Weights::none)
        {
            if (settings.weights == Weights::maxrE)
                for (int i = 0; i < matrix.getNumColumns(); ++i)
                    for (int j = 0; j < matrix.getNumRows(); ++j)
                        matrix(j,i) = matrix(j,i) / getMaxRELUT(order)[i];
            else if (settings.weights == Weights::inPhase)
                for (int i = 0; i < matrix.getNumColumns(); ++i)
                    for (int j = 0; j < matrix.getNumRows(); ++j)
                        matrix(j,i) = matrix(j,i) / getInPhaseLUT(order)[i];
            settings.weightsAlreadyApplied = false;
        }
    }

    const int getOrder()
    {
        return order;
    }

private:
    Settings settings;
    const int order;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedDecoder)
};
