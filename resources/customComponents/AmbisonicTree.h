/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
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

class  AmbisonicTree :  public Component
{
public:
    AmbisonicTree() : Component() {};
    ~AmbisonicTree() {};
    
    void setOrder (int i) {
        order = i;
        repaint();
    }
    void setActiveOrder (int i) {
        active = i;
        repaint();
    }

    void paint (Graphics& g) override
    {
        const Rectangle<int> bounds = getBounds();
        const float centreX = (float)bounds.getX() + 0.5* (float)bounds.getWidth();
        const float centreY = (float)bounds.getY() + 0.5* (float)bounds.getHeight();
        const float rectW = 3.0f;
        const float rectH = 5.0f;
        const float moveX = rectW+1.0f;
        const float moveY = rectH+1.0f;
        const float cornerSize = 1.0f;
        
        const float startY = centreY - (order+1)*0.5*moveY;
        int count = 0;
        Path act;
        Path inact;
        Path* handle;
        
        handle = &act;

        
        for (int i = 0; i<=order; i++)
        {
            for (int j = 0; j<=2*i; j++)
            {
                int offsetX = i*moveX;
                handle->addRoundedRectangle(centreX - offsetX + j*moveX - 0.5*rectW, startY + i*moveY - 0.5*rectH, rectW, rectH, cornerSize);
                if (++count == active) handle = &inact;
            }

            
            
        }
        
        //circle.addEllipse(centreX - 0.5*diameter, centreY - 0.5*diameter, diameter, diameter);
        
        g.setColour(Colours::white);
        g.fillPath(act);
        
        g.setColour((Colours::white).withMultipliedAlpha(0.8));
        g.fillPath(inact);
        
        g.setColour((Colours::white).withMultipliedAlpha(0.2));
        g.drawRect(bounds);
       
        
    };
private:
    int order = 7;
    int active = 13;

};




//void paint (Graphics& g) override
//{
//    const Rectangle<int> bounds = getBounds();
//    const float centreX = (float)bounds.getX() + 0.5* (float)bounds.getWidth();
//    const float centreY = (float)bounds.getY() + 0.5* (float)bounds.getHeight();
//    const float diameter = 8.0f;
//    const float outlineThickness = .5f;
//    const float move = 8.0f;
//    
//    const float startY = centreY - (order+1)*0.5*move;
//    
//    Path circle;
//    
//    
//    
//    
//    for (int i = 0; i<=order; i++)
//    {
//        for (int j = 0; j<=2*i; j++)
//        {
//            int offsetX = i*move;
//            circle.addEllipse(centreX - offsetX + j*move - 0.5*diameter, startY + i*move - 0.5*diameter, diameter, diameter);
//        }
//        g.setColour((Colours::white).withMultipliedAlpha(0.5));
//        if (i == active) g.fillPath(circle);
//    }
//    
//    //circle.addEllipse(centreX - 0.5*diameter, centreY - 0.5*diameter, diameter, diameter);
//    
//    g.setColour(Colours::white);
//    g.strokePath(circle,PathStrokeType(outlineThickness));
//    
//    //g.drawRect(bounds);
//    
//    
//};
