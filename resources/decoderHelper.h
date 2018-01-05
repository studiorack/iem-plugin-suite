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
#include "ReferenceCountedMatrix.h"

class DecoderHelper {
public:
    static Result parseFileForMatrix(const File& fileToParse, ReferenceCountedMatrix::Ptr* matrix)
    {
        
        if (!fileToParse.exists())
            return Result::fail("ERROR: File '" + fileToParse.getFullPathName() + "' does not exist!");
        
        String jsonString = fileToParse.loadFileAsString();
        var parsedJson;
        Result result = JSON::parse (jsonString, parsedJson);
        if (!result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        String name = parsedJson.getProperty(Identifier("name"), var("no Name available")).toString();
        
        var data = parsedJson.getProperty (Identifier("data"), var());
        if (!data.isArray())
            return Result::fail("ERROR: 'data' field missing or represents no proper matrix.");
        
        const int rows = data.size();
        const int cols = data.getArray()->getUnchecked(0).size();
        
        ReferenceCountedMatrix::Ptr newMatrix = new ReferenceCountedMatrix(name, rows, cols);
        for (int r = 0; r < rows; ++r)
        {
            var rowVar = data.getArray()->getUnchecked(r);
            if (rowVar.size() != cols)
                return Result::fail("ERROR: Matrix rows differ in size.");
            
            for (int c = 0; c < cols; ++c)
            {
                var colVar = rowVar.getArray()->getUnchecked(c);
                if (colVar.isDouble() || colVar.isInt())
                {
                    newMatrix->getMatrix()->coeffRef(r, c) = colVar;
                }
            }
            *matrix = newMatrix;
        }
        
        return Result::ok();
    } // method
};
