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



class  PositionPlane :  public Component

{
public:
    PositionPlane() :
    Component(),
    drawPlane(xy),
    autoScale(true),
    dimensions(1.0f, 1.0f, 1.0f)
    {};
    
    ~PositionPlane() { deleteAllChildren(); };
    
    enum Planes
    {
        xy,
        zy,
        zx
    };
    
    class PositionPlaneElement
    {
    public:
        PositionPlaneElement() :
        pos(0.0f, 0.0f, 0.0f)
        {}
        PositionPlaneElement(String newID) {ID = newID;}
        virtual ~PositionPlaneElement () {}
        
        void moveElement (const MouseEvent &event, float centreX, float centreY, float scale, Planes plane, PositionPlane* positionPlane) {
            //bool leftClick = event.mods.isLeftButtonDown();
            
            Point<float> mousePos = event.getPosition().toFloat();
            mousePos.x -= centreX;
            mousePos.y -= centreY;
            mousePos /= scale;
            //DBG(mousePos.x << " " << mousePos.y);
            switch(plane)
            {
                case xy:
                    pos.x = -mousePos.y;
                    pos.y = -mousePos.x;
                    break;
                case zy:
                    pos.z = -mousePos.y;
                    pos.y = -mousePos.x;
                    break;
                case zx:
                    pos.z = -mousePos.y;
                    pos.x = mousePos.x;
                    break;
            }
            
            //clip pos to room dimensions
            Vector3D<float> roomDims = positionPlane->getDimensions();
            pos.x = Range<float>(- 0.5* roomDims.x, 0.5* roomDims.x).clipValue(pos.x);
            pos.y = Range<float>(- 0.5* roomDims.y, 0.5* roomDims.y).clipValue(pos.y);
            pos.z = Range<float>(- 0.5* roomDims.z, 0.5* roomDims.z).clipValue(pos.z);
        }
        
        
        
        void setActive ( bool shouldBeActive) {
            active = shouldBeActive;
        }
        bool isActive() { return active; }
        
        void setColour( Colour newColour) { faceColour = newColour; }
        Colour getColour() { return faceColour; }
        void setPosition(Vector3D<float> newPos, bool repaint = false)
        {
            pos = newPos;
            if (repaint) repaintAllPlanesImIn();
        }
        
        void setLabel(String newLabel) {label = newLabel;}
        void setID(String newID) {ID = newID;}
        
        
        Vector3D<float> getPosition() {return pos;}
        
        String getLabel() {return label;};
        String getID() {return ID;};
        
        void addPlane (PositionPlane* const positionPlane) {
            jassert (positionPlane != 0);
            if (positionPlane !=0)
                planesImIn.add (positionPlane);
        };
        void removePlane (PositionPlane* const positionPlane) {
            planesImIn.removeFirstMatchingValue(positionPlane);
        };
        
        void repaintAllPlanesImIn ()
        {
            for (int i = planesImIn.size (); --i >= 0;)
            {
                PositionPlane* handle = (PositionPlane*) planesImIn.getUnchecked (i);
                handle->repaint();
            }
        }
        
    private:
        Vector3D<float> pos;
        
        bool active = true;
    
        Colour faceColour = Colours::white;
        String ID = "";
        String label = "";
        
        Array<void*> planesImIn;
    };
    
    class PositionPlaneListener
    {
    public:
        virtual ~PositionPlaneListener () {}
        
