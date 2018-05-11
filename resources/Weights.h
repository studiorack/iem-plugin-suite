/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
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
#include "ambisonicTools.h"

constexpr const static float maxRe[8][8] =
{
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 5.7754104119288496e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 7.7520766107019334e-01f, 4.0142037667287966e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 8.6155075887658639e-01f, 6.1340456518123299e-01f, 3.0643144179936538e-01f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 9.0644136637224459e-01f, 7.3245392600617265e-01f, 5.0224998490808703e-01f, 2.4736484001129033e-01f, 0.0f, 0.0f, 0.0f},
    {1.0f, 9.3263709143129281e-01f, 8.0471791647013236e-01f, 6.2909156744472861e-01f, 4.2321128963220900e-01f, 2.0719132924646289e-01f, 0.0f, 0.0f},
    {1.0f, 9.4921830632793713e-01f, 8.5152308960211620e-01f, 7.1432330396679700e-01f, 5.4794300713180655e-01f, 3.6475291657556469e-01f, 1.7813609450688817e-01f, 0.0f},
    {1.0f, 9.6036452263662697e-01f, 8.8345002450861454e-01f, 7.7381375334313540e-01f, 6.3791321433685355e-01f, 4.8368159255186721e-01f, 3.2000849790781744e-01f, 1.5616185043093761e-01f}
};

constexpr const static float inPhase[8][8] =
{
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 3.3333333333333331e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 5.0000000000000000e-01f, 1.0000000000000001e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 5.9999999999999998e-01f, 2.0000000000000001e-01f, 2.8571428571428571e-02f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 6.6666666666666663e-01f, 2.8571428571428570e-01f,  7.1428571428571425e-02f, 7.9365079365079361e-03f,  0.0f, 0.0f, 0.0f},
    {1.0f, 7.1428571428571430e-01f, 3.5714285714285715e-01f, 1.1904761904761904e-01f, 2.3809523809523808e-02f,  2.1645021645021645e-03f, 0.0f, 0.0f},
    {1.0f, 7.5000000000000000e-01f, 4.1666666666666669e-01f, 1.6666666666666666e-01f, 4.5454545454545456e-02f, 7.5757575757575760e-03f, 5.8275058275058275e-04f, 0.0f},
    {1.0f, 7.7777777777777779e-01f, 4.6666666666666667e-01f, 2.1212121212121213e-01f, 7.0707070707070704e-02f, 1.6317016317016316e-02f, 2.3310023310023310e-03f, 1.5540015540015540e-04f}
};

constexpr const static float basic[8][8] =
{
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
};

class Weights {

public:
    enum Normalization
    {
        BasicDecode,
        OnAxis,
        ConstantEnergy
    };

    static void applyNormalization (float* weights, const float order, const int decodeOrder, const Normalization normalization, const bool useSN3D = false)
    {
        float orderBlend, integer;
        orderBlend = modff(order, &integer);
        int lowerOrder = roundToInt(integer);
        if (lowerOrder == 7)
        {
            lowerOrder = 6;
            orderBlend = 1.0f;
        }
        int higherOrder = lowerOrder + 1;

        if (normalization == Normalization::BasicDecode)
        {
            const float ssqrt4PI = 3.544907701811032f;
            float correction = ssqrt4PI / (higherOrder * higherOrder + (2 * higherOrder + 1) * orderBlend);
            correction /= decodeCorrection(decodeOrder);

            //correction = decodeCorrection(order) /  decodeCorrection(decodeOrder); <- would be nice

            for (int i = 0; i <= decodeOrder; ++i)
                weights[i] *= correction;
        }
        else if (normalization == Normalization::OnAxis)
        {
            const float ssqrt4PI = 3.544907701811032f;
            float correction = ssqrt4PI / (higherOrder * higherOrder + (2 * higherOrder + 1) * orderBlend);
            correction /= decodeCorrection(decodeOrder);

            float sum = 0.0f;
            for (int i = 0; i <= decodeOrder; ++i)
                sum += (2*i + 1) * weights[i];

            float cor3;
            if (higherOrder > decodeOrder)
                cor3 = squares[decodeOrder + 1];
            else
                cor3 = squares[higherOrder] + (2 * higherOrder + 1) * orderBlend;

            //cor2 = cor3 / cor2;
            correction = correction * cor3 / sum;

            for (int i = 0; i <= decodeOrder; ++i)
                weights[i] *= correction;
        }
        else // ConstantEnergy
        {
            float sum = 0.0f;
            for (int i = 0; i <= decodeOrder; ++i)
                sum += weights[i]  * weights[i] * (2 * i + 1);
            sum = 1.0f / sqrt(sum) * (decodeOrder + 1);

            for (int i = 0; i < decodeOrder; ++i )
                weights[i] *= sum;
        }


        if (useSN3D) // apply SN3D normalization
        {
            FloatVectorOperations::multiply(weights, n3d2sn3d_short, decodeOrder);
        }
    }

    static void getWeights (const float order, const float shape, float* dest, const int decodeOrder, Normalization normalization)
    {
        float weights[8];
        for (int i = 0; i <= decodeOrder; ++i)
            weights[i] = 0.0f;

        getWeights (order, shape, weights);

        // Normalization
        applyNormalization (weights, order, decodeOrder, normalization);
        //

        for (int i = 0; i <= decodeOrder; ++i)
            dest[i] = weights[i];
    }

    /** Returns number of weights
     shape: 0: basic, 0.5: maxrE, 1.0: inPhase
     */
    static int getWeights (const float order, const float shape, float* dest)
    {
        float orderBlend, integer;
        orderBlend = modff(order, &integer);
        int lowerOrder = roundToInt(integer);

        if (lowerOrder == 7)
        {
            lowerOrder = 6;
            orderBlend = 1.0f;
        }
        int higherOrder = lowerOrder + 1;

        float tempWeights[8];

        if (shape >= 0.5f)
        {
            const float blend = shape * 2.0f - 1.0f;
            for (int i = 0; i <= higherOrder; ++i)
            {
                tempWeights[i] = (1.0f - blend) *  maxRe[lowerOrder][i] + blend * inPhase[lowerOrder][i];
                tempWeights[i] *= (1.0f - orderBlend);
                tempWeights[i] += orderBlend * ((1.0f - blend) *  maxRe[higherOrder][i] + blend * inPhase[higherOrder][i]); ;
            }
        }
        else
        {
            const float blend = shape * 2.0f;
            for (int i = 0; i <= higherOrder; ++i)
            {
                tempWeights[i] = (1.0f - blend) *  basic[lowerOrder][i] + blend * maxRe[lowerOrder][i];
                tempWeights[i] *= (1.0f - orderBlend);
                tempWeights[i] += orderBlend * ((1.0f - blend) *  basic[higherOrder][i] + blend * maxRe[higherOrder][i]); ;
            }
        }

        // copy weights to destination
        for (int i = 0; i <= higherOrder; ++i)
        {
            dest[i] = tempWeights[i];
        }

        return higherOrder + 1;
    }


private:

};
