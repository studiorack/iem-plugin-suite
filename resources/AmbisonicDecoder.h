/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Authors: Daniel Rudrich
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
#include "ReferenceCountedDecoder.h"
#include "MatrixMultiplication.h"
#include "ambisonicTools.h"
#include "MaxRE.h"
#include "inPhase.h"

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

    void setInputNormalization(ReferenceCountedDecoder::Normalization newNormalization)
    {
        inputNormalization = newNormalization;
    }

    void process (const ProcessContextReplacing<float>& context) override {
        ScopedNoDenormals noDenormals;
        checkIfNewDecoderAvailable();

        ReferenceCountedDecoder::Ptr retainedDecoder = currentDecoder;
        AudioBlock<float> inputBlock = context.getInputBlock();


        const int order = isqrt((int) inputBlock.getNumChannels()) - 1;
        const int chAmbi = square(order+1);

        float weights[64];
        const float correction = (static_cast<float>(retainedDecoder->getOrder()) + 1) / (static_cast<float>(order) + 1);
        FloatVectorOperations::fill(weights, correction, chAmbi);

        if (retainedDecoder->getSettings().weights == ReferenceCountedDecoder::Weights::maxrE)
            multiplyMaxRE(order, weights);
        else if (retainedDecoder->getSettings().weights == ReferenceCountedDecoder::Weights::inPhase)
            multiplyInPhase(order, weights);

        if (retainedDecoder->getSettings().expectedNormalization != inputNormalization)
        {
            const float* conversionPtr (inputNormalization == ReferenceCountedDecoder::Normalization::sn3d ? sn3d2n3d : n3d2sn3d);
            FloatVectorOperations::multiply(weights, conversionPtr, chAmbi);
        }

        for (int ch = 0; ch < chAmbi; ++ch)
            FloatVectorOperations::multiply(inputBlock.getChannelPointer(ch), weights[ch], (int) inputBlock.getNumSamples());

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
    bool newDecoderAvailable {false};

    ReferenceCountedDecoder::Normalization inputNormalization {ReferenceCountedDecoder::Normalization:: sn3d};

    MatrixMultiplication matMult;


};