        virtual void PositionPlaneElementChanged (PositionPlane* plane, PositionPlaneElement* element) = 0;
    };
    
    
    void paint (Graphics& g) override
    {
        Rectangle<float> bounds(0,0,getBounds().getWidth(),getBounds().getHeight());
        float innerSpacing = 3.0f;
        bounds.reduce(innerSpacing,innerSpacing);
        
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();
        
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        
        float drawH;
        float drawW;
        
        switch(drawPlane)
        {
            case xy:
                drawH = dimensions.x;
                drawW = dimensions.y;
                break;
            case zy:
                drawH = dimensions.z;
                drawW = dimensions.y;
                break;
            case zx:
                drawH = dimensions.z;
                drawW = dimensions.x;
                break;
        }
        
        if (autoScale) {
            float dimRatio = drawH /drawW;
            
            if (dimRatio >= height/width)
                scale = height/drawH;
            else
                scale = width/drawW;
            
            if (dimRatio >= height/width)
                scale = height/drawH;
            else
                scale = width/drawW;
        }
        drawW *= scale;
        drawH *= scale;
        
        Rectangle<float> room(innerSpacing + 0.5f * (width - drawW), innerSpacing + 0.5f * (height - drawH), drawW, drawH);
        
        
        
        //g.setColour(Colours::white.withMultipliedAlpha(0.1f));
        //g.fillAll();
        
        g.setColour(Colours::white.withMultipliedSaturation(0.9f));
        g.setFont(10.0f);
        switch(drawPlane)
        {
            case xy:
                g.drawArrow(Line<float>(centreX, centreY, centreX, centreY - 20.0f), 1.0f, 4.0f, 4.0f);
                g.drawArrow(Line<float>(centreX, centreY, centreX - 20.0f, centreY), 1.0f, 4.0f, 4.0f);
                g.drawSingleLineText("x", centreX + 2.0f, centreY - 7.0f);
                g.drawSingleLineText("y", centreX - 12.0f, centreY + 7.0f);
                break;
            case zy:
                g.drawArrow(Line<float>(centreX, centreY, centreX, centreY - 20.0f), 1.0f, 4.0f, 4.0f);
                g.drawArrow(Line<float>(centreX, centreY, centreX - 20.0f, centreY), 1.0f, 4.0f, 4.0f);
                g.drawSingleLineText("z", centreX + 2.0f, centreY - 7.0f);
                g.drawSingleLineText("y", centreX - 12.0f, centreY + 7.0f);
                break;
            case zx:
                g.drawArrow(Line<float>(centreX, centreY, centreX, centreY - 20.0f), 1.0f, 4.0f, 4.0f);
                g.drawArrow(Line<float>(centreX, centreY, centreX + 20.0f, centreY), 1.0f, 4.0f, 4.0f);
                g.drawSingleLineText("z", centreX + 2.0f, centreY - 7.0f);
                g.drawSingleLineText("x", centreX + 7.0f, centreY + 7.0f);
                break;
        }
        
        
        g.setColour(Colours::steelblue.withMultipliedAlpha(0.3f));
        g.fillRect(room);
        
        g.setColour(Colours::white);
        g.drawRect(room,1.0f);
        
        
        
        for (int i = elements.size (); --i >= 0;) {
            PositionPlaneElement* handle = (PositionPlaneElement*) elements.getUnchecked (i);
            
            Vector3D<float> position = handle->getPosition();
            g.setColour(handle->isActive() ? handle->getColour() : Colours::grey);
            
            Path path;
            float posH, posW;
            switch(drawPlane)
            {
                case xy:
                    posH = position.x;
                    posW = position.y;
                    break;
                case zy:
                    posH = position.z;
                    posW = position.y;
                    break;
                case zx:
                    posH = position.z;
                    posW = -position.x;
                    break;
            }
            Rectangle<float> temp(centreX-posW * scale-10/2,centreY-posH * scale-10/2,11,11);
            path.addEllipse(temp);
            //            g.strokePath(panPos,PathStrokeType(1.0f));
            //            g.setColour((handle->isActive() ? handle->getColour() : Colours::grey).withMultipliedAlpha(handle->getZ()>=-0.0f ? 1.0f : 0.4f));
            g.fillPath(path);
            //            g.setColour(Colours::white);
            //            g.drawFittedText(handle->getLabel(), temp.toNearestInt(), Justification::centred, 1);
        }
        
        
    };
    
