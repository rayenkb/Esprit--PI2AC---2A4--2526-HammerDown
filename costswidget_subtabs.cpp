#include "costswidget.h"
#include <QPainterPath>
#include <QDebug>
#include <QtMath>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <algorithm>

// ============================================================================
// FORECAST WIDGET
// ============================================================================
ForecastWidget::ForecastWidget(QWidget *parent) : QWidget(parent) {
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

void ForecastWidget::setData(const QList<EquipmentFinancials> &data) {
    m_data = data;
    m_animProgress = 0.0f;
    recalculate();
    update();
}

void ForecastWidget::recalculate() {
    m_months.clear();
    QDate today = QDate::currentDate();
    QStringList monthNames = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    for (int m = 0; m < m_periodMonths; m++) {
        MonthForecast mf;
        QDate month = today.addMonths(m + 1);
        mf.monthName = monthNames[month.month() - 1] + " " + QString::number(month.year()).right(2);
        mf.total = 0;

        for (const auto &eq : m_data) {
            double monthlyCost = eq.monthlyBudgetNeeded;
            // Add maintenance spikes for older equipment
            if (eq.ageInYears > 10) monthlyCost *= 1.2;
            if (eq.status == "Under Maintenance") monthlyCost *= 1.5;
            if (monthlyCost > 0.5) {
                mf.items.append(qMakePair(eq.name, monthlyCost));
                mf.total += monthlyCost;
            }
        }

        // Sort items by cost descending
        std::sort(mf.items.begin(), mf.items.end(),
                  [](const QPair<QString,double> &a, const QPair<QString,double> &b) { return a.second > b.second; });
        m_months.append(mf);
    }
}

void ForecastWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor(8, 5, 3));

    if (m_months.isEmpty()) {
        p.setPen(QColor("#C17F3E")); QFont f = p.font(); f.setPixelSize(14); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "No forecast data");
        return;
    }

    // Period selector
    drawPeriodSelector(p, QRectF(20, 10, 300, 36));

    // Total reserve box
    double totalForecast = 0;
    for (const auto &m : m_months) totalForecast += m.total;
    double reserve = totalForecast * 1.1;

    QRectF reserveBox(340, 10, 280, 36);
    float borderOp = 0.5f + 0.3f * sinf(m_globalTime * 1.5f);
    p.setBrush(QColor(26, 18, 8));
    p.setPen(QPen(QColor(193, 127, 62, (int)(255 * borderOp)), 1));
    p.drawRoundedRect(reserveBox, 8, 8);
    QFont rf = p.font(); rf.setPixelSize(12); rf.setBold(true); p.setFont(rf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(reserveBox.adjusted(10, 0, -10, 0), Qt::AlignVCenter,
               QString("RECOMMENDED RESERVE: %1 dt").arg(reserve, 0, 'f', 0));

    // Bar chart
    drawBarChart(p, QRectF(20, 56, width() - 40, height() - 76));
}

void ForecastWidget::drawPeriodSelector(QPainter &p, const QRectF &area) {
    QStringList periods = {"3M", "6M", "12M"};
    QList<int> values = {3, 6, 12};
    int btnW = 60;
    int x = area.x();

    for (int i = 0; i < 3; i++) {
        QRectF btn(x, area.y(), btnW, area.height());
        bool active = (m_periodMonths == values[i]);

        if (active) {
            QLinearGradient bg(btn.topLeft(), btn.topRight());
            bg.setColorAt(0, QColor("#C17F3E")); bg.setColorAt(1, QColor("#8B4A1E"));
            p.setBrush(bg);
            p.setPen(QPen(QColor(255, 176, 96, 153), 1));
        } else {
            p.setBrush(Qt::transparent);
            p.setPen(QPen(QColor(193, 127, 62, 89), 1));
        }
        p.drawRoundedRect(btn, 17, 17);

        QFont bf = p.font(); bf.setPixelSize(11); bf.setBold(true); p.setFont(bf);
        p.setPen(active ? Qt::white : QColor(193, 127, 62, 179));
        p.drawText(btn, Qt::AlignCenter, periods[i]);

        x += btnW + 8;
    }
}

