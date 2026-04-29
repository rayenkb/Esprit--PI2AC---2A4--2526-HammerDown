#include "modelingwidget.h"
#include <algorithm>
#include <QtMath>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QToolBar>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QPainter>
#include <QLineEdit>
#include <QShortcut>
#include <QRandomGenerator>
#include <cfloat>
#include <functional>
#include <QSet>
#include <QMap>
#include <QSignalBlocker>
#include <QQuaternion>
#include <cmath>

// ════════════════════════════════════════════════════════════════════════════
//  GLViewport – OpenGL 3D viewport
// ════════════════════════════════════════════════════════════════════════════

GLViewport::GLViewport(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_camAnimTimer.setInterval(16);
    connect(&m_camAnimTimer, &QTimer::timeout, this, &GLViewport::updateCameraAnimation);

    m_inertiaTimer.setInterval(16);
    connect(&m_inertiaTimer, &QTimer::timeout, this, &GLViewport::updateInertia);

    m_walkTimer.setInterval(16);
    connect(&m_walkTimer, &QTimer::timeout, this, &GLViewport::updateWalkNavigation);

    m_fpsTimer.start();
}

void GLViewport::resetCamera()
{
    setWalkMode(false);
    setPresetView(ViewPreset::Perspective);
}

void GLViewport::setWalkMode(bool enabled)
{
    if (m_walkMode == enabled)
        return;

    if (enabled) {
        if (m_hasSpawnAnchor) {
            m_freeCamPos = m_spawnAnchor;
        } else {
            QMatrix4x4 invView = computeViewMatrix().inverted();
            m_freeCamPos = invView.map(QVector3D(0, 0, 0));
        }
        m_pressedKeys.clear();
        m_rightMouseLook = false;
        m_ctrlLookLock = false;
        m_mouseLook = false;
        m_cursorWarpInProgress = false;
        m_walkTimer.start();
        setCursor(Qt::CrossCursor);
        setFocus(Qt::OtherFocusReason);
    } else {
        m_walkTimer.stop();
        m_pressedKeys.clear();
        m_mouseLook = false;
        m_rightMouseLook = false;
        m_ctrlLookLock = false;
        m_cursorWarpInProgress = false;
        unsetCursor();
    }

    m_walkMode = enabled;
    emit walkModeChanged(enabled);
    update();
}

void GLViewport::setWalkStart(const QVector3D &position, float yawDeg, float pitchDeg, bool enableWalk)
{
    m_freeCamPos = position;
    m_camYaw = yawDeg;
    m_camPitch = qBound(-85.0f, pitchDeg, 85.0f);
    if (enableWalk) {
        setWalkMode(true);
    } else {
        update();
    }
}

void GLViewport::setPresetView(ViewPreset preset)
{
    if (m_walkMode)
        setWalkMode(false);

    float targetYaw = m_camYaw;
    float targetPitch = m_camPitch;
    float targetDist = qMax(6.0f, m_camDist);
    float targetProjectionBlend = 1.0f;

    switch (preset) {
    case ViewPreset::Top:
        targetYaw = 0.0f;
        targetPitch = 89.0f;
        break;
    case ViewPreset::Bottom:
        targetYaw = 0.0f;
        targetPitch = -89.0f;
        break;
    case ViewPreset::Left:
        targetYaw = -90.0f;
        targetPitch = 0.0f;
        break;
    case ViewPreset::Right:
        targetYaw = 90.0f;
        targetPitch = 0.0f;
        break;
    case ViewPreset::Front:
        targetYaw = 0.0f;
        targetPitch = 0.0f;
        break;
    case ViewPreset::Back:
        targetYaw = 180.0f;
        targetPitch = 0.0f;
        break;
    case ViewPreset::Perspective:
        targetYaw = 30.0f;
        targetPitch = 25.0f;
        targetProjectionBlend = 0.0f;
        break;
    }

    startCameraAnimation(targetYaw, targetPitch, targetDist, QVector3D(0, 0, 0), targetProjectionBlend);
}

void GLViewport::startCameraAnimation(float targetYaw,
                                      float targetPitch,
                                      float targetDist,
                                      const QVector3D &targetCamTarget,
                                      float targetProjectionBlend)
{
    m_animStartYaw = m_camYaw;
    m_animStartPitch = m_camPitch;
    m_animStartDist = m_camDist;
    m_animStartTarget = m_camTarget;
    m_animStartProjectionBlend = m_projectionBlend;

    m_animEndYaw = targetYaw;
    m_animEndPitch = qBound(-89.0f, targetPitch, 89.0f);
    m_animEndDist = qBound(1.0f, targetDist, 100.0f);
    m_animEndTarget = targetCamTarget;
    m_animEndProjectionBlend = qBound(0.0f, targetProjectionBlend, 1.0f);

    m_camAnimElapsed.restart();
    m_camAnimTimer.start();
    update();
}

void GLViewport::updateCameraAnimation()
{
    const qreal rawT = qreal(m_camAnimElapsed.elapsed()) / qreal(m_camAnimDurationMs);
    const qreal t = qBound<qreal>(0.0, rawT, 1.0);
    const qreal eased = t * t * (3.0 - 2.0 * t); // smoothstep

    m_camYaw = m_animStartYaw + float((m_animEndYaw - m_animStartYaw) * eased);
    m_camPitch = m_animStartPitch + float((m_animEndPitch - m_animStartPitch) * eased);
    m_camDist = m_animStartDist + float((m_animEndDist - m_animStartDist) * eased);
    m_camTarget = m_animStartTarget + (m_animEndTarget - m_animStartTarget) * float(eased);
    m_projectionBlend = m_animStartProjectionBlend + float((m_animEndProjectionBlend - m_animStartProjectionBlend) * eased);

    if (t >= 1.0) {
        m_camAnimTimer.stop();
        m_camYaw = m_animEndYaw;
        m_camPitch = m_animEndPitch;
        m_camDist = m_animEndDist;
        m_camTarget = m_animEndTarget;
        m_projectionBlend = m_animEndProjectionBlend;
    }

    update();
}

void GLViewport::applyProjectionMatrix()
{
    QMatrix4x4 proj = computeProjectionMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(proj.constData());
    glMatrixMode(GL_MODELVIEW);
}

QMatrix4x4 GLViewport::computeProjectionMatrix() const
{
    const int h = height() > 0 ? height() : 1;
    const float aspect = float(width()) / float(h);

    QMatrix4x4 perspective;
    perspective.perspective(45.0f, aspect, 0.1f, 200.0f);

    const float orthoHalf = qMax(1.0f, m_camDist * 0.7f);
    QMatrix4x4 ortho;
    ortho.ortho(-orthoHalf * aspect, orthoHalf * aspect, -orthoHalf, orthoHalf, -200.0f, 200.0f);

    if (m_projectionBlend <= 0.0f)
        return perspective;
    if (m_projectionBlend >= 1.0f)
        return ortho;

    QMatrix4x4 blended;
    const float *pm = perspective.constData();
    const float *om = ortho.constData();
    float out[16];
    for (int i = 0; i < 16; ++i)
        out[i] = pm[i] + (om[i] - pm[i]) * m_projectionBlend;
    blended = QMatrix4x4(out);
    return blended;
}

QMatrix4x4 GLViewport::computeViewMatrix() const
{
    if (m_walkMode) {
        const float yawRad = qDegreesToRadians(m_camYaw);
        const float pitchRad = qDegreesToRadians(m_camPitch);

        QVector3D forward(cosf(pitchRad) * sinf(yawRad),
                          sinf(pitchRad),
                          cosf(pitchRad) * cosf(yawRad));
        forward.normalize();

        QMatrix4x4 view;
        view.lookAt(m_freeCamPos, m_freeCamPos + forward, QVector3D(0, 1, 0));
        return view;
    }

    float yawRad   = qDegreesToRadians(m_camYaw);
    float pitchRad = qDegreesToRadians(m_camPitch);
    float cx = m_camTarget.x() + m_camDist * cosf(pitchRad) * sinf(yawRad);
    float cy = m_camTarget.y() + m_camDist * sinf(pitchRad);
    float cz = m_camTarget.z() + m_camDist * cosf(pitchRad) * cosf(yawRad);

    QMatrix4x4 view;
    view.lookAt(QVector3D(cx, cy, cz), m_camTarget, QVector3D(0, 1, 0));
    return view;
}

void GLViewport::buildWorldMatrices()
{
    if (!m_objects) return;
    m_worldMatrices.resize(m_objects->size());

    QVector<bool> done(m_objects->size(), false);
    std::function<void(int)> build = [&](int idx) {
        if (idx < 0 || idx >= m_objects->size()) return;
        if (done[idx]) return;
        const SceneObject &obj = m_objects->at(idx);

        QMatrix4x4 local;
        local.translate(obj.position);
        local.rotate(QQuaternion::fromEulerAngles(obj.rotation.x(), obj.rotation.y(), obj.rotation.z()));
        local.scale(obj.scale);

        if (obj.parentIndex >= 0 && obj.parentIndex < m_objects->size()) {
            build(obj.parentIndex);
            m_worldMatrices[idx] = m_worldMatrices[obj.parentIndex] * local;
        } else {
            m_worldMatrices[idx] = local;
        }

        done[idx] = true;
    };

    for (int i = 0; i < m_objects->size(); ++i)
        build(i);
}

void GLViewport::computeObjectBounds(int index, QVector3D &outMin, QVector3D &outMax) const
{
    if (!m_objects || index < 0 || index >= m_objects->size()) {
        outMin = QVector3D();
        outMax = QVector3D();
        return;
    }

    const QMatrix4x4 &world = m_worldMatrices.at(index);
    const QVector3D corners[8] = {
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f,  0.5f}
    };

    QVector3D minV(FLT_MAX, FLT_MAX, FLT_MAX);
    QVector3D maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (const QVector3D &c : corners) {
        QVector3D w = world.map(c);
        minV.setX(qMin(minV.x(), w.x()));
        minV.setY(qMin(minV.y(), w.y()));
        minV.setZ(qMin(minV.z(), w.z()));
        maxV.setX(qMax(maxV.x(), w.x()));
        maxV.setY(qMax(maxV.y(), w.y()));
        maxV.setZ(qMax(maxV.z(), w.z()));
    }
    outMin = minV;
    outMax = maxV;
}

void GLViewport::computeSelectionBounds(QVector3D &outMin, QVector3D &outMax, bool &outValid) const
{
    outValid = false;
    if (!m_objects) return;
    QVector3D minV(FLT_MAX, FLT_MAX, FLT_MAX);
    QVector3D maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    QList<int> targets = m_selectedIndices.isEmpty()
                             ? QList<int>{m_selectedIdx}
                             : m_selectedIndices;
    for (int idx : targets) {
        if (idx < 0 || idx >= m_objects->size()) continue;
        if (!m_objects->at(idx).visible) continue;
        QVector3D objMin, objMax;
        computeObjectBounds(idx, objMin, objMax);
        minV.setX(qMin(minV.x(), objMin.x()));
        minV.setY(qMin(minV.y(), objMin.y()));
        minV.setZ(qMin(minV.z(), objMin.z()));
        maxV.setX(qMax(maxV.x(), objMax.x()));
        maxV.setY(qMax(maxV.y(), objMax.y()));
        maxV.setZ(qMax(maxV.z(), objMax.z()));
        outValid = true;
    }

    outMin = minV;
    outMax = maxV;
}

void GLViewport::computeVisibleBounds(QVector3D &outMin, QVector3D &outMax, bool &outValid) const
{
    outValid = false;
    if (!m_objects) return;
    QVector3D minV(FLT_MAX, FLT_MAX, FLT_MAX);
    QVector3D maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < m_objects->size(); ++i) {
        if (!m_objects->at(i).visible) continue;
        QVector3D objMin, objMax;
        computeObjectBounds(i, objMin, objMax);
        minV.setX(qMin(minV.x(), objMin.x()));
        minV.setY(qMin(minV.y(), objMin.y()));
        minV.setZ(qMin(minV.z(), objMin.z()));
        maxV.setX(qMax(maxV.x(), objMax.x()));
        maxV.setY(qMax(maxV.y(), objMax.y()));
        maxV.setZ(qMax(maxV.z(), objMax.z()));
        outValid = true;
    }

    outMin = minV;
    outMax = maxV;
}

QVector3D GLViewport::computeWorldPivot(int index) const
{
    if (!m_objects || index < 0 || index >= m_objects->size())
        return QVector3D(0, 0, 0);

    if (index >= 0 && index < m_worldMatrices.size())
        return m_worldMatrices.at(index).map(QVector3D(0, 0, 0));

    return QVector3D(0, 0, 0);
}

void GLViewport::computeSceneBounds()
{
    QVector3D minV, maxV;
    bool valid = false;
    computeVisibleBounds(minV, maxV, valid);
    m_hasSceneBounds = valid;
    if (valid) {
        m_sceneBoundsMin = minV;
        m_sceneBoundsMax = maxV;
    }
}

void GLViewport::frameSelected()
{
    buildWorldMatrices();
    QVector3D minV, maxV;
    bool valid = false;
    computeSelectionBounds(minV, maxV, valid);
    if (!valid) return;

    QVector3D center = (minV + maxV) * 0.5f;
    float radius = (maxV - minV).length() * 0.5f;
    float fov = 45.0f;
    float targetDist = radius / qMax(0.1f, sinf(qDegreesToRadians(fov * 0.5f)));
    targetDist = qBound(1.0f, targetDist, 100.0f);

    startCameraAnimation(m_camYaw, m_camPitch, targetDist, center, m_projectionBlend);
}

void GLViewport::frameAll()
{
    buildWorldMatrices();
    QVector3D minV, maxV;
    bool valid = false;
    computeVisibleBounds(minV, maxV, valid);
    if (!valid) return;

    QVector3D center = (minV + maxV) * 0.5f;
    float radius = (maxV - minV).length() * 0.5f;
    float fov = 45.0f;
    float targetDist = radius / qMax(0.1f, sinf(qDegreesToRadians(fov * 0.5f)));
    targetDist = qBound(1.0f, targetDist, 100.0f);

    startCameraAnimation(m_camYaw, m_camPitch, targetDist, center, m_projectionBlend);
}

void GLViewport::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat ambient[]  = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat diffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat specular[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat lightPos[] = {5.0f, 10.0f, 7.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
}

void GLViewport::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    applyProjectionMatrix();
}

void GLViewport::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    applyProjectionMatrix();
    glLoadIdentity();

    QMatrix4x4 view = computeViewMatrix();
    glLoadMatrixf(view.constData());

    buildWorldMatrices();
    computeSceneBounds();

    // Reposition light
    GLfloat lightPos[] = {5.0f, 10.0f, 7.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    if (m_showGrid) drawGrid();

    if (m_objects) {
        for (int i = 0; i < m_objects->size(); ++i) {
            const SceneObject &obj = m_objects->at(i);
            if (!obj.visible) continue;
            drawObject(i, obj, m_selectedIndices.contains(i) || i == m_selectedIdx);
        }
    }

    if (m_showBounds)
        drawSceneBounds();

    drawGizmo();

    drawOrientationCube();

    drawStatsOverlay();
}

void GLViewport::drawGrid()
{
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);

    const float step = computeGridStep();
    const int halfLines = 20;
    const float extent = step * halfLines;

    for (int i = -halfLines; i <= halfLines; ++i) {
        if (i == 0) glColor4f(0.5f, 0.5f, 0.5f, 0.6f);
        else        glColor4f(0.3f, 0.3f, 0.3f, 0.4f);

        float v = float(i) * step;
        glBegin(GL_LINES);
        glVertex3f(v, 0, -extent);
        glVertex3f(v, 0, extent);
        glVertex3f(-extent, 0, v);
        glVertex3f(extent, 0, v);
        glEnd();
    }

    // Axes
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // X red
    glColor3f(0.9f, 0.2f, 0.2f);
    glVertex3f(0, 0.01f, 0); glVertex3f(3, 0.01f, 0);
    // Y green
    glColor3f(0.2f, 0.9f, 0.2f);
    glVertex3f(0, 0, 0); glVertex3f(0, 3, 0);
    // Z blue
    glColor3f(0.2f, 0.4f, 0.9f);
    glVertex3f(0, 0.01f, 0); glVertex3f(0, 0.01f, 3);
    glEnd();

    glEnable(GL_LIGHTING);
}

void GLViewport::drawObject(int index, const SceneObject &obj, bool highlight)
{
    glPushMatrix();
    if (index >= 0 && index < m_worldMatrices.size())
        glMultMatrixf(m_worldMatrices[index].constData());

    if (highlight) {
        // Draw selection outline
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(3.0f);
        glColor3f(1.0f, 0.8f, 0.0f); // gold
        switch (obj.type) {
            case PrimitiveType::Cube:     drawCube(); break;
            case PrimitiveType::Sphere:   drawSphere(24, 16); break;
            case PrimitiveType::Cylinder: drawCylinder(24); break;
            case PrimitiveType::Cone:     drawCone(24); break;
            case PrimitiveType::Pyramid:  drawPyramid(); break;
            case PrimitiveType::Plane:    drawPlane(); break;
            case PrimitiveType::Capsule:  drawCapsule(24, 12); break;
            case PrimitiveType::Torus:    drawTorus(28, 16); break;
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glLineWidth(1.0f);
        if (m_shadingMode != ShadingMode::Unlit)
            glEnable(GL_LIGHTING);
    }

    glColor3f(obj.color.redF(), obj.color.greenF(), obj.color.blueF());

    if (m_shadingMode == ShadingMode::Unlit)
        glDisable(GL_LIGHTING);
    else
        glEnable(GL_LIGHTING);

    if (m_shadingMode == ShadingMode::Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);
    }

    switch (obj.type) {
        case PrimitiveType::Cube:     drawCube(); break;
        case PrimitiveType::Sphere:   drawSphere(24, 16); break;
        case PrimitiveType::Cylinder: drawCylinder(24); break;
        case PrimitiveType::Cone:     drawCone(24); break;
        case PrimitiveType::Pyramid:  drawPyramid(); break;
        case PrimitiveType::Plane:    drawPlane(); break;
        case PrimitiveType::Capsule:  drawCapsule(24, 12); break;
        case PrimitiveType::Torus:    drawTorus(28, 16); break;
    }

    if (m_shadingMode == ShadingMode::Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
    }

    if (m_shadingMode == ShadingMode::SolidWire) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.5f);
        glColor3f(0.1f, 0.1f, 0.1f);
        switch (obj.type) {
            case PrimitiveType::Cube:     drawCube(); break;
            case PrimitiveType::Sphere:   drawSphere(24, 16); break;
            case PrimitiveType::Cylinder: drawCylinder(24); break;
            case PrimitiveType::Cone:     drawCone(24); break;
            case PrimitiveType::Pyramid:  drawPyramid(); break;
            case PrimitiveType::Plane:    drawPlane(); break;
            case PrimitiveType::Capsule:  drawCapsule(24, 12); break;
            case PrimitiveType::Torus:    drawTorus(28, 16); break;
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    glPopMatrix();
}

