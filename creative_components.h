#ifndef CREATIVE_COMPONENTS_H
#define CREATIVE_COMPONENTS_H

#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QEasingCurve>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <QKeyEvent>

// ============================================================================
// 1. RADIAL COMMAND WHEEL — Game-like circular menu
// ============================================================================
class RadialCommandMenu : public QWidget {
    Q_OBJECT
public:
    explicit RadialCommandMenu(QWidget *parent = nullptr);
    void showMenu(const QPoint &globalPos, int equipmentId);

signals:
    void actionSelected(const QString &action, int id);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override { m_hoverIdx = -1; update(); }
    void hideEvent(QHideEvent *) override { m_hoverIdx = -1; }
    void keyPressEvent(QKeyEvent *e) override { if(e->key() == Qt::Key_Escape) hide(); }

private:
    struct Action { QString name; QString icon; QColor color; };
    QList<Action> m_actions;
    int m_hoverIdx = -1;
    int m_activeId = -1;
    float m_animProgress = 0.0f;
};

// ============================================================================
// 2. DIGITAL TWIN IDENTITY CARD — Premium side-panel slide-out
// ============================================================================
class IdentityCard : public QFrame {
    Q_OBJECT
public:
    explicit IdentityCard(QWidget *parent = nullptr);
    void setup(const QString &name, const QString &type, const QString &status, double price, int id);
    void animateOpen();
    void animateClose();

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    void drawBlueprint(QPainter &p, const QRect &r);
    void drawHealthGauge(QPainter &p, const QRect &r, int score);
    void drawQRCode(QPainter &p, const QRect &r, int id);

    QString m_name, m_type, m_status;
    double m_price;
    int m_id;
    int m_healthScore = 85;
    float m_scanLineY = 0.0f;
    QTimer *m_scanTimer;
};

#endif // CREATIVE_COMPONENTS_H
