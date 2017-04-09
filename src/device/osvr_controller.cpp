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
#include "osvr_controller.h"
using namespace motorcar;

#include <compositor.h>
#include <string>

OSVRController::OSVRController(std::string side, PhysicalNode *parent, const glm::mat4 &transform )
	: PhysicalNode(parent, transform)
	, m_pointingDevice(NULL)
	, m_boneTracker(NULL)
	, m_triggerDown(false)
	, m_side(side)
	, m_context("com.motorcar")
{

	m_interface.tracker = m_context.getInterface("/controller/" + side);
	m_interface.one = m_context.getInterface("/controller/" + side + "/1");
	m_interface.two = m_context.getInterface("/controller/" + side + "/2");
	m_interface.three = m_context.getInterface("/controller/" + side + "/3");
	m_interface.four = m_context.getInterface("/controller/" + side + "/4");
	m_interface.middle = m_context.getInterface("/controller/" + side + "/middle");
	m_interface.bumper = m_context.getInterface("/controller/" + side + "/bumper");
	m_interface.joystick = m_context.getInterface("/controller/" + side + "/joystick/button");
	m_interface.joystickX = m_context.getInterface("/controller/" + side + "/joystick/x");
	m_interface.joystickY = m_context.getInterface("/controller/" + side + "/joystick/y");
	m_interface.trigger = m_context.getInterface("/controller/" + side + "/trigger");

}

void OSVRController::handleFrameBegin(Scene *scene) {
	PhysicalNode::handleFrameBegin(scene);

	m_context.update();

	OSVR_PoseState pose;
	OSVR_TimeValue timestamp;
	OSVR_ReturnCode ret = osvrGetPoseState(m_interface.tracker.get(), &timestamp, &pose);

	glm::vec3 position = glm::vec3(
		osvrVec3GetX(&pose.translation), 
		osvrVec3GetY(&pose.translation), 
		osvrVec3GetZ(&pose.translation)
	);

	glm::quat orientation;
	orientation.x = osvrQuatGetX(&(pose.rotation));
	orientation.y = osvrQuatGetY(&(pose.rotation));
	orientation.z = osvrQuatGetZ(&(pose.rotation));
	orientation.w = osvrQuatGetW(&(pose.rotation));
	glm::mat4 transform = glm::translate(glm::mat4(), position) * glm::mat4_cast(orientation);

	this->setTransform(transform);

	if(m_pointingDevice != NULL){
		OSVR_AnalogState trigger = 0;
		ret = osvrGetAnalogState(m_interface.trigger.get(), &timestamp, &trigger);
		if(trigger > 25){
			if(!m_triggerDown){
				m_triggerDown = true;
				m_pointingDevice->grabSurfaceUnderCursor();
			}
		}else{
			if(m_triggerDown){
				m_triggerDown = false;
				m_pointingDevice->releaseGrabbedSurface();
			}
		}

		OSVR_ButtonState button = 1;

		ret = osvrGetButtonState(m_interface.middle.get(), &timestamp, &button);
		if (button) std::cout << "Left button pressed" << std::endl;
		m_pointingDevice->setLeftMouseDown(button);

		ret = osvrGetButtonState(m_interface.two.get(), &timestamp, &button);
		m_pointingDevice->setRightMouseDown(button);

		ret = osvrGetButtonState(m_interface.four.get(), &timestamp, &button);
		m_pointingDevice->setMiddleMouseDown(button);
	}
}

SixDOFPointingDevice *OSVRController::pointingDevice() const
{
	return m_pointingDevice;
}

void OSVRController::setPointingDevice(SixDOFPointingDevice *pointingDevice)
{
	m_pointingDevice = pointingDevice;
}
SingleBoneTracker *OSVRController::boneTracker() const
{
	return m_boneTracker;
}

void OSVRController::setBoneTracker(SingleBoneTracker *boneTracker)
{
	m_boneTracker = boneTracker;
}