// ─── Primitive geometry ────────────────────────────────────────────

void GLViewport::drawCube()
{
    static const GLfloat V[8][3] = {
        {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
        {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}
    };
    static const int F[6][4] = {
        {0,1,2,3},{5,4,7,6},{1,5,6,2},{4,0,3,7},{3,2,6,7},{4,5,1,0}
    };
    static const GLfloat N[6][3] = {
        {0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
    };

    glBegin(GL_QUADS);
    for (int f = 0; f < 6; ++f) {
        glNormal3fv(N[f]);
        for (int v = 0; v < 4; ++v)
            glVertex3fv(V[F[f][v]]);
    }
    glEnd();
}

void GLViewport::drawSphere(int slices, int stacks)
{
    for (int i = 0; i < stacks; ++i) {
        float lat0 = M_PI * (-0.5f + float(i) / stacks);
        float lat1 = M_PI * (-0.5f + float(i + 1) / stacks);
        float y0 = sinf(lat0), yr0 = cosf(lat0);
        float y1 = sinf(lat1), yr1 = cosf(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lng = 2.0f * M_PI * float(j) / slices;
            float x = cosf(lng), z = sinf(lng);
            glNormal3f(x * yr0, y0, z * yr0);
            glVertex3f(0.5f * x * yr0, 0.5f * y0, 0.5f * z * yr0);
            glNormal3f(x * yr1, y1, z * yr1);
            glVertex3f(0.5f * x * yr1, 0.5f * y1, 0.5f * z * yr1);
        }
        glEnd();
    }
}

void GLViewport::drawCylinder(int slices)
{
    float r = 0.5f, h = 1.0f;
    // Side
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float a = 2.0f * M_PI * i / slices;
        float x = cosf(a), z = sinf(a);
        glNormal3f(x, 0, z);
        glVertex3f(r * x, h / 2, r * z);
        glVertex3f(r * x, -h / 2, r * z);
    }
    glEnd();

    // Caps
    for (int cap = 0; cap < 2; ++cap) {
        float y = cap ? h / 2 : -h / 2;
        float ny = cap ? 1.0f : -1.0f;
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, ny, 0);
        glVertex3f(0, y, 0);
        for (int i = 0; i <= slices; ++i) {
            float a = (cap ? 1 : -1) * 2.0f * M_PI * i / slices;
            glVertex3f(r * cosf(a), y, r * sinf(a));
        }
        glEnd();
    }
}

void GLViewport::drawCone(int slices)
{
    float r = 0.5f, h = 1.0f;
    // Side
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, h / 2, 0);
    for (int i = 0; i <= slices; ++i) {
        float a = 2.0f * M_PI * i / slices;
        float x = cosf(a), z = sinf(a);
        glNormal3f(x, 0.5f, z);
        glVertex3f(r * x, -h / 2, r * z);
    }
    glEnd();

    // Base
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, -h / 2, 0);
    for (int i = slices; i >= 0; --i) {
        float a = 2.0f * M_PI * i / slices;
        glVertex3f(r * cosf(a), -h / 2, r * sinf(a));
    }
    glEnd();
}

void GLViewport::drawPyramid()
{
    float s = 0.5f, h = 1.0f;
    // Base
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-s, 0, -s);
    glVertex3f( s, 0, -s);
    glVertex3f( s, 0,  s);
    glVertex3f(-s, 0,  s);
    glEnd();

    // 4 faces
    QVector3D apex(0, h, 0);
    QVector3D corners[4] = {{-s,0,-s},{s,0,-s},{s,0,s},{-s,0,s}};
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 4; ++i) {
        QVector3D a = corners[i], b = corners[(i+1)%4];
        QVector3D n = QVector3D::crossProduct(b - a, apex - a).normalized();
        glNormal3f(n.x(), n.y(), n.z());
        glVertex3f(a.x(), a.y(), a.z());
        glVertex3f(b.x(), b.y(), b.z());
        glVertex3f(apex.x(), apex.y(), apex.z());
    }
    glEnd();
}

void GLViewport::drawPlane()
{
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0, -0.5f);
    glVertex3f( 0.5f, 0, -0.5f);
    glVertex3f( 0.5f, 0,  0.5f);
    glVertex3f(-0.5f, 0,  0.5f);
    glEnd();
}

void GLViewport::drawCapsule(int slices, int stacks)
{
    const float radius = 0.25f;
    const float cylinderHalf = 0.25f;

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float a = 2.0f * float(M_PI) * float(i) / float(slices);
        float x = std::cos(a);
        float z = std::sin(a);
        glNormal3f(x, 0.0f, z);
        glVertex3f(radius * x, cylinderHalf, radius * z);
        glVertex3f(radius * x, -cylinderHalf, radius * z);
    }
    glEnd();

    for (int stack = 0; stack < stacks; ++stack) {
        float t0 = (float(stack) / float(stacks)) * (float(M_PI) * 0.5f);
        float t1 = (float(stack + 1) / float(stacks)) * (float(M_PI) * 0.5f);

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float a = 2.0f * float(M_PI) * float(i) / float(slices);
            float ca = std::cos(a), sa = std::sin(a);

            float ct0 = std::cos(t0), st0 = std::sin(t0);
            float ct1 = std::cos(t1), st1 = std::sin(t1);

            glNormal3f(ca * ct0, st0, sa * ct0);
            glVertex3f(radius * ca * ct0, cylinderHalf + radius * st0, radius * sa * ct0);

            glNormal3f(ca * ct1, st1, sa * ct1);
            glVertex3f(radius * ca * ct1, cylinderHalf + radius * st1, radius * sa * ct1);
        }
        glEnd();

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float a = 2.0f * float(M_PI) * float(i) / float(slices);
            float ca = std::cos(a), sa = std::sin(a);

            float ct0 = std::cos(t0), st0 = std::sin(t0);
            float ct1 = std::cos(t1), st1 = std::sin(t1);

            glNormal3f(ca * ct0, -st0, sa * ct0);
            glVertex3f(radius * ca * ct0, -cylinderHalf - radius * st0, radius * sa * ct0);

            glNormal3f(ca * ct1, -st1, sa * ct1);
            glVertex3f(radius * ca * ct1, -cylinderHalf - radius * st1, radius * sa * ct1);
        }
        glEnd();
    }
}

void GLViewport::drawTorus(int majorSegments, int minorSegments)
{
    const float majorRadius = 0.33f;
    const float minorRadius = 0.14f;

    for (int i = 0; i < majorSegments; ++i) {
        float u0 = 2.0f * float(M_PI) * float(i) / float(majorSegments);
        float u1 = 2.0f * float(M_PI) * float(i + 1) / float(majorSegments);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= minorSegments; ++j) {
            float v = 2.0f * float(M_PI) * float(j) / float(minorSegments);
            float cv = std::cos(v), sv = std::sin(v);

            float cu0 = std::cos(u0), su0 = std::sin(u0);
            float cu1 = std::cos(u1), su1 = std::sin(u1);

            float r0 = majorRadius + minorRadius * cv;
            float r1 = majorRadius + minorRadius * cv;

            QVector3D n0(cu0 * cv, sv, su0 * cv);
            QVector3D n1(cu1 * cv, sv, su1 * cv);

            glNormal3f(n0.x(), n0.y(), n0.z());
            glVertex3f(r0 * cu0, minorRadius * sv, r0 * su0);

            glNormal3f(n1.x(), n1.y(), n1.z());
            glVertex3f(r1 * cu1, minorRadius * sv, r1 * su1);
        }
        glEnd();
    }
}

float GLViewport::computeGridStep() const
{
    const float target = qMax(0.1f, m_camDist / 8.0f);
    const float steps[] = {0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f};
    float best = steps[0];
    float bestDelta = fabsf(target - best);
    for (float step : steps) {
        float delta = fabsf(target - step);
        if (delta < bestDelta) {
            bestDelta = delta;
            best = step;
        }
    }
    return best;
}

