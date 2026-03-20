#include "costswidget.h"
#include <QPainterPath>
#include <QScrollBar>
#include <QDebug>
#include <QtMath>
#include <algorithm>

// ============================================================================
// TCO WIDGET
// ============================================================================
TCOWidget::TCOWidget(QWidget *parent) : QWidget(parent), m_selectedIndex(-1) {
    setMouseTracking(true);
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_globalTime += 0.016f;
        m_animProgress = qMin(1.0f, m_animProgress + 0.008f);
        update();
    });
    m_animTimer->start();
}

void TCOWidget::setData(const QList<EquipmentFinancials> &data) {
    m_data = data;
    m_animProgress = 0.0f;
    m_detailAnimProgress = 0.0f;
    if (!data.isEmpty()) m_selectedIndex = 0;
    update();
}

QColor TCOWidget::getRecommendationColor(const QString &rec) {
    if (rec == "EXCEPTIONAL") return QColor("#4CAF7D");
    if (rec == "GOOD VALUE") return QColor("#C17F3E");
    if (rec == "MONITOR") return QColor("#3B82F6");
    if (rec == "REVIEW") return QColor("#F59E0B");
    return QColor("#CC2200");
}

void TCOWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(13, 8, 5));
    bg.setColorAt(1, QColor(8, 5, 3));
    p.fillRect(rect(), bg);

    if (m_data.isEmpty()) {
        p.setPen(QColor("#C17F3E"));
        QFont f = p.font(); f.setPixelSize(14); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "No equipment data available");
        return;
    }

    drawListPanel(p);

    // Divider
    p.setPen(QPen(QColor(30, 21, 8), 1));
    p.drawLine(260, 0, 260, height());
    QLinearGradient divGlow(260, 0, 260, height());
    divGlow.setColorAt(0, Qt::transparent);
    divGlow.setColorAt(0.5, QColor(193, 127, 62, 30));
    divGlow.setColorAt(1, Qt::transparent);
    p.setPen(QPen(QBrush(divGlow), 1));
    p.drawLine(261, 0, 261, height());

    if (m_selectedIndex >= 0 && m_selectedIndex < m_data.size()) {
        drawDetailPanel(p);
    }
}

