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

#include <qt/qtwaylandmotorcarcompositor.h>
#include <qt/qtwaylandmotorcarsurface.h>
#include <qt/qtwaylandmotorcarseat.h>


#include <QtCompositor/qwaylandsurface.h>
#include <QtCompositor/qwaylandcompositor.h>
#include <QtCompositor/qwaylandinput.h>

#include <QScreen>
#include <QDateTime>
#include <QKeyEvent>
#include <QMouseEvent>
//#include <QTouchEvent>

#include <sys/time.h>

#include <iostream>

using namespace qtmotorcar;

QtWaylandMotorcarCompositor::QtWaylandMotorcarCompositor(QOpenGLWindow *window, QGuiApplication *app, motorcar::Scene * scene)
    : QWaylandCompositor(0, DefaultExtensions | SubSurfaceExtension)
    , m_scene(scene)
    , m_glData(new OpenGLData(window))
    , m_renderScheduler(this)
    , m_draggingWindow(0)
    , m_dragKeyIsPressed(false)
    , m_cursorSurface(NULL)
    , m_cursorHotspotX(0)
    , m_cursorHotspotY(0)
    , m_camIsMoving(false)
    , m_modifiers(Qt::NoModifier)
    , m_app(app)
    , m_defaultSeat(NULL)
    , m_frames(0)
    , m_window(window)
{
    m_window->makeCurrent();
    setDisplay(NULL);

    m_renderScheduler.setSingleShot(true);
    connect(&m_renderScheduler,SIGNAL(timeout()),this,SLOT(render()));

    window->installEventFilter(this);

    setRetainedSelectionEnabled(true);

    setOutputGeometry(QRect(QPoint(0, 0), window->size()));
    setOutputRefreshRate(qRound(qGuiApp->primaryScreen()->refreshRate() * 1000.0));

    m_defaultSeat = new QtWaylandMotorcarSeat(this->defaultInputDevice());
    createOutput(window, "", "");
    addDefaultShell();
}

QtWaylandMotorcarCompositor::~QtWaylandMotorcarCompositor()
{
    delete m_glData;
}

QtWaylandMotorcarCompositor *QtWaylandMotorcarCompositor::create(int &argc, char** argv, motorcar::Scene *scene)
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
    return  new QtWaylandMotorcarCompositor(window, app, scene);
}

int QtWaylandMotorcarCompositor::start()
{
    if (!this->glData()->m_window->isVisible()) this->glData()->m_window->showFullScreen();
    this->cleanupGraphicsResources();
    int result = m_app->exec();
    delete m_app;
    return result;
}

motorcar::OpenGLContext *QtWaylandMotorcarCompositor::getContext()
{
    return new QtWaylandMotorcarOpenGLContext(this->glData()->m_window);
}

wl_display *QtWaylandMotorcarCompositor::wlDisplay()
{
    return QWaylandCompositor::waylandDisplay();
}

motorcar::WaylandSurface *QtWaylandMotorcarCompositor::getSurfaceFromResource(wl_resource *resource)
{
    QWaylandSurface *surface = QWaylandSurface::fromResource(resource);
    std::cout << "got surface from resource: " << surface <<std::endl;

    QtWaylandMotorcarSurface *motorsurface = this->getMotorcarSurface(surface);
    if(motorsurface == NULL){
        std::cout << "Warning: surface has not been created, creating now " << surface <<std::endl;
        motorsurface = new QtWaylandMotorcarSurface(surface, this, motorcar::WaylandSurface::SurfaceType::NA);
    }

    return motorsurface;
}

OpenGLData *QtWaylandMotorcarCompositor::glData() const
{
    return m_glData;
}

void QtWaylandMotorcarCompositor::setGlData(OpenGLData *glData)
{
    m_glData = glData;
}

motorcar::Scene *QtWaylandMotorcarCompositor::scene() const
{
    return m_scene;
}

void QtWaylandMotorcarCompositor::setScene(motorcar::Scene *scene)
{
    m_scene = scene;
}