void GLViewport::drawSceneBounds()
{
    if (!m_hasSceneBounds) return;

    const QVector3D &minV = m_sceneBoundsMin;
    const QVector3D &maxV = m_sceneBoundsMax;

    glDisable(GL_LIGHTING);
    glLineWidth(1.5f);
    glColor4f(0.8f, 0.8f, 0.2f, 0.8f);

    glBegin(GL_LINES);
    // Bottom rectangle
    glVertex3f(minV.x(), minV.y(), minV.z()); glVertex3f(maxV.x(), minV.y(), minV.z());
    glVertex3f(maxV.x(), minV.y(), minV.z()); glVertex3f(maxV.x(), minV.y(), maxV.z());
    glVertex3f(maxV.x(), minV.y(), maxV.z()); glVertex3f(minV.x(), minV.y(), maxV.z());
    glVertex3f(minV.x(), minV.y(), maxV.z()); glVertex3f(minV.x(), minV.y(), minV.z());
    // Top rectangle
    glVertex3f(minV.x(), maxV.y(), minV.z()); glVertex3f(maxV.x(), maxV.y(), minV.z());
    glVertex3f(maxV.x(), maxV.y(), minV.z()); glVertex3f(maxV.x(), maxV.y(), maxV.z());
    glVertex3f(maxV.x(), maxV.y(), maxV.z()); glVertex3f(minV.x(), maxV.y(), maxV.z());
    glVertex3f(minV.x(), maxV.y(), maxV.z()); glVertex3f(minV.x(), maxV.y(), minV.z());
    // Vertical lines
    glVertex3f(minV.x(), minV.y(), minV.z()); glVertex3f(minV.x(), maxV.y(), minV.z());
    glVertex3f(maxV.x(), minV.y(), minV.z()); glVertex3f(maxV.x(), maxV.y(), minV.z());
    glVertex3f(maxV.x(), minV.y(), maxV.z()); glVertex3f(maxV.x(), maxV.y(), maxV.z());
    glVertex3f(minV.x(), minV.y(), maxV.z()); glVertex3f(minV.x(), maxV.y(), maxV.z());
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void GLViewport::drawGizmo()
{
    if (!m_objects || m_selectedIdx < 0 || m_selectedIdx >= m_objects->size()) return;
    if (m_tool == Select) return;

    GizmoMode mode = GizmoMode::None;
    if (m_tool == Move) mode = GizmoMode::Move;
    else if (m_tool == Rotate) mode = GizmoMode::Rotate;
    else if (m_tool == Scale) mode = GizmoMode::Scale;
    m_gizmoMode = mode;

    QVector3D pivot = computeWorldPivot(m_selectedIdx);
    float size = qBound(0.8f, m_camDist * 0.08f, 4.0f);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    glColor3f(1.0f, 1.0f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(pivot.x() - size * 0.1f, pivot.y(), pivot.z());
    glVertex3f(pivot.x() + size * 0.1f, pivot.y(), pivot.z());
    glVertex3f(pivot.x(), pivot.y() - size * 0.1f, pivot.z());
    glVertex3f(pivot.x(), pivot.y() + size * 0.1f, pivot.z());
    glVertex3f(pivot.x(), pivot.y(), pivot.z() - size * 0.1f);
    glVertex3f(pivot.x(), pivot.y(), pivot.z() + size * 0.1f);
    glEnd();

    if (mode == GizmoMode::Rotate) {
        const int segments = 48;
        for (int axis = 0; axis < 3; ++axis) {
            if (axis == 0) glColor3f(0.9f, 0.2f, 0.2f);
            if (axis == 1) glColor3f(0.2f, 0.9f, 0.2f);
            if (axis == 2) glColor3f(0.2f, 0.4f, 0.9f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < segments; ++i) {
                float a = float(i) / float(segments) * 2.0f * float(M_PI);
                float x = cosf(a) * size;
                float y = sinf(a) * size;
                QVector3D p;
                if (axis == 0) p = QVector3D(0, x, y);
                if (axis == 1) p = QVector3D(x, 0, y);
                if (axis == 2) p = QVector3D(x, y, 0);
                glVertex3f(pivot.x() + p.x(), pivot.y() + p.y(), pivot.z() + p.z());
            }
            glEnd();
        }
    } else {
        auto setAxisColor = [this](GizmoAxis axis, float r, float g, float b) {
            if (m_gizmoDragging && m_activeGizmoAxis == axis)
                glColor3f(1.0f, 0.9f, 0.2f);
            else
                glColor3f(r, g, b);
        };

        // Axis lines
        glBegin(GL_LINES);
        setAxisColor(GizmoAxis::AxisX, 0.9f, 0.2f, 0.2f);
        glVertex3f(pivot.x(), pivot.y(), pivot.z());
        glVertex3f(pivot.x() + size, pivot.y(), pivot.z());
        setAxisColor(GizmoAxis::AxisY, 0.2f, 0.9f, 0.2f);
        glVertex3f(pivot.x(), pivot.y(), pivot.z());
        glVertex3f(pivot.x(), pivot.y() + size, pivot.z());
        setAxisColor(GizmoAxis::AxisZ, 0.2f, 0.4f, 0.9f);
        glVertex3f(pivot.x(), pivot.y(), pivot.z());
        glVertex3f(pivot.x(), pivot.y(), pivot.z() + size);
        glEnd();

        if (mode == GizmoMode::Move) {
            glPushMatrix();
            glTranslatef(pivot.x() + size, pivot.y(), pivot.z());
            glRotatef(90.0f, 0, 0, 1);
            glScalef(size * 0.15f, size * 0.15f, size * 0.15f);
            glColor3f(0.9f, 0.2f, 0.2f);
            drawCone(12);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(pivot.x(), pivot.y() + size, pivot.z());
            glRotatef(-90.0f, 1, 0, 0);
            glScalef(size * 0.15f, size * 0.15f, size * 0.15f);
            glColor3f(0.2f, 0.9f, 0.2f);
            drawCone(12);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(pivot.x(), pivot.y(), pivot.z() + size);
            glRotatef(90.0f, 1, 0, 0);
            glScalef(size * 0.15f, size * 0.15f, size * 0.15f);
            glColor3f(0.2f, 0.4f, 0.9f);
            drawCone(12);
            glPopMatrix();
        }

        if (mode == GizmoMode::Scale) {
            float h = size * 0.08f;
            glBegin(GL_QUADS);
            setAxisColor(GizmoAxis::AxisX, 0.9f, 0.2f, 0.2f);
            glVertex3f(pivot.x() + size - h, pivot.y() - h, pivot.z() - h);
            glVertex3f(pivot.x() + size + h, pivot.y() - h, pivot.z() - h);
            glVertex3f(pivot.x() + size + h, pivot.y() + h, pivot.z() + h);
            glVertex3f(pivot.x() + size - h, pivot.y() + h, pivot.z() + h);

            setAxisColor(GizmoAxis::AxisY, 0.2f, 0.9f, 0.2f);
            glVertex3f(pivot.x() - h, pivot.y() + size - h, pivot.z() - h);
            glVertex3f(pivot.x() + h, pivot.y() + size - h, pivot.z() - h);
            glVertex3f(pivot.x() + h, pivot.y() + size + h, pivot.z() + h);
            glVertex3f(pivot.x() - h, pivot.y() + size + h, pivot.z() + h);

            setAxisColor(GizmoAxis::AxisZ, 0.2f, 0.4f, 0.9f);
            glVertex3f(pivot.x() - h, pivot.y() - h, pivot.z() + size - h);
            glVertex3f(pivot.x() + h, pivot.y() - h, pivot.z() + size - h);
            glVertex3f(pivot.x() + h, pivot.y() + h, pivot.z() + size + h);
            glVertex3f(pivot.x() - h, pivot.y() + h, pivot.z() + size + h);
            glEnd();
        }
    }

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void GLViewport::drawStatsOverlay()
{
    ++m_frameCount;
    const qint64 elapsed = m_fpsTimer.elapsed();
    if (elapsed > 500) {
        m_fps = (m_frameCount * 1000.0f) / float(elapsed);
        m_frameCount = 0;
        m_fpsTimer.restart();
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QColor(230, 230, 230));
    p.setFont(QFont("Segoe UI", 9));

    QVector3D camPos = computeViewMatrix().inverted().map(QVector3D(0, 0, 0));
    QStringList lines;
    lines << QString("Objects: %1").arg(m_objects ? m_objects->size() : 0)
          << QString("Cam Pos: %1 %2 %3")
                 .arg(camPos.x(), 0, 'f', 2)
                 .arg(camPos.y(), 0, 'f', 2)
                 .arg(camPos.z(), 0, 'f', 2)
          << QString("Yaw/Pitch: %1 / %2").arg(m_camYaw, 0, 'f', 1).arg(m_camPitch, 0, 'f', 1)
            << QString("Mode: %1").arg(m_walkMode ? "Walk" : "Orbit")
             << QString("Look Lock: %1").arg(m_ctrlLookLock ? "On" : "Off")
            << QString("Speed: %1").arg(m_walkSpeed, 0, 'f', 1)
            << QString("FPS: %1").arg(m_fps, 0, 'f', 1);

    int y = 48;
    for (const QString &line : lines) {
        p.drawText(10, y, line);
        y += 14;
    }
}

void GLViewport::drawOrientationCube()
{
    const int size = 80;
    const int pad = 10;
    const int sideLabelGap = 4;
    const int sideLabelWidth = 34;
    const int shiftRight = 12;
    const int shiftDown = 28;
    int x = width() - size - pad - sideLabelGap - sideLabelWidth + shiftRight;
    int y = pad + shiftDown;
    QRect rect(x, y, size, size);

    // Use framebuffer-pixel viewport coordinates so placement is stable on HiDPI displays.
    const qreal dpr = devicePixelRatioF();
    const int glX = qRound(x * dpr);
    const int glY = qRound((height() - y - size) * dpr);
    const int glSize = qMax(1, qRound(size * dpr));
    glViewport(glX, glY, glSize, glSize);
    glClear(GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 proj;
    proj.ortho(-1.2f, 1.2f, -1.2f, 1.2f, -5.0f, 5.0f);
    QMatrix4x4 view;
    view.lookAt(QVector3D(0, 0, 3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

    QMatrix4x4 camRot;
    camRot.rotate(m_camYaw, 0, 1, 0);
    camRot.rotate(m_camPitch, 1, 0, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(proj.constData());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf((view * camRot).constData());

    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3f(0.6f, 0.6f, 0.6f);
    // Front
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    // Back
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    // Left
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    // Right
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    // Top
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    // Bottom
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();

    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    applyProjectionMatrix();
    glMatrixMode(GL_MODELVIEW);
    const int fbWidth = qMax(1, qRound(width() * dpr));
    const int fbHeight = qMax(1, qRound(height() * dpr));
    glViewport(0, 0, fbWidth, fbHeight);

    QVector3D camPos = computeViewMatrix().inverted().map(QVector3D(0, 0, 0));
    QVector3D viewDir = (m_camTarget - camPos).normalized();

    ViewPreset activePreset = ViewPreset::Front;
    const float ax = qAbs(viewDir.x());
    const float ay = qAbs(viewDir.y());
    const float az = qAbs(viewDir.z());
    if (ay >= ax && ay >= az) {
        activePreset = (viewDir.y() < 0.0f) ? ViewPreset::Top : ViewPreset::Bottom;
    } else if (ax >= ay && ax >= az) {
        activePreset = (viewDir.x() < 0.0f) ? ViewPreset::Right : ViewPreset::Left;
    } else {
        activePreset = (viewDir.z() < 0.0f) ? ViewPreset::Front : ViewPreset::Back;
    }

    QRect topRect(rect.left(), rect.top() - 14, rect.width(), 14);
    QRect bottomRect(rect.left(), rect.bottom() + 1, rect.width(), 14);
    QRect leftRect(rect.left() - sideLabelWidth, rect.top(), sideLabelWidth - 4, rect.height());
    QRect rightRect(rect.right() + sideLabelGap, rect.top(), sideLabelWidth, rect.height());
    QRect backRect(rect.right() - 36, rect.bottom() - 14, 36, 14);

    QRect markerAnchor = rect;
    switch (activePreset) {
    case ViewPreset::Top: markerAnchor = topRect; break;
    case ViewPreset::Bottom: markerAnchor = bottomRect; break;
    case ViewPreset::Left: markerAnchor = leftRect; break;
    case ViewPreset::Right: markerAnchor = rightRect; break;
    case ViewPreset::Back: markerAnchor = backRect; break;
    case ViewPreset::Front:
    case ViewPreset::Perspective:
    default:
        markerAnchor = rect;
        break;
    }

    QPainter p(this);
    p.setFont(QFont("Segoe UI", 8, QFont::Bold));

    const QPoint markerCenter = markerAnchor.center();
    const QRect markerRect(markerCenter.x() - 10, markerCenter.y() - 10, 20, 20);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(190, 190, 190, 210));
    p.drawRoundedRect(markerRect, 3, 3);

    auto drawLabel = [&](const QRect &r, const QString &text, Qt::Alignment align, ViewPreset preset) {
        p.setPen((activePreset == preset) ? QColor(255, 255, 255) : QColor(240, 240, 240));
        p.drawText(r, align, text);
    };

    drawLabel(rect, "Front", Qt::AlignCenter, ViewPreset::Front);
    drawLabel(topRect, "Top", Qt::AlignCenter, ViewPreset::Top);
    drawLabel(bottomRect, "Bottom", Qt::AlignCenter, ViewPreset::Bottom);
    drawLabel(leftRect, "Left", Qt::AlignVCenter | Qt::AlignRight, ViewPreset::Left);
    drawLabel(rightRect, "Right", Qt::AlignVCenter | Qt::AlignLeft, ViewPreset::Right);
    drawLabel(backRect, "Back", Qt::AlignRight | Qt::AlignBottom, ViewPreset::Back);
}

bool GLViewport::pickOrientationCube(const QPoint &pos, ViewPreset &outPreset) const
{
    const int size = 80;
    const int pad = 10;
    const int sideLabelGap = 4;
    const int sideLabelWidth = 34;
    const int shiftRight = 12;
    const int shiftDown = 28;
    QRect rect(width() - size - pad - sideLabelGap - sideLabelWidth + shiftRight, pad + shiftDown, size, size);
    if (!rect.contains(pos)) return false;

    QPoint local = pos - rect.topLeft();
    float ndcX = (local.x() / float(rect.width())) * 2.0f - 1.0f;
    float ndcY = 1.0f - (local.y() / float(rect.height())) * 2.0f;

    QMatrix4x4 proj;
    proj.ortho(-1.2f, 1.2f, -1.2f, 1.2f, -5.0f, 5.0f);
    QMatrix4x4 view;
    view.lookAt(QVector3D(0, 0, 3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    QMatrix4x4 camRot;
    camRot.rotate(m_camYaw, 0, 1, 0);
    camRot.rotate(m_camPitch, 1, 0, 0);
    QMatrix4x4 inv = (proj * view * camRot).inverted();

    QVector3D nearPoint = inv.map(QVector3D(ndcX, ndcY, -1.0f));
    QVector3D farPoint = inv.map(QVector3D(ndcX, ndcY, 1.0f));
    QVector3D dir = (farPoint - nearPoint).normalized();

    float tmin = 0.0f;
    float tmax = 1000.0f;
    QVector3D minV(-0.5f, -0.5f, -0.5f);
    QVector3D maxV(0.5f, 0.5f, 0.5f);

    for (int axis = 0; axis < 3; ++axis) {
        float origin = (axis == 0) ? nearPoint.x() : (axis == 1) ? nearPoint.y() : nearPoint.z();
        float direction = (axis == 0) ? dir.x() : (axis == 1) ? dir.y() : dir.z();
        float minA = (axis == 0) ? minV.x() : (axis == 1) ? minV.y() : minV.z();
        float maxA = (axis == 0) ? maxV.x() : (axis == 1) ? maxV.y() : maxV.z();
        if (fabsf(direction) < 1e-6f) {
            if (origin < minA || origin > maxA) return false;
        } else {
            float ood = 1.0f / direction;
            float t1 = (minA - origin) * ood;
            float t2 = (maxA - origin) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tmin = qMax(tmin, t1);
            tmax = qMin(tmax, t2);
            if (tmin > tmax) return false;
        }
    }

    QVector3D hit = nearPoint + dir * tmin;
    float ax = fabsf(hit.x());
    float ay = fabsf(hit.y());
    float az = fabsf(hit.z());
    if (ax >= ay && ax >= az)
        outPreset = (hit.x() > 0) ? ViewPreset::Right : ViewPreset::Left;
    else if (ay >= ax && ay >= az)
        outPreset = (hit.y() > 0) ? ViewPreset::Top : ViewPreset::Bottom;
    else
        outPreset = (hit.z() > 0) ? ViewPreset::Front : ViewPreset::Back;

    return true;
}

void GLViewport::updateInertia()
{
    if (m_rotating || m_panning || m_dragging) {
        m_inertiaTimer.stop();
        return;
    }

    if (m_orbitVelocity.lengthSquared() < 0.0001f) {
        m_inertiaTimer.stop();
        return;
    }

    m_camYaw += m_orbitVelocity.x();
    m_camPitch += m_orbitVelocity.y();
    m_camPitch = qBound(-89.0f, m_camPitch, 89.0f);
    m_orbitVelocity *= m_orbitDamping;
    update();
}

bool GLViewport::pickObject(const QPoint &pos, int &outIndex)
{
    if (!m_objects || m_objects->isEmpty()) return false;

    const float w = float(width() > 0 ? width() : 1);
    const float h = float(height() > 0 ? height() : 1);
    float ndcX = (2.0f * pos.x()) / w - 1.0f;
    float ndcY = 1.0f - (2.0f * pos.y()) / h;

    QMatrix4x4 view = computeViewMatrix();
    QMatrix4x4 proj = computeProjectionMatrix();
    QMatrix4x4 inv = (proj * view).inverted();

    QVector3D nearPoint = inv.map(QVector3D(ndcX, ndcY, -1.0f));
    QVector3D farPoint = inv.map(QVector3D(ndcX, ndcY, 1.0f));
    QVector3D dir = (farPoint - nearPoint).normalized();

    float closestT = FLT_MAX;
    int closestIdx = -1;
    for (int i = 0; i < m_objects->size(); ++i) {
        if (!m_objects->at(i).visible) continue;
        QVector3D minV, maxV;
        computeObjectBounds(i, minV, maxV);

        float tmin = 0.0f;
        float tmax = 10000.0f;
        for (int axis = 0; axis < 3; ++axis) {
            float origin = (axis == 0) ? nearPoint.x() : (axis == 1) ? nearPoint.y() : nearPoint.z();
            float direction = (axis == 0) ? dir.x() : (axis == 1) ? dir.y() : dir.z();
            float minA = (axis == 0) ? minV.x() : (axis == 1) ? minV.y() : minV.z();
            float maxA = (axis == 0) ? maxV.x() : (axis == 1) ? maxV.y() : maxV.z();
            if (fabsf(direction) < 1e-6f) {
                if (origin < minA || origin > maxA) {
                    tmin = 1.0f; tmax = 0.0f; break;
                }
            } else {
                float ood = 1.0f / direction;
                float t1 = (minA - origin) * ood;
                float t2 = (maxA - origin) * ood;
                if (t1 > t2) std::swap(t1, t2);
                tmin = qMax(tmin, t1);
                tmax = qMin(tmax, t2);
                if (tmin > tmax) break;
            }
        }
        if (tmin <= tmax && tmin < closestT) {
            closestT = tmin;
            closestIdx = i;
        }
    }

    if (closestIdx >= 0) {
        outIndex = closestIdx;
        return true;
    }
    return false;
}

bool GLViewport::screenToGroundPoint(const QPoint &pos, QVector3D &outPoint) const
{
    const float w = float(width() > 0 ? width() : 1);
    const float h = float(height() > 0 ? height() : 1);
    const float ndcX = (2.0f * pos.x()) / w - 1.0f;
    const float ndcY = 1.0f - (2.0f * pos.y()) / h;

    const QMatrix4x4 view = computeViewMatrix();
    const QMatrix4x4 proj = computeProjectionMatrix();
    const QMatrix4x4 inv = (proj * view).inverted();

    const QVector3D nearPoint = inv.map(QVector3D(ndcX, ndcY, -1.0f));
    const QVector3D farPoint = inv.map(QVector3D(ndcX, ndcY, 1.0f));
    const QVector3D dir = (farPoint - nearPoint);

    // Intersect with ground plane y = 0.
    if (qFuzzyIsNull(dir.y()))
        return false;

    const float t = (0.0f - nearPoint.y()) / dir.y();
    if (t < 0.0f)
        return false;

    outPoint = nearPoint + dir * t;
    return true;
}

GLViewport::GizmoAxis GLViewport::pickGizmoAxis(const QPoint &pos, GizmoMode mode) const
{
    Q_UNUSED(mode);
    if (m_selectedIdx < 0 || m_selectedIdx >= (m_objects ? m_objects->size() : 0))
        return GizmoAxis::None;

    QVector3D pivot = computeWorldPivot(m_selectedIdx);
    float size = qBound(0.8f, m_camDist * 0.08f, 4.0f);

    QMatrix4x4 view = computeViewMatrix();
    QMatrix4x4 proj = computeProjectionMatrix();
    QMatrix4x4 vp = proj * view;

    auto project = [&](const QVector3D &p) {
        QVector4D clip = vp * QVector4D(p, 1.0f);
        if (fabsf(clip.w()) < 1e-6f) return QPointF();
        QPointF ndc(clip.x() / clip.w(), clip.y() / clip.w());
        return QPointF((ndc.x() * 0.5f + 0.5f) * width(), (0.5f - ndc.y() * 0.5f) * height());
    };

    QPointF p0 = project(pivot);
    QPointF px = project(pivot + QVector3D(size, 0, 0));
    QPointF py = project(pivot + QVector3D(0, size, 0));
    QPointF pz = project(pivot + QVector3D(0, 0, size));

    auto distToSeg = [&](const QPointF &a, const QPointF &b) {
        QPointF ap = QPointF(pos) - a;
        QPointF ab = b - a;
        float ab2 = float(ab.x() * ab.x() + ab.y() * ab.y());
        float t = ab2 > 0 ? float((ap.x() * ab.x() + ap.y() * ab.y()) / ab2) : 0.0f;
        t = qBound(0.0f, t, 1.0f);
        QPointF closest = a + (b - a) * t;
        QPointF diff = QPointF(pos) - closest;
        return sqrtf(float(diff.x() * diff.x() + diff.y() * diff.y()));
    };

    const float thresh = 10.0f;
    float dx = distToSeg(p0, px);
    float dy = distToSeg(p0, py);
    float dz = distToSeg(p0, pz);

    float best = thresh;
    GizmoAxis axis = GizmoAxis::None;
    if (dx < best) { best = dx; axis = GizmoAxis::AxisX; }
    if (dy < best) { best = dy; axis = GizmoAxis::AxisY; }
    if (dz < best) { best = dz; axis = GizmoAxis::AxisZ; }

    return axis;
}

QVector3D GLViewport::screenDeltaToWorldAxis(const QPoint &delta, GizmoAxis axis) const
{
    float scale = m_camDist * 0.0025f;
    float dx = delta.x() * scale;
    float dy = -delta.y() * scale;

    if (axis == GizmoAxis::AxisX) return QVector3D(dx, 0, 0);
    if (axis == GizmoAxis::AxisY) return QVector3D(0, dy, 0);
    if (axis == GizmoAxis::AxisZ) return QVector3D(0, 0, dx);
    return QVector3D(0, 0, 0);
}

void GLViewport::applyGizmoDrag(const QPoint &pos)
{
    if (!m_objects || m_activeGizmoAxis == GizmoAxis::None) return;
    QPoint delta = pos - m_gizmoDragStart;
    QVector3D axisDelta = screenDeltaToWorldAxis(delta, m_activeGizmoAxis);

    QList<int> targets = m_selectedIndices.isEmpty()
                             ? QList<int>{m_selectedIdx}
                             : m_selectedIndices;

    const bool multiSelection = targets.size() > 1;
    const bool rotateAsGroup = (m_gizmoMode == GizmoMode::Rotate && multiSelection);
    const bool scaleAsGroup = (m_gizmoMode == GizmoMode::Scale && multiSelection);
    const bool useGroupCenter = rotateAsGroup || scaleAsGroup;

    QSet<int> selectedSet;
    for (int idx : targets)
        selectedSet.insert(idx);

    QList<int> groupTargets = targets;
    if (useGroupCenter) {
        groupTargets.clear();
        for (int idx : targets) {
            if (idx < 0 || idx >= m_objects->size()) continue;
            const int parent = m_objects->at(idx).parentIndex;
            if (parent >= 0 && selectedSet.contains(parent))
                continue;
            groupTargets.append(idx);
        }
        if (groupTargets.isEmpty())
            groupTargets = targets;
    }

    QSet<int> groupTargetSet;
    for (int idx : groupTargets)
        groupTargetSet.insert(idx);

    QVector3D groupCenter(0.0f, 0.0f, 0.0f);
    float groupAngle = (delta.x() + delta.y()) * 0.3f;
    if (m_snapEnabled)
        groupAngle = qRound(groupAngle / m_snapRotate) * m_snapRotate;

    QVector3D rotateAxis(0.0f, 0.0f, 0.0f);
    if (m_gizmoMode == GizmoMode::Rotate) {
        if (m_activeGizmoAxis == GizmoAxis::AxisX) rotateAxis = QVector3D(1.0f, 0.0f, 0.0f);
        if (m_activeGizmoAxis == GizmoAxis::AxisY) rotateAxis = QVector3D(0.0f, 1.0f, 0.0f);
        if (m_activeGizmoAxis == GizmoAxis::AxisZ) rotateAxis = QVector3D(0.0f, 0.0f, 1.0f);
    }
    const QQuaternion rotateDeltaQuat = QQuaternion::fromAxisAndAngle(rotateAxis, groupAngle).normalized();

    if (useGroupCenter) {
        int count = 0;
        for (int idx : groupTargets) {
            if (idx < 0 || idx >= m_objects->size()) continue;
            groupCenter += m_startPositions.value(idx, m_objects->at(idx).position);
            ++count;
        }
        if (count > 0)
            groupCenter /= float(count);
    }

    for (int i = 0; i < targets.size(); ++i) {
        int idx = targets[i];
        if (idx < 0 || idx >= m_objects->size()) continue;
        SceneObject &obj = (*m_objects)[idx];

        QVector3D basePos = m_startPositions.value(idx, obj.position);
        QVector3D baseRot = m_startRotations.value(idx, obj.rotation);
        QVector3D baseScale = m_startScales.value(idx, obj.scale);

        if (m_gizmoMode == GizmoMode::Move) {
            QVector3D next = basePos + axisDelta;
            if (m_snapEnabled) {
                next.setX(qRound(next.x() / m_snapMove) * m_snapMove);
                next.setY(qRound(next.y() / m_snapMove) * m_snapMove);
                next.setZ(qRound(next.z() / m_snapMove) * m_snapMove);
            }
            obj.position = next;
        } else if (m_gizmoMode == GizmoMode::Rotate) {
            if (rotateAsGroup && !groupTargetSet.contains(idx))
                continue;

            const QQuaternion baseQuat = QQuaternion::fromEulerAngles(baseRot.x(), baseRot.y(), baseRot.z());
            const QQuaternion newQuat = (rotateDeltaQuat * baseQuat).normalized();
            obj.rotation = newQuat.toEulerAngles();

            if (rotateAsGroup) {
                const QVector3D rel = basePos - groupCenter;
                obj.position = groupCenter + rotateDeltaQuat.rotatedVector(rel);
            }
        } else if (m_gizmoMode == GizmoMode::Scale) {
            if (scaleAsGroup && !groupTargetSet.contains(idx))
                continue;

            QVector3D next = baseScale;
            float s = 1.0f + (delta.y() * -0.005f);
            if (m_activeGizmoAxis == GizmoAxis::AxisX) next.setX(baseScale.x() * s);
            if (m_activeGizmoAxis == GizmoAxis::AxisY) next.setY(baseScale.y() * s);
            if (m_activeGizmoAxis == GizmoAxis::AxisZ) next.setZ(baseScale.z() * s);
            if (m_snapEnabled) {
                next.setX(qRound(next.x() / m_snapScale) * m_snapScale);
                next.setY(qRound(next.y() / m_snapScale) * m_snapScale);
                next.setZ(qRound(next.z() / m_snapScale) * m_snapScale);
            }
            next.setX(qMax(0.01f, next.x()));
            next.setY(qMax(0.01f, next.y()));
            next.setZ(qMax(0.01f, next.z()));
            obj.scale = next;

            if (scaleAsGroup) {
                QVector3D rel = basePos - groupCenter;
                if (m_activeGizmoAxis == GizmoAxis::AxisX) rel.setX(rel.x() * s);
                if (m_activeGizmoAxis == GizmoAxis::AxisY) rel.setY(rel.y() * s);
                if (m_activeGizmoAxis == GizmoAxis::AxisZ) rel.setZ(rel.z() * s);
                obj.position = groupCenter + rel;
            }
        }
    }

    emit objectMoved();
    update();
}

// ─── Mouse interaction ─────────────────────────────────────────────

void GLViewport::mousePressEvent(QMouseEvent *e)
{
    m_lastMouse = e->pos();
    m_inertiaTimer.stop();
    m_orbitVelocity = QVector2D(0, 0);

    if (m_walkMode) {
        if (e->button() == Qt::RightButton) {
            m_rightMouseLook = true;
            m_mouseLook = true;
            setCursor(Qt::BlankCursor);
            setFocus(Qt::OtherFocusReason);
            return;
        }
        if (e->button() == Qt::LeftButton && m_tool == Select) {
            buildWorldMatrices();
            int picked = -1;
            if (pickObject(e->pos(), picked))
                emit objectPicked(picked, e->modifiers());
            else
                emit objectPicked(-1, e->modifiers());
            return;
        }
    }

    buildWorldMatrices();

    if (e->button() == Qt::LeftButton) {
        QVector3D groundPoint;
        if (screenToGroundPoint(e->pos(), groundPoint)) {
            m_spawnAnchor = groundPoint + QVector3D(0.0f, 1.72f, 0.0f);
            m_hasSpawnAnchor = true;
        }
    }

    ViewPreset preset;
    if (e->button() == Qt::LeftButton && pickOrientationCube(e->pos(), preset)) {
        setPresetView(preset);
        return;
    }

    if (e->button() == Qt::LeftButton && m_tool == Select && !(e->modifiers() & Qt::AltModifier)) {
        int picked = -1;
        if (pickObject(e->pos(), picked))
            emit objectPicked(picked, e->modifiers());
        else
            emit objectPicked(-1, e->modifiers());
        return;
    }

    const bool isPerspectiveMode = (m_projectionBlend < 0.5f);
    const bool altOrbit = isPerspectiveMode && e->button() == Qt::LeftButton && (e->modifiers() & Qt::AltModifier);
    if (e->button() == Qt::MiddleButton || altOrbit) {
        if (e->button() == Qt::MiddleButton && (e->modifiers() & Qt::ShiftModifier)) {
            m_panning = true;
        } else if (e->button() == Qt::MiddleButton && (e->modifiers() & Qt::ControlModifier)) {
            m_dollying = true;
        } else {
            m_rotating = true;
        }
    } else if (e->button() == Qt::RightButton) {
        m_panning = true; // fallback for non-Blender users
    } else if (e->button() == Qt::LeftButton) {
        if (m_tool == Move || m_tool == Rotate || m_tool == Scale) {
            GizmoMode mode = (m_tool == Move) ? GizmoMode::Move :
                             (m_tool == Rotate) ? GizmoMode::Rotate :
                             GizmoMode::Scale;
            m_activeGizmoAxis = pickGizmoAxis(e->pos(), mode);
            if (m_activeGizmoAxis != GizmoAxis::None) {
                m_gizmoMode = mode;
                m_gizmoDragging = true;
                m_gizmoDragStart = e->pos();
                if (m_objects) {
                    m_startPositions = QList<QVector3D>(m_objects->size(), QVector3D());
                    m_startRotations = QList<QVector3D>(m_objects->size(), QVector3D());
                    m_startScales = QList<QVector3D>(m_objects->size(), QVector3D(1, 1, 1));
                    for (int i = 0; i < m_objects->size(); ++i) {
                        m_startPositions[i] = m_objects->at(i).position;
                        m_startRotations[i] = m_objects->at(i).rotation;
                        m_startScales[i] = m_objects->at(i).scale;
                    }
                }
                emit transformStarted();
                return;
            }

            m_dragging = true;
            m_gizmoDragStart = e->pos();
            if (m_objects) {
                m_startPositions = QList<QVector3D>(m_objects->size(), QVector3D());
                m_startRotations = QList<QVector3D>(m_objects->size(), QVector3D());
                m_startScales = QList<QVector3D>(m_objects->size(), QVector3D(1, 1, 1));
                for (int i = 0; i < m_objects->size(); ++i) {
                    m_startPositions[i] = m_objects->at(i).position;
                    m_startRotations[i] = m_objects->at(i).rotation;
                    m_startScales[i] = m_objects->at(i).scale;
                }
            }
            emit transformStarted();
        }
    }
}

void GLViewport::mouseMoveEvent(QMouseEvent *e)
{
    if (m_walkMode && m_mouseLook) {
        if (m_ctrlLookLock && m_cursorWarpInProgress) {
            m_cursorWarpInProgress = false;
            m_lastMouse = e->pos();
            return;
        }

        QPoint delta;
        if (m_ctrlLookLock) {
            const QPoint center(width() / 2, height() / 2);
            delta = e->pos() - center;
            m_lastMouse = center;
        } else {
            delta = e->pos() - m_lastMouse;
            m_lastMouse = e->pos();
        }

        // Natural FPS-style direction: mouse left turns camera left.
        m_camYaw -= delta.x() * 0.25f;
        m_camPitch -= delta.y() * 0.18f;
        m_camPitch = qBound(-85.0f, m_camPitch, 85.0f);
        update();

        if (m_ctrlLookLock && isVisible()) {
            const QPoint center(width() / 2, height() / 2);
            m_cursorWarpInProgress = true;
            QCursor::setPos(mapToGlobal(center));
        }
        return;
    }

    QPoint delta = e->pos() - m_lastMouse;
    m_lastMouse = e->pos();

    if (m_gizmoDragging) {
        applyGizmoDrag(e->pos());
        return;
    }

    if (m_rotating) {
        m_camAnimTimer.stop();
        m_camYaw   += delta.x() * 0.5f;
        m_camPitch += delta.y() * 0.3f;
        m_camPitch = qBound(-89.0f, m_camPitch, 89.0f);
        m_orbitVelocity = QVector2D(delta.x() * 0.5f, delta.y() * 0.3f);
        update();
    } else if (m_dollying) {
        m_camAnimTimer.stop();
        const float zoomStep = 1.0f + delta.y() * 0.01f;
        m_camDist *= qBound(0.2f, zoomStep, 5.0f);
        m_camDist = qBound(1.0f, m_camDist, 100.0f);
        update();
    } else if (m_panning) {
        m_camAnimTimer.stop();
        float factor = m_camDist * 0.003f;
        float yawRad = qDegreesToRadians(m_camYaw);
        m_camTarget.setX(m_camTarget.x() - cosf(yawRad) * delta.x() * factor);
        m_camTarget.setZ(m_camTarget.z() + sinf(yawRad) * delta.x() * factor);
        m_camTarget.setY(m_camTarget.y() + delta.y() * factor);
        update();
    } else if (m_dragging && m_objects && m_selectedIdx >= 0 && m_selectedIdx < m_objects->size()) {
        const QPoint dragDelta = e->pos() - m_gizmoDragStart;
        float speed = m_camDist * 0.001f;
        float yawRad = qDegreesToRadians(m_camYaw);

        // Build the list of indices to transform: all selected, or just primary if none
        QList<int> targets = m_selectedIndices.isEmpty()
                                 ? QList<int>{m_selectedIdx}
                                 : m_selectedIndices;

        const bool multiSelection = targets.size() > 1;
        const bool rotateAsGroup = (m_tool == Rotate && multiSelection);
        const bool scaleAsGroup = (m_tool == Scale && multiSelection);
        const bool useGroupCenter = rotateAsGroup || scaleAsGroup;

        QSet<int> selectedSet;
        for (int idx : targets)
            selectedSet.insert(idx);

        QList<int> groupTargets = targets;
        if (useGroupCenter) {
            groupTargets.clear();
            for (int idx : targets) {
                if (idx < 0 || idx >= m_objects->size()) continue;
                const int parent = m_objects->at(idx).parentIndex;
                if (parent >= 0 && selectedSet.contains(parent))
                    continue;
                groupTargets.append(idx);
            }
            if (groupTargets.isEmpty())
                groupTargets = targets;
        }

        QSet<int> groupTargetSet;
        for (int idx : groupTargets)
            groupTargetSet.insert(idx);

        QVector3D groupCenter(0.0f, 0.0f, 0.0f);
        if (useGroupCenter) {
            int count = 0;
            for (int idx : groupTargets) {
                if (idx < 0 || idx >= m_objects->size()) continue;
                groupCenter += m_startPositions.value(idx, m_objects->at(idx).position);
                ++count;
            }
            if (count > 0)
                groupCenter /= float(count);
        }

        float rotateY = dragDelta.x() * 0.5f;
        float rotateX = dragDelta.y() * 0.5f;
        if (m_snapEnabled) {
            rotateX = qRound(rotateX / m_snapRotate) * m_snapRotate;
            rotateY = qRound(rotateY / m_snapRotate) * m_snapRotate;
        }
        QVector3D cameraRight(cosf(yawRad), 0.0f, -sinf(yawRad));
        if (cameraRight.lengthSquared() < 1e-6f)
            cameraRight = QVector3D(1.0f, 0.0f, 0.0f);
        cameraRight.normalize();
        const QQuaternion dragDeltaQuat =
            (QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), rotateY) *
             QQuaternion::fromAxisAndAngle(cameraRight, rotateX)).normalized();

        for (int idx : targets) {
            if (idx < 0 || idx >= m_objects->size()) continue;
            SceneObject &obj = (*m_objects)[idx];
            const QVector3D basePos = m_startPositions.value(idx, obj.position);
            const QVector3D baseRot = m_startRotations.value(idx, obj.rotation);
            const QVector3D baseScale = m_startScales.value(idx, obj.scale);

            if (m_tool == Move) {
                if (e->modifiers() & Qt::ShiftModifier)
                    obj.position = basePos + QVector3D(0.0f, -dragDelta.y() * speed, 0.0f);
                else {
                    obj.position = basePos;
                    obj.position.setX(basePos.x() + cosf(yawRad) * dragDelta.x() * speed);
                    obj.position.setZ(basePos.z() - sinf(yawRad) * dragDelta.x() * speed);
                    obj.position.setY(basePos.y() - dragDelta.y() * speed);
                }
                if (m_snapEnabled) {
                    obj.position.setX(qRound(obj.position.x() / m_snapMove) * m_snapMove);
                    obj.position.setY(qRound(obj.position.y() / m_snapMove) * m_snapMove);
                    obj.position.setZ(qRound(obj.position.z() / m_snapMove) * m_snapMove);
                }
            } else if (m_tool == Rotate) {
                if (rotateAsGroup && !groupTargetSet.contains(idx))
                    continue;
                const QQuaternion baseQuat = QQuaternion::fromEulerAngles(baseRot.x(), baseRot.y(), baseRot.z());
                const QQuaternion newQuat = (dragDeltaQuat * baseQuat).normalized();
                obj.rotation = newQuat.toEulerAngles();

                if (rotateAsGroup) {
                    const QVector3D rel = basePos - groupCenter;
                    obj.position = groupCenter + dragDeltaQuat.rotatedVector(rel);
                }
            } else if (m_tool == Scale) {
                if (scaleAsGroup && !groupTargetSet.contains(idx))
                    continue;

                float s = 1.0f + dragDelta.y() * (-0.005f);
                obj.scale = baseScale * s;
                if (m_snapEnabled) {
                    obj.scale.setX(qMax(0.01f, qRound(obj.scale.x() / m_snapScale) * m_snapScale));
                    obj.scale.setY(qMax(0.01f, qRound(obj.scale.y() / m_snapScale) * m_snapScale));
                    obj.scale.setZ(qMax(0.01f, qRound(obj.scale.z() / m_snapScale) * m_snapScale));
                }

                if (scaleAsGroup) {
                    const QVector3D rel = basePos - groupCenter;
                    obj.position = groupCenter + (rel * s);
                }
            }
        }
        emit objectMoved();
        update();
    }
}

void GLViewport::mouseReleaseEvent(QMouseEvent *)
{
    if (m_walkMode) {
        m_rightMouseLook = false;
        m_mouseLook = m_ctrlLookLock;
        if (m_mouseLook)
            setCursor(Qt::BlankCursor);
        else
            setCursor(Qt::CrossCursor);
        return;
    }

    m_rotating = false;
    m_panning = false;
    m_dollying = false;
    if (m_dragging || m_gizmoDragging)
        emit transformFinished();
    m_dragging = false;
    m_gizmoDragging = false;
    m_activeGizmoAxis = GizmoAxis::None;

    if (m_orbitVelocity.lengthSquared() > 0.01f)
        m_inertiaTimer.start();
}

void GLViewport::wheelEvent(QWheelEvent *e)
{
    if (m_walkMode) {
        const float delta = e->angleDelta().y();
        if (delta > 0)
            m_walkSpeed = qMin(14.0f, m_walkSpeed + 0.4f);
        else if (delta < 0)
            m_walkSpeed = qMax(1.2f, m_walkSpeed - 0.4f);
        update();
        return;
    }

    m_camAnimTimer.stop();
    float d = e->angleDelta().y();
    m_camDist *= (d > 0) ? 0.9f : 1.1f;
    m_camDist = qBound(1.0f, m_camDist, 100.0f);
    update();
}

void GLViewport::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat()) {
        QOpenGLWidget::keyPressEvent(e);
        return;
    }

    if (m_gizmoDragging) {
        if (e->key() == Qt::Key_X || e->key() == Qt::Key_Y || e->key() == Qt::Key_Z) {
            if (e->key() == Qt::Key_X) m_activeGizmoAxis = GizmoAxis::AxisX;
            if (e->key() == Qt::Key_Y) m_activeGizmoAxis = GizmoAxis::AxisY;
            if (e->key() == Qt::Key_Z) m_activeGizmoAxis = GizmoAxis::AxisZ;
            update();
            e->accept();
            return;
        }

        if (e->key() == Qt::Key_Escape && m_objects) {
            QList<int> targets = m_selectedIndices.isEmpty()
                                     ? QList<int>{m_selectedIdx}
                                     : m_selectedIndices;
            for (int idx : targets) {
                if (idx < 0 || idx >= m_objects->size()) continue;
                SceneObject &obj = (*m_objects)[idx];
                obj.position = m_startPositions.value(idx, obj.position);
                obj.rotation = m_startRotations.value(idx, obj.rotation);
                obj.scale = m_startScales.value(idx, obj.scale);
            }
            m_gizmoDragging = false;
            m_dragging = false;
            m_activeGizmoAxis = GizmoAxis::None;
            emit objectMoved();
            emit transformFinished();
            update();
            e->accept();
            return;
        }
    }

    if (e->key() == Qt::Key_F6) {
        setWalkMode(!m_walkMode);
        e->accept();
        return;
    }

    if (!m_walkMode) {
        const bool ctrl = (e->modifiers() & Qt::ControlModifier);
        if (e->key() == Qt::Key_1 || e->key() == Qt::Key_End) {
            setPresetView(ctrl ? ViewPreset::Back : ViewPreset::Front);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_3 || e->key() == Qt::Key_PageDown) {
            setPresetView(ctrl ? ViewPreset::Left : ViewPreset::Right);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_7 || e->key() == Qt::Key_Home) {
            setPresetView(ctrl ? ViewPreset::Bottom : ViewPreset::Top);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_5) {
            const float targetBlend = (m_projectionBlend < 0.5f) ? 1.0f : 0.0f;
            startCameraAnimation(m_camYaw, m_camPitch, m_camDist, m_camTarget, targetBlend);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Period) {
            frameSelected();
            e->accept();
            return;
        }
    }

    if (m_walkMode) {
        if (e->key() == Qt::Key_Shift) {
            m_ctrlLookLock = !m_ctrlLookLock;
            m_mouseLook = (m_ctrlLookLock || m_rightMouseLook);
            if (m_mouseLook)
                setCursor(Qt::BlankCursor);
            else
                setCursor(Qt::CrossCursor);

            if (m_ctrlLookLock && isVisible()) {
                const QPoint center(width() / 2, height() / 2);
                m_lastMouse = center;
                m_cursorWarpInProgress = true;
                QCursor::setPos(mapToGlobal(center));
            }
            update();
            e->accept();
            return;
        }
        m_pressedKeys.insert(e->key());
        e->accept();
        return;
    }

    QOpenGLWidget::keyPressEvent(e);
}

void GLViewport::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat()) {
        QOpenGLWidget::keyReleaseEvent(e);
        return;
    }

    if (m_walkMode) {
        if (e->key() == Qt::Key_Shift) {
            e->accept();
            return;
        }
        m_pressedKeys.remove(e->key());
        e->accept();
        return;
    }

    QOpenGLWidget::keyReleaseEvent(e);
}

void GLViewport::updateWalkNavigation()
{
    if (!m_walkMode)
        return;

    const float dt = 0.016f;
    const float yawRad = qDegreesToRadians(m_camYaw);
    const float pitchRad = qDegreesToRadians(m_camPitch);

    QVector3D forward(cosf(pitchRad) * sinf(yawRad),
                      sinf(pitchRad),
                      cosf(pitchRad) * cosf(yawRad));
    forward.normalize();

    QVector3D right = QVector3D::crossProduct(forward, QVector3D(0, 1, 0)).normalized();
    QVector3D move(0, 0, 0);

    if (m_pressedKeys.contains(Qt::Key_Z) || m_pressedKeys.contains(Qt::Key_Up))
        move += QVector3D(forward.x(), 0.0f, forward.z()).normalized();
    if (m_pressedKeys.contains(Qt::Key_S) || m_pressedKeys.contains(Qt::Key_Down))
        move -= QVector3D(forward.x(), 0.0f, forward.z()).normalized();
    if (m_pressedKeys.contains(Qt::Key_Q) || m_pressedKeys.contains(Qt::Key_Left))
        move -= right;
    if (m_pressedKeys.contains(Qt::Key_D) || m_pressedKeys.contains(Qt::Key_Right))
        move += right;
    if (m_pressedKeys.contains(Qt::Key_Space))
        move += QVector3D(0, 1, 0);
    if (m_pressedKeys.contains(Qt::Key_Control) || m_pressedKeys.contains(Qt::Key_C))
        move -= QVector3D(0, 1, 0);

    if (!qFuzzyIsNull(move.lengthSquared())) {
        move.normalize();
        const bool boost = m_pressedKeys.contains(Qt::Key_Alt);
        const float speed = boost ? (m_walkSpeed * 1.8f) : m_walkSpeed;
        m_freeCamPos += move * speed * dt;

        // Keep the camera in a practical room-testing range.
        m_freeCamPos.setY(qBound(0.6f, m_freeCamPos.y(), 8.0f));
        m_freeCamPos.setX(qBound(-24.0f, m_freeCamPos.x(), 24.0f));
        m_freeCamPos.setZ(qBound(-24.0f, m_freeCamPos.z(), 24.0f));
        update();
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  ModelingWidget – Full 3D modeler panel
// ════════════════════════════════════════════════════════════════════════════

static const QString kBtnStyle = R"(
    QPushButton {
        background-color: #8B6F47;
        border-radius: 8px;
        color: white;
        font-weight: bold;
        padding: 6px 14px;
        font-size: 13px;
    }
    QPushButton:hover {
        background-color: #a38253;
        border: 2px solid #8B6F47;
    }
    QPushButton:checked {
        background-color: #FFF;
        color: #8B6F47;
        border: 2px solid #8B6F47;
    }
)";

static const QString kGroupStyle = R"(
    QGroupBox {
        font-weight: bold;
        color: #F5E6C8;
        border: 1px solid #8B6F47;
        border-radius: 6px;
        margin-top: 14px;
        padding-top: 16px;
        background: rgba(30, 25, 20, 180);
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 12px;
        padding: 0 6px;
        color: #F5E6C8;
    }
)";

ModelingWidget::ModelingWidget(QWidget *parent)
    : QWidget(parent)
{
    // ── Main layout: splitter with viewport + side panel ──
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter);

    // ── Left: viewport + toolbar ──
    auto *viewportContainer = new QWidget;
    auto *vpLayout = new QVBoxLayout(viewportContainer);
    vpLayout->setContentsMargins(4, 4, 4, 4);
    vpLayout->setSpacing(4);

    // Toolbar
    auto *toolbar = new QWidget;
    auto *toolbarLayout = new QVBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(4, 2, 4, 2);
    toolbarLayout->setSpacing(4);

    auto *tbRowTop = new QHBoxLayout;
    tbRowTop->setSpacing(6);

    auto *tbRowViews = new QHBoxLayout;
    tbRowViews->setSpacing(6);

    m_btnSelect = new QPushButton(tr("Select"));
    m_btnMove   = new QPushButton(tr("Move"));
    m_btnRotate = new QPushButton(tr("Rotate"));
    m_btnScale  = new QPushButton(tr("Scale"));
    for (auto *b : {m_btnSelect, m_btnMove, m_btnRotate, m_btnScale}) {
        b->setCheckable(true);
        b->setStyleSheet(kBtnStyle);
        b->setFixedHeight(30);
        b->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        tbRowTop->addWidget(b);
    }
    m_btnSelect->setChecked(true);

    tbRowTop->addSpacing(8);
    m_gridCheck = new QCheckBox(tr("Grid"));
    m_gridCheck->setChecked(true);
    m_gridCheck->setStyleSheet("color: #F5E6C8; font-weight: bold;");
    m_gridCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_gridCheck->setMinimumWidth(m_gridCheck->sizeHint().width() + 6);
    tbRowTop->addWidget(m_gridCheck);

    m_boundsCheck = new QCheckBox(tr("Bounds"));
    m_boundsCheck->setStyleSheet("color: #F5E6C8; font-weight: bold;");
    m_boundsCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_boundsCheck->setMinimumWidth(m_boundsCheck->sizeHint().width() + 6);
    tbRowTop->addWidget(m_boundsCheck);

    auto *walkModeCheck = new QCheckBox(tr("Walk Mode"));
    walkModeCheck->setStyleSheet("color: #F5E6C8; font-weight: bold;");
    walkModeCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    walkModeCheck->setMinimumWidth(walkModeCheck->sizeHint().width() + 6);
    tbRowTop->addWidget(walkModeCheck);

    auto *shadeLabel = new QLabel(tr("Shading"));
    shadeLabel->setStyleSheet("color: #F5E6C8; font-weight: bold;");
    shadeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tbRowTop->addWidget(shadeLabel);
    m_shadingCombo = new QComboBox;
    m_shadingCombo->addItems({tr("Solid"), tr("Wireframe"), tr("Solid+Wire"), tr("Unlit")});
    m_shadingCombo->setStyleSheet("QComboBox { background: #FFF; border-radius: 6px; padding: 4px 8px; font-size: 12px; }");
    m_shadingCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_shadingCombo->setMinimumContentsLength(10);
    m_shadingCombo->setMinimumWidth(122);
    m_shadingCombo->setFixedHeight(28);
    tbRowTop->addWidget(m_shadingCombo);

    auto *btnHelpModeling = new QToolButton;
    btnHelpModeling->setObjectName("btn_help_modeling");
    btnHelpModeling->setText(tr("?"));
    btnHelpModeling->setCheckable(true);
    btnHelpModeling->setCursor(Qt::PointingHandCursor);
    btnHelpModeling->setFixedSize(32, 32);
    btnHelpModeling->setStyleSheet(
        "QToolButton { background-color: #8B6F47; border-radius: 16px; color: white; font-weight: bold; border: none; font-size: 16px; }"
        "QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"
    );
    tbRowTop->addWidget(btnHelpModeling);

    tbRowTop->addStretch();

    auto addViewButton = [&](const QString &label, GLViewport::ViewPreset preset) {
        auto *btn = new QPushButton(label);
        btn->setStyleSheet(kBtnStyle);
        btn->setFixedHeight(30);
        btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        tbRowViews->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, preset]() {
            m_viewport->setPresetView(preset);
        });
    };
    addViewButton(tr("Top"), GLViewport::ViewPreset::Top);
    addViewButton(tr("Bottom"), GLViewport::ViewPreset::Bottom);
    addViewButton(tr("Left"), GLViewport::ViewPreset::Left);
    addViewButton(tr("Right"), GLViewport::ViewPreset::Right);
    addViewButton(tr("Front"), GLViewport::ViewPreset::Front);
    addViewButton(tr("Back"), GLViewport::ViewPreset::Back);
    addViewButton(tr("Perspective"), GLViewport::ViewPreset::Perspective);

    tbRowViews->addStretch();

    toolbarLayout->addLayout(tbRowTop);
    toolbarLayout->addLayout(tbRowViews);

    toolbar->setStyleSheet("background: rgba(30, 25, 20, 200); border-radius: 6px;");

    // OpenGL viewport — above the toolbar so toolbar sits at the bottom away from radio buttons
    m_viewport = new GLViewport;
    m_viewport->setObjects(&m_objects);
    m_viewport->setStyleSheet("border: 2px solid #8B6F47; border-radius: 6px;");
    vpLayout->addWidget(m_viewport, 1);

    // Toolbar placed BELOW the viewport
    vpLayout->addWidget(toolbar);

    splitter->addWidget(viewportContainer);

    // ── Right: side panel ──
    auto *sideScroll = new QScrollArea;
    sideScroll->setWidgetResizable(true);
    sideScroll->setMinimumWidth(280);
    sideScroll->setMaximumWidth(340);
    sideScroll->setStyleSheet("QScrollArea { border: none; background: rgba(20, 18, 15, 220); }");

    auto *sidePanel = new QWidget;
    auto *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(8, 8, 8, 8);
    sideLayout->setSpacing(8);

    // ── Add primitives ──
    auto *addGroup = new QGroupBox(tr("Add Primitives"));
    addGroup->setStyleSheet(kGroupStyle);
    auto *addLayout = new QHBoxLayout(addGroup);
    m_addCombo = new QComboBox;
    m_addCombo->addItems({
        tr("Cube"), tr("Cylinder"), tr("Sphere"), tr("Plane"),
        tr("Cone"), tr("Pyramid"), tr("Capsule"), tr("Torus")
    });
    m_addCombo->setStyleSheet("QComboBox { background: #FFF; border-radius: 6px; padding: 4px 8px; font-size: 13px; }");
    addLayout->addWidget(m_addCombo);

    auto *btnAdd = new QPushButton(tr("Add"));
    btnAdd->setStyleSheet(kBtnStyle);
    addLayout->addWidget(btnAdd);
    sideLayout->addWidget(addGroup);

    // ── Presets ──
    auto *presetGroup = new QGroupBox(tr("Furniture & Test Presets"));
    presetGroup->setStyleSheet(kGroupStyle);
    auto *presetLayout = new QVBoxLayout(presetGroup);
    auto *presetRow1 = new QHBoxLayout;
    auto *presetRow2 = new QHBoxLayout;
    auto *btnChair    = new QPushButton(tr("Chair"));
    auto *btnTable    = new QPushButton(tr("Table"));
    auto *btnCabinet  = new QPushButton(tr("Cabinet"));
    auto *btnWardrobe = new QPushButton(tr("Wardrobe"));
    auto *btnHouseOnly = new QPushButton(tr("House (Empty)"));
    auto *btnSimRoom  = new QPushButton(tr("House Sim Room"));
    for (auto *b : {btnChair, btnTable, btnCabinet, btnWardrobe, btnHouseOnly, btnSimRoom})
        b->setStyleSheet(kBtnStyle);
    presetRow1->addWidget(btnChair);
    presetRow1->addWidget(btnTable);
    presetRow2->addWidget(btnCabinet);
    presetRow2->addWidget(btnWardrobe);
    presetLayout->addWidget(btnHouseOnly);
    presetLayout->addWidget(btnSimRoom);
    presetLayout->addLayout(presetRow1);
    presetLayout->addLayout(presetRow2);
    sideLayout->addWidget(presetGroup);

    // ── Object list ──
    auto *objGroup = new QGroupBox(tr("Scene Objects"));
    objGroup->setStyleSheet(kGroupStyle);
    auto *objLayout = new QVBoxLayout(objGroup);
    m_objectSearch = new QLineEdit;
    m_objectSearch->setPlaceholderText(tr("Search objects..."));
    m_objectSearch->setStyleSheet("QLineEdit { background: #FFF; border-radius: 6px; padding: 4px 8px; font-size: 12px; }");
    objLayout->addWidget(m_objectSearch);

    m_objectList = new QListWidget;
    m_objectList->setSelectionMode(QAbstractItemView::ExtendedSelection);  // Ctrl/Shift multi-select
    m_objectList->setStyleSheet(R"(
        QListWidget { background: rgba(40,35,30,200); color: #F5E6C8; border: 1px solid #8B6F47; border-radius: 4px; font-size: 13px; }
        QListWidget::item:selected { background: #8B6F47; color: white; }
    )");
    m_objectList->setMaximumHeight(160);
    auto *multiHint = new QLabel(tr("Ctrl+Click or Shift+Click to select multiple"));
    multiHint->setStyleSheet("color: #999; font-size: 10px; padding: 1px 2px;");
    objLayout->addWidget(m_objectList);
    objLayout->addWidget(multiHint);

    auto *objBtnRow = new QHBoxLayout;
    auto *btnDuplicate = new QPushButton(tr("Duplicate"));
    auto *btnDelete = new QPushButton(tr("Delete"));
    auto *btnClear = new QPushButton(tr("Clear All"));
    btnDuplicate->setStyleSheet(kBtnStyle);
    btnDelete->setStyleSheet(kBtnStyle + "QPushButton { background-color: #A03030; } QPushButton:hover { background-color: #D04040; }");
    btnClear->setStyleSheet(kBtnStyle + "QPushButton { background-color: #A03030; } QPushButton:hover { background-color: #D04040; }");
    objBtnRow->addWidget(btnDuplicate);
    objBtnRow->addWidget(btnDelete);
    objBtnRow->addWidget(btnClear);
    objLayout->addLayout(objBtnRow);

    auto *visRow = new QHBoxLayout;
    m_btnHideSelected = new QPushButton(tr("Hide"));
    m_btnIsolateSelected = new QPushButton(tr("Isolate"));
    m_btnUnhideAll = new QPushButton(tr("Unhide All"));
    m_btnHideSelected->setStyleSheet(kBtnStyle);
    m_btnIsolateSelected->setStyleSheet(kBtnStyle);
    m_btnUnhideAll->setStyleSheet(kBtnStyle);
    visRow->addWidget(m_btnHideSelected);
    visRow->addWidget(m_btnIsolateSelected);
    visRow->addWidget(m_btnUnhideAll);
    objLayout->addLayout(visRow);

    // Save / Load row
    auto *ioRow = new QHBoxLayout;
    auto *btnSave = new QPushButton(tr("Save Scene"));
    auto *btnLoad = new QPushButton(tr("Load Scene"));
    const QString ioStyle = kBtnStyle + "QPushButton { background-color: #2E6B3E; } QPushButton:hover { background-color: #3A8A4F; }";
    btnSave->setStyleSheet(ioStyle);
    btnLoad->setStyleSheet(ioStyle);
    ioRow->addWidget(btnSave);
    ioRow->addWidget(btnLoad);
    objLayout->addLayout(ioRow);
    sideLayout->addWidget(objGroup);

    // ── Snapping ──
    auto *snapGroup = new QGroupBox(tr("Snapping"));
    snapGroup->setStyleSheet(kGroupStyle);
    auto *snapLayout = new QVBoxLayout(snapGroup);
    m_snapCheck = new QCheckBox(tr("Enable snapping"));
    m_snapCheck->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 12px;");
    snapLayout->addWidget(m_snapCheck);

    auto *snapRow1 = new QHBoxLayout;
    auto *lblMoveSnap = new QLabel(tr("Move"));
    lblMoveSnap->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 11px;");
    m_snapMoveCombo = new QComboBox;
    m_snapMoveCombo->addItems({"0.1", "0.5", "1"});
    m_snapMoveCombo->setCurrentIndex(1);
    m_snapMoveCombo->setStyleSheet("QComboBox { background: #FFF; border-radius: 4px; padding: 2px 6px; font-size: 11px; }");
    snapRow1->addWidget(lblMoveSnap);
    snapRow1->addWidget(m_snapMoveCombo);
    snapLayout->addLayout(snapRow1);

    auto *snapRow2 = new QHBoxLayout;
    auto *lblRotSnap = new QLabel(tr("Rotate"));
    lblRotSnap->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 11px;");
    m_snapRotateCombo = new QComboBox;
    m_snapRotateCombo->addItems({"5", "15", "30"});
    m_snapRotateCombo->setCurrentIndex(1);
    m_snapRotateCombo->setStyleSheet("QComboBox { background: #FFF; border-radius: 4px; padding: 2px 6px; font-size: 11px; }");
    snapRow2->addWidget(lblRotSnap);
    snapRow2->addWidget(m_snapRotateCombo);
    snapLayout->addLayout(snapRow2);

    auto *snapRow3 = new QHBoxLayout;
    auto *lblScaleSnap = new QLabel(tr("Scale"));
    lblScaleSnap->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 11px;");
    m_snapScaleCombo = new QComboBox;
    m_snapScaleCombo->addItems({"0.1", "0.25", "0.5"});
    m_snapScaleCombo->setCurrentIndex(0);
    m_snapScaleCombo->setStyleSheet("QComboBox { background: #FFF; border-radius: 4px; padding: 2px 6px; font-size: 11px; }");
    snapRow3->addWidget(lblScaleSnap);
    snapRow3->addWidget(m_snapScaleCombo);
    snapLayout->addLayout(snapRow3);

    sideLayout->addWidget(snapGroup);
    
    // ── Properties ──
    auto *propGroup = new QGroupBox(tr("Properties"));
    propGroup->setStyleSheet(kGroupStyle);
    auto *propLayout = new QVBoxLayout(propGroup);

    auto makeRow = [&](const QString &label, QDoubleSpinBox *&x, QDoubleSpinBox *&y, QDoubleSpinBox *&z, double minVal, double maxVal, double step) {
        auto *row = new QHBoxLayout;
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 12px;");
        lbl->setFixedWidth(50);
        row->addWidget(lbl);
        for (auto **sp : {&x, &y, &z}) {
            *sp = new QDoubleSpinBox;
            (*sp)->setRange(minVal, maxVal);
            (*sp)->setSingleStep(step);
            (*sp)->setDecimals(2);
            (*sp)->setStyleSheet("QDoubleSpinBox { background: #FFF; border-radius: 4px; padding: 2px; font-size: 12px; }");
            row->addWidget(*sp);
        }
        propLayout->addLayout(row);
    };

    makeRow(tr("Pos"),   m_posX,   m_posY,   m_posZ,   -50, 50, 0.1);
    makeRow(tr("Rot"),   m_rotX,   m_rotY,   m_rotZ,   -360, 360, 5.0);
    makeRow(tr("Scale"), m_scaleX, m_scaleY, m_scaleZ,  0.01, 50, 0.1);

    auto *parentRow = new QHBoxLayout;
    auto *parentLabel = new QLabel(tr("Parent"));
    parentLabel->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 12px;");
    parentLabel->setFixedWidth(50);
    parentRow->addWidget(parentLabel);
    m_parentCombo = new QComboBox;
    m_parentCombo->setStyleSheet("QComboBox { background: #FFF; border-radius: 4px; padding: 2px; font-size: 12px; }");
    parentRow->addWidget(m_parentCombo);
    propLayout->addLayout(parentRow);

    // Color
    auto *colorRow = new QHBoxLayout;
    auto *colorLabel = new QLabel(tr("Color"));
    colorLabel->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 12px;");
    colorLabel->setFixedWidth(50);
    colorRow->addWidget(colorLabel);
    m_colorBtn = new QPushButton;
    m_colorBtn->setFixedSize(60, 28);
    m_colorBtn->setStyleSheet("background: #c8a064; border: 2px solid #8B6F47; border-radius: 6px;");
    colorRow->addWidget(m_colorBtn);

    m_visibleCheck = new QCheckBox(tr("Visible"));
    m_visibleCheck->setChecked(true);
    m_visibleCheck->setStyleSheet("color: #F5E6C8; font-weight: bold; font-size: 12px;");
    colorRow->addWidget(m_visibleCheck);
    colorRow->addStretch();
    propLayout->addLayout(colorRow);

    sideLayout->addWidget(propGroup);
    sideLayout->addStretch();

    sideScroll->setWidget(sidePanel);
    splitter->addWidget(sideScroll);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    // ══════════ Connections ══════════

    // Tool buttons (radio-like)
    auto setToolBtn = [this](QPushButton *active, GLViewport::Tool tool) {
        for (auto *b : {m_btnSelect, m_btnMove, m_btnRotate, m_btnScale})
            b->setChecked(b == active);
        m_viewport->setTool(tool);
    };
    connect(m_btnSelect, &QPushButton::clicked, this, [=]() { setToolBtn(m_btnSelect, GLViewport::Select); });
    connect(m_btnMove,   &QPushButton::clicked, this, [=]() { setToolBtn(m_btnMove,   GLViewport::Move);   });
    connect(m_btnRotate, &QPushButton::clicked, this, [=]() { setToolBtn(m_btnRotate, GLViewport::Rotate); });
    connect(m_btnScale,  &QPushButton::clicked, this, [=]() { setToolBtn(m_btnScale,  GLViewport::Scale);  });

    auto bindViewportToolShortcut = [this, setToolBtn](int key, QPushButton *button, GLViewport::Tool tool) {
        auto *sc = new QShortcut(QKeySequence(key), m_viewport);
        sc->setContext(Qt::WidgetWithChildrenShortcut);
        connect(sc, &QShortcut::activated, this, [=]() { setToolBtn(button, tool); });
    };
    bindViewportToolShortcut(Qt::Key_Q, m_btnSelect, GLViewport::Select);
    bindViewportToolShortcut(Qt::Key_G, m_btnMove, GLViewport::Move);
    bindViewportToolShortcut(Qt::Key_R, m_btnRotate, GLViewport::Rotate);
    bindViewportToolShortcut(Qt::Key_S, m_btnScale, GLViewport::Scale);

    connect(m_gridCheck, &QCheckBox::toggled, m_viewport, &GLViewport::setShowGrid);
    connect(m_boundsCheck, &QCheckBox::toggled, m_viewport, &GLViewport::setShowBounds);
    connect(walkModeCheck, &QCheckBox::toggled, m_viewport, &GLViewport::setWalkMode);
    connect(m_viewport, &GLViewport::walkModeChanged, this, [walkModeCheck](bool enabled) {
        QSignalBlocker blocker(walkModeCheck);
        walkModeCheck->setChecked(enabled);
    });
    connect(m_shadingCombo, &QComboBox::currentIndexChanged, this, [this](int idx) {
        m_viewport->setShadingMode(static_cast<GLViewport::ShadingMode>(idx));
    });

    // Add primitive
    connect(btnAdd, &QPushButton::clicked, this, [this]() {
        addPrimitive(static_cast<PrimitiveType>(m_addCombo->currentIndex()));
    });

    // Presets
    connect(btnChair,    &QPushButton::clicked, this, [this]() { loadPreset("Chair"); });
    connect(btnTable,    &QPushButton::clicked, this, [this]() { loadPreset("Table"); });
    connect(btnCabinet,  &QPushButton::clicked, this, [this]() { loadPreset("Cabinet"); });
    connect(btnWardrobe, &QPushButton::clicked, this, [this]() { loadPreset("Wardrobe"); });
    connect(btnHouseOnly,&QPushButton::clicked, this, [this]() { loadPreset("House"); });
    connect(btnSimRoom,  &QPushButton::clicked, this, [this]() { loadPreset("HouseSimRoom"); });

    // Object list — handle both single and multi selection
    connect(m_objectList, &QListWidget::itemSelectionChanged, this, [this]() {
        QList<QListWidgetItem*> sel = m_objectList->selectedItems();
        if (sel.isEmpty()) {
            m_viewport->setSelectedIndex(-1);
            m_viewport->setSelectedIndices({});
            return;
        }
        int row = m_objectList->row(m_objectList->currentItem());
        m_lastClickedRow = row;

        QList<int> selectedIndices;
        for (auto *item : sel) {
            int r = m_objectList->row(item);
            int objIndex = objectIndexFromRow(r);
            if (objIndex >= 0)
                selectedIndices.append(objIndex);
        }
        int primaryIndex = objectIndexFromRow(row);
        m_viewport->setSelectedIndices(selectedIndices);
        m_viewport->setSelectedIndex(primaryIndex);
        updatePropertyPanel();
    });
    connect(btnDelete,    &QPushButton::clicked, this, &ModelingWidget::deleteSelected);
    connect(btnDuplicate, &QPushButton::clicked, this, &ModelingWidget::duplicateSelected);
    connect(btnClear,     &QPushButton::clicked, this, &ModelingWidget::clearScene);
    connect(btnSave,      &QPushButton::clicked, this, &ModelingWidget::saveScene);
    connect(btnLoad,      &QPushButton::clicked, this, &ModelingWidget::loadScene);
    connect(m_btnHideSelected, &QPushButton::clicked, this, &ModelingWidget::hideSelectedObjects);
    connect(m_btnUnhideAll, &QPushButton::clicked, this, &ModelingWidget::unhideAllObjects);
    connect(m_btnIsolateSelected, &QPushButton::clicked, this, &ModelingWidget::isolateSelectedObjects);

    connect(m_objectSearch, &QLineEdit::textChanged, this, &ModelingWidget::onObjectSearchChanged);

    // Property changes
    for (auto *sp : {m_posX, m_posY, m_posZ, m_rotX, m_rotY, m_rotZ, m_scaleX, m_scaleY, m_scaleZ})
        connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ModelingWidget::onPropertyChanged);
    connect(m_visibleCheck, &QCheckBox::toggled,    this, &ModelingWidget::onPropertyChanged);
    connect(m_colorBtn,     &QPushButton::clicked,  this, &ModelingWidget::onColorPick);
    connect(m_parentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModelingWidget::onParentChanged);

    connect(m_snapCheck, &QCheckBox::toggled, m_viewport, &GLViewport::setSnapEnabled);
    auto updateSnapSteps = [this]() {
        float moveStep = m_snapMoveCombo->currentText().toFloat();
        float rotStep = m_snapRotateCombo->currentText().toFloat();
        float scaleStep = m_snapScaleCombo->currentText().toFloat();
        m_viewport->setSnapSteps(moveStep, rotStep, scaleStep);
    };
    connect(m_snapMoveCombo, &QComboBox::currentTextChanged, this, [=](const QString &) { updateSnapSteps(); });
    connect(m_snapRotateCombo, &QComboBox::currentTextChanged, this, [=](const QString &) { updateSnapSteps(); });
    connect(m_snapScaleCombo, &QComboBox::currentTextChanged, this, [=](const QString &) { updateSnapSteps(); });
    updateSnapSteps();

    // Viewport dragging syncs properties
    connect(m_viewport, &GLViewport::objectMoved, this, &ModelingWidget::updatePropertyPanel);
    connect(m_viewport, &GLViewport::objectPicked, this, &ModelingWidget::onObjectPicked);

    auto *frameSelected = new QShortcut(QKeySequence(Qt::Key_F), this);
    connect(frameSelected, &QShortcut::activated, this, &ModelingWidget::onFrameSelected);
    auto *frameAll = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F), this);
    connect(frameAll, &QShortcut::activated, this, &ModelingWidget::onFrameAll);

    auto *dupShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this);
    connect(dupShortcut, &QShortcut::activated, this, &ModelingWidget::duplicateSelectedShortcut);

    auto *dupBlenderShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_D), m_viewport);
    dupBlenderShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(dupBlenderShortcut, &QShortcut::activated, this, &ModelingWidget::duplicateSelectedShortcut);

    auto *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), m_viewport);
    deleteShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteShortcut, &QShortcut::activated, this, &ModelingWidget::deleteSelected);

    auto *snapToggleShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Tab), m_viewport);
    snapToggleShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(snapToggleShortcut, &QShortcut::activated, this, [this]() {
        m_snapCheck->setChecked(!m_snapCheck->isChecked());
    });

    auto *hideSelectedShortcut = new QShortcut(QKeySequence(Qt::Key_H), m_viewport);
    hideSelectedShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(hideSelectedShortcut, &QShortcut::activated, this, &ModelingWidget::hideSelectedObjects);

    auto *isolateShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_H), m_viewport);
    isolateShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(isolateShortcut, &QShortcut::activated, this, &ModelingWidget::isolateSelectedObjects);

    auto *unhideShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_H), m_viewport);
    unhideShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(unhideShortcut, &QShortcut::activated, this, &ModelingWidget::unhideAllObjects);

    auto *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, &ModelingWidget::onUndo);
    auto *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    redoShortcut->setKey(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z));
    connect(redoShortcut, &QShortcut::activated, this, &ModelingWidget::onRedo);
    auto *selectWholeShortcut = new QShortcut(QKeySequence::SelectAll, this);
    connect(selectWholeShortcut, &QShortcut::activated, this, &ModelingWidget::selectWholeObjectFromCurrent);

    connect(m_viewport, &GLViewport::transformStarted, this, &ModelingWidget::pushUndoSnapshot);

    // Keep first-load defaults predictable for embedded flows (e.g. order management).
    m_gridCheck->setChecked(true);
    m_viewport->setShowGrid(true);
    m_viewport->setPresetView(GLViewport::ViewPreset::Perspective);
}

