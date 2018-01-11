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
#include "ReferenceCountedDecoder.h"

using namespace dsp;
class MatrixMultiplication : private ProcessorBase
{
public:
    MatrixMultiplication() {
    }
    
    ~MatrixMultiplication() {}
    
    void prepare (const ProcessSpec& newSpec) override {
        spec = newSpec;
        
        bufferMatrix.resize(Eigen::NoChange, spec.maximumBlockSize);
        bufferMatrix.setZero();
        
        inputMatrix.resize(Eigen::NoChange, spec.maximumBlockSize);
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
        
        // make contiguous-memory copy of input data
        for (int ch = 0; ch < nInputChannels; ++ch)
            inputMatrix.row(ch) = Eigen::VectorXf::Map(inputBlock.getChannelPointer(ch), inputBlock.getNumSamples());
        
        bufferMatrix.noalias() = T.block(0, 0, T.rows(), nInputChannels) * inputMatrix.block(0, 0, nInputChannels, inputBlock.getNumSamples()); // maybe get subBlock from bufferMatrix for blocksize different from specs

        auto& outputBlock = context.getOutputBlock();
        const int nChOut = jmin((int) outputBlock.getNumChannels(), (int) bufferMatrix.rows());
        
        int lastDest = -1;
        for (int ch = 0; ch < nChOut; ++ch)
        {
            const int destCh = retainedCurrentMatrix->getRoutingArrayReference().getUnchecked(ch);
            if (destCh < outputBlock.getNumChannels())
                FloatVectorOperations::copy(outputBlock.getChannelPointer(destCh), bufferMatrix.data() + ch*spec.maximumBlockSize, spec.maximumBlockSize);
        
            for (; ++lastDest < destCh;)
            {
                if (lastDest < outputBlock.getNumChannels())
                    FloatVectorOperations::clear(outputBlock.getChannelPointer(lastDest), (int) outputBlock.getNumSamples());
            }
            lastDest = destCh;
        }
        
        for (int ch = ++lastDest; ch < outputBlock.getNumChannels(); ++ch)
            FloatVectorOperations::clear(outputBlock.getChannelPointer(ch), (int) outputBlock.getNumSamples());
    }
    
    void reset() override {};
    
    bool checkIfNewMatrixAvailable() {
        if (newMatrixAvailable)
        {
            if (newMatrix == nullptr)
            {
                DBG("MatrixTransformer: Matrix set to nullptr");
            }
            else
            {
                DBG("MatrixTransformer: New matrix with name '" << newMatrix->getName() << "' set.");
                const int rows = (int) newMatrix->getMatrix()->rows();
                const int cols = (int) newMatrix->getMatrix()->cols();
                bufferMatrix.resize(rows, Eigen::NoChange);
                bufferMatrix.setZero();
                inputMatrix.resize(cols, Eigen::NoChange);
                DBG("MatrixTransformer: buffer resized to " << bufferMatrix.rows() << "x" << bufferMatrix.cols());
            }
            
            currentMatrix = newMatrix;
            newMatrix = nullptr;
            newMatrixAvailable = false;
            return true;
        }
        return false;
    };
    
    void setMatrix(ReferenceCountedMatrix::Ptr newMatrixToUse, bool force = false) {
        newMatrix = newMatrixToUse;
        newMatrixAvailable = true;
        if (force)
            checkIfNewMatrixAvailable();
    }
    
    ReferenceCountedMatrix::Ptr getMatrix()
    {
        return currentMatrix;
    }
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
    //==============================================================================
    ProcessSpec spec = {-1, 0, 0};
    ReferenceCountedMatrix::Ptr currentMatrix {nullptr};
    ReferenceCountedMatrix::Ptr newMatrix {nullptr};
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> bufferMatrix;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> inputMatrix;
    bool newMatrixAvailable {false};
    
};