QtWaylandMotorcarSurface *QtWaylandMotorcarCompositor::getMotorcarSurface(QWaylandSurface *surface) const
{
    //if passed NULL return first Surface
    if(surface == NULL){
        if(!m_surfaceMap.empty()){
            return m_surfaceMap.begin()->second;
        }else{
            return NULL;
        }
    }
    if(!m_surfaceMap.empty() && m_surfaceMap.count(surface)){
        auto it = m_surfaceMap.find(surface);
        if(it != m_surfaceMap.end()){
            return it->second;
        }
    }
    return NULL;
}

//  @@TODO: consider revising to take  MotorcarSurfaceNode as argument depending on call sites
void QtWaylandMotorcarCompositor::ensureKeyboardFocusSurface(QWaylandSurface *oldSurface)
{

}

void QtWaylandMotorcarCompositor::surfaceDestroyed()
{
    QWaylandSurface *surface = static_cast<QWaylandSurface *>(sender());

    if(surface != NULL){
        // Get surfaceNode whose destructor will remove it from the scenegraph
        motorcar::WaylandSurface *motorcarsurface = this->getMotorcarSurface(surface);
        if(motorcarsurface != NULL){
            this->scene()->windowManager()->destroySurface(motorcarsurface);
            m_surfaceMap.erase (surface);
            //destroy surface;
        }
    }
}

void QtWaylandMotorcarCompositor::surfaceMapped()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    QPoint pos;

    std::cout << "mapped surface: " << surface << std::endl;

    motorcar::WaylandSurface::SurfaceType surfaceType;

    int type = static_cast<int>(surface->windowType());
    float popupZOffset = 0.05f;

    if(type == QWaylandSurface::WindowType::Toplevel){
        surfaceType = motorcar::WaylandSurface::SurfaceType::TOPLEVEL;
    }else if(type == QWaylandSurface::WindowType::Popup){
        surfaceType = motorcar::WaylandSurface::SurfaceType::POPUP;
    }else if(type == QWaylandSurface::WindowType::Transient){
        surfaceType = motorcar::WaylandSurface::SurfaceType::TRANSIENT;
    }else{
        surfaceType = motorcar::WaylandSurface::SurfaceType::NA;
    }

    QtWaylandMotorcarSurface *motorsurface = this->getMotorcarSurface(surface);
    // if it is not present for some weird reason just go
    // ahead and create it for good measure
    if(motorsurface == NULL){
        std::cout << "Warning: qwaylandsurface was mapped but motorcar surface does not exist yet, creating now" <<std::endl;
        motorsurface = new QtWaylandMotorcarSurface(surface, this, motorcar::WaylandSurface::SurfaceType::NA);
        m_surfaceMap.insert(std::pair<QWaylandSurface *, QtWaylandMotorcarSurface *>(surface, motorsurface));
    }

    this->scene()->windowManager()->mapSurface(motorsurface, surfaceType);
}

void QtWaylandMotorcarCompositor::surfaceUnmapped()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

    if(surface != NULL){
        motorcar::WaylandSurface *motorsurface = this->getMotorcarSurface(surface);
        if(motorsurface != NULL){
            this->scene()->windowManager()->unmapSurface(motorsurface);
        }else{
            std::cout << "Warning: surface unmapped but doesnt have associated surfaceNode" <<std::endl;
        }
    }

    ensureKeyboardFocusSurface(surface);
}

void QtWaylandMotorcarCompositor::surfaceDamaged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

    if(surface != NULL){
        motorcar::WaylandSurface *motorsurface = this->getMotorcarSurface(surface);
        if(motorsurface != NULL){
        }else{
        }
    }
    surfaceDamaged(surface);
}

void QtWaylandMotorcarCompositor::surfaceDamaged(QWaylandSurface *surface)
{
    Q_UNUSED(surface)
    m_renderScheduler.start(0);
}

void QtWaylandMotorcarCompositor::surfacePosChanged()
{
}

