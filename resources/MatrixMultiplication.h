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
#include "ReferenceCountedMatrix.h"
#include "ReferenceCountedDecoder.h"

using namespace dsp;
class MatrixMultiplication
{
public:
    MatrixMultiplication() {
    }

    ~MatrixMultiplication() {}

    void prepare (const ProcessSpec& newSpec)
    {
        spec = newSpec;

        buffer.setSize(buffer.getNumChannels(), spec.maximumBlockSize);

        //inputMatrix.resize(Eigen::NoChange, spec.maximumBlockSize);
        checkIfNewMatrixAvailable();
    }

    void process (const ProcessContextNonReplacing<float>& context)
    {
        ScopedNoDenormals noDenormals;
        checkIfNewMatrixAvailable();

        ReferenceCountedMatrix::Ptr retainedCurrentMatrix (currentMatrix);
        if (retainedCurrentMatrix == nullptr)
        {
            context.getOutputBlock().clear();
            return;
        }

        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        auto& T = retainedCurrentMatrix->getMatrix();

        const int nInputChannels = jmin( (int) inputBlock.getNumChannels(), (int) T.getNumColumns());
        const int nSamples = (int) inputBlock.getNumSamples();

        // copy input data to buffer
        for (int ch = 0; ch < nInputChannels; ++ch)
            buffer.copyFrom(ch, 0, inputBlock.getChannelPointer(ch), nSamples);

        for (int row = 0; row < T.getNumRows(); ++row)
        {
            const int destCh = retainedCurrentMatrix->getRoutingArrayReference().getUnchecked(row);
            if (destCh < outputBlock.getNumChannels())
            {
                float* dest = outputBlock.getChannelPointer(destCh);
                FloatVectorOperations::multiply(dest, buffer.getReadPointer(0), T(row, 0), nSamples); // ch 0
                for (int i = 1; i < nInputChannels; ++i) // input channels
                    FloatVectorOperations::addWithMultiply(dest, buffer.getReadPointer(i), T(row, i), nSamples); // ch 0
            }
        }

        Array<int> routingCopy (retainedCurrentMatrix->getRoutingArrayReference());
        routingCopy.sort();
        int lastDest = -1;
        const int nElements = routingCopy.size();
        for (int i = 0; i < nElements; ++i)
        {
            const int destCh = routingCopy[i];
            for (; ++lastDest < destCh;)
                if (lastDest < outputBlock.getNumChannels())
                    FloatVectorOperations::clear(outputBlock.getChannelPointer(lastDest), (int) outputBlock.getNumSamples());
            lastDest = destCh;
        }

        for (int ch = routingCopy.getLast() + 1; ch < outputBlock.getNumChannels(); ++ch)
            FloatVectorOperations::clear(outputBlock.getChannelPointer(ch), (int) outputBlock.getNumSamples());
    }

    const bool checkIfNewMatrixAvailable()
    {
        if (newMatrixAvailable)
        {
            if (newMatrix == nullptr)
            {
                DBG("MatrixTransformer: Matrix set to nullptr");
            }
            else
            {
                DBG("MatrixTransformer: New matrix with name '" << newMatrix->getName() << "' set.");
                //const int rows = (int) newMatrix->getMatrix().getNumRows();
                const int cols = (int) newMatrix->getMatrix().getNumColumns();
                buffer.setSize(cols, buffer.getNumSamples());
                DBG("MatrixTransformer: buffer resized to " << buffer.getNumChannels() << "x" << buffer.getNumSamples());
            }

            currentMatrix = newMatrix;
            newMatrix = nullptr;
            newMatrixAvailable = false;
            return true;
        }
        return false;
    };

    void setMatrix(ReferenceCountedMatrix::Ptr newMatrixToUse, bool force = false)
    {
        newMatrix = newMatrixToUse;
        newMatrixAvailable = true;
        if (force)
            checkIfNewMatrixAvailable();
    }

    ReferenceCountedMatrix::Ptr getMatrix()
    {
        return currentMatrix;
    }

private:
    //==============================================================================
    ProcessSpec spec = {-1, 0, 0};
    ReferenceCountedMatrix::Ptr currentMatrix {nullptr};
    ReferenceCountedMatrix::Ptr newMatrix {nullptr};
    AudioBuffer<float> buffer;
    bool newMatrixAvailable {false};

};
