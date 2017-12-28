/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
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

#define rcirc15 0.258819045102521
#define rcirc30 0.5
#define rcirc45 0.707106781186547
#define rcirc60 0.866025403784439
#define rcirc75 0.965925826289068

class  IEMSphere :  public Component

{
public:
    IEMSphere() : Component() {
        setBufferedToImage(true);
    };
    ~IEMSphere() { };
    
    class IEMSphereElement
    {
    public:
        IEMSphereElement() : position(1.0f, 0.0f, 0.0f) {}
        IEMSphereElement(String newID) {ID = newID;}
        virtual ~IEMSphereElement () {}
        
        void moveElement (const MouseEvent &event, float centreX, float centreY, float radius, bool upBeforeDrag) {
            bool leftClick = event.mods.isLeftButtonDown();
            
            Point<int> pos = event.getPosition();
            position.x = (centreY-pos.getY())/radius;
            position.y = (centreX-pos.getX())/radius;
            float rr = position.x*position.x + position.y*position.y;
            float r = sqrt(rr);
            if (rr>1.0f) {
                position.x /= leftClick ? rr : r;
                position.y /= leftClick ? rr : r;
                rr = leftClick ? 1.0f/rr : 1.0f;
                calcZ(leftClick ? !upBeforeDrag : upBeforeDrag);
            }
            else calcZ(upBeforeDrag);
        }
        
        void setActive ( bool shouldBeActive) {
            active = shouldBeActive;
        }
        bool isActive() { return active; }
        
        void setBackgroundColour( Colour newColour) { backgroundColour = newColour; }
        void setTextColour( Colour newColour) { textColour = newColour; }
        Colour getBackgroundColour() { return backgroundColour; }
        Colour getTextColour() { return textColour; }
        
        bool setPosition(Vector3D<float> newPosition) // is true when position is updated (use it for repainting)
        {
            if (position.x != newPosition.x || position.y != newPosition.y || position.z != newPosition.z)
            {
                position = newPosition;
                return true;
            }
            return false;
        }

        void setLabel(String newLabel) {label = newLabel;}
        void setID(String newID) {ID = newID;}
        
        void setGrabPriority(int newPriority) {grabPriority = newPriority;}
        int getGrabPriority() {return grabPriority;}
        void setGrabRadius(float newRadius) {grabRadius = newRadius;}
        float getGrabRadius() {return grabRadius;}
        
        Vector3D<float> getPosition() {return position;}
        String getLabel() {return label;};
        String getID() {return ID;};
        void calcZ(bool up) {
            float rr = position.x*position.x + position.y*position.y;
            if (rr<=1.0f) position.z = sqrt(1.0f - rr);
            else position.z = 0.0f;
            if (!up) position.z *= -1.0f;
        }
        
    private:
        Vector3D<float> position;
        
        bool active = true;
        
        float grabRadius = 0.015f;
        int grabPriority = 0;
        
        Colour backgroundColour = Colours::white;
        Colour textColour = Colours::black;
        String ID = "";
        String label = "";
    };
    
    class IEMSphereListener
    {
    public:
        
        virtual ~IEMSphereListener () {}
        
        virtual void IEMSphereElementChanged (IEMSphere* sphere, IEMSphereElement* element) = 0;
        virtual void IEMSphereMouseWheelMoved (IEMSphere* sphere, const MouseEvent &event, const MouseWheelDetails &wheel) {};
    };


