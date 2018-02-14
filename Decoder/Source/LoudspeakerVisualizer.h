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

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/*
*/
class LoudspeakerVisualizer    : public Component, public OpenGLRenderer, private Timer
{
public:
    LoudspeakerVisualizer(std::vector<float>& pts, std::vector<int>& tris, std::vector<float>& norms) : extPoints(pts), extTriangles(tris), extNormals(norms)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        
        openGLContext.setComponentPaintingEnabled(true);
        openGLContext.setContinuousRepainting(false);
        openGLContext.setRenderer(this);
        openGLContext.attachTo(*this);
        
        startTimer(100);
    }

    ~LoudspeakerVisualizer()
    {
        openGLContext.detach();
        openGLContext.setRenderer(nullptr);
    }

    void initialise()
    {
        PixelARGB colormapData[8];
        colormapData[0] = Colours::cornflowerblue.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[1] = Colours::orange.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[2] = Colours::red.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[3] = Colours::greenyellow.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[4] = Colours::lemonchiffon.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[5] = Colours::cornflowerblue.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[6] = Colours::orange.withMultipliedAlpha(1.0f).getPixelARGB();
        colormapData[7] = Colours::red.withMultipliedAlpha(1.0f).getPixelARGB();
        
        texture.loadARGB(colormapData, 8, 1);
    }
    
    void timerCallback() override {
        openGLContext.triggerRepaint();
    }
    
    void setRmsDataPtr(Array<float>* newRms) {
        pRMS = newRms;
    }
    
    void newOpenGLContextCreated() override
    {
        createShaders();
        initialise();
        updateVerticesAndIndices();
    }
    
    void updateVerticesAndIndices()
    {
        vertices = extPoints;
        indices = extTriangles;
        normals = extNormals;
        
        nVertices = (int) vertices.size();
        nTriangleIndices = (int) indices.size();
        for (int i = 0; i < nVertices; ++i)
            indices.push_back(i);
        
        openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        
        openGLContext.extensions.glGenBuffers(1, &indexBuffer);
        openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
        
        openGLContext.extensions.glGenBuffers(1, &normalsBuffer);
        openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, normalsBuffer);
        openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
    }
    
    void renderOpenGL() override
    {
        jassert (OpenGLHelpers::isContextActive());
        
        OpenGLHelpers::clear (Colour(0xFF2D2D2D));
        //OpenGLHelpers::clear (Colours::white);
        
        const float desktopScale = (float) openGLContext.getRenderingScale();
        glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        
//        glEnable(GL_POLYGON_SMOOTH);
//        glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
        
        
        openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
        
        
        texture.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // linear interpolation when too small
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear interpolation when too bi
        
        
        
        shader->use();
        
        GLint programID = shader->getProgramID();

        
        if (projectionMatrix != nullptr)
            projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
        if (viewMatrix != nullptr)
            viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);
        
        
        std::vector<GLfloat> colorMap_data; // every vertex gets a colour
        std::vector<GLfloat> colorMap_inverse; // every vertex gets a colour
        for (int i = 0; i < nVertices; ++i){
            colorMap_data.push_back(((float) i) / nVertices * 1);
            colorMap_inverse.push_back((float) (nVertices - i - 1) / nVertices);
        }
        
        GLuint colorBuffer;
        openGLContext.extensions.glGenBuffers(1, &colorBuffer);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, colorMap_data.size() * sizeof(GLfloat), &colorMap_data[0], GL_STATIC_DRAW);
        
        GLuint colorBufferInverse;
        openGLContext.extensions.glGenBuffers(1, &colorBufferInverse);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, colorBufferInverse);
        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, colorMap_inverse.size() * sizeof(GLfloat), &colorMap_inverse[0], GL_STATIC_DRAW);
        
        GLint attributeID;
        
        
        // 1st attribute buffer : vertices
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "position");
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                                       3,                  // size
                                                       GL_FLOAT,           // type
                                                       GL_FALSE,           // normalized?
                                                       0,                  // stride
                                                       (void*)0            // array buffer offset
                                                       );
        // 1st attribute buffer : normals
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "normals");
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
        
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                                       4,                  // size
                                                       GL_FLOAT,           // type
                                                       GL_FALSE,           // normalized?
                                                       0,                  // stride
                                                       (void*)0            // array buffer offset
                                                       );
        
        // 2nd attribute buffer : colors
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "colormapDepthIn");
        
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,            // attribute
                                                       1,                      // size
                                                       GL_FLOAT,               // type
                                                       GL_FALSE,               // normalized?
                                                       0,                      // stride
                                                       (void*)0                // array buffer offset
                                                       );
        
        glPointSize(20.0);
        glDrawElements (GL_POINTS, nVertices / 3, GL_UNSIGNED_INT,  (void*)(nTriangleIndices * sizeof(GLuint)));  // Draw points!
        
        
        glLineWidth(5.0);

        
        attributeID = openGLContext.extensions.glGetAttribLocation(programID, "colormapDepthIn");
        
        openGLContext.extensions.glEnableVertexAttribArray(attributeID);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, colorBufferInverse);
        
        openGLContext.extensions.glVertexAttribPointer(
                                                       attributeID,            // attribute
                                                       1,                      // size
                                                       GL_FLOAT,               // type
                                                       GL_FALSE,               // normalized?
                                                       0,                      // stride
                                                       (void*)0                // array buffer offset
                                                       );
        
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangleIndices,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*)0           // element array buffer offset
                       );

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(
                       GL_TRIANGLES,      // mode
                       nTriangleIndices,    // count
                       GL_UNSIGNED_INT,   // type
                       (void*)0           // element array buffer offset
                       );

        openGLContext.extensions.glDisableVertexAttribArray(0);
        openGLContext.extensions.glDisableVertexAttribArray(1);
        openGLContext.extensions.glDisableVertexAttribArray(2);
        
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
        
        openGLContext.extensions.glDeleteBuffers (1, &colorBuffer);
        //openGLContext.extensions.glDeleteBuffers (1, &colorBufferInverse);
    }
    
    void openGLContextClosing() override {
        openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
        openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
        texture.release();
    }
    
    void mouseWheelMove (const MouseEvent &e, const MouseWheelDetails &wheel) override
    {
        const double delta = (std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? -wheel.deltaX : wheel.deltaY);
        
        zoom += delta;
        zoom = jmin(zoom, 5.0f);
        zoom = jmax(zoom, 0.01f);
    }
    
    void mouseDown (const MouseEvent& e) override
    {
        tiltBeforeDrag = tilt;
        yawBeforeDrag = yaw;
    }
    
    void mouseUp (const MouseEvent& e) override
    {}
    
    void mouseDrag (const MouseEvent& e) override {
        float deltaY = (float) e.getDistanceFromDragStartY() / 100;
        tilt = tiltBeforeDrag + deltaY;
        tilt = jmin(tilt, (float) M_PI / 2.0f);
        tilt = jmax(tilt, 0.0f);
        
        float deltaX = (float) e.getDistanceFromDragStartX() / 100;
        yaw = yawBeforeDrag + deltaX;
    }
    
    void paint (Graphics& g) override
    {
        
    }
    
    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
    }
    
    void createShaders()
    {
        vertexShader =
        "attribute vec3 position;\n"
        "attribute vec4 normals;\n"
        "attribute float colormapDepthIn;\n"
        "\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "\n"
        "varying float colormapDepthOut;\n"
        "varying float lightIntensity;\n"
        "void main()\n"
        "{\n"
        "   gl_Position.xyz = position;\n"
        "   gl_Position.w = 1.0;\n"
        "   gl_Position = projectionMatrix * viewMatrix * gl_Position;\n"
        "   vec4 light = vec4(1, 1, 1, 1);\n"
        "   lightIntensity = abs( dot (light, normals));\n"
        "   light = vec4(1, -8, 1, 1);\n"
        "   lightIntensity = lightIntensity + 0.5 * abs( dot (light, normals));\n"
        "   colormapDepthOut = colormapDepthIn;\n"
        "}";
        
        fragmentShader =
        "varying float colormapDepthOut;\n"
        "varying float lightIntensity;\n"
        "uniform sampler2D tex0;\n"
        "void main()\n"
        "{\n"
        //"      gl_FragColor = vec4(1, 1, 1, 1);\n"
        "      gl_FragColor = texture2D(tex0, vec2(colormapDepthOut, 0));\n"
        //"      gl_FragColor.xyz = gl_FragColor.xyz * lightIntensity * 0.25;\n"
        //"      gl_FragColor.w = 1.0;\n"
        "}";
        
        ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
        String statusText;
        
        if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
            && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
            && newShader->link())
        {
            shader = newShader;
            shader->use();
            statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
            
            projectionMatrix = createUniform (openGLContext, *shader, "projectionMatrix");
            viewMatrix       = createUniform (openGLContext, *shader, "viewMatrix");
        }
        else
        {
            statusText = newShader->getLastError();
        }
        
    }
    
    Matrix3D<float> getProjectionMatrix() const
    {
        float w = 1.0f / (0.5f + 0.1f);
        float h = w * getLocalBounds().toFloat().getAspectRatio (false);
        // All objects coordinates in the pre-final view will be modified by multiplying
        // their position vectors by this matrix (see getViewMatrix() below).
        // The first 4 parameters define the view size, the 5th one is a factor that
        // defines from how far the near side will be seen.
        // The 6th one is a factor that defines from how far the far side will be seen.
        // Obiviously this 6th param should be greater than the 5th one.
        // All the values used in this method are somewhat empirical and depend on
        // what you want to show and how close (big) the displayed objects will be.
        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 1.0f, 400.0f);
    }
    
    //==============================================================================
    // This creates a rotation matrix that can be used to rotate an object.
    // A 3-dimension vector is given as an argmument to define the rotations around
    // each axis (x,y,z), using the well-known Euler formulas.
    // As always in our graphics world, a 4th dimension is added to get a homogeneous
    // matrix.
    
    Matrix3D<float> createRotationMatrix (Vector3D<float> eulerAngleRadians) const noexcept
    {
        const float cx = std::cos (eulerAngleRadians.x),  sx = std::sin (eulerAngleRadians.x),
        cy = std::cos (eulerAngleRadians.y),  sy = std::sin (eulerAngleRadians.y),
        cz = std::cos (eulerAngleRadians.z),  sz = std::sin (eulerAngleRadians.z);
        
        return Matrix3D<float> ((cy * cz) + (sx * sy * sz), cx * sz, (cy * sx * sz) - (cz * sy), 0.0f,
                                (cz * sx * sy) - (cy * sz), cx * cz, (cy * cz * sx) + (sy * sz), 0.0f,
                                cx * sy, -sx, cx * cy, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);
    }
    Matrix3D<float> getViewMatrix()
    {
        // The viewMatrix will be used to modify the vertex coordinates in order
        // to transform object coordinates as viewed-by-camera (or eye) coordinates.
        // Standard x,y,z values are used. Obviously the z value used here will have
        // to be in the range near side < z < far side as defined by the frustum used
        // in the projection matrix (see getProjectionMatrix() above).
        Matrix3D<float> viewMatrix (Vector3D<float> (0.0f, 0.0f, -50.0f * zoom)); // shifts
        
        // The rotation matrix will be applied on each frame.
        // The vector passed here contains the Euler angle values for each axis
        // The empiric values used as params will create a slight but constant tilting
        // on the x-axis, a periodic rotation on the y-axis and no rotation on the
        // z-axis.
        
        Matrix3D<float> tiltMatrix
        = createRotationMatrix (Vector3D<float> (tilt, 0.0f, 0.0f));
        

        Matrix3D<float> rotationMatrix
        = createRotationMatrix (Vector3D<float> (0.0f, yaw, 0.0f));
        
        return rotationMatrix * tiltMatrix  * viewMatrix;
    }
    
private:
    GLuint vertexBuffer, indexBuffer, normalsBuffer;
    const char* vertexShader;
    const char* fragmentShader;
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;
    
    static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                        OpenGLShaderProgram& shaderProgram,
                                                        const char* uniformName)
    {
        // Get the ID
        if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
            return nullptr; // Return if error
        // Create the uniform variable
        return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
    }
    
    std::vector<float>& extPoints;
    std::vector<int>& extTriangles;
    std::vector<float>& extNormals;
    
    std::vector<float> vertices;
    std::vector<int> indices;
    std::vector<float> normals;
    
    int nVertices;
    int nTriangleIndices;
    
    float zoom = 2.0f;
    float tilt = 0.0f;
    float tiltBeforeDrag;
    
    float yaw = 0.0f;
    float yawBeforeDrag;
    
    OpenGLTexture texture;
    
    bool firstRun = true;
    
    Array<float>* pRMS;
    
    OpenGLContext openGLContext;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudspeakerVisualizer)
};
