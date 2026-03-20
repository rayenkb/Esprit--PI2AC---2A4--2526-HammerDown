#ifndef MODELINGWIDGET_H
#define MODELINGWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#ifdef Q_OS_WIN
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QTimer>
#include <QElapsedTimer>
#include <QColor>
#include <QList>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QColorDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QOpenGLShaderProgram>
#include <QShortcut>

// ─── Primitive types ───
enum class PrimitiveType { Cube, Cylinder, Sphere, Plane, Cone, Pyramid };

// ─── A single 3D object in the scene ───
struct SceneObject {
    PrimitiveType type = PrimitiveType::Cube;
    QVector3D position{0, 0, 0};
    QVector3D rotation{0, 0, 0};
    QVector3D scale{1, 1, 1};
    QVector3D pivot{0, 0, 0};
    QColor color{200, 160, 100};
    QString name;
    bool visible = true;
    bool selected = false;
    int parentIndex = -1;
};

// ─── OpenGL viewport ───
class GLViewport : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLViewport(QWidget *parent = nullptr);

    enum class ViewPreset { Top, Bottom, Left, Right, Front, Back, Perspective };
    enum class ShadingMode { Solid, Wireframe, SolidWire, Unlit };
    enum class GizmoAxis { None, AxisX, AxisY, AxisZ };
    enum class GizmoMode { None, Move, Rotate, Scale, Pivot };

    void setObjects(QList<SceneObject> *objs) { m_objects = objs; }
    void setSelectedIndex(int idx) { m_selectedIdx = idx; update(); }
    int selectedIndex() const { return m_selectedIdx; }
    void setSelectedIndices(const QList<int> &indices) { m_selectedIndices = indices; update(); }
    const QList<int> &selectedIndices() const { return m_selectedIndices; }
    void resetCamera();
    void setPresetView(ViewPreset preset);
    void frameSelected();
    void frameAll();

    // Tool mode
    enum Tool { Select, Move, Rotate, Scale, Pivot };
    void setTool(Tool t) { m_tool = t; }
    void setShowGrid(bool v) { m_showGrid = v; update(); }
    void setShowWireframe(bool v) { m_shadingMode = v ? ShadingMode::Wireframe : ShadingMode::Solid; update(); }
    void setShowBounds(bool v) { m_showBounds = v; update(); }
    void setShadingMode(ShadingMode mode) { m_shadingMode = mode; update(); }
    void setSnapEnabled(bool v) { m_snapEnabled = v; }
    void setSnapSteps(float moveStep, float rotateStep, float scaleStep) {
        m_snapMove = moveStep; m_snapRotate = rotateStep; m_snapScale = scaleStep;
    }

signals:
    void objectMoved();
    void objectClicked(int index);
    void objectPicked(int index, Qt::KeyboardModifiers mods);
    void transformStarted();
    void transformFinished();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    QMatrix4x4 computeProjectionMatrix() const;
    QMatrix4x4 computeViewMatrix() const;
    void buildWorldMatrices();
    void computeSceneBounds();
    void computeSelectionBounds(QVector3D &outMin, QVector3D &outMax, bool &outValid) const;
    void computeVisibleBounds(QVector3D &outMin, QVector3D &outMax, bool &outValid) const;
    void computeObjectBounds(int index, QVector3D &outMin, QVector3D &outMax) const;
    QVector3D computeWorldPivot(int index) const;
    bool pickObject(const QPoint &pos, int &outIndex);
    GizmoAxis pickGizmoAxis(const QPoint &pos, GizmoMode mode) const;
    QVector3D screenDeltaToWorldAxis(const QPoint &delta, GizmoAxis axis) const;
    void applyGizmoDrag(const QPoint &pos);
    void drawGizmo();
    void drawSceneBounds();
    void drawStatsOverlay();
    void drawOrientationCube();
    bool pickOrientationCube(const QPoint &pos, ViewPreset &outPreset) const;
    float computeGridStep() const;
    void updateInertia();
    void applyProjectionMatrix();
    void startCameraAnimation(float targetYaw,
                              float targetPitch,
                              float targetDist,
                              const QVector3D &targetCamTarget,
                              float targetProjectionBlend);
    void updateCameraAnimation();

    void drawGrid();
    void drawObject(int index, const SceneObject &obj, bool highlight);
    void drawCube();
    void drawSphere(int slices, int stacks);
    void drawCylinder(int slices);
    void drawCone(int slices);
    void drawPyramid();
    void drawPlane();

    QList<SceneObject> *m_objects = nullptr;
    int m_selectedIdx = -1;
    QList<int> m_selectedIndices;

    // Camera
    float m_camDist = 12.0f;
    float m_camYaw = 30.0f;
    float m_camPitch = 25.0f;
    QVector3D m_camTarget{0, 0, 0};
    float m_projectionBlend = 0.0f; // 0 = perspective, 1 = orthographic

    QTimer m_camAnimTimer;
    QElapsedTimer m_camAnimElapsed;
    int m_camAnimDurationMs = 500;

    float m_animStartYaw = 0.0f;
    float m_animStartPitch = 0.0f;
    float m_animStartDist = 12.0f;
    QVector3D m_animStartTarget{0, 0, 0};
    float m_animStartProjectionBlend = 0.0f;

    float m_animEndYaw = 0.0f;
    float m_animEndPitch = 0.0f;
    float m_animEndDist = 12.0f;
    QVector3D m_animEndTarget{0, 0, 0};
    float m_animEndProjectionBlend = 0.0f;

    // Mouse
    QPoint m_lastMouse;
    bool m_rotating = false;
    bool m_panning = false;
    bool m_dragging = false;
    Tool m_tool = Select;
    GizmoMode m_gizmoMode = GizmoMode::None;
    GizmoAxis m_activeGizmoAxis = GizmoAxis::None;
    bool m_gizmoDragging = false;
    QPoint m_gizmoDragStart;
    QList<QVector3D> m_startPositions;
    QList<QVector3D> m_startRotations;
    QList<QVector3D> m_startScales;
    QList<QVector3D> m_startPivots;

    QTimer m_inertiaTimer;
    QVector2D m_orbitVelocity{0.0f, 0.0f};
    float m_orbitDamping = 0.90f;

    bool m_snapEnabled = false;
    float m_snapMove = 0.5f;
    float m_snapRotate = 15.0f;
    float m_snapScale = 0.1f;

    bool m_showGrid = true;
    bool m_wireframe = false;
    bool m_showBounds = false;
    ShadingMode m_shadingMode = ShadingMode::Solid;

    QVector<QMatrix4x4> m_worldMatrices;
    QVector3D m_sceneBoundsMin{0, 0, 0};
    QVector3D m_sceneBoundsMax{0, 0, 0};
    bool m_hasSceneBounds = false;

    QElapsedTimer m_fpsTimer;
    int m_frameCount = 0;
    float m_fps = 0.0f;
};