void QtWaylandMotorcarCompositor::surfaceCreated(QWaylandSurface *surface)
{
    connect(surface, SIGNAL(surfaceDestroyed()), this, SLOT(surfaceDestroyed()));
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this, SLOT(surfaceUnmapped()));
    connect(surface, SIGNAL(redraw()), this, SLOT(surfaceDamaged()));
    connect(surface, SIGNAL(extendedSurfaceReady()), this, SLOT(sendExpose()));

    surface->setBufferAttacher(new BufferAttacher);

    QtWaylandMotorcarSurface *motorsurface = new QtWaylandMotorcarSurface(surface, this, motorcar::WaylandSurface::SurfaceType::NA);
    m_surfaceMap.insert(std::pair<QWaylandSurface *, QtWaylandMotorcarSurface *>(surface, motorsurface));
}

void QtWaylandMotorcarCompositor::sendExpose()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    surface->sendOnScreenVisibilityChange(true);
}

QPointF QtWaylandMotorcarCompositor::toSurface(QWaylandSurface *surface, const QPointF &point) const
{
    return QPointF();
}

motorcar::Seat *QtWaylandMotorcarCompositor::defaultSeat() const
{
    return m_defaultSeat;
}

void QtWaylandMotorcarCompositor::setDefaultSeat(QtWaylandMotorcarSeat *defaultSeat)
{
    m_defaultSeat = defaultSeat;
}

void QtWaylandMotorcarCompositor::updateCursor()
{
    if (!m_cursorSurface)
        return;
    QImage image = (static_cast<BufferAttacher *>(m_cursorSurface->bufferAttacher()))->image();
    QCursor cursor(QPixmap::fromImage(image), m_cursorHotspotX, m_cursorHotspotY);
    static bool cursorIsSet = false;
    if (cursorIsSet) {
        QGuiApplication::changeOverrideCursor(cursor);
    } else {
        QGuiApplication::setOverrideCursor(cursor);
        cursorIsSet = true;
    }
}

void QtWaylandMotorcarCompositor::setCursorSurface(QWaylandSurface *surface, int hotspotX, int hotspotY)
{
    return; // TODO: Disables cirtual mouse cursor image to avoid a crash. This is temporary.

    if(m_defaultSeat->pointer()->cursorNode() == NULL){
        QtWaylandMotorcarSurface *cursorMotorcarSurface =new QtWaylandMotorcarSurface(surface, this, motorcar::WaylandSurface::SurfaceType::CURSOR);
        motorcar::WaylandSurfaceNode *cursorSurfaceNode = this->scene()->windowManager()->createSurface(cursorMotorcarSurface);
        m_surfaceMap.insert(std::pair<QWaylandSurface *, QtWaylandMotorcarSurface *>(surface, cursorMotorcarSurface));
        m_defaultSeat->pointer()->setCursorNode(cursorSurfaceNode);
        std::cout << "created cursor surface node " << cursorSurfaceNode << std::endl;
    }
    if(!surface){
        std::cout << "cursor surface set to NULL" <<std::endl;
        delete m_defaultSeat->pointer()->cursorNode();
        m_defaultSeat->pointer()->setCursorNode(NULL);
    }else{
        (static_cast<QtWaylandMotorcarSurface *>(m_defaultSeat->pointer()->cursorNode()->surface()))->setSurface(surface);
        m_defaultSeat->pointer()->setCursorHotspot(glm::ivec2(hotspotX, hotspotY));
    }

    if ((m_cursorSurface != surface) && surface){
        connect(surface, SIGNAL(redraw()), this, SLOT(updateCursor()));
    }

    m_cursorSurface = surface;
    m_cursorHotspotX = hotspotX;
    m_cursorHotspotY = hotspotY;

    if (m_cursorSurface && !m_cursorSurface->bufferAttacher()) {
        m_cursorSurface->setBufferAttacher(new BufferAttacher);
    }
}

QWaylandSurface *QtWaylandMotorcarCompositor::surfaceAt(const QPointF &point, QPointF *local)
{
    return NULL;
}