// ─── Add a single primitive ───

void ModelingWidget::addPrimitive(PrimitiveType type)
{
    pushUndoSnapshot();
    static const char *names[] = {"Cube", "Cylinder", "Sphere", "Plane", "Cone", "Pyramid", "Capsule", "Torus"};
    const int groupId = m_nextGroupId++;
    SceneObject obj;
    obj.type = type;
    obj.name = QString("%1_%2").arg(names[int(type)]).arg(m_nextId++);
    if (m_viewport->hasPlacementAnchor()) {
        const QVector3D ground = m_viewport->placementAnchorGround();
        const float lift = (type == PrimitiveType::Plane) ? 0.01f : 0.5f;
        obj.position = ground + QVector3D(0.0f, lift, 0.0f);
    } else {
        obj.position = QVector3D(0, 0.5f, 0);
    }
    obj.color = QColor(200, 160, 100);
    obj.groupId = groupId;
    m_objects.append(obj);
    refreshObjectList();
    int row = rowFromObjectIndex(m_objects.size() - 1);
    if (row >= 0)
        m_objectList->setCurrentRow(row);
    m_viewport->update();
}

void ModelingWidget::deleteSelected()
{
    QList<QListWidgetItem*> sel = m_objectList->selectedItems();
    if (sel.isEmpty()) return;
    pushUndoSnapshot();
    // Collect rows in descending order to remove from back first
    QList<int> rows;
    for (auto *item : sel) {
        int row = m_objectList->row(item);
        int objIndex = objectIndexFromRow(row);
        if (objIndex >= 0)
            rows.append(objIndex);
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int r : rows) {
        if (r >= 0 && r < m_objects.size())
            m_objects.removeAt(r);
    }
    if (!rows.isEmpty()) {
        for (SceneObject &obj : m_objects) {
            if (rows.contains(obj.parentIndex))
                obj.parentIndex = -1;
            else {
                int shift = 0;
                for (int removed : rows) {
                    if (removed < obj.parentIndex)
                        shift++;
                }
                obj.parentIndex = (obj.parentIndex >= 0) ? (obj.parentIndex - shift) : -1;
            }
        }
    }
    m_viewport->setSelectedIndex(-1);
    m_viewport->setSelectedIndices({});
    refreshObjectList();
    m_viewport->update();
}

