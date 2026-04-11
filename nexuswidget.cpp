#include "nexuswidget.h"
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtMath>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <algorithm>

// ============================================================================
// HELPER: Load equipment from DB
// ============================================================================
// HELPER: Load equipment from DB
// ============================================================================
QList<NexusEquipment> loadAllEquipment() {
    QList<NexusEquipment> list;
    // Keep this query aligned with the actual EQUIPMENT schema used by MainWindow CRUD.
    QSqlQuery q("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, QUANTITY, UNIT_PRICE, STATUS, DESCRIPTION, "
                "EMPLOYEE_ID, PURCHASE_DATE, LOCATION, NEXT_MAINTENANCE, "
                "COUT_ACQUISITION, RESPONSABLE FROM EQUIPMENT "
                "WHERE STATUS != 'Retired' ORDER BY EQUIPMENT_ID");
    if (!q.isActive()) {
        qDebug() << "NEXUS loadAllEquipment SQL error:" << q.lastError().text();
        return list;
    }
    while (q.next()) {
        NexusEquipment e;
        e.id = q.value(0).toInt();
        e.type = q.value(1).toString();
        e.quantity = q.value(2).toInt();
        e.unitPrice = q.value(3).toDouble();
        e.status = q.value(4).toString();
        e.description = q.value(5).toString();
        e.employeeId = q.value(6).toInt();
        e.purchaseDate = q.value(7).toDate();
        e.location = q.value(8).toString();
        e.notes = QString();
        e.nextMaintenance = q.value(9).toDate();
        e.coutAcquisition = q.value(10).toDouble();
        e.responsable = q.value(11).toString();
        list.append(e);
    }
    return list;
}

QList<NexusEmployee> loadAllEmployees() {
    QList<NexusEmployee> list;
    QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME, JOB_TITLE FROM EMPLOYEES ORDER BY EMPLOYEE_ID");
    while (q.next()) {
        NexusEmployee e;
        e.id = q.value(0).toInt();
        e.firstName = q.value(1).toString();
        e.lastName = q.value(2).toString();
        e.jobTitle = q.value(3).toString();
        list.append(e);
    }
    return list;
}

// ============================================================================
// KNOWLEDGE GRAPH WIDGET
// ============================================================================
KnowledgeGraphWidget::KnowledgeGraphWidget(QWidget *parent) : QWidget(parent),
    m_hoveredNode(-1), m_draggedNode(-1), m_frameTime(0)
{
    setMouseTracking(true);
    setMinimumHeight(450); // Provide enough vertical space to breathe
    
    // Heartbeat timer ONLY for pulse, NOT for movement
    m_physicsTimer = new QTimer(this);
    connect(m_physicsTimer, &QTimer::timeout, this, [this](){
        for(auto &n : m_nodes) n.glowPhase += 0.05;
        update();
    });
}

QColor KnowledgeGraphWidget::statusColor(const QString &status) {
    if (status == "Available") return QColor(76, 175, 80);
    if (status == "In Use") return QColor(33, 150, 243);
    if (status == "Under Maintenance") return QColor(255, 152, 0);
    if (status == "Retired") return QColor(158, 158, 158);
    return QColor(212, 175, 55);
}

void KnowledgeGraphWidget::loadData() {
    m_equipment = loadAllEquipment();
    m_employees = loadAllEmployees();
    buildGraph();
    computeHiddenConnections();
    
    // Run physics 300 times INSTANTLY to find fixed positions
    for(int i = 0; i < 300; i++) {
        stepPhysics();
    }
    
    // Stop all velocity to ensure it's "FIXED"
    for(auto &n : m_nodes) n.vx = n.vy = 0;
    
    m_physicsTimer->start(30); // Start pulse animation
}

