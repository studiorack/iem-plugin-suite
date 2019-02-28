/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich / Markus Huber
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
#include <set>

// ============================

struct Settings
{
    float fMin = 20.0f;    // minimum displayed frequency
    float fMax = 20000.0f; // maximum displayed frequency
    float dbMin = -15.0f;  // min displayed dB
    float dbMax = 15.0f;   // max displayed dB
    float gridDiv = 5.0f;  // how many dB per divisions (between grid lines)

    double& sampleRate;

    float dyn = dbMax - dbMin;
    float zero = 2.0f * dbMax / dyn;
    float scale = 1.0f / (zero + std::tanh (dbMin / dyn * -2.0f));

    float height;
    float width;

    int xMin = hzToX (fMin);
    int xMax = hzToX (fMax);
    int yMin = jmax (dbToY (dbMax), 0);
    int yMax = jmax (dbToY (dbMin), yMin);
    int yZero = dbToY (0.0f);

    int numPixels = xMax - xMin + 1;
    Array<double> frequencies;

    const float mL = 23.0f;
    const float mR = 10.0f;
    const float mT = 7.0f;
    const float mB = 15.0f;
    const float OH = 3.0f;

    void setHeight (int newHeight)
    {
        height = static_cast<float> (newHeight) - mT - mB;
    }

    void setWidth (int newWidth)
    {
        width = static_cast<float> (newWidth) - mL - mR;
    }

    int dbToY (const float dB)
    {
        int ypos = dbToYFloat (dB);
        return ypos;
    }

    float dbToYFloat (const float dB)
    {
        if (height <= 0.0f) return 0.0f;
        float temp;
        if (dB < 0.0f)
            temp = zero + std::tanh (dB / dyn * -2.0f);
        else
            temp = zero - 2.0f * dB / dyn;

        return mT + scale * height * temp;
    }

    float yToDb (const float y)
    {
        float temp = (y - mT) / height / scale - zero;
        float dB;
        if (temp > 0.0f)
            dB =  std::atanh (temp) * dyn * -0.5f;
        else
            dB = - 0.5f * temp * dyn;
        return std::isnan (dB) ? dbMin : dB;
    }

    int hzToX (float hz)
    {
        int xpos = mL + width * (log (hz / fMin) / log (fMax / fMin));
        return xpos;
    }

    float xToHz (int x)
    {
        return fMin * pow ((fMax / fMin), ((x - mL) / width));
    }
};


// ============================

class FilterBackdrop : public Component
{
public:
    FilterBackdrop (Settings& settings) : s (settings)
    {
        setBufferedToImage (true);
    };

    ~FilterBackdrop() {};