void ModelingWidget::duplicateSelected()
{
    QList<QListWidgetItem*> sel = m_objectList->selectedItems();
    if (sel.isEmpty()) return;
    pushUndoSnapshot();
    QList<int> rows;
    for (auto *item : sel) {
        int row = m_objectList->row(item);
        int objIndex = objectIndexFromRow(row);
        if (objIndex >= 0)
            rows.append(objIndex);
    }
    std::sort(rows.begin(), rows.end());  // ascending
    QVector3D offset(0.5f, 0, 0.5f);
    for (int i = 0; i < rows.size(); ++i) {
        int r = rows[i];
        if (r < 0 || r >= m_objects.size()) continue;
        SceneObject dup = m_objects[r];
        dup.name = dup.name + "_copy";
        dup.position += offset * (i + 1);
        m_objects.append(dup);
    }
    refreshObjectList();
    int row = rowFromObjectIndex(m_objects.size() - 1);
    if (row >= 0)
        m_objectList->setCurrentRow(row);
    m_viewport->update();
}

void ModelingWidget::clearScene()
{
    if (m_objects.isEmpty()) return;
    if (QMessageBox::question(this, "Clear Scene", "Delete all objects?") != QMessageBox::Yes) return;
    pushUndoSnapshot();
    m_objects.clear();
    m_viewport->setSelectedIndex(-1);
    refreshObjectList();
    m_viewport->update();
}

