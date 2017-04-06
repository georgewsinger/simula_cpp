/****************************************************************************
**This file is part of the Motorcar QtWayland Compositor, derived from the
**QtWayland QWindow Reference Compositor
**
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
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

#ifndef OSVRQWINDOWCOMPOSITOR_H
#define OSVRQWINDOWCOMPOSITOR_H

#include <qt/qtwaylandmotorcarcompositor.h>

#include <osvr/ClientKit/Context.h>
#include <osvr/RenderKit/RenderManager.h>

namespace qtmotorcar{


class OsvrQtWaylandMotorcarCompositor : public QtWaylandMotorcarCompositor
{
public:
    OsvrQtWaylandMotorcarCompositor(QOpenGLWindow *window, QGuiApplication *app, motorcar::Scene *scene);
    ~OsvrQtWaylandMotorcarCompositor();

	static OsvrQtWaylandMotorcarCompositor* create(int &argc, char** argv, motorcar::Scene *scene);

    virtual int start() override;

	void registerOsvrWindow(QOpenGLWindow *window);

protected slots:
    void render() override;
    void renderPreview();

protected:
    QTimer m_renderPreviewScheduler;

	osvr::clientkit::ClientContext m_context;
	osvr::renderkit::RenderManager* m_render;

	osvr::renderkit::RenderManager::RenderParams m_params;

private:
	static void drawScene(
		void* userData
		, osvr::renderkit::GraphicsLibrary library
		, osvr::renderkit::RenderBuffer buffers
		, osvr::renderkit::OSVR_ViewportDescription viewport
		, OSVR_PoseState pose
		, osvr::renderkit::OSVR_ProjectionMatrix projection
		, OSVR_TimeValue deadline
    );

    QOpenGLWindow *m_osvr_window;
};

}

#endif // OSVRQWINDOWCOMPOSITOR_H
