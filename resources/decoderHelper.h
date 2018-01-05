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
    
    static Result parseJsonFile (const File& fileToParse, var& dest)
    {
        if (!fileToParse.exists())
            return Result::fail("File '" + fileToParse.getFullPathName() + "' does not exist!");
        
        String jsonString = fileToParse.loadFileAsString();
        Result result = JSON::parse (jsonString, dest);
        if (!result.wasOk())
            return Result::fail("File '" + fileToParse.getFullPathName() + "' could not be parsed:\n" + result.getErrorMessage());
        
        return Result::ok();
    }
    
    static Result transformationMatrixVarToMatrix (var& tmVar, ReferenceCountedMatrix::Ptr* matrix, var nameFallback = var(""), var descriptionFallback = var(""))
    {
        String name = tmVar.getProperty(Identifier("name"), nameFallback);
        String description = tmVar.getProperty(Identifier("description"), descriptionFallback);
        
        if (! tmVar.hasProperty("matrix"))
            return Result::fail("There is no 'matrix' array.");
        
        int rows, cols;
        var matrixData = tmVar.getProperty("matrix", var());
        Result result = getMatrixDataSize (matrixData, rows, cols);

        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        
        ReferenceCountedMatrix::Ptr newMatrix = new ReferenceCountedMatrix(name, description, rows, cols);
        for (int r = 0; r < rows; ++r)
        {
            var rowVar = matrixData.getArray()->getUnchecked(r);
            if (rowVar.size() != cols)
                return Result::fail("Matrix rows differ in size.");
            
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
    }
    
    static Result getMatrixDataSize (var& matrixData, int& rows, int& cols)
    {
        rows = matrixData.size();
        cols = matrixData.getArray()->getUnchecked(0).size();
        
        return Result::ok();
    }
    
    
    static Result parseFileForTransformationMatrix (const File& fileToParse, ReferenceCountedMatrix::Ptr* matrix)
    {
        // parse configuration file
        var parsedJson;
        {
        Result result = parseJsonFile(fileToParse, parsedJson);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        }
        
        // looks for 'TransformationMatrix' object; if missing, it uses the 'root' to look for a 'matrix' object
        var tmVar = parsedJson.getProperty("TransformationMatrix", parsedJson);
        Result result = transformationMatrixVarToMatrix (tmVar, matrix,
                                                         parsedJson.getProperty("name", var("")), parsedJson.getProperty("description", var("")));
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        return Result::ok();
    }
};
