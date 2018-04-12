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

void SHEval0(const float fX, const float fY, const float fZ, float *pSH);
void SHEval1(const float fX, const float fY, const float fZ, float *pSH);
void SHEval2(const float fX, const float fY, const float fZ, float *pSH);
void SHEval3(const float fX, const float fY, const float fZ, float *pSH);
void SHEval4(const float fX, const float fY, const float fZ, float *pSH);
void SHEval5(const float fX, const float fY, const float fZ, float *pSH);
void SHEval6(const float fX, const float fY, const float fZ, float *pSH);
void SHEval7(const float fX, const float fY, const float fZ, float *pSH);

#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257389615890312154517
#endif



// encoding and decoding with same direction and order yields the same encoded signal
constexpr float correction(int N) { return 4.0f/(N+1)/M_2_SQRTPI;};

inline void SHEval(int N, const float fX, const float fY, const float fZ, float *pSH)
{
    switch(N)
    {
        case 0: SHEval0(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(0), 1);
            break;
        case 1: SHEval1(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(1), 4);
            break;
        case 2: SHEval2(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(2), 9);
            break;
        case 3: SHEval3(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(3), 16);
            break;
        case 4: SHEval4(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(4), 25);
            break;
        case 5: SHEval5(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(5), 36);
            break;
        case 6: SHEval6(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(6), 49);
            break;
        case 7: SHEval7(fX, fY, fZ, pSH);
            FloatVectorOperations::multiply(pSH, correction(7), 64);
            break;
    }
};

inline void SHEval(int N, Vector3D<float> position, float *pSH)
{
    SHEval(N, position.x, position.y, position.z, pSH);
}