void KnowledgeGraphWidget::buildGraph() {
    m_nodes.clear();
    m_edges.clear();

    // Equipment nodes
    for (const auto &eq : m_equipment) {
        GraphNode n;
        n.id = eq.id;
        n.label = eq.type;
        n.isEquipment = true;
        n.status = eq.status;
        n.price = eq.unitPrice;
        n.radius = qBound(20.0, eq.unitPrice / 50.0, 60.0);
        if (n.radius < 20) n.radius = 25;
        n.color = statusColor(eq.status);
        
        // SPREAD: Place nodes across the entire page initially (10% to 90% range)
        n.x = (width() * 0.1) + QRandomGenerator::global()->generateDouble() * (width() * 0.8);
        n.y = (height() * 0.1) + QRandomGenerator::global()->generateDouble() * (height() * 0.8);
        
        n.vx = n.vy = 0;
        n.opacity = 1.0;
        n.glowPhase = QRandomGenerator::global()->bounded(100) / 100.0 * 6.28;
        n.isDragging = false;
        n.isGhost = false;
        n.specialtyAura = "";

        // Ghost: check if no maintenance and status available for 90+ days
        if (eq.status == "Available" && eq.purchaseDate.isValid()) {
            if (eq.purchaseDate.daysTo(QDate::currentDate()) > 90 && !eq.nextMaintenance.isValid()) {
                n.isGhost = true;
                n.opacity = 0.35;
            }
        }
        m_nodes.append(n);
    }

    // Employee nodes
    for (const auto &emp : m_employees) {
        GraphNode n;
        n.id = 10000 + emp.id; // offset to avoid ID conflicts
        n.label = emp.firstName + " " + emp.lastName;
        n.isEquipment = false;
        n.status = "";
        n.price = 0;
        n.radius = 28;
        n.color = QColor(212, 175, 55); // amber
        
        // SPREAD: Place nodes across the entire page initially
        n.x = (width() * 0.1) + QRandomGenerator::global()->generateDouble() * (width() * 0.8);
        n.y = (height() * 0.1) + QRandomGenerator::global()->generateDouble() * (height() * 0.8);
        
        n.vx = n.vy = 0;
        n.opacity = 1.0;
        n.glowPhase = QRandomGenerator::global()->bounded(100) / 100.0 * 6.28;
        n.isDragging = false;
        n.isGhost = false;
        n.specialtyAura = "";
        m_nodes.append(n);
    }

    // Equipment-Employee edges (by EMPLOYEE_ID or RESPONSABLE)
    for (const auto &eq : m_equipment) {
        if (eq.employeeId > 0) {
            GraphEdge edge;
            edge.fromId = eq.id;
            edge.toId = 10000 + eq.employeeId;
            edge.weight = 2;
            edge.color = QColor(212, 175, 55, 120);
            edge.isDashed = false;
            edge.label = "";
            m_edges.append(edge);
        }
    }

    // Equipment-Equipment edges: same type = potential relationship
    for (int i = 0; i < m_equipment.size(); ++i) {
        for (int j = i + 1; j < m_equipment.size(); ++j) {
            if (m_equipment[i].type == m_equipment[j].type) {
                GraphEdge edge;
                edge.fromId = m_equipment[i].id;
                edge.toId = m_equipment[j].id;
                edge.weight = 1;
                edge.color = QColor(139, 111, 71, 80);
                edge.isDashed = false;
                edge.label = "";
                m_edges.append(edge);
            }
        }
    }
}

void KnowledgeGraphWidget::computeHiddenConnections() {
    // Operational dependency: equipment sharing same employee
    QMap<int, QList<int>> empToEquip;
    for (const auto &eq : m_equipment) {
        if (eq.employeeId > 0) empToEquip[eq.employeeId].append(eq.id);
    }
    for (auto it = empToEquip.begin(); it != empToEquip.end(); ++it) {
        if (it.value().size() >= 2) {
            for (int i = 0; i < it.value().size(); ++i) {
                for (int j = i + 1; j < it.value().size(); ++j) {
                    GraphEdge edge;
                    edge.fromId = it.value()[i];
                    edge.toId = it.value()[j];
                    edge.weight = 3;
                    edge.color = QColor(255, 215, 0, 150);
                    edge.isDashed = true;
                    edge.label = "OP. DEPENDENCY";
                    m_edges.append(edge);
                }
            }
        }
    }

    // Employee specialty: count types per employee
    QMap<int, QMap<QString, int>> empTypeCounts;
    for (const auto &eq : m_equipment) {
        if (eq.employeeId > 0) empTypeCounts[eq.employeeId][eq.type]++;
    }
    for (auto it = empTypeCounts.begin(); it != empTypeCounts.end(); ++it) {
        int total = 0;
        QString maxType;
        int maxCount = 0;
        for (auto jt = it.value().begin(); jt != it.value().end(); ++jt) {
            total += jt.value();
            if (jt.value() > maxCount) { maxCount = jt.value(); maxType = jt.key(); }
        }
        if (total > 0 && (double)maxCount / total >= 0.8) {
            for (auto &node : m_nodes) {
                if (node.id == 10000 + it.key()) {
                    node.specialtyAura = maxType;
                    break;
                }
            }
        }
    }
}