    float setDimensions (Vector3D<float> newDimensions)
    {
        dimensions = newDimensions;
        repaint();
        
        float width = getBounds().getWidth();
        float height = getBounds().getHeight();
        
        float drawH, drawW;
        float tempScale;
        switch(drawPlane)
        {
            case xy:
                drawH = dimensions.x;
                drawW = dimensions.y;
                break;
            case zy:
                drawH = dimensions.z;
                drawW = dimensions.y;
                break;
            case zx:
                drawH = dimensions.z;
                drawW = dimensions.x;
                break;
        }
        
        
        float dimRatio = drawH /drawW;
        
        if (dimRatio >= height/width)
            tempScale = height/drawH;
        else
            tempScale = width/drawW;
        
        if (dimRatio >= height/width)
            tempScale = height/drawH;
        else
            tempScale = width/drawW;
        
        return tempScale;
    }
    
    void useAutoScale(bool shouldUseAutoScale) {autoScale = shouldUseAutoScale;}
    bool usingAutoScale() {return autoScale;}
    
    void setScale(float newScale) {
        if (~autoScale)
            scale = newScale;
    }
    
    Vector3D<float> getDimensions ()
    {
        return dimensions;
    }
    
    void mouseDown (const MouseEvent &event) override {
        PositionPlaneElement *handle;
        
        Rectangle<float> bounds = getLocalBounds().toType<float>();
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        
        int nElem = elements.size();
        activeElem = -1;
        float activeDSquared = 80.0f; //dummy value
        if (nElem > 0) {
            Point<int> pos = event.getPosition();
            
            float mouseX = (centreY-pos.getY());
            float mouseY = (centreX-pos.getX());
            
            if (drawPlane == zx) mouseY *= -1;
            
            for (int i = elements.size(); --i >= 0;) {
                handle = (PositionPlaneElement*) elements.getUnchecked (i);
                
                float posH, posW;
                Vector3D<float> position = handle->getPosition();
                switch(drawPlane)
                {
                    case xy:
                        posH = position.x;
                        posW = position.y;
                        break;
                    case zy:
                        posH = position.z;
                        posW = position.y;
                        break;
                    case zx:
                        posH = position.z;
                        posW = position.x;
                        break;
                }
                
                float tx = (mouseX - posH*scale);
                float ty = (mouseY - posW*scale);
                
                float dSquared = tx*tx + ty*ty;
                if (dSquared <= 80.0f && dSquared < activeDSquared) {
                    activeElem = i;
                    activeDSquared = dSquared;
                }
            }
        }
        
        //DBG(activeElem);
        //activeElem = indexofSmallestElement(dist,nElem);
        
    }
    
    void mouseDrag (const MouseEvent &event) override
    {
        Rectangle<float> bounds = getLocalBounds().toType<float>();
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        
        if (activeElem != -1) {
            PositionPlaneElement* handle = (PositionPlaneElement*) elements.getUnchecked (activeElem);
            handle->moveElement(event, centreX, centreY, scale, drawPlane, this);
            handle->repaintAllPlanesImIn();
            sendChanges(handle);
        }
    }
    
    
    
    void setPlane(Planes PlaneToDraw)
    {
        drawPlane = PlaneToDraw;
    }
    
    void addListener (PositionPlaneListener* const listener) {
        jassert (listener != 0);
        if (listener !=0)
            listeners.add (listener);
    };
    void removeListener (PositionPlaneListener* const listener) {
        listeners.removeFirstMatchingValue(listener);
    };
    
    void sendChanges(PositionPlaneElement* element)
    {
        for (int i = listeners.size (); --i >= 0;)
            ((PositionPlaneListener*) listeners.getUnchecked (i))->PositionPlaneElementChanged (this, element);
    }
    
    void addElement (PositionPlaneElement* const element) {
        jassert (element != 0);
        if (element !=0)
        {
            elements.add (element);
            element->addPlane(this);
        }
    };
    void removeElement (PositionPlaneElement* const element) {
        element->removePlane(this);
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
    Planes drawPlane;
    String suffix;
    
    bool autoScale;
    Vector3D<float> dimensions;
    float scale;
    int activeElem;
    bool activeElemWasUpBeforeDrag;
    Array<void*> listeners;
    Array<void*> elements;
};