void ForecastWidget::drawBarChart(QPainter &p, const QRectF &area) {
    if (m_months.isEmpty()) return;

    float ease = 1.0f - powf(1.0f - m_animProgress, 4.0f);

    // Chart background
    p.setBrush(QColor(8, 5, 3));
    p.setPen(QPen(QColor(30, 18, 8), 1));
    p.drawRoundedRect(area, 12, 12);

    double maxVal = 1;
    for (const auto &m : m_months) if (m.total > maxVal) maxVal = m.total;
    maxVal *= 1.2;

    int padding = 60;
    double chartW = area.width() - padding - 20;
    double chartH = area.height() - 80;
    double chartX = area.x() + padding;
    double chartY = area.y() + 20;

    // Y-axis grid lines
    for (int i = 0; i <= 4; i++) {
        double yy = chartY + chartH - (chartH * i / 4.0);
        p.setPen(QPen(QColor(193, 127, 62, 26), 1));
        p.drawLine(chartX, yy, chartX + chartW, yy);

        QFont af = p.font(); af.setPixelSize(9); p.setFont(af);
        p.setPen(QColor("#C17F3E"));
        p.drawText(QRectF(area.x(), yy - 8, padding - 10, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString("%1").arg((int)(maxVal * i / 4.0)));
    }

    // Bars
    double barW = qMin(50.0, (chartW / m_months.size()) - 16.0);
    for (int i = 0; i < m_months.size(); i++) {
        double barH = (m_months[i].total / maxVal) * chartH * ease;
        double bx = chartX + (chartW / m_months.size()) * i + (chartW / m_months.size() - barW) / 2.0;
        double by = chartY + chartH - barH;

        // Bar color by threshold
        QColor barColor;
        if (m_months[i].total > 200) barColor = QColor("#CC2200");
        else if (m_months[i].total > 100) barColor = QColor("#F59E0B");
        else if (m_months[i].total > 50) barColor = QColor("#C17F3E");
        else barColor = QColor("#4CAF7D");

        bool hovered = (i == m_hoveredBar);
        float opacity = hovered ? 1.0f : (m_hoveredBar >= 0 ? 0.5f : 1.0f);
        p.setOpacity(opacity);

        // 3D right face
        p.setPen(Qt::NoPen);
        p.setBrush(barColor.darker(180));
        QPolygonF rightF;
        rightF << QPointF(bx + barW, by) << QPointF(bx + barW + 6, by - 4)
               << QPointF(bx + barW + 6, chartY + chartH - 4) << QPointF(bx + barW, chartY + chartH);
        p.drawPolygon(rightF);

        // 3D top face
        p.setBrush(barColor.lighter(130));
        QPolygonF topF;
        topF << QPointF(bx, by) << QPointF(bx + 6, by - 4)
             << QPointF(bx + barW + 6, by - 4) << QPointF(bx + barW, by);
        p.drawPolygon(topF);

        // Front face
        QLinearGradient fg(bx, by, bx, chartY + chartH);
        fg.setColorAt(0, barColor);
        fg.setColorAt(1, barColor.darker(140));
        p.setBrush(fg);
        QPainterPath barPath;
        barPath.addRoundedRect(QRectF(bx, by, barW, barH), 4, 4);
        p.drawPath(barPath);

        // Value label
        QFont vf = p.font(); vf.setPixelSize(10); vf.setBold(true); p.setFont(vf);
        p.setPen(Qt::white);
        p.setBrush(QColor(0, 0, 0, 150));
        QRectF valRect(bx - 4, by - 20, barW + 8, 16);
        p.drawRoundedRect(valRect, 4, 4);
        p.drawText(valRect, Qt::AlignCenter, QString("%1").arg(m_months[i].total, 0, 'f', 0));

        // Month label
        p.setPen(QColor(245, 230, 211, 153));
        vf.setPixelSize(9); vf.setBold(false); p.setFont(vf);
        p.drawText(QRectF(bx - 4, chartY + chartH + 4, barW + 8, 16), Qt::AlignCenter, m_months[i].monthName);

        // Hover tooltip
        if (hovered) {
            QRectF tooltip(bx + barW + 14, by, 160, 60 + m_months[i].items.size() * 14);
            if (tooltip.right() > area.right()) tooltip.moveLeft(bx - 174);
            p.setBrush(QColor(13, 8, 5, 230));
            p.setPen(QPen(QColor("#C17F3E"), 1));
            p.drawRoundedRect(tooltip, 8, 8);

            vf.setPixelSize(11); vf.setBold(true); p.setFont(vf);
            p.setPen(QColor("#C17F3E"));
            p.drawText(tooltip.adjusted(8, 6, 0, 0), Qt::AlignLeft, m_months[i].monthName);
            p.drawText(tooltip.adjusted(8, 20, -8, 0), Qt::AlignLeft,
                       QString("Total: %1 dt").arg(m_months[i].total, 0, 'f', 0));

            vf.setPixelSize(9); vf.setBold(false); p.setFont(vf);
            p.setPen(QColor(245, 230, 211, 179));
            int ty = 38;
            for (int j = 0; j < qMin(5, m_months[i].items.size()); j++) {
                p.drawText(tooltip.adjusted(8, ty, -8, 0), Qt::AlignLeft,
                           QString("%1: %2 dt").arg(m_months[i].items[j].first.left(14))
                           .arg(m_months[i].items[j].second, 0, 'f', 1));
                ty += 14;
            }
        }

        p.setOpacity(1.0);
    }
}

void ForecastWidget::mousePressEvent(QMouseEvent *event) {
    // Period selector click
    QStringList periods = {"3M", "6M", "12M"};
    QList<int> values = {3, 6, 12};
    for (int i = 0; i < 3; i++) {
        QRectF btn(20 + i * 68, 10, 60, 36);
        if (btn.contains(event->pos())) {
            m_periodMonths = values[i];
            m_animProgress = 0.0f;
            recalculate();
            update();
            return;
        }
    }
}

void ForecastWidget::mouseMoveEvent(QMouseEvent *event) {
    int old = m_hoveredBar;
    m_hoveredBar = -1;
    if (!m_months.isEmpty()) {
        double chartX = 80;
        double chartW = width() - 80 - 20;
        for (int i = 0; i < m_months.size(); i++) {
            double bx = chartX + (chartW / m_months.size()) * i;
            double bw = chartW / m_months.size();
            if (event->x() >= bx && event->x() < bx + bw && event->y() > 56) {
                m_hoveredBar = i;
                break;
            }
        }
    }
    if (old != m_hoveredBar) update();
}

// ============================================================================
// ROI WIDGET
// ============================================================================
ROIWidget::ROIWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_globalTime += 0.016f;
        m_animProgress = qMin(1.0f, m_animProgress + 0.006f);
        update();
    });
    m_animTimer->start();
}