void KnowledgeGraphWidget::stepPhysics() {
    const qreal repulsion = 150000.0; // Increased repulsion for better initial spread
    const qreal attraction = 0.0008;  // Minimal attraction
    const qreal damping = 0.94;       // Slightly more fluid movement
    const qreal wallRepulsion = 25.0; // Stronger push away from edges
    const qreal collisionForce = 800.0; // Stronger force to avoid ANY overlap
    
    qreal width_v = qMax(800.0, (qreal)width());
    qreal height_v = qMax(600.0, (qreal)height());

    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].isDragging) continue;
        qreal fx = 0, fy = 0;

        // Repulsion from others
        for (int j = 0; j < m_nodes.size(); ++j) {
            if (i == j) continue;
            qreal dx = m_nodes[i].x - m_nodes[j].x;
            qreal dy = m_nodes[i].y - m_nodes[j].y;
            qreal distSq = dx * dx + dy * dy;
            qreal dist = qSqrt(qMax(5.0, distSq));
            
            // Extreme push for close proximity
            qreal minDist = m_nodes[i].radius + m_nodes[j].radius + 80; // Larger safety margin
            if (dist < minDist) {
                qreal push = (minDist - dist) * collisionForce;
                fx += (dx / dist) * push;
                fy += (dy / dist) * push;
            } else if (dist < 1000) {
                fx += (dx / dist) * (repulsion / distSq);
                fy += (dy / dist) * (repulsion / distSq);
            }
        }

        // Attraction from edges
        for (const auto &edge : m_edges) {
            int otherIdx = -1;
            int otherId = (edge.fromId == m_nodes[i].id) ? edge.toId : (edge.toId == m_nodes[i].id ? edge.fromId : -1);
            if (otherId == -1) continue;
            for(int k=0; k<m_nodes.size(); k++) if(m_nodes[k].id == otherId) { otherIdx = k; break; }
            if (otherIdx == -1) continue;
            
            qreal dx = m_nodes[otherIdx].x - m_nodes[i].x;
            qreal dy = m_nodes[otherIdx].y - m_nodes[i].y;
            fx += dx * attraction;
            fy += dy * attraction;
        }

        // UNIFORM PAGE COVERAGE: Edge Repulsion (Shifted L-T-B for framing)
        // Adjusting X boundaries to be slightly more to the left
        if (m_nodes[i].x < 20) fx += wallRepulsion; 
        if (m_nodes[i].x > width_v - 220) fx -= (wallRepulsion * 1.5); 
        if (m_nodes[i].y < 40) fy += wallRepulsion; 
        if (m_nodes[i].y > height_v - 260) fy -= (wallRepulsion * 2.0); // Stronger push up to clear UI

        m_nodes[i].vx = (m_nodes[i].vx + fx) * damping;
        m_nodes[i].vy = (m_nodes[i].vy + fy) * damping;

        // Apply Speed limit for stability
        qreal speed = qSqrt(m_nodes[i].vx*m_nodes[i].vx + m_nodes[i].vy*m_nodes[i].vy);
        if(speed > 15.0) { m_nodes[i].vx *= (15.0/speed); m_nodes[i].vy *= (15.0/speed); }
        
        m_nodes[i].x += m_nodes[i].vx;
        m_nodes[i].y += m_nodes[i].vy;

        // Strict hard bounds (Elevated floor)
        m_nodes[i].x = qBound(30.0, m_nodes[i].x, width_v - 150.0);
        m_nodes[i].y = qBound(30.0, m_nodes[i].y, height_v - 130.0);
    }
}

