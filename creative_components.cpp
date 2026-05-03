#include "creative_components.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QSequentialAnimationGroup>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QScrollBar>

// ============================================================================
// ACTION CARD MENU (formerly Radial Menu) — Premium animated card
// ============================================================================
RadialCommandMenu::RadialCommandMenu(QWidget *parent) : QFrame(parent) {
    setFixedSize(240, 280);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    hide();

    m_actions << Action{"STATS", "View Insights", QColor(76, 175, 80)}
              << Action{"EDIT", "Modify Item", QColor(212, 175, 55)}
              << Action{"NEXUS", "Go to Nexus", QColor(64, 196, 255)}
              << Action{"DELETE", "Remove Item", QColor(211, 47, 47)};
}

void RadialCommandMenu::showMenu(const QPoint &globalPos, int equipmentId) {
    m_activeId = equipmentId;
    if (parentWidget()) {
        // Moved slightly left (-30) and slightly down (+60)
        move(parentWidget()->width() - width() - 30, 60);
    } else {
        move(globalPos);
    }
    show();
    raise();
    setFocus();
    animateOpen();
}

void RadialCommandMenu::animateOpen() {
    auto *group = new QParallelAnimationGroup(this);

    // Slide from right
    QPropertyAnimation *slideAnim = new QPropertyAnimation(this, "pos");
    slideAnim->setDuration(420);
    slideAnim->setEasingCurve(QEasingCurve::OutExpo);
    if(parentWidget()) {
        slideAnim->setStartValue(QPoint(parentWidget()->width() + 20, 60));
        slideAnim->setEndValue(QPoint(parentWidget()->width() - width() - 30, 60));
    }
    group->addAnimation(slideAnim);

    // Opacity fade-in via graphics effect
    auto *opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnim->setDuration(350);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::InQuad);
    group->addAnimation(fadeAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [this]() {
        setGraphicsEffect(nullptr); // Remove effect to avoid rendering overhead
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void RadialCommandMenu::animateClose() {
    auto *opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);

    auto *group = new QParallelAnimationGroup(this);

    QPropertyAnimation *slideAnim = new QPropertyAnimation(this, "pos");
    slideAnim->setDuration(300);
    slideAnim->setEasingCurve(QEasingCurve::InBack);
    if(parentWidget()) {
        slideAnim->setStartValue(pos());
        slideAnim->setEndValue(QPoint(parentWidget()->width() + 20, pos().y()));
    }
    group->addAnimation(slideAnim);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnim->setDuration(280);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutQuad);
    group->addAnimation(fadeAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [this]() {
        setGraphicsEffect(nullptr);
        hide();
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

QRect RadialCommandMenu::getItemRect(int i) const {
    int startY = 70;
    int h = 44;
    int gap = 6;
    return QRect(14, startY + i * (h + gap), width() - 28, h);
}

void RadialCommandMenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QRect r = rect().adjusted(2, 2, -2, -2);
    
    // Drop shadow simulation
    for (int i = 6; i > 0; --i) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 8 * i));
        p.drawRoundedRect(r.adjusted(-i, -i, i, i), 14, 14);
    }

    // Base card — dark frosted glass
    QLinearGradient cardBg(r.topLeft(), r.bottomRight());
    cardBg.setColorAt(0.0, QColor(22, 18, 14, 248));
    cardBg.setColorAt(1.0, QColor(10, 8, 6, 252));
    p.setBrush(cardBg);
    p.setPen(QPen(QColor(193, 127, 62, 80), 1.5));
    p.drawRoundedRect(r, 14, 14);

    // Top accent line
    QLinearGradient accentGrad(r.left() + 20, r.top(), r.right() - 20, r.top());
    accentGrad.setColorAt(0.0, Qt::transparent);
    accentGrad.setColorAt(0.3, QColor(212, 175, 55, 200));
    accentGrad.setColorAt(0.7, QColor(193, 127, 62, 200));
    accentGrad.setColorAt(1.0, Qt::transparent);
    p.setPen(Qt::NoPen);
    p.setBrush(accentGrad);
    p.drawRect(QRectF(r.left() + 20, r.top() + 2, r.width() - 40, 2.5));

    // Header text
    p.setPen(QColor(245, 230, 211));
    p.setFont(QFont("Outfit", 13, QFont::Bold));
    p.drawText(QRect(16, 16, width() - 32, 22), Qt::AlignLeft | Qt::AlignVCenter, QString("ITEM #%1").arg(m_activeId));

    // Subtle separator
    QLinearGradient sepGrad(16, 0, width() - 16, 0);
    sepGrad.setColorAt(0.0, Qt::transparent);
    sepGrad.setColorAt(0.5, QColor(193, 127, 62, 60));
    sepGrad.setColorAt(1.0, Qt::transparent);
    p.setPen(QPen(sepGrad, 1));
    p.drawLine(16, 48, width() - 16, 48);

    // Quick actions subtitle
    p.setPen(QColor(193, 127, 62, 120));
    p.setFont(QFont("Segoe UI", 8, QFont::DemiBold));
    p.drawText(QRect(16, 52, width() - 32, 14), Qt::AlignLeft, "QUICK ACTIONS");

    // Draw action buttons — NO description text, just clean name + indicator
    for (int i = 0; i < m_actions.size(); ++i) {
        QRect itemBox = getItemRect(i);
        QColor col = m_actions[i].color;
        
        bool hover = (i == m_hoverIdx);
        
        if (hover) {
            // Glow behind the button
            QRadialGradient glow(itemBox.center(), itemBox.width() * 0.6);
            glow.setColorAt(0.0, QColor(col.red(), col.green(), col.blue(), 25));
            glow.setColorAt(1.0, Qt::transparent);
            p.setPen(Qt::NoPen);
            p.setBrush(glow);
            p.drawRoundedRect(itemBox.adjusted(-8, -4, 8, 4), 12, 12);

            // Hover card background
            QLinearGradient hoverBg(itemBox.topLeft(), itemBox.bottomRight());
            hoverBg.setColorAt(0.0, QColor(col.red(), col.green(), col.blue(), 50));
            hoverBg.setColorAt(1.0, QColor(col.red(), col.green(), col.blue(), 20));
            p.setBrush(hoverBg);
            p.setPen(QPen(col, 1.5));
            p.drawRoundedRect(itemBox, 8, 8);
        } else {
            // Normal state
            p.setBrush(QColor(25, 20, 16, 200));
            p.setPen(QPen(QColor(60, 48, 35), 1));
            p.drawRoundedRect(itemBox, 8, 8);
        }
        
        // Color indicator dot
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        int dotSize = hover ? 12 : 10;
        p.drawEllipse(QPoint(itemBox.left() + 18, itemBox.center().y()), dotSize / 2, dotSize / 2);

        // Glow around dot on hover
        if (hover) {
            QRadialGradient dotGlow(QPointF(itemBox.left() + 18, itemBox.center().y()), 14);
            dotGlow.setColorAt(0.0, QColor(col.red(), col.green(), col.blue(), 60));
            dotGlow.setColorAt(1.0, Qt::transparent);
            p.setBrush(dotGlow);
            p.drawEllipse(QPoint(itemBox.left() + 18, itemBox.center().y()), 14, 14);
        }

        // Action name — single line, clean
        p.setPen(hover ? Qt::white : QColor(210, 200, 190));
        p.setFont(QFont("Outfit", 11, hover ? QFont::Bold : QFont::DemiBold));
        p.drawText(itemBox.adjusted(34, 0, -14, 0), Qt::AlignLeft | Qt::AlignVCenter, m_actions[i].name);

        // Arrow indicator on hover
        if (hover) {
            p.setPen(QPen(col, 2, Qt::SolidLine, Qt::RoundCap));
            int arrowX = itemBox.right() - 16;
            int arrowY = itemBox.center().y();
            p.drawLine(arrowX - 6, arrowY - 4, arrowX, arrowY);
            p.drawLine(arrowX - 6, arrowY + 4, arrowX, arrowY);
        }
    }

    // Footer close hint
    p.setPen(QColor(120, 100, 80, 100));
    p.setFont(QFont("Segoe UI", 7));
    p.drawText(QRect(0, height() - 22, width(), 16), Qt::AlignCenter, "ESC TO CLOSE");
}

