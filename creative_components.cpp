#include "creative_components.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QDateTime>

// ============================================================================
// RADIAL COMMAND WHEEL IMPLEMENTATION
// ============================================================================
RadialCommandMenu::RadialCommandMenu(QWidget *parent) : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(280, 280);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // Using explicit hex icons/clean text to avoid UTF-8 issues
    m_actions << Action{"EDIT", "EDIT", QColor(212, 175, 55)}
              << Action{"STATS", "DATA", QColor(76, 175, 80)}
              << Action{"DELETE", "TRASH", QColor(211, 47, 47)};
}

void RadialCommandMenu::showMenu(const QPoint &globalPos, int equipmentId) {
    m_activeId = equipmentId;
    move(globalPos - QPoint(140, 140));
    m_animProgress = 0.0f;
    show();
    setFocus();
    
    QPropertyAnimation *anim = new QPropertyAnimation(this, ""); 
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    connect(anim, &QPropertyAnimation::valueChanged, this, [this](const QVariant &v){
        m_animProgress = v.toFloat();
        update();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RadialCommandMenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QPointF center(140, 140);
    double innerR = 50 * m_animProgress;
    double outerR = 120 * m_animProgress;
    
    // Draw Central Hub
    p.setPen(QPen(QColor(212, 175, 55, 150), 2));
    p.setBrush(QColor(25, 20, 15, 240));
    p.drawEllipse(center, innerR-5, innerR-5);
    
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 9, QFont::Bold));
    p.drawText(QRectF(center.x()-innerR, center.y()-innerR, innerR*2, innerR*2), Qt::AlignCenter, "#" + QString::number(m_activeId));

    int count = m_actions.size();
    for (int i = 0; i < count; ++i) {
        // Correcting angles to avoid overlap
        double sectorSize = 360.0 / count;
        double startAngle = i * sectorSize;
        double spanAngle = sectorSize - 4; // 4 degree gap
        
        QPainterPath path;
        // Outer arc (CW)
        path.arcTo(QRectF(center.x()-outerR, center.y()-outerR, outerR*2, outerR*2), startAngle, spanAngle);
        // Inner arc (CCW)
        path.arcTo(QRectF(center.x()-innerR, center.y()-innerR, innerR*2, innerR*2), startAngle + spanAngle, -spanAngle);
        path.closeSubpath();
        
        QColor col = m_actions[i].color;
        if (i == m_hoverIdx) {
            col = col.lighter(130);
            // Shadow/Glow
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(m_actions[i].color.red(), m_actions[i].color.green(), m_actions[i].color.blue(), 40));
            p.drawEllipse(center, outerR+5, outerR+5);
        } else {
            col.setAlpha(160);
        }
        
        p.setPen(QPen(m_actions[i].color, 2));
        p.setBrush(col);
        p.drawPath(path);
        
        // Draw Text/Label
        double midAngle = qDegreesToRadians(startAngle + spanAngle/2.0);
        double labelR = (innerR + outerR) / 2.0;
        QPointF labelPos(center.x() + labelR * cos(midAngle), center.y() - labelR * sin(midAngle));
        
        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 8, QFont::Black));
        p.drawText(QRectF(labelPos.x()-30, labelPos.y()-15, 60, 30), Qt::AlignCenter, m_actions[i].name);
    }
}

void RadialCommandMenu::mouseMoveEvent(QMouseEvent *e) {
    QPointF center(140, 140);
    QPointF delta = e->position() - center;
    double dist = qSqrt(delta.x()*delta.x() + delta.y()*delta.y());
    
    if (dist < 45 || dist > 130) { m_hoverIdx = -1; }
    else {
        double angle = qAtan2(-delta.y(), delta.x());
        double deg = qRadiansToDegrees(angle);
        if (deg < 0) deg += 360;
        m_hoverIdx = int(deg / (360.0 / m_actions.size()));
    }
    update();
}

void RadialCommandMenu::mousePressEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    if (m_hoverIdx != -1) {
        emit actionSelected(m_actions[m_hoverIdx].name, m_activeId);
        hide();
    } else {
        hide();
    }
}

