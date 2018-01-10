/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich
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

#include "../JuceLibraryCode/JuceHeader.h"
#include "ReferenceCountedDecoder.h"
#include "MatrixMultiplication.h"
#include "ambisonicTools.h"

using namespace dsp;
class AmbisonicDecoder : private ProcessorBase
{
public:
    AmbisonicDecoder() {
    }
    
    ~AmbisonicDecoder() {}
    
    void prepare (const ProcessSpec& newSpec) override {
        spec = newSpec;
        matMult.prepare(newSpec);
        
        checkIfNewDecoderAvailable();
    }
    
    void process (const ProcessContextReplacing<float>& context) override {
        ScopedNoDenormals noDenormals;
        checkIfNewDecoderAvailable();
        
        // TODO: preprocessing: weights, normalization
        
        matMult.process(context);
    }
    
    void reset() override {};
    
    bool checkIfNewDecoderAvailable() {
        if (newDecoderAvailable)
        {
            currentDecoder = newDecoder;
            matMult.setMatrix(currentDecoder, true);
            
            newDecoder = nullptr;
            newDecoderAvailable = false;
            return true;
        }
        return false;
    };
    
    void setDecoder(ReferenceCountedDecoder::Ptr newDecoderToUse) {
        newDecoder = newDecoderToUse;
        newDecoderAvailable = true;
    }
    
    ReferenceCountedDecoder::Ptr getCurrentDecoder() {
        return currentDecoder;
    }
    
private:
    //==============================================================================
    ProcessSpec spec = {-1, 0, 0};
    ReferenceCountedDecoder::Ptr currentDecoder {nullptr};
    ReferenceCountedDecoder::Ptr newDecoder {nullptr};
    
    MatrixMultiplication matMult;
    bool newDecoderAvailable {false};
};
