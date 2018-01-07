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
#include "Eigen/Dense"

class ReferenceCountedDecoder : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<ReferenceCountedDecoder> Ptr;
    
    enum Normalization
    {
        n3d,
        sn3d
    };
    
    struct Settings {
        Normalization expectedNormalization = sn3d;
    };
    
    ReferenceCountedDecoder (const String& nameToUse, const String& descriptionToUse, int rows, int columns)
    :   name (nameToUse), description (descriptionToUse), matrix (rows, columns)
    {
        DBG ("Decoder named '" << name << "' constructed. Size: " << rows << "x" << columns);
    }
    
    ~ReferenceCountedDecoder()
    {
        DBG (String ("Decoder named '") + name + "' destroyed.");
    }
    
    Eigen::MatrixXf* getMatrix()
    {
        return &matrix;
    }
    const String getName()
    {
        return name;
    }
    
    const String getDescription()
    {
        return description;
    }
    
    void setSettings(Settings newSettings)
    {
        settings = newSettings;
    }
    
    const Settings getSettings()
    {
        return settings;
    }

    const String getSettingsAsString()
    {
        return "Decoder expects Ambisonic input with " + String(settings.expectedNormalization == Normalization::n3d ? "N3D" : "SN3D") + " normalization.";
    }
    
    const int getNumOutputChannels()
    {
        return (int) matrix.rows();
    }
    
    const int getNumInputChannels()
    {
        return (int) matrix.cols();
    }
    
private:
    String name;
    String description;
    Eigen::MatrixXf matrix;
    Settings settings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedDecoder)
};