// ─── Full 3D Modeler panel ───
class ModelingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ModelingWidget(QWidget *parent = nullptr);

    void loadPreset(const QString &typeName);

signals:
    void closed();

private slots:
    void addPrimitive(PrimitiveType type);
    void deleteSelected();
    void duplicateSelected();
    void duplicateSelectedShortcut();
    void onObjectListClicked(int row);
    void onObjectPicked(int index, Qt::KeyboardModifiers mods);
    void onFrameSelected();
    void onFrameAll();
    void updatePropertyPanel();
    void onPropertyChanged();
    void onColorPick();
    void onParentChanged(int index);
    void clearScene();
    void saveScene();
    void loadScene();
    void onObjectSearchChanged(const QString &text);
    void onGenerateEnvironment();
    void onAiReplyFinished(QNetworkReply *reply);
    void onUndo();
    void onRedo();

private:
    struct SceneSnapshot {
        QList<SceneObject> objects;
        int nextId = 1;
    };

    void refreshObjectList();
    void refreshObjectList(const QString &filterText);
    void applySnapshot(const SceneSnapshot &snapshot);
    void pushUndoSnapshot();
    void clearRedoStack();
    int objectIndexFromRow(int row) const;
    int rowFromObjectIndex(int objIndex) const;
    int computeDepth(int index) const;
    void buildPresetChair();
    void buildPresetTable();
    void buildPresetCabinet();
    void buildPresetWardrobe();

    GLViewport *m_viewport;
    QList<SceneObject> m_objects;
    int m_nextId = 1;

    // Generate environment panel
    QWidget *m_generatePanel = nullptr;
    QLineEdit *m_generateInput = nullptr;
    QLabel *m_genStatusLabel = nullptr;
    QNetworkAccessManager *m_genNetworkManager = nullptr;

    // Side panel widgets
    QListWidget *m_objectList;
    QLineEdit *m_objectSearch;
    QComboBox *m_addCombo;

    // Properties
    QDoubleSpinBox *m_posX, *m_posY, *m_posZ;
    QDoubleSpinBox *m_rotX, *m_rotY, *m_rotZ;
    QDoubleSpinBox *m_scaleX, *m_scaleY, *m_scaleZ;
    QComboBox *m_parentCombo;
    QPushButton *m_colorBtn;
    QCheckBox *m_visibleCheck;

    // Toolbar
    QPushButton *m_btnSelect, *m_btnMove, *m_btnRotate, *m_btnScale;
    QPushButton *m_btnPivot = nullptr;
    QCheckBox *m_gridCheck, *m_wireCheck = nullptr;
    QCheckBox *m_boundsCheck = nullptr;
    QComboBox *m_shadingCombo = nullptr;
    QCheckBox *m_snapCheck = nullptr;
    QComboBox *m_snapMoveCombo = nullptr;
    QComboBox *m_snapRotateCombo = nullptr;
    QComboBox *m_snapScaleCombo = nullptr;

    // Previous property values (used to compute position delta for multi-select)
    QVector3D m_prevPropPos, m_prevPropRot, m_prevPropScale;

    bool m_updatingProps = false;
    bool m_pendingPropUndo = false;

    QList<int> m_filteredIndices;
    int m_lastClickedRow = -1;

    QVector<SceneSnapshot> m_undoStack;
    QVector<SceneSnapshot> m_redoStack;
};

#endif // MODELINGWIDGET_H