int KnowledgeGraphWidget::nodeAt(QPointF pos) {
    for (int i = m_nodes.size() - 1; i >= 0; --i) {
        qreal dx = pos.x() - m_nodes[i].x;
        qreal dy = pos.y() - m_nodes[i].y;
        if (qSqrt(dx * dx + dy * dy) <= m_nodes[i].radius + 5)
            return i;
    }
    return -1;
}

void KnowledgeGraphWidget::highlightEquipment(int equipId) {
    m_hoveredNode = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].id == equipId) {
            m_hoveredNode = i;
            // Center the view on this node
            break;
        }
    }
    update();
}

void KnowledgeGraphWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(20, 15, 8));
    bg.setColorAt(1, QColor(35, 25, 15));
    p.fillRect(rect(), bg);

    // Grid dots
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(139, 111, 71, 25));
    for (int x = 0; x < width(); x += 30) {
        for (int y = 0; y < height(); y += 30) {
            p.drawEllipse(QPointF(x, y), 1, 1);
        }
    }

    // Determine connected nodes for hover dimming
    QSet<int> connectedIds;
    if (m_hoveredNode >= 0 && m_hoveredNode < m_nodes.size()) {
        int hovId = m_nodes[m_hoveredNode].id;
        connectedIds.insert(hovId);
        for (const auto &e : m_edges) {
            if (e.fromId == hovId) connectedIds.insert(e.toId);
            if (e.toId == hovId) connectedIds.insert(e.fromId);
        }
    }

    // Draw edges
    for (const auto &edge : m_edges) {
        // Only draw relevant edges at higher opacity
        bool isRelatedToHover = false;
        if (m_hoveredNode >= 0) {
            int hovId = m_nodes[m_hoveredNode].id;
            if (edge.fromId == hovId || edge.toId == hovId) isRelatedToHover = true;
        }

        qreal edgeOp = isRelatedToHover ? 0.8 : 0.15; // Dim most edges to reduce yellow clutter
        p.setOpacity(edgeOp);
        drawEdge(p, edge);
    }
    p.setOpacity(1.0);

    // Draw nodes
    for (int i = 0; i < m_nodes.size(); ++i) {
        bool connected = (m_hoveredNode < 0) || connectedIds.contains(m_nodes[i].id);
        drawNode(p, m_nodes[i], connected);
    }

    // Hover card
    if (m_hoveredNode >= 0 && m_hoveredNode < m_nodes.size()) {
        drawHoverCard(p, m_nodes[m_hoveredNode]);
    }
}

void KnowledgeGraphWidget::drawEdge(QPainter &p, const GraphEdge &edge) {
    QPointF from, to;
    bool foundFrom = false, foundTo = false;
    for (const auto &n : m_nodes) {
        if (n.id == edge.fromId) { from = QPointF(n.x, n.y); foundFrom = true; }
        if (n.id == edge.toId) { to = QPointF(n.x, n.y); foundTo = true; }
    }
    if (!foundFrom || !foundTo) return;

    QPen pen(edge.color, qBound(1, edge.weight, 4));
    if (edge.isDashed) pen.setStyle(Qt::DashLine);
    p.setPen(pen);
    p.drawLine(from, to);

    if (!edge.label.isEmpty()) {
        QPointF mid = (from + to) / 2.0;
        p.setPen(QColor(255, 215, 0, 200));
        QFont f = p.font();
        f.setPixelSize(9);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(mid.x() - 60, mid.y() - 8, 120, 16), Qt::AlignCenter, edge.label);
    }
}