void TCOWidget::drawListPanel(QPainter &p) {
    // Panel background
    QLinearGradient lbg(0, 0, 0, height());
    lbg.setColorAt(0, QColor(26, 17, 8));
    lbg.setColorAt(1, QColor(13, 8, 5));
    p.fillRect(0, 0, 260, height(), lbg);

    // Title
    QFont tf = p.font(); tf.setPixelSize(10); tf.setBold(true);
    tf.setLetterSpacing(QFont::AbsoluteSpacing, 3);
    p.setFont(tf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(12, 22, "SELECT EQUIPMENT");
    p.setPen(QPen(QColor("#C17F3E"), 1));
    p.drawLine(12, 28, 248, 28);

    // Items
    int y = 36;
    for (int i = 0; i < m_data.size() && y < height(); i++) {
        const auto &eq = m_data[i];
        QRectF itemRect(0, y, 260, m_listItemHeight);
        bool selected = (i == m_selectedIndex);
        bool hovered = (i == m_hoveredIndex);

        // Background
        if (selected) {
            QLinearGradient sbg(0, 0, 260, 0);
            sbg.setColorAt(0, QColor(42, 26, 8));
            sbg.setColorAt(1, QColor(21, 14, 5));
            p.fillRect(itemRect, sbg);
            // Left accent bar
            QColor recCol = getRecommendationColor(eq.recommendation);
            p.fillRect(QRectF(0, y, 3, m_listItemHeight), recCol);
        } else if (hovered) {
            QLinearGradient hbg(0, 0, 260, 0);
            hbg.setColorAt(0, QColor(30, 20, 8));
            hbg.setColorAt(1, QColor(13, 8, 5));
            p.fillRect(itemRect, hbg);
        } else {
            p.fillRect(itemRect, QColor(13, 8, 5));
        }

        // Bottom border
        p.setPen(QPen(QColor(26, 18, 8), 1));
        p.drawLine(0, y + m_listItemHeight - 1, 260, y + m_listItemHeight - 1);

        // Recommendation dot
        QColor recCol = getRecommendationColor(eq.recommendation);
        float dotR = 4.5f;
        if (eq.recommendation == "EXCEPTIONAL" || eq.recommendation == "RETIRE")
            dotR = 4.5f + 1.0f * sinf(m_globalTime * 2.0f + i);
        p.setPen(Qt::NoPen);
        QRadialGradient dotGlow(QPointF(16, y + 20), 7);
        dotGlow.setColorAt(0, QColor(recCol.red(), recCol.green(), recCol.blue(), 102));
        dotGlow.setColorAt(1, QColor(recCol.red(), recCol.green(), recCol.blue(), 0));
        p.setBrush(dotGlow);
        p.drawEllipse(QPointF(16, y + 20), 7, 7);
        p.setBrush(recCol);
        p.drawEllipse(QPointF(16, y + 20), dotR, dotR);

        // Name
        QFont nf = p.font(); nf.setPixelSize(12); nf.setBold(true); p.setFont(nf);
        p.setPen(QColor(245, 230, 211));
        QString name = eq.name.length() > 20 ? eq.name.left(18) + ".." : eq.name;
        p.drawText(28, y + 22, name);

        // ROI badge
        QFont bf = p.font(); bf.setPixelSize(9); bf.setBold(true); p.setFont(bf);
        QFontMetrics bfm(bf);
        int badgeW = bfm.horizontalAdvance(eq.recommendation) + 12;
        QRectF badgeRect(248 - badgeW - 4, y + 10, badgeW, 18);
        p.setBrush(QColor(recCol.red(), recCol.green(), recCol.blue(), 51));
        p.setPen(QPen(QColor(recCol.red(), recCol.green(), recCol.blue(), 128), 1));
        p.drawRoundedRect(badgeRect, 8, 8);
        p.setPen(recCol);
        p.drawText(badgeRect, Qt::AlignCenter, eq.recommendation);

        // TCO value
        QFont vf = p.font(); vf.setPixelSize(11); vf.setBold(false); p.setFont(vf);
        p.setPen(QColor("#C17F3E"));
        p.drawText(28, y + 40, QString("TCO: %1 dt").arg(eq.totalTCO, 0, 'f', 0));

        // Cost per year
        QFont cf = p.font(); cf.setPixelSize(10); p.setFont(cf);
        p.setPen(QColor(245, 230, 211, 153));
        QString cpy = QString("%1dt/yr").arg(eq.costPerYear, 0, 'f', 0);
        p.drawText(QRectF(160, y + 30, 90, 16), Qt::AlignRight, cpy);

        y += m_listItemHeight;
    }
}

void TCOWidget::drawDetailPanel(QPainter &p) {
    const auto &eq = m_data[m_selectedIndex];
    int px = 270;
    int pw = width() - px - 10;

    m_detailAnimProgress = qMin(1.0f, m_detailAnimProgress + 0.02f);
    float ease = 1.0f - powf(1.0f - m_detailAnimProgress, 3.0f);

    // Equipment Identity Header
    QLinearGradient hbg(px, 0, px, 80);
    hbg.setColorAt(0, QColor(30, 18, 8));
    hbg.setColorAt(1, QColor(13, 8, 5, 0));
    p.fillRect(px, 0, pw, 80, hbg);
    p.setPen(QPen(QColor(193, 127, 62, 51), 1));
    p.drawLine(px, 79, px + pw, 79);

    // Name
    QFont nf = p.font(); nf.setPixelSize(22); nf.setBold(true); p.setFont(nf);
    p.setPen(QColor(0, 0, 0, 100)); p.drawText(px + 13, 33, eq.name);
    p.setPen(QColor(245, 230, 211)); p.drawText(px + 12, 32, eq.name);

    // Status badge
    QColor statusCol("#C17F3E");
    if (eq.status == "Available") statusCol = QColor("#4CAF7D");
    else if (eq.status == "In Use") statusCol = QColor("#3B82F6");
    else if (eq.status == "Under Maintenance") statusCol = QColor("#F59E0B");
    else if (eq.status == "Retired") statusCol = QColor("#CC2200");

    QFont sf = p.font(); sf.setPixelSize(10); sf.setBold(true); p.setFont(sf);
    QFontMetrics sfm(sf);
    int statusW = sfm.horizontalAdvance(eq.status) + 16;
    QRectF statusRect(px + 12, 44, statusW, 22);
    p.setBrush(QColor(statusCol.red(), statusCol.green(), statusCol.blue(), 64));
    p.setPen(QPen(statusCol, 1));
    p.drawRoundedRect(statusRect, 11, 11);
    p.setPen(Qt::white);
    p.drawText(statusRect, Qt::AlignCenter, eq.status);

    // Age info
    p.setPen(QColor(245, 230, 211, 128));
    sf.setPixelSize(10); sf.setBold(false); p.setFont(sf);
    p.drawText(px + statusW + 20, 58, QString("Age: %1 years | Purchase: %2 dt")
               .arg(eq.ageInYears, 0, 'f', 1).arg(eq.purchasePrice, 0, 'f', 0));

    // Cost Breakdown Stack (3D visualization)
    int stackY = 90;
    int stackH = 170;
    QRectF stackArea(px, stackY, pw, stackH);
    drawCostStack(p, stackArea, eq);

    // 3 Metric Boxes
    int boxY = stackY + stackH + 10;
    int boxW = (pw - 20) / 3;
    int boxH = 80;

    QColor cpyColor = (eq.costPerYear < 30) ? QColor("#4CAF7D") :
                       (eq.costPerYear < 100) ? QColor("#C17F3E") : QColor("#CC2200");
    drawMetricBox(p, QRectF(px, boxY, boxW, boxH), "COST / YEAR",
                  QString("%1 dt").arg(eq.costPerYear, 0, 'f', 1), cpyColor, ease);

    drawMetricBox(p, QRectF(px + boxW + 10, boxY, boxW, boxH), "COST / DAY",
                  QString("%1 dt").arg(eq.costPerDay, 0, 'f', 2), QColor("#C17F3E"), ease);

    QColor bvColor = (eq.depreciationPct < 50) ? QColor("#4CAF7D") :
                      (eq.depreciationPct < 80) ? QColor("#C17F3E") : QColor("#CC2200");
    drawMetricBox(p, QRectF(px + 2*(boxW + 10), boxY, boxW, boxH), "BOOK VALUE",
                  QString("%1 dt").arg(eq.bookValue, 0, 'f', 0), bvColor, ease);

    // Depreciation Gauge
    int gaugeY = boxY + boxH + 20;
    double gaugeR = qMin(70.0, (double)(pw - 40) / 4.0);
    QPointF gaugeCenter(px + pw / 2.0, gaugeY + gaugeR + 10);
    drawDepreciationGauge(p, gaugeCenter, gaugeR, eq);

    // Verdict box
    int verdictY = (int)(gaugeCenter.y() + gaugeR + 30);
    if (verdictY + 80 < height()) {
        drawVerdictBox(p, QRectF(px, verdictY, pw, 70), eq);
    }
}

void TCOWidget::drawCostStack(QPainter &p, const QRectF &area, const EquipmentFinancials &eq) {
    if (eq.totalTCO <= 0) return;
    float animEase = 1.0f - powf(1.0f - qMin(1.0f, m_animProgress * 1.5f), 4.0f);

    struct Layer { QString label; double value; QColor c1, c2; };
    QList<Layer> layers = {
        {"Purchase Price", eq.purchasePrice, QColor("#3B82F6"), QColor("#1E40AF")},
        {"Maintenance", eq.estimatedMaintenanceCost, QColor("#F59E0B"), QColor("#B45309")},
        {"Operational", eq.estimatedOperationalCost, QColor("#8B5CF6"), QColor("#5B21B6")},
        {"TOTAL TCO", eq.totalTCO, QColor("#C17F3E"), QColor("#8B4A1E")}
    };

    float y = area.y() + 10;
    float maxW = area.width() - 30;

    for (int i = 0; i < layers.size(); i++) {
        float barW = (layers[i].value / eq.totalTCO) * maxW * animEase;
        if (i == 3) barW = maxW * animEase; // Total always full
        float barH = (i == 3) ? 38 : 30;

        // 3D right face
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(layers[i].c2.red()/2, layers[i].c2.green()/2, layers[i].c2.blue()/2));
        QPolygonF rightFace;
        rightFace << QPointF(area.x() + barW, y)
                  << QPointF(area.x() + barW + 6, y - 4)
                  << QPointF(area.x() + barW + 6, y + barH - 4)
                  << QPointF(area.x() + barW, y + barH);
        p.drawPolygon(rightFace);

        // 3D top face
        QPolygonF topFace;
        topFace << QPointF(area.x(), y)
                << QPointF(area.x() + 6, y - 4)
                << QPointF(area.x() + barW + 6, y - 4)
                << QPointF(area.x() + barW, y);
        p.setBrush(layers[i].c1.lighter(130));
        p.drawPolygon(topFace);

        // Front face
        QLinearGradient fg(area.x(), y, area.x(), y + barH);
        fg.setColorAt(0, layers[i].c1);
        fg.setColorAt(1, layers[i].c2);
        p.setBrush(fg);
        p.drawRoundedRect(QRectF(area.x(), y, barW, barH), 3, 3);

        // Label
        QFont lf = p.font(); lf.setPixelSize(10); lf.setBold(i == 3); p.setFont(lf);
        p.setPen(Qt::white);
        p.drawText(QRectF(area.x() + 8, y, barW - 16, barH), Qt::AlignVCenter | Qt::AlignLeft, layers[i].label);

        // Value
        p.drawText(QRectF(area.x() + 8, y, barW - 16, barH), Qt::AlignVCenter | Qt::AlignRight,
                   QString("%1 dt").arg(layers[i].value, 0, 'f', 0));

        y += barH + 5;
    }
}