    void paint (Graphics& g) override
    {
        g.setColour (Colours::steelblue.withMultipliedAlpha (0.01f));
        g.fillAll();

        g.setFont (getLookAndFeel().getTypefaceForFont (Font (12.0f, 2)));
        g.setFont (12.0f);

        // db labels
        float dyn = s.dbMax - s.dbMin;
        int numgridlines = dyn / s.gridDiv + 1;

        //g.setFont (Font (getLookAndFeel().getTypefaceForFont (Font (10.0f, 1)));
        g.setColour (Colours::white);
        int lastTextDrawPos = -1;
        for (int i = 0; i < numgridlines; ++i)
        {
            float db_val = s.dbMax - i * s.gridDiv;
            lastTextDrawPos = drawLevelMark (g, 0, 20, db_val, String (db_val, 0), lastTextDrawPos);
        }


        // frequency labels
        for (float f = s.fMin; f <= s.fMax; f += pow (10, floor (log10 (f)))) {
            int xpos = s.hzToX (f);

            String axislabel;
            bool drawText = false;

            if ((f == 20) || (f == 50) || (f == 100) || (f == 500))
            {
                axislabel = String (f, 0);
                drawText = true;
            }
            else if ((f == 1000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                axislabel = String (f / 1000, 0);
                axislabel << "k";
                drawText = true;
            }
            else
                continue;

            g.drawText (axislabel, xpos - 10, s.dbToY (s.dbMin) + s.OH + 0.0f, 20, 12, Justification::centred, false);
        }


        g.setColour (Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (dbGridPath, PathStrokeType (0.5f));

        g.setColour (Colours::steelblue.withMultipliedAlpha (0.9f));
        g.strokePath (hzGridPathBold, PathStrokeType (1.0f));

        g.setColour (Colours::steelblue.withMultipliedAlpha (0.8f));
        g.strokePath (hzGridPath, PathStrokeType (0.5f));
    }

    void resized() override
    {
        const float width = getWidth() - s.mL - s.mR;
        dbGridPath.clear();

        int numgridlines = s.dyn / s.gridDiv + 1;

        for (int i = 0; i < numgridlines; ++i)
        {
            float db_val = s.dbMax - i * s.gridDiv;

            int ypos = s.dbToY (db_val);

            dbGridPath.startNewSubPath (s.mL - s.OH, ypos);
            dbGridPath.lineTo (s.mL + width + s.OH, ypos);
        }

        hzGridPath.clear();
        hzGridPathBold.clear();

        for (float f = s.fMin; f <= s.fMax; f += powf (10, floor (log10 (f))))
        {
            int xpos = s.hzToX (f);

            if ((f == 20) || (f == 50) || (f == 100) || (f == 500) || (f == 1000) || (f == 5000) || (f == 10000) || (f == 20000))
            {
                hzGridPathBold.startNewSubPath (xpos, s.dbToY (s.dbMax) - s.OH);
                hzGridPathBold.lineTo (xpos, s.dbToY (s.dbMin) + s.OH);

            } else
            {
                hzGridPath.startNewSubPath (xpos, s.dbToY (s.dbMax) - s.OH);
                hzGridPath.lineTo (xpos, s.dbToY (s.dbMin) + s.OH);
            }
        }
    }

    int inline drawLevelMark (Graphics& g, int x, int width, const int level, const String& label, int lastTextDrawPos = -1)
    {
        float yPos = s.dbToYFloat (level);
        x = x + 1.0f;
        width = width - 2.0f;

        if (yPos - 4 > lastTextDrawPos)
        {
            g.drawText (label, x + 2, yPos - 4, width - 4, 9, Justification::right, false);
            return yPos + 5;
        }
        return lastTextDrawPos;
    }

private:

    Settings& s;

    Path dbGridPath;
    Path hzGridPath;
    Path hzGridPathBold;
};


// ============================

template <typename coeffType>
class FrequencyBand    : public Component
{
public:
    FrequencyBand (Settings& settings) : s (settings)
    {
    };

    FrequencyBand (Settings& settings, typename dsp::IIR::Coefficients<coeffType>::Ptr coeffs1, typename dsp::IIR::Coefficients<coeffType>::Ptr coeffs2, Colour newColour) : s (settings), colour (newColour)
    {
        addCoeffs (coeffs1, coeffs2);
    };

    ~FrequencyBand () {};

    void paint (Graphics& g) override
    {
        g.setColour (colour.withMultipliedAlpha (0.7f));
        g.strokePath (path, PathStrokeType (1.0f));

        g.setColour (colour.withMultipliedAlpha (0.3f));
        g.fillPath (closedPath);
    }

    void resized() override
    {
        magnitudes.resize (s.numPixels);
        magnitudes.fill (1.0f);
        magnitudesIncludingGains.resize (s.numPixels);
        magnitudesIncludingGains.fill (1.0f);
    }

    void updateMakeUpGain (const float& newMakeUp)
    {
        if (fabs (makeUp - newMakeUp) > 0.01)
        {
            makeUp = newMakeUp;
            updatePath();
        }
    }

    void updateGainReduction (const float& newGainReduction)
    {
        if (fabs (gainReduction - newGainReduction) > 0.01)
        {
            gainReduction = newGainReduction;
            updatePath();
        }
    }


    void updateFilterResponse ()
    {
        Array<double> tempMagnitude;
        tempMagnitude.resize (s.numPixels);
        tempMagnitude.fill (1.0f);
        for (int i = 0; i < coeffs.size(); ++i)
        {
            Array<double> filterMagnitude;
            filterMagnitude.resize (s.numPixels);
            if (coeffs[i] != nullptr)
                coeffs[i]->getMagnitudeForFrequencyArray (s.frequencies.getRawDataPointer(), filterMagnitude.getRawDataPointer(), s.numPixels, s.sampleRate);
            FloatVectorOperations::multiply (tempMagnitude.getRawDataPointer(), filterMagnitude.getRawDataPointer(), s.numPixels);
        }
        FloatVectorOperations::copy (magnitudes.getRawDataPointer(), tempMagnitude.getRawDataPointer(), s.numPixels);
        updatePath();
    }

    void updatePath ()
    {
        path.clear();
        closedPath.clear();

        float gain1 = makeUp;
        float gain2 = gainReduction;

        if (bypassed)
        {
            gain1 = 0.0f;
            gain2 = 0.0f;
        }

        float db = Decibels::gainToDecibels (magnitudes[0]) + gain1 + gain2;
        magnitudesIncludingGains.set (0, Decibels::decibelsToGain (db) );
        path.startNewSubPath (s.xMin, jlimit (static_cast<float> (s.yMin), static_cast<float> (s.yMax) + s.OH + 1.0f, s.dbToYFloat (db)));

        for (int i = 1; i < s.numPixels; ++i)
        {
            db = Decibels::gainToDecibels (magnitudes[i]) + gain1 + gain2;
            magnitudesIncludingGains.set (i, Decibels::decibelsToGain (db) );
            float y = jlimit (static_cast<float> (s.yMin), static_cast<float> (s.yMax) + s.OH + 1.0f, s.dbToYFloat (db));
            path.lineTo (s.xMin + i, y);
        }

        closedPath = path;
        closedPath.lineTo (s.xMax, s.yMax + s.OH + 1.0f);
        closedPath.lineTo (s.xMin, s.yMax + s.OH + 1.0f);
        closedPath.closeSubPath();

        repaint();
    }

    double* getMagnitudeIncludingGains ()
    {
        return magnitudesIncludingGains.getRawDataPointer();
    }

    Array<double> getMagnitudeIncludingGainsArray () const
    {
        return magnitudesIncludingGains;
    }

    void setColour (const Colour newColour)
    {
        colour = newColour;
    }

    void setBypassed (bool newBypassed)
    {
        bypassed = newBypassed;
        updatePath();
    }

    void addCoeffs (typename dsp::IIR::Coefficients<coeffType>::Ptr coeffs1, typename dsp::IIR::Coefficients<coeffType>::Ptr coeffs2)
    {
        coeffs.add (coeffs1);
        coeffs.add (coeffs2);
    }

private:
    Settings& s;
    Array<typename dsp::IIR::Coefficients<coeffType>::Ptr> coeffs;

    Colour colour;
    bool bypassed {false};

    float makeUp = 0.0f, gainReduction = 0.0f;

    Array<double> magnitudes, magnitudesIncludingGains;
    Path path, closedPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyBand);
};

// ============================

template <typename coefficientType>
class OverallMagnitude  : public Component
{
public:
    OverallMagnitude (Settings& settings, int numFreqBands) : s (settings), numBands (numFreqBands), overallGain (0.0f)
    {
    };