void ROIWidget::setData(const QList<EquipmentFinancials> &data) {
    m_data = data;
    std::sort(m_data.begin(), m_data.end(), [](const EquipmentFinancials &a, const EquipmentFinancials &b) {
        return a.roiScore > b.roiScore;
    });
    m_animProgress = 0.0f;
    update();
}

void ROIWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor(8, 5, 3));

    if (m_data.isEmpty()) {
        p.setPen(QColor("#C17F3E")); QFont f = p.font(); f.setPixelSize(14); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "No ROI data");
        return;
    }

    // Workshop gauge
    double avgRoi = 0;
    for (const auto &d : m_data) avgRoi += d.roiScore;
    avgRoi /= m_data.size();
    drawWorkshopGauge(p, QRectF(width() / 2 - 80, 10, 160, 100), avgRoi);

    // Cards
    int cy = 120;
    for (int i = 0; i < m_data.size() && cy < height() - 10; i++) {
        bool expanded = (i == m_expandedCard);
        bool hovered = (i == m_hoveredCard);
        int cardH = expanded ? 160 : 72;
        QRectF cardRect(20, cy, width() - 40, cardH);
        drawROICard(p, cardRect, m_data[i], i + 1, expanded, hovered);
        cy += cardH + 6;
    }
}

void ROIWidget::drawWorkshopGauge(QPainter &p, const QRectF &area, double avgScore) {
    QPointF center = area.center();
    double r = qMin(area.width(), area.height()) / 2.0 - 4;
    float ease = 1.0f - powf(1.0f - qMin(1.0f, m_animProgress * 2.0f), 3.0f);

    // Background arc
    p.setPen(QPen(QColor(26, 18, 8), 10, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(center.x() - r, center.y() - r, r * 2, r * 2), 90 * 16, 360 * 16);

    // Filled arc
    double span = (avgScore / 100.0) * 360 * ease;
    QColor gaugeCol = (avgScore >= 60) ? QColor("#4CAF7D") :
                       (avgScore >= 40) ? QColor("#C17F3E") : QColor("#CC2200");
    p.setPen(QPen(gaugeCol, 10, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(QRectF(center.x() - r, center.y() - r, r * 2, r * 2),
              90 * 16, -(int)(span * 16));

    // Center text
    QFont cf = p.font(); cf.setPixelSize(20); cf.setBold(true); p.setFont(cf);
    p.setPen(gaugeCol);
    p.drawText(QRectF(center.x() - 30, center.y() - 16, 60, 20), Qt::AlignCenter,
               QString("%1").arg((int)(avgScore * ease)));

    cf.setPixelSize(8); cf.setBold(false); p.setFont(cf);
    p.setPen(QColor(245, 230, 211, 128));
    p.drawText(QRectF(center.x() - 50, center.y() + 4, 100, 14), Qt::AlignCenter, "WORKSHOP ROI SCORE");
}

void ROIWidget::drawROICard(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq,
                             int rank, bool expanded, bool hovered) {
    float cardDelay = rank * 0.055f;
    float cardProgress = qMax(0.0f, qMin(1.0f, (m_animProgress - cardDelay) * 3.0f));
    float ease = 1.0f - powf(1.0f - cardProgress, 3.0f);

    if (ease < 0.01f) return;
    p.setOpacity(ease);

    // Shadow
    int sd = hovered ? 5 : 3;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, hovered ? 140 : 115));
    p.drawRoundedRect(rect.adjusted(sd, sd + 1, sd, sd + 1), 12, 12);

    // Card background
    QLinearGradient bg(rect.topLeft(), rect.bottomRight());
    bg.setColorAt(0, hovered ? QColor(30, 22, 12) : QColor(26, 18, 8));
    bg.setColorAt(1, QColor(10, 8, 4));
    p.setBrush(bg);
    p.setPen(QPen(QColor(193, 127, 62, hovered ? 60 : 30), 1));
    p.drawRoundedRect(rect, 12, 12);

    // Rank
    QFont rf = p.font(); rf.setPixelSize(24); rf.setBold(true); p.setFont(rf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(QRectF(rect.x() + 10, rect.y(), 50, rect.height()), Qt::AlignCenter, QString("#%1").arg(rank));

    // Medal for top 3
    if (rank <= 3) {
        QColor medalCol = (rank == 1) ? QColor("#FFD700") : (rank == 2) ? QColor("#C0C0C0") : QColor("#CD7F32");
        float rot = sinf(m_globalTime * 0.5f + rank) * 5.0f;
        p.save();
        QPointF mc(rect.x() + 35, rect.y() + 52);
        p.translate(mc);
        p.rotate(rot);
        QPainterPath star;
        for (int i = 0; i < 5; i++) {
            double a = i * 72.0 * M_PI / 180.0 - M_PI / 2;
            double ai = (i * 72.0 + 36.0) * M_PI / 180.0 - M_PI / 2;
            if (i == 0) star.moveTo(7 * cos(a), 7 * sin(a));
            else star.lineTo(7 * cos(a), 7 * sin(a));
            star.lineTo(3 * cos(ai), 3 * sin(ai));
        }
        star.closeSubpath();
        p.setPen(Qt::NoPen); p.setBrush(medalCol);
        p.drawPath(star);
        p.restore();
    }

    // Name + info
    QFont nf = p.font(); nf.setPixelSize(13); nf.setBold(true); p.setFont(nf);
    p.setPen(QColor(245, 230, 211));
    p.drawText(QRectF(rect.x() + 65, rect.y() + 10, rect.width() - 200, 20), Qt::AlignLeft | Qt::AlignVCenter, eq.name);

    nf.setPixelSize(10); nf.setBold(false); p.setFont(nf);
    p.setPen(QColor(245, 230, 211, 128));
    p.drawText(QRectF(rect.x() + 65, rect.y() + 28, rect.width() - 200, 16), Qt::AlignLeft,
               QString("Paid: %1dt | Age: %2yr").arg(eq.purchasePrice, 0, 'f', 0).arg(eq.ageInYears, 0, 'f', 1));

    // ROI progress bar
    QRectF barRect(rect.x() + 65, rect.y() + 48, rect.width() - 215, 8);
    p.setBrush(QColor(26, 18, 8));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(barRect, 4, 4);

    QColor barCol = (eq.roiScore >= 60) ? QColor("#4CAF7D") :
                     (eq.roiScore >= 40) ? QColor("#C17F3E") : QColor("#CC2200");
    double fillW = (eq.roiScore / 100.0) * barRect.width() * ease;
    QLinearGradient barFill(barRect.x(), 0, barRect.x() + fillW, 0);
    barFill.setColorAt(0, barCol.lighter(120)); barFill.setColorAt(1, barCol);
    p.setBrush(barFill);
    p.drawRoundedRect(QRectF(barRect.x(), barRect.y(), fillW, 8), 4, 4);

    // ROI score
    float scoreScale = 1.0f + 0.02f * sinf(m_globalTime * 1.5f + rank * 0.4f);
    QColor recCol(eq.recommendationColor);
    p.save();
    QPointF sc(rect.right() - 70, rect.y() + 24);
    p.translate(sc); p.scale(scoreScale, scoreScale); p.translate(-sc);
    QFont scf = p.font(); scf.setPixelSize(28); scf.setBold(true); p.setFont(scf);
    p.setPen(recCol);
    p.drawText(QRectF(rect.right() - 110, rect.y() + 8, 80, 32), Qt::AlignRight | Qt::AlignVCenter,
               QString("%1").arg((int)(eq.roiScore * ease)));
    scf.setPixelSize(11); p.setFont(scf);
    p.setPen(QColor(245, 230, 211, 102));
    p.drawText(QRectF(rect.right() - 30, rect.y() + 18, 25, 16), Qt::AlignLeft, "/100");
    p.restore();

    // Badge
    QFont bf = p.font(); bf.setPixelSize(9); bf.setBold(true); p.setFont(bf);
    QFontMetrics bfm(bf);
    int bw = bfm.horizontalAdvance(eq.recommendation) + 14;
    QRectF badge(rect.right() - bw - 8, rect.y() + 48, bw, 18);
    p.setBrush(QColor(recCol.red(), recCol.green(), recCol.blue(), 51));
    p.setPen(QPen(recCol, 1));
    p.drawRoundedRect(badge, 9, 9);
    p.setPen(recCol);
    p.drawText(badge, Qt::AlignCenter, eq.recommendation);

    // Expanded content
    if (expanded) {
        p.setPen(QPen(QColor(26, 18, 8), 1));
        p.drawLine(rect.x() + 20, rect.y() + 72, rect.right() - 20, rect.y() + 72);

        nf.setPixelSize(10); p.setFont(nf);
        p.setPen(QColor(245, 230, 211, 179));
        int ey = rect.y() + 82;
        p.drawText(rect.x() + 20, ey, QString("TCO: %1 dt | Cost/yr: %2 dt | Book Value: %3 dt")
                   .arg(eq.totalTCO, 0, 'f', 0).arg(eq.costPerYear, 0, 'f', 1).arg(eq.bookValue, 0, 'f', 0));
        ey += 18;
        p.drawText(rect.x() + 20, ey, QString("Depreciation: %1% | Remaining: %2 yr | Monthly Budget: %3 dt")
                   .arg(eq.depreciationPct, 0, 'f', 0).arg(eq.remainingYears).arg(eq.monthlyBudgetNeeded, 0, 'f', 1));
        ey += 18;
        p.setPen(recCol);
        p.drawText(rect.x() + 20, ey, QString("Verdict: %1 — %2").arg(eq.recommendation).arg(
                       eq.roiScore >= 60 ? "Strong performer, maintain investment" :
                       eq.roiScore >= 40 ? "Acceptable, monitor for changes" : "Action needed, review usage/costs"));
    }

    p.setOpacity(1.0);
}

void ROIWidget::mousePressEvent(QMouseEvent *event) {
    int cy = 120;
    for (int i = 0; i < m_data.size(); i++) {
        int cardH = (i == m_expandedCard) ? 160 : 72;
        if (event->y() >= cy && event->y() < cy + cardH) {
            m_expandedCard = (m_expandedCard == i) ? -1 : i;
            update();
            return;
        }
        cy += cardH + 6;
    }
}

void ROIWidget::mouseMoveEvent(QMouseEvent *event) {
    int old = m_hoveredCard;
    m_hoveredCard = -1;
    int cy = 120;
    for (int i = 0; i < m_data.size(); i++) {
        int cardH = (i == m_expandedCard) ? 160 : 72;
        if (event->y() >= cy && event->y() < cy + cardH) {
            m_hoveredCard = i;
            break;
        }
        cy += cardH + 6;
    }
    if (old != m_hoveredCard) update();
}

// ============================================================================
// TIMELINE WIDGET
// ============================================================================
TimelineWidget::TimelineWidget(QWidget *parent) : QWidget(parent) {
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_globalTime += 0.016f;
        m_animProgress = qMin(1.0f, m_animProgress + 0.006f);
        update();
    });
    m_animTimer->start();
}

