/****************************************************************************
**This file is part of the Motorcar 3D windowing framework
**
**
**Copyright (C) 2015 Forrest Reiling
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
#include "osvr_hmd.h"

using namespace motorcar;

void OsvrHMD::prepareForDraw()
{

    RenderToTextureDisplay::prepareForDraw();


    osvrcontext.update();
    OSVR_PoseState state;
    OSVR_TimeValue timestamp;
    OSVR_ReturnCode ret = osvrGetPoseState(head.get(), &timestamp, &state);
    if (OSVR_RETURN_SUCCESS != ret) {
        std::cout << "No pose state!" << std::endl;
    } else {
        std::cout << "Got POSE state: Position = ("
                  << state.translation.data[0] << ", "
                  << state.translation.data[1] << ", "
                  << state.translation.data[2] << "), orientation = ("
                  << osvrQuatGetW(&(state.rotation)) << ", ("
                  << osvrQuatGetX(&(state.rotation)) << ", "
                  << osvrQuatGetY(&(state.rotation)) << ", "
                  << osvrQuatGetZ(&(state.rotation)) << ")"
                  << std::endl;
    }

	glm::vec3 position = glm::vec3(state.translation.data[0], state.translation.data[1], state.translation.data[2]);
    glm::quat orientation;
    orientation.x = osvrQuatGetW(&(state.rotation));
    orientation.y = osvrQuatGetW(&(state.rotation));
    orientation.z = osvrQuatGetW(&(state.rotation));
    orientation.w = osvrQuatGetW(&(state.rotation));
    glm::mat4 transform = glm::translate(glm::mat4(), position) * glm::mat4_cast(orientation);
    this->setTransform(transform);

}


void OsvrHMD::finishDraw()
{
	RenderToTextureDisplay::finishDraw();

	m_frameIndex++;

}


OsvrHMD::OsvrHMD(Skeleton *skeleton, OpenGLContext *glContext, PhysicalNode *parent)
    :RenderToTextureDisplay(glContext, glm::vec2(0.126, 0.0706), parent, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.10f)))
    , m_initialized(true)
    , m_frameIndex(0)
	, osvrcontext("com.motorcar")
{
    

	head = osvrcontext.getInterface("/me/head");

    osvrcontext.update();

    OSVR_ReturnCode ret = osvrGetPoseState(head.get(), &timestamp, &state);
    if (OSVR_RETURN_SUCCESS != ret) {
        std::cout << "No pose state!" << std::endl;
    } else {
        std::cout << "Got POSE state: Position = ("
                  << state.translation.data[0] << ", "
                  << state.translation.data[1] << ", "
                  << state.translation.data[2] << "), orientation = ("
                  << osvrQuatGetW(&(state.rotation)) << ", ("
                  << osvrQuatGetX(&(state.rotation)) << ", "
                  << osvrQuatGetY(&(state.rotation)) << ", "
                  << osvrQuatGetZ(&(state.rotation)) << ")"
                  << std::endl;
    }

    int win_width, win_height;
    int fb_width, fb_height;

	printf("detected %d hmds\n", -1);



	/*
	//TODO: fill in
	glm::vec4 normalizedViewportParams = glm::vec4( 1,1,1,1);
	float near = .01f, far = 10.0f;

	glm::vec3 HmdToEyeViewOffset = -1.0f * glm::vec3(1,
												1,
												1 );

	ViewPoint *vp = new ViewPoint(near, far, this, this,
                                 glm::translate(glm::mat4(), HmdToEyeViewOffset),
                                 normalizedViewportParams, glm::vec3(0));
	addViewpoint(vp);
	*/
}



OsvrHMD::~OsvrHMD()
{
}