void TCOWidget::drawMetricBox(QPainter &p, const QRectF &rect, const QString &label,
                                const QString &value, const QColor &color, float progress) {
    // 3D card
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(13, 8, 5));
    p.drawRoundedRect(rect.adjusted(0, 4, 3, 4), 10, 10);

    p.setBrush(QColor(26, 18, 8));
    p.setPen(QPen(QColor(193, 127, 62, 64), 1));
    p.drawRoundedRect(rect, 10, 10);

    // Corner highlight
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(42, 31, 10, 80));
    QPainterPath corner;
    corner.moveTo(rect.x() + 1, rect.y() + 1);
    corner.lineTo(rect.x() + 20, rect.y() + 1);
    corner.lineTo(rect.x() + 1, rect.y() + 20);
    corner.closeSubpath();
    p.drawPath(corner);

    // Label
    QFont lf = p.font(); lf.setPixelSize(8); lf.setBold(false);
    lf.setLetterSpacing(QFont::AbsoluteSpacing, 1); p.setFont(lf);
    p.setPen(QColor(245, 230, 211, 115));
    p.drawText(rect.adjusted(10, 8, -10, 0), Qt::AlignTop | Qt::AlignLeft, label);

    // Value
    QFont vf = p.font(); vf.setPixelSize(20); vf.setBold(true); p.setFont(vf);
    p.setPen(color);
    p.drawText(rect.adjusted(10, 0, -10, -10), Qt::AlignBottom | Qt::AlignLeft, value);
}