void KnowledgeGraphWidget::drawNode(QPainter &p, const GraphNode &node, bool isConnected) {
    p.save();
    qreal op = isConnected ? node.opacity : 0.25 * node.opacity;
    p.setOpacity(op);

    QPointF center(node.x, node.y);
    
    // Heartbeat Pulse Effect: Scale radius based on glowPhase
    qreal pulse = 1.0 + 0.08 * qAbs(qSin(node.glowPhase));
    qreal r = node.radius * pulse;

    // Glow pulse (Ambient light)
    qreal glowSize = r + 10 + 5 * qSin(node.glowPhase * 0.5);
    QRadialGradient glow(center, glowSize);
    glow.setColorAt(0, QColor(node.color.red(), node.color.green(), node.color.blue(), 60));
    glow.setColorAt(1, QColor(node.color.red(), node.color.green(), node.color.blue(), 0));
    p.setPen(Qt::NoPen);
    p.setBrush(glow);
    p.drawEllipse(center, glowSize, glowSize);

    if (node.isEquipment) {
        // Circle for equipment
        QRadialGradient grad(center, r);
        grad.setColorAt(0, node.color.lighter(130));
        grad.setColorAt(0.7, node.color);
        grad.setColorAt(1, node.color.darker(130));
        p.setBrush(grad);

        if (node.isGhost) {
            QPen ghostPen(node.color, 2, Qt::DotLine);
            p.setPen(ghostPen);
        } else {
            p.setPen(QPen(node.color.darker(150), 2));
        }
        p.drawEllipse(center, r, r);
    } else {
        // Diamond for employee
        QPainterPath diamond;
        diamond.moveTo(center.x(), center.y() - r);
        diamond.lineTo(center.x() + r * 0.7, center.y());
        diamond.lineTo(center.x(), center.y() + r);
        diamond.lineTo(center.x() - r * 0.7, center.y());
        diamond.closeSubpath();

        QRadialGradient grad(center, r);
        grad.setColorAt(0, QColor(255, 223, 100));
        grad.setColorAt(1, QColor(212, 175, 55));
        p.setBrush(grad);
        p.setPen(QPen(QColor(139, 111, 71), 2));
        p.drawPath(diamond);

        // Specialty aura
        if (!node.specialtyAura.isEmpty()) {
            QRadialGradient aura(center, r + 12);
            aura.setColorAt(0.6, QColor(255, 165, 0, 40));
            aura.setColorAt(1, QColor(255, 165, 0, 0));
            p.setBrush(aura);
            p.setPen(Qt::NoPen);
            p.drawEllipse(center, r + 12, r + 12);
        }
    }

    // Label
    QFont f = p.font();
    f.setPixelSize(qMax(9, (int)(r / 3)));
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::white);
    QString shortLabel = node.label.length() > 12 ? node.label.left(10) + ".." : node.label;
    p.drawText(QRectF(center.x() - r, center.y() - r, r * 2, r * 2), Qt::AlignCenter | Qt::TextWordWrap, shortLabel);

    p.restore();
}

void KnowledgeGraphWidget::drawHoverCard(QPainter &p, const GraphNode &node) {
    QRectF card(node.x + node.radius + 10, node.y - 50, 200, 90);
    if (card.right() > width()) card.moveLeft(node.x - node.radius - 210);
    if (card.bottom() > height()) card.moveTop(height() - 95);

    p.setPen(QPen(QColor(212, 175, 55), 2));
    p.setBrush(QColor(44, 36, 24, 230));
    p.drawRoundedRect(card, 10, 10);

    QFont f = p.font();
    f.setPixelSize(13); f.setBold(true);
    p.setFont(f);
    p.setPen(QColor(212, 175, 55));
    p.drawText(card.adjusted(10, 8, -10, 0), Qt::AlignTop | Qt::AlignLeft, node.label);

    f.setPixelSize(11); f.setBold(false);
    p.setFont(f);
    p.setPen(Qt::white);
    QString detail;
    if (node.isEquipment) {
        detail = QString("Status: %1\nPrice: %2 dt").arg(node.status).arg(node.price, 0, 'f', 2);
    } else {
        detail = "Employee Node";
    }
    p.drawText(card.adjusted(10, 30, -10, -5), Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, detail);
}

void KnowledgeGraphWidget::mousePressEvent(QMouseEvent *event) {
    int idx = nodeAt(event->position());
    if (idx >= 0) {
        m_draggedNode = idx;
        m_nodes[idx].isDragging = true;
        m_lastMouse = event->position();
    }
}

void KnowledgeGraphWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_draggedNode >= 0) {
        m_nodes[m_draggedNode].x += event->position().x() - m_lastMouse.x();
        m_nodes[m_draggedNode].y += event->position().y() - m_lastMouse.y();
        m_lastMouse = event->position();
    } else {
        int oldHover = m_hoveredNode;
        m_hoveredNode = nodeAt(event->position());
        if (oldHover != m_hoveredNode) update();
    }
}