static auto vec3ToJson(const QVector3D &v) {
    QJsonArray a; a << v.x() << v.y() << v.z(); return a;
}
static QVector3D jsonToVec3(const QJsonArray &a) {
    return { float(a[0].toDouble()), float(a[1].toDouble()), float(a[2].toDouble()) };
}

void ModelingWidget::saveScene()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save Scene"),
        QString(),
        tr("HammerDown Scene (*.hd3d);;JSON (*.json)")
    );
    if (path.isEmpty()) return;

    QJsonArray arr;
    for (const SceneObject &o : m_objects) {
        QJsonObject obj;
        obj["name"]     = o.name;
        obj["type"]     = int(o.type);
        obj["position"] = vec3ToJson(o.position);
        obj["rotation"] = vec3ToJson(o.rotation);
        obj["scale"]    = vec3ToJson(o.scale);
        obj["color"]    = o.color.name();
        obj["visible"]  = o.visible;
        obj["parentIndex"] = o.parentIndex;
        obj["groupId"] = o.groupId;
        arr.append(obj);
    }
    QJsonObject root;
    root["objects"] = arr;
    root["nextId"]  = m_nextId;
    root["nextGroupId"] = m_nextGroupId;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Save Failed"), tr("Could not open file for writing:\n") + path);
        return;
    }
    f.write(QJsonDocument(root).toJson());
}

void ModelingWidget::loadScene()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Load Scene"),
        QString(),
        tr("HammerDown Scene (*.hd3d);;JSON (*.json);;All Files (*)")
    );
    if (path.isEmpty()) return;
    pushUndoSnapshot();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Failed"), tr("Could not open file:\n") + path);
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (doc.isNull()) {
        QMessageBox::warning(this, tr("Load Failed"), tr("Invalid scene file:\n") + err.errorString());
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray arr = root["objects"].toArray();

    m_objects.clear();
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        SceneObject obj;
        obj.name     = o["name"].toString();
        obj.type     = PrimitiveType(o["type"].toInt());
        obj.position = jsonToVec3(o["position"].toArray());
        obj.rotation = jsonToVec3(o["rotation"].toArray());
        obj.scale    = jsonToVec3(o["scale"].toArray());
        obj.color    = QColor(o["color"].toString());
        obj.visible  = o["visible"].toBool(true);
        obj.parentIndex = o.contains("parentIndex") ? o["parentIndex"].toInt(-1) : -1;
        obj.groupId = o.contains("groupId") ? o["groupId"].toInt(-1) : -1;
        m_objects.append(obj);
    }
    m_nextId = root["nextId"].toInt(m_objects.size() + 1);
    m_nextGroupId = root["nextGroupId"].toInt(1);

    m_viewport->setSelectedIndex(-1);
    m_viewport->setSelectedIndices({});
    refreshObjectList();
    if (!m_objects.isEmpty())
        m_objectList->setCurrentRow(0);
    m_viewport->resetCamera();
    m_viewport->update();
}

void ModelingWidget::refreshObjectList()
{
    refreshObjectList(m_objectSearch ? m_objectSearch->text() : QString());
}

void ModelingWidget::refreshObjectList(const QString &filterText)
{
    m_objectList->clear();
    m_filteredIndices.clear();

    QString filter = filterText.trimmed();
    for (int i = 0; i < m_objects.size(); ++i) {
        const SceneObject &obj = m_objects[i];
        if (!filter.isEmpty() && !obj.name.contains(filter, Qt::CaseInsensitive))
            continue;

        int depth = computeDepth(i);
        QString indent(depth * 2, ' ');
        const QString hiddenPrefix = obj.visible ? "" : "[H] ";
        m_objectList->addItem(hiddenPrefix + indent + obj.name);
        m_filteredIndices.append(i);
    }
}

