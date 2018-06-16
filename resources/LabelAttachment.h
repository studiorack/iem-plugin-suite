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

#include "../JuceLibraryCode/JuceHeader.h"


// This class is copied from AudioProcessorValueTreeState.cpp to make it accessible.
struct AttachedControlBase  : public AudioProcessorValueTreeState::Listener,
public AsyncUpdater
{
    AttachedControlBase (AudioProcessorValueTreeState& s, const String& p)
    : state (s), paramID (p), lastValue (0)
    {
        state.addParameterListener (paramID, this);
    }

    void removeListener()
    {
        state.removeParameterListener (paramID, this);
    }

    void setNewUnnormalisedValue (float newUnnormalisedValue)
    {
        if (AudioProcessorParameter* p = state.getParameter (paramID))
        {
            const float newValue = state.getParameterRange (paramID)
            .convertTo0to1 (newUnnormalisedValue);

            if (p->getValue() != newValue)
            p->setValueNotifyingHost (newValue);
        }
    }

    void setNewNormalisedValue (float newNormalisedValue)
    {
        if (AudioProcessorParameter* p = state.getParameter (paramID))
        {
            if (p->getValue() != newNormalisedValue)
            p->setValueNotifyingHost (newNormalisedValue);
        }
    }

    void sendInitialUpdate()
    {
        if (float* v = state.getRawParameterValue (paramID))
        parameterChanged (paramID, *v);
    }

    void parameterChanged (const String&, float newValue) override
    {
        lastValue = newValue;

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            cancelPendingUpdate();
            setValue (newValue);
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    void beginParameterChange()
    {
        if (AudioProcessorParameter* p = state.getParameter (paramID))
        {
            if (state.undoManager != nullptr)
            state.undoManager->beginNewTransaction();

            p->beginChangeGesture();
        }
    }

    void endParameterChange()
    {
        if (AudioProcessorParameter* p = state.getParameter (paramID))
        p->endChangeGesture();
    }

    void handleAsyncUpdate() override
    {
        setValue (lastValue);
    }

    virtual void setValue (float) = 0;

    AudioProcessorValueTreeState& state;
    String paramID;
    float lastValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttachedControlBase)
};


// This one I wrote myself ;-)
class LabelAttachment : private AttachedControlBase,
private Label::Listener
{
public:
    LabelAttachment (AudioProcessorValueTreeState& stateToControl,
                      const String& parameterID,
                     Label& labelToControl)
    : AttachedControlBase (stateToControl, parameterID),
    label (labelToControl), ignoreCallbacks (false)
    {
        parameter = state.getParameter (paramID);
        sendInitialUpdate();
        label.addListener (this);
    }

    ~LabelAttachment()
    {
        label.removeListener (this);
        removeListener();
    }

    void labelTextChanged (Label *labelThatHasChanged) override
    {
        auto newValue = getNormalizedValueFromText (label.getText());
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if (! ignoreCallbacks)
        {
            beginParameterChange();
            setNewUnnormalisedValue (newValue);
            endParameterChange();
        }

        updateText();
    }

    float getNormalizedValueFromText (const String& text)
    {
        float value = text.getFloatValue();
        return value;
    }


    void setValue (float newValue) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        {
            ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            updateText();
        }
    }

    void updateText()
    {
        String text = parameter->getText (parameter->getValue(), 2) + " " + parameter->label;
        label.setText (text, NotificationType::dontSendNotification);
    }


private:
    Label& label;
    bool ignoreCallbacks;
    CriticalSection selfCallbackMutex;

    const AudioProcessorParameterWithID* parameter {nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabelAttachment)
};