void TimelineWidget::setData(const QList<EquipmentFinancials> &data) {
    m_data = data;
    std::sort(m_data.begin(), m_data.end(), [](const EquipmentFinancials &a, const EquipmentFinancials &b) {
        return a.remainingYears < b.remainingYears;
    });
    m_animProgress = 0.0f;
    update();
}

void TimelineWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor(8, 5, 3));

    if (m_data.isEmpty()) return;

    drawSummaryBanner(p, QRectF(20, 10, width() - 40, 56));
    drawTimeline(p, QRectF(20, 76, width() - 40, height() - 86));
}

void TimelineWidget::drawSummaryBanner(QPainter &p, const QRectF &area) {
    p.setBrush(QColor(26, 18, 8));
    p.setPen(QPen(QColor("#C17F3E"), 1));
    p.drawRoundedRect(area, 10, 10);

    double totalMonthly = 0;
    for (const auto &eq : m_data) totalMonthly += eq.monthlyBudgetNeeded;

    QFont lf = p.font(); lf.setPixelSize(12); lf.setBold(true); p.setFont(lf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(area.adjusted(16, 8, 0, -24), Qt::AlignLeft, QString::fromUtf8("\xF0\x9F\x92\xB0 MONTHLY SAVINGS TARGET"));

    lf.setPixelSize(18); p.setFont(lf);
    p.setPen(Qt::white);
    p.drawText(area.adjusted(16, 24, 0, -4), Qt::AlignLeft, QString("%1 dt per month").arg(totalMonthly, 0, 'f', 0));

    // Next replacement
    if (!m_data.isEmpty()) {
        p.setPen(QColor(245, 230, 211, 179));
        lf.setPixelSize(10); lf.setBold(false); p.setFont(lf);
        p.drawText(area.adjusted(0, 8, -16, -24), Qt::AlignRight,
                   QString("Next: %1 in %2 yr (%3 dt)")
                   .arg(m_data.first().name).arg(m_data.first().remainingYears)
                   .arg(m_data.first().replacementCost, 0, 'f', 0));
    }
}

void TimelineWidget::drawTimeline(QPainter &p, const QRectF &area) {
    float ease = 1.0f - powf(1.0f - m_animProgress, 3.0f);

    int infoW = 180;
    double timelineX = area.x() + infoW;
    double timelineW = area.width() - infoW;
    int maxYears = 30;
    int rowH = qMin(48, (int)(area.height() / qMax(1, m_data.size())));

    // Year headers
    QFont hf = p.font(); hf.setPixelSize(9); hf.setBold(true); p.setFont(hf);
    int currentYear = QDate::currentDate().year();
    for (int y = 0; y <= 10; y++) {
        double xPos = timelineX + (y / (double)maxYears) * timelineW;
        p.setPen(QPen(QColor(30, 21, 8), 1));
        p.drawLine(xPos, area.y(), xPos, area.y() + m_data.size() * rowH);
        p.setPen(QColor(193, 127, 62, 128));
        p.drawText(QRectF(xPos - 15, area.y() - 14, 30, 12), Qt::AlignCenter,
                   QString::number(currentYear + y));
    }

    // TODAY marker
    double todayX = timelineX;
    float todayOp = 0.7f + 0.3f * sinf(m_globalTime * 1.5f);
    p.setPen(QPen(QColor(193, 127, 62, (int)(255 * todayOp)), 1, Qt::DashLine));
    p.drawLine(todayX, area.y(), todayX, area.y() + m_data.size() * rowH);
    p.setPen(QColor("#C17F3E"));
    p.drawText(QRectF(todayX - 20, area.y() - 14, 40, 12), Qt::AlignCenter, "TODAY");

    // Urgency separators
    int urgentCount = 0, soonCount = 0;
    for (const auto &eq : m_data) {
        if (eq.remainingYears <= 2) urgentCount++;
        else if (eq.remainingYears <= 5) soonCount++;
    }

    // Rows
    for (int i = 0; i < m_data.size(); i++) {
        const auto &eq = m_data[i];
        double ry = area.y() + i * rowH;
        float rowDelay = i * 0.04f;
        float rowEase = qMax(0.0f, qMin(1.0f, (m_animProgress - rowDelay) * 3.0f));

        // Alternate background
        if (i % 2 == 0) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(17, 10, 6, 128));
            p.drawRect(QRectF(area.x(), ry, area.width(), rowH));
        }

        // Info area
        QFont nf = p.font(); nf.setPixelSize(11); nf.setBold(true); p.setFont(nf);
        p.setPen(QColor(245, 230, 211));
        p.drawText(QRectF(area.x() + 8, ry, infoW - 50, rowH / 2), Qt::AlignLeft | Qt::AlignVCenter,
                   eq.name.left(16));

        // Age badge
        QFont af = p.font(); af.setPixelSize(8); af.setBold(false); p.setFont(af);
        p.setPen(QColor("#C17F3E"));
        QRectF ageBadge(area.x() + 8, ry + rowH / 2 - 2, 32, 14);
        p.setBrush(QColor(193, 127, 62, 30));
        p.drawRoundedRect(ageBadge, 4, 4);
        p.drawText(ageBadge, Qt::AlignCenter, QString("%1yr").arg((int)eq.ageInYears));

        // Status dot
        QColor statusCol = (eq.status == "Available") ? QColor("#4CAF7D") :
                            (eq.status == "In Use") ? QColor("#3B82F6") :
                            (eq.status == "Under Maintenance") ? QColor("#F59E0B") : QColor("#CC2200");
        p.setPen(Qt::NoPen);
        p.setBrush(statusCol);
        p.drawEllipse(QPointF(infoW - 16, ry + rowH / 2), 3, 3);

        // Elapsed life bar (growing left from TODAY)
        double elapsedW = (eq.ageInYears / maxYears) * timelineW * rowEase;
        p.setBrush(QColor(193, 127, 62, 77));
        p.drawRect(QRectF(todayX - elapsedW, ry + 4, elapsedW, rowH - 8));

        // Remaining life bar (growing right from TODAY)
        QColor remainCol;
        if (eq.remainingYears > 10) remainCol = QColor(76, 175, 125, 153);
        else if (eq.remainingYears > 5) remainCol = QColor(193, 127, 62, 153);
        else if (eq.remainingYears > 2) remainCol = QColor(245, 158, 11, 179);
        else remainCol = QColor(204, 34, 0, 204);

        double remainW = (eq.remainingYears / (double)maxYears) * timelineW * rowEase;
        p.setBrush(remainCol);
        p.drawRect(QRectF(todayX, ry + 4, remainW, rowH - 8));

        // Replacement marker
        if (eq.remainingYears <= 20) {
            double markerX = todayX + remainW;
            p.setPen(QPen(remainCol, 1));
            p.drawLine(markerX, ry, markerX, ry + rowH);

            // Diamond
            p.setPen(Qt::NoPen); p.setBrush(remainCol);
            QPainterPath diamond;
            diamond.moveTo(markerX, ry + 2);
            diamond.lineTo(markerX + 4, ry + 6);
            diamond.lineTo(markerX, ry + 10);
            diamond.lineTo(markerX - 4, ry + 6);
            diamond.closeSubpath();
            p.drawPath(diamond);

            // Year label
            af.setPixelSize(8); p.setFont(af);
            p.setPen(remainCol);
            p.drawText(QRectF(markerX - 15, ry + rowH - 14, 30, 12), Qt::AlignCenter,
                       QString::number(currentYear + eq.remainingYears));
        }
    }
}