void QtWaylandMotorcarCompositor::render()
{
    m_glData->m_window->makeCurrent();
    frameStarted();
    cleanupGraphicsResources();
    scene()->prepareForFrame(QDateTime::currentMSecsSinceEpoch());
    sendFrameCallbacks(surfaces());
    moveCamera();
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

    m_renderScheduler.start(16);
}

void QtWaylandMotorcarCompositor::moveCamera() {
    // Camera debug mode
    if(m_camIsMoving) {
        glm::vec4 camPos;
        camPos *= 0;
        camPos.w = 1;
        glm::vec4 delta = camPos;
        delta.x = m_camMoveVec.x;
        delta.y = m_camMoveVec.y;
        delta.z = m_camMoveVec.z;

        const float speed = 0.01;
        delta *= speed;
        delta.w /= speed;
        glm::mat4 trans = display()->transform();
        //camPos = trans * camPos;
        //delta = trans * delta;
        glm::vec3 move = glm::vec3(delta.x/delta.w - camPos.x/camPos.w, delta.y/delta.w - camPos.y/camPos.w,
                                   delta.z/delta.w - camPos.z/camPos.w);
        trans = glm::translate(trans, move);
        display()->setTransform(trans);
    }
}

bool QtWaylandMotorcarCompositor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_glData->m_window)
        return false;

    QWaylandInputDevice *input = defaultInputDevice();

    //  For now just handle the typical keyboard events as input.
    //  How can we integrate the following that are comming up?
    //  case QEvent::MouseButtonPress: {
    //  case QEvent::MouseButtonRelease: {
    //  case QEvent::MouseMove: {
    //  case QEvent::Wheel: {
    //  case QEvent::TouchBegin:
    //  case QEvent::TouchUpdate:
    //  case QEvent::TouchEnd:

    switch (event->type()) {
        case QEvent::Expose:
            m_renderScheduler.start(0);
            if (m_glData->m_window->isExposed()) {
                Qt::KeyboardModifiers mods = QGuiApplication::queryKeyboardModifiers();
                if (m_modifiers != mods && input->keyboardFocus()) {
                    Qt::KeyboardModifiers stuckMods = m_modifiers ^ mods;
                    if (stuckMods & Qt::AltModifier)
                        input->sendKeyReleaseEvent(64); // native scancode for left alt
                    m_modifiers = mods;
                }
            }
        break;
        case QEvent::KeyPress: {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if(ke->key() == Qt::Key::Key_F12) {
                m_camIsMoving = !m_camIsMoving;
                std::cout << "Camera debug mode set to " << m_camIsMoving << "\n";
            }
            if(m_camIsMoving) {
                if(ke->key() == Qt::Key::Key_A)
                    m_camMoveVec.x -= 1;
                else if(ke->key() == Qt::Key::Key_D)
                    m_camMoveVec.x += 1;
                else if(ke->key() == Qt::Key::Key_W)
                    m_camMoveVec.z -= 1;
                else if(ke->key() == Qt::Key::Key_S)
                    m_camMoveVec.z += 1;
                return false;
            }
            else {
                m_camMoveVec *= 0;
            }
            this->scene()->windowManager()->sendEvent(motorcar::KeyboardEvent(motorcar::KeyboardEvent::Event::KEY_PRESS, ke->nativeScanCode(), defaultSeat()));
        }
        break;
        case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if(m_camIsMoving) {
                if(ke->key() == Qt::Key::Key_A)
                    m_camMoveVec.x += 1;
                else if(ke->key() == Qt::Key::Key_D)
                    m_camMoveVec.x -= 1;
                else if(ke->key() == Qt::Key::Key_W)
                    m_camMoveVec.z += 1;
                else if(ke->key() == Qt::Key::Key_S)
                    m_camMoveVec.z -= 1;
            }
            this->scene()->windowManager()->sendEvent(motorcar::KeyboardEvent(motorcar::KeyboardEvent::Event::KEY_RELEASE, ke->nativeScanCode(), defaultSeat()));
        }
        break;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove: {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            glm::vec2 pos(me->x(), me->y());

            motorcar::MouseEvent::Event mouseEvent;
            motorcar::MouseEvent::Button button;


            if(me->button() == Qt::LeftButton)
                button = motorcar::MouseEvent::Button::LEFT;
            else if(me->button() == Qt::RightButton)
                button = motorcar::MouseEvent::Button::RIGHT;
            else if(me->button() == Qt::MiddleButton)
                button = motorcar::MouseEvent::Button::MIDDLE;

            if(me->type() == QEvent::MouseButtonPress)
                mouseEvent = motorcar::MouseEvent::Event::BUTTON_PRESS;
            else if(me->type() == QEvent::MouseButtonRelease)
                mouseEvent = motorcar::MouseEvent::Event::BUTTON_RELEASE;
            else if(me->type() == QEvent::MouseMove)
                mouseEvent = motorcar::MouseEvent::Event::MOVE;

            // Camera debug mode
            if(m_camIsMoving && mouseEvent == motorcar::MouseEvent::Event::MOVE) {
                glm::vec2 delta = pos - m_oldMousePos;
                const float speed = -.001;

                glm::mat4 camTrans = display()->transform();

                //glm::vec4 up4 = glm::inverse(camTrans) * glm::vec4(0,0,1,1);
                //glm::vec3 up = glm::vec3(up4.x, up4.y, up4.z) / up4.w;

                camTrans = glm::rotate(camTrans, (glm::mediump_float)speed * delta.x, glm::vec3(0,1,0));
                camTrans = glm::rotate(camTrans, (glm::mediump_float)speed * delta.y, glm::vec3(1,0,0));

                display()->setTransform(camTrans);

                QCursor::setPos(m_oldMousePos.x, m_oldMousePos.y);
                pos = m_oldMousePos;

                return false;
            }
            m_oldMousePos = pos;

            motorcar::Geometry::Ray ray = display()->worldRayAtDisplayPosition(pos);
            ray.d *= -1; // Ray seems to face the wrong way without this
            motorcar::Geometry::RaySurfaceIntersection *inter = this->scene()->intersectWithSurfaces(ray);

            if(inter != NULL && inter->surfaceNode != NULL) {
                if(inter->surfaceNode->surface() != defaultSeat()->pointerFocus()) {
                    // TODO: Do we need to generate LEAVE and ENTER events?
                    defaultSeat()->setPointerFocus(inter->surfaceNode->surface(), inter->surfaceLocalCoordinates);
                }

                pos = inter->surfaceLocalCoordinates;
            }
            else
                return false; // Ignore clicks outside windows

            this->scene()->windowManager()->sendEvent(motorcar::MouseEvent(mouseEvent, button, pos, defaultSeat()));
        break;
        }
        case QEvent::Wheel: {
            QWheelEvent *we = static_cast<QWheelEvent *>(event);
            glm::vec2 pos(we->x(), we->y());

            motorcar::Geometry::Ray ray = display()->worldRayAtDisplayPosition(pos);
            ray.d *= -1;
            motorcar::Geometry::RaySurfaceIntersection *inter = this->scene()->intersectWithSurfaces(ray);

            if(inter == NULL || inter->surfaceNode == NULL || inter->surfaceNode->surface() == NULL)
                return false; // Ignore events outside all windows

            pos = inter->surfaceLocalCoordinates;
            float delta = we->delta();

            motorcar::WheelEvent::Orientation ori;
            switch (we->orientation()) {
            case Qt::Orientation::Horizontal:
                ori = motorcar::WheelEvent::Orientation::HORIZONTAL;
                break;
            case Qt::Orientation::Vertical:
                ori = motorcar::WheelEvent::Orientation::VERTICAL;
                break;
            default:
                break;
            }

            // Sends wheel event to surface under pointer. Ignores current pointer focus. Doesn't change focus.
            inter->surfaceNode->surface()->sendEvent(motorcar::WheelEvent(ori, delta, pos, defaultSeat()));
        }

        default:
        break;
    }
    return false;
}
