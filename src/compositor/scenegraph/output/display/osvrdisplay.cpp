/****************************************************************************
**This file is part of the Motorcar 3D windowing framework
**
**
**Copyright (C) 2014 Forrest Reiling
**
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/
#include <scenegraph/output/display/osvrdisplay.h>
#include <scenegraph/output/viewpoint.h>

#include <iostream>

using namespace motorcar;


OSVRDisplay::OSVRDisplay(OpenGLContext *glContext, glm::vec2 displayDimensions, PhysicalNode *parent, const glm::mat4 &transform)
    : Display(glContext, displayDimensions, parent, transform)

{
    float camToDisplayDistance = 0.1f;
    ViewPoint *vp = new ViewPoint(.01f, 100.0f, this, this, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, camToDisplayDistance)));
    addViewpoint(vp);
}

OSVRDisplay::~OSVRDisplay()
{
    glDeleteTextures(1, &m_scratchColorBufferTexture);
    glDeleteTextures(1, &m_scratchDepthBufferTexture);
    glDeleteFramebuffers(1, &m_scratchFrameBuffer);
}

bool OSVRDisplay::SetupRendering(osvr::renderkit::GraphicsLibrary library) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr << "SetupRendering: No OpenGL GraphicsLibrary, this should "
                     "not happen"
                  << std::endl;
        return false;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    // Turn on depth testing, so we get correct ordering.
    glEnable(GL_DEPTH_TEST);

    return true;
}

// Callback to set up a given display, which may have one or more eyes in it
void OSVRDisplay::SetupDisplay(
    void* userData //< Passed into SetDisplayCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "SetupDisplay: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr
            << "SetupDisplay: No OpenGL RenderBuffer, this should not happen"
            << std::endl;
        return;
    }

    OSVRDisplay *display = reinterpret_cast<OSVRDisplay *>(userData);

    display->glContext()->makeCurrent();

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    // Clear the screen to black and clear depth
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Callback to set up for rendering into a given eye (viewpoint and projection).
void OSVRDisplay::SetupEye(
    void* userData //< Passed into SetViewProjectionCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    , osvr::renderkit::OSVR_ViewportDescription
        viewport //< Viewport set by RenderManager
    , osvr::renderkit::OSVR_ProjectionMatrix
        projectionToUse //< Projection matrix set by RenderManager
    , size_t whichEye //< Which eye are we setting up for?
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "SetupEye: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr << "SetupEye: No OpenGL RenderBuffer, this should not happen"
                  << std::endl;
        return;
    }
    
    OSVRDisplay *display = reinterpret_cast<OSVRDisplay *>(userData);

    ViewPoint *viewpoint = display->viewpoints().at(0);

    // Set the viewport
    ViewPort *motorcarViewport = viewpoint->viewport();

    glm::vec2 displaySize = display->size();
    glm::vec2 vpPos(viewport.left / displaySize.x, viewport.lower / displaySize.y);
    glm::vec2 vpSize(viewport.width / displaySize.x, viewport.height / displaySize.y);

    motorcarViewport->update(vpPos, vpSize, display);

    GLdouble glProjection[16];
	osvr::renderkit::OSVR_Projection_to_OpenGL(glProjection, projectionToUse);

    glm::mat4 glmProjection( 
        glProjection[0], glProjection[1], glProjection[2], glProjection[3],
        glProjection[4], glProjection[5], glProjection[6], glProjection[7],
        glProjection[8], glProjection[9], glProjection[10], glProjection[11],
        glProjection[12], glProjection[13], glProjection[14], glProjection[15]
    );
    
    viewpoint->overrideProjectionMatrix(glmProjection);
}

// Callbacks to draw things in world space, left-hand space, and right-hand
// space.
void OSVRDisplay::DrawWorld(
    void* userData //< Passed into AddRenderCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    , osvr::renderkit::OSVR_ViewportDescription
        viewport //< Viewport we're rendering into
    , OSVR_PoseState pose //< OSVR ModelView matrix set by RenderManager
    , osvr::renderkit::OSVR_ProjectionMatrix
        projection //< Projection matrix set by RenderManager
    , OSVR_TimeValue deadline //< When the frame should be sent to the screen
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "DrawWorld: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr << "DrawWorld: No OpenGL RenderBuffer, this should not happen"
                  << std::endl;
        return;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    OSVRDisplay *display = reinterpret_cast<OSVRDisplay *>(userData);

    ViewPoint *viewpoint = display->viewpoints().at(0);
	glm::mat4 position = glm::translate(
			glm::mat4(), 
			glm::vec3(pose.translation.data[0], pose.translation.data[1], pose.translation.data[2])
	);

    pose.translation.data[0] = pose.translation.data[1] = pose.translation.data[2] = 0;

    /// Put the transform into the OpenGL ModelView matrix
    GLdouble glModelView[16];
    osvr::renderkit::OSVR_PoseState_to_OpenGL(glModelView, pose);

    // Why invert the matrix? Who knows but it's backwards otherwise
    glm::mat4 orientation( 
        glModelView[0], glModelView[4], glModelView[8], glModelView[12],
        glModelView[1], glModelView[5], glModelView[9], glModelView[13],
        glModelView[2], glModelView[6], glModelView[10], glModelView[14],
        glModelView[3], glModelView[7], glModelView[11], glModelView[15]
    );
    viewpoint->setWorldTransform(position * orientation);
    viewpoint->updateViewMatrix();

}
