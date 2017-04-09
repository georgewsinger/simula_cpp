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
#ifndef OSVR_CONTROLLER_H
#define OSVR_CONTROLLER_H

#include <scenegraph/physicalnode.h>
#include <scenegraph/input/sixdofpointingdevice.h>
#include <scenegraph/input/singlebonetracker.h>
#include <scenegraph/scene.h>

#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>
#include <osvr/ClientKit/InterfaceStateC.h>

#include <string>

namespace motorcar {

	typedef osvr::clientkit::Interface osvrInterface;

	typedef struct osvrControllerInterface {
		osvrInterface tracker;

		osvrInterface one;
		osvrInterface two;
		osvrInterface three;
		osvrInterface four;

		osvrInterface middle;
		osvrInterface bumper;

		osvrInterface joystick;
		osvrInterface joystickX;
		osvrInterface joystickY;

		osvrInterface trigger;
	} osvrControllerInterface;

	class OSVRController : public PhysicalNode
	{
		public:
			OSVRController(std::string side, PhysicalNode *parent, const glm::mat4 &transform = glm::mat4());

			void handleFrameBegin(Scene *scene);

			SixDOFPointingDevice *pointingDevice() const;
			void setPointingDevice(SixDOFPointingDevice *pointingDevice);

			SingleBoneTracker *boneTracker() const;
			void setBoneTracker(SingleBoneTracker *boneTracker);

		private:
			SixDOFPointingDevice *m_pointingDevice;
			SingleBoneTracker *m_boneTracker;

			bool m_triggerDown;

			std::string m_side;

			osvr::clientkit::ClientContext m_context;
			osvrControllerInterface m_interface;
	};
}

#endif