// ============================================================================
// REPORT WIDGET
// ============================================================================
ReportWidget::ReportWidget(QWidget *parent) : QWidget(parent) {
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_globalTime += 0.016f;
        if (m_isGenerating) {
            m_generateProgress = qMin(1.0f, m_generateProgress + 0.02f);
            if (m_generateProgress >= 1.0f) {
                m_isGenerating = false;
                m_isGenerated = true;
            }
        }
        update();
    });
    m_animTimer->start();

    // Title input
    m_titleInput = new QLineEdit(this);
    m_titleInput->setGeometry(340, 16, 320, 30);
    m_titleInput->setText(QString("Workshop Financial Report — %1").arg(QDate::currentDate().toString("yyyy-MM-dd")));
    m_titleInput->setStyleSheet(
        "QLineEdit { background: #0D0805; color: #F5E6D3; border: 1px solid #C17F3E; "
        "border-radius: 8px; padding: 4px 12px; font-size: 12px; }");

    // Section checkboxes
    QStringList sections = {"Executive Summary", "TCO Analysis", "ROI Rankings",
                            "Budget Forecast", "Replacement Timeline", "AI Recommendations"};
    int cy = 58;
    for (const auto &s : sections) {
        QCheckBox *cb = new QCheckBox(s, this);
        cb->setChecked(true);
        cb->setGeometry(340, cy, 220, 22);
        cb->setStyleSheet("QCheckBox { color: #F5E6D3; font-size: 11px; } "
                          "QCheckBox::indicator { width: 16px; height: 16px; } "
                          "QCheckBox::indicator:unchecked { border: 1px solid #C17F3E; background: #0D0805; border-radius: 3px; } "
                          "QCheckBox::indicator:checked { background: #C17F3E; border: 1px solid #C17F3E; border-radius: 3px; }");
        m_sectionChecks.append(cb);
        cy += 26;
    }

    // Generate button
    m_generateBtn = new QPushButton(QString::fromUtf8("\xE2\x96\xB6 GENERATE PREVIEW"), this);
    m_generateBtn->setGeometry(680, 16, 170, 30);
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    m_generateBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #C17F3E,stop:1 #8B4A1E); "
        "border: 1px solid #FFB060; color: white; font-weight: bold; font-size: 13px; border-radius: 8px; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #D09050,stop:1 #A06030); }");
    connect(m_generateBtn, &QPushButton::clicked, this, [this]() { generateReport(); });

    // Export button
    m_exportBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\x84 EXPORT PDF"), this);
    m_exportBtn->setGeometry(860, 16, 150, 30);
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2D6B3F,stop:1 #1A4025); "
        "border: 1px solid #4CAF7D; color: white; font-weight: bold; font-size: 13px; border-radius: 8px; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3D8B5F,stop:1 #2A5A35); }");
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() { exportPDF(); });
}