    ~OverallMagnitude () {};

    void paint (Graphics& g) override
    {
        g.setColour (Colours::white);
        g.strokePath (path, PathStrokeType (1.25f));

        g.setColour (Colours::white.withMultipliedAlpha (0.125f));
        g.fillPath (closedPath);
    }

    void resized() override
    {
        overallMagnitude.resize (s.numPixels);
        overallMagnitude.fill (overallGain);
    }

    void setOverallGain (const float newGain)
    {
        overallGain = newGain;
    }

    void updateOverallMagnitude ()
    {
        overallMagnitude.fill (overallGain);
        for (int i = 0; i < (*freqBands).size(); ++i)
        {
            FloatVectorOperations::add (overallMagnitude.getRawDataPointer(), (*freqBands)[i]->getMagnitudeIncludingGains (), s.numPixels);
        }

        updatePath();
    }


    void updatePath ()
    {
        path.clear();
        closedPath.clear();

        float db = Decibels::gainToDecibels (overallMagnitude[0]);
        path.startNewSubPath (s.xMin, jlimit (static_cast<float> (s.yMin), static_cast<float> (s.yMax) + s.OH + 1.0f, s.dbToYFloat (db)));

        for (int i = 1; i < s.numPixels; ++i)
        {
            db = Decibels::gainToDecibels (overallMagnitude[i]);
            float y = jlimit (static_cast<float> (s.yMin), static_cast<float> (s.yMax) + s.OH + 1.0f, s.dbToYFloat (db));
            path.lineTo (s.xMin + i, y);
        }
        closedPath = path;

        closedPath.lineTo (s.xMax, s.yZero);
        closedPath.lineTo (s.xMin, s.yZero);
        closedPath.closeSubPath();

        repaint();
    }

