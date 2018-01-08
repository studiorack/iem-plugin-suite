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

class ReferenceCountedMatrix : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<ReferenceCountedMatrix> Ptr;
    
    ReferenceCountedMatrix (const String& nameToUse, const String& descriptionToUse, int rows, int columns)
    :   name (nameToUse), description (descriptionToUse), matrix (rows, columns)
    {
        DBG ("Matrix named '" << name << "' constructed. Size: " << rows << "x" << columns);
    }
    
    ~ReferenceCountedMatrix()
    {
        DBG (String ("Matrix named '") + name + "' destroyed.");
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedMatrix)
};