// ============================================================================
// IDENTITY CARD IMPLEMENTATION
// ============================================================================
IdentityCard::IdentityCard(QWidget *parent) : QFrame(parent) {
    setFixedSize(400, 580);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("background: transparent;");
    
    m_scanTimer = new QTimer(this);
    connect(m_scanTimer, &QTimer::timeout, this, [this](){
        m_scanLineY += 0.015f;
        if (m_scanLineY > 1.0f) m_scanLineY = 0.0f;
        update();
    });
    
    auto *eff = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(eff);
}

void IdentityCard::setup(const QString &name, const QString &type, const QString &status, double price, int id) {
    m_name = name; m_type = type; m_status = status; m_price = price; m_id = id;
    m_healthScore = 65 + QRandomGenerator::global()->bounded(35);
    update();
}

void IdentityCard::animateOpen() {
    show();
    raise();
    setFocus();
    m_scanTimer->start(25);
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(600);
    anim->setStartValue(QPoint(parentWidget()->width(), 20));
    anim->setEndValue(QPoint(parentWidget()->width() - width() - 20, 20));
    anim->setEasingCurve(QEasingCurve::OutExpo);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void IdentityCard::animateClose() {
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(400);
    anim->setStartValue(pos());
    anim->setEndValue(QPoint(parentWidget()->width(), 20));
    anim->setEasingCurve(QEasingCurve::InExpo);
    connect(anim, &QPropertyAnimation::finished, this, &IdentityCard::hide);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    m_scanTimer->stop();
}

void IdentityCard::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) {
        animateClose();
    }
}

void IdentityCard::mousePressEvent(QMouseEvent *e) {
    // Close on any click outside the main rounded rect if needed, 
    // but here we close on any click to fulfill "Click anywhere to dismiss"
    animateClose();
}

void IdentityCard::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QRect r = rect().adjusted(15, 15, -15, -15);
    
    // Backdrop Glow
    QRadialGradient glow(r.center(), r.width());
    glow.setColorAt(0, QColor(212, 175, 55, 30));
    glow.setColorAt(1, Qt::transparent);
    p.fillRect(rect(), glow);

    // Background glass (Frosted)
    QPainterPath bg; bg.addRoundedRect(r, 24, 24);
    p.fillPath(bg, QColor(20, 18, 16, 245));
    p.setPen(QPen(QColor(212, 175, 55, 120), 2));
    p.drawPath(bg);
    
    // Top Bar Decoration
    p.setBrush(QColor(212, 175, 55, 200));
    p.drawRect(r.x()+40, r.y(), 120, 4);

    // Title Section
    p.setPen(QColor(212, 175, 55));
    p.setFont(QFont("Segoe UI", 18, QFont::Black));
    p.drawText(r.adjusted(30, 30, -30, 0), Qt::AlignTop | Qt::AlignLeft, m_type.toUpper());
    
    p.setFont(QFont("Segoe UI Semibold", 10));
    p.setPen(QColor(150, 140, 130));
    p.drawText(r.adjusted(32, 65, -30, 0), Qt::AlignTop | Qt::AlignLeft, "NEXUS CERTIFIED UNIT // " + QString::number(m_id).rightJustified(6, '0'));

    // Blueprint / Wireframe Section
    drawBlueprint(p, QRect(r.x() + 30, r.y() + 100, r.width() - 60, 200));
    
    // Bottom Section Grid
    int baseY = r.y() + 320;
    
    // Health Score
    drawHealthGauge(p, QRect(r.x() + 30, baseY, 120, 120), m_healthScore);
    
    // Data Fields
    p.setPen(QColor(180, 170, 160));
    p.setFont(QFont("Segoe UI", 8, QFont::Bold));
    QStringList labels = {"STATUS", "VALUATION", "MAINTENANCE", "LOCATION"};
    QStringList values = {m_status.toUpper(), QString::number(m_price) + " DT", "OPTIMAL", "WAREHOUSE A"};
    
    for(int i=0; i<labels.size(); ++i) {
        p.setPen(QColor(212, 175, 55, 180));
        p.drawText(r.x() + 170, baseY + 10 + (i*32), labels[i]);
        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI Semibold", 10));
        p.drawText(r.x() + 170, baseY + 26 + (i*32), values[i]);
        p.setFont(QFont("Segoe UI", 8, QFont::Bold));
    }

    // High-Density QR Code
    drawQRCode(p, QRect(r.right() - 110, r.bottom() - 110, 80, 80), m_id);
    
    // Footer
    p.setPen(QColor(80, 80, 80));
    p.setFont(QFont("Segoe UI", 8));
    p.drawText(r.adjusted(0, 0, 0, -15), Qt::AlignBottom | Qt::AlignCenter, "PRESS ESC OR CLICK TO DISMISS");
    
    // Close Icon (X)
    p.setPen(QPen(QColor(212, 175, 55, 150), 2));
    p.drawLine(r.right()-25, r.top()+20, r.right()-15, r.top()+30);
    p.drawLine(r.right()-15, r.top()+20, r.right()-25, r.top()+30);
}