    void setFreqBands (OwnedArray<FrequencyBand<coefficientType>>* bandsForOverallGain)
    {
        freqBands = bandsForOverallGain;
    }

private:
    Settings& s;
    OwnedArray<FrequencyBand<coefficientType>>* freqBands;

    Array<double> overallMagnitude;
    Path path, closedPath;

    int numBands;
    float overallGain;
};


// ============================

template <typename T>
class FilterBankVisualizer : public Component
{
public:
    FilterBankVisualizer (float fMin, float fMax, float dbMin, float dbMax, float gridDiv, double& sampleRate, int numBands, bool overallMagnitude=false) : s {fMin, fMax, dbMin, dbMax, gridDiv, sampleRate}, filterBackdrop (s), overallMagnitude (s, numBands), numFreqBands (numBands), displayOverallMagnitude (overallMagnitude)
    {
        init();
    };

    ~FilterBankVisualizer() {};

    void init ()
    {
        updateSettings();

        addAndMakeVisible (&filterBackdrop);

        for (int i = 0; i < numFreqBands; ++i)
        {
            freqBands.add (new FrequencyBand<T> (s));
            addAndMakeVisible (freqBands[i]);
            freqBands.getLast()->addMouseListener (this, true);
        }

        if (displayOverallMagnitude)
            activateOverallMagnitude ();

        freqBandColours.resize (numFreqBands);
    }

    void updateSettings ()
    {
        s.setWidth (getWidth());
        s.setHeight (getHeight());

        s.xMin = s.hzToX (s.fMin);
        s.xMax = s.hzToX (s.fMax);
        s.yMin = jmax (s.dbToY (s.dbMax), 0);
        s.yMax = jmax (s.dbToY (s.dbMin), s.yMin);
        s.yZero = s.dbToY (0.0f);

        s.numPixels = s.xMax - s.xMin + 1;

        s.frequencies.resize (s.numPixels < 0 ? 0.0f : s.numPixels);
        for (int i = 0; i < s.frequencies.size(); ++i)
            s.frequencies.set (i, s.xToHz (s.xMin + i));
    }

    void paint (Graphics& g) override
    {
    }

    void paintOverChildren (Graphics& g) override
    {
        g.excludeClipRegion (Rectangle<int> (0.0f, s.yMax + s.OH, s.width, s.height - s.yMax - s.OH));

        // draw crossovers separators and knobs
        float yPos = s.dbToYFloat (0.0f);
        float prevXPos = s.xMin;
        for (int i = 0; i < crossoverSliders.size(); ++i)
        {
            float xPos = crossoverSliders[i] == nullptr ? s.xMin : s.hzToX (crossoverSliders[i]->getValue());
            Line<float> freqBandSeparator;
            freqBandSeparator.setStart (xPos, s.yMin);
            freqBandSeparator.setEnd (xPos, s.yMax + s.yMin);
            g.setColour (activeElem == i ? colour.brighter() : colour.withMultipliedAlpha (0.8));
            g.drawLine (freqBandSeparator, i == activeElem ? 2.5f : 2.0f);
            prevXPos = xPos;

            g.setColour (Colours::black);
            g.drawEllipse (xPos - 5.0f, yPos - 5.0f , 10.0f, 10.0f, 3.0f);
            g.setColour (activeElem == i ? colour.brighter() : colour);
            g.fillEllipse (xPos - 5.0f, yPos - 5.0f , 10.0f, 10.0f);

        }
    }

    void resized() override
    {
        updateSettings ();

        Rectangle<int> area = getLocalBounds();
        filterBackdrop.setBounds (area);
        for (int i = 0; i < freqBands.size(); ++i)
        {
            freqBands[i]->setBounds (area);
            freqBands[i]->updateFilterResponse();
        }

        if (displayOverallMagnitude)
        {
            overallMagnitude.setBounds (area);
            overallMagnitude.updateOverallMagnitude();
        }

    }

    void setNumFreqBands (const int newValue)
    {
        numFreqBands = newValue;
    }

    void mouseDrag (const MouseEvent &event) override
    {
        Point<int> pos = event.getPosition();
        float frequency = s.xToHz (pos.x);

        if (activeElem != -1)
        {
            if (crossoverSliders[activeElem] != nullptr)
            {
                crossoverSliders[activeElem]->setValue (frequency);
                repaint();
            }

        }
    }