void RadialCommandMenu::mouseMoveEvent(QMouseEvent *e) {
    QPoint pos = e->pos();
    int oldIdx = m_hoverIdx;
    m_hoverIdx = -1;
    for (int i = 0; i < m_actions.size(); ++i) {
        if (getItemRect(i).contains(pos)) {
            m_hoverIdx = i;
            break;
        }
    }
    if (oldIdx != m_hoverIdx) {
        setCursor(m_hoverIdx >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}

void RadialCommandMenu::mousePressEvent(QMouseEvent *e) {
    if (m_hoverIdx != -1 && e->button() == Qt::LeftButton) {
        emit actionSelected(m_actions[m_hoverIdx].name, m_activeId);
        animateClose();
    } else if (e->button() == Qt::LeftButton) {
        animateClose();
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
    Q_UNUSED(e);
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
// ============================================================================
// CHAT BUBBLE IMPLEMENTATION
// ============================================================================
ChatBubble::ChatBubble(const QString &msg, const QString &time, bool isMe, const QString &senderName, int index, QWidget *parent)
    : QWidget(parent), m_message(msg), m_time(time), m_sender(senderName), m_isMe(isMe), m_index(index) {
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ChatBubble::enterEvent(QEnterEvent *event) {
    Q_UNUSED(event);
    m_hover = true;
    update();
}

void ChatBubble::setImage(const QByteArray &data) {
    m_image.loadFromData(data);
    if (!m_image.isNull()) {
        m_hasImage = true;
        m_image = m_image.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    updateGeometry();
    update();
}

void ChatBubble::setGif(const QString &url) {
    if (url.isEmpty()) return;
    m_gifUrl = url;
    m_hasGif = true;

    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    QNetworkReply *reply = mgr->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QBuffer *buffer = new QBuffer(this);
            buffer->setData(data);
            buffer->open(QIODevice::ReadOnly);

            m_gifMovie = new QMovie(buffer, QByteArray(), this);
            if (m_gifMovie->isValid()) {
                m_gifMovie->setScaledSize(QSize(200, 150));
                connect(m_gifMovie, &QMovie::frameChanged, this, [this](){ update(); });
                m_gifMovie->start();
                updateGeometry();
                emit gif_loaded();
            }
        }
        reply->deleteLater();
    });
    updateGeometry();
}

void ChatBubble::setVoiceNote(bool isVoice) {
    m_isVoiceNote = isVoice;
    updateGeometry();
    update();
}

void ChatBubble::animateEntrance() {
    auto *group = new QParallelAnimationGroup(this);
    
    auto *fade = new QPropertyAnimation(this, "opacity");
    fade->setDuration(450);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutQuad);
    
    auto *slide = new QPropertyAnimation(this, "offset");
    slide->setDuration(550);
    slide->setStartValue(25.0);
    slide->setEndValue(0.0);
    slide->setEasingCurve(QEasingCurve::OutBack);
    
    group->addAnimation(fade);
    group->addAnimation(slide);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void ChatBubble::mousePressEvent(QMouseEvent *e) {
    if (m_isMe && m_hover && m_deleteRect.contains(e->pos())) {
        emit deleteRequested(m_index);
    }
}

void ChatBubble::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(m_opacity);
    p.translate(0, m_offset);

    // Dynamic sizes based on content
    QFont msgFont("Segoe UI", 10);
    QFontMetrics fm(msgFont);
    int maxW = width() * 0.75;
    if (maxW < 200) maxW = 350;
    
    QRect textRect = fm.boundingRect(0, 0, maxW - 30, 2000, Qt::TextWordWrap, m_message);
    
    int contentH = textRect.height();
    int contentW = textRect.width();
    
    if (m_hasImage) { contentH += 210; contentW = qMax(contentW, 200); }
    if (m_hasGif) { contentH += 160; contentW = qMax(contentW, 200); }
    if (m_isVoiceNote) { contentH += 40; contentW = qMax(contentW, 180); }

    int bubbleW = contentW + 30;
    int bubbleH = contentH + 40;
    
    int x = m_isMe ? (width() - bubbleW - 10) : 10;
    QRect bubbleRect(x, 5, bubbleW, bubbleH);

    // Glassmorphism Bubble
    QColor baseCol = m_isMe ? QColor(45, 35, 25, 220) : QColor(25, 30, 35, 220);
    if (m_hover) baseCol = baseCol.lighter(130);
    
    p.setPen(QPen(m_isMe ? QColor(212, 175, 55, 100) : QColor(59, 130, 246, 100), 1.2));
    p.setBrush(baseCol);
    p.drawRoundedRect(bubbleRect, 16, 16);

    // Hover Shiny Glint
    if (m_hover) {
        qreal glintX = std::fmod(QDateTime::currentMSecsSinceEpoch() * 0.3, bubbleRect.width() * 4.0) - bubbleRect.width();
        QLinearGradient g(bubbleRect.left() + glintX, 0, bubbleRect.left() + glintX + 50, 0);
        g.setColorAt(0, Qt::transparent);
        g.setColorAt(0.5, QColor(255, 255, 255, 30));
        g.setColorAt(1, Qt::transparent);
        p.fillRect(bubbleRect, g);
        
        if (m_isMe) {
            m_deleteRect = QRect(bubbleRect.right() - 25, bubbleRect.top() + 5, 20, 20);
            p.setPen(QColor(255, 100, 100, 200));
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(m_deleteRect, Qt::AlignCenter, "×");
        }
    } else {
        m_deleteRect = QRect();
    }

    // Sender & Time
    p.setPen(QColor(180, 170, 160, 180));
    p.setFont(QFont("Outfit", 7, QFont::Bold));
    p.drawText(bubbleRect.adjusted(12, 8, -12, 0), Qt::AlignLeft | Qt::AlignTop, m_sender.toUpper());
    p.drawText(bubbleRect.adjusted(12, 8, -12, 0), Qt::AlignRight | Qt::AlignTop, m_time);

    int currentY = bubbleRect.top() + 25;

    // Render Media Content
    if (m_hasImage) {
        p.drawPixmap(bubbleRect.left() + 15, currentY, m_image);
        currentY += 210;
    }
    
    if (m_hasGif && m_gifMovie && m_gifMovie->isValid()) {
        p.drawPixmap(bubbleRect.left() + 15, currentY, m_gifMovie->currentPixmap());
        currentY += 160;
    }

    if (m_isVoiceNote) {
        // Draw Waveform
        p.setPen(QPen(QColor(212, 175, 55, 180), 1.5));
        for(int i=0; i<20; ++i) {
            int h = 5 + qAbs(qSin(QDateTime::currentMSecsSinceEpoch()*0.01 + i)*15);
            p.drawLine(bubbleRect.left() + 15 + i*8, currentY + 15 - h/2, bubbleRect.left() + 15 + i*8, currentY + 15 + h/2);
        }
        p.setPen(QColor(212, 175, 55));
        p.setFont(QFont("Outfit", 8, QFont::Bold));
        p.drawText(bubbleRect.left() + 15, currentY + 32, "VOICE MESSAGE - 0:08");
        currentY += 45;
    }

    // Message Text (if not special placeholder)
    if (!m_message.isEmpty() && m_message != "[Image]" && m_message != "[GIF]" && !m_message.contains("Voice Note")) {
        p.setPen(Qt::white);
        p.setFont(msgFont);
        p.drawText(bubbleRect.left() + 15, currentY, bubbleRect.width() - 30, 2000, Qt::TextWordWrap, m_message);
    }
}

QSize ChatBubble::sizeHint() const {
    QFont msgFont("Segoe UI", 10);
    QFontMetrics fm(msgFont);
    int maxW = 400;
    QRect textRect = fm.boundingRect(0, 0, maxW - 30, 2000, Qt::TextWordWrap, m_message);
    
    int h = textRect.height();
    if (m_hasImage) h += 210;
    if (m_hasGif) h += 170; // Increased
    if (m_isVoiceNote) h += 50; // Increased
    
    return QSize(qMax(240, width()), h + 80); // Increased padding
}

// ============================================================================
// CHAT EMPLOYEE DELEGATE IMPLEMENTATION
// ============================================================================
void ChatEmployeeDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    QRect r = option.rect.adjusted(6, 4, -6, -4);
    bool selected = option.state & QStyle::State_Selected;
    bool hover = option.state & QStyle::State_MouseOver;

    // Background Card
    QColor bg = selected ? QColor(60, 50, 40, 240) : (hover ? QColor(40, 35, 30, 200) : QColor(25, 22, 20, 180));
    p->setPen(QPen(selected ? QColor(212, 175, 55, 150) : QColor(193, 127, 62, 40), 1));
    p->setBrush(bg);
    p->drawRoundedRect(r, 12, 12);

    if (selected) {
        // Golden accent line
        p->setBrush(QColor(212, 175, 55));
        p->setPen(Qt::NoPen);
        p->drawRect(r.left(), r.top() + 10, 3, r.height() - 20);
    }

    // Text Content
    QString fullText = index.data().toString();
    QString name = fullText.section(" (ID:", 0, 0);
    QString id = "ID: " + fullText.section("(ID: ", 1, 1).replace(")", "");
    QString title = index.data(Qt::ToolTipRole).toString();

    p->setPen(selected ? Qt::white : QColor(220, 210, 200));
    p->setFont(QFont("Outfit", 10, QFont::Bold));
    p->drawText(r.adjusted(15, 8, -10, -25), Qt::AlignLeft | Qt::AlignTop, name.toUpper());

    p->setPen(QColor(193, 127, 62, 180));
    p->setFont(QFont("Consolas", 8));
    p->drawText(r.adjusted(15, 28, -10, -5), Qt::AlignLeft | Qt::AlignTop, id);

    p->setPen(QColor(150, 140, 130, 150));
    p->setFont(QFont("Segoe UI", 7, QFont::DemiBold));
    p->drawText(r.adjusted(15, 0, -10, -8), Qt::AlignLeft | Qt::AlignBottom, title.toUpper());

    p->restore();
}

QSize ChatEmployeeDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
    return QSize(200, 70);
}