void IdentityCard::drawBlueprint(QPainter &p, const QRect &r) {
    p.setPen(QPen(QColor(33, 150, 243, 40), 1));
    for (int i = 0; i <= r.width(); i += 25) p.drawLine(r.x() + i, r.y(), r.x() + i, r.bottom());
    for (int i = 0; i <= r.height(); i += 25) p.drawLine(r.x(), r.y() + i, r.right(), r.y() + i);

    // Decorative Coordinates
    p.setPen(QColor(33, 150, 243, 150));
    p.setFont(QFont("Consolas", 7));
    p.drawText(r.adjusted(5, 5, 0, 0), "X: 42.091");
    p.drawText(r.adjusted(5, 15, 0, 0), "Y: 18.223");

    p.setPen(QPen(QColor(212, 175, 55, 220), 1.2));
    QRandomGenerator gen(m_id);
    // Draw more complex technical wireframe
    QPointF c = r.center();
    for (int i = 0; i < 4; ++i) {
        QRectF box(c.x()-40, c.y()-60, 80, 120);
        p.drawRect(box.adjusted(gen.bounded(10), gen.bounded(10), -gen.bounded(10), -gen.bounded(10)));
        p.drawLine(box.topLeft(), box.bottomRight());
    }
    
    // Animating Scan Line
    int sy = r.y() + (r.height() * m_scanLineY);
    p.setPen(QPen(QColor(212, 175, 55, 255), 2));
    p.drawLine(r.x(), sy, r.right(), sy);
    
    QLinearGradient g(0, sy-30, 0, sy);
    g.setColorAt(0, Qt::transparent); g.setColorAt(1, QColor(212, 175, 55, 60));
    p.fillRect(r.x(), sy-30, r.width(), 30, g);
}

void IdentityCard::drawHealthGauge(QPainter &p, const QRect &r, int score) {
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 10));
    p.drawEllipse(r);
    
    QColor c = score > 80 ? QColor(76, 175, 80) : (score > 45 ? QColor(212, 175, 55) : QColor(211, 47, 47));
    
    // Outer Ring
    p.setPen(QPen(QColor(255, 255, 255, 30), 4));
    p.drawEllipse(r.adjusted(5, 5, -5, -5));
    
    // Score Arc
    p.setPen(QPen(c, 6, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(r.adjusted(5, 5, -5, -5), 90 * 16, - (score * 3.6) * 16);
    
    p.setPen(Qt::white);
    p.setFont(QFont("Segoe UI", 20, QFont::Black));
    p.drawText(r, Qt::AlignCenter, QString::number(score));
    
    p.setFont(QFont("Segoe UI", 7, QFont::Bold));
    p.setPen(c);
    p.drawText(r.adjusted(0, 45, 0, 0), Qt::AlignCenter, "INTEGRITY");
}

void IdentityCard::drawQRCode(QPainter &p, const QRect &r, int id) {
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(r, Qt::white);
    
    QRandomGenerator gen(id);
    int res = 16; // 16x16 grid for higher density
    double step = (double)r.width() / res;
    
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    
    for (int x = 0; x < res; ++x) {
        for (int y = 0; y < res; ++y) {
            // Static corners for "real" QR look
            bool isCorner = (x < 3 && y < 3) || (x > res-4 && y < 3) || (x < 3 && y > res-4);
            if (isCorner) {
                if (x==0 || x==2 || y==0 || y==2 || (x==res-1 || x==res-3) || (y==res-1 || y==res-3))
                    p.drawRect(QRectF(r.x() + x*step, r.y() + y*step, step, step));
            } else if (gen.bounded(100) > 45) {
                p.drawRect(QRectF(r.x() + x*step, r.y() + y*step, step, step));
            }
        }
    }
}
