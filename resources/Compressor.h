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

class Compressor
{
public:
    
    Compressor()
    {
    }
    ~Compressor() {}
    
    void prepare (juce::dsp::ProcessSpec spec)
    {
        sampleRate = spec.sampleRate;
        
        alphaAttack = timeToGain(attackTime);
        alphaRelease = timeToGain(releaseTime);
        
        prepared = true;
    }
    
    void setAttackTime (float attackTimeInSeconds)
    {
        attackTime = attackTimeInSeconds;
        alphaAttack = 1.0 - timeToGain(attackTime);
    }
    
    void setReleaseTime (float releaseTimeInSeconds)
    {
        releaseTime = releaseTimeInSeconds;
        alphaRelease = 1.0 - timeToGain(releaseTime);
    }
    
    double timeToGain (float timeInSeconds) {
        return exp(-1.0/(sampleRate * timeInSeconds));
    }
    
    void setKnee (float kneeInDecibels)
    {
        knee = kneeInDecibels;
        kneeHalf = knee / 2.0f;
    }
    
    void setThreshold (float thresholdInDecibels)
    {
        threshold = thresholdInDecibels;
    }
    
    void setMakeUpGain (float makeUpGainInDecibels)
    {
        makeUpGain = makeUpGainInDecibels;
    }
    
    void setRatio (float ratio)
    {
        slope = 1.0f / ratio - 1.0f;
    }
    
    float getMaxLevelInDecibels()
    {
        return maxLevel;
    }
    
    
    void getGainFromSidechainSignal(const float* sideChainSignal, float* destination, int numSamples)
    {
        maxLevel = -INFINITY;
        for (int i = 0; i < numSamples; ++i)
        {
            // convert sample to decibels
            float levelInDecibels =  Decibels::gainToDecibels(abs(sideChainSignal[i]));
            if (levelInDecibels > maxLevel)
                maxLevel = levelInDecibels;
            // calculate overshoot and apply knee and ratio
            float overShoot = levelInDecibels - threshold;
            if (overShoot <= -kneeHalf)
                overShoot = 0.0f; //y_G = levelInDecibels;
            else if (overShoot > -kneeHalf && overShoot <= kneeHalf)
                overShoot = 0.5f * slope * square(overShoot + kneeHalf) / knee; //y_G = levelInDecibels + 0.5f * slope * square(overShoot + kneeHalf) / knee;
            else
                overShoot = slope * overShoot; //y_G = levelInDecibels + slope * overShoot;
            
            // ballistics
            float diff = overShoot - state;
            if (diff < 0.0f)
                state += alphaAttack * diff;
            else
                state += alphaRelease * diff;
            
            destination[i] = Decibels::decibelsToGain(state + makeUpGain);
        }
    }
    
    
private:
    double sampleRate {0.0};
    bool prepared;
    
    float knee {0.0f}, kneeHalf {0.0f};
    float threshold {- 10.0f};
    float attackTime {0.01};
    float releaseTime {0.15};
    float slope {0.0f};
    float makeUpGain {0.0f};
    
    float maxLevel;
    
    //state variable
    float state {0.0f};
    
    double alphaAttack;
    double alphaRelease;
};