void ReportWidget::setData(const QList<EquipmentFinancials> &data) {
    m_data = data;
    update();
}

void ReportWidget::generateReport() {
    m_isGenerating = true;
    m_isGenerated = false;
    m_generateProgress = 0.0f;
}

void ReportWidget::exportPDF() {
    if (m_data.isEmpty()) return;

    QString filePath = QFileDialog::getSaveFileName(this, "Export Financial Report",
        QDir::homePath() + "/Workshop_Report.pdf", "PDF Files (*.pdf)");
    if (filePath.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);

    QTextDocument doc;
    QString html = "<html><head><style>"
        "body { font-family: Arial; color: #333; }"
        "h1 { color: #8B4A1E; border-bottom: 2px solid #C17F3E; padding-bottom: 8px; }"
        "h2 { color: #C17F3E; margin-top: 20px; }"
        "table { width: 100%; border-collapse: collapse; margin: 10px 0; }"
        "th { background: #8B4A1E; color: white; padding: 8px; text-align: left; }"
        "td { padding: 6px 8px; border-bottom: 1px solid #ddd; }"
        "tr:nth-child(even) { background: #f9f5f0; }"
        ".kpi { display: inline-block; padding: 10px; margin: 5px; border: 1px solid #C17F3E; border-radius: 8px; }"
        "</style></head><body>";

    html += "<h1>" + m_titleInput->text() + "</h1>";
    html += "<p>Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";

    // Executive Summary
    double totalVal = 0, totalTCO = 0;
    for (const auto &eq : m_data) { totalVal += eq.purchasePrice; totalTCO += eq.totalTCO; }
    html += "<h2>Executive Summary</h2>";
    html += QString("<p>Total Equipment: %1 | Total Value: %2 dt | Total TCO: %3 dt</p>")
            .arg(m_data.size()).arg(totalVal, 0, 'f', 0).arg(totalTCO, 0, 'f', 0);

    // TCO Table
    html += "<h2>TCO Analysis</h2>";
    html += "<table><tr><th>Equipment</th><th>Purchase</th><th>TCO</th><th>Cost/yr</th><th>ROI</th><th>Status</th></tr>";
    for (const auto &eq : m_data) {
        html += QString("<tr><td>%1</td><td>%2 dt</td><td>%3 dt</td><td>%4 dt</td><td>%5/100</td><td>%6</td></tr>")
                .arg(eq.name).arg(eq.purchasePrice, 0, 'f', 0).arg(eq.totalTCO, 0, 'f', 0)
                .arg(eq.costPerYear, 0, 'f', 1).arg(eq.roiScore, 0, 'f', 0).arg(eq.recommendation);
    }
    html += "</table>";

    html += "</body></html>";
    doc.setHtml(html);
    doc.print(&printer);
}

void ReportWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(rect(), QColor(8, 5, 3));

    // Keep classic layout: left preview, right controls
    if (m_titleInput) m_titleInput->setGeometry(340, 16, qMax(260, width() - 680), 30);
    if (m_generateBtn) m_generateBtn->setGeometry(width() - 340, 16, 160, 30);
    if (m_exportBtn) m_exportBtn->setGeometry(width() - 170, 16, 150, 30);
    for (int i = 0; i < m_sectionChecks.size(); ++i) {
        m_sectionChecks[i]->setGeometry(340, 58 + i * 26, qMax(220, width() - 360), 22);
    }

    drawPreview(p, QRectF(20, 16, 300, height() - 36));

    // Labels
    QFont lf = p.font(); lf.setPixelSize(10); lf.setBold(true);
    lf.setLetterSpacing(QFont::AbsoluteSpacing, 2); p.setFont(lf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(340, 12, "REPORT TITLE");
    p.drawText(340, 52, "SECTIONS");

    // Generation progress
    if (m_isGenerating) {
        QRectF progRect(width() - 230, height() - 44, 210, 26);
        p.setBrush(QColor(26, 18, 8));
        p.setPen(QPen(QColor("#C17F3E"), 1));
        p.drawRoundedRect(progRect, 6, 6);
        p.setBrush(QColor("#C17F3E"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(progRect.x() + 2, progRect.y() + 2,
                                 (progRect.width() - 4) * m_generateProgress, progRect.height() - 4), 4, 4);
        lf.setPixelSize(10); p.setFont(lf);
        p.setPen(Qt::white);
        p.drawText(progRect, Qt::AlignCenter, "Generating...");
    }

    if (m_isGenerated) {
        p.setPen(QColor("#4CAF7D"));
        lf.setPixelSize(12); lf.setBold(true); p.setFont(lf);
        p.drawText(QRectF(20, height() - 44, 420, 24), Qt::AlignVCenter,
                   QString::fromUtf8("\xE2\x9C\x93 Report Generated Successfully"));
    }
}

void ReportWidget::drawPreview(QPainter &p, const QRectF &area) {
    // Panel background
    p.setPen(QPen(QColor(193, 127, 62, 110), 1));
    p.setBrush(QColor(18, 12, 7, 200));
    p.drawRoundedRect(area, 10, 10);

    if (m_data.isEmpty()) {
        p.setPen(QColor(100, 80, 60));
        QFont f = p.font(); f.setPixelSize(10); p.setFont(f);
        p.drawText(area, Qt::AlignCenter, "Report Preview");
        return;
    }

    const QRectF paperRect(area.x() + 20, area.y() + 20, area.width() - 40, area.height() - 40);

    // Paper page
    p.setBrush(QColor(250, 245, 235));
    p.setPen(QPen(QColor(193, 127, 62), 1));
    p.drawRoundedRect(paperRect, 5, 5);

    p.save();
    p.translate(paperRect.topLeft());

    const qreal sx = paperRect.width() / 420.0;
    const qreal sy = paperRect.height() / 760.0;
    p.scale(sx, sy);

    // Mini report content
    p.setPen(QColor(139, 74, 30));
    QFont hf = p.font(); hf.setPixelSize(16); hf.setBold(true); p.setFont(hf);
    p.drawText(20, 30, "Workshop Financial Report");

    p.setPen(QPen(QColor("#C17F3E"), 2));
    p.drawLine(20, 38, 400, 38);

    hf.setPixelSize(10); hf.setBold(false); p.setFont(hf);
    p.setPen(QColor(80, 60, 40));
    p.drawText(20, 55, QDate::currentDate().toString("MMMM d, yyyy"));

    // Mini table
    int ty = 75;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(139, 74, 30));
    p.drawRect(20, ty, 380, 14);
    p.setPen(Qt::white);
    hf.setPixelSize(7); hf.setBold(true); p.setFont(hf);
    p.drawText(22, ty + 10, "Equipment");
    p.drawText(160, ty + 10, "TCO");
    p.drawText(240, ty + 10, "ROI");
    p.drawText(310, ty + 10, "Status");

    ty += 16;
    hf.setBold(false); p.setFont(hf);
    for (int i = 0; i < qMin(8, m_data.size()); i++) {
        if (i % 2 == 0) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(240, 230, 220));
            p.drawRect(20, ty, 380, 12);
        }
        p.setPen(QColor(60, 40, 20));
        p.drawText(22, ty + 9, m_data[i].name.left(20));
        p.drawText(160, ty + 9, QString::number((int)m_data[i].totalTCO));
        p.drawText(240, ty + 9, QString::number((int)m_data[i].roiScore));
        p.drawText(310, ty + 9, m_data[i].recommendation);
        ty += 12;
    }

    p.restore();
}