int ModelingWidget::objectIndexFromRow(int row) const
{
    if (row < 0 || row >= m_filteredIndices.size()) return -1;
    return m_filteredIndices[row];
}

int ModelingWidget::rowFromObjectIndex(int objIndex) const
{
    return m_filteredIndices.indexOf(objIndex);
}

int ModelingWidget::computeDepth(int index) const
{
    int depth = 0;
    int current = index;
    QSet<int> visited;
    while (current >= 0 && current < m_objects.size()) {
        if (visited.contains(current)) break;
        visited.insert(current);
        int parent = m_objects[current].parentIndex;
        if (parent < 0) break;
        depth++;
        current = parent;
    }
    return depth;
}

void ModelingWidget::onObjectListClicked(int row)
{
    int objIndex = objectIndexFromRow(row);
    m_viewport->setSelectedIndex(objIndex);
    updatePropertyPanel();
}

void ModelingWidget::onObjectPicked(int index, Qt::KeyboardModifiers mods)
{
    if (index < 0) {
        if (!(mods & (Qt::ControlModifier | Qt::ShiftModifier))) {
            m_objectList->clearSelection();
            m_viewport->setSelectedIndex(-1);
            m_viewport->setSelectedIndices({});
        }
        return;
    }

    int row = rowFromObjectIndex(index);
    if (row < 0) return;

    if (mods & Qt::ShiftModifier) {
        int start = (m_lastClickedRow >= 0) ? m_lastClickedRow : row;
        int minRow = qMin(start, row);
        int maxRow = qMax(start, row);
        m_objectList->clearSelection();
        for (int r = minRow; r <= maxRow; ++r) {
            if (auto *item = m_objectList->item(r))
                item->setSelected(true);
        }
        m_objectList->setCurrentRow(row);
    } else if (mods & Qt::ControlModifier) {
        if (auto *item = m_objectList->item(row))
            item->setSelected(!item->isSelected());
        m_objectList->setCurrentRow(row);
    } else {
        m_objectList->clearSelection();
        m_objectList->setCurrentRow(row);
    }

    m_lastClickedRow = row;
    updatePropertyPanel();
}

void ModelingWidget::onFrameSelected()
{
    m_viewport->frameSelected();
}

void ModelingWidget::onFrameAll()
{
    m_viewport->frameAll();
}

void ModelingWidget::onObjectSearchChanged(const QString &text)
{
    refreshObjectList(text);
}

void ModelingWidget::duplicateSelectedShortcut()
{
    duplicateSelected();
}

QList<int> ModelingWidget::selectedObjectIndices() const
{
    QList<int> raw;
    if (m_viewport)
        raw = m_viewport->selectedIndices();

    if (raw.isEmpty() && m_viewport) {
        const int idx = m_viewport->selectedIndex();
        if (idx >= 0)
            raw.append(idx);
    }

    QList<int> out;
    QSet<int> seen;
    for (int idx : raw) {
        if (idx < 0 || idx >= m_objects.size()) continue;
        if (seen.contains(idx)) continue;
        seen.insert(idx);
        out.append(idx);
    }
    return out;
}

void ModelingWidget::hideSelectedObjects()
{
    const QList<int> targets = selectedObjectIndices();
    if (targets.isEmpty()) return;

    pushUndoSnapshot();
    for (int idx : targets) {
        if (idx >= 0 && idx < m_objects.size())
            m_objects[idx].visible = false;
    }

    m_viewport->setSelectedIndices({});
    m_viewport->setSelectedIndex(-1);
    refreshObjectList();
    m_viewport->update();
}

void ModelingWidget::unhideAllObjects()
{
    bool hasHidden = false;
    for (const SceneObject &obj : m_objects) {
        if (!obj.visible) {
            hasHidden = true;
            break;
        }
    }
    if (!hasHidden) return;

    pushUndoSnapshot();
    for (SceneObject &obj : m_objects)
        obj.visible = true;

    refreshObjectList();
    m_viewport->update();
}

void ModelingWidget::isolateSelectedObjects()
{
    const QList<int> targets = selectedObjectIndices();
    if (targets.isEmpty()) return;

    pushUndoSnapshot();
    QSet<int> keep;
    for (int idx : targets)
        keep.insert(idx);

    for (int i = 0; i < m_objects.size(); ++i)
        m_objects[i].visible = keep.contains(i);

    refreshObjectList();
    m_viewport->setSelectedIndices(targets);
    m_viewport->setSelectedIndex(targets.first());
    updatePropertyPanel();
    m_viewport->update();
}

void ModelingWidget::pushUndoSnapshot()
{
    SceneSnapshot snap;
    snap.objects = m_objects;
    snap.nextId = m_nextId;
    m_undoStack.push_back(snap);
    clearRedoStack();
}

void ModelingWidget::clearRedoStack()
{
    m_redoStack.clear();
}

void ModelingWidget::applySnapshot(const SceneSnapshot &snapshot)
{
    m_objects = snapshot.objects;
    m_nextId = snapshot.nextId;
    refreshObjectList();
    m_viewport->setObjects(&m_objects);
    m_viewport->setSelectedIndex(-1);
    m_viewport->setSelectedIndices({});
    m_viewport->update();
}

void ModelingWidget::onUndo()
{
    if (m_undoStack.isEmpty()) return;
    SceneSnapshot current;
    current.objects = m_objects;
    current.nextId = m_nextId;
    m_redoStack.push_back(current);

    SceneSnapshot snap = m_undoStack.takeLast();
    applySnapshot(snap);
}

void ModelingWidget::onRedo()
{
    if (m_redoStack.isEmpty()) return;
    SceneSnapshot current;
    current.objects = m_objects;
    current.nextId = m_nextId;
    m_undoStack.push_back(current);

    SceneSnapshot snap = m_redoStack.takeLast();
    applySnapshot(snap);
}

void ModelingWidget::selectWholeObjectFromCurrent()
{
    int idx = m_viewport->selectedIndex();
    if ((idx < 0 || idx >= m_objects.size()) && m_objectList->currentRow() >= 0) {
        idx = objectIndexFromRow(m_objectList->currentRow());
    }
    if (idx < 0 || idx >= m_objects.size()) return;

    QList<int> indices;
    const int groupId = m_objects[idx].groupId;
    if (groupId >= 0) {
        for (int i = 0; i < m_objects.size(); ++i) {
            if (m_objects[i].groupId == groupId)
                indices.append(i);
        }
    } else {
        int root = idx;
        while (root >= 0 && root < m_objects.size() && m_objects[root].parentIndex >= 0) {
            root = m_objects[root].parentIndex;
        }

        auto isInRootSubtree = [&](int node) {
            int cur = node;
            QSet<int> visited;
            while (cur >= 0 && cur < m_objects.size()) {
                if (cur == root) return true;
                if (visited.contains(cur)) break;
                visited.insert(cur);
                cur = m_objects[cur].parentIndex;
            }
            return false;
        };

        for (int i = 0; i < m_objects.size(); ++i) {
            if (isInRootSubtree(i))
                indices.append(i);
        }
    }

    if (indices.isEmpty()) {
        indices.append(idx);
    }

    m_objectList->clearSelection();
    for (int objIndex : indices) {
        int row = rowFromObjectIndex(objIndex);
        if (row >= 0) {
            if (auto *item = m_objectList->item(row))
                item->setSelected(true);
        }
    }

    int currentRow = rowFromObjectIndex(idx);
    if (currentRow >= 0)
        m_objectList->setCurrentRow(currentRow);

    m_viewport->setSelectedIndex(idx);
    m_viewport->setSelectedIndices(indices);
    updatePropertyPanel();
}

void ModelingWidget::updatePropertyPanel()
{
    int idx = m_viewport->selectedIndex();
    if (idx < 0 || idx >= m_objects.size()) return;
    m_updatingProps = true;
    const SceneObject &o = m_objects[idx];
    m_posX->setValue(o.position.x()); m_posY->setValue(o.position.y()); m_posZ->setValue(o.position.z());
    m_rotX->setValue(o.rotation.x()); m_rotY->setValue(o.rotation.y()); m_rotZ->setValue(o.rotation.z());
    m_scaleX->setValue(o.scale.x());  m_scaleY->setValue(o.scale.y());  m_scaleZ->setValue(o.scale.z());
    m_visibleCheck->setChecked(o.visible);
    m_colorBtn->setStyleSheet(QString("background: %1; border: 2px solid #8B6F47; border-radius: 6px;").arg(o.color.name()));

    m_parentCombo->blockSignals(true);
    m_parentCombo->clear();
    m_parentCombo->addItem("None", -1);
    for (int i = 0; i < m_objects.size(); ++i) {
        if (i == idx) continue;
        m_parentCombo->addItem(m_objects[i].name, i);
    }
    int parentRow = m_parentCombo->findData(o.parentIndex);
    if (parentRow >= 0)
        m_parentCombo->setCurrentIndex(parentRow);
    m_parentCombo->blockSignals(false);
    // Snapshot current values so onPropertyChanged can compute deltas
    m_prevPropPos   = o.position;
    m_prevPropRot   = o.rotation;
    m_prevPropScale = o.scale;
    m_updatingProps = false;
}

void ModelingWidget::onPropertyChanged()
{
    if (m_updatingProps) return;
    if (!m_pendingPropUndo) {
        pushUndoSnapshot();
        m_pendingPropUndo = true;
        QTimer::singleShot(200, this, [this]() { m_pendingPropUndo = false; });
    }
    int primaryIdx = m_viewport->selectedIndex();
    if (primaryIdx < 0 || primaryIdx >= m_objects.size()) return;

    QVector3D newPos  (float(m_posX->value()),   float(m_posY->value()),   float(m_posZ->value()));
    QVector3D newRot  (float(m_rotX->value()),   float(m_rotY->value()),   float(m_rotZ->value()));
    QVector3D newScale(float(m_scaleX->value()), float(m_scaleY->value()), float(m_scaleZ->value()));
    bool newVisible = m_visibleCheck->isChecked();

    // Delta for position so multi-selected objects move together without stacking
    QVector3D posDelta = newPos - m_prevPropPos;

    QList<int> targets = m_viewport->selectedIndices().isEmpty()
                             ? QList<int>{primaryIdx}
                             : m_viewport->selectedIndices();

    for (int idx : targets) {
        if (idx < 0 || idx >= m_objects.size()) continue;
        SceneObject &o = m_objects[idx];
        o.position += posDelta;           // relative offset preserves spacing
        o.rotation  = newRot;             // absolute: align all to same rotation
        o.scale     = newScale;           // absolute: same scale for all
        o.visible   = newVisible;
    }

    // Update snapshot so next incremental change uses the right base
    m_prevPropPos   = newPos;
    m_prevPropRot   = newRot;
    m_prevPropScale = newScale;

    m_viewport->update();
}

void ModelingWidget::onColorPick()
{
    int primaryIdx = m_viewport->selectedIndex();
    if (primaryIdx < 0 || primaryIdx >= m_objects.size()) return;
    pushUndoSnapshot();
    QColor c = QColorDialog::getColor(m_objects[primaryIdx].color, this, "Pick Object Color");
    if (!c.isValid()) return;

    QList<int> targets = m_viewport->selectedIndices().isEmpty()
                             ? QList<int>{primaryIdx}
                             : m_viewport->selectedIndices();
    for (int idx : targets) {
        if (idx >= 0 && idx < m_objects.size())
            m_objects[idx].color = c;
    }
    m_colorBtn->setStyleSheet(QString("background: %1; border: 2px solid #8B6F47; border-radius: 6px;").arg(c.name()));
    m_viewport->update();
}

void ModelingWidget::onParentChanged(int)
{
    if (m_updatingProps) return;
    int idx = m_viewport->selectedIndex();
    if (idx < 0 || idx >= m_objects.size()) return;

    int newParent = m_parentCombo->currentData().toInt();
    if (newParent == idx)
        return;

    // Prevent cycles
    int cur = newParent;
    QSet<int> visited;
    while (cur >= 0 && cur < m_objects.size()) {
        if (cur == idx || visited.contains(cur)) {
            return;
        }
        visited.insert(cur);
        cur = m_objects[cur].parentIndex;
    }

    pushUndoSnapshot();
    m_objects[idx].parentIndex = newParent;
    refreshObjectList();
    m_viewport->update();
}

// ════════════════════════════════════════════════════════════════════════════
//  Furniture presets — built from combinations of primitives
// ════════════════════════════════════════════════════════════════════════════

void ModelingWidget::loadPreset(const QString &typeName)
{
    auto isFurniture = [&](const QString &t) {
        return t == "Chair" || t == "Table" || t == "Cabinet" || t == "Wardrobe";
    };
    auto isHousePreset = [&](const QString &t) {
        return t == "House" || t == "HouseShell" ||
               t == "HouseSimRoom" || t == "Simulation Room" || t == "House Simulation Room";
    };
    auto sceneHasHouse = [&]() {
        for (const SceneObject &o : m_objects) {
            if (o.name.startsWith("House_")) return true;
        }
        return false;
    };

    pushUndoSnapshot();

    const bool appendFurnitureIntoHouse = isFurniture(typeName) && sceneHasHouse();
    const int oldCount = m_objects.size();
    if (!appendFurnitureIntoHouse) {
        m_objects.clear();
        m_nextId = 1;
    }

    if (typeName == "Chair")         buildPresetChair();
    else if (typeName == "Table")    buildPresetTable();
    else if (typeName == "Cabinet")  buildPresetCabinet();
    else if (typeName == "Wardrobe") buildPresetWardrobe();
    else if (typeName == "House" || typeName == "HouseShell")
        buildPresetHouseShell();
    else if (typeName == "HouseSimRoom" ||
             typeName == "Simulation Room" ||
             typeName == "House Simulation Room")
        buildPresetSimulationRoom();

    if (appendFurnitureIntoHouse) {
        if (m_viewport->hasPlacementAnchor() && oldCount < m_objects.size()) {
            const QVector3D target = m_viewport->placementAnchorGround();
            QVector3D center(0.0f, 0.0f, 0.0f);
            float minY = FLT_MAX;
            int count = 0;
            for (int i = oldCount; i < m_objects.size(); ++i) {
                center += m_objects[i].position;
                minY = qMin(minY, m_objects[i].position.y());
                ++count;
            }
            if (count > 0) {
                center /= float(count);
                const QVector3D offset(target.x() - center.x(), target.y() - minY, target.z() - center.z());
                for (int i = oldCount; i < m_objects.size(); ++i)
                    m_objects[i].position += offset;
            }
        } else {
            const float offsetX = float((QRandomGenerator::global()->generateDouble() * 8.0) - 4.0);
            const float offsetZ = float((QRandomGenerator::global()->generateDouble() * 6.0) - 3.0);
            for (int i = oldCount; i < m_objects.size(); ++i) {
                m_objects[i].position += QVector3D(offsetX, 0.0f, offsetZ);
            }
        }
    }

    refreshObjectList();
    if (!m_objects.isEmpty())
        m_objectList->setCurrentRow(0);

    if (isHousePreset(typeName)) {
        // Preserve the user's current camera and viewport toggles when loading house presets.
        m_viewport->setPlacementAnchor(QVector3D(0.0f, 1.72f, 4.2f));
    } else if (!appendFurnitureIntoHouse) {
        m_viewport->resetCamera();
    }
}

void ModelingWidget::buildPresetHouseShell()
{
    const int groupId = m_nextGroupId++;
    const float roomW = 20.0f;
    const float roomD = 14.0f;
    const float wallH = 5.5f;
    const float wallT = 0.26f;

    const QColor wallA(194, 188, 180);
    const QColor wallB(118, 110, 100);
    const QColor floorA(84, 79, 74);
    const QColor floorB(146, 138, 126);
    const QColor trim(92, 71, 52);
    const QColor glass(172, 210, 226);

    auto addCube = [this, groupId](const QString &name, const QVector3D &pos, const QVector3D &scale, const QColor &color) {
        SceneObject o;
        o.type = PrimitiveType::Cube;
        o.name = QString("House_%1_%2").arg(name).arg(m_nextId++);
        o.position = pos;
        o.scale = scale;
        o.color = color;
        o.groupId = groupId;
        m_objects.append(o);
    };

    addCube("Foundation", {0.0f, -0.3f, 0.0f}, {roomW + 0.8f, 0.6f, roomD + 0.8f}, QColor(64, 61, 56));
    addCube("Ceiling", {0.0f, wallH, 0.0f}, {roomW, 0.18f, roomD}, QColor(222, 216, 206));
    addCube("BackWall", {0.0f, wallH / 2.0f, -roomD / 2.0f}, {roomW, wallH, wallT}, wallA);
    addCube("LeftWall", {-roomW / 2.0f, wallH / 2.0f, 0.0f}, {wallT, wallH, roomD}, wallA);
    addCube("RightWall", {roomW / 2.0f, wallH / 2.0f, 0.0f}, {wallT, wallH, roomD}, wallA);

    const float doorW = 2.4f;
    const float doorH = 3.1f;
    const float sideWallW = (roomW - doorW) * 0.5f;
    addCube("FrontWallL", {-(doorW * 0.5f + sideWallW * 0.5f), wallH * 0.5f, roomD * 0.5f}, {sideWallW, wallH, wallT}, wallA);
    addCube("FrontWallR", {(doorW * 0.5f + sideWallW * 0.5f), wallH * 0.5f, roomD * 0.5f}, {sideWallW, wallH, wallT}, wallA);
    addCube("FrontWallTop", {0.0f, doorH + (wallH - doorH) * 0.5f, roomD * 0.5f}, {doorW, wallH - doorH, wallT}, wallA);
    addCube("DoorHeader", {0.0f, doorH + 0.08f, roomD * 0.5f - 0.03f}, {doorW + 0.24f, 0.10f, 0.10f}, trim);

    const int tilesX = 10;
    const int tilesZ = 7;
    const float tileW = roomW / float(tilesX);
    const float tileD = roomD / float(tilesZ);
    for (int ix = 0; ix < tilesX; ++ix) {
        for (int iz = 0; iz < tilesZ; ++iz) {
            const float x = -roomW * 0.5f + tileW * (ix + 0.5f);
            const float z = -roomD * 0.5f + tileD * (iz + 0.5f);
            const QColor c = ((ix + iz) % 2 == 0) ? floorA : floorB;
            addCube("FloorTile", {x, 0.02f, z}, {tileW - 0.04f, 0.04f, tileD - 0.04f}, c);
        }
    }

    for (int ix = 0; ix < 8; ++ix) {
        const float x = -roomW * 0.42f + ix * ((roomW * 0.84f) / 7.0f);
        const QColor c = (ix % 2 == 0) ? wallB : wallA;
        addCube("BackPanel", {x, 1.9f, -roomD * 0.5f + 0.01f}, {1.7f, 3.4f, 0.04f}, c);
    }

    const float winW = 8.6f;
    const float winH = 2.3f;
    addCube("WindowGlass", {3.9f, 2.7f, -roomD * 0.5f + 0.02f}, {winW, winH, 0.05f}, glass);
    addCube("WindowFrameTop", {3.9f, 2.7f + winH * 0.5f, -roomD * 0.5f + 0.03f}, {winW + 0.18f, 0.08f, 0.10f}, trim);
    addCube("WindowFrameBottom", {3.9f, 2.7f - winH * 0.5f, -roomD * 0.5f + 0.03f}, {winW + 0.18f, 0.08f, 0.10f}, trim);
    for (int i = -4; i <= 4; ++i) {
        addCube("WindowMullion", {3.9f + i * 0.95f, 2.7f, -roomD * 0.5f + 0.035f}, {0.06f, winH - 0.06f, 0.07f}, trim);
    }

    for (int side : {-1, 1}) {
        addCube("ColumnA", {float(side) * (roomW * 0.5f - 1.4f), 2.75f, -1.8f}, {0.45f, 5.5f, 0.45f}, QColor(112, 104, 94));
        addCube("ColumnB", {float(side) * (roomW * 0.5f - 1.4f), 2.75f, 2.0f}, {0.45f, 5.5f, 0.45f}, QColor(112, 104, 94));
    }
}