void TCOWidget::drawDepreciationGauge(QPainter &p, const QPointF &center, double radius,
                                        const EquipmentFinancials &eq) {
    float animVal = qMin(1.0f, m_animProgress * 0.8f);
    float easeVal = 1.0f - powf(1.0f - animVal, 3.0f);

    // Outer glow
    float glowOp = 0.6f + 0.4f * sinf(m_globalTime * 0.8f);
    p.setPen(QPen(QColor(193, 127, 62, (int)(38 * glowOp)), 4));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(center, radius + 14, radius + 14);

    // Background ring
    QPen bgPen(QColor(26, 18, 8), 16);
    bgPen.setCapStyle(Qt::RoundCap);
    p.setPen(bgPen);
    p.drawArc(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2),
              -90 * 16, 360 * 16);

    // Filled arc
    double span = eq.depreciationPct * 3.6 * easeVal;
    QConicalGradient cg(center, 90);
    cg.setColorAt(0, QColor("#4CAF7D"));
    cg.setColorAt(0.33, QColor("#F59E0B"));
    cg.setColorAt(0.66, QColor("#CC2200"));
    cg.setColorAt(1.0, QColor("#CC2200"));
    QPen arcPen(QBrush(cg), 16);
    arcPen.setCapStyle(Qt::RoundCap);
    p.setPen(arcPen);
    p.drawArc(QRectF(center.x() - radius, center.y() - radius, radius * 2, radius * 2),
              90 * 16, -(int)(span * 16));

    // Tick marks
    p.setPen(QPen(QColor(193, 127, 62, 77), 1));
    for (int i = 0; i < 12; i++) {
        double angle = i * 30.0 * M_PI / 180.0 - M_PI / 2;
        p.drawLine(QPointF(center.x() + (radius - 14) * cos(angle), center.y() + (radius - 14) * sin(angle)),
                   QPointF(center.x() + (radius - 8) * cos(angle), center.y() + (radius - 8) * sin(angle)));
    }

    // Center text
    QFont cf = p.font(); cf.setPixelSize(22); cf.setBold(true); p.setFont(cf);
    p.setPen(QColor(0, 0, 0, 128));
    p.drawText(QRectF(center.x() - 40, center.y() - 18, 80, 24), Qt::AlignCenter,
               QString("%1%").arg((int)(eq.depreciationPct * easeVal)));
    p.setPen(QColor(245, 230, 211));
    p.drawText(QRectF(center.x() - 40, center.y() - 20, 80, 24), Qt::AlignCenter,
               QString("%1%").arg((int)(eq.depreciationPct * easeVal)));

    cf.setPixelSize(9); cf.setBold(false); p.setFont(cf);
    p.setPen(QColor(245, 230, 211, 128));
    p.drawText(QRectF(center.x() - 40, center.y() + 4, 80, 16), Qt::AlignCenter, "depreciated");

    QColor recCol = getRecommendationColor(eq.recommendation);
    cf.setPixelSize(11); cf.setBold(true); p.setFont(cf);
    p.setPen(recCol);
    p.drawText(QRectF(center.x() - 50, center.y() + 18, 100, 18), Qt::AlignCenter, eq.recommendation);
}