void KnowledgeGraphWidget::mouseReleaseEvent(QMouseEvent *) {
    if (m_draggedNode >= 0) {
        m_nodes[m_draggedNode].isDragging = false;
        m_nodes[m_draggedNode].vx = m_nodes[m_draggedNode].vy = 0;
        m_draggedNode = -1;
    }
}

// ============================================================================
// TIME MACHINE WIDGET
// ============================================================================
TimeMachineWidget::TimeMachineWidget(QWidget *parent) : QWidget(parent), m_isPlaying(false) {
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(15, 15, 15, 15);
    mainLay->setSpacing(10);

    // Title
    QLabel *title = new QLabel("TIME MACHINE", this);
    title->setStyleSheet("color: #D4AF37; font-size: 20px; font-weight: bold; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(title);

    // Date label
    m_dateLabel = new QLabel("Workshop on [date]", this);
    m_dateLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold; background: transparent;");
    m_dateLabel->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(m_dateLabel);

    // Slider
    QHBoxLayout *sliderLay = new QHBoxLayout();
    m_playBtn = new QPushButton("▶ Play", this);
    m_playBtn->setFixedSize(90, 36);
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setStyleSheet(
        "QPushButton { background: #8B6F47; color: white; border-radius: 18px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: #A0825A; }");
    sliderLay->addWidget(m_playBtn);

    m_timeSlider = new QSlider(Qt::Horizontal, this);
    m_timeSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: #3C2D1E; height: 8px; border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #D4AF37; width: 20px; height: 20px; margin: -6px 0; border-radius: 10px; }"
        "QSlider::sub-page:horizontal { background: #8B6F47; border-radius: 4px; }");
    sliderLay->addWidget(m_timeSlider, 1);
    mainLay->addLayout(sliderLay);

    // Stats label
    m_statsLabel = new QLabel("", this);
    m_statsLabel->setStyleSheet("color: #B8925A; font-size: 13px; background: transparent;");
    m_statsLabel->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(m_statsLabel);

    // Card area
    m_cardArea = new QScrollArea(this);
    m_cardArea->setWidgetResizable(true);
    m_cardArea->setStyleSheet("QScrollArea { background: transparent; border: none; } QWidget { background: transparent; }");
    m_cardContainer = new QWidget();
    m_cardArea->setWidget(m_cardContainer);
    mainLay->addWidget(m_cardArea, 1);

    // Play timer
    m_playTimer = new QTimer(this);
    m_playTimer->setInterval(100); // 1 month per second roughly

    connect(m_playBtn, &QPushButton::clicked, this, [this](){
        if (m_isPlaying) {
            m_playTimer->stop();
            m_playBtn->setText("▶ Play");
            m_isPlaying = false;
        } else {
            m_timeSlider->setValue(m_timeSlider->minimum());
            m_playTimer->start();
            m_playBtn->setText("⏸ Pause");
            m_isPlaying = true;
        }
    });

    connect(m_playTimer, &QTimer::timeout, this, [this](){
        if (m_timeSlider->value() < m_timeSlider->maximum()) {
            m_timeSlider->setValue(m_timeSlider->value() + 1);
        } else {
            m_playTimer->stop();
            m_playBtn->setText("▶ Play");
            m_isPlaying = false;
        }
    });

    connect(m_timeSlider, &QSlider::valueChanged, this, [this](int val){
        if (m_minDate.isValid() && m_maxDate.isValid()) {
            QDate d = m_minDate.addDays(val);
            if (d > m_maxDate) d = m_maxDate;
            m_currentDate = d;
            reconstructState(d);
        }
    });
}

void TimeMachineWidget::loadData() {
    m_equipment = loadAllEquipment();
    m_minDate = QDate::currentDate();
    m_maxDate = QDate(2000, 1, 1);

    for (const auto &eq : m_equipment) {
        if (eq.purchaseDate.isValid()) {
            if (eq.purchaseDate < m_minDate) m_minDate = eq.purchaseDate;
            if (eq.purchaseDate > m_maxDate) m_maxDate = eq.purchaseDate;
        }
    }
    if (m_maxDate < QDate::currentDate()) m_maxDate = QDate::currentDate();
    if (!m_minDate.isValid() || m_minDate > m_maxDate) m_minDate = m_maxDate.addYears(-5);

    int range = m_minDate.daysTo(m_maxDate);
    m_timeSlider->setRange(0, qMax(1, range));
    m_timeSlider->setValue(range);
    m_currentDate = m_maxDate;
    reconstructState(m_maxDate);
}

