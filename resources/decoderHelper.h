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
#include "ReferenceCountedDecoder.h"
#include "ambisonicTools.h"

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
        String name = tmVar.getProperty(Identifier("Name"), nameFallback);
        String description = tmVar.getProperty(Identifier("Description"), descriptionFallback);
        
        if (! tmVar.hasProperty("Matrix"))
            return Result::fail("There is no 'Matrix' array.");
        
        int rows, cols;
        var matrixData = tmVar.getProperty("Matrix", var());
        Result result = getMatrixDataSize (matrixData, rows, cols);
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        
        ReferenceCountedMatrix::Ptr newMatrix = new ReferenceCountedMatrix(name, description, rows, cols);
        result = getMatrix(matrixData, rows, cols, newMatrix->getMatrix());
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        *matrix = newMatrix;
        return Result::ok();
    }
    
    static Result decoderObjectToDecoder (var& decoderObject, ReferenceCountedDecoder::Ptr* decoder, var nameFallback = var(""), var descriptionFallback = var(""))
    {
        String name = decoderObject.getProperty(Identifier("Name"), nameFallback);
        String description = decoderObject.getProperty(Identifier("Description"), descriptionFallback);
        
        if (! decoderObject.hasProperty("Matrix"))
            return Result::fail("There is no 'Matrix' array within the 'Decoder' object.");
        
        // get matrix size
        int rows, cols;
        var matrixData = decoderObject.getProperty("Matrix", var());
        Result result = getMatrixDataSize (matrixData, rows, cols);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        //check if cols is a valid number of Ambisonic channels
        const int decoderOrder = isqrt(cols) - 1;
        if (cols != square(decoderOrder + 1))
            return Result::fail("Decoder matrix's number of columns is no valid Ambisonic channel count: nCh = (order+1)^2.");
        
        // create decoder and get matrix from 'Decoder' object
        ReferenceCountedDecoder::Ptr newDecoder = new ReferenceCountedDecoder(name, description, rows, cols);
        result = getMatrix(matrixData, rows, cols, newDecoder->getMatrix());
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        if (decoderObject.hasProperty("Routing"))
        {
            var routingData = decoderObject.getProperty("Routing", var());
            result = getRoutingArray(routingData, rows, newDecoder);
            if (! result.wasOk())
                return Result::fail(result.getErrorMessage());
        }
        
        
        // ============ SETTINGS =====================
        ReferenceCountedDecoder::Settings settings;
        // normalization
        if ( !decoderObject.hasProperty("ExpectedInputNormalization"))
            Result::fail("Could not find 'ExpectedInputNormalization' attribute.");
        
        
        var expectedNormalization (decoderObject.getProperty("ExpectedInputNormalization", var()));
        if (expectedNormalization.toString().equalsIgnoreCase("sn3d"))
            settings.expectedNormalization = ReferenceCountedDecoder::Normalization::sn3d;
        else if (expectedNormalization.toString().equalsIgnoreCase("n3d"))
            settings.expectedNormalization = ReferenceCountedDecoder::Normalization::n3d;
        else
            return Result::fail("Could not parse 'ExpectedInputNormalization' attribute. Expected 'sn3d' or 'n3d' but got '" + expectedNormalization.toString() + "'.");
        
        
        // weights
        if (decoderObject.hasProperty("Weights"))
        {
            var weights (decoderObject.getProperty("Weights", var()));
            if (weights.toString().equalsIgnoreCase("maxrE"))
                settings.weights = ReferenceCountedDecoder::Weights::maxrE;
            else if (weights.toString().equalsIgnoreCase("none"))
                settings.weights = ReferenceCountedDecoder::Weights::none;
            else
                return Result::fail("Could not parse 'Weights' attribute. Expected 'maxrE', 'inPhase' or 'none' but got '" + weights.toString() + "'.");
        }
        // weights already applied
        if (decoderObject.hasProperty("WeightsAlreadyApplied"))
        {
            var weightsAlreadyApplied (decoderObject.getProperty("WeightsAlreadyApplied", var()));
            if (weightsAlreadyApplied.isBool())
                settings.weightsAlreadyApplied = weightsAlreadyApplied;
            else
                return Result::fail("Could not parse 'WeightsAlreadyApplied' attribute. Expected bool but got '" + weightsAlreadyApplied.toString() + "'.");
        }
        if (decoderObject.hasProperty("SubwooferChannel"))
        {
            var subwooferChannel (decoderObject.getProperty("SubwooferChannel", var()));
            if (subwooferChannel.isInt())
            {
                if (static_cast<int>(subwooferChannel) < 1 || static_cast<int>(subwooferChannel) > 64)
                    return Result::fail("'SubwooferChannel' attribute is not a valid channel number (1<=subwooferChannel>=64).");
                
                settings.subwooferChannel = subwooferChannel;
            }
            else
                return Result::fail("Could not parse 'SubwooferChannel' attribute. Expected channel number (int) but got '" + subwooferChannel.toString() + "'.");
        }
        
        newDecoder->setSettings(settings);
        
        *decoder = newDecoder;
        return Result::ok();
    }
    
    static Result getMatrixDataSize (var& matrixData, int& rows, int& cols)
    {
        rows = matrixData.size();
        cols = matrixData.getArray()->getUnchecked(0).size();
        
        return Result::ok();
    }
    
    // call getMatrixDataSize() before and create the 'dest' matrix with the resulting size
    static Result getMatrix (var&  matrixData, const int rows, const int cols, Matrix<float>& dest)
    {
        for (int r = 0; r < rows; ++r)
        {
            var rowVar = matrixData.getArray()->getUnchecked(r);
            if (rowVar.size() != cols)
                return Result::fail("Matrix row " + String(r+1) + " has wrong length (should be " + String(cols) + ").");
            
            for (int c = 0; c < cols; ++c)
            {
                var colVar = rowVar.getArray()->getUnchecked(c);
                if (colVar.isDouble() || colVar.isInt())
                {
                    dest(r, c) = colVar;
                }
                else
                    return Result::fail("Datatype of matrix element (" + String(r+1) + "," + String(c+1) + ") could not be parsed.");
            }
            
        }
        return Result::ok();
    }
    
    
    static Result getRoutingArray (var& routingData, const int rows, ReferenceCountedMatrix::Ptr dest)
    {
        if (routingData.size() != rows)
            return Result::fail("Length of 'Routing' attribute does not match number of matrix outputs (rows).");
        
        Array<int>& routingArray = dest->getRoutingArrayReference();
        for (int r = 0; r < rows; ++r)
        {
            var element = routingData.getArray()->getUnchecked(r);
            
            if (element.isInt())
            {
                routingArray.set(r, (int) element - 1);
            }
            else
                return Result::fail("Datatype of 'Routing' element at position " + String(r+1) + " could not be interpreted (expected integer).");
        }
        return Result::ok();
    }
    
    
    static Result parseFileForDecoder (const File& fileToParse, ReferenceCountedDecoder::Ptr* decoder)
    {
        // parse configuration file
        var parsedJson;
        Result result = parseJsonFile(fileToParse, parsedJson);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        
        // looks for 'Decoder' object
        if (! parsedJson.hasProperty("Decoder"))
            return Result::fail("No 'Decoder' object found in the configuration file.");
        
        var decoderObject = parsedJson.getProperty("Decoder", parsedJson);
        result = decoderObjectToDecoder (decoderObject, decoder,
                                         parsedJson.getProperty("Name", var("")), parsedJson.getProperty("Description", var("")));
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
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
                                                         parsedJson.getProperty("Name", var("")), parsedJson.getProperty("Description", var("")));
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        return Result::ok();
    }
    
    static Result parseFileForLoudspeakerLayout (const File& fileToParse, ValueTree& loudspeakers, UndoManager& undoManager)
    {
        // parse configuration file
        var parsedJson;
        Result result = parseJsonFile(fileToParse, parsedJson);
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        
        // looks for 'Decoder' object
        if (! parsedJson.hasProperty("LoudspeakerLayout"))
            return Result::fail("No 'LoudspeakerLayout' object found in the configuration file.");
        
        var loudspeakerLayout = parsedJson.getProperty("LoudspeakerLayout", var());
        if (! loudspeakerLayout.hasProperty("Loudspeakers"))
            return Result::fail("No 'Loudspeakers' object found within the 'LoudspeakerLayout' attribute.");
        
        var loudspeakerArray = loudspeakerLayout.getProperty("Loudspeakers", var());
        result = addLoudspeakersToValueTree (loudspeakerArray, loudspeakers, undoManager);
        
        if (! result.wasOk())
            return Result::fail(result.getErrorMessage());
        
        return Result::ok();
    }
    
    static Result addLoudspeakersToValueTree (var& loudspeakerArray, ValueTree& loudspeakers, UndoManager& undoManager)
    {
        if (! loudspeakerArray.isArray())
            return Result::fail("'Loudspeakers' is not an array.");
        
        const int nLsps = loudspeakerArray.size();
        
        for (int i = 0; i < nLsps; ++i)
        {
            var& loudspeaker = loudspeakerArray[i];
            float azimuth, elevation, radius, gain;
            int channel;
            bool isImaginary;
            
            if (! loudspeaker.hasProperty("Azimuth"))
                return Result::fail("No 'Azimuth' attribute for loudspeaker #" + String(i+1) + ".");
            var azi = loudspeaker.getProperty("Azimuth", var());
            if (azi.isDouble() || azi.isInt())
                azimuth = azi;
            else
                return Result::fail("Wrong datatype for attribute 'Azimuth' for loudspeaker #" + String(i+1) + ".");
            
            if (! loudspeaker.hasProperty("Elevation"))
                return Result::fail("No 'Elevation' attribute for loudspeaker #" + String(i+1) + ".");
            var ele = loudspeaker.getProperty("Elevation", var());
            if (ele.isDouble() || ele.isInt())
                elevation = ele;
            else
                return Result::fail("Wrong datatype for attribute 'Elevation' for loudspeaker #" + String(i+1) + ".");
            
            if (! loudspeaker.hasProperty("Radius"))
                return Result::fail("No 'Radius' attribute for loudspeaker #" + String(i+1) + ".");
            var rad = loudspeaker.getProperty("Radius", var());
            if (rad.isDouble() || rad.isInt())
                radius = rad;
            else
                return Result::fail("Wrong datatype for attribute 'Radius' for loudspeaker #" + String(i+1) + ".");
            
            if (! loudspeaker.hasProperty("Gain"))
                return Result::fail("No 'Gain' attribute for loudspeaker #" + String(i+1) + ".");
            var g = loudspeaker.getProperty("Gain", var());
            if (g.isDouble() || g.isInt())
                gain = g;
            else
                return Result::fail("Wrong datatype for attribute 'Gain' for loudspeaker #" + String(i+1) + ".");
            
            if (! loudspeaker.hasProperty("Channel"))
                return Result::fail("No 'Channel' attribute for loudspeaker #" + String(i+1) + ".");
            var ch = loudspeaker.getProperty("Channel", var());
            if (ch.isInt())
                channel = ch;
            else
                return Result::fail("Wrong datatype for attribute 'Channel' for loudspeaker #" + String(i+1) + ".");
                
            if (! loudspeaker.hasProperty("IsImaginary"))
                return Result::fail("No 'IsImaginary' attribute for loudspeaker #" + String(i+1) + ".");
            var im = loudspeaker.getProperty("IsImaginary", var());
            if (im.isBool())
                isImaginary = im;
            else
                return Result::fail("Wrong datatype for attribute 'IsImaginary' for loudspeaker #" + String(i+1) + ".");
            
            loudspeakers.appendChild(createLoudspeaker(azimuth, elevation, radius, channel, isImaginary, gain), &undoManager);
        }
        
        return Result::ok();
    }
    
    static ValueTree createLoudspeaker (const float azimuth, const float elevation, const float radius, const int channel, const bool isImaginary, const float gain)
    {
        ValueTree newLoudspeaker ("Loudspeaker");
        
        newLoudspeaker.setProperty("Azimuth", azimuth, nullptr);
        newLoudspeaker.setProperty("Elevation", elevation, nullptr);
        newLoudspeaker.setProperty("Radius", radius, nullptr);
        newLoudspeaker.setProperty("Channel", channel, nullptr);
        newLoudspeaker.setProperty("Imaginary", isImaginary, nullptr);
        newLoudspeaker.setProperty("Gain", gain, nullptr);
        
        return newLoudspeaker;
    }
    static var convertDecoderToVar (ReferenceCountedDecoder::Ptr& decoder)
    {
        if (decoder == nullptr)
            return var();
        
        DynamicObject* obj = new DynamicObject();
        obj->setProperty("Name", decoder->getName());
        obj->setProperty("Description", decoder->getDescription());
        
        ReferenceCountedDecoder::Settings settings = decoder->getSettings();
        
        obj->setProperty("ExpectedInputNormalization", settings.expectedNormalization == ReferenceCountedDecoder::n3d ? "n3d" : "sn3d");
        
        obj->setProperty("Weights", settings.weights == ReferenceCountedDecoder::maxrE ? "maxrE" : settings.weights == ReferenceCountedDecoder::inPhase ? "inPhase" : "none");
        obj->setProperty("WeightsAlreadyApplied", var(settings.weightsAlreadyApplied));

        const int subwooferChannel = settings.subwooferChannel;
        if (subwooferChannel > 0)
          obj->setProperty("SubwooferCHannel", subwooferChannel);
        
        // decoder matrix
        var decoderMatrix;
        Matrix<float>& mat = decoder->getMatrix();
        for (int m = 0; m < mat.getSize()[0]; ++m)
        {
            var row;
            for (int n = 0; n < mat.getSize()[1]; ++n)
            {
                row.append(mat(m,n));
            }
            decoderMatrix.append(row);
        }
        
        obj->setProperty("Matrix", decoderMatrix);
        
        var routing;
        Array<int>& routingArray = decoder->getRoutingArrayReference();
        for (int i = 0; i < routingArray.size(); ++i)
            routing.append(routingArray[i] + 1); // one count
        obj->setProperty("Routing", routing);
        
        return var(obj);
    }
};