void TCOWidget::drawVerdictBox(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq) {
    QColor recCol = getRecommendationColor(eq.recommendation);

    // Background
    QLinearGradient bg(rect.topLeft(), rect.bottomRight());
    bg.setColorAt(0, QColor(26, 18, 8)); bg.setColorAt(1, QColor(10, 8, 4));
    p.setBrush(bg);
    float borderOp = 0.4f + 0.2f * sinf(m_globalTime * 1.3f);
    p.setPen(QPen(QColor(recCol.red(), recCol.green(), recCol.blue(), (int)(255 * borderOp)), 1));
    p.drawRoundedRect(rect, 12, 12);

    // Left accent
    p.setPen(Qt::NoPen);
    p.setBrush(recCol);
    p.drawRoundedRect(QRectF(rect.x() + 2, rect.y() + 8, 3, rect.height() - 16), 2, 2);

    // Glow
    QRadialGradient glow(rect.x() + 8, rect.center().y(), 12);
    glow.setColorAt(0, QColor(recCol.red(), recCol.green(), recCol.blue(), 51));
    glow.setColorAt(1, Qt::transparent);
    p.setBrush(glow); p.drawRect(QRectF(rect.x(), rect.y(), 24, rect.height()));

    // Text
    QFont tf = p.font(); tf.setPixelSize(12); tf.setItalic(true); p.setFont(tf);
    p.setPen(QColor(245, 230, 211));
    QString verdict = QString("%1 — %2 with TCO of %3 dt (%4 dt/yr). Book value: %5 dt. ROI: %6/100.")
                      .arg(eq.recommendation).arg(eq.name).arg(eq.totalTCO, 0, 'f', 0)
                      .arg(eq.costPerYear, 0, 'f', 1).arg(eq.bookValue, 0, 'f', 0)
                      .arg(eq.roiScore, 0, 'f', 0);
    p.drawText(rect.adjusted(18, 8, -10, -8), Qt::AlignVCenter | Qt::TextWordWrap, verdict);
}

void TCOWidget::mousePressEvent(QMouseEvent *event) {
    if (event->x() < 260) {
        int y = 36;
        for (int i = 0; i < m_data.size(); i++) {
            if (event->y() >= y && event->y() < y + m_listItemHeight) {
                m_selectedIndex = i;
                m_detailAnimProgress = 0.0f;
                update();
                return;
            }
            y += m_listItemHeight;
        }
    }
}

void TCOWidget::mouseMoveEvent(QMouseEvent *event) {
    int oldHover = m_hoveredIndex;
    m_hoveredIndex = -1;
    if (event->x() < 260) {
        int y = 36;
        for (int i = 0; i < m_data.size(); i++) {
            if (event->y() >= y && event->y() < y + m_listItemHeight) {
                m_hoveredIndex = i;
                break;
            }
            y += m_listItemHeight;
        }
    }
    if (oldHover != m_hoveredIndex) update();
}

void TCOWidget::resizeEvent(QResizeEvent *) { update(); }