    void paint (Graphics& g) override
    {
        const Rectangle<float> bounds = getLocalBounds().toFloat();
        const float centreX = 0.5f * bounds.getWidth();
        const float centreY = 0.5f * bounds.getHeight();
        const Rectangle<float> sphere(bounds.reduced(10, 11));
        
        radius = 0.5f * jmin(sphere.getWidth(), sphere.getHeight());
        
        
        g.setColour(Colours::white);
        g.drawEllipse(centreX-radius, centreY - radius, 2.0f * radius, 2.0f * radius, 1.0f);
        
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 1)));
        g.setFont(12.0f);
        g.drawText("FRONT", centreX-15, centreY-radius-12, 30, 12, Justification::centred);
        g.drawText("BACK", centreX-15, centreY+radius, 30, 12, Justification::centred);
        //g.drawMultiLineText("LEFT", 0, centreY-12, 10);
        g.drawFittedText("L\nE\nF\nT", 0, centreY-40, 10, 80, Justification::centred, 4);
        g.drawFittedText("R\nI\nG\nH\nT", bounds.getWidth()-10, centreY-40, 10, 80, Justification::centred, 5);
        //g.drawMultiLineText("RIGHT", bounds.getWidth()-8, centreY-12, 10);
        
        g.setColour(Colours::steelblue.withMultipliedAlpha(0.3f));
        Path circles;
        
        float rCirc = radius*rcirc15;
        circles.addEllipse(centreX - rCirc, centreY - rCirc, 2.0f * rCirc, 2.0f * rCirc);
        g.fillPath(circles);
        
        rCirc = radius*rcirc30;
        circles.addEllipse(centreX - rCirc, centreY - rCirc, 2.0f * rCirc, 2.0f * rCirc);
        g.fillPath(circles);
        
        rCirc = radius*rcirc45;
        circles.addEllipse(centreX - rCirc, centreY - rCirc, 2.0f * rCirc, 2.0f * rCirc);
        g.fillPath(circles);
        
        rCirc = radius*rcirc60;
        circles.addEllipse(centreX - rCirc, centreY - rCirc, 2.0f * rCirc, 2.0f * rCirc);
        g.fillPath(circles);
        
        rCirc = radius*rcirc75;
        circles.addEllipse(centreX - rCirc, centreY - rCirc, 2.0f * rCirc, 2.0f * rCirc);
        g.fillPath(circles);
        
        g.setColour(Colours::Colours::steelblue.withMultipliedAlpha(0.7f));
        g.strokePath(circles, PathStrokeType(.5f));
        
        
        ColourGradient gradient(Colours::black.withMultipliedAlpha(0.7f), centreX, centreY, Colours::black.withMultipliedAlpha(0.1f), 0, 0, true);
        g.setGradientFill(gradient);

        Path line;
        line.startNewSubPath(centreX, centreY-radius);
        line.lineTo(centreX, centreY+radius);

        Path path;
        path.addPath(line);
        path.addPath(line, AffineTransform().rotation(0.25f*M_PI, centreX, centreY));
        path.addPath(line, AffineTransform().rotation(0.5f*M_PI, centreX, centreY));
        path.addPath(line, AffineTransform().rotation(0.75f*M_PI, centreX, centreY));

        g.strokePath(path, PathStrokeType(0.5f));
        
        Path panPos;
        for (int i = elements.size(); --i >= 0;) {
            IEMSphereElement* handle = (IEMSphereElement*) elements.getUnchecked (i);
            Vector3D<float> pos = handle->getPosition();
            float diam = 15.0f + 4.0f * pos.z;
            g.setColour(handle->isActive() ? handle->getBackgroundColour() : Colours::grey);
            Rectangle<float> temp(centreX-pos.y*radius-diam/2,centreY-pos.x*radius-diam/2,diam,diam);
            panPos.addEllipse(temp);
            g.strokePath(panPos,PathStrokeType(1.0f));
            g.setColour((handle->isActive() ? handle->getBackgroundColour() : Colours::grey).withMultipliedAlpha(pos.z>=-0.0f ? 1.0f : 0.4f));
            g.fillPath(panPos);
            g.setColour(handle->getTextColour());
            g.drawFittedText(handle->getLabel(), temp.toNearestInt(), Justification::centred, 1);
            panPos.clear();
        }
        
        
        
    };
    void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel) override {
        for (int i = listeners.size (); --i >= 0;)
            ((IEMSphereListener*) listeners.getUnchecked (i))->IEMSphereMouseWheelMoved (this, event, wheel);
    }
    
    void mouseDown (const MouseEvent &event) override {
        IEMSphereElement *handle;
        
        const float centreX = 0.5* (float)getBounds().getWidth();
        const float centreY = 0.5* (float)getBounds().getHeight();
        
        int nElem = elements.size();
        activeElem = -1;
        
        if (nElem > 0) {
            Point<int> pos = event.getPosition();
            
            float mouseX = (centreY-pos.getY())/radius;
            float mouseY = (centreX-pos.getX())/radius;
            
            float *dist = (float*) malloc(nElem*sizeof(float));
            
            //int nGrabed = 0;
            int highestPriority = -1;
            
            float tx,ty;
            const int nElements = elements.size();
            for (int i = 0;  i < nElements; ++i) {
                handle =  (IEMSphereElement*) elements.getUnchecked (i);
                Vector3D<float> pos = handle->getPosition();
                tx = (mouseX - pos.x);
                ty = (mouseY - pos.y);
                dist[i] = tx*tx + ty*ty;
                
                if (dist[i] <= handle->getGrabRadius()) {
                    if (handle->getGrabPriority() > highestPriority) {
                        activeElem = i;
                        highestPriority = handle->getGrabPriority();
                    }
                    else if (handle->getGrabPriority() == highestPriority && dist[i] < dist[activeElem]) {
                        activeElem = i;
                    }
                }
            }
            
            
            //activeElem = indexofSmallestElement(dist,nElem);
        }
        if (activeElem != -1)  activeElemWasUpBeforeDrag = ((IEMSphereElement*) elements.getUnchecked (activeElem))->getPosition().z >= 0.0f;
        
    }
    void mouseDrag (const MouseEvent &event) override {
        const float centreX = 0.5* (float)getBounds().getWidth();
        const float centreY = 0.5* (float)getBounds().getHeight();
        if (activeElem != -1) {
            ((IEMSphereElement*) elements.getUnchecked (activeElem))->moveElement(event,centreX, centreY, radius,activeElemWasUpBeforeDrag);
            sendChanges(((IEMSphereElement*) elements.getUnchecked (activeElem)));
        }
    }
    
    
    
    
    void addListener (IEMSphereListener* const listener) {
        jassert (listener != 0);
        if (listener !=0)
            listeners.add (listener);
    };
    void removeListener (IEMSphereListener* const listener) {
        listeners.removeFirstMatchingValue(listener);
    };
    
    void sendChanges(IEMSphereElement* element)
    {
        for (int i = listeners.size (); --i >= 0;)
            ((IEMSphereListener*) listeners.getUnchecked (i))->IEMSphereElementChanged (this, element);
    }
    
    void addElement (IEMSphereElement* const element) {
        jassert (element != 0);
        if (element !=0)
            elements.add (element);
    };
    void removeElement (IEMSphereElement* const element) {
        elements.removeFirstMatchingValue(element);
    };
    
    int indexofSmallestElement(float *array, int size)
    {
        int index = 0;
        
        for(int i = 1; i < size; i++)
        {
            if(array[i] < array[index])
                index = i;
        }
        
        return index;
    }
    
private:
    float radius;
    int activeElem;
    bool activeElemWasUpBeforeDrag;
    Array<void*> listeners;
    Array<void*> elements;
};



