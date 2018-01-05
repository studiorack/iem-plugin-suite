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
#include "ReferenceCountedMatrix.h"

using namespace dsp;
class MatrixTransformer : private ProcessorBase
{
public:
    MatrixTransformer() {
    }
    
    ~MatrixTransformer() {}
    
    void prepare (const ProcessSpec& newSpec) override {
        spec = newSpec;
        
        bufferMatrix.resize(Eigen::NoChange, spec.maximumBlockSize);
        checkIfNewMatrixAvailable();
    }
    
    void process (const ProcessContextReplacing<float>& context) override {
        ScopedNoDenormals noDenormals;
        checkIfNewMatrixAvailable();
        
        ReferenceCountedMatrix::Ptr retainedCurrentMatrix (currentMatrix);
        if (retainedCurrentMatrix == nullptr)
        {
            context.getOutputBlock().clear();
            return;
        }
        
        
        auto& inputBlock = context.getInputBlock();
        auto& T = *retainedCurrentMatrix->getMatrix();
        
        const int nInputChannels = jmin( (int) inputBlock.getNumChannels(), (int) T.cols());
        
        Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> inpMatrix (inputBlock.getChannelPointer(0), nInputChannels, inputBlock.getNumSamples());
        
        //DBG("sizes: " << bufferMatrix.rows() << "x" << bufferMatrix.cols() << " is " << T.rows() << "x" << T.cols() << " times " << inpMatrix.rows() << "x" << inpMatrix.cols());
        bufferMatrix = T * inpMatrix; // maybe get subBlock from bufferMatrix for blocksize different from specs
        
        auto& outputBlock = context.getOutputBlock();
        
        const int nCH = jmin((int) outputBlock.getNumChannels(), (int) bufferMatrix.rows());
        
        
        for (int ch = 0; ch < nCH; ++ch)
            FloatVectorOperations::copy(outputBlock.getChannelPointer(ch), bufferMatrix.data() + ch*spec.maximumBlockSize, spec.maximumBlockSize);
        for (int ch = nCH; ch < outputBlock.getNumChannels(); ++ch)
            FloatVectorOperations::clear(outputBlock.getChannelPointer(ch), (int) outputBlock.getNumSamples());
    }
    
    void reset() override {};
    
    void checkIfNewMatrixAvailable() {
        if (newMatrixAvailable)
        {
            if (newMatrix == nullptr)
            {
                DBG("MatrixTransformer: Matrix set to nullptr");
            }
            else
            {
                DBG("MatrixTransformer: New matrix with name '" << newMatrix->getName() << "' set.");
                bufferMatrix.resize(newMatrix->getMatrix()->rows(), Eigen::NoChange);
                DBG("MatrixTransformer: buffer resized to " << bufferMatrix.rows() << "x" << bufferMatrix.cols());
            }
            
            currentMatrix = newMatrix;
            newMatrix = nullptr;
            newMatrixAvailable = false;
        }
    };
    
    void setMatrix(ReferenceCountedMatrix::Ptr newMatrixToUse) {
        newMatrix = newMatrixToUse;
        newMatrixAvailable = true;
    }
    
private:
    //==============================================================================
    ProcessSpec spec = {-1, 0, 0};
    ReferenceCountedMatrix::Ptr currentMatrix {nullptr};
    ReferenceCountedMatrix::Ptr newMatrix {nullptr};
    
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> bufferMatrix;
    bool newMatrixAvailable {false};
    
    
};