// ============================================================================
// REPAIR VS REPLACE WIDGET
// ============================================================================
RepairReplaceWidget::RepairReplaceWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_globalTime += 0.016f;
        m_animProgress = qMin(1.0f, m_animProgress + 0.01f);
        update();
    });
    m_animTimer->start();

    m_equipCombo = new QComboBox(this);
    m_equipCombo->setStyleSheet(
        "QComboBox { background: #0D0805; color: #C17F3E; border: 1px solid #C17F3E; "
        "border-radius: 8px; padding: 6px 14px; font-weight: bold; font-size: 12px; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox QAbstractItemView { background: #0D0805; color: #F5E6D3; "
        "selection-background-color: #1E1208; border: 1px solid #C17F3E; }");
    m_equipCombo->setGeometry(20, 10, 350, 36);
    connect(m_equipCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        m_selectedIndex = idx;
        m_animProgress = 0.0f;
        update();
    });
}

void RepairReplaceWidget::setData(const QList<EquipmentFinancials> &data) {
    m_data = data;
    m_equipCombo->clear();
    for (const auto &eq : data) {
        m_equipCombo->addItem(QString("[%1] %2 — %3 dt").arg(eq.id).arg(eq.name).arg(eq.purchasePrice, 0, 'f', 0));
    }
    m_animProgress = 0.0f;
    update();
}

bool RepairReplaceWidget::repairWins(const EquipmentFinancials &eq) {
    return eq.repairCost < eq.replacementCost * 0.4 && eq.remainingYears > 3;
}

void RepairReplaceWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    p.fillRect(rect(), QColor(8, 5, 3));

    if (m_data.isEmpty() || m_selectedIndex < 0 || m_selectedIndex >= m_data.size()) {
        p.setPen(QColor("#C17F3E")); QFont f = p.font(); f.setPixelSize(14); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "Select equipment to analyze");
        return;
    }

    const auto &eq = m_data[m_selectedIndex];
    bool rWins = repairWins(eq);
    float ease = 1.0f - powf(1.0f - m_animProgress, 3.0f);

    int panelY = 56;
    int panelH = height() - panelY - 130;
    int panelW = (width() - 100) / 2;

    // Repair panel (left)
    drawRepairPanel(p, QRectF(20, panelY, panelW, panelH), eq);

    // VS section (center)
    drawVSSection(p, QRectF(20 + panelW, panelY, 60, panelH), eq);

    // Replace panel (right)
    drawReplacePanel(p, QRectF(80 + panelW, panelY, panelW, panelH), eq);

    // Recommendation banner (bottom)
    drawRecommendationBanner(p, QRectF(20, height() - 110, width() - 40, 70), eq);
}

