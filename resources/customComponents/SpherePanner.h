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

#define rcirc15 0.258819045102521
#define rcirc30 0.5
#define rcirc45 0.707106781186547
#define rcirc60 0.866025403784439
#define rcirc75 0.965925826289068

class  SpherePanner :  public Component

{
public:
    SpherePanner() : Component() {
        setBufferedToImage(true);
    };
    ~SpherePanner() { };
    
    inline Vector3D<float> yawPitchToCartesian(float yawInRad, float pitchInRad) {
        float cosPitch = cosf(pitchInRad);
        return Vector3D<float>(cosPitch * cosf(yawInRad), cosPitch * sinf(yawInRad), sinf(-1.0f * pitchInRad));
    }
    
    inline Point<float> cartesianToYawPitch(Vector3D<float> pos) {
        float hypxy = sqrt(pos.x*pos.x+pos.y*pos.y);
        return Point<float>(atan2f(pos.y,pos.x), atan2f(hypxy,pos.z)-M_PI/2);
    }
    
    class Element
    {
    public:
        Element() {}
        Element(String newID) {ID = newID;}
        ~Element () {}
        
        void setSliders(Slider* newYawSlider, Slider* newPitchSlider)
        {
            yawSlider = newYawSlider;
            pitchSlider = newPitchSlider;
        }
        
        void moveElement (const MouseEvent &event, Point<int> centre, float radius, bool upBeforeDrag) {
            //bool leftClick = event.mods.isLeftButtonDown();
            

            Point<int> pos = event.getPosition();
            float yaw = -1.0f * centre.getAngleToPoint(pos);
            float r = centre.getDistanceFrom(pos)/radius;
            if (r > 1.0f) {
                r = 1.0f/r;
                upBeforeDrag = !upBeforeDrag;
            }
            
            float pitch = acosf(r);
            if (upBeforeDrag) pitch *= -1.0f;
  
            if (yawSlider != nullptr)
                yawSlider->setValue(yaw * 180.0f / M_PI);
            if (pitchSlider != nullptr)
                pitchSlider->setValue(pitch * 180.0f / M_PI);
        }
        
        void setActive ( bool shouldBeActive) {
            active = shouldBeActive;
        }
        bool isActive() { return active; }
        
        void setColour( Colour newColour) { colour = newColour; }
        void setTextColour( Colour newColour) { textColour = newColour; }
        Colour getColour() { return colour; }
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
//            float rr = position.x*position.x + position.y*position.y;
//            if (rr<=1.0f) position.z = sqrt(1.0f - rr);
//            else position.z = 0.0f;
//            if (!up) position.z *= -1.0f;
        }
        
        Slider *yawSlider = nullptr;
        Slider *pitchSlider = nullptr;
    private:
        
        Vector3D<float> position;
        bool active = true;
        
        float grabRadius = 0.015f;
        int grabPriority = 0;
        
        Colour colour = Colours::white;
        Colour textColour = Colours::black;
        String ID = "";
        String label = "";
    };
    
