/****************************************************************************
**This file is part of the MotorCar QtWayland Compositor, derived from the
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

#include <osvr/osvrqtwaylandmotorcarcompositor.h>
#include <scenegraph/output/display/osvrdisplay.h>
#include <qt/qopenglwindow.h>

#include <osvr/RenderKit/GraphicsLibraryOpenGL.h>
#include <osvr/RenderKit/RenderManagerOpenGLC.h>

#include <QScreen>
#include <QOpenGLContext>
#include <QDateTime>
//#include <QMouseEvent>
//#include <QTouchEvent>

#include <sys/time.h>

#include <iostream>

using namespace qtmotorcar;

class Qt5ToolkitImpl {
    OSVR_OpenGLToolkitFunctions toolkit;

    static void createImpl(void* data) {
    }
    static void destroyImpl(void* data) {
        delete ((Qt5ToolkitImpl*)data);
    }
    static OSVR_CBool addOpenGLContextImpl(void* data, const OSVR_OpenGLContextParams* p) {
        return ((Qt5ToolkitImpl*)data)->addOpenGLContext(p);
    }
    static OSVR_CBool removeOpenGLContextsImpl(void* data) {
        return ((Qt5ToolkitImpl*)data)->removeOpenGLContexts();
    }
    static OSVR_CBool makeCurrentImpl(void* data, size_t display) {
        return ((Qt5ToolkitImpl*)data)->makeCurrent(display);
    }
    static OSVR_CBool swapBuffersImpl(void* data, size_t display) {
        return ((Qt5ToolkitImpl*)data)->swapBuffers(display);
    }
    static OSVR_CBool setVerticalSyncImpl(void* data, OSVR_CBool verticalSync) {
        return ((Qt5ToolkitImpl*)data)->setVerticalSync(verticalSync);
    }
    static OSVR_CBool handleEventsImpl(void* data) {
        return ((Qt5ToolkitImpl*)data)->handleEvents();
    }
    static OSVR_CBool getDisplayFrameBufferImpl(void* data, size_t display, GLuint* displayFrameBufferOut) {
      return ((Qt5ToolkitImpl*)data)->getDisplayFrameBuffer(display, displayFrameBufferOut);
    }
    static OSVR_CBool getDisplaySizeOverrideImpl(void* data, size_t display, int* width, int* height) {
        return ((Qt5ToolkitImpl*)data)->getDisplaySizeOverride(display, width, height);
    }

    QList<QOpenGLWindow*> windows;
	OsvrQtWaylandMotorcarCompositor *m_compositor;

  public:
    Qt5ToolkitImpl(OsvrQtWaylandMotorcarCompositor *compositor) {
		m_compositor = compositor;

        memset(&toolkit, 0, sizeof(toolkit));
        toolkit.size = sizeof(toolkit);
        toolkit.data = this;

        toolkit.create = createImpl;
        toolkit.destroy = destroyImpl;
        toolkit.addOpenGLContext = addOpenGLContextImpl;
        toolkit.removeOpenGLContexts = removeOpenGLContextsImpl;
        toolkit.makeCurrent = makeCurrentImpl;
        toolkit.swapBuffers = swapBuffersImpl;
        toolkit.setVerticalSync = setVerticalSyncImpl;
        toolkit.handleEvents = handleEventsImpl;
        toolkit.getDisplaySizeOverride = getDisplaySizeOverrideImpl;
    }

    ~Qt5ToolkitImpl() {
    }

    const OSVR_OpenGLToolkitFunctions* getToolkit() const { return &toolkit; }

    bool addOpenGLContext(const OSVR_OpenGLContextParams* p) {
    
        QRect screenGeometry(p->xPos, p->yPos, p->width, p->height);

        QSurfaceFormat format;
        format.setDepthBufferSize(8);
        format.setStencilBufferSize(8);
        format.setSwapInterval(1);
        format.setStencilBufferSize(8);

        QOpenGLWindow *window;

		if (windows.size()) {
			window = new QOpenGLWindow(m_compositor->m_window->context(), format, screenGeometry);
		} else {
			window = m_compositor->m_window;
			window->setGeometry(screenGeometry);
			window->setFormat(format);
		}

		m_compositor->registerOsvrWindow(window);

		window->hide();
        if (p->fullScreen) {
            window->showFullScreen();
        } else {
            window->showNormal();
        }

        windows.push_back(window);

        return true;
    }
    bool removeOpenGLContexts() {
        for (auto window : windows) {
          window->deleteLater();
        }
        windows.clear();

        return true;
    }
    bool makeCurrent(size_t display) {
        windows.at(static_cast<int>(display))->makeCurrent();
        return true;
    }
    bool swapBuffers(size_t display) {
        windows.at(static_cast<int>(display))->swapBuffers();
        return true;
    }
    bool setVerticalSync(bool verticalSync) {
      return true;
    }
    bool handleEvents() {
      return true;
    }
    bool getDisplayFrameBuffer(size_t display, GLuint* displayFrameBufferOut) {
      *displayFrameBufferOut = 0;
      return true;
    }
    bool getDisplaySizeOverride(size_t display, int* width, int* height) {
        // we don't override the display. Use default behavior.
        return false;
    }
};

OsvrQtWaylandMotorcarCompositor::OsvrQtWaylandMotorcarCompositor(QOpenGLWindow *window, QGuiApplication *app, motorcar::Scene * scene)
    : QtWaylandMotorcarCompositor(window, app, scene)
      , m_context("com.motorcar")
{
    m_renderPreviewScheduler.setSingleShot(true);
    connect(&m_renderPreviewScheduler,SIGNAL(timeout()),this,SLOT(renderPreview()));

}

OsvrQtWaylandMotorcarCompositor::~OsvrQtWaylandMotorcarCompositor()
{
	delete m_render;
}

OsvrQtWaylandMotorcarCompositor *OsvrQtWaylandMotorcarCompositor::create(int argc, char** argv, motorcar::Scene *scene)
{
    QGuiApplication *app = new QGuiApplication(argc, argv);
    QScreen *screen = NULL;

    screen = QGuiApplication::screens().back();
    QRect screenGeometry = screen->geometry();

    QSurfaceFormat format;
    format.setDepthBufferSize(8);
    format.setStencilBufferSize(8);
    format.setSwapInterval(1);
    format.setStencilBufferSize(8);

    QOpenGLWindow *window = new QOpenGLWindow(format, screenGeometry);
    return new OsvrQtWaylandMotorcarCompositor(window, app, scene);
}

int OsvrQtWaylandMotorcarCompositor::start()
{
    auto toolkit = new Qt5ToolkitImpl(this);

    osvr::renderkit::GraphicsLibrary myLibrary;
    myLibrary.OpenGL = new osvr::renderkit::GraphicsLibraryOpenGL;
    myLibrary.OpenGL->toolkit = toolkit->getToolkit();

    m_render = osvr::renderkit::createRenderManager(m_context.get(), "OpenGL", myLibrary);

    if ((m_render == nullptr) || (!m_render->doingOkay())) {
        std::cerr << "Could not create RenderManager" << std::endl;
    }

    // Open the display and make sure this worked.
    osvr::renderkit::RenderManager::OpenResults ret = m_render->OpenDisplay();
    if (ret.status == osvr::renderkit::RenderManager::OpenStatus::FAILURE) {
        std::cerr << "Could not open display" << std::endl;
    }

    m_params.nearClipDistanceMeters = 0.01;
    m_params.farClipDistanceMeters = 100.0;

	motorcar::OSVRDisplay::SetupRendering(myLibrary);
    m_render->SetDisplayCallback(motorcar::OSVRDisplay::SetupDisplay, display());
    m_render->SetViewProjectionCallback(motorcar::OSVRDisplay::SetupEye, display());
    m_render->AddRenderCallback("/", drawScene, this);

    QtWaylandMotorcarCompositor::start();
}


void OsvrQtWaylandMotorcarCompositor::registerOsvrWindow(QOpenGLWindow *window) {
	osvr::renderkit::RenderTimingInfo info;
	m_render->GetTimingInfo(0, info);
	setOutputGeometry(QRect(QPoint(0, 0), window->size()));
	setOutputRefreshRate( qRound(1000000.0 / info.hardwareDisplayInterval.microseconds) );
}

void OsvrQtWaylandMotorcarCompositor::drawScene(
        void* userData
        , osvr::renderkit::GraphicsLibrary library
        , osvr::renderkit::RenderBuffer buffers
        , osvr::renderkit::OSVR_ViewportDescription viewport
        , OSVR_PoseState pose
        , osvr::renderkit::OSVR_ProjectionMatrix projection
        , OSVR_TimeValue deadline
    )
    
{
    OsvrQtWaylandMotorcarCompositor *compositor = static_cast<OsvrQtWaylandMotorcarCompositor *>(userData);

    motorcar::OSVRDisplay::DrawWorld(compositor->display(), library, buffers, viewport, pose, projection, deadline);

    compositor->scene()->drawFrame();
}

void OsvrQtWaylandMotorcarCompositor::render()
{
    m_context.update();

	display()->prepareForDraw();

    scene()->prepareForFrame(QDateTime::currentMSecsSinceEpoch());
    if (m_render->Render(m_params)) {
        m_renderScheduler.start(0);
    } else {
		m_window->close();
	}
    scene()->finishFrame();

	display()->prepareForDraw();
}

void OsvrQtWaylandMotorcarCompositor::renderPreview()
{
    /*
    m_glData->m_window->makeCurrent();
    frameStarted();
    cleanupGraphicsResources();
    scene()->prepareForFrame(QDateTime::currentMSecsSinceEpoch());
    sendFrameCallbacks(surfaces());
    scene()->drawFrame();
    scene()->finishFrame();

    m_glData->m_window->swapBuffers();

    struct timeval tv;
    static const int32_t benchmark_interval = 5;
    gettimeofday(&tv, NULL);
    uint32_t time = QDateTime::currentMSecsSinceEpoch();
    if (m_frames == 0)
        m_benchmark_time = time;
    if (time - m_benchmark_time > (benchmark_interval * 1000)) {
        std::cout << m_frames << " frames in " << benchmark_interval
                  << " seconds: " << (float)m_frames / benchmark_interval
                  << std::endl;
        m_benchmark_time = time;
        m_frames = 0;
    }
    m_frames++;
    */

    m_renderPreviewScheduler.start(16);
}