void RepairReplaceWidget::drawRepairPanel(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq) {
    bool winner = repairWins(eq);
    float ease = 1.0f - powf(1.0f - m_animProgress, 3.0f);

    // Shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 128));
    p.drawRoundedRect(rect.adjusted(4, 6, 4, 6), 14, 14);

    // Panel
    QLinearGradient bg(rect.topLeft(), rect.bottomLeft());
    bg.setColorAt(0, QColor(26, 21, 8)); bg.setColorAt(1, QColor(13, 8, 5));
    p.setBrush(bg);
    p.setPen(QPen(QColor(193, 127, 62, 102), 1));
    p.drawRoundedRect(rect, 14, 14);

    // Top band
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#C17F3E"));
    p.drawRoundedRect(QRectF(rect.x() + 1, rect.y() + 1, rect.width() - 2, 4), 2, 2);

    // Title
    QFont tf = p.font(); tf.setPixelSize(18); tf.setBold(true); p.setFont(tf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(rect.adjusted(16, 20, 0, 0), Qt::AlignLeft, QString::fromUtf8("\xF0\x9F\x94\xA7 REPAIR"));

    QFont sf = p.font(); sf.setPixelSize(10); sf.setBold(false); p.setFont(sf);
    p.setPen(QColor(245, 230, 211, 128));
    p.drawText(rect.adjusted(16, 42, 0, 0), Qt::AlignLeft, "Option A");

    // Metrics
    int my = rect.y() + 70;
    auto drawRow = [&](const QString &label, const QString &value, const QColor &valColor) {
        p.setPen(QColor(245, 230, 211, 179));
        sf.setPixelSize(12); p.setFont(sf);
        p.drawText(QRectF(rect.x() + 16, my, rect.width() / 2, 20), Qt::AlignLeft | Qt::AlignVCenter, label);
        sf.setBold(true); p.setFont(sf);
        p.setPen(valColor);
        p.drawText(QRectF(rect.x() + rect.width() / 2, my, rect.width() / 2 - 16, 20), Qt::AlignRight | Qt::AlignVCenter, value);
        sf.setBold(false); p.setFont(sf);
        p.setPen(QPen(QColor(30, 21, 8), 1));
        p.drawLine(rect.x() + 16, my + 22, rect.x() + rect.width() - 16, my + 22);
        my += 28;
    };

    drawRow("Estimated Cost:", QString("%1 dt").arg(eq.repairCost * ease, 0, 'f', 0), QColor("#C17F3E"));
    drawRow("Extended Life:", QString("+%1 years").arg(qMin(5, eq.remainingYears)), QColor("#4CAF7D"));
    drawRow("Cost per Year:", QString("%1 dt/yr").arg(eq.repairCost / qMax(1, qMin(5, eq.remainingYears)), 0, 'f', 1), QColor("#F59E0B"));
    drawRow("Downtime:", "1-3 days", QColor("#3B82F6"));

    // Risk level
    my += 10;
    p.setPen(QColor(245, 230, 211, 179));
    sf.setPixelSize(12); p.setFont(sf);
    p.drawText(QRectF(rect.x() + 16, my, 100, 20), Qt::AlignLeft | Qt::AlignVCenter, "Risk Level:");

    int segW = (rect.width() - 80) / 3;
    QStringList levels = {"LOW", "MEDIUM", "HIGH"};
    QList<QColor> colors = {QColor("#4CAF7D"), QColor("#F59E0B"), QColor("#CC2200")};
    int activeLevel = (eq.ageInYears > 20) ? 2 : (eq.ageInYears > 10) ? 1 : 0;
    for (int i = 0; i < 3; i++) {
        QRectF segRect(rect.x() + 100 + i * segW, my, segW - 4, 20);
        p.setBrush((i == activeLevel) ? colors[i] : QColor(26, 18, 8));
        p.setPen(QPen(colors[i], 1));
        p.drawRoundedRect(segRect, 4, 4);
        sf.setPixelSize(8); sf.setBold(true); p.setFont(sf);
        p.setPen((i == activeLevel) ? Qt::white : colors[i]);
        p.drawText(segRect, Qt::AlignCenter, levels[i]);
    }
}

void RepairReplaceWidget::drawReplacePanel(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq) {
    bool winner = !repairWins(eq);
    float ease = 1.0f - powf(1.0f - m_animProgress, 3.0f);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 128));
    p.drawRoundedRect(rect.adjusted(4, 6, 4, 6), 14, 14);

    QLinearGradient bg(rect.topLeft(), rect.bottomLeft());
    bg.setColorAt(0, winner ? QColor(10, 26, 8) : QColor(26, 21, 8));
    bg.setColorAt(1, QColor(13, 8, 5));
    p.setBrush(bg);
    QColor borderCol = winner ? QColor("#4CAF7D") : QColor(193, 127, 62, 102);
    p.setPen(QPen(borderCol, 1));
    p.drawRoundedRect(rect, 14, 14);

    p.setPen(Qt::NoPen);
    p.setBrush(winner ? QColor("#4CAF7D") : QColor("#3B82F6"));
    p.drawRoundedRect(QRectF(rect.x() + 1, rect.y() + 1, rect.width() - 2, 4), 2, 2);

    QFont tf = p.font(); tf.setPixelSize(18); tf.setBold(true); p.setFont(tf);
    p.setPen(winner ? QColor("#4CAF7D") : QColor("#3B82F6"));
    p.drawText(rect.adjusted(16, 20, 0, 0), Qt::AlignLeft, QString::fromUtf8("\xF0\x9F\x94\x84 REPLACE"));

    QFont sf = p.font(); sf.setPixelSize(10); sf.setBold(false); p.setFont(sf);
    p.setPen(QColor(245, 230, 211, 128));
    p.drawText(rect.adjusted(16, 42, 0, 0), Qt::AlignLeft, "Option B");

    int my = rect.y() + 70;
    auto drawRow = [&](const QString &label, const QString &value, const QColor &valColor) {
        p.setPen(QColor(245, 230, 211, 179));
        sf.setPixelSize(12); p.setFont(sf);
        p.drawText(QRectF(rect.x() + 16, my, rect.width() / 2, 20), Qt::AlignLeft | Qt::AlignVCenter, label);
        sf.setBold(true); p.setFont(sf);
        p.setPen(valColor);
        p.drawText(QRectF(rect.x() + rect.width() / 2, my, rect.width() / 2 - 16, 20), Qt::AlignRight | Qt::AlignVCenter, value);
        sf.setBold(false); p.setFont(sf);
        p.setPen(QPen(QColor(30, 21, 8), 1));
        p.drawLine(rect.x() + 16, my + 22, rect.x() + rect.width() - 16, my + 22);
        my += 28;
    };

    drawRow("Estimated Cost:", QString("%1 dt").arg(eq.replacementCost * ease, 0, 'f', 0), QColor("#CC2200"));
    drawRow("New Lifespan:", "20 years", QColor("#4CAF7D"));
    drawRow("Cost per Year:", QString("%1 dt/yr").arg(eq.replacementCost / 20.0, 0, 'f', 1), QColor("#F59E0B"));
    drawRow("Downtime:", "1-2 weeks", QColor("#3B82F6"));
}