void TimeMachineWidget::goToDate(const QDate &date) {
    if (m_minDate.isValid()) {
        int days = m_minDate.daysTo(date);
        m_timeSlider->setValue(qBound(0, days, m_timeSlider->maximum()));
    }
}

QString TimeMachineWidget::statusAtDate(int /*equipId*/, const QDate &date) {
    // Since there's no history table, derive from equipment data
    // Equipment purchased after date = doesn't exist yet
    // Use current status as proxy
    Q_UNUSED(date);
    return "";
}

void TimeMachineWidget::reconstructState(const QDate &date) {
    m_dateLabel->setText(QString("Workshop on %1").arg(date.toString("MMMM d, yyyy")));

    // Clear old cards
    if (m_cardContainer->layout()) {
        QLayoutItem *child;
        while ((child = m_cardContainer->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        delete m_cardContainer->layout();
    }

    QGridLayout *grid = new QGridLayout(m_cardContainer);
    grid->setSpacing(10);

    int count = 0, available = 0, inUse = 0, maintenance = 0, retired = 0;
    double totalValue = 0;
    int col = 0, row = 0;

    for (const auto &eq : m_equipment) {
        if (!eq.purchaseDate.isValid() || eq.purchaseDate > date) continue;
        count++;
        QString status = eq.status; // Use current status as approximation
        totalValue += eq.unitPrice;

        if (status == "Available") available++;
        else if (status == "In Use") inUse++;
        else if (status == "Under Maintenance") maintenance++;
        else if (status == "Retired") retired++;

        QFrame *card = createEquipmentCard(eq, status, date);
        grid->addWidget(card, row, col);
        col++;
        if (col >= 4) { col = 0; row++; }
    }

    m_statsLabel->setText(QString("%1 equipment existed | Workshop value: %2 dt | %3 Available, %4 In Use, %5 Maintenance, %6 Retired")
        .arg(count).arg(totalValue, 0, 'f', 0).arg(available).arg(inUse).arg(maintenance).arg(retired));
}

QFrame* TimeMachineWidget::createEquipmentCard(const NexusEquipment &eq, const QString &status, const QDate &) {
    QFrame *card = new QFrame();
    card->setFixedSize(180, 110);

    QColor statusCol;
    if (status == "Available") statusCol = QColor(76, 175, 80);
    else if (status == "In Use") statusCol = QColor(33, 150, 243);
    else if (status == "Under Maintenance") statusCol = QColor(255, 152, 0);
    else statusCol = QColor(158, 158, 158);

    card->setStyleSheet(QString(
        "QFrame { background: rgba(50, 40, 30, 0.8); border: 2px solid %1; border-radius: 10px; }")
        .arg(statusCol.name()));

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(10, 8, 10, 8);
    lay->setSpacing(3);

    QLabel *nameLbl = new QLabel(eq.type, card);
    nameLbl->setStyleSheet("color: white; font-weight: bold; font-size: 12px; background: transparent;");
    nameLbl->setWordWrap(true);
    lay->addWidget(nameLbl);

    QLabel *idLbl = new QLabel(QString("ID: %1").arg(eq.id), card);
    idLbl->setStyleSheet("color: #B8925A; font-size: 10px; background: transparent;");
    lay->addWidget(idLbl);

    QLabel *statusLbl = new QLabel(status, card);
    statusLbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; background: transparent;").arg(statusCol.name()));
    lay->addWidget(statusLbl);

    QLabel *priceLbl = new QLabel(QString("%1 dt").arg(eq.unitPrice, 0, 'f', 2), card);
    priceLbl->setStyleSheet("color: #B8925A; font-size: 10px; background: transparent;");
    lay->addWidget(priceLbl);

    return card;
}

void TimeMachineWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(20, 15, 8));
    bg.setColorAt(1, QColor(35, 25, 15));
    p.fillRect(rect(), bg);
}




