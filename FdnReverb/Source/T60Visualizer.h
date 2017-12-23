/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich, Sebastian Grill
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 http://www.iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */


#pragma once

class  T60Visualizer :  public Component
{
    struct Settings {
        float fMin = 20.0f;        // minimum displayed frequency
        float fMax = 20000.0f;        // maximum displayed frequency
        float yMin = 0.1f;       // min displayed seconds
        float yMax = 60.0f;       // max displayed seconds
        float gridDiv = 5.0f;     // how many seconds per divisions (between grid lines)
    };
    
    const float mL = 23.0f;
    const float mR = 10.0f;
    const float mT = 7.0f;
    const float mB = 15.0f;
    const float OH = 3.0f;
    
public:
    T60Visualizer() : Component(), overallGainInDb(0.0f), sampleRate(48000) {};
    T60Visualizer(float fMin, float fMax, float yMin, float yMax, float gridDiv) : Component(), sampleRate(48000), s{fMin, fMax, yMin, yMax, gridDiv} {};
    ~T60Visualizer() {};
    

    void paint (Graphics& g) override
    {
        g.setColour(Colours::steelblue.withMultipliedAlpha(0.01f));
        g.fillAll();
        
        //int width = getWidth();
        
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 2)));
        g.setFont(12.0f);

        // time labels
        g.setColour (Colours::white);

        String axislabel = String (0.1f);
        g.drawText (axislabel, 0, t60ToY (0.1f) - 6, 18, 12.0f, Justification::right, false);

        axislabel = String (1.f);
        g.drawText (axislabel, 0, t60ToY (1.f) - 6, 18, 12.0f, Justification::right, false);

        for (float t=s.yMin; t <= s.yMax; t += powf(10, floorf(log10(t)))) {
            int ypos = t60ToY (t);
            
            String axislabel;
            bool drawText = false;

            if ((t == 1) || (t == 10) || (t==60))
            {
                axislabel = String((int)t);
                drawText = true;
            }
            if (drawText)
            {
                g.drawText (axislabel, 0, ypos-6, 18, 12.0f, Justification::right, false);
            }
        }

        // frequncy labels
        for (float f=s.fMin; f <= s.fMax; f += powf(10, floorf(log10(f)))) {
            int xpos = hzToX(f);
            
            String axislabel;
            bool drawText = false;
            
            if ((f == 20) || (f == 50) || (f == 100) || (f == 500))
            {
                axislabel = String((int)f);
                drawText = true;
            }
            else if ((f == 1000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                axislabel = String((int)f/1000);
                axislabel << "k";
                drawText = true;
            }
            
            if (drawText)
            {
                g.drawText (axislabel, xpos - 10, t60ToY(s.yMin) + OH + 0.0f, 20, 12, Justification::centred, false);
            }
        }


        g.setColour (Colours::steelblue.withMultipliedAlpha(0.8f));
        g.strokePath (dbGridPath, PathStrokeType (0.5f));

        g.setColour (Colours::steelblue.withMultipliedAlpha(0.9f));
        g.strokePath (dbGridPathBold, PathStrokeType (1.0f));

        g.setColour(Colours::steelblue.withMultipliedAlpha(0.9f));
        g.strokePath (hzGridPathBold, PathStrokeType (1.0f));

        g.setColour(Colours::steelblue.withMultipliedAlpha(0.8f));
        g.strokePath (hzGridPath, PathStrokeType (0.5f));
               
        
        // draw filter magnitude responses
        Path magnitude;
        allMagnitudesInDb.fill(overallGainInDb);
        
        int xMin = hzToX(s.fMin);
        int xMax = hzToX(s.fMax);
        int yMax = t60ToY(s.yMin);
        int yMin = t60ToY(s.yMax);
        
        g.excludeClipRegion(Rectangle<int>(0.0f, yMax+OH, getWidth(), getHeight()-yMax-OH));
        
        for (int i = arrayOfCoefficients.size (); --i >= 0;) {
            //bool isActive = activeElem == i;

            dsp::IIR::Coefficients<float>::Ptr handle = (dsp::IIR::Coefficients<float>::Ptr) arrayOfCoefficients.getUnchecked (i);
            magnitude.clear();

            float db = Decibels::gainToDecibels(handle->getMagnitudeForFrequency(xToHz(xMin), sampleRate));
            allMagnitudesInDb.setUnchecked(0, allMagnitudesInDb[0] + db);
            //magnitude.startNewSubPath(xMin, jlimit((float) yMin, (float) yMax + OH + 1, t60ToYFloat (gainToT60Float (db))));

            for (int x = xMin+1; x<=xMax; ++x)
            {
                float db = Decibels::gainToDecibels(handle->getMagnitudeForFrequency(xToHz(x), sampleRate));
                allMagnitudesInDb.setUnchecked(x-xMin, allMagnitudesInDb[x-xMin] + db);
                //DBG (gainToT60Float (db));
                //float y = jlimit((float) yMin, (float) yMax + OH + 1, t60ToYFloat (gainToT60Float (db)));
                //magnitude.lineTo(x, y);
            }

            // g.setColour(arrayOfColours[i].withMultipliedAlpha(0.5f));
            // g.strokePath(magnitude, PathStrokeType(isActive ? 2.5f : 0.9f));

            // magnitude.lineTo(xMax, yZero);
            // magnitude.lineTo(xMin, yZero);
            // magnitude.closeSubPath();
            // g.setColour(arrayOfColours[i].withMultipliedAlpha(0.3f));
            // g.fillPath(magnitude);
        }
        
        //all magnitudes combined
        magnitude.clear();
        
        magnitude.startNewSubPath(xMin, jlimit((float) yMin, (float) yMax + OH + 1, t60ToYFloat (gainToT60Float (allMagnitudesInDb[0]))));
        
        for (int x = xMin + 1; x<=xMax; ++x)
        {
            magnitude.lineTo(x, jlimit((float) yMin, (float) yMax + OH + 1, t60ToYFloat (gainToT60Float (allMagnitudesInDb[x-xMin]))));
        }
        g.setColour(Colours::white);
        g.strokePath(magnitude, PathStrokeType(1.5f));

        magnitude.clear();
        float tolerance;
        tolerance = getToleranceT60 (xToHz (xMin), std::string ("upper"));
        magnitude.startNewSubPath(xMin, jlimit((float) yMin, (float) yMax + OH + 1, t60ToYFloat (tolerance)))
        ;
        for (int x = xMin + 1; x<=xMax; ++x)
        {   
            tolerance = getToleranceT60 (xToHz (x), std::string ("upper"));
            magnitude.lineTo (x, t60ToYFloat (tolerance));
        }

        for (int x = xMax; x>=xMin; --x)
        {   
            tolerance = getToleranceT60 (xToHz (x), std::string ("lower"));
            magnitude.lineTo (x, t60ToYFloat (tolerance));
        }
        magnitude.closeSubPath();
        g.setColour(Colours::green.withMultipliedAlpha(0.3f));
        g.fillPath(magnitude);

        // magnitude.lineTo(xMax, yZero);
        // magnitude.lineTo(xMin, yZero);
        // magnitude.closeSubPath();
        // g.setColour(Colours::white.withMultipliedAlpha(0.3f));
        // g.fillPath(magnitude);
        
        // for (int i = arrayOfCoefficients.size (); --i >= 0;) {
        //     float circX = arrayOfFrequencySliders[i] == nullptr ? hzToX(0.0f) : hzToX(arrayOfFrequencySliders[i]->getValue());
        //     float circY = arrayOfGainSliders[i] == nullptr ? t60ToY(0.0f) : t60ToY (arrayOfGainSliders[i]->getValue());
            
        //     g.setColour(Colour(0xFF191919));
        //     g.drawEllipse(circX - 5.0f, circY - 5.0f , 10.0f, 10.0f, 3.0f);
            
        //     g.setColour(arrayOfColours[i]);
        //     g.drawEllipse(circX - 5.0f, circY - 5.0f , 10.0f, 10.0f, 1.0f);
        //     g.setColour(activeElem == i ? arrayOfColours[i] : arrayOfColours[i].withSaturation(0.2));
        //     g.fillEllipse(circX - 5.0f, circY - 5.0f , 10.0f, 10.0f);
        // }
    };

    float getToleranceT60 (float frequency, std::string which)
    {   
        float tempT60;
        // if (frequency < 250)
        //     {
        //         if (which == std::string ("upper"))
        //         {
        //             tempT60 = gainToT60Float (overallGainInDb);
        //             return tempT60 * (frequency * -0.02306405f + 0.246964856f);
        //         }
        //         else
        //         {
        //             tempT60 = gainToT60Float (overallGainInDb);
        //             return tempT60 * (frequency * -0.001953034f + 1.334759358);
        //         }
        //     }
        // else if (frequency < 2000)
        // {
            if (which == std::string ("upper"))
                return gainToT60Float (overallGainInDb) * 1.2f;
            else
                return gainToT60Float (overallGainInDb) * 0.8f;
        // }
        // else
        // {
        //     if (which == std::string ("upper"))
        //         return gainToT60Float (overallGainInDb) * 1.2f;
        //     else
        //         tempT60 = gainToT60Float (overallGainInDb);
        //         return tempT60 * (frequency * -0.00005f + 0.9f);
        // }
    }
    
    float gainToT60Float (float db)
    {
        return -3.f * 20.f / db;
    }

    float t60ToGain (float T60)
    {
        return -3.f * 20.f / T60;
    }
    
    int t60ToY(float db)
    {
        int ypos = t60ToYFloat(db);
        return ypos;
    }
    
    float t60ToYFloat(float db)
    {
        float height = (float) getHeight() - mB - mT;
        float ypos = mT + height * (1.f - (log (db / s.yMin) / log (s.yMax / s.yMin)));
        return ypos;
    }
    
    float yToT60 (float y)
    {
        float height = (float) getHeight() - mB - mT;
        float db = s.yMin * powf (s.yMax / s.yMin, 1.f - (y - mT) / height);
        return db;
    }
    
    int hzToX(float hz)
    {
        float width = (float) getWidth() - mL - mR;
        int xpos = mL + width * (log(hz/s.fMin) / log(s.fMax/s.fMin));
        return xpos;
    }
    
    float xToHz(int x)
    {
        float width = (float) getWidth() - mL - mR;
        return s.fMin * powf ((s.fMax / s.fMin), ((x - mL) / width));
    }

    void setSampleRate(int newSampleRate) {
        sampleRate = newSampleRate;
    }
    
    void setOverallGain(float newGain) {
        overallGainInDb = Decibels::gainToDecibels(newGain, -500.0f);
    }
    
    void mouseDrag(const MouseEvent &event) override
    {
        Point<int> pos = event.getPosition();
        float frequency = xToHz (pos.x);
        float gain = yToT60 (pos.y);
        
        if (activeElem != -1)
        {
            Slider* slHandle;
            
            slHandle = arrayOfFrequencySliders[activeElem];
            if (slHandle != nullptr)
                slHandle->setValue(frequency);
            
            slHandle = arrayOfGainSliders[activeElem];
            if (slHandle != nullptr)
                slHandle->setValue(gain);
        }
    }
    
    void mouseMove(const MouseEvent &event) override
    {
        Point<int> pos = event.getPosition();
        int oldActiveElem = activeElem;
        activeElem = -1;
        for (int i = arrayOfCoefficients.size (); --i >= 0;)
        {
            Point<int> filterPos (arrayOfFrequencySliders[i] == nullptr ? hzToX(0.0f) : hzToX(arrayOfFrequencySliders[i]->getValue()), arrayOfGainSliders[i] == nullptr ? t60ToY(0.0f) : t60ToY(arrayOfGainSliders[i]->getValue()));
            if (pos.getDistanceSquaredFrom(filterPos) < 45) {
                activeElem = i;
                break;
            }
        }
        
        if (oldActiveElem != activeElem)
            repaint();
        
    }
    
    void resized() override {

        int xMin = hzToX(s.fMin);
        int xMax = hzToX(s.fMax);
        allMagnitudesInDb.resize(xMax - xMin + 1);
        
        const float width = getWidth() - mL - mR;
        dbGridPath.clear();
        dbGridPathBold.clear();

        dbGridPathBold.startNewSubPath(mL-OH, t60ToY (0.1f));
        dbGridPathBold.lineTo(mL + width+OH, t60ToY (0.1f));

        dbGridPathBold.startNewSubPath(mL-OH, t60ToY (1.f));
        dbGridPathBold.lineTo(mL + width+OH, t60ToY (1.f));

        for (float t=s.yMin; t <= s.yMax; t += powf(10, floorf(log10(t)))) {
            int ypos = t60ToY (t);

            if ((t == 10) || (t==60))
            {
                dbGridPathBold.startNewSubPath(mL-OH, ypos);
                dbGridPathBold.lineTo(mL + width+OH, ypos);
            } else
            {
                dbGridPath.startNewSubPath(mL-OH, ypos);
                dbGridPath.lineTo(mL + width+OH, ypos);
            }
        }
        
        hzGridPath.clear();
        hzGridPathBold.clear();
        
        for (float f=s.fMin; f <= s.fMax; f += powf(10, floorf(log10(f)))) {
            int xpos = hzToX(f);
            
            if ((f == 20) || (f == 50) || (f == 100) || (f == 500) || (f == 1000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                hzGridPathBold.startNewSubPath(xpos, t60ToY(s.yMax)-OH);
                hzGridPathBold.lineTo(xpos, t60ToY(s.yMin)+OH);
                
            } else
            {
                hzGridPath.startNewSubPath(xpos, t60ToY(s.yMax)-OH);
                hzGridPath.lineTo(xpos, t60ToY(s.yMin)+OH);
            }
        }
    }
    
    void addCoefficients(dsp::IIR::Coefficients<float>::Ptr newCoeffs, Colour newColourForCoeffs, Slider* frequencySlider = nullptr, Slider* gainSlider = nullptr)
    {
        arrayOfCoefficients.add(newCoeffs);
        arrayOfColours.add(newColourForCoeffs);
        arrayOfGainSliders.add(gainSlider);
        arrayOfFrequencySliders.add(frequencySlider);
    }
    
private:
    
    float overallGainInDb;
    
    int sampleRate;
    
    int activeElem = 0;
    
    Settings s;
    Path dbGridPath;
    Path dbGridPathBold;
    Path hzGridPath;
    Path hzGridPathBold;
    
    Array<float> allMagnitudesInDb;
    Array<dsp::IIR::Coefficients<float>::Ptr> arrayOfCoefficients;
    Array<Slider*> arrayOfGainSliders, arrayOfFrequencySliders;
    
    Array<Colour> arrayOfColours;

};