void RepairReplaceWidget::drawVSSection(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq) {
    float scale = 1.0f + 0.06f * sinf(m_globalTime * 1.8f);

    QFont vf = p.font(); vf.setPixelSize(28); vf.setBold(true); p.setFont(vf);

    p.save();
    p.translate(rect.center());
    p.scale(scale, scale);

    // Glow
    QRadialGradient glow(0, 0, 25);
    glow.setColorAt(0, QColor(193, 127, 62, 60));
    glow.setColorAt(1, Qt::transparent);
    p.setPen(Qt::NoPen);
    p.setBrush(glow);
    p.drawEllipse(QPointF(0, 0), 25, 25);

    QLinearGradient tg(0, -14, 0, 14);
    tg.setColorAt(0, QColor("#FFD700"));
    tg.setColorAt(1, QColor("#C17F3E"));
    p.setPen(QPen(QBrush(tg), 1));
    p.drawText(QRectF(-20, -14, 40, 28), Qt::AlignCenter, "VS");
    p.restore();

    // Winner arrow
    bool rWins = repairWins(eq);
    float arrowX = rWins ? -15 : 15;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FFD700"));
    QPainterPath arrow;
    QPointF ac = rect.center() + QPointF(arrowX, 30);
    arrow.moveTo(ac + QPointF(-8, -6));
    arrow.lineTo(ac + QPointF(8 * (rWins ? -1 : 1), 0));
    arrow.lineTo(ac + QPointF(-8, 6));
    arrow.closeSubpath();
    p.drawPath(arrow);
}

void RepairReplaceWidget::drawRecommendationBanner(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq) {
    bool rWins = repairWins(eq);

    QColor bgC1 = rWins ? QColor(10, 26, 8) : QColor(26, 8, 8);
    QColor bgC2 = rWins ? QColor(5, 13, 4) : QColor(13, 4, 4);
    QColor borderCol = rWins ? QColor("#4CAF7D") : QColor("#CC2200");

    QLinearGradient bg(rect.topLeft(), rect.bottomRight());
    bg.setColorAt(0, bgC1); bg.setColorAt(1, bgC2);
    p.setBrush(bg);
    p.setPen(QPen(borderCol, 1));
    p.drawRoundedRect(rect, 12, 12);

    // Left bar
    p.setPen(Qt::NoPen);
    p.setBrush(borderCol);
    p.drawRoundedRect(QRectF(rect.x() + 2, rect.y() + 8, 4, rect.height() - 16), 2, 2);

    // Text
    QFont tf = p.font(); tf.setPixelSize(15); tf.setBold(true); p.setFont(tf);
    p.setPen(borderCol);
    QString text = rWins
        ? QString::fromUtf8("\xF0\x9F\x94\xA7 REPAIR RECOMMENDED")
        : QString::fromUtf8("\xF0\x9F\x94\x84 REPLACEMENT RECOMMENDED");
    p.drawText(rect.adjusted(20, 12, -10, -28), Qt::AlignLeft | Qt::AlignVCenter, text);

    QFont sf = p.font(); sf.setPixelSize(11); sf.setBold(false); p.setFont(sf);
    p.setPen(QColor(245, 230, 211));
    double savings = rWins ? eq.replacementCost - eq.repairCost : eq.repairCost;
    QString sub = rWins
        ? QString("Saves %1 dt over %2 years").arg(savings, 0, 'f', 0).arg(eq.remainingYears)
        : QString("Replacement delivers %1x better long-term value").arg(eq.replacementCost / qMax(1.0, eq.repairCost), 0, 'f', 1);
    p.drawText(rect.adjusted(20, 28, -10, -6), Qt::AlignLeft | Qt::AlignVCenter, sub);
}