void ModelingWidget::buildPresetSimulationRoom()
{
    const int groupId = m_nextGroupId++;
    // Full interior room preset intended for immersive object testing.
    const float roomW = 20.0f;
    const float roomD = 14.0f;
    const float wallH = 5.5f;
    const float wallT = 0.26f;

    const QColor wallA(194, 188, 180);
    const QColor wallB(118, 110, 100);
    const QColor floorA(84, 79, 74);
    const QColor floorB(146, 138, 126);
    const QColor trim(92, 71, 52);
    const QColor glass(172, 210, 226);

    auto addCube = [this, groupId](const QString &name, const QVector3D &pos, const QVector3D &scale, const QColor &color) {
        SceneObject o;
        o.type = PrimitiveType::Cube;
        o.name = QString("House_%1_%2").arg(name).arg(m_nextId++);
        o.position = pos;
        o.scale = scale;
        o.color = color;
        o.groupId = groupId;
        m_objects.append(o);
    };

    auto addCylinder = [this, groupId](const QString &name, const QVector3D &pos, const QVector3D &scale, const QColor &color) {
        SceneObject o;
        o.type = PrimitiveType::Cylinder;
        o.name = QString("House_%1_%2").arg(name).arg(m_nextId++);
        o.position = pos;
        o.scale = scale;
        o.color = color;
        o.groupId = groupId;
        m_objects.append(o);
    };

    // Structural shell.
    addCube("Foundation", {0.0f, -0.3f, 0.0f}, {roomW + 0.8f, 0.6f, roomD + 0.8f}, QColor(64, 61, 56));
    addCube("Ceiling", {0.0f, wallH, 0.0f}, {roomW, 0.18f, roomD}, QColor(222, 216, 206));
    addCube("BackWall", {0.0f, wallH / 2.0f, -roomD / 2.0f}, {roomW, wallH, wallT}, wallA);
    addCube("LeftWall", {-roomW / 2.0f, wallH / 2.0f, 0.0f}, {wallT, wallH, roomD}, wallA);
    addCube("RightWall", {roomW / 2.0f, wallH / 2.0f, 0.0f}, {wallT, wallH, roomD}, wallA);

    // Front wall with central opening.
    const float doorW = 2.4f;
    const float doorH = 3.1f;
    const float sideWallW = (roomW - doorW) * 0.5f;
    addCube("FrontWallL", {-(doorW * 0.5f + sideWallW * 0.5f), wallH * 0.5f, roomD * 0.5f}, {sideWallW, wallH, wallT}, wallA);
    addCube("FrontWallR", {(doorW * 0.5f + sideWallW * 0.5f), wallH * 0.5f, roomD * 0.5f}, {sideWallW, wallH, wallT}, wallA);
    addCube("FrontWallTop", {0.0f, doorH + (wallH - doorH) * 0.5f, roomD * 0.5f}, {doorW, wallH - doorH, wallT}, wallA);
    addCube("DoorHeader", {0.0f, doorH + 0.08f, roomD * 0.5f - 0.03f}, {doorW + 0.24f, 0.10f, 0.10f}, trim);

    // Checkered floor tiles.
    const int tilesX = 10;
    const int tilesZ = 7;
    const float tileW = roomW / float(tilesX);
    const float tileD = roomD / float(tilesZ);
    for (int ix = 0; ix < tilesX; ++ix) {
        for (int iz = 0; iz < tilesZ; ++iz) {
            const float x = -roomW * 0.5f + tileW * (ix + 0.5f);
            const float z = -roomD * 0.5f + tileD * (iz + 0.5f);
            const QColor c = ((ix + iz) % 2 == 0) ? floorA : floorB;
            addCube("FloorTile", {x, 0.02f, z}, {tileW - 0.04f, 0.04f, tileD - 0.04f}, c);
        }
    }

    // Accent wall paneling to create a room-like atmosphere.
    for (int ix = 0; ix < 8; ++ix) {
        const float x = -roomW * 0.42f + ix * ((roomW * 0.84f) / 7.0f);
        const QColor c = (ix % 2 == 0) ? wallB : wallA;
        addCube("BackPanel", {x, 1.9f, -roomD * 0.5f + 0.01f}, {1.7f, 3.4f, 0.04f}, c);
    }

    // Large rear window with mullions.
    const float winW = 8.6f;
    const float winH = 2.3f;
    addCube("WindowGlass", {3.9f, 2.7f, -roomD * 0.5f + 0.02f}, {winW, winH, 0.05f}, glass);
    addCube("WindowFrameTop", {3.9f, 2.7f + winH * 0.5f, -roomD * 0.5f + 0.03f}, {winW + 0.18f, 0.08f, 0.10f}, trim);
    addCube("WindowFrameBottom", {3.9f, 2.7f - winH * 0.5f, -roomD * 0.5f + 0.03f}, {winW + 0.18f, 0.08f, 0.10f}, trim);
    for (int i = -4; i <= 4; ++i) {
        addCube("WindowMullion", {3.9f + i * 0.95f, 2.7f, -roomD * 0.5f + 0.035f}, {0.06f, winH - 0.06f, 0.07f}, trim);
    }

    // Support columns.
    for (int side : {-1, 1}) {
        addCube("Column", {float(side) * (roomW * 0.5f - 1.4f), 2.75f, -1.8f}, {0.45f, 5.5f, 0.45f}, QColor(112, 104, 94));
        addCube("Column", {float(side) * (roomW * 0.5f - 1.4f), 2.75f, 2.0f}, {0.45f, 5.5f, 0.45f}, QColor(112, 104, 94));
    }

    // Main test platform and target marker.
    addCube("TestPlatform", {0.0f, 0.20f, 0.0f}, {5.4f, 0.4f, 5.4f}, QColor(137, 105, 70));
    addCube("PlatformTrim", {0.0f, 0.44f, 0.0f}, {5.6f, 0.06f, 5.6f}, QColor(84, 60, 37));
    addCylinder("CenterMarker", {0.0f, 0.72f, 0.0f}, {0.30f, 0.56f, 0.30f}, QColor(212, 158, 85));

    // Demo props in room corners.
    addCube("ShowTableTop", {-5.8f, 1.2f, 3.6f}, {2.2f, 0.18f, 1.4f}, QColor(154, 117, 76));
    addCube("ShowTableLeg", {-6.6f, 0.6f, 3.0f}, {0.16f, 1.2f, 0.16f}, trim);
    addCube("ShowTableLeg", {-5.0f, 0.6f, 3.0f}, {0.16f, 1.2f, 0.16f}, trim);
    addCube("ShowTableLeg", {-6.6f, 0.6f, 4.2f}, {0.16f, 1.2f, 0.16f}, trim);
    addCube("ShowTableLeg", {-5.0f, 0.6f, 4.2f}, {0.16f, 1.2f, 0.16f}, trim);

    addCube("BenchSeat", {6.3f, 0.9f, 3.8f}, {2.4f, 0.2f, 0.9f}, QColor(149, 114, 78));
    addCube("BenchBack", {6.3f, 1.5f, 3.45f}, {2.4f, 1.0f, 0.12f}, QColor(121, 90, 57));
    addCylinder("BenchLeg", {5.4f, 0.45f, 3.5f}, {0.10f, 0.9f, 0.10f}, trim);
    addCylinder("BenchLeg", {7.2f, 0.45f, 3.5f}, {0.10f, 0.9f, 0.10f}, trim);
    addCylinder("BenchLeg", {5.4f, 0.45f, 4.1f}, {0.10f, 0.9f, 0.10f}, trim);
    addCylinder("BenchLeg", {7.2f, 0.45f, 4.1f}, {0.10f, 0.9f, 0.10f}, trim);

    addCube("StorageBody", {-8.3f, 1.6f, -2.8f}, {1.4f, 3.2f, 1.1f}, QColor(132, 98, 64));
    addCube("StorageDoor", {-8.3f, 1.6f, -2.25f}, {1.25f, 3.0f, 0.06f}, QColor(158, 120, 76));
    addCylinder("StorageHandle", {-8.0f, 1.6f, -2.18f}, {0.05f, 0.45f, 0.05f}, QColor(70, 54, 38));

    // Ambient overhead lamps.
    for (int i = -1; i <= 1; ++i) {
        addCylinder("LampStem", {float(i) * 4.0f, 4.8f, -0.5f}, {0.05f, 0.40f, 0.05f}, QColor(60, 60, 62));
        addCube("LampHead", {float(i) * 4.0f, 4.55f, -0.5f}, {0.85f, 0.18f, 0.85f}, QColor(236, 210, 150));
    }
}

void ModelingWidget::buildPresetChair()
{
    const int groupId = m_nextGroupId++;
    QColor wood(139, 90, 43);
    QColor seat(180, 130, 70);

    // Seat
    SceneObject s;
    s.type = PrimitiveType::Cube;
    s.name = QString("Seat_%1").arg(m_nextId++);
    s.position = {0, 1.8f, 0};
    s.scale = {2.0f, 0.2f, 2.0f};
    s.color = seat;
    s.groupId = groupId;
    m_objects.append(s);

    // 4 legs
    QVector3D legPos[4] = {{-0.8f, 0.85f, -0.8f}, {0.8f, 0.85f, -0.8f}, {-0.8f, 0.85f, 0.8f}, {0.8f, 0.85f, 0.8f}};
    for (int i = 0; i < 4; ++i) {
        SceneObject leg;
        leg.type = PrimitiveType::Cylinder;
        leg.name = QString("Leg_%1").arg(m_nextId++);
        leg.position = legPos[i];
        leg.scale = {0.15f, 1.7f, 0.15f};
        leg.color = wood;
        leg.groupId = groupId;
        m_objects.append(leg);
    }

    // Backrest
    SceneObject back;
    back.type = PrimitiveType::Cube;
    back.name = QString("Backrest_%1").arg(m_nextId++);
    back.position = {0, 3.0f, -0.9f};
    back.scale = {2.0f, 2.2f, 0.15f};
    back.color = wood;
    back.groupId = groupId;
    m_objects.append(back);

    // Back support bars
    for (int i = -1; i <= 1; i += 2) {
        SceneObject bar;
        bar.type = PrimitiveType::Cylinder;
        bar.name = QString("BackBar_%1").arg(m_nextId++);
        bar.position = {i * 0.4f, 2.5f, -0.85f};
        bar.scale = {0.08f, 1.4f, 0.08f};
        bar.color = QColor(110, 70, 35);
        bar.groupId = groupId;
        m_objects.append(bar);
    }
}

void ModelingWidget::buildPresetTable()
{
    const int groupId = m_nextGroupId++;
    QColor wood(160, 110, 60);
    QColor darkWood(100, 65, 30);

    // Tabletop
    SceneObject top;
    top.type = PrimitiveType::Cube;
    top.name = QString("Tabletop_%1").arg(m_nextId++);
    top.position = {0, 3.0f, 0};
    top.scale = {4.0f, 0.2f, 2.5f};
    top.color = wood;
    top.groupId = groupId;
    m_objects.append(top);

    // 4 legs
    QVector3D legPos[4] = {{-1.7f, 1.45f, -1.0f}, {1.7f, 1.45f, -1.0f}, {-1.7f, 1.45f, 1.0f}, {1.7f, 1.45f, 1.0f}};
    for (int i = 0; i < 4; ++i) {
        SceneObject leg;
        leg.type = PrimitiveType::Cube;
        leg.name = QString("Leg_%1").arg(m_nextId++);
        leg.position = legPos[i];
        leg.scale = {0.2f, 2.9f, 0.2f};
        leg.color = darkWood;
        leg.groupId = groupId;
        m_objects.append(leg);
    }

}

void ModelingWidget::buildPresetCabinet()
{
    const int groupId = m_nextGroupId++;
    QColor body(130, 95, 50);
    QColor door(160, 120, 70);
    QColor handle(80, 60, 30);

    // Main body
    SceneObject b;
    b.type = PrimitiveType::Cube;
    b.name = QString("Body_%1").arg(m_nextId++);
    b.position = {0, 2.0f, 0};
    b.scale = {3.0f, 3.8f, 1.5f};
    b.color = body;
    b.groupId = groupId;
    m_objects.append(b);

    // Shelves
    for (int i = 0; i < 3; ++i) {
        SceneObject shelf;
        shelf.type = PrimitiveType::Cube;
        shelf.name = QString("Shelf_%1").arg(m_nextId++);
        shelf.position = {0, 0.8f + i * 1.3f, 0};
        shelf.scale = {2.8f, 0.1f, 1.3f};
        shelf.color = QColor(150, 110, 60);
        shelf.groupId = groupId;
        m_objects.append(shelf);
    }

    // Door handles
    for (int side = -1; side <= 1; side += 2) {
        SceneObject h;
        h.type = PrimitiveType::Cylinder;
        h.name = QString("Handle_%1").arg(m_nextId++);
        h.position = {side * 0.3f, 2.0f, 0.78f};
        h.scale = {0.06f, 0.6f, 0.06f};
        h.color = handle;
        h.groupId = groupId;
        m_objects.append(h);
    }

    // Top piece
    SceneObject tp;
    tp.type = PrimitiveType::Cube;
    tp.name = QString("TopPiece_%1").arg(m_nextId++);
    tp.position = {0, 3.95f, 0};
    tp.scale = {3.2f, 0.12f, 1.6f};
    tp.color = QColor(110, 80, 40);
    tp.groupId = groupId;
    m_objects.append(tp);
}

void ModelingWidget::buildPresetWardrobe()
{
    const int groupId = m_nextGroupId++;
    QColor body(120, 85, 45);
    QColor door(140, 100, 55);
    QColor accent(90, 65, 30);

    // Main body
    SceneObject b;
    b.type = PrimitiveType::Cube;
    b.name = QString("Body_%1").arg(m_nextId++);
    b.position = {0, 3.0f, 0};
    b.scale = {3.5f, 6.0f, 1.8f};
    b.color = body;
    b.groupId = groupId;
    m_objects.append(b);

    // Left door
    SceneObject ld;
    ld.type = PrimitiveType::Cube;
    ld.name = QString("LeftDoor_%1").arg(m_nextId++);
    ld.position = {-0.85f, 3.0f, 0.91f};
    ld.scale = {1.7f, 5.8f, 0.08f};
    ld.color = door;
    ld.groupId = groupId;
    m_objects.append(ld);

    // Right door
    SceneObject rd;
    rd.type = PrimitiveType::Cube;
    rd.name = QString("RightDoor_%1").arg(m_nextId++);
    rd.position = {0.85f, 3.0f, 0.91f};
    rd.scale = {1.7f, 5.8f, 0.08f};
    rd.color = door;
    rd.groupId = groupId;
    m_objects.append(rd);

    // Handles
    for (int side = -1; side <= 1; side += 2) {
        SceneObject h;
        h.type = PrimitiveType::Cylinder;
        h.name = QString("Handle_%1").arg(m_nextId++);
        h.position = {side * 0.15f, 3.0f, 0.96f};
        h.scale = {0.06f, 0.8f, 0.06f};
        h.color = accent;
        h.groupId = groupId;
        m_objects.append(h);
    }

    // Crown top
    SceneObject crown;
    crown.type = PrimitiveType::Cube;
    crown.name = QString("Crown_%1").arg(m_nextId++);
    crown.position = {0, 6.05f, 0};
    crown.scale = {3.7f, 0.15f, 1.9f};
    crown.color = accent;
    crown.groupId = groupId;
    m_objects.append(crown);

    // Base
    SceneObject base;
    base.type = PrimitiveType::Cube;
    base.name = QString("Base_%1").arg(m_nextId++);
    base.position = {0, 0.08f, 0};
    base.scale = {3.6f, 0.15f, 1.85f};
    base.color = accent;
    base.groupId = groupId;
    m_objects.append(base);

    // Feet
    for (int x = -1; x <= 1; x += 2) {
        for (int z = -1; z <= 1; z += 2) {
            SceneObject foot;
            foot.type = PrimitiveType::Sphere;
            foot.name = QString("Foot_%1").arg(m_nextId++);
            foot.position = {x * 1.5f, -0.05f, z * 0.7f};
            foot.scale = {0.2f, 0.15f, 0.2f};
            foot.color = accent;
            foot.groupId = groupId;
            m_objects.append(foot);
        }
    }
}