    void mouseMove (const MouseEvent &event) override
    {
        Point<int> pos = event.getPosition();
        int oldActiveElem = activeElem;
        activeElem = -1;

        for (int i = 0; i < crossoverSliders.size(); ++i)
        {
            int x = crossoverSliders[i] == nullptr ? s.hzToX (s.fMin) : s.hzToX (crossoverSliders[i]->getValue());
            float y = s.dbToYFloat (0.0f);
            Point<int> filterPos (x, y);

            if (pos.getDistanceSquaredFrom (filterPos) < 48)
            {
                activeElem = i;
                break;
            }
        }

        if (oldActiveElem != activeElem)
            repaint();
    }

    void updateMakeUpGain (const int freqBand, const float newMakeUp)
    {
        freqBands[freqBand]->updateMakeUpGain (newMakeUp);
    }


    void updateGainReduction (const int freqBand, const float newGainReduction)
    {
        freqBands[freqBand]->updateGainReduction (newGainReduction);
    }

    void updateFreqBandResponse (const int freqBand)
    {
        freqBands[freqBand]->updateFilterResponse();
    }

    void updateFreqBandResponses ()
    {
        for (int i = 0; i < numFreqBands; ++i)
            freqBands[i]->updateFilterResponse();
    }

    void updateOverallMagnitude ()
    {
        overallMagnitude.updateOverallMagnitude();
    }

    void setBypassed (const int i, const bool bypassed)
    {
        freqBands[i]->setBypassed (bypassed);
    }

    void setSolo (const int i, const bool soloed)
    {
        if (soloed)
            soloSet.insert (i);
        else
            soloSet.erase (i);


        for (int i = 0; i < numFreqBands; ++i)
        {
            Colour colour = freqBandColours[i];

            if (! soloSet.empty())
            {
                if (! soloSet.count (i))
                    colour = colour.withMultipliedSaturation (0.4f);
            }

            freqBands[i]->setColour (colour);
            freqBands[i]->repaint ();
        }
    }

    void setFrequencyBand (const int i, typename dsp::IIR::Coefficients<T>::Ptr coeffs1, typename  dsp::IIR::Coefficients<T>::Ptr coeffs2, Colour colour)
    {
        freqBands[i]->addCoeffs (coeffs1, coeffs2);
        freqBands[i]->setColour (colour);
        freqBands[i]->updateFilterResponse();

        freqBandColours.set (i, colour);
    }

    void addFrequencyBand (typename dsp::IIR::Coefficients<T>::Ptr coeffs1, typename dsp::IIR::Coefficients<T>::Ptr coeffs2, Colour colour)
    {
        freqBands.add (new FrequencyBand<T> (s, coeffs1, coeffs2, colour));
        addAndMakeVisible (freqBands.getLast());

        freqBandColours.add (colour);
    }

    void activateOverallMagnitude (const float gain=0.0f)
    {
        displayOverallMagnitude = true;
        overallMagnitude.setFreqBands (&freqBands);
        overallMagnitude.setOverallGain (gain);
        addAndMakeVisible (&overallMagnitude);
        overallMagnitude.setBounds (getLocalBounds ());
        overallMagnitude.resized();
        overallMagnitude.addMouseListener (this, true);
    }

    void deactivateOverallMagnitude ()
    {
        displayOverallMagnitude = false;
        overallMagnitude.removeMouseListener (this);
        overallMagnitude.setVisible (false);
        removeChildComponent (&overallMagnitude);
        displayOverallMagnitude = false;
    }

    void setOverallGain (const float gain=0.0f)
    {
        overallMagnitude.setOverallGain (gain);
    }

    void addCrossover (Slider* crossoverSlider = nullptr)
    {
        crossoverSliders.add (crossoverSlider);
    }


private:
    Settings s;

    FilterBackdrop filterBackdrop;
    OwnedArray<FrequencyBand<T>> freqBands;
    OverallMagnitude<T> overallMagnitude;

    Array<Slider*> crossoverSliders;

    int numFreqBands, activeElem {-1};
    bool displayOverallMagnitude {false};

    Colour colour {0xFFD8D8D8};
    Array<Colour> freqBandColours;

    std::set<int> soloSet;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterBankVisualizer);
};