//    class IEMSphereListener
//    {
//    public:
//
//        virtual ~IEMSphereListener () {}
//
//        virtual void IEMSphereElementChanged (IEMSphere* sphere, IEMSphereElement* element) = 0;
//        virtual void IEMSphereMouseWheelMoved (IEMSphere* sphere, const MouseEvent &event, const MouseWheelDetails &wheel) {};
//    };


    void resized () override
    {
        const Rectangle<float> sphere(getLocalBounds().reduced(10, 10).toFloat());

        radius = 0.5f * jmin(sphere.getWidth(), sphere.getHeight());
        centre = getLocalBounds().getCentre();
        sphereArea.setBounds(0, 0, 2*radius, 2*radius);
        sphereArea.setCentre(centre.toFloat());
    }
    
    void paint (Graphics& g) override
    {
        const Rectangle<float> bounds = getLocalBounds().toFloat();
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        const Rectangle<float> sphere(bounds.reduced(10, 10));
        
        
        g.setColour(Colours::white);
        g.drawEllipse(centreX-radius, centreY - radius, 2.0f * radius, 2.0f * radius, 1.0f);
        
        g.setFont(getLookAndFeel().getTypefaceForFont (Font(12.0f, 1)));
        g.setFont(12.0f);
        g.drawText("FRONT", centreX-15, centreY-radius-12, 30, 12, Justification::centred);
        g.drawText("BACK", centreX-15, centreY+radius, 30, 12, Justification::centred);
        //g.drawMultiLineText("LEFT", 0, centreY-12, 10);
        g.drawFittedText("L\nE\nF\nT", sphereArea.getX()-10, centreY-40, 10, 80, Justification::centred, 4);
        g.drawFittedText("R\nI\nG\nH\nT", sphereArea.getRight(), centreY-40, 10, 80, Justification::centred, 5);
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
        int size = elements.size();
        for (int i = 0; i < size; ++i) {
            Element* handle = (Element*) elements.getUnchecked (i);
            
            float yaw;
            float pitch;
            
            if (handle->yawSlider != nullptr)
                yaw = handle->yawSlider->getValue() * M_PI / 180;
            else yaw = 0.0f;

            if (handle->pitchSlider != nullptr)
                pitch = handle->pitchSlider->getValue() * M_PI / 180;
            else pitch = 0.0f;
            
            Vector3D<float> pos = yawPitchToCartesian(yaw, pitch);
            handle->setPosition(pos);
            
            float diam = 15.0f + 4.0f * pos.z;
            g.setColour(handle->isActive() ? handle->getColour() : Colours::grey);
            Rectangle<float> temp(centreX-pos.y*radius-diam/2,centreY-pos.x*radius-diam/2,diam,diam);
            panPos.addEllipse(temp);
            g.strokePath(panPos,PathStrokeType(1.0f));
            g.setColour((handle->isActive() ? handle->getColour() : Colours::grey).withMultipliedAlpha(pos.z>=-0.0f ? 1.0f : 0.4f));
            g.fillPath(panPos);
            g.setColour(handle->getTextColour());
            g.drawFittedText(handle->getLabel(), temp.toNearestInt(), Justification::centred, 1);
            panPos.clear();
        }
        
        
        
    };
    
    
    void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel) override {
//        for (int i = listeners.size (); --i >= 0;)
//            ((IEMSphereListener*) listeners.getUnchecked (i))->IEMSphereMouseWheelMoved (this, event, wheel);
    }
    
    void mouseMove (const MouseEvent &event) override {
        int oldActiveElem = activeElem;
        activeElem = -1;
        
        const float centreX = 0.5* (float)getBounds().getWidth();
        const float centreY = 0.5* (float)getBounds().getHeight();

        int nElem = elements.size();

        if (nElem > 0) {
            Point<int> pos = event.getPosition();

            float mouseX = (centreY-pos.getY())/radius;
            float mouseY = (centreX-pos.getX())/radius;

            float *dist = (float*) malloc(nElem*sizeof(float));

            //int nGrabed = 0;
            int highestPriority = -1;

            float tx,ty;
            for (int i = elements.size(); --i >= 0;) {
                Element* handle(elements.getUnchecked (i));
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
        }
        if (activeElem != -1)  activeElemWasUpBeforeDrag = elements.getUnchecked (activeElem)->getPosition().z >= 0.0f;
        if (oldActiveElem != activeElem)
            repaint();
    }
    void mouseDrag (const MouseEvent &event) override {
        if (activeElem != -1) {
            elements.getUnchecked(activeElem)->moveElement(event, centre, radius, activeElemWasUpBeforeDrag);
//            sendChanges(((Element*) elements.getUnchecked (activeElem)));
            repaint();
        }
    }
    
    
    
    
//    void addListener (IEMSphereListener* const listener) {
//        jassert (listener != 0);
//        if (listener !=0)
//            listeners.add (listener);
//    };
//    void removeListener (IEMSphereListener* const listener) {
//        listeners.removeFirstMatchingValue(listener);
//    };
//
//    void sendChanges(Element* element)
//    {
//        for (int i = listeners.size (); --i >= 0;)
//            ((IEMSphereListener*) listeners.getUnchecked (i))->IEMSphereElementChanged (this, element);
//    }
//
    void addElement (Element* const element) {
        jassert (element != nullptr);
        if (element !=0)
            elements.add (element);
    };
    void removeElement (Element* const element) {
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
    float radius = 1.0f;
    Rectangle<float> sphereArea;
    Point<int> centre;
    int activeElem;
    bool activeElemWasUpBeforeDrag;
    Array<void*> listeners;
    Array<Element*> elements;
};



