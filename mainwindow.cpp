#include <QDialog>
#include <QGraphicsBlurEffect>
#include <QFrame>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QProgressBar>
#include <QTimer>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QUrl>
#include <QBuffer>
#include <QMovie>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFileDialog>
#include <QWheelEvent>
#include <QtMath>

#include "mainwindow.h"
#include "smtpsender.h"
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QStatusBar>
#include "welcomenotificationbar.h"
#include "ui_mainwindow.h"
#include "ui_client_management.h"
#include "ui_employee_management.h"
#include "ui_equipment_management.h"
#include "ui_order_management.h"
#include "ui_supplier_management.h"
#include "weatherassistant.h"
#include "chatbotdialog.h"
#include "buttonanimator.h"
#include "modelingwidget.h"
#include "imagedropzone.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QScrollArea>
#include <QShortcut>
#include <QButtonGroup>
#include <QRadioButton>
#include <QAbstractButton>
#include <QSplineSeries>
#include <QCalendarWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QGroupBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QMenu>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QToolTip>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QCursor>
#include <QLocale>
#include <QFileInfo>
#include <QProcess>
#include <QUuid>
#include <functional>
#include <algorithm>
// =============================================================================
// ANIMATED DONUT CHART WIDGET
// =============================================================================
AnimatedDonutChart::AnimatedDonutChart(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
}

void AnimatedDonutChart::setData(const QList<DataPoint> &data) {
    m_data = data;
    update();
}

void AnimatedDonutChart::startAnimation() {
    QPropertyAnimation *anim = new QPropertyAnimation(this, "animationValue");
    anim->setDuration(1200);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuart);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedDonutChart::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double total = 0;
    for (const auto &d : m_data) total += d.value;
    if (total == 0) return;

    QRectF contentRect = rect().adjusted(20, 20, -20, -20);
    int size = qMin((int)contentRect.width(), (int)contentRect.height());
    contentRect = QRectF(contentRect.center().x() - size/2, contentRect.center().y() - size/2, size, size);

    double startAngle = 90; // Start from top
    for (int i = 0; i < m_data.size(); ++i) {
        double span = (m_data[i].value / total) * 360.0 * m_animationValue;
        
        QRadialGradient grad(contentRect.center(), size/2);
        grad.setColorAt(0.5, m_data[i].color.lighter(120));
        grad.setColorAt(1.0, m_data[i].color);

        p.setBrush(grad);
        p.setPen(QPen(QColor(40,40,40), 2));
        
        if (i == m_hoveredIndex) {
            p.setBrush(m_data[i].color.lighter(140));
            QRectF offsetRect = contentRect.adjusted(-5, -5, 5, 5);
            p.drawPie(offsetRect, startAngle * 16, -span * 16);
        } else {
            p.drawPie(contentRect, startAngle * 16, -span * 16);
        }
        
        startAngle -= (m_data[i].value / total) * 360.0;
    }

    // Mask for Donut
    p.setBrush(QColor(30, 20, 10)); // Match background
    p.setPen(QPen(QColor(139, 111, 71), 2));
    p.drawEllipse(contentRect.center(), size/4, size/4);
}

void AnimatedDonutChart::mouseMoveEvent(QMouseEvent *event) {
    double total = 0;
    for (const auto &d : m_data) total += d.value;
    if (total == 0) return;

    QRectF contentRect = this->rect().adjusted(20, 20, -20, -20);
    int size = qMin((int)contentRect.width(), (int)contentRect.height());
    contentRect = QRectF(contentRect.center().x() - size/2, contentRect.center().y() - size/2, size, size);

    QPointF center = contentRect.center();
    QPointF pos = event->position();
    double dist = QLineF(center, pos).length();

    if (dist < size/4 || dist > size/2) {
        m_hoveredIndex = -1;
        QToolTip::hideText();
        update();
        return;
    }

    double angle = QLineF(center, pos).angle(); // 0-360, 0 is right
    // Normalize angle to start from 90 (top) and go counter-clockwise
    double normalizedAngle = 90 - angle;
    if (normalizedAngle < 0) normalizedAngle += 360;

    double current = 0;
    int oldHover = m_hoveredIndex;
    m_hoveredIndex = -1;

    for (int i = 0; i < m_data.size(); ++i) {
        double span = (m_data[i].value / total) * 360.0;
        if (normalizedAngle >= current && normalizedAngle < current + span) {
            m_hoveredIndex = i;
            m_tooltipText = QString("%1: %2 (%3%)")
                .arg(m_data[i].label)
                .arg(m_data[i].value)
                .arg(qRound(m_data[i].value / total * 100));
            QToolTip::showText(event->globalPosition().toPoint(), m_tooltipText, this);
            break;
        }
        current += span;
    }

    if (oldHover != m_hoveredIndex) update();
}

bool AnimatedDonutChart::event(QEvent *event) {
    if (event->type() == QEvent::Leave) {
        m_hoveredIndex = -1;
        update();
    }
    return QWidget::event(event);
}

// =============================================================================
// STAT CARD WIDGET
// =============================================================================
StatCard::StatCard(const QString &title, const QString &value, const QString &trend, bool isUp, QWidget *parent) 
    : QFrame(parent) {
    setFixedSize(200, 100);
    setObjectName("statCard");
    setStyleSheet(
        "QFrame#statCard { background: rgba(50, 40, 30, 0.6); border: 2px solid #8B6F47; border-radius: 16px; }"
        "QFrame#statCard:hover { background: rgba(80, 60, 40, 0.7); border-color: #D4AF37; margin: -2px; }"
    );

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(15, 12, 15, 12);
    lay->setSpacing(2);

    m_titleLbl = new QLabel(title, this);
    m_titleLbl->setStyleSheet("color: #B8925A; font-size: 13px; font-weight: bold; border:none;");
    lay->addWidget(m_titleLbl);

    m_valLbl = new QLabel(value, this);
    m_valLbl->setStyleSheet("color: white; font-size: 24px; font-weight: bold; border:none;");
    lay->addWidget(m_valLbl);

    m_trendLbl = new QLabel(this);
    updateData(value, trend, isUp);
    lay->addWidget(m_trendLbl);
}

void StatCard::updateData(const QString &value, const QString &trendText, bool isUp) {
    m_valLbl->setText(value);
    QString arrow = isUp ? "↑" : "↓";
    QString color = isUp ? "#4CAF50" : "#FF5252";
    m_trendLbl->setText(QString("%1 %2 from last month").arg(arrow).arg(trendText));
    m_trendLbl->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: bold; border:none;").arg(color));
}

namespace {

struct EquipmentRankingEntry {
    int id = 0;
    QString name;
    QString type;
    QString status;
    int quantity = 0;
    double unitPrice = 0.0;
};

struct EquipmentStatsVisualData {
    int total = 0;
    int available = 0;
    int inUse = 0;
    int maintenance = 0;
    int retired = 0;
    double totalValue = 0.0;

    int lastMonthTotal = 0;
    double lastMonthValue = 0.0;

    QVector<EquipmentRankingEntry> ranking;
    QStringList months;
    QVector<int> monthlyAdded;
    QVector<int> monthlyMaintenance;

    QString mostActiveName;
    int mostActiveEvents = 0;

    QString newestName;
    QDate newestDate;
    int newestDays = 0;

    double avgAgeYears = 0.0;
    QString oldestName;
    QString newestAgeName;

    QVector<double> radarScores;
};

static qreal clamp01(qreal v) {
    return qBound<qreal>(0.0, v, 1.0);
}

static qreal easeSin(qreal t) {
    return qSin(clamp01(t) * M_PI_2);
}

class EquipmentStatsCanvas final : public QWidget {
public:
    explicit EquipmentStatsCanvas(QWidget *parent = nullptr) : QWidget(parent) {
        setMouseTracking(true);
        m_frameTimer.setInterval(16);
        connect(&m_frameTimer, &QTimer::timeout, this, [this]() {
            m_globalTime += 0.016;
            update();
        });
        m_frameTimer.start();
        restartAnimations();
    }

    void setData(const EquipmentStatsVisualData &data) {
        m_data = data;
        restartAnimations();
        update();
    }

    void restartAnimations() {
        m_bootMs = QDateTime::currentMSecsSinceEpoch();
    }

protected:
    void mouseMoveEvent(QMouseEvent *event) override {
        const QPointF pos = event->position();
        m_hoveredPanel = -1;
        for (int i = 0; i < m_panelRects.size(); ++i) {
            if (m_panelRects[i].contains(pos)) {
                m_hoveredPanel = i;
                break;
            }
        }

        int oldSeg = m_hoveredSegment;
        m_hoveredSegment = -1;
        if (m_donutOuter > 1.0 && m_donutInner > 1.0) {
            const qreal d = QLineF(m_donutCenter, pos).length();
            if (d >= m_donutInner && d <= m_donutOuter + 16.0) {
                qreal angle = QLineF(m_donutCenter, pos).angle();
                qreal fromTopCW = 90.0 - angle;
                if (fromTopCW < 0) fromTopCW += 360.0;
                for (int i = 0; i < m_segmentRanges.size(); ++i) {
                    const qreal s = m_segmentRanges[i].first;
                    const qreal e = m_segmentRanges[i].second;
                    if (fromTopCW >= s && fromTopCW < e) {
                        m_hoveredSegment = i;
                        break;
                    }
                }
            }
        }

        if (m_hoveredSegment >= 0 && m_hoveredSegment < 4) {
            const int total = qMax(1, m_data.total + m_data.retired);
            const int value = (m_hoveredSegment == 0) ? m_data.available
                            : (m_hoveredSegment == 1) ? m_data.inUse
                            : (m_hoveredSegment == 2) ? m_data.maintenance
                            : m_data.retired;
            const QString name = (m_hoveredSegment == 0) ? "Available"
                               : (m_hoveredSegment == 1) ? "In Use"
                               : (m_hoveredSegment == 2) ? "Maintenance"
                               : "Retired";
            const qreal pct = 100.0 * value / total;
            QToolTip::showText(event->globalPosition().toPoint(), QString("%1: %2 (%3%)").arg(name).arg(value).arg(QString::number(pct, 'f', 1)), this);
        } else {
            QToolTip::hideText();
        }

        if (oldSeg != m_hoveredSegment) {
            update();
        }
        QWidget::mouseMoveEvent(event);
    }

    void leaveEvent(QEvent *event) override {
        m_hoveredPanel = -1;
        m_hoveredSegment = -1;
        QToolTip::hideText();
        update();
        QWidget::leaveEvent(event);
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::TextAntialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);

        const QRectF canvas = rect();
        drawBackground(p, canvas);

        const QRectF content = canvas.adjusted(16, 46, -16, -14);
        const QRectF header(content.left(), content.top(), content.width(), 112.0);
        drawHeader(p, header);

        const qreal gap = 8.0;
        const qreal cardsH = 90.0;
        const qreal topY = header.bottom() + 10.0;
        const qreal usableH = content.bottom() - topY;
        const qreal gridH = qMax<qreal>(240.0, usableH - cardsH - 12.0);
        const qreal panelW = (content.width() - gap) * 0.5;
        const qreal panelH = (gridH - gap) * 0.5;

        m_panelRects.clear();
        m_panelRects << QRectF(content.left(), topY, panelW, panelH)
                     << QRectF(content.left() + panelW + gap, topY, panelW, panelH)
                     << QRectF(content.left(), topY + panelH + gap, panelW, panelH)
                     << QRectF(content.left() + panelW + gap, topY + panelH + gap, panelW, panelH);

        drawPanelDonut(p, m_panelRects[0]);
        drawPanelRanking(p, m_panelRects[1]);
        drawPanelTimeline(p, m_panelRects[2]);
        drawPanelRadar(p, m_panelRects[3]);

        const qreal cardY = topY + gridH + 12.0;
        const qreal cardW = (content.width() - 2.0 * gap) / 3.0;
        drawBottomCards(p,
                        QRectF(content.left(), cardY, cardW, cardsH),
                        QRectF(content.left() + cardW + gap, cardY, cardW, cardsH),
                        QRectF(content.left() + 2.0 * (cardW + gap), cardY, cardW, cardsH));
    }

private:
    qreal elapsedSec() const {
        return (QDateTime::currentMSecsSinceEpoch() - m_bootMs) / 1000.0;
    }

    void drawBackground(QPainter &p, const QRectF &r) {
        QRadialGradient deep(r.center(), qMax(r.width(), r.height()) * 0.62);
        deep.setColorAt(0.0, QColor("#1A0E06"));
        deep.setColorAt(0.40, QColor("#0F0804"));
        deep.setColorAt(1.0, QColor("#000000"));
        p.fillRect(r, deep);

        p.setPen(QPen(QColor(139, 90, 43, 8), 1));
        const int lines = 55;
        for (int i = 0; i < lines; ++i) {
            const qreal yy = r.top() + (r.height() * i) / (lines - 1.0);
            QPainterPath path;
            path.moveTo(r.left(), yy);
            for (qreal x = r.left(); x <= r.right(); x += 14.0) {
                const qreal y = yy + qSin(x * 0.012 + i * 0.25) * 2.0;
                path.lineTo(x, y);
            }
            p.drawPath(path);
        }

        for (int i = 0; i < 180; ++i) {
            const qreal seed = i * 27.0;
            const qreal speed = 10.0 + (i % 7) * 3.0;
            qreal y = r.bottom() - std::fmod(m_globalTime * speed + seed, r.height() + 40.0);
            qreal x = r.left() + (i * 43 % int(r.width())) + qSin(m_globalTime * 0.18 + i * 0.85) * 1.2;
            const qreal sz = 1.0 + (i % 2);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(193, 127, 62, 13));
            p.drawEllipse(QPointF(x, y), sz, sz);
        }

        const qreal b = 0.5 + 0.5 * qSin(m_globalTime * 0.22);
        auto drawGlow = [&](const QPointF &c, qreal radius, const QColor &col, qreal alphaScale) {
            QColor cc = col;
            cc.setAlphaF(alphaScale * (0.65 + 0.35 * b));
            QRadialGradient g(c, radius);
            g.setColorAt(0.0, cc);
            QColor t = cc;
            t.setAlpha(0);
            g.setColorAt(1.0, t);
            p.setBrush(g);
            p.setPen(Qt::NoPen);
            p.drawEllipse(c, radius, radius);
        };

        drawGlow(QPointF(r.right() - r.width() * 0.18, r.top() + r.height() * 0.18), r.width() * 0.40, QColor("#C17F3E"), 0.05);
        drawGlow(QPointF(r.left() + r.width() * 0.22, r.bottom() - r.height() * 0.18), r.width() * 0.32, QColor("#8B5A2B"), 0.03);
        drawGlow(r.center(), r.width() * 0.50, QColor("#C17F3E"), 0.03);
    }

    void drawHeader(QPainter &p, const QRectF &rect) {
        QLinearGradient g(rect.topLeft(), rect.bottomRight());
        g.setColorAt(0.0, QColor("#1E1108"));
        g.setColorAt(1.0, QColor("#0D0805"));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawRoundedRect(rect, 12, 12);

        QLinearGradient border(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        border.setColorAt(0.0, QColor(0, 0, 0, 0));
        border.setColorAt(0.35, QColor("#C17F3E"));
        border.setColorAt(0.55, QColor("#FF6600"));
        border.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.setPen(QPen(border, 1.5));
        p.drawLine(QPointF(rect.left() + 14, rect.bottom() - 1), QPointF(rect.right() - 14, rect.bottom() - 1));

        const QPointF symCenter(rect.left() + 26, rect.top() + 32);
        QPainterPath oct;
        const qreal rr = 10.0;
        const qreal rot = m_globalTime * 15.0;
        for (int i = 0; i < 8; ++i) {
            qreal a = qDegreesToRadians(rot + i * 45.0 - 90.0);
            QPointF pt(symCenter.x() + rr * qCos(a), symCenter.y() + rr * qSin(a));
            if (i == 0) oct.moveTo(pt); else oct.lineTo(pt);
        }
        oct.closeSubpath();
        QRadialGradient og(symCenter, 18);
        og.setColorAt(0.0, QColor("#FFD700"));
        og.setColorAt(1.0, QColor("#C17F3E"));
        p.setBrush(og);
        p.setPen(QPen(QColor(255, 215, 0, 130), 1));
        p.drawPath(oct);

        QRadialGradient glow(symCenter, 18);
        glow.setColorAt(0.0, QColor(193, 127, 62, 52));
        glow.setColorAt(1.0, QColor(193, 127, 62, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(glow);
        p.drawEllipse(symCenter, 18, 18);

        const QString title = QString::fromUtf8("◈ WORKSHOP STATISTICS");
        const QPointF tpos(rect.left() + 46, rect.top() + 48);
        QFont tf("Segoe UI", 22, QFont::Bold);
        p.setFont(tf);
        p.setPen(QColor(0, 0, 0, 217));
        p.drawText(tpos + QPointF(3, 4), title);
        p.setPen(QColor(107, 58, 26, 153));
        p.drawText(tpos + QPointF(1, 2), title);
        p.setPen(QColor("#C17F3E"));
        p.drawText(tpos, title);

        QFontMetrics fm(tf);
        QRect tr = fm.boundingRect(title);
        QRectF clipRect(tpos.x(), tpos.y() - tr.height() + 2, tr.width(), tr.height() * 0.38);
        p.save();
        p.setClipRect(clipRect);
        p.setPen(QColor(255, 212, 160, 102));
        p.drawText(tpos, title);
        p.restore();

        QFont sf("Segoe UI", 10, QFont::DemiBold);
        sf.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
        p.setFont(sf);
        QColor sc("#F5E6D3");
        sc.setAlphaF(0.35 + 0.15 * (0.5 + 0.5 * qSin(m_globalTime * 0.4)));
        p.setPen(sc);
        p.drawText(QPointF(rect.left() + 48, rect.top() + 76), "REAL-TIME EQUIPMENT INTELLIGENCE");

        const qreal uw = tr.width() * (0.5 + 0.5 * qSin(m_globalTime * 0.7));
        const qreal ux = rect.left() + 48 + tr.width() * 0.5 - uw * 0.5;
        QLinearGradient ug(ux, 0, ux + uw, 0);
        ug.setColorAt(0.0, QColor(0, 0, 0, 0));
        ug.setColorAt(0.45, QColor("#C17F3E"));
        ug.setColorAt(0.75, QColor("#FFD700"));
        ug.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.setPen(QPen(ug, 1.5));
        p.drawLine(QPointF(ux, rect.top() + 84), QPointF(ux + uw, rect.top() + 84));

        const QVector<QColor> kpiColors = {QColor("#C17F3E"), QColor("#4CAF7D"), QColor("#F59E0B"), QColor("#FFD700")};
        const QVector<QString> labels = {"TOTAL EQUIPMENT", "AVAILABLE NOW", "IN MAINTENANCE", "TOTAL VALUE (dt)"};
        const QVector<double> values = {
            (double)(m_data.total + m_data.retired),
            (double)m_data.available,
            (double)m_data.maintenance,
            m_data.totalValue
        };

        const qreal boxW = 160.0;
        const qreal boxH = 64.0;
        const qreal startX = rect.right() - (boxW * 4.0 + 24.0);
        const qreal y = rect.bottom() - boxH - 10.0;
        const qreal t = elapsedSec();
        const qreal cnt = easeSin(t / 1.4);

        for (int i = 0; i < 4; ++i) {
            QRectF b(startX + i * boxW + i * 8, y, boxW, boxH);
            QColor c = kpiColors[i];
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 153));
            p.drawRoundedRect(b.translated(2, 3), 10, 10);

            QLinearGradient bg(b.topLeft(), b.bottomRight());
            bg.setColorAt(0.0, QColor("#1E1510"));
            bg.setColorAt(1.0, QColor("#0D0805"));
            p.setBrush(bg);
            p.setPen(QPen(QColor(193, 127, 62, 130), 1));
            p.drawRoundedRect(b, 10, 10);

            p.fillRect(QRectF(b.left() + 1, b.top() + 1, b.width() - 2, 2), c);
            p.fillRect(QRectF(b.left() + 1, b.bottom() - 3, b.width() - 2, 3), QColor(0, 0, 0, 130));
            p.fillRect(QRectF(b.right() - 2, b.top() + 2, 2, b.height() - 4), QColor(0, 0, 0, 120));
            p.fillRect(QRectF(b.left() + 1, b.top() + 1, b.width() * 0.55, 1), QColor(245, 230, 211, 35));

            if (i < 3) {
                QLinearGradient sep(b.right() + 4, b.top(), b.right() + 4, b.bottom());
                sep.setColorAt(0.0, QColor(0, 0, 0, 0));
                sep.setColorAt(0.5, QColor(193, 127, 62, 50));
                sep.setColorAt(1.0, QColor(0, 0, 0, 0));
                p.setPen(QPen(sep, 1));
                p.drawLine(QPointF(b.right() + 4, b.top() + 8), QPointF(b.right() + 4, b.bottom() - 8));
            }

            QFont lf("Segoe UI", 8, QFont::DemiBold);
            lf.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
            p.setFont(lf);
            p.setPen(QColor(245, 230, 211, 170));
            p.drawText(QRectF(b.left() + 28, b.top() + 8, b.width() - 34, 14), Qt::AlignLeft | Qt::AlignVCenter, labels[i]);

            const qreal breath = 1.0 + 0.015 * qSin(m_globalTime * 1.2 + i);
            QFont nf("Segoe UI", int(22 * breath), QFont::Bold);
            p.setFont(nf);
            p.setPen(c);
            const qreal shown = values[i] * cnt;
            const QString txt = (i == 3) ? QString::number(shown, 'f', 0) : QString::number((int)qRound(shown));
            p.drawText(QRectF(b.left() + 26, b.top() + 22, b.width() - 30, 34), Qt::AlignLeft | Qt::AlignVCenter, txt);

            p.setPen(c);
            p.setBrush(Qt::NoBrush);
            if (i == 0) {
                QPainterPath hammer;
                hammer.moveTo(b.left() + 10, b.top() + 43);
                hammer.lineTo(b.left() + 15, b.top() + 36);
                hammer.lineTo(b.left() + 18, b.top() + 38);
                hammer.lineTo(b.left() + 13, b.top() + 45);
                hammer.addRect(QRectF(b.left() + 9, b.top() + 32, 9, 3));
                p.drawPath(hammer);
            } else if (i == 1) {
                p.drawEllipse(QPointF(b.left() + 14, b.top() + 40), 6, 6);
                p.drawLine(QPointF(b.left() + 11, b.top() + 40), QPointF(b.left() + 13, b.top() + 42));
                p.drawLine(QPointF(b.left() + 13, b.top() + 42), QPointF(b.left() + 17, b.top() + 38));
            } else if (i == 2) {
                p.drawLine(QPointF(b.left() + 9, b.top() + 44), QPointF(b.left() + 18, b.top() + 35));
                p.drawEllipse(QPointF(b.left() + 19, b.top() + 34), 2, 2);
                p.drawLine(QPointF(b.left() + 11, b.top() + 41), QPointF(b.left() + 14, b.top() + 44));
            } else {
                QPainterPath d;
                d.moveTo(b.left() + 14, b.top() + 31);
                d.lineTo(b.left() + 19, b.top() + 36);
                d.lineTo(b.left() + 14, b.top() + 41);
                d.lineTo(b.left() + 9, b.top() + 36);
                d.closeSubpath();
                p.drawPath(d);
            }
        }
    }

    void drawPanelFrame(QPainter &p, const QRectF &r, const QString &title, const QColor &accent, const QString &sub, int panelIndex) {
        const qreal appear = easeSin((elapsedSec() - panelIndex * 0.08) / 0.4);
        p.save();
        p.setOpacity(appear);

        const bool hovered = (panelIndex == m_hoveredPanel);
        const QPointF sh = hovered ? QPointF(6, 9) : QPointF(4, 6);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, hovered ? 153 : 127));
        p.drawRoundedRect(r.translated(sh), 16, 16);

        QLinearGradient bg(r.topLeft(), r.bottomRight());
        bg.setColorAt(0.0, QColor("#1A1208"));
        bg.setColorAt(1.0, QColor("#0A0804"));
        p.setBrush(bg);
        p.setPen(QPen(QColor(193, 127, 62, hovered ? 115 : 64), 1));
        p.drawRoundedRect(r, 16, 16);

        QLinearGradient ag(r.left(), r.top(), r.right(), r.top());
        ag.setColorAt(0.0, QColor(0, 0, 0, 0));
        ag.setColorAt(0.5, accent);
        ag.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.fillRect(QRectF(r.left() + 8, r.top() + 6, r.width() - 16, 2), ag);
        QColor glow = accent; glow.setAlpha(40);
        p.fillRect(QRectF(r.left() + 8, r.top() + 8, r.width() - 16, 12), glow);

        p.setFont(QFont("Segoe UI", 12, QFont::Bold));
        p.setPen(QColor("#F5E6D3"));
        p.drawText(QRectF(r.left() + 14, r.top() + 12, r.width() - 130, 18), Qt::AlignLeft | Qt::AlignVCenter, title);
        p.setPen(accent);
        p.drawText(QRectF(r.right() - 120, r.top() + 12, 106, 18), Qt::AlignRight | Qt::AlignVCenter, sub);
        p.setPen(QPen(QColor("#1E1508"), 1));
        p.drawLine(QPointF(r.left() + 10, r.top() + 36), QPointF(r.right() - 10, r.top() + 36));
        p.restore();
    }

    void drawPanelDonut(QPainter &p, const QRectF &panel) {
        const int all = m_data.total + m_data.retired;
        drawPanelFrame(p, panel, "STATUS DISTRIBUTION", QColor("#C17F3E"), QString::number(all), 0);

        const QRectF body = panel.adjusted(12, 42, -12, -10);
        const QRectF chartRect(body.left() + 6, body.top() + 6, body.width() * 0.62, body.height() - 12);
        const QPointF c = chartRect.center();
        const qreal r = qMin(chartRect.width(), chartRect.height()) * 0.42;
        const qreal inner = r * 0.52;

        m_donutCenter = c;
        m_donutInner = inner;
        m_donutOuter = r;
        m_segmentRanges.clear();

        const QVector<int> vals = {m_data.available, m_data.inUse, m_data.maintenance, m_data.retired};
        const QVector<QString> names = {"Available", "In Use", "Maintenance", "Retired"};
        const QVector<QColor> colors = {QColor("#4CAF7D"), QColor("#C17F3E"), QColor("#F59E0B"), QColor("#6B7280")};
        const qreal total = qMax(1, all);

        qreal consumed = 0.0;
        for (int i = 0; i < vals.size(); ++i) {
            const qreal span = 360.0 * vals[i] / total;
            const qreal segProg = easeSin((elapsedSec() - i * 0.1) / 0.35);
            const qreal visSpan = span * segProg;
            const qreal start = consumed;
            const qreal mid = start + visSpan * 0.5;
            const qreal hoverOut = (i == m_hoveredSegment) ? 10.0 : 0.0;
            const qreal dim = (m_hoveredSegment >= 0 && m_hoveredSegment != i) ? 0.65 : 1.0;

            m_segmentRanges.push_back({consumed, consumed + span});

            const qreal theta = qDegreesToRadians(-90.0 + mid);
            const QPointF segC(c.x() + qCos(theta) * hoverOut, c.y() + qSin(theta) * hoverOut);

            QRectF baseOuter(segC.x() - (r + 10), segC.y() - (r + 10), 2 * (r + 10), 2 * (r + 10));
            QRectF baseInner(segC.x() - inner, segC.y() - inner, 2 * inner, 2 * inner);
            QPainterPath baseRing;
            baseRing.addEllipse(baseOuter.translated(5, 7));
            baseRing.addEllipse(baseInner.translated(5, 7));
            baseRing.setFillRule(Qt::OddEvenFill);
            p.save();
            p.setClipPath(baseRing);
            QColor sh = colors[i];
            sh.setAlphaF(0.30 * dim);
            p.setBrush(sh);
            p.setPen(Qt::NoPen);
            p.drawPie(baseOuter.translated(5, 7), qRound((90 - start) * 16), -qRound(visSpan * 16));
            p.restore();

            QRectF o(segC.x() - r, segC.y() - r, 2 * r, 2 * r);
            QRectF in(segC.x() - inner, segC.y() - inner, 2 * inner, 2 * inner);
            QPainterPath ring;
            ring.addEllipse(o);
            ring.addEllipse(in);
            ring.setFillRule(Qt::OddEvenFill);
            p.save();
            p.setClipPath(ring);
            QConicalGradient cg(segC, 90 - start);
            QColor c0 = colors[i]; c0.setAlphaF(dim);
            QColor c1 = colors[i].darker(125); c1.setAlphaF(dim * 0.90);
            cg.setColorAt(0.0, c0);
            cg.setColorAt(1.0, c1);
            p.setBrush(cg);
            p.setPen(QPen(QColor(0, 0, 0, 80), 1));
            p.drawPie(o, qRound((90 - start) * 16), -qRound(visSpan * 16));
            p.restore();

            p.setPen(QPen(colors[i].lighter(140), 4));
            p.setBrush(Qt::NoBrush);
            p.drawArc(o.adjusted(-1, -1, 1, 1), qRound((90 - start) * 16), -qRound(visSpan * 16));

            if (span > 0) {
                const qreal bd = qDegreesToRadians(-90.0 + consumed + span);
                p.setPen(QPen(QColor(0, 0, 0), 2));
                p.drawLine(QPointF(segC.x() + qCos(bd) * inner, segC.y() + qSin(bd) * inner), QPointF(segC.x() + qCos(bd) * r, segC.y() + qSin(bd) * r));
            }

            consumed += span;
        }

        QRadialGradient hole(c, inner);
        hole.setColorAt(0.0, QColor("#1A0E06"));
        hole.setColorAt(0.55, QColor("#0D0805"));
        hole.setColorAt(1.0, QColor("#1A1208"));
        p.setBrush(hole);
        p.setPen(Qt::NoPen);
        p.drawEllipse(c, inner, inner);
        p.setPen(QPen(QColor(0, 0, 0, 178), 6));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(c.x() - inner, c.y() - inner, inner * 2, inner * 2), 0, 360 * 16);

        const qreal centerCount = (m_data.total + m_data.retired) * easeSin(elapsedSec() / 1.4);
        p.setFont(QFont("Segoe UI", 26, QFont::Bold));
        p.setPen(QColor("#C17F3E"));
        p.drawText(QRectF(c.x() - 70, c.y() - 18, 140, 36), Qt::AlignCenter, QString::number((int)qRound(centerCount)));
        p.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        p.setPen(QColor(245, 230, 211, 115));
        p.drawText(QRectF(c.x() - 70, c.y() + 12, 140, 18), Qt::AlignCenter, "TOTAL");

        const QRectF legend(body.left() + body.width() * 0.66, body.top() + 12, body.width() * 0.33, body.height() - 24);
        qreal ly = legend.top();
        for (int i = 0; i < names.size(); ++i) {
            QColor col = colors[i];
            qreal pct = 100.0 * vals[i] / total;
            p.setPen(Qt::NoPen);
            QColor glow = col; glow.setAlpha(80);
            p.setBrush(glow);
            p.drawRoundedRect(QRectF(legend.left(), ly + 2, 10, 10), 2, 2);
            p.setBrush(col);
            p.drawRoundedRect(QRectF(legend.left() + 1, ly + 3, 8, 8), 2, 2);

            p.setFont(QFont("Segoe UI", 10));
            p.setPen(QColor("#F5E6D3"));
            p.drawText(QRectF(legend.left() + 16, ly, 82, 14), Qt::AlignLeft | Qt::AlignVCenter, names[i]);

            p.setFont(QFont("Segoe UI", 10, QFont::Bold));
            p.setPen(col);
            p.drawText(QRectF(legend.left() + 98, ly, 40, 14), Qt::AlignLeft | Qt::AlignVCenter, QString::number(vals[i]));

            p.setFont(QFont("Segoe UI", 9));
            p.setPen(QColor(245, 230, 211, 128));
            p.drawText(QRectF(legend.left() + 136, ly, 52, 14), Qt::AlignLeft | Qt::AlignVCenter, QString::number(pct, 'f', 1) + "%");
            ly += 24;
        }
    }

    void drawPanelRanking(QPainter &p, const QRectF &panel) {
        drawPanelFrame(p, panel, "EQUIPMENT VALUE RANKING", QColor("#FFD700"), QString::number(m_data.ranking.size()), 1);
        QRectF body = panel.adjusted(10, 44, -10, -10);

        const int rows = qMin(8, m_data.ranking.size());
        if (rows == 0) return;
        qreal maxVal = 1.0;
        for (int i = 0; i < rows; ++i) maxVal = qMax(maxVal, m_data.ranking[i].unitPrice);

        const qreal rowH = qMin(42.0, body.height() / rows);
        for (int i = 0; i < rows; ++i) {
            QRectF row(body.left(), body.top() + i * rowH, body.width(), rowH - 2);
            QColor rowBg = (i % 2 == 0) ? QColor("#0D0805") : QColor("#110A06");
            if (m_hoveredPanel == 1 && row.contains(mapFromGlobal(QCursor::pos()))) rowBg = QColor("#1E1408");
            p.fillRect(row, rowBg);

            const EquipmentRankingEntry &e = m_data.ranking[i];
            p.setFont(QFont("Segoe UI", 11, QFont::Bold));
            p.setPen(QColor("#F5E6D3"));
            p.drawText(QRectF(row.left() + 6, row.top() + 3, 160, 16), Qt::AlignLeft, e.name.left(20));
            p.setFont(QFont("Segoe UI", 9));
            p.setPen(QColor(245, 230, 211, 102));
            p.drawText(QRectF(row.left() + 6, row.top() + 20, 160, 14), Qt::AlignLeft, e.status);

            const qreal areaLeft = row.left() + 170;
            const qreal areaW = row.width() - 250;
            const qreal t = easeSin((elapsedSec() - i * 0.07) / 0.7);
            const qreal bw = areaW * (e.unitPrice / maxVal) * t;

            QRectF sh(areaLeft + 4, row.center().y() - 5, bw, 14);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(193, 127, 62, 51));
            p.drawRoundedRect(sh, 7, 7);

            QRectF bar(areaLeft, row.center().y() - 9, bw, 18);
            QLinearGradient bg(bar.topLeft(), bar.topRight());
            bg.setColorAt(0.0, QColor("#FFD700"));
            bg.setColorAt(0.5, QColor("#C17F3E"));
            bg.setColorAt(1.0, QColor("#8B4A1E"));
            p.setBrush(bg);
            p.drawRoundedRect(bar, 9, 9);

            p.setPen(QPen(QColor(255, 224, 102, 150), 1.5));
            p.drawLine(QPointF(bar.left() + 2, bar.top() + 1), QPointF(bar.right() - 2, bar.top() + 1));

            if (bw > 30) {
                qreal shimmerX = std::fmod(m_globalTime * 50.0 + i * 30.0, bw);
                qreal op = qSin((shimmerX / qMax<qreal>(1.0, bw)) * M_PI) * 0.25;
                QColor shc(255, 255, 255, int(op * 255));
                p.fillRect(QRectF(bar.left() + shimmerX, bar.top() + 2, 20, bar.height() - 4), shc);
            }

            p.setFont(QFont("Segoe UI", 11, QFont::Bold));
            p.setPen(QColor("#FFD700"));
            p.drawText(QRectF(row.right() - 80, row.top(), 76, row.height()), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(e.unitPrice, 'f', 0) + " dt");
        }
    }

    void drawPanelTimeline(QPainter &p, const QRectF &panel) {
        drawPanelFrame(p, panel, "WORKSHOP ACTIVITY TIMELINE", QColor("#3B82F6"), QString::number(m_data.months.size()), 2);

        QRectF body = panel.adjusted(14, 46, -14, -14);
        p.setPen(QPen(QColor("#1E1508"), 1));
        p.setBrush(QColor("#080503"));
        p.drawRoundedRect(body, 8, 8);

        if (m_data.months.isEmpty()) return;

        const int n = m_data.months.size();
        int maxY = 1;
        for (int v : m_data.monthlyAdded) maxY = qMax(maxY, v);
        for (int v : m_data.monthlyMaintenance) maxY = qMax(maxY, v);

        QRectF plot = body.adjusted(42, 16, -10, -28);
        p.setPen(QPen(QColor(193, 127, 62, 20), 1));
        for (int i = 0; i < 4; ++i) {
            qreal y = plot.bottom() - i * (plot.height() / 3.0);
            p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
            p.setFont(QFont("Segoe UI", 9));
            p.setPen(QColor(193, 127, 62, 150));
            p.drawText(QRectF(body.left(), y - 8, 34, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number((maxY * i) / 3));
            p.setPen(QPen(QColor(193, 127, 62, 20), 1));
        }

        QVector<QPointF> addPts;
        QVector<QPointF> mntPts;
        for (int i = 0; i < n; ++i) {
            qreal x = plot.left() + (n == 1 ? 0.0 : (plot.width() * i / (n - 1.0)));
            qreal y1 = plot.bottom() - (plot.height() * m_data.monthlyAdded.value(i) / maxY);
            qreal y2 = plot.bottom() - (plot.height() * m_data.monthlyMaintenance.value(i) / maxY);
            addPts << QPointF(x, y1);
            mntPts << QPointF(x, y2);
            p.setPen(QColor(245, 230, 211, 120));
            p.setFont(QFont("Segoe UI", 9));
            p.drawText(QRectF(x - 20, plot.bottom() + 6, 40, 14), Qt::AlignCenter, m_data.months[i]);
        }

        auto buildSmooth = [](const QVector<QPointF> &pts) {
            QPainterPath path;
            if (pts.isEmpty()) return path;
            path.moveTo(pts.first());
            for (int i = 1; i < pts.size(); ++i) {
                QPointF p0 = pts[i - 1];
                QPointF p1 = pts[i];
                qreal dx = (p1.x() - p0.x()) * 0.5;
                path.cubicTo(QPointF(p0.x() + dx, p0.y()), QPointF(p1.x() - dx, p1.y()), p1);
            }
            return path;
        };

        QPainterPath addPath = buildSmooth(addPts);
        QPainterPath mntPath = buildSmooth(mntPts);

        const qreal lineProg = easeSin(elapsedSec() / 1.2);
        p.save();
        p.setClipRect(QRectF(plot.left(), plot.top(), plot.width() * lineProg, plot.height()));

        QPainterPath fill = addPath;
        fill.lineTo(plot.bottomRight());
        fill.lineTo(plot.bottomLeft());
        fill.closeSubpath();
        QLinearGradient fg(plot.left(), plot.top(), plot.left(), plot.bottom());
        fg.setColorAt(0.0, QColor(59, 130, 246, 89));
        fg.setColorAt(1.0, QColor(59, 130, 246, 0));
        p.fillPath(fill, fg);

        p.setPen(QPen(QColor("#3B82F6"), 2.5));
        p.setBrush(Qt::NoBrush);
        p.drawPath(addPath);
        p.setPen(QPen(QColor("#F59E0B"), 2.0, Qt::DashLine));
        p.drawPath(mntPath);
        p.restore();

        for (int i = 0; i < addPts.size(); ++i) {
            p.setPen(QPen(Qt::white, 2));
            p.setBrush(QColor("#3B82F6"));
            p.drawEllipse(addPts[i], 3.5, 3.5);
            p.setPen(QPen(QColor("#F59E0B"), 2));
            p.setBrush(QColor("#F59E0B"));
            p.drawEllipse(mntPts[i], 2.8, 2.8);
        }

        p.setPen(QColor("#3B82F6"));
        p.setFont(QFont("Segoe UI", 9));
        p.drawText(QRectF(plot.right() - 160, plot.top() + 2, 150, 12), "Blue  Equipment Added");
        p.setPen(QColor("#F59E0B"));
        p.drawText(QRectF(plot.right() - 160, plot.top() + 16, 150, 12), "Orange  Maintenance Events");
    }

    void drawPanelRadar(QPainter &p, const QRectF &panel) {
        drawPanelFrame(p, panel, "WORKSHOP HEALTH RADAR", QColor("#4CAF7D"), QString::number(m_data.radarScores.size()), 3);
        QRectF body = panel.adjusted(12, 44, -12, -10);

        const QVector<QString> labels = {
            "Equipment Health", "Maintenance Score", "Value Density",
            "Activity Level", "Age Balance", "Documentation"
        };
        QVector<double> scores = m_data.radarScores;
        if (scores.size() != 6) {
            scores = {40, 40, 40, 40, 40, 40};
        }

        QPointF c(body.center().x(), body.center().y() + 2);
        const qreal rr = qMin(body.width(), body.height()) * 0.33;
        const int axes = 6;

        p.setPen(QPen(QColor(193, 127, 62, 20), 1));
        for (int ring = 1; ring <= 4; ++ring) {
            qreal k = ring / 4.0;
            QPolygonF poly;
            for (int i = 0; i < axes; ++i) {
                qreal a = qDegreesToRadians(-90.0 + i * (360.0 / axes));
                poly << QPointF(c.x() + rr * k * qCos(a), c.y() + rr * k * qSin(a));
            }
            p.drawPolygon(poly);
        }

        for (int i = 0; i < axes; ++i) {
            qreal a = qDegreesToRadians(-90.0 + i * (360.0 / axes));
            p.setPen(QPen(QColor(193, 127, 62, 38), 1));
            p.drawLine(c, QPointF(c.x() + rr * qCos(a), c.y() + rr * qSin(a)));
        }

        const qreal radarAnim = easeSin(elapsedSec() / 1.0);
        QPolygonF dataPoly;
        QVector<QPointF> points;
        for (int i = 0; i < axes; ++i) {
            qreal a = qDegreesToRadians(-90.0 + i * (360.0 / axes));
            qreal k = clamp01(scores[i] / 100.0) * radarAnim;
            QPointF pt(c.x() + rr * k * qCos(a), c.y() + rr * k * qSin(a));
            points << pt;
            dataPoly << pt;
        }

        QRadialGradient rg(c, rr);
        rg.setColorAt(0.0, QColor(76, 175, 125, 64));
        rg.setColorAt(1.0, QColor(76, 175, 125, 20));
        p.setBrush(rg);
        p.setPen(QPen(QColor(76, 175, 125, 204), 2));
        p.drawPolygon(dataPoly);

        for (int i = 0; i < points.size(); ++i) {
            QRadialGradient g(points[i], 10);
            g.setColorAt(0.0, QColor(76, 175, 125, 90));
            g.setColorAt(1.0, QColor(76, 175, 125, 0));
            p.setBrush(g);
            p.setPen(Qt::NoPen);
            p.drawEllipse(points[i], 10, 10);

            p.setBrush(QColor("#4CAF7D"));
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(points[i], 3, 3);
        }

        for (int i = 0; i < axes; ++i) {
            qreal a = qDegreesToRadians(-90.0 + i * (360.0 / axes));
            QPointF lp(c.x() + (rr + 18) * qCos(a), c.y() + (rr + 18) * qSin(a));
            QRectF lr(lp.x() - 52, lp.y() - 10, 104, 32);
            p.setFont(QFont("Segoe UI", 9, QFont::Bold));
            p.setPen(QColor("#F5E6D3"));
            p.drawText(QRectF(lr.left(), lr.top(), lr.width(), 14), Qt::AlignCenter, labels[i]);
            p.setFont(QFont("Segoe UI", 12, QFont::Bold));
            p.setPen(QColor("#4CAF7D"));
            p.drawText(QRectF(lr.left(), lr.top() + 14, lr.width(), 16), Qt::AlignCenter, QString::number((int)qRound(scores[i])));
        }
    }

    void drawBottomCards(QPainter &p, const QRectF &a, const QRectF &b, const QRectF &c) {
        auto drawCardFrame = [&](const QRectF &r) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 120));
            p.drawRoundedRect(r.translated(3, 5), 12, 12);
            QLinearGradient bg(r.topLeft(), r.bottomRight());
            bg.setColorAt(0.0, QColor("#1A1208"));
            bg.setColorAt(1.0, QColor("#0A0804"));
            p.setBrush(bg);
            p.setPen(QPen(QColor(193, 127, 62, 64), 1));
            p.drawRoundedRect(r, 12, 12);
        };

        drawCardFrame(a);
        p.setPen(QColor("#C17F3E"));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(QRectF(a.left() + 10, a.top() + 10, a.width() - 20, 14), "MOST ACTIVE EQUIPMENT");
        p.setPen(QColor("#F5E6D3"));
        p.setFont(QFont("Segoe UI", 11, QFont::Bold));
        p.drawText(QRectF(a.left() + 10, a.top() + 28, a.width() - 20, 16), m_data.mostActiveName.isEmpty() ? "No tracked activity" : m_data.mostActiveName);
        p.setFont(QFont("Segoe UI", 9));
        p.setPen(QColor(245, 230, 211, 150));
        p.drawText(QRectF(a.left() + 10, a.top() + 46, a.width() - 20, 14), QString::number(m_data.mostActiveEvents) + " Most interactions");
        qreal actw = (a.width() - 20) * clamp01(m_data.mostActiveEvents / 12.0);
        p.fillRect(QRectF(a.left() + 10, a.bottom() - 16, actw, 6), QColor("#C17F3E"));

        drawCardFrame(b);
        p.setPen(QColor("#4CAF7D"));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(QRectF(b.left() + 10, b.top() + 10, b.width() - 20, 14), "NEWEST ACQUISITION");
        p.setPen(QColor("#F5E6D3"));
        p.setFont(QFont("Segoe UI", 11, QFont::Bold));
        p.drawText(QRectF(b.left() + 10, b.top() + 28, b.width() - 20, 16), m_data.newestName.isEmpty() ? "No purchase date" : m_data.newestName);
        p.setFont(QFont("Segoe UI", 9));
        p.setPen(QColor(245, 230, 211, 150));
        const QString dateTxt = m_data.newestDate.isValid() ? m_data.newestDate.toString("yyyy-MM-dd") : "n/a";
        p.drawText(QRectF(b.left() + 10, b.top() + 46, b.width() - 20, 14), dateTxt + "  |  " + QString::number(m_data.newestDays) + " days ago");
        p.setBrush(QColor("#4CAF7D"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(b.right() - 58, b.top() + 10, 48, 16), 8, 8);
        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 8, QFont::Bold));
        p.drawText(QRectF(b.right() - 58, b.top() + 10, 48, 16), Qt::AlignCenter, "NEW");

        drawCardFrame(c);
        p.setPen(QColor("#C17F3E"));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(QRectF(c.left() + 10, c.top() + 10, c.width() - 20, 14), "WORKSHOP AGE SUMMARY");
        p.setPen(QColor("#F5E6D3"));
        p.setFont(QFont("Segoe UI", 10, QFont::Bold));
        p.drawText(QRectF(c.left() + 10, c.top() + 26, c.width() - 20, 14), "Average: " + QString::number(m_data.avgAgeYears, 'f', 1) + " years");
        p.setFont(QFont("Segoe UI", 9));
        p.setPen(QColor(245, 230, 211, 150));
        p.drawText(QRectF(c.left() + 10, c.top() + 42, c.width() - 20, 12), "Oldest: " + (m_data.oldestName.isEmpty() ? "n/a" : m_data.oldestName));
        p.drawText(QRectF(c.left() + 10, c.top() + 56, c.width() - 20, 12), "Newest: " + (m_data.newestAgeName.isEmpty() ? "n/a" : m_data.newestAgeName));

        qreal ageNorm = clamp01(m_data.avgAgeYears / 20.0);
        QRectF tl(c.left() + 10, c.bottom() - 16, c.width() - 20, 6);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(245, 230, 211, 30));
        p.drawRoundedRect(tl, 3, 3);
        p.setBrush(QColor("#C17F3E"));
        p.drawRoundedRect(QRectF(tl.left(), tl.top(), tl.width() * ageNorm, tl.height()), 3, 3);
    }

private:
    EquipmentStatsVisualData m_data;
    QTimer m_frameTimer;
    qreal m_globalTime = 0.0;
    qint64 m_bootMs = 0;

    QVector<QRectF> m_panelRects;
    int m_hoveredPanel = -1;
    int m_hoveredSegment = -1;

    QPointF m_donutCenter;
    qreal m_donutInner = 0.0;
    qreal m_donutOuter = 0.0;
    QVector<QPair<qreal, qreal>> m_segmentRanges;
};

}


namespace {
QString trKey(const QString &key)
{
    return QCoreApplication::translate("QObject", key.toUtf8().constData());
}

void setTrKey(QWidget *widget, const QString &key)
{
    if (widget) {
        widget->setProperty("trKey", key);
    }
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , translator(new QTranslator(this))
    , currentLanguage("en")
    , currentEmployeeId(0)
    , m_isChatModernTheme(false) // Classic is now default
    , currentVolume(1.0) // Initialize at start
{
    ui->setupUi(this);
    {
        QSettings settings("HammerDown", "HammerDown");
        currentVolume = qBound<qreal>(0.0, settings.value("audio/volume", 1.0).toDouble(), 1.5);
    }
    weatherAssistant = nullptr;
    currentChatPartnerId = -1;
    
    // Set application icon
    setWindowIcon(QIcon(":/assets/logo.png"));

    // --- CRITICAL REFACTOR: CLEAR STACK AND REBUILD ---
    // Remove any hardcoded pages (e.g., page_login, page_home) created by setupUi
    while (ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();
    }

    // 1. Login Window (Index 0)
    loginWindow = new LoginWindow(this);
    ui->stackedWidget->addWidget(loginWindow);
    connect(loginWindow, &LoginWindow::loginSuccessful, this, [this](int employeeId) {
        currentEmployeeId = employeeId;
        on_login_clicked();
        // Show unread messages splash after login
        QTimer::singleShot(600, this, &MainWindow::showUnreadMessagesSplash);
    });

    // 2. Home Window (Index 1)
    homeWindow = new HomeWindow(this);
    ui->stackedWidget->addWidget(homeWindow);
    
    // Connect HomeWindow signals
    connect(homeWindow, &HomeWindow::employesClicked,    this, &MainWindow::on_gs_employes_clicked);
    connect(homeWindow, &HomeWindow::clientClicked,      this, &MainWindow::on_gs_client_clicked);
    connect(homeWindow, &HomeWindow::fournisseurClicked, this, &MainWindow::on_gs_fournisseur_clicked);
    connect(homeWindow, &HomeWindow::equipmentClicked,   this, &MainWindow::on_gs_equipment_clicked);
    connect(homeWindow, &HomeWindow::orderClicked,       this, &MainWindow::on_gs_order_clicked);
    connect(homeWindow, &HomeWindow::languageChanged,    this, &MainWindow::onLanguageChanged);
    connect(homeWindow, &HomeWindow::volumeChanged,      this, &MainWindow::setAudioVolume);
    connect(homeWindow, &HomeWindow::disconnectClicked,  this, &MainWindow::on_btn_logout_clicked);

    // Initial sync of homeWindow state
    homeWindow->setLanguage(currentLanguage);
    homeWindow->setVolume(currentVolume);

    // 3. Employee Management (Index 2)
    ui_employee = new Ui::EmployeeManagement;
    employeePage = new QWidget(this);
    ui_employee->setupUi(employeePage);
    ui->stackedWidget->addWidget(employeePage);
    connect(ui_employee->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    // Employee CRUD connections
    connect(ui_employee->btn_add,    &QPushButton::clicked, this, &MainWindow::onEmployeeAdd);
    connect(ui_employee->btn_modify, &QPushButton::clicked, this, &MainWindow::onEmployeeModify);
    connect(ui_employee->btn_upload_avatar, &QPushButton::clicked, this, &MainWindow::onUploadAvatar);
    connect(ui_employee->btn_scan_face, &QPushButton::clicked, this, &MainWindow::onScanFace);
    
    // Salary Intelligence
    ui_employee->dsb_salaire->setRange(0, 9999.99);
    connect(ui_employee->dsb_salaire, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateSalaryInsight);
    connect(ui_employee->le_fonction, &QLineEdit::textChanged, this, &MainWindow::updateSalaryInsight);
    connect(ui_employee->btn_suggest_salary, &QPushButton::clicked, this, &MainWindow::onSuggestSalary);
    connect(ui_employee->btn_stats_ai_gen, &QPushButton::clicked, this, &MainWindow::onStatsAiClicked);
    connect(ui_employee->btn_ai_pulse, &QPushButton::clicked, this, &MainWindow::onAIPulseClicked);
    
    // --- Employee Input Validation & Restrictions ---
    ui_employee->le_id->setValidator(new QIntValidator(1, 9999999, this));
    ui_employee->le_num->setValidator(new QIntValidator(1, 99999999, this));
    
    // Letters only for First/Last Names
    QRegularExpression regLetters("^[A-Za-z\\s]+$");
    QRegularExpressionValidator *letterVal = new QRegularExpressionValidator(regLetters, this);
    ui_employee->le_nom->setValidator(letterVal);
    ui_employee->le_prenom->setValidator(letterVal);

    // Job Title Selection (Restrict to 6 premium options)
    QStringList workRoles = {"Smith", "Cleaner", "Developer", "Cashier", "Carpenter", "Boss"};
    QCompleter *jobComp = new QCompleter(workRoles, this);
    jobComp->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui_employee->le_fonction->setCompleter(jobComp);
    ui_employee->le_fonction->setPlaceholderText("Select: Smith, Cleaner, Dev, Cashier, Carp, Boss");

    ui_employee->de_birthdate->setDateRange(QDate(1950, 1, 1), QDate::currentDate());
    ui_employee->de_birthdate->setDate(QDate(1995, 1, 1));
    // Auto-refresh employee view when switching to view tab
    connect(ui_employee->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_employee->tabWidget->widget(idx) == ui_employee->tab_view)
            onEmployeeRefreshView();
        else if (ui_employee->tabWidget->widget(idx) == ui_employee->tab_history)
            onEmployeeRefreshHistory();
        else if (ui_employee->tabWidget->widget(idx) == ui_employee->tab_stats)
            setupEmployeeStats();
    });
    connect(ui_employee->btn_refresh_emp, &QPushButton::clicked, this, &MainWindow::onEmployeeRefreshView);
    connect(ui_employee->le_recherche_emp, &QLineEdit::textChanged, this, &MainWindow::onEmployeeSearch);
    connect(ui_employee->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEmployeeRefreshHistory);
    connect(ui_employee->btn_delete, &QPushButton::clicked, this, &MainWindow::onEmployeeDelete);
    connect(ui_employee->tableView_employes, &QTableView::clicked, this, [this](const QModelIndex &idx){
        if (idx.column() == 0) { // Edit Action
            onEmployeeRowSelected(idx);
            ui_employee->tabWidget->setCurrentIndex(0);
            // Switch UI to "Modify Employee" mode
            if (auto *rb = ui_employee->tab_add->findChild<QRadioButton*>("rb_employee_mod_mode")) {
                rb->setChecked(true);
            }
        } else if (idx.column() == 1) { // Delete Action
            onEmployeeDelete();
        }
    });

    // ID-Unlock Logic for Management CRUD (Consolidated)
    connect(ui_employee->le_id, &QLineEdit::textChanged, this, [=](const QString &t){
        toggleEmployeeFields(!t.trimmed().isEmpty());
    });
    toggleEmployeeFields(false); // Default to locked

    // 4. Client Management (Index 3)
    ui_client = new Ui::ClientManagement;
    clientPage = new QWidget(this);
    ui_client->setupUi(clientPage);
    ui->stackedWidget->addWidget(clientPage);
    connect(ui_client->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    // Client CRUD connections
    connect(ui_client->btn_add,    &QPushButton::clicked, this, &MainWindow::onClientAdd);
    connect(ui_client->btn_modify, &QPushButton::clicked, this, &MainWindow::onClientModify);
    connect(ui_client->btn_delete, &QPushButton::clicked, this, &MainWindow::onClientDelete);
    connect(ui_client->btn_search, &QPushButton::clicked, this, &MainWindow::onClientSearch);
    connect(ui_client->tableView, &QAbstractItemView::clicked, this, &MainWindow::onClientRowSelected);

    // Add explict View Tab buttons for Edit
    connect(ui_client->btn_edit_view, &QPushButton::clicked, this, [this]() {
        QModelIndex idx = ui_client->tableView->currentIndex();
        if (idx.isValid()) {
            // First run row prepopulation logic
            onClientRowSelected(idx);
            // Switch to unified Manage tab (which contains add/modify forms)
            ui_client->tabWidget->setCurrentWidget(ui_client->tab_add);
            // Programmatically click the "Modify Mode" radio button inside that tab
            if (auto *rb = ui_client->tab_add->findChild<QRadioButton*>("rb_client_mod_mode")) {
                rb->setChecked(true);
            }
        } else {
            QMessageBox::warning(this, "Selection", "Please select a client to edit.");
        }
    });

    // Also search on Enter in the search box
    connect(ui_client->le_recherche, &QLineEdit::returnPressed, this, &MainWindow::onClientSearch);
    connect(ui_client->btn_pdf,    &QPushButton::clicked, this, &MainWindow::onClientExportPDF);
    // Mail tab buttons
    connect(ui_client->btn_send,   &QPushButton::clicked, this, &MainWindow::onClientSendMail);
    connect(ui_client->btn_browse, &QPushButton::clicked, this, &MainWindow::onClientBrowseMail);

    // Hide SMTP config fields — credentials are hardcoded in onClientSendMail
    ui_client->l_smtp->hide();  ui_client->le_smtp->hide();
    ui_client->l_port->hide();  ui_client->le_port->hide();
    ui_client->l_user->hide();  ui_client->le_user->hide();
    ui_client->l_pass->hide();  ui_client->le_pass->hide();

    // Auto-refresh client view when switching to view tab
    connect(ui_client->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_client->tabWidget->widget(idx) == ui_client->tab_view)
            onClientRefreshView();
    });

    // 5. Supplier Management (Index 4)
    ui_supplier = new Ui::SupplierManagement;
    supplierPage = new QWidget(this);
    ui_supplier->setupUi(supplierPage);
    ui->stackedWidget->addWidget(supplierPage);
    connect(ui_supplier->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    m_supplierMapNet = new QNetworkAccessManager(this);
    connect(m_supplierMapNet, &QNetworkAccessManager::finished, this, &MainWindow::onSupplierGeocodeFinished);
    setupSupplierMapTab();

    // 6. Equipment Management (Index 5)
    ui_equipment = new Ui::EquipmentManagement;
    equipmentPage = new QWidget(this);
    ui_equipment->setupUi(equipmentPage);
    ui->stackedWidget->addWidget(equipmentPage);

    // --- NEXUS TAB: Add programmatically as tab index 5 ---
    m_nexusWidget = new NexusWidget(equipmentPage);
    ui_equipment->tabWidget->addTab(m_nexusWidget, "NEXUS");

    // --- COSTS TAB: Add programmatically as tab index 6 ---
    m_costsWidget = new CostsWidget(equipmentPage);
    ui_equipment->tabWidget->addTab(m_costsWidget, "COSTS");

    // Auto-refresh equipment views/stats when switching tabs
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_view)
            onEquipmentRefreshView();
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_history)
            onEquipmentHistoryRefresh();
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_stats)
            setupEquipmentStats();
        else if (ui_equipment->tabWidget->widget(idx) == m_nexusWidget) {
            // Initialize NEXUS when first shown
            m_nexusWidget->initialize();
        }
        else if (ui_equipment->tabWidget->widget(idx) == m_costsWidget) {
            // Initialize COSTS when first shown
            m_costsWidget->initialize();
        }
    });
    
    // Ensure History Table Exists
    onChatEnsureTable();
    
    chatRefreshTimer = new QTimer(this);
    connect(chatRefreshTimer, &QTimer::timeout, this, &MainWindow::onChatRefresh);
    
    // Connect Chat buttons
    connect(ui_equipment->btn_chat_send, &QPushButton::clicked, this, &MainWindow::onChatSendMessage);
    connect(ui_equipment->le_chat_input, &QLineEdit::returnPressed, this, &MainWindow::onChatSendMessage);
    connect(ui_equipment->list_employees, &QListWidget::itemClicked, this, &MainWindow::onChatEmployeeSelected);
    connect(ui_equipment->btn_chat_img, &QPushButton::clicked, this, &MainWindow::onChatAttachImage);
    
    // --- Add Emoji & GIF buttons to the input bar ---
    {
        QHBoxLayout *inputLayout = ui_equipment->horizontalLayout_input;
        
        QPushButton *emojiBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x98\x80"), equipmentPage);
        emojiBtn->setObjectName("btn_chat_emoji");
        emojiBtn->setFixedSize(44, 44);
        emojiBtn->setToolTip("Emoji Picker");
        emojiBtn->setCursor(Qt::PointingHandCursor);
        emojiBtn->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
        
        QPushButton *gifBtn = new QPushButton("GIF", equipmentPage);
        gifBtn->setObjectName("btn_chat_gif");
        gifBtn->setFixedSize(50, 44);
        gifBtn->setToolTip("Search GIFs (Powered by GIPHY)");
        gifBtn->setCursor(Qt::PointingHandCursor);
        gifBtn->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 13px; font-weight: bold; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
        
        // Insert after btn_chat_img (index 0) but before le_chat_input
        inputLayout->insertWidget(1, emojiBtn);
        inputLayout->insertWidget(2, gifBtn);
        
        connect(emojiBtn, &QPushButton::clicked, this, &MainWindow::onChatEmojiClicked);
        connect(gifBtn, &QPushButton::clicked, this, &MainWindow::onChatGifClicked);
    }
    
    // GIPHY network manager
    giphyNetworkManager = new QNetworkAccessManager(this);
    
    // AI Summarization network manager
    chatSummaryNetManager = new QNetworkAccessManager(this);
    
    // --- Equipment Hover Card for #id preview ---
    m_hoverCard = new EquipmentHoverCard(equipmentPage);
    m_hoverCard->hide();
    
    // --- Smart Chat Completer ---
    m_completerModel = new QStringListModel(this);
    m_chatCompleter = new QCompleter(m_completerModel, this);
    m_chatCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_chatCompleter->setCompletionMode(QCompleter::PopupCompletion);
    ui_equipment->le_chat_input->setCompleter(m_chatCompleter);
    
    // Connect textChanged for #id detection anywhere in the text
    connect(ui_equipment->le_chat_input, &QLineEdit::textChanged, this, [this](const QString &text){
        if (!ui_equipment || !m_hoverCard) return;
        
        // Find the last instance of #id in the text
        static QRegularExpression reg("#(\\d+)");
        QRegularExpressionMatchIterator it = reg.globalMatch(text);
        QRegularExpressionMatch lastMatch;
        while (it.hasNext()) lastMatch = it.next();
        
        if (lastMatch.hasMatch()) {
            int id = lastMatch.captured(1).toInt();
            // Map position to a slightly better spot
            QPoint pos = ui_equipment->le_chat_input->mapTo(equipmentPage, QPoint(120, -145));
            m_hoverCard->showCard(id, pos);
        } else {
            m_hoverCard->hide();
        }
    });

    // Ensure Microphone button is visible and styled properly
    if (ui_equipment->btn_chat_voice) {
        ui_equipment->btn_chat_voice->setVisible(true);
        ui_equipment->btn_chat_voice->setToolTip("Voice Note - Recording Waveform");
        ui_equipment->btn_chat_voice->setCursor(Qt::PointingHandCursor);
        ui_equipment->btn_chat_voice->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 2.2px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
    }
    
    // --- Ctrl+F Chat Search Shortcut ---
    {
        QShortcut *chatSearchShortcut = new QShortcut(QKeySequence("Ctrl+F"), equipmentPage);
        connect(chatSearchShortcut, &QShortcut::activated, this, &MainWindow::onChatSearchToggle);
    }
    
    // --- Equipment Quick-Share Button in Chat Input Bar ---
    {
        QHBoxLayout *inputLayout = ui_equipment->horizontalLayout_input;
        
        QPushButton *shareEquipBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\xA6"), equipmentPage);
        shareEquipBtn->setObjectName("btn_chat_share_equip");
        shareEquipBtn->setFixedSize(44, 44);
        shareEquipBtn->setToolTip("Share Equipment Card");
        shareEquipBtn->setCursor(Qt::PointingHandCursor);
        shareEquipBtn->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
        
        // Insert after GIF button (index 3)
        inputLayout->insertWidget(3, shareEquipBtn);
        
        connect(shareEquipBtn, &QPushButton::clicked, this, [this]() {
            // Show equipment picker dialog
            QDialog *pickDialog = new QDialog(this);
            pickDialog->setWindowTitle("Share Equipment");
            pickDialog->setFixedSize(520, 480);
            pickDialog->setStyleSheet(
                "QDialog { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
                " border: 2px solid #8B6F47; border-radius: 16px; }");
            
            QVBoxLayout *dLay = new QVBoxLayout(pickDialog);
            dLay->setContentsMargins(20, 20, 20, 20);
            dLay->setSpacing(12);
            
            QLabel *headerLbl = new QLabel(QString::fromUtf8("\xF0\x9F\x93\xA6 Select Equipment to Share"), pickDialog);
            headerLbl->setStyleSheet("color: #D4AF37; font-size: 16px; font-weight: bold; background: transparent;");
            headerLbl->setAlignment(Qt::AlignCenter);
            dLay->addWidget(headerLbl);
            
            // Search field
            QLineEdit *searchField = new QLineEdit(pickDialog);
            searchField->setPlaceholderText("Search equipment...");
            searchField->setStyleSheet(
                "QLineEdit { background: rgba(0,0,0,0.3); color: #F0E0C0; border: 1.5px solid #5A4A32;"
                " border-radius: 14px; padding: 8px 14px; font-size: 13px; }"
                "QLineEdit:focus { border-color: #D4AF37; }");
            dLay->addWidget(searchField);
            
            // Equipment list
            QListWidget *equipList = new QListWidget(pickDialog);
            equipList->setStyleSheet(
                "QListWidget { background: rgba(0,0,0,0.2); color: #F0E0C0; border: 1px solid #5A4A32; border-radius: 10px; }"
                "QListWidget::item { padding: 10px; border-bottom: 1px solid rgba(139,111,71,0.2); font-size: 13px; }"
                "QListWidget::item:selected { background: rgba(139,111,71,0.4); color: #D4AF37; }"
                "QListWidget::item:hover { background: rgba(139,111,71,0.25); }");
            
            // Populate from database
            QSqlQuery q("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, STATUS, QUANTITY, UNIT_PRICE FROM EQUIPMENT WHERE STATUS != 'Retired' ORDER BY EQUIPMENT_ID");
            while (q.next()) {
                int eId = q.value(0).toInt();
                QString eType = q.value(1).toString();
                QString eStatus = q.value(2).toString();
                int qty = q.value(3).toInt();
                double price = q.value(4).toDouble();
                
                QString display = QString("[ID: %1] %2 — %3 | Qty: %4 | $%5")
                    .arg(eId).arg(eType).arg(eStatus).arg(qty).arg(price, 0, 'f', 2);
                    
                QListWidgetItem *item = new QListWidgetItem(display);
                item->setData(Qt::UserRole, eId);
                item->setData(Qt::UserRole + 1, eType);
                item->setData(Qt::UserRole + 2, eStatus);
                item->setData(Qt::UserRole + 3, qty);
                item->setData(Qt::UserRole + 4, price);
                equipList->addItem(item);
            }
            dLay->addWidget(equipList, 1);
            
            // Filter
            connect(searchField, &QLineEdit::textChanged, [equipList](const QString &txt){
                for (int i = 0; i < equipList->count(); ++i) {
                    auto *item = equipList->item(i);
                    item->setHidden(!item->text().contains(txt, Qt::CaseInsensitive));
                }
            });
            
            QPushButton *shareBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\xA8 Share to Chat"), pickDialog);
            shareBtn->setStyleSheet(
                "QPushButton { background: #8B6F47; color: white; border-radius: 14px;"
                " padding: 10px 24px; font-weight: bold; font-size: 14px; border: none; }"
                "QPushButton:hover { background: #A0825A; }");
            shareBtn->setCursor(Qt::PointingHandCursor);
            dLay->addWidget(shareBtn, 0, Qt::AlignCenter);
            
            connect(shareBtn, &QPushButton::clicked, this, [this, equipList, pickDialog]() {
                if (!equipList->currentItem()) {
                    QMessageBox::information(pickDialog, "Select", "Please select an equipment item to share.");
                    return;
                }
                auto *item = equipList->currentItem();
                int eId = item->data(Qt::UserRole).toInt();
                QString eType = item->data(Qt::UserRole + 1).toString();
                QString eStatus = item->data(Qt::UserRole + 2).toString();
                int qty = item->data(Qt::UserRole + 3).toInt();
                double price = item->data(Qt::UserRole + 4).toDouble();
                
                QString shareCard = QString::fromUtf8("\xF0\x9F\x93\xA6 [Equipment Card]\n"
                    "━━━━━━━━━━━━━━━━\n"
                    "ID: %1\n"
                    "Type: %2\n"
                    "Status: %3\n"
                    "Quantity: %4\n"
                    "Unit Price: $%5\n"
                    "━━━━━━━━━━━━━━━━")
                    .arg(eId).arg(eType).arg(eStatus).arg(qty).arg(price, 0, 'f', 2);
                
                if (currentChatPartnerId == -1) {
                    QMessageBox::information(pickDialog, "Select Chat", "Please select a chat partner first, then share equipment.");
                    return;
                }
                
                ui_equipment->le_chat_input->setText(shareCard);
                pickDialog->accept();
                onChatSendMessage(); // Auto-send the card
            });
            
            pickDialog->exec();
            delete pickDialog;
        });
    }
    
    if (auto *refreshBtn = equipmentPage->findChild<QPushButton*>("btn_chat_refresh"))
        connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onChatRefresh);
    
    // Connect Weather Assistant button
    // Weather buttons are now configured later in the constructor with high-quality icons.
    // Live search filter for employee list
    if (auto *searchEdit = equipmentPage->findChild<QLineEdit*>("le_chat_search"))
        connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &txt){
            if (!ui_equipment) return;
            for (int i = 0; i < ui_equipment->list_employees->count(); ++i) {
                auto *item = ui_equipment->list_employees->item(i);
                item->setHidden(!item->text().contains(txt, Qt::CaseInsensitive));
            }
        });

    
    connect(ui_equipment->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);

    // 7. Order Management (Index 6)
    ui_order = new Ui::OrderManagement;
    orderPage = new QWidget(this);
    ui_order->setupUi(orderPage);

    m_mapNet = new QNetworkAccessManager(this);
    connect(m_mapNet, &QNetworkAccessManager::finished, this, &MainWindow::onMapNetworkFinished);
    setupOrderMapTab();
    
    // Order Input Validation
    ui_order->le_id->setValidator(new QIntValidator(1, 999999999, this));
    ui_order->le_stock->setValidator(new QIntValidator(1, 999999, this));
    ui_order->le_buyer->setValidator(new QIntValidator(1, 999999999, this));
    QDoubleValidator *priceValidator = new QDoubleValidator(0.01, 9999999.99, 2, this);
    priceValidator->setNotation(QDoubleValidator::StandardNotation);
    ui_order->le_prix->setValidator(priceValidator);
    
    ui_order->table_catalog->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_order->table_catalog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_order->table_catalog->setAlternatingRowColors(false);
    ui_order->table_catalog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_order->table_catalog->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_order->table_catalog->setShowGrid(true);
    ui_order->table_catalog->setFocusPolicy(Qt::NoFocus);
    ui_order->table_catalog->setIconSize(QSize(54, 54));
    ui_order->table_catalog->verticalHeader()->setVisible(false);
    ui_order->table_catalog->horizontalHeader()->setFixedHeight(42);
    ui_order->table_catalog->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui_order->table_catalog->setStyleSheet(
        "QTableWidget {"
        "  background: rgba(255, 255, 255, 0.94);"
        "  border: 1px solid #8B6F47;"
        "  border-radius: 0px;"
        "  color: #1D1D1D;"
        "  gridline-color: #8B6F47;"
        "  selection-background-color: #E0E0E0;"
        "  selection-color: #1D1D1D;"
        "}"
        "QTableWidget::item {"
        "  padding: 5px 8px;"
        "  border-right: 1px solid #8B6F47;"
        "  border-bottom: 1px solid #8B6F47;"
        "}"
        "QHeaderView::section {"
        "  background: #8B6F47;"
        "  color: #1F2A44;"
        "  border: 1px solid #705a39;"
        "  padding: 6px;"
        "  font-weight: bold;"
        "}"
        "QTableCornerButton::section {"
        "  background: #8B6F47;"
        "  border: 1px solid #705a39;"
        "}"
    );

    ui_order->le_catalog_search->setStyleSheet(
        "QLineEdit {"
        "  background: rgba(255,255,255,0.96);"
        "  color: #2E261C;"
        "  border: 1.5px solid #8B6F47;"
        "  border-radius: 8px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #A38253;"
        "}"
    );

    const QString catalogBtnStyle =
        "QPushButton {"
        "  background-color: #8B6F47;"
        "  color: white;"
        "  border-radius: 8px;"
        "  padding: 8px 14px;"
        "  font-weight: bold;"
        "  border: 1px solid #6d5638;"
        "}"
        "QPushButton:hover {"
        "  background-color: #a38253;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #6d5638;"
        "}";
    ui_order->btn_export_catalog->setStyleSheet(catalogBtnStyle);
    ui_order->btn_print_catalog->setStyleSheet(catalogBtnStyle);
    ui_order->btn_delete_all->setStyleSheet(catalogBtnStyle);
    ui->stackedWidget->addWidget(orderPage);
    connect(ui_order->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    connect(ui_order->btn_clear, &QPushButton::clicked, this, &MainWindow::onOrderClearFields);
    connect(ui_order->btn_add, &QPushButton::clicked, this, &MainWindow::onOrderAdd);
    connect(ui_order->btn_modify, &QPushButton::clicked, this, &MainWindow::onOrderModify);
    connect(ui_order->btn_delete, &QPushButton::clicked, this, &MainWindow::onOrderDelete);
    
    // Connect catalog buttons
    connect(ui_order->le_catalog_search, &QLineEdit::returnPressed, this, &MainWindow::onOrderSearchCatalog);
    connect(ui_order->btn_export_catalog, &QPushButton::clicked, this, &MainWindow::onOrderExportCatalog);
    connect(ui_order->btn_import, &QPushButton::clicked, this, &MainWindow::onOrderImportCatalog);
    connect(ui_order->btn_print_catalog, &QPushButton::clicked, this, &MainWindow::onOrderPrintCatalog);
    connect(ui_order->btn_delete_all, &QPushButton::clicked, this, &MainWindow::onOrderDeleteAll);
    
    // Connect QR Code tab buttons
    connect(ui_order->btn_generate_qr, &QPushButton::clicked, this, &MainWindow::onGenerateQR);
    connect(ui_order->btn_save_qr, &QPushButton::clicked, this, &MainWindow::onSaveQR);
    connect(ui_order->btn_print_qr, &QPushButton::clicked, this, &MainWindow::onPrintQR);
    
    // Embed the 3D modeling widget inside the 3D Modeling tab
    {
        auto *modeler = new ModelingWidget(ui_order->tab_3d_modeling);
        auto *tabLayout = new QVBoxLayout(ui_order->tab_3d_modeling);
        tabLayout->setContentsMargins(0, 0, 0, 0);
        tabLayout->addWidget(modeler);

        // Auto-load preset when order type changes
        connect(ui_order->cb_type, &QComboBox::currentTextChanged, modeler, &ModelingWidget::loadPreset);
    }

    // Auto-refresh catalog when switching to catalog tab, and close help panels
    connect(ui_order->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        // Close help buttons when switching tabs
        ui_order->btn_help->setChecked(false);
        ui_order->btn_help_qr->setChecked(false);
        
        // Stop ost4 and fade back to ost1 if tutorial audio is playing
        if (tutorialLoopAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOutAndPlay(tutorialLoopAudioPlayer, tutorialLoopAudioOutput,
                           loginAudioPlayer, loginAudioOutput);
        }
        
        QWidget *currentTab = ui_order->tabWidget->widget(index);
        if (currentTab == ui_order->tab_catalog) {
            onOrderRefreshCatalog();
        } else if (currentTab && currentTab->objectName() == "tab_map") {
            populateMapClients();
        }
    });
    
    // New connections
    connect(ui_employee->btn_clear,            &QPushButton::clicked, this, &MainWindow::onEmployeeClearFields);
    connect(ui_employee->btn_send_mail,       &QPushButton::clicked, this, &MainWindow::onEmployeeSendMail);
    connect(ui_employee->btn_export_pdf,      &QPushButton::clicked, this, &MainWindow::onEmployeeExportHistoryPDF);
    connect(ui_employee->le_history_search,   &QLineEdit::textChanged, this, &MainWindow::onEmployeeHistorySearch);
    connect(ui_employee->cb_history_filter,   &QComboBox::currentIndexChanged, this, &MainWindow::onEmployeeHistorySearch);
    connect(ui_employee->cb_mail_template,    &QComboBox::currentIndexChanged, this, &MainWindow::onEmployeeMailTemplateChanged);
    connect(ui_employee->btn_scan_face,       &QPushButton::clicked, this, &MainWindow::onScanFace);
    connect(ui_employee->btn_upload_avatar,   &QPushButton::clicked, this, &MainWindow::onUploadAvatar);
    // btn_modify / btn_delete are already connected earlier (avoid duplicate CRUD calls)
    
    connect(ui_supplier->btn_add,            &QPushButton::clicked, this, &MainWindow::onSupplierAdd);
    connect(ui_supplier->btn_modify,         &QPushButton::clicked, this, &MainWindow::onSupplierModify);
    connect(ui_supplier->btn_delete,         &QPushButton::clicked, this, &MainWindow::onSupplierDelete);
    connect(ui_supplier->btn_clear,          &QPushButton::clicked, this, &MainWindow::onSupplierClearFields);

    connect(ui_supplier->btn_export_pdf_view, &QPushButton::clicked, this, &MainWindow::onSupplierExportPDF);
    connect(ui_supplier->btn_print_view,      &QPushButton::clicked, this, &MainWindow::onSupplierPrint);
    connect(ui_supplier->btn_delete_all_view, &QPushButton::clicked, this, &MainWindow::onSupplierDeleteAll);

    connect(ui_supplier->btn_send_sms,       &QPushButton::clicked, this, &MainWindow::onSupplierSendSMS);
    connect(ui_supplier->btn_upload_image,   &QPushButton::clicked, this, &MainWindow::onSupplierUploadImage);
    connect(ui_supplier->btn_chercher,       &QPushButton::clicked, this, &MainWindow::onSupplierSearch);
    connect(ui_supplier->tableView, &QAbstractItemView::clicked, this, [this](const QModelIndex &idx){
        if (idx.column() == 0) { // Edit
            onSupplierLoad(idx);
        } else if (idx.column() == 1) { // Delete
            onSupplierDelete();
        }
    });

    // --- UI CLEANUP: Hide help buttons and chat icons as requested ---
    // Suppliers
    ui_supplier->btn_delete->hide();

    // Equipments
    ui_equipment->btn_delete->hide();
    
    // Chat Header Buttons
    ui_equipment->btn_chat_settings->hide();
    ui_equipment->btn_chat_music->hide();
    ui_equipment->btn_chat_refresh->hide();

    // Employees

    // Clients

    // Orders

    connect(ui_equipment->btn_clear,  &QPushButton::clicked, this, &MainWindow::onEquipmentClearFields);
    connect(ui_equipment->btn_add,    &QPushButton::clicked, this, &MainWindow::onEquipmentAdd);
    connect(ui_equipment->btn_modify, &QPushButton::clicked, this, &MainWindow::onEquipmentModify);
    connect(ui_equipment->btn_delete, &QPushButton::clicked, this, &MainWindow::onEquipmentDelete);
    connect(ui_equipment->btn_delete_confirm, &QPushButton::clicked, this, &MainWindow::onEquipmentDelete);
    connect(ui_equipment->btn_share_chat, &QPushButton::clicked, this, &MainWindow::onEquipmentShareToChat);
    connect(ui_equipment->btn_search, &QPushButton::clicked, this, &MainWindow::onEquipmentSearch);
    connect(ui_equipment->le_recherche, &QLineEdit::returnPressed, this, &MainWindow::onEquipmentSearch);
    // Load row into form when row is clicked in view tab
    connect(ui_equipment->table_equipments, &QAbstractItemView::clicked, this, [this](const QModelIndex &idx){
        if (idx.column() == 0) { // Edit
            QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
            if (!m) return;
            // Shifting by 2 actions
            QString id      = m->data(m->index(idx.row(), 2)).toString();
            QString type    = m->data(m->index(idx.row(), 3)).toString();
            int     qty     = m->data(m->index(idx.row(), 4)).toInt();
            double  price   = m->data(m->index(idx.row(), 5)).toDouble();
            QString cond    = m->data(m->index(idx.row(), 6)).toString();
            QString dateStr = m->data(m->index(idx.row(), 7)).toString();
            QString desc    = m->data(m->index(idx.row(), 8)).toString();

            ui_equipment->le_id->setText(id);
            ui_equipment->le_type->setText(type);
            ui_equipment->sb_quantity->setValue(qty);
            ui_equipment->dsb_unit_price->setValue(price);
            ui_equipment->te_desc->setText(desc);

            int statusIdx = ui_equipment->cb_status->findText(cond, Qt::MatchFixedString);
            if (statusIdx >= 0) ui_equipment->cb_status->setCurrentIndex(statusIdx);

            QDate pDate = QDate::fromString(dateStr, "dd/MM/yyyy");
            if (!pDate.isValid()) pDate = QDate::fromString(dateStr, "yyyy-MM-dd");
            if (pDate.isValid()) ui_equipment->de_date_achat->setDate(pDate);

            // Switch to management tab
            ui_equipment->tabWidget->setCurrentWidget(ui_equipment->tab_gestion);
            // Force modify mode so updating edits the selected record instead of adding a new one.
            if (QRadioButton *rbMod = ui_equipment->tab_gestion->findChild<QRadioButton*>("rb_equipment_mod_mode")) {
                rbMod->setChecked(true);
            }
        } else if (idx.column() == 1) { // Delete
            ui_equipment->table_equipments->setCurrentIndex(idx);
            onEquipmentDelete();
        }
    });

    // Hide global delete button
    ui_equipment->btn_delete->hide();
    // Auto-refresh equipment view when switching to view tab, and history when switching to history
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_view) {
            onEquipmentRefreshView();
        }
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_history) {
            onEquipmentHistoryRefresh();
        }
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_chat) {
            onChatEmployeeListRefresh();
            chatRefreshTimer->start(3000);
        } else if (idx == 5) { // NEXUS
            chatRefreshTimer->stop();
        } else if (idx == 6) { // COSTS
            chatRefreshTimer->stop();
        } else {
            chatRefreshTimer->stop();
        }
    });

    // History Connections
    connect(ui_equipment->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_clear_history,   &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryClear);
    connect(ui_equipment->le_history_search,   &QLineEdit::textChanged, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_history_search, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);

    // --- Supplier Delivery Rating System ---
    connect(ui_supplier->btn_submit_review,   &QPushButton::clicked, this, &MainWindow::onSupplierReviewSubmit);
    connect(ui_supplier->btn_refresh_reviews, &QPushButton::clicked, this, &MainWindow::onSupplierReviewLoad);
    connect(ui_supplier->cb_supplier_reviews, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int){ onSupplierReviewLoad(); });
    connect(ui_supplier->sb_rating, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSupplierReviewRatingChanged);

    // Auto-refresh when switching tabs (stats, view, reviews)
    connect(ui_supplier->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        ui_supplier->btn_help_gestion->setChecked(false);
        ui_supplier->btn_help_stats->setChecked(false);
        // ui_supplier->btn_help_reviews->setChecked(false); // Enable if present

        if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_stats) {
            setupSupplierStats();
        } else if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_view) {
            onSupplierRefreshView();
        } else if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_reviews) {
            onSupplierPopulateRatingCombos();
            onSupplierReviewLoad();
        }
    });

    onSupplierEnsureReviewsTable();
    onSupplierPopulateRatingCombos();
    onEmployeeEnsureHistoryTable();

    // --- Apply Hover Animations to Management Module Buttons ---
    // Employee Management
    for (auto* button : employeePage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Client Management
    for (auto* button : clientPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Supplier Management
    for (auto* button : supplierPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Equipment Management
    for (auto* button : equipmentPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Order Management
    for (auto* button : orderPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }

    // Connect page change signal for dynamic retranslation
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onPageChanged);
    
    // Start at login page
    ui->stackedWidget->setCurrentIndex(0);

    // Call setup function for client stats
    setupClientStats();
    
    // Consolidate Client Management Tabs
    setupClientManagement();

    // Setup Equipment Stats
    setupEquipmentStats();

    // Setup Employee Stats
    setupEmployeeStats();

    // Setup Supplier Stats
    setupSupplierStats();

    // Setup Client Calendar
    setupClientCalendar();

    // Standardize Add/Modify Modes
    setupEmployeeModes();
    setupSupplierModes();
    onSupplierRefreshView();
    setupEquipmentModes();
    setupOrderModes();

    // Hide tab bars and setup radio button navigation
    ui_client->tabWidget->tabBar()->hide();
    ui_employee->tabWidget->tabBar()->hide();
    ui_supplier->tabWidget->tabBar()->hide();
    ui_equipment->tabWidget->tabBar()->hide();
    ui_order->tabWidget->tabBar()->hide();
    // Setup radio button navigation for all UIs
    // Y set to (TabWidgetY + 25) to align with inner buttons.
    setupTabNavigation(clientPage, ui_client->tabWidget, {"Manage", "View", "Stats", "Mail", "Calendar"}, 150, 45, {0, 1, 2, 3, 4}, 115, 40);   // Override indices: Manage->0, View->1, Stats->2, Mail->3, Calendar->4
    setupTabNavigation(employeePage, ui_employee->tabWidget, {"Manage", "View", "Stats", "History"}, 150, 95, {}, 115, 40);  // 70+25
    setupTabNavigation(supplierPage, ui_supplier->tabWidget, {"Manage", "Stats", "View", "Reviews", "Map"}, 150, 45, {0, 1, 2, 3, 4}, 115, 40);
    setupTabNavigation(equipmentPage, ui_equipment->tabWidget, {"Manage", "View", "History", "Stats", "Chat", "NEXUS", "COSTS"}, 96, 95, {0, 1, 2, 3, 4, 5, 6}, 103, 34);
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
        Q_UNUSED(idx);
        if (!equipmentPage) return;
        const QStringList equipTabs = {"Manage", "View", "History", "Stats", "Chat", "NEXUS", "COSTS"};
        const int startX = 96;
        const int y = 95;
        const int spacing = 103;
        const int afterFirstShift = 34;
        const int manageNudgeRight = 50;
        const int costsNudgeLeft = 16;
        for (int i = 0; i < equipTabs.size(); ++i) {
            const auto radios = equipmentPage->findChildren<QRadioButton*>();
            for (auto *rb : radios) {
                if (!rb) continue;
                if (rb->property("trKey").toString() == equipTabs[i]) {
                    const int extra = (i > 0 ? afterFirstShift : 0) + (i == 0 ? manageNudgeRight : 0) + (i == 6 ? -costsNudgeLeft : 0);
                    rb->move(startX + (spacing * i) + extra, y);
                    break;
                }
            }
        }
    });
    ui_equipment->tabWidget->setCurrentIndex(ui_equipment->tabWidget->currentIndex());
    setupTabNavigation(orderPage, ui_order->tabWidget, {"Manage", "QR Code", "Catalog", "3D Modeling", "Map"}, 250, 85, {}, 125, 40);   // 60+25

    // ---- Voice Command Engine ------------------------------------------------
    {
        // Walk up from the exe to find vosk/vosk-model/ in the project tree
        auto findUpward = [](const QString &startDir, const QString &relPath) -> QString {
            QDir dir(startDir);
            for (int i = 0; i < 8; ++i) {
                const QString c = dir.absoluteFilePath(relPath);
                if (QDir(c).exists()) return c;
                if (!dir.cdUp()) break;
            }
            return startDir + "/" + relPath; // fallback keeps original error message
        };
        const QString modelPath = findUpward(
            QCoreApplication::applicationDirPath(), "vosk/vosk-model");
        m_voiceEngine = new VoiceCommandEngine(modelPath, this);
        connect(m_voiceEngine, &VoiceCommandEngine::commandDetected,
                this, &MainWindow::onVoiceCommand);
        connect(m_voiceEngine, &VoiceCommandEngine::listeningChanged,
                this, &MainWindow::onVoiceListeningChanged);
        // initFailed must be connected BEFORE init() is ever called
        connect(m_voiceEngine, &VoiceCommandEngine::initFailed, this,
                [this](const QString &msg) {
            QMessageBox::warning(this, "Voice Commands", msg);
            if (m_micBtn) m_micBtn->setChecked(false);
        });

        // Mic toggle button — sits in the status bar, visible on every page
        m_micBtn = new QPushButton("  Mic: OFF");
        m_micBtn->setCheckable(true);
        m_micBtn->setFixedHeight(28);
        m_micBtn->setCursor(Qt::PointingHandCursor);
        m_micBtn->setStyleSheet(R"(
            QPushButton {
                background: #2C2418; color: #806050;
                border: 1px solid #4A3728; border-radius: 6px;
                padding: 0 14px; font-size: 12px; font-weight: bold;
            }
            QPushButton:hover  { background: #3C3020; color: #A08060; }
            QPushButton:checked {
                background: #8B6F47; color: white;
                border: 1px solid #D4AF37;
            }
        )");
        statusBar()->addPermanentWidget(m_micBtn);
        statusBar()->setStyleSheet("background: #1C1610; border-top: 1px solid #3A2A1A;");

        connect(m_micBtn, &QPushButton::clicked, this, [this]() {
            if (!m_voiceEngine->isReady()) {
                // init() will emit initFailed if something is missing
                if (!m_voiceEngine->init()) {
                    m_micBtn->setChecked(false);
                    return;
                }
            }
            m_voiceEngine->toggleListening();
        });
    }
    // --------------------------------------------------------------------------

    // Standardize UI Styling
    setupGlobalStyles();
    
    // Connect all help buttons to toggle between ost1 and ost4 with fade
    auto connectHelpButton = [this](QWidget* page, const QString& buttonName, const QString& tutorialText) {
        QToolButton* btn = page->findChild<QToolButton*>(buttonName);
        if (btn) {
            connect(btn, &QToolButton::clicked, this, [this, btn, tutorialText]() {
                // Fade ost1 out and play ost4
                fadeOutAndPlay(loginAudioPlayer, loginAudioOutput,
                               tutorialLoopAudioPlayer, tutorialLoopAudioOutput);
                // Show blur overlay dialog (blocks until closed)
                showTutorialOverlay(tutorialText);
                // When dialog closes: uncheck the button, fade ost4 out and resume ost1
                btn->setChecked(false);
                tutorialLoopAudioPlayer->stop();
                fadeOutAndPlay(tutorialLoopAudioPlayer, tutorialLoopAudioOutput,
                               loginAudioPlayer, loginAudioOutput);
            });
        }
    };
    
    // Connect Employee Management help buttons
    connectHelpButton(employeePage, "btn_help_add", "This is the employee management tutorial. Here you can add, modify, or delete employees. Fill in all fields and click 'Add' to create a new employee.");

    // Connect Client Management help buttons
    connectHelpButton(clientPage, "btn_help_add", "This is the client management tutorial. Here you can add, modify, or delete clients. Fill in all fields and click 'Add' to create a new client.");

    // Connect Supplier Management help buttons
    connectHelpButton(supplierPage, "btn_help_gestion", "This is the supplier management tutorial. Here you can manage suppliers.");
    connectHelpButton(supplierPage, "btn_help_stats", "This is the supplier stats tutorial. Here you can view supplier statistics.");
    connectHelpButton(supplierPage, "btn_help_reviews", "This is the supplier reviews tutorial. Here you can view and manage supplier reviews.");

    // Connect Equipment Management help buttons
    connectHelpButton(equipmentPage, "btn_help_gestion", "This is the equipment management tutorial. Here you can manage equipment.");
    connectHelpButton(equipmentPage, "btn_help_stats", "This is the equipment stats tutorial. Here you can view equipment statistics.");
    connectHelpButton(equipmentPage, "btn_help_view", "This is the equipment list tutorial. Here you can search and filter through all existing equipment.");
    
    // Connect Weather Assistant
    if (auto *weatherBtn = equipmentPage->findChild<QPushButton*>("btn_weather_assistant")) {
        connect(weatherBtn, &QPushButton::clicked, this, &MainWindow::onWeatherAssistantClicked);
    }
    if (auto *botBtn = equipmentPage->findChild<QPushButton*>("btn_weather_bot")) {
        botBtn->hide(); // Hide the AI Bot button as requested
    }

// Connect 3D Model Gallery Buttons
    auto connect3DView = [this](const QString& btnName, const QString& query) {
        if (auto *btn = equipmentPage->findChild<QPushButton*>(btnName)) {
            connect(btn, &QPushButton::clicked, this, [query]() {
                QString url = QString("https://sketchfab.com/search?q=%1&type=models").arg(query);
                QDesktopServices::openUrl(QUrl(url));
            });
        }
    };
    connect3DView("btn_view_1", "industrial+drill");
    connect3DView("btn_view_2", "hydraulic+pump");
    connect3DView("btn_view_3", "engine+assembly");


    // Connect Order Management help buttons
    connectHelpButton(orderPage, "btn_help", "This is the order tutorial. Here you can add, modify, or delete orders. Fill in all fields and click 'Add' to create a new order. Use the search and catalog features to manage orders efficiently.");
    connectHelpButton(orderPage, "btn_help_qr", "This is the QR code tutorial. Here you can generate, save, and print QR codes for orders.");
    
    // Initialize audio player for management pages
    loginAudioPlayer = new QMediaPlayer(this);
    loginAudioOutput = new QAudioOutput(this);
    loginAudioPlayer->setAudioOutput(loginAudioOutput);
    loginAudioPlayer->setSource(QUrl("qrc:/assets/ost1.mp3"));
    loginAudioPlayer->setLoops(QMediaPlayer::Infinite); // Loop indefinitely
    loginAudioOutput->setVolume(currentVolume);
    
    // Initialize audio player for home page
    homeAudioPlayer = new QMediaPlayer(this);
    homeAudioOutput = new QAudioOutput(this);
    homeAudioPlayer->setAudioOutput(homeAudioOutput);
    homeAudioPlayer->setSource(QUrl("qrc:/assets/ost2.mp3"));
    homeAudioPlayer->setLoops(1); // Play once
    homeAudioOutput->setVolume(currentVolume);
    
    // Initialize audio player for tutorial (help buttons) - only ost4
    tutorialLoopAudioPlayer = new QMediaPlayer(this);
    tutorialLoopAudioOutput = new QAudioOutput(this);
    tutorialLoopAudioPlayer->setAudioOutput(tutorialLoopAudioOutput);
    tutorialLoopAudioPlayer->setSource(QUrl("qrc:/assets/ost4.mp3"));
    tutorialLoopAudioPlayer->setLoops(QMediaPlayer::Infinite);
    tutorialLoopAudioOutput->setVolume(currentVolume);

    // Initialize audio player for chat page (classical style)
    chatAudioPlayer = new QMediaPlayer(this);
    chatAudioOutput = new QAudioOutput(this);
    chatAudioPlayer->setAudioOutput(chatAudioOutput);
    chatAudioPlayer->setSource(QUrl("qrc:/assets/ost2.mp3")); // Synced with Home music
    chatAudioPlayer->setLoops(QMediaPlayer::Infinite);
    chatAudioOutput->setVolume(currentVolume); // Removed 0.4 scaling for full volume

    // Music toggle button (OST2 removed from chat)

    // Connect Chat Settings Button ("§")
    if (auto *settingsBtn = equipmentPage->findChild<QPushButton*>("btn_chat_settings")) {
        settingsBtn->setIcon(QIcon(":/assets/gear.png"));
        settingsBtn->setIconSize(QSize(22, 22));
        settingsBtn->setText("");
        connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onChatSettingsClicked);
    }
    
    // Connect Chat Refresh Button
    if (auto *refreshBtn = equipmentPage->findChild<QPushButton*>("btn_chat_refresh")) {
        refreshBtn->setIcon(QIcon(":/assets/refresh.png"));
        refreshBtn->setIconSize(QSize(22, 22));
        refreshBtn->setText("");
        connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onChatRefresh);
    }

}


void MainWindow::showTutorialOverlay(const QString &text)
{
    // Apply blur effect to background
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(8.0);
    this->setGraphicsEffect(blur);

    // Create overlay dialog
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setAlignment(Qt::AlignCenter);

    QFrame *card = new QFrame(&dialog);
    card->setObjectName("tutorialCard");
    card->setStyleSheet("QFrame#tutorialCard { background: #fffbe6; border-radius: 16px; border: 2px solid #8B6F47; padding: 32px; }");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    QLabel *label = new QLabel(text, card);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setStyleSheet("font-size: 20px; color: #6b4f1d; font-weight: bold;");
    cardLayout->addWidget(label);

    QPushButton *closeBtn = new QPushButton("Close", card);
    closeBtn->setObjectName("closeTutorialBtn");
    closeBtn->setStyleSheet("QPushButton#closeTutorialBtn { background: #8B6F47; color: white; font-size: 16px; border-radius: 8px; padding: 8px 24px; border: 2px solid transparent; }"
                           "QPushButton#closeTutorialBtn:hover { background: #a8845a; border: 2px solid #6b4f1d; }");
    cardLayout->addWidget(closeBtn, 0, Qt::AlignCenter);

    layout->addWidget(card, 0, Qt::AlignCenter);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();

    // Remove blur after closing
    this->setGraphicsEffect(nullptr);
}

void MainWindow::setupClientStats()
{
    // 1. Create/Clear Layout for the stats container
    if (!ui_client->widget_chart->layout()) {
        QHBoxLayout *layout = new QHBoxLayout(ui_client->widget_chart);
        ui_client->widget_chart->setLayout(layout);
    } else {
        QLayoutItem *child;
        while ((child = ui_client->widget_chart->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    }

    // --- CYBERPUNK THEME HELPERS ---
    QColor bgTrans(10, 10, 10, 180);
    QColor goldColor("#D4AF37");
    QColor silverColor("#C0C0C0");
    QFont chartFont("Consolas", 11, QFont::Bold);
    QFont titleFont("Outfit", 14, QFont::Bold);

    QSqlQuery q;
    
    // --- 1. Total Clients Widget ---
    int totalClients = 0;
    if (q.exec("SELECT COUNT(*) FROM CLIENTS") && q.next()) {
        totalClients = q.value(0).toInt();
    }
    
    QFrame *summaryBox = new QFrame();
    summaryBox->setMinimumWidth(250);
    summaryBox->setStyleSheet("QFrame { background: rgba(10, 10, 10, 0.7); border: 2px solid #D4AF37; border-radius: 10px; }");
    QVBoxLayout *sumLayout = new QVBoxLayout(summaryBox);
    
    QLabel *lblTitle = new QLabel(trKey("TOTAL CLIENTS"));
    lblTitle->setStyleSheet("color: #D4AF37; border: none; font-family: 'Outfit'; font-size: 16px; font-weight: bold;");
    lblTitle->setAlignment(Qt::AlignCenter);
    
    QLabel *lblCount = new QLabel(QString::number(totalClients));
    lblCount->setStyleSheet("color: #FFFFFF; border: none; font-family: 'Consolas'; font-size: 52px; font-weight: bold;");
    lblCount->setAlignment(Qt::AlignCenter);
    
    sumLayout->addStretch();
    sumLayout->addWidget(lblTitle);
    sumLayout->addWidget(lblCount);
    sumLayout->addStretch();

    // --- 2. CHART 1: PIE CHART (Gender Matrix) ---
    QPieSeries *series = new QPieSeries();
    int maleCount = 0, femaleCount = 0;
    
    if (q.exec("SELECT GENDER, COUNT(*) FROM CLIENTS GROUP BY GENDER")) {
        while (q.next()) {
            QString g = q.value(0).toString().trimmed();
            int c = q.value(1).toInt();
            if (g.compare("Male", Qt::CaseInsensitive) == 0) maleCount = c;
            else if (g.compare("Female", Qt::CaseInsensitive) == 0) femaleCount = c;
        }
    }
    
    if (maleCount == 0 && femaleCount == 0) {
        series->append("No Data", 1);
    } else {
        series->append(trKey("Male"), maleCount);
        series->append(trKey("Female"), femaleCount);
        
        QPieSlice *sliceMale = series->slices().at(0);
        sliceMale->setBrush(goldColor);
        sliceMale->setLabelVisible(maleCount > 0);
        sliceMale->setLabelColor(Qt::white);
        sliceMale->setLabelFont(chartFont);

        QPieSlice *sliceFemale = series->slices().at(1);
        sliceFemale->setBrush(silverColor);
        sliceFemale->setLabelVisible(femaleCount > 0);
        sliceFemale->setLabelColor(Qt::white);
        sliceFemale->setLabelFont(chartFont);
        if(femaleCount > 0) sliceFemale->setExploded();
    }

    QChart *chartPie = new QChart();
    chartPie->addSeries(series);
    chartPie->setTitle(trKey("Gender Distribution"));
    chartPie->setProperty("trTitleKey", "Gender Distribution");
    chartPie->setTitleFont(titleFont);
    chartPie->setTitleBrush(goldColor);
    chartPie->setBackgroundBrush(bgTrans);
    chartPie->legend()->setLabelBrush(Qt::white);
    chartPie->legend()->setFont(chartFont);
    chartPie->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartViewPie = new QChartView(chartPie);
    chartViewPie->setRenderHint(QPainter::Antialiasing);
    chartViewPie->setStyleSheet("background: transparent; border: 2px solid #8B6F47; border-radius: 10px;");

    // --- 3. CHART 2: BAR CHART (Contact Vectors) ---
    QBarSeries *seriesBar = new QBarSeries();
    QBarSet *domainSet = new QBarSet(trKey("Clients"));
    domainSet->setColor(goldColor);
    
    QStringList categories;
    int maxVal = 0;
    
    QString domainQuery = "SELECT SUBSTR(EMAIL, INSTR(EMAIL, '@') + 1) AS DOMAIN, COUNT(*) AS C "
                          "FROM CLIENTS WHERE EMAIL LIKE '%@%' "
                          "GROUP BY SUBSTR(EMAIL, INSTR(EMAIL, '@') + 1) "
                          "ORDER BY C DESC";
                          
    if (q.exec(domainQuery)) {
        int count = 0;
        while (q.next() && count < 5) { // Top 5 limits
            QString dom = q.value(0).toString().trimmed();
            int c = q.value(1).toInt();
            if(dom.isEmpty()) continue;
            
            // Extract just the provider name
            dom = dom.split('.').first().toUpper();
            
            *domainSet << c;
            categories << dom;
            if(c > maxVal) maxVal = c;
            count++;
        }
    }
    
    // Fallback if no valid emails found
    if (categories.isEmpty()) {
       *domainSet << 0;
       categories << "NONE";
    }

    seriesBar->append(domainSet);
    
    QChart *chartBar = new QChart();
    chartBar->addSeries(seriesBar);
    chartBar->setTitle(trKey("Top Email Providers"));
    chartBar->setProperty("trTitleKey", "Top Email Providers");
    chartBar->setTitleFont(titleFont);
    chartBar->setTitleBrush(goldColor);
    chartBar->setBackgroundBrush(bgTrans);
    chartBar->legend()->hide();
    chartBar->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsBrush(Qt::white);
    axisX->setLabelsFont(chartFont);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    seriesBar->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxVal + 1);
    axisY->setLabelsBrush(Qt::white);
    axisY->setLabelsFont(chartFont);
    axisY->setGridLineColor(QColor(212, 175, 55, 40));
    chartBar->addAxis(axisY, Qt::AlignLeft);
    seriesBar->attachAxis(axisY);

    QChartView *chartViewBar = new QChartView(chartBar);
    chartViewBar->setRenderHint(QPainter::Antialiasing);
    chartViewBar->setStyleSheet("background: transparent; border: 2px solid #8B6F47; border-radius: 10px;");
    
    // Add dynamically mapped visuals to layout
    ui_client->widget_chart->layout()->addWidget(summaryBox);
    ui_client->widget_chart->layout()->addWidget(chartViewPie);
    ui_client->widget_chart->layout()->addWidget(chartViewBar);
}

MainWindow::~MainWindow()
{
    {
        QSettings settings("HammerDown", "HammerDown");
        settings.setValue("audio/volume", currentVolume);
    }
    delete ui;
    delete ui_client;
    delete ui_employee;
    delete ui_equipment;
    delete ui_order;
    delete ui_supplier;
}

// =============================================================================
// VOICE COMMANDS
// =============================================================================

// Helper: returns true if 'text' contains any of the given keywords
static bool hasAny(const QString &text, const QStringList &kw) {
    for (const QString &w : kw)
        if (text.contains(w)) return true;
    return false;
}

void MainWindow::onVoiceCommand(const QString &text)
{
    // ── Module navigation ────────────────────────────────────────────────────
    if (hasAny(text, {"client", "clients"})) {
        on_nav_clients_clicked();
    } else if (hasAny(text, {"employee", "employees", "staff"})) {
        on_nav_employees_clicked();
    } else if (hasAny(text, {"supplier", "suppliers"})) {
        on_nav_suppliers_clicked();
    } else if (hasAny(text, {"equipment"})) {
        on_nav_equipments_clicked();
    } else if (hasAny(text, {"order", "orders"})) {
        on_nav_orders_clicked();
    } else if (hasAny(text, {"home", "dashboard", "back"})) {
        on_btn_home_clicked();

    // ── Client tabs ──────────────────────────────────────────────────────────
    } else if (hasAny(text, {"manage", "management", "add"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(0);
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(0);
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(0);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(0);
        if (ui_order) ui_order->tabWidget->setCurrentIndex(0);
    } else if (hasAny(text, {"view", "list", "show all"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(1);
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(1);
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(2);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(1);
    } else if (hasAny(text, {"stats", "statistics", "chart", "analytics"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(2);
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(2);
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(1);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(3);
    } else if (hasAny(text, {"mail", "email", "message", "send"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(3);
    } else if (hasAny(text, {"calendar", "schedule", "events"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(4);
    } else if (hasAny(text, {"history", "log", "audit"})) {
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(3);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(2);
    } else if (hasAny(text, {"catalog"})) {
        if (ui_order) ui_order->tabWidget->setCurrentIndex(2);
    } else if (hasAny(text, {"map", "location"})) {
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(4);
        if (ui_order) ui_order->tabWidget->setCurrentIndex(4);
    } else if (hasAny(text, {"review", "rating"})) {
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(3);
    } else if (hasAny(text, {"nexus", "ai", "intelligence"})) {
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(5);
    } else if (hasAny(text, {"cost", "costs", "budget"})) {
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(6);
    }

    // Show a brief status hint
    statusBar()->showMessage("Voice: \"" + text + "\"", 3000);
}

void MainWindow::onVoiceListeningChanged(bool active)
{
    if (m_micBtn) {
        m_micBtn->setChecked(active);
        m_micBtn->setText(active ? "  Mic: ON" : "  Mic: OFF");
    }
    if (active)
        statusBar()->showMessage("Listening...", 0);
    else
        statusBar()->clearMessage();
}

void MainWindow::setAudioVolume(qreal volume)
{
    currentVolume = qBound<qreal>(0.0, volume, 1.5);
    {
        QSettings settings("HammerDown", "HammerDown");
        settings.setValue("audio/volume", currentVolume);
    }

    emit audioVolumeChanged(currentVolume);
    if (loginAudioOutput) {
        loginAudioOutput->setVolume(currentVolume);
    }
    if (homeWindow) {
        homeWindow->setVolume(currentVolume);
    }
    if (homeAudioOutput) {
        homeAudioOutput->setVolume(currentVolume);
    }
    if (tutorialLoopAudioOutput) {
        tutorialLoopAudioOutput->setVolume(currentVolume);
    }
    if (chatAudioOutput) {
        chatAudioOutput->setVolume(currentVolume);
    }
}

// Fade out audio and then play another audio with fade in
void MainWindow::fadeOutAndPlay(QMediaPlayer *fadeOutPlayer, QAudioOutput *fadeOutOutput,
                                 QMediaPlayer *fadeInPlayer, QAudioOutput *fadeInOutput)
{
    if (fadeOutPlayer && fadeOutPlayer->playbackState() == QMediaPlayer::PlayingState) {
        fadeOut(fadeOutOutput, [=]() {
            fadeOutPlayer->stop();
            if (fadeInPlayer) {
                fadeInPlayer->setPosition(0);
                fadeInPlayer->play();
                fadeIn(fadeInOutput);
            }
        });
    } else {
        if (fadeInPlayer) {
            fadeInPlayer->setPosition(0);
            fadeInPlayer->play();
            fadeIn(fadeInOutput);
        }
    }
}

// Fade out effect
void MainWindow::fadeOut(QAudioOutput *output, std::function<void()> onComplete)
{
    if (!output) return;
    
    QPropertyAnimation *fadeAnimation = new QPropertyAnimation(output, "volume");
    fadeAnimation->setDuration(500); // 500ms fade
    fadeAnimation->setStartValue(output->volume());
    fadeAnimation->setEndValue(0.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(fadeAnimation, &QPropertyAnimation::finished, this, [=]() {
        if (onComplete) onComplete();
        fadeAnimation->deleteLater();
    });
    
    fadeAnimation->start();
}

// Fade in effect
void MainWindow::fadeIn(QAudioOutput *output)
{
    if (!output) return;
    
    output->setVolume(0.0);
    
    QPropertyAnimation *fadeAnimation = new QPropertyAnimation(output, "volume");
    fadeAnimation->setDuration(500); // 500ms fade
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(currentVolume);
    fadeAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    connect(fadeAnimation, &QPropertyAnimation::finished, fadeAnimation, &QPropertyAnimation::deleteLater);
    
    fadeAnimation->start();
}

// --- Navigation Slots ---

// Login -> Home (Page 0 -> Page 1)
void MainWindow::on_login_clicked()
{
    m_homeWelcomeShown = false; // Reset to show welcome notification on fresh login
    updateUserProfileDisplay();
    ui->stackedWidget->setCurrentIndex(1); 
}

// Home -> Modules
void MainWindow::on_gs_employes_clicked()    { 
    ui->stackedWidget->setCurrentIndex(2); 
    ui_employee->tabWidget->setCurrentIndex(0); 
    onEmployeeRefreshView(); 
    onEmployeeRefreshHistory();
}
void MainWindow::on_gs_client_clicked()      { ui->stackedWidget->setCurrentIndex(3); ui_client->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_fournisseur_clicked() { ui->stackedWidget->setCurrentIndex(4); ui_supplier->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_equipment_clicked()   { ui->stackedWidget->setCurrentIndex(5); ui_equipment->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_order_clicked()       { ui->stackedWidget->setCurrentIndex(6); ui_order->tabWidget->setCurrentIndex(0); }

// Navigation sidebar
void MainWindow::on_nav_employees_clicked()  { 
    ui->stackedWidget->setCurrentIndex(2); 
    ui_employee->tabWidget->setCurrentIndex(0); 
    onEmployeeRefreshView(); 
    onEmployeeRefreshHistory();
}
void MainWindow::on_nav_clients_clicked()    { ui->stackedWidget->setCurrentIndex(3); ui_client->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_suppliers_clicked()  { ui->stackedWidget->setCurrentIndex(4); ui_supplier->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_equipments_clicked() { ui->stackedWidget->setCurrentIndex(5); ui_equipment->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_orders_clicked()     { ui->stackedWidget->setCurrentIndex(6); ui_order->tabWidget->setCurrentIndex(0); }

// Logout / Home
void MainWindow::on_btn_logout_clicked()
{
    currentEmployeeId = 0;
    currentChatPartnerId = -1;
    if (chatRefreshTimer) chatRefreshTimer->stop();
    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::on_btn_home_clicked()       
{ 
    // Stop ost4 (tutorial audio) if it's playing
    if (tutorialLoopAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
        tutorialLoopAudioPlayer->stop();
    }
    
    // Uncheck all help buttons when returning to home
    auto uncheckHelpButton = [](QWidget* page, const QString& buttonName) {
        QToolButton* btn = page ? page->findChild<QToolButton*>(buttonName) : nullptr;
        if (btn && btn->isCheckable()) {
            btn->setChecked(false);
        }
    };
    
    // Uncheck all help buttons across all pages
    uncheckHelpButton(employeePage, "btn_help_add");
    uncheckHelpButton(clientPage, "btn_help_add");
    uncheckHelpButton(supplierPage, "btn_help_gestion");
    uncheckHelpButton(supplierPage, "btn_help_stats");
    uncheckHelpButton(supplierPage, "btn_help_reviews");
    uncheckHelpButton(equipmentPage, "btn_help_gestion");
    uncheckHelpButton(equipmentPage, "btn_help_stats");
    uncheckHelpButton(orderPage, "btn_help");
    uncheckHelpButton(orderPage, "btn_help_qr");
    
    // Stop OST1 immediately and start OST2 when coming from a management page
    int prevIndex = ui->stackedWidget->currentIndex();
    if (prevIndex >= 2 && prevIndex <= 6) {
        loginAudioPlayer->stop();
        homeAudioOutput->setVolume(currentVolume);
        homeAudioPlayer->setPosition(0);
        homeAudioPlayer->play();
    }
    ui->stackedWidget->setCurrentIndex(1); 
}

void MainWindow::onOrderClearFields()
{
    if (ui_order) {
        ui_order->le_id->clear();
        if (ui_order->cb_type && ui_order->cb_type->count() > 0) {
            ui_order->cb_type->setCurrentIndex(0);
        }
        ui_order->le_stock->clear();
        ui_order->le_prix->clear();
        ui_order->le_buyer->clear();
        ui_order->le_qr_order_id->clear();
        ui_order->le_catalog_search->clear();
    }
}

void MainWindow::updateSalaryInsight()
{
    if (!ui_employee) return;
    QString role;
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) {
        role = cb->currentText().trimmed();
    } else {
        role = ui_employee->le_fonction->text().trimmed();
    }
    double currentSalary = ui_employee->dsb_salaire->value();

    if (role.isEmpty()) {
        ui_employee->lbl_salary_insight->setText("Market Avg: --");
        ui_employee->lbl_salary_insight->setStyleSheet("color: #D4AF37; font-size: 11px; font-weight: bold; background: transparent;");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT AVG(SALARY) FROM EMPLOYEES WHERE JOB_TITLE = :role");
    q.bindValue(":role", role);
    
    if (q.exec() && q.next()) {
        double avg = q.value(0).toDouble();
        if (avg > 0) {
            QString trend = (currentSalary > avg) ? "↑ High" : (currentSalary < avg) ? "↓ Low" : "● Fair";
            QString color = (currentSalary > avg * 1.5) ? "#FF5252" : (currentSalary > avg) ? "#D4AF37" : "#4CAF50";
            
            ui_employee->lbl_salary_insight->setText(QString("Market Avg: $%1 (%2)").arg(avg, 0, 'f', 0).arg(trend));
            ui_employee->lbl_salary_insight->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: bold; background: transparent;").arg(color));
        } else {
            ui_employee->lbl_salary_insight->setText("New Role: Competitive Area");
            ui_employee->lbl_salary_insight->setStyleSheet("color: #D4AF37; font-size: 11px; font-weight: bold; background: transparent;");
        }
    }
}

void MainWindow::onSuggestSalary()
{
    if (!ui_employee) return;
    QString role;
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) {
        role = cb->currentText().trimmed();
    } else {
        role = ui_employee->le_fonction->text().trimmed();
    }
    if (role.isEmpty()) return;

    QSqlQuery q;
    q.prepare("SELECT AVG(SALARY) FROM EMPLOYEES WHERE JOB_TITLE = :role");
    q.bindValue(":role", role);
    
    if (q.exec() && q.next()) {
        double avg = q.value(0).toDouble();
        if (avg > 0) {
            ui_employee->dsb_salaire->setValue(avg);
        } else {
            // Suggest a default based on typical ranges if no data exists
            ui_employee->dsb_salaire->setValue(2500); 
        }
    }
}

void MainWindow::onOrderAdd()
{
    if (!ui_order) return;
    
    // Check if we are in Add mode (where ID is auto-generated)
    bool isAddMode = false;
    QRadioButton *rbAdd = ui_order->tab_manage->findChild<QRadioButton*>("rb_order_add_mode");
    if (rbAdd && rbAdd->isChecked()) {
        isAddMode = true;
    }

    QString type = ui_order->cb_type ? ui_order->cb_type->currentText() : QString();
    QString stock = ui_order->le_stock->text();
    QString prix = ui_order->le_prix->text();
    QString buyer = ui_order->le_buyer->text();

    int orderId = 0;
    
    if (isAddMode) {
        // Find the lowest available (missing) positive integer
        // By checking where (order_id + 1) does NOT exist in the table.
        // We also handle the case where 1 itself is missing or the table is empty.
        QSqlQuery query;
        QString qStr = "SELECT MIN(t1.order_id + 1) AS next_id "
                       "FROM ORDERS t1 "
                       "WHERE NOT EXISTS (SELECT 1 FROM ORDERS t2 WHERE t2.order_id = t1.order_id + 1)";
                       
        // First check if '1' is available
        QSqlQuery checkOne("SELECT 1 FROM ORDERS WHERE order_id = 1");
        if (!checkOne.next()) {
            orderId = 1; // 1 is available
        } else if (query.exec(qStr) && query.next() && !query.value(0).isNull()) {
            orderId = query.value(0).toInt();
        } else {
            // Fallback (should theoretically never happen if 1 exists but just in case)
            QSqlQuery maxQuery("SELECT NVL(MAX(order_id), 0) + 1 FROM ORDERS");
            if (maxQuery.next()) {
                orderId = maxQuery.value(0).toInt();
            } else {
                orderId = 1;
            }
        }
    } else {
        // Required for Modify (shouldn't be reached from Add button, but safe to keep)
        QString id = ui_order->le_id->text();
        if (id.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Order ID is required!");
            return;
        }
        bool idOk;
        orderId = id.toInt(&idOk);
        if (!idOk) {
            QMessageBox::warning(this, "Input Error", "Order ID must be a whole number.");
            return;
        }
        
        // Check if order ID already exists
        QSqlQuery checkOrder;
        checkOrder.prepare("SELECT COUNT(*) FROM ORDERS WHERE order_id = :id");
        checkOrder.bindValue(":id", orderId);
        if (checkOrder.exec() && checkOrder.next() && checkOrder.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Duplicate Error", 
                "Order ID " + QString::number(orderId) + " already exists!\n\n"
                "Please use a different Order ID.");
            return;
        }
    }
    
    // Validate other fields
    if (type.isEmpty() || stock.isEmpty() || prix.isEmpty() || buyer.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required!");
        return;
    }
    
    // Validate numeric inputs
    bool buyerOk, stockOk, priceOk;
    
    int clientId = buyer.toInt(&buyerOk);
    int quantity = stock.toInt(&stockOk);
    double price = prix.toDouble(&priceOk);
    
    if (!buyerOk || !stockOk || !priceOk) {
        QMessageBox::warning(this, "Input Error", 
            "Please enter valid numbers:\n"
            "• Client ID: whole number\n"
            "• Quantity: whole number\n"
            "• Price: decimal number");
        return;
    }
    
    // Check if client exists
    QSqlQuery checkClient;
    checkClient.prepare("SELECT FIRST_NAME, LAST_NAME FROM CLIENTS WHERE CLIENT_ID = :id");
    checkClient.bindValue(":id", clientId);
    
    if (!checkClient.exec() || !checkClient.next()) {
        QMessageBox::warning(this, "Invalid Client ID", 
            "Client ID " + QString::number(clientId) + " does not exist!\n\n"
            "Please enter a valid Client ID from the Clients table.\n"
            "You can check existing clients in the Client Management section.");
        return;
    }
    
    QString clientName = checkClient.value(0).toString() + " " + checkClient.value(1).toString();
    
    // Confirm order creation
    int reply = QMessageBox::question(this, "Confirm Order", 
        "Create order with these details?\n\n"
        "Order ID: " + QString::number(orderId) + " (Auto)\n"
        "Client: " + clientName + " (ID: " + QString::number(clientId) + ")\n"
        "Type: " + type + "\n"
        "Quantity: " + QString::number(quantity) + "\n"
        "Price: $" + QString::number(price, 'f', 2),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::No) return;
    
    // Insert order
    QSqlQuery query;
    query.prepare("INSERT INTO ORDERS (order_id, client_id, employee_id, order_type, total_quantity, total_price, order_date, order_status, payment_status) "
                  "VALUES (:id, :buyer, :employee, :type, :quantity, :price, SYSDATE, 'Pending', 'Unpaid')");
    query.bindValue(":id", orderId);
    query.bindValue(":buyer", clientId);
    query.bindValue(":employee", currentEmployeeId);
    query.bindValue(":type", type);
    query.bindValue(":quantity", quantity);
    query.bindValue(":price", price);
    
    if (query.exec()) {
        QMessageBox::information(this, "Success", "Order #" + QString::number(orderId) + " added successfully!");
        logActivity("Added new order #" + QString::number(orderId) + " for Client ID: " + buyer, "Orders");
        onOrderClearFields();
        onOrderRefreshCatalog();
    } else {
        QString errorMsg = query.lastError().databaseText();
        QMessageBox::critical(this, "Database Error", 
            "Failed to add order.\n\n" + errorMsg);
    }
}

void MainWindow::onOrderModify()
{
    if (!ui_order) return;
    
    QString id = ui_order->le_id->text();
    QString type = ui_order->cb_type ? ui_order->cb_type->currentText() : QString();
    QString stock = ui_order->le_stock->text();
    QString prix = ui_order->le_prix->text();
    QString buyer = ui_order->le_buyer->text();
    
    // Validate input
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Order ID is required!");
        return;
    }
    
    if (type.isEmpty() || stock.isEmpty() || prix.isEmpty() || buyer.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled to modify!");
        return;
    }
    
    // Validate numeric inputs
    bool idOk, buyerOk, stockOk, priceOk;
    id.toInt(&idOk);
    buyer.toInt(&buyerOk);
    stock.toInt(&stockOk);
    prix.toDouble(&priceOk);
    
    if (!idOk || !buyerOk || !stockOk || !priceOk) {
        QMessageBox::warning(this, "Input Error",
            "Please enter valid numbers:\n"
            "\u2022 Order ID: whole number\n"
            "\u2022 Client ID: whole number\n"
            "\u2022 Quantity: whole number\n"
            "\u2022 Price: decimal number");
        return;
    }
    
    // Check if client exists
    QSqlQuery checkClient;
    checkClient.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
    checkClient.bindValue(":id", buyer.toInt());
    if (!checkClient.exec() || !checkClient.next() || checkClient.value(0).toInt() == 0) {
        QMessageBox::warning(this, "Invalid Client ID",
            "Client ID " + buyer + " does not exist!");
        return;
    }
    
    QSqlQuery query;
    query.prepare("UPDATE ORDERS SET order_type = :type, total_quantity = :quantity, "
                  "total_price = :price, client_id = :buyer WHERE order_id = :id");
    query.bindValue(":id", id.toInt());
    query.bindValue(":type", type);
    query.bindValue(":quantity", stock.toInt());
    query.bindValue(":price", prix.toDouble());
    query.bindValue(":buyer", buyer.toInt());
    
    if (query.exec()) {
        if (query.numRowsAffected() > 0) {
            QMessageBox::information(this, "Success", "Order modified successfully!");
            logActivity("Modified order #" + id, "Orders");
            onOrderClearFields();
            onOrderRefreshCatalog();
        } else {
            QMessageBox::warning(this, "Not Found", "Order ID not found in database!");
        }
    } else {
        QMessageBox::critical(this, "Database Error", 
            "Failed to modify order.\n\nTechnical details: " + query.lastError().databaseText());
    }
}

void MainWindow::onOrderDelete()
{
    if (!ui_order) return;
    
    QString id = ui_order->le_id->text();
    
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Order ID is required!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        "Are you sure you want to delete order: " + id + "?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::No) {
        return;
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM ORDERS WHERE order_id = :id");
    query.bindValue(":id", id.toInt());
    
    if (query.exec()) {
        if (query.numRowsAffected() > 0) {
            QMessageBox::information(this, "Success", "Order deleted successfully!");
            logActivity("Deleted order #" + id, "Orders");
            onOrderClearFields();
            onOrderRefreshCatalog();
        } else {
            QMessageBox::warning(this, "Not Found", "Order ID not found in database!");
        }
    } else {
        QMessageBox::critical(this, "Database Error", 
            "Failed to delete order.\n\nTechnical details: " + query.lastError().databaseText());
    }
}

void MainWindow::onOrderDeleteAll()
{
    if (!ui_order) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete All", 
        "Are you sure you want to delete ALL orders? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (reply == QMessageBox::No) {
        return;
    }
    
    QSqlQuery query;
    if (query.exec("DELETE FROM ORDERS")) {
        QMessageBox::information(this, "Success", "All orders have been deleted successfully!");
        logActivity("Deleted ALL orders from database", "Orders");
        onOrderClearFields();
        onOrderRefreshCatalog();
    } else {
        QMessageBox::critical(this, "Database Error", 
            "Failed to delete orders.\n\nTechnical details: " + query.lastError().databaseText());
    }
}

void MainWindow::onOrderLoad()
{
    if (!ui_order) return;
    
    QString id = ui_order->le_id->text();
    
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Order ID is required!");
        return;
    }
    
    QSqlQuery query;
    query.prepare("SELECT order_type, total_quantity, total_price, client_id FROM ORDERS WHERE order_id = :id");
    query.bindValue(":id", id.toInt());
    
    if (query.exec() && query.next()) {
        const QString dbType = query.value(0).toString();
        if (ui_order->cb_type) {
            int idx = ui_order->cb_type->findText(dbType, Qt::MatchFixedString);
            if (idx == -1 && !dbType.isEmpty()) {
                ui_order->cb_type->addItem(dbType);
                idx = ui_order->cb_type->findText(dbType, Qt::MatchFixedString);
            }
            if (idx >= 0) {
                ui_order->cb_type->setCurrentIndex(idx);
            }
        }
        ui_order->le_stock->setText(query.value(1).toString());
        ui_order->le_prix->setText(query.value(2).toString());
        ui_order->le_buyer->setText(query.value(3).toString());
        QMessageBox::information(this, "Success", "Order loaded successfully!");
    } else {
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, "Database Error", 
                "Failed to load order.\n\nTechnical details: " + query.lastError().databaseText());
        } else {
            QMessageBox::warning(this, "Not Found", "Order ID not found in database!");
        }
    }
}

// ==================== QR Code Helper ====================
static QPixmap generateQrPixmap(const QString &text, int pixelSize = 8, int border = 4)
{
    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(text.toUtf8().constData(), QrCode::Ecc::MEDIUM);
    int qrSize = qr.getSize();
    int imgSize = (qrSize + border * 2) * pixelSize;

    QImage img(imgSize, imgSize, QImage::Format_RGB32);
    img.fill(Qt::white);

    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {
            if (qr.getModule(x, y)) {
                for (int dy = 0; dy < pixelSize; dy++) {
                    for (int dx = 0; dx < pixelSize; dx++) {
                        img.setPixel((x + border) * pixelSize + dx,
                                     (y + border) * pixelSize + dy,
                                     qRgb(0, 0, 0));
                    }
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

static QString buildOrderQrContent(int orderId, const QString &orderType, int quantity,
                                    double price, const QString &orderDate,
                                    const QString &orderStatus, const QString &paymentStatus,
                                    int clientId, const QString &clientName,
                                    const QString &clientEmail, const QString &clientPhone)
{
    QString content;
    content += "=== Hammer Down Order Invoice ===\n";
    content += "Order #" + QString::number(orderId) + "\n";
    content += "Date: " + orderDate + "\n";
    content += "Type: " + orderType + "\n";
    content += "Quantity: " + QString::number(quantity) + "\n";
    content += "Unit Price: $" + QString::number(price, 'f', 2) + "\n";
    content += "Total: $" + QString::number(price * quantity, 'f', 2) + "\n";
    content += "Status: " + orderStatus + "\n";
    content += "Payment: " + paymentStatus + "\n";
    content += "---\n";
    content += "Client #" + QString::number(clientId) + "\n";
    content += "Name: " + clientName + "\n";
    if (!clientEmail.isEmpty()) content += "Email: " + clientEmail + "\n";
    if (!clientPhone.isEmpty()) content += "Phone: " + clientPhone + "\n";
    content += "==============================";
    return content;
}

void MainWindow::onOrderRefreshCatalog()
{
    if (!ui_order) return;
    
    QSqlQuery query;
    if (!query.exec("SELECT order_id, order_type, total_quantity, total_price, client_id FROM ORDERS ORDER BY order_id")) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to load orders.\n\nTechnical details: " + query.lastError().databaseText());
        return;
    }
    
    // Clear existing rows
    ui_order->table_catalog->setRowCount(0);
    
    int row = 0;
    while (query.next()) {
        ui_order->table_catalog->insertRow(row);
        
        // Order ID
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(query.value(0).toString());
        orderIdItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 0, orderIdItem);
        
        // Type
        QTableWidgetItem *typeItem = new QTableWidgetItem(trKey(query.value(1).toString()));
        typeItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        ui_order->table_catalog->setItem(row, 1, typeItem);
        
        // Quantity
        QTableWidgetItem *qtyItem = new QTableWidgetItem(query.value(2).toString());
        qtyItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 2, qtyItem);
        
        double unitPrice = query.value(3).toDouble();
        double totalPrice = unitPrice * query.value(2).toInt();
        QTableWidgetItem *unitPriceItem = new QTableWidgetItem(QString::number(unitPrice, 'f', 2));
        unitPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 3, unitPriceItem);
        
        // Total Price
        QTableWidgetItem *totalPriceItem = new QTableWidgetItem(QString::number(totalPrice, 'f', 2));
        totalPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 4, totalPriceItem);
        
        // Buyer ID
        QTableWidgetItem *buyerItem = new QTableWidgetItem(query.value(4).toString());
        buyerItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 5, buyerItem);
        
        // QR Code thumbnail
        QString qrText = "Order #" + query.value(0).toString()
                        + " | Type: " + query.value(1).toString()
                        + " | Qty: " + query.value(2).toString()
                        + " | Unit: $" + QString::number(unitPrice, 'f', 2)
                        + " | Total: $" + QString::number(totalPrice, 'f', 2)
                        + " | Client: " + query.value(4).toString();
        QPixmap qrPix = generateQrPixmap(qrText, 2, 1);
        QTableWidgetItem *qrItem = new QTableWidgetItem();
        qrItem->setData(Qt::DecorationRole, qrPix.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation));
        qrItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 6, qrItem);
        ui_order->table_catalog->setRowHeight(row, 62);
        
        row++;
    }
    
    // Keep columns stretched so the table always fills available width.
    ui_order->table_catalog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onOrderSearchCatalog()
{
    if (!ui_order) return;
    
    QString searchText = ui_order->le_catalog_search->text().trimmed();
    
    if (searchText.isEmpty()) {
        onOrderRefreshCatalog();
        return;
    }
    
    QSqlQuery query;
    query.prepare("SELECT order_id, order_type, total_quantity, total_price, client_id FROM ORDERS "
                  "WHERE CAST(order_id AS VARCHAR2(50)) LIKE :search "
                  "OR UPPER(order_type) LIKE :search "
                  "OR CAST(client_id AS VARCHAR2(50)) LIKE :search "
                  "ORDER BY order_id");
    query.bindValue(":search", "%" + searchText.toUpper() + "%");
    
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to search orders.\n\nTechnical details: " + query.lastError().databaseText());
        return;
    }
    
    // Clear existing rows
    ui_order->table_catalog->setRowCount(0);
    
    int row = 0;
    while (query.next()) {
        ui_order->table_catalog->insertRow(row);
        
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(query.value(0).toString());
        orderIdItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 0, orderIdItem);

        QTableWidgetItem *typeItem = new QTableWidgetItem(trKey(query.value(1).toString()));
        typeItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        ui_order->table_catalog->setItem(row, 1, typeItem);

        QTableWidgetItem *qtyItem = new QTableWidgetItem(query.value(2).toString());
        qtyItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 2, qtyItem);
        
        double unitPrice = query.value(3).toDouble();
        double totalPrice = unitPrice * query.value(2).toInt();
        QTableWidgetItem *unitPriceItem = new QTableWidgetItem(QString::number(unitPrice, 'f', 2));
        unitPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 3, unitPriceItem);
        
        QTableWidgetItem *totalPriceItem = new QTableWidgetItem(QString::number(totalPrice, 'f', 2));
        totalPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 4, totalPriceItem);
        
        QTableWidgetItem *buyerItem = new QTableWidgetItem(query.value(4).toString());
        buyerItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 5, buyerItem);
        
        // QR Code thumbnail
        QString qrText = "Order #" + query.value(0).toString()
                        + " | Type: " + query.value(1).toString()
                        + " | Qty: " + query.value(2).toString()
                        + " | Unit: $" + QString::number(unitPrice, 'f', 2)
                        + " | Total: $" + QString::number(totalPrice, 'f', 2)
                        + " | Client: " + query.value(4).toString();
        QPixmap qrPix = generateQrPixmap(qrText, 2, 1);
        QTableWidgetItem *qrItem = new QTableWidgetItem();
        qrItem->setData(Qt::DecorationRole, qrPix.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation));
        qrItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 6, qrItem);
        ui_order->table_catalog->setRowHeight(row, 62);
        
        row++;
    }
    
    ui_order->table_catalog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onOrderExportCatalog()
{
    if (!ui_order) return;
    
    // Get the order ID from search bar
    QString searchId = ui_order->le_catalog_search->text().trimmed();
    
    if (searchId.isEmpty()) {
        QMessageBox::warning(this, tr("No Order Selected"), 
            tr("Please enter an Order ID in the search box to export."));
        return;
    }
    
    // Validate it's a number
    bool ok;
    int orderId = searchId.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Invalid Order ID"), 
            tr("Please enter a valid Order ID number."));
        return;
    }
    
    // Query the specific order
    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderId);
    
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Order Not Found"), 
            tr("Order ID %1 does not exist in the database.").arg(searchId));
        return;
    }
    
    // Extract order data
    QString orderType = query.value(1).toString();
    int quantity = query.value(2).toInt();
    double price = query.value(3).toDouble();
    int clientId = query.value(4).toInt();
    QString orderDate = query.value(5).toDateTime().toString("MMMM dd, yyyy");
    QString orderStatus = query.value(6).toString();
    QString paymentStatus = query.value(7).toString();
    QString clientFirstName = query.value(8).toString();
    QString clientLastName = query.value(9).toString();
    QString clientEmail = query.value(10).toString();
    QString clientPhone = query.value(11).toString();
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Order to PDF"), 
                                                    QDir::homePath() + "/Order_" + searchId + ".pdf",
                                                    "PDF Files (*.pdf)");
    
    if (fileName.isEmpty()) return;
    
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageSize(QPageSize(QPageSize::A4));
    
    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to create PDF file.");
        return;
    }
    
    // Page dimensions and margins
    int pageWidth = printer.width();
    int pageHeight = printer.height();
    int margin = 80;  // 1+ inch margins for professional look
    int contentWidth = pageWidth - 2 * margin;
    int y = margin;
    
    // ==================== HEADER SECTION ====================
    // System title bar with background
    painter.fillRect(0, 0, pageWidth, 100, QColor(45, 45, 45));
    
    // System title
    QFont titleFont("Segoe UI", 22, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(Qt::white);
    painter.drawText(margin, 35, "Order Management System");
    
    // Subtitle
    QFont subtitleFont("Segoe UI", 10);
    painter.setFont(subtitleFont);
    painter.setPen(QColor(220, 220, 220));
    painter.drawText(margin, 60, "Professional Order Processing & Invoice Generation");
    
    y = 130;
    
    // ==================== COMPANY & ORDER INFO ====================
    // Company logo and info (left side)
    int logoSize = 70;
    QPixmap logo(":/assets/logo.png");
    if (!logo.isNull()) {
        QPixmap scaledLogo = logo.scaled(logoSize, logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(margin, y, scaledLogo);
    }
    
    QFont companyFont("Segoe UI", 18, QFont::Bold);
    painter.setFont(companyFont);
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(margin + logoSize + 20, y + 25, "Hammer Down");
    
    QFont companySubFont("Segoe UI", 9);
    painter.setFont(companySubFont);
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(margin + logoSize + 20, y + 45, "Business Solutions Provider");
    
    // Order number and date (right side)
    QFont orderNumFont("Segoe UI", 20, QFont::Bold);
    painter.setFont(orderNumFont);
    painter.setPen(QColor(45, 45, 45));
    QString orderText = "ORDER #" + searchId;
    QFontMetrics fm1(orderNumFont);
    int orderWidth = fm1.horizontalAdvance(orderText);
    painter.drawText(pageWidth - margin - orderWidth, y + 25, orderText);
    
    QFont dateFont("Segoe UI", 9);
    painter.setFont(dateFont);
    painter.setPen(QColor(100, 100, 100));
    QString dateGenerated = "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy hh:mm AP");
    QFontMetrics fm2(dateFont);
    int dateWidth = fm2.horizontalAdvance(dateGenerated);
    painter.drawText(pageWidth - margin - dateWidth, y + 50, dateGenerated);
    
    y += 100;
    
    // Horizontal separator
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawLine(margin, y, pageWidth - margin, y);
    
    y += 40;
    
    // ==================== ORDER DETAILS SECTION ====================
    // Section header
    QFont sectionHeaderFont("Segoe UI", 14, QFont::Bold);
    painter.setFont(sectionHeaderFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin, y, "Order Details");
    
    y += 10;
    
    // Section underline
    painter.setPen(QPen(QColor(139, 111, 71), 3));
    painter.drawLine(margin, y, margin + 120, y);
    
    y += 30;
    
    // Order details box with light background
    int detailsBoxHeight = 180;
    painter.fillRect(margin, y, contentWidth, detailsBoxHeight, QColor(250, 250, 252));
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawRect(margin, y, contentWidth, detailsBoxHeight);
    
    y += 30;
    
    // Two-column layout for order info
    QFont labelFont("Segoe UI", 10, QFont::Bold);
    QFont valueFont("Segoe UI", 10);
    int labelCol = margin + 30;
    int valueCol = margin + 200;
    int rowHeight = 28;
    
    auto drawDetailRow = [&](const QString& label, const QString& value) {
        painter.setFont(labelFont);
        painter.setPen(QColor(90, 90, 90));
        painter.drawText(labelCol, y, label);
        
        painter.setFont(valueFont);
        painter.setPen(QColor(40, 40, 40));
        painter.drawText(valueCol, y, value);
        
        y += rowHeight;
    };
    
    drawDetailRow("Order Date:", orderDate);
    drawDetailRow("Order Type:", orderType);
    drawDetailRow("Quantity:", QString::number(quantity) + " units");
    drawDetailRow("Unit Price:", "$" + QString::number(price, 'f', 2));
    drawDetailRow("Order Status:", orderStatus);
    drawDetailRow("Payment Status:", paymentStatus);
    
    y += 30;
    
    // ==================== CLIENT INFORMATION SECTION ====================
    // Section header
    painter.setFont(sectionHeaderFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin, y, "Client Information");
    
    y += 10;
    
    // Section underline
    painter.setPen(QPen(QColor(139, 111, 71), 3));
    painter.drawLine(margin, y, margin + 140, y);
    
    y += 30;
    
    // Client details box
    int clientBoxHeight = 140;
    painter.fillRect(margin, y, contentWidth, clientBoxHeight, QColor(250, 250, 252));
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawRect(margin, y, contentWidth, clientBoxHeight);
    
    y += 30;
    
    drawDetailRow("Client ID:", QString::number(clientId));
    drawDetailRow("Full Name:", clientFirstName + " " + clientLastName);
    drawDetailRow("Email Address:", clientEmail.isEmpty() ? "Not provided" : clientEmail);
    drawDetailRow("Phone Number:", clientPhone.isEmpty() ? "Not provided" : clientPhone);
    
    y += 40;
    
    // ==================== PAYMENT SUMMARY ====================
    // Summary section with accent color
    int summaryBoxHeight = 100;
    painter.fillRect(margin, y, contentWidth, summaryBoxHeight, QColor(139, 111, 71));
    
    // Inner white box for amount
    int innerMargin = 3;
    painter.fillRect(margin + innerMargin, y + innerMargin, 
                     contentWidth - 2 * innerMargin, summaryBoxHeight - 2 * innerMargin, 
                     QColor(255, 255, 255));
    
    y += 40;
    
    // Total amount label
    QFont summaryLabelFont("Segoe UI", 16, QFont::Bold);
    painter.setFont(summaryLabelFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin + 30, y, "TOTAL AMOUNT");
    
    // Total amount value (right aligned)
    QFont totalAmountFont("Segoe UI", 24, QFont::Bold);
    painter.setFont(totalAmountFont);
    painter.setPen(QColor(139, 111, 71));
    double totalPrice = price * quantity;
    QString totalText = "$" + QString::number(totalPrice, 'f', 2);
    QFontMetrics fm3(totalAmountFont);
    int totalWidth = fm3.horizontalAdvance(totalText);
    painter.drawText(pageWidth - margin - totalWidth - 30, y + 5, totalText);
    
    y += 35;
    
    // Payment status in summary
    QFont statusFont("Segoe UI", 10);
    painter.setFont(statusFont);
    painter.setPen(QColor(100, 100, 100));
    QString statusText = "Payment Status: " + paymentStatus;
    painter.drawText(margin + 30, y, statusText);
    
    // ==================== QR CODE ====================
    y += 80;  // Move below the summary box
    
    // Build QR content with same info as PDF
    QString qrContent = buildOrderQrContent(orderId, orderType, quantity, price,
                                             orderDate, orderStatus, paymentStatus,
                                             clientId, clientFirstName + " " + clientLastName,
                                             clientEmail, clientPhone);
    QPixmap qrPixmap = generateQrPixmap(qrContent, 4, 2);
    
    // QR section label
    painter.setFont(sectionHeaderFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin, y, "Scan QR Code");
    y += 10;
    painter.setPen(QPen(QColor(139, 111, 71), 3));
    painter.drawLine(margin, y, margin + 120, y);
    y += 20;
    
    // Draw QR code
    int qrDisplaySize = 140;
    QPixmap scaledQr = qrPixmap.scaled(qrDisplaySize, qrDisplaySize, Qt::KeepAspectRatio, Qt::FastTransformation);
    painter.drawPixmap(margin, y, scaledQr);
    
    // QR description text next to QR code
    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(margin + qrDisplaySize + 20, y + 30, "Scan this QR code to view");
    painter.drawText(margin + qrDisplaySize + 20, y + 50, "complete order details.");
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor(140, 140, 140));
    painter.drawText(margin + qrDisplaySize + 20, y + 80, "Contains: Order info, client data,");
    painter.drawText(margin + qrDisplaySize + 20, y + 95, "pricing and payment status.");
    
    // ==================== FOOTER ======================================
    y = pageHeight - 50;
    
    // Footer separator line
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawLine(margin, y, pageWidth - margin, y);
    
    y += 25;
    
    // Centered footer text
    QFont footerFont("Segoe UI", 8);
    painter.setFont(footerFont);
    painter.setPen(QColor(120, 120, 120));
    
    QString footerText = "";
    QFontMetrics fmFooter(footerFont);
    int footerWidth = fmFooter.horizontalAdvance(footerText);
    int footerX = (pageWidth - footerWidth) / 2;
    painter.drawText(footerX, y, footerText);
    
    painter.end();
    
    QMessageBox::information(this, tr("Success"), 
        tr("Order #%1 exported successfully!\n\nFile saved to:\n%2").arg(searchId).arg(fileName));
}

void MainWindow::onOrderImportCatalog()
{
    if (!ui_order) return;
    
    QMessageBox msgBox;
    msgBox.setWindowTitle("Import Orders");
    msgBox.setText("Would you like to download a blank template to fill out, or import an already filled file?");
    QPushButton *btnTemplate = msgBox.addButton("Download Template", QMessageBox::ActionRole);
    QPushButton *btnImport = msgBox.addButton("Import File", QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == btnTemplate) {
        // Option 1: Generate Template with 4 columns + example
        QString fileName = QFileDialog::getSaveFileName(this, "Save Template", QDir::homePath() + "/Template_Orders.csv", "CSV Files (*.csv)");
        if (fileName.isEmpty()) return;
        
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Could not create the template file.");
            return;
        }
        
        QTextStream out(&file);
        const QChar delimiter = ';';
        out << "sep=" << delimiter << "\n";
        out << "Type" << delimiter << "Quantity" << delimiter << "Price" << delimiter << "BuyerID\n";
        out << "Other" << delimiter << "10" << delimiter << "15.50" << delimiter << "1\n";
        file.close();
        
        QMessageBox::information(this, "Template Created",
            "Template saved successfully!\n\n"
            "Please open it in Excel or Notepad, fill out your orders using exactly those 4 columns:\n"
            "  Type, Quantity, Price, BuyerID\n\n"
            "An example row is included. You can remove the header line if you want — the importer is smart enough to handle it either way.");
        
    } else if (msgBox.clickedButton() == btnImport) {
        // Option 2: Import Filled File — smart parsing
        QString fileName = QFileDialog::getOpenFileName(this, "Import Orders", QDir::homePath(), "CSV Files (*.csv);;Text Files (*.txt);;Excel Files (*.xlsx *.xls);;All Files (*)");
        if (fileName.isEmpty()) return;

        const QFileInfo fi(fileName);
        const QString suffix = fi.suffix().toLower();
        const bool isWorkbookExtension = (suffix == "xlsx" || suffix == "xls" || suffix == "xlsm" || suffix == "xlsb");
        const bool isXlsxExtension = (suffix == "xlsx");

        QString tempCsvPath;
        QString importPath = fileName;

        if (isWorkbookExtension) {
            tempCsvPath = QDir::tempPath() + "/orders_import_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".csv";
            QFile::remove(tempCsvPath);
            const QString tempPath = QDir::toNativeSeparators(tempCsvPath);

            auto psEscape = [](QString s) {
                s.replace("'", "''");
                return s;
            };

            const QString sourcePath = QDir::toNativeSeparators(fileName);

            auto runExcelComConversion = [&](QString *outError) {
                const QString psScript = QString(
                    "$ErrorActionPreference='Stop'; "
                    "$excel=$null; $wb=$null; "
                    "try { "
                    "  $excel=New-Object -ComObject Excel.Application; "
                    "  $excel.Visible=$false; "
                    "  $excel.DisplayAlerts=$false; "
                    "  $wb=$excel.Workbooks.Open('%1'); "
                    "  $wb.SaveAs('%2', 62); "
                    "  $wb.Close($false); "
                    "} finally { "
                    "  if ($wb -ne $null) { [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($wb) } "
                    "  if ($excel -ne $null) { $excel.Quit(); [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($excel) } "
                    "}"
                ).arg(psEscape(sourcePath), psEscape(tempPath));

                QProcess ps;
                ps.start("powershell", QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);
                const bool finished = ps.waitForFinished(120000);
                const bool ok = finished && ps.exitStatus() == QProcess::NormalExit && ps.exitCode() == 0;
                if (!ok && outError) {
                    *outError = QString::fromLocal8Bit(ps.readAllStandardError()).trimmed();
                    if (outError->isEmpty()) {
                        *outError = QString::fromLocal8Bit(ps.readAllStandardOutput()).trimmed();
                    }
                }
                return ok;
            };

            auto runOpenXmlPowerShellConversion = [&](QString *outError) {
                const QString psScript = QString(
                    "$ErrorActionPreference='Stop'; "
                    "$src='%1'; $dst='%2'; "
                    "Add-Type -AssemblyName System.IO.Compression.FileSystem; "
                    "$zip=[System.IO.Compression.ZipFile]::OpenRead($src); "
                    "try { "
                    "  function Get-EntryText($z,$name) { "
                    "    $entry=$z.GetEntry($name); if ($null -eq $entry) { return $null }; "
                    "    $sr=New-Object System.IO.StreamReader($entry.Open()); "
                    "    try { return $sr.ReadToEnd() } finally { $sr.Close() } "
                    "  }; "
                    "  $shared=@(); "
                    "  $sharedXml=Get-EntryText $zip 'xl/sharedStrings.xml'; "
                    "  if ($sharedXml) { "
                    "    [xml]$sx=$sharedXml; "
                    "    foreach($si in $sx.SelectNodes(\"//*[local-name()='si']\")) { "
                    "      $txt=''; foreach($t in $si.SelectNodes(\".//*[local-name()='t']\")) { $txt += [string]$t.InnerText }; $shared += $txt "
                    "    } "
                    "  }; "
                    "  [xml]$wb=(Get-EntryText $zip 'xl/workbook.xml'); "
                    "  [xml]$rels=(Get-EntryText $zip 'xl/_rels/workbook.xml.rels'); "
                    "  $sheet=$wb.SelectSingleNode(\"/*[local-name()='workbook']/*[local-name()='sheets']/*[local-name()='sheet']\"); "
                    "  if ($null -eq $sheet) { throw 'No worksheet found in workbook.' }; "
                    "  $rid=$sheet.GetAttribute('id','http://schemas.openxmlformats.org/officeDocument/2006/relationships'); "
                    "  $relNode=$rels.SelectSingleNode(\"/*[local-name()='Relationships']/*[local-name()='Relationship'][@Id='\" + $rid + \"']\"); "
                    "  $target=if($relNode){[string]$relNode.Attributes['Target'].Value}else{''}; "
                    "  if ([string]::IsNullOrWhiteSpace($target)) { throw 'Cannot resolve first worksheet relationship.' }; "
                    "  if ($target.StartsWith('/')) { $sheetPath=$target.TrimStart('/') } "
                    "  elseif ($target.StartsWith('xl/')) { $sheetPath=$target } "
                    "  else { $sheetPath='xl/' + $target }; "
                    "  [xml]$sh=(Get-EntryText $zip $sheetPath); "
                    "  $sw=New-Object System.IO.StreamWriter($dst,$false,[System.Text.UTF8Encoding]::new($false)); "
                    "  try { "
                    "    foreach($row in @($sh.SelectNodes(\"/*[local-name()='worksheet']/*[local-name()='sheetData']/*[local-name()='row']\"))) { "
                    "      $map=@{}; $max=-1; "
                    "      foreach($c in @($row.SelectNodes(\"*[local-name()='c']\"))) { "
                    "        $ref=[string]$c.GetAttribute('r'); $letters=''; "
                    "        for($i=0; $i -lt $ref.Length; $i++) { $ch=$ref[$i]; if ($ch -ge 'A' -and $ch -le 'Z') { $letters += $ch } else { break } }; "
                    "        $idx=0; foreach($ch in $letters.ToCharArray()) { $idx = ($idx * 26) + ([int][char]$ch - 64) }; $idx=$idx-1; "
                    "        if ($idx -lt 0) { $idx = 0 }; if ($idx -gt $max) { $max=$idx }; "
                    "        $t=[string]$c.GetAttribute('t'); $value=''; "
                    "        if ($t -eq 's') { "
                    "          $vNode=$c.SelectSingleNode(\"*[local-name()='v']\"); $raw=if($vNode){[string]$vNode.InnerText}else{''}; if ($raw -match '^\\d+$') { $si=[int]$raw; if ($si -ge 0 -and $si -lt $shared.Count) { $value=$shared[$si] } } "
                    "        } elseif ($t -eq 'inlineStr') { "
                    "          $isNode=$c.SelectSingleNode(\"*[local-name()='is']\"); if($isNode){ foreach($n in @($isNode.SelectNodes(\".//*[local-name()='t']\"))){ $value += [string]$n.InnerText } } "
                    "        } else { "
                    "          $vNode=$c.SelectSingleNode(\"*[local-name()='v']\"); if($vNode){ $value=[string]$vNode.InnerText } "
                    "        }; "
                    "        $map[$idx]=$value; "
                    "      }; "
                    "      if ($max -lt 0) { continue }; "
                    "      $vals=New-Object System.Collections.Generic.List[string]; "
                    "      for($i=0; $i -le $max; $i++) { if ($map.ContainsKey($i)) { [void]$vals.Add([string]$map[$i]) } else { [void]$vals.Add('') } }; "
                    "      while($vals.Count -gt 0 -and [string]::IsNullOrEmpty($vals[$vals.Count-1])) { $vals.RemoveAt($vals.Count-1) }; "
                    "      $escaped=@(); foreach($v in $vals) { $escaped += ('\"' + ($v -replace '\"','\"\"') + '\"') }; "
                    "      $sw.WriteLine(($escaped -join ',')); "
                    "    } "
                    "  } finally { $sw.Close() } "
                    "} finally { $zip.Dispose() }"
                ).arg(psEscape(sourcePath), psEscape(tempPath));

                QProcess ps;
                ps.start("powershell", QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);
                const bool finished = ps.waitForFinished(120000);
                const bool ok = finished && ps.exitStatus() == QProcess::NormalExit && ps.exitCode() == 0;
                if (!ok && outError) {
                    *outError = QString::fromLocal8Bit(ps.readAllStandardError()).trimmed();
                    if (outError->isEmpty()) {
                        *outError = QString::fromLocal8Bit(ps.readAllStandardOutput()).trimmed();
                    }
                }
                return ok;
            };

            auto runPythonXlsxConversion = [&](QString *outError) {
                const QString pyScript =
                    "import csv, re, sys, zipfile, xml.etree.ElementTree as ET\n"
                    "src, dst = sys.argv[1], sys.argv[2]\n"
                    "NS_MAIN='http://schemas.openxmlformats.org/spreadsheetml/2006/main'\n"
                    "NS_REL_DOC='http://schemas.openxmlformats.org/officeDocument/2006/relationships'\n"
                    "NS_REL_PKG='http://schemas.openxmlformats.org/package/2006/relationships'\n"
                    "def col_to_idx(ref):\n"
                    "    m = re.match(r'([A-Z]+)', ref or '')\n"
                    "    if not m: return 0\n"
                    "    idx = 0\n"
                    "    for ch in m.group(1): idx = idx * 26 + (ord(ch) - 64)\n"
                    "    return idx - 1\n"
                    "with zipfile.ZipFile(src) as z:\n"
                    "    shared = []\n"
                    "    if 'xl/sharedStrings.xml' in z.namelist():\n"
                    "        sroot = ET.fromstring(z.read('xl/sharedStrings.xml'))\n"
                    "        for si in sroot.findall('{%s}si' % NS_MAIN):\n"
                    "            txt = ''.join(t.text or '' for t in si.findall('.//{%s}t' % NS_MAIN))\n"
                    "            shared.append(txt)\n"
                    "    wb = ET.fromstring(z.read('xl/workbook.xml'))\n"
                    "    rels = ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))\n"
                    "    rel_map = {}\n"
                    "    for rel in rels.findall('{%s}Relationship' % NS_REL_PKG):\n"
                    "        rel_map[rel.get('Id')] = rel.get('Target', '')\n"
                    "    first_sheet = wb.find('.//{%s}sheets/{%s}sheet' % (NS_MAIN, NS_MAIN))\n"
                    "    if first_sheet is None:\n"
                    "        raise RuntimeError('No worksheet found in workbook.')\n"
                    "    rid = first_sheet.get('{%s}id' % NS_REL_DOC)\n"
                    "    target = rel_map.get(rid, '')\n"
                    "    if not target:\n"
                    "        raise RuntimeError('Cannot resolve first worksheet relationship.')\n"
                    "    if target.startswith('/'):\n"
                    "        sheet_path = target.lstrip('/')\n"
                    "    elif target.startswith('xl/'):\n"
                    "        sheet_path = target\n"
                    "    else:\n"
                    "        sheet_path = 'xl/' + target\n"
                    "    sheet = ET.fromstring(z.read(sheet_path))\n"
                    "    with open(dst, 'w', newline='', encoding='utf-8') as f:\n"
                    "        writer = csv.writer(f)\n"
                    "        for row in sheet.findall('.//{%s}sheetData/{%s}row' % (NS_MAIN, NS_MAIN)):\n"
                    "            data = {}\n"
                    "            max_col = -1\n"
                    "            for cell in row.findall('{%s}c' % NS_MAIN):\n"
                    "                ref = cell.get('r', '')\n"
                    "                col = col_to_idx(ref)\n"
                    "                max_col = max(max_col, col)\n"
                    "                ctype = cell.get('t', '')\n"
                    "                value = ''\n"
                    "                if ctype == 'inlineStr':\n"
                    "                    is_elem = cell.find('{%s}is' % NS_MAIN)\n"
                    "                    if is_elem is not None:\n"
                    "                        value = ''.join(t.text or '' for t in is_elem.findall('.//{%s}t' % NS_MAIN))\n"
                    "                else:\n"
                    "                    v = cell.find('{%s}v' % NS_MAIN)\n"
                    "                    raw = v.text if v is not None and v.text is not None else ''\n"
                    "                    if ctype == 's':\n"
                    "                        try:\n"
                    "                            value = shared[int(raw)]\n"
                    "                        except Exception:\n"
                    "                            value = ''\n"
                    "                    else:\n"
                    "                        value = raw\n"
                    "                data[col] = value\n"
                    "            if max_col < 0:\n"
                    "                continue\n"
                    "            out = [data.get(i, '') for i in range(max_col + 1)]\n"
                    "            while out and out[-1] == '':\n"
                    "                out.pop()\n"
                    "            writer.writerow(out)\n";

                QProcess py;
                py.start("python", QStringList() << "-c" << pyScript << sourcePath << tempPath);
                bool finished = py.waitForFinished(120000);
                bool ok = finished && py.exitStatus() == QProcess::NormalExit && py.exitCode() == 0;

                if (!ok) {
                    QProcess pyLauncher;
                    pyLauncher.start("py", QStringList() << "-3" << "-c" << pyScript << sourcePath << tempPath);
                    finished = pyLauncher.waitForFinished(120000);
                    ok = finished && pyLauncher.exitStatus() == QProcess::NormalExit && pyLauncher.exitCode() == 0;

                    if (!ok && outError) {
                        *outError = QString::fromLocal8Bit(py.readAllStandardError()).trimmed();
                        if (outError->isEmpty()) {
                            *outError = QString::fromLocal8Bit(pyLauncher.readAllStandardError()).trimmed();
                        }
                        if (outError->isEmpty()) {
                            *outError = "Python-based .xlsx conversion failed.";
                        }
                    }
                }

                return ok;
            };

            QString importError;
            bool converted = false;
            if (isXlsxExtension) {
                QStringList conversionErrors;
                QString stepError;

                converted = runOpenXmlPowerShellConversion(&stepError);
                if (!converted && !stepError.isEmpty()) {
                    conversionErrors << ("OpenXML parser: " + stepError);
                }
                if (!converted) {
                    stepError.clear();
                    converted = runPythonXlsxConversion(&stepError);
                    if (!converted && !stepError.isEmpty()) {
                        conversionErrors << ("Python parser: " + stepError);
                    }
                }

                if (!converted) {
                    importError = conversionErrors.join("\n\n");
                    if (importError.isEmpty()) {
                        importError = "Unable to read .xlsx workbook. Install Python 3 or use CSV import.";
                    }
                }
            } else {
                converted = runExcelComConversion(&importError);
            }

            if (!converted) {
                QMessageBox::critical(this, "Excel Import Error",
                    "Failed to import workbook.\n\n" +
                    (importError.isEmpty()
                        ? "For .xlsx files, install Python 3 (or Microsoft Excel). For .xls/.xlsm/.xlsb files, Microsoft Excel is required."
                        : importError));
                return;
            }

            importPath = tempCsvPath;
        }

        QFile file(importPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "File Error", "Could not open the file for reading.");
            return;
        }
        
        QTextStream in(&file);
        int lineNumber = 0;
        int successCount = 0;
        QStringList errorMessages;
        QList<QVariantList> validRows;
        
        bool isFirstLine = true;
        
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            line.remove('\r');
            lineNumber++;

            if (isFirstLine && !line.isEmpty() && line.front() == QChar(0xFEFF)) {
                line.remove(0, 1);
            }
            
            if (line.isEmpty()) continue;

            // Excel CSV separator hint (for example: "sep=;")
            if (isFirstLine && line.startsWith("sep=", Qt::CaseInsensitive)) {
                continue;
            }

            QChar delimiter = ',';
            if (line.contains(';')) {
                delimiter = ';';
            } else if (line.contains('\t')) {
                delimiter = '\t';
            }

            QStringList parts = line.split(delimiter);
            
            // Smart header detection: skip any first line that looks like a header
            if (isFirstLine && parts.size() > 0 &&
                (parts[0].contains("Type", Qt::CaseInsensitive) ||
                 parts[0].contains("Quantity", Qt::CaseInsensitive) ||
                 parts[0].contains("Price", Qt::CaseInsensitive) ||
                 parts[0].contains("Buyer", Qt::CaseInsensitive))) {
                isFirstLine = false;
                continue;
            }
            isFirstLine = false;
            
            // Must have exactly 4 fields
            if (parts.size() < 4) {
                errorMessages << QString("Line %1: Missing fields. Expected 4 columns: Type, Quantity, Price, BuyerID.").arg(lineNumber);
                continue;
            }
            if (parts.size() > 4) {
                errorMessages << QString("Line %1: Too many fields (%2). Expected exactly 4 columns: Type, Quantity, Price, BuyerID.").arg(lineNumber).arg(parts.size());
                continue;
            }

            auto normalizeCsvField = [](QString value) {
                value = value.trimmed();
                if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"')) {
                    value = value.mid(1, value.size() - 2);
                }
                value.replace("\"\"", "\"");
                return value.trimmed();
            };
            
            QString typeStr   = normalizeCsvField(parts[0]);
            QString qtyStr    = normalizeCsvField(parts[1]);
            QString priceStr  = normalizeCsvField(parts[2]);
            QString buyerStr  = normalizeCsvField(parts[3]);
            
            QString lineErrors;
            
            // --- Smart Validation 1: Type (must match allowed types) ---
            QString cleanType;
            for (QChar c : typeStr) {
                if (c.isLetter() || c.isSpace()) cleanType += c;
            }
            cleanType = cleanType.trimmed();
            static const QStringList allowedTypes = {"Chair", "Table", "Cabinet", "Wardrobe", "Other"};
            if (cleanType.isEmpty()) {
                lineErrors += "  - Type could not be read (must contain letters).\n";
            } else if (!allowedTypes.contains(cleanType, Qt::CaseInsensitive)) {
                lineErrors += QString("  - Type '%1' is not allowed. Must be one of: Chair, Table, Cabinet, Wardrobe, Other.\n").arg(cleanType);
            }
            
            // --- Smart Validation 2: Quantity (extract digits, ignore 'units'/spaces) ---
            QString cleanQtyStr;
            for (QChar c : qtyStr) {
                if (c.isDigit()) cleanQtyStr += c;
            }
            bool qtyOk;
            int qty = cleanQtyStr.toInt(&qtyOk);
            if (!qtyOk || qty <= 0) {
                lineErrors += "  - Quantity must be a valid positive number.\n";
            }
            
            // --- Smart Validation 3: Price (extract digits + one decimal, ignore $, DT, spaces) ---
            QString cleanPriceStr;
            bool decimalFound = false;
            for (QChar c : priceStr) {
                if (c.isDigit()) {
                    cleanPriceStr += c;
                } else if ((c == '.' || c == ',') && !decimalFound) {
                    cleanPriceStr += '.';
                    decimalFound = true;
                }
            }
            bool priceOk;
            double price = cleanPriceStr.toDouble(&priceOk);
            if (!priceOk || price <= 0.0) {
                lineErrors += "  - Price must be a valid positive number.\n";
            }
            
            // --- Smart Validation 4: BuyerID (must be positive int & exist in DB) ---
            bool buyerOk;
            int buyerId = buyerStr.toInt(&buyerOk);
            if (!buyerOk || buyerId <= 0) {
                lineErrors += "  - Buyer ID must be a positive whole number.\n";
            } else {
                QSqlQuery checkClient;
                checkClient.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
                checkClient.bindValue(":id", buyerId);
                if (checkClient.exec() && checkClient.next() && checkClient.value(0).toInt() == 0) {
                    lineErrors += QString("  - Buyer ID %1 does not exist in the database.\n").arg(buyerId);
                }
                checkClient.finish();
            }
            
            if (!lineErrors.isEmpty()) {
                errorMessages << QString("Line %1 ('%2'):\n%3").arg(lineNumber).arg(line).arg(lineErrors);
            } else {
                validRows.append(QVariantList{cleanType, qty, price, buyerId});
            }
        }
        file.close();
        
        // Show errors and ask whether to continue with valid rows
        if (!errorMessages.isEmpty()) {
            QString errorSummary = QString("Found %1 error(s) in the file:\n\n").arg(errorMessages.size());
            int displayLimit = qMin(10, (int)errorMessages.size());
            for (int i = 0; i < displayLimit; ++i)
                errorSummary += errorMessages[i] + "\n";
            if (errorMessages.size() > 10)
                errorSummary += "... and more.\n\n";
            
            if (validRows.isEmpty()) {
                QMessageBox::warning(this, "Import Failed", "No valid rows found to import.\n\n" + errorSummary);
                return;
            } else {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "Import Encountered Errors",
                    errorSummary + QString("\nDo you want to skip the errors and import the %1 valid order(s)?").arg(validRows.size()),
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::No) return;
            }
        } else {
            if (validRows.isEmpty()) {
                QMessageBox::information(this, "Import", "The file was empty or contained only a header.");
                return;
            }
        }
        
        // Get next available order ID
        int baseOrderId = 1;
        {
            QSqlQuery maxQuery;
            if (maxQuery.exec("SELECT MAX(order_id) FROM ORDERS")) {
                if (maxQuery.next() && !maxQuery.value(0).isNull())
                    baseOrderId = maxQuery.value(0).toInt() + 1;
            } else {
                QMessageBox::critical(this, "Database Error", "Failed to retrieve the next Order ID:\n" + maxQuery.lastError().text());
                return;
            }
            maxQuery.finish();
        }
        
        // Insert valid rows
        for (const QVariantList &row : validRows) {
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO ORDERS (order_id, client_id, employee_id, order_type, total_quantity, total_price, order_date, order_status, payment_status) "
                                "VALUES (:id, :buyer, :employee, :type, :quantity, :price, SYSDATE, 'Pending', 'Unpaid')");
            insertQuery.bindValue(":id", baseOrderId);
            insertQuery.bindValue(":buyer", row[3]);
            insertQuery.bindValue(":employee", currentEmployeeId);
            insertQuery.bindValue(":type", row[0]);
            insertQuery.bindValue(":quantity", row[1]);
            insertQuery.bindValue(":price", row[2]);
            
            if (insertQuery.exec()) {
                successCount++;
                baseOrderId++;
            } else {
                QMessageBox::critical(this, "Database Error",
                    "Failed to insert row " + QString::number(successCount + 1) + ":\n" + insertQuery.lastError().text());
            }
        }
        
        QMessageBox::information(this, "Import Complete", QString("Successfully imported %1 order(s)!").arg(successCount));
        onOrderRefreshCatalog();
    }
}

void MainWindow::onOrderPrintCatalog()
{
    if (!ui_order) return;
    
    // Get the order ID from search bar
    QString searchId = ui_order->le_catalog_search->text().trimmed();
    
    if (searchId.isEmpty()) {
        QMessageBox::warning(this, tr("No Order Selected"), 
            tr("Please enter an Order ID in the search box to print."));
        return;
    }
    
    // Validate it's a number
    bool ok;
    int orderId = searchId.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Invalid Order ID"), 
            tr("Please enter a valid Order ID number."));
        return;
    }
    
    // Query the specific order
    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderId);
    
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Order Not Found"), 
            tr("Order ID %1 does not exist in the database.").arg(searchId));
        return;
    }
    
    // Extract order data
    QString orderType = query.value(1).toString();
    int quantity = query.value(2).toInt();
    double price = query.value(3).toDouble();
    int clientId = query.value(4).toInt();
    QString orderDate = query.value(5).toDateTime().toString("MMMM dd, yyyy");
    QString orderStatus = query.value(6).toString();
    QString paymentStatus = query.value(7).toString();
    QString clientFirstName = query.value(8).toString();
    QString clientLastName = query.value(9).toString();
    QString clientEmail = query.value(10).toString();
    QString clientPhone = query.value(11).toString();
    
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageSize(QPageSize(QPageSize::A4));
    
    QPrintDialog printDialog(&printer, this);
    
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter;
        if (!painter.begin(&printer)) {
            QMessageBox::critical(this, "Print Error", "Failed to print.");
            return;
        }
        
        // Page dimensions and margins
        int pageWidth = printer.width();
        int pageHeight = printer.height();
        int margin = 80;  // 1+ inch margins for professional look
        int contentWidth = pageWidth - 2 * margin;
        int y = margin;
        
        // ==================== HEADER SECTION ====================
        // System title bar with background
        painter.fillRect(0, 0, pageWidth, 100, QColor(45, 45, 45));
        
        // System title
        QFont titleFont("Segoe UI", 22, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(Qt::white);
        painter.drawText(margin, 35, "Order Management System");
        
        // Subtitle
        QFont subtitleFont("Segoe UI", 10);
        painter.setFont(subtitleFont);
        painter.setPen(QColor(220, 220, 220));
        painter.drawText(margin, 60, "Professional Order Processing & Invoice Generation");
        
        y = 130;
        
        // ==================== COMPANY & ORDER INFO ====================
        // Company logo and info (left side)
        int logoSize = 70;
        QPixmap logo(":/assets/logo.png");
        if (!logo.isNull()) {
            QPixmap scaledLogo = logo.scaled(logoSize, logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(margin, y, scaledLogo);
        }
        
        QFont companyFont("Segoe UI", 18, QFont::Bold);
        painter.setFont(companyFont);
        painter.setPen(QColor(139, 111, 71));
        painter.drawText(margin + logoSize + 20, y + 25, "Hammer Down");
        
        QFont companySubFont("Segoe UI", 9);
        painter.setFont(companySubFont);
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(margin + logoSize + 20, y + 45, "Business Solutions Provider");
        
        // Order number and date (right side)
        QFont orderNumFont("Segoe UI", 20, QFont::Bold);
        painter.setFont(orderNumFont);
        painter.setPen(QColor(45, 45, 45));
        QString orderText = "ORDER #" + searchId;
        QFontMetrics fm1(orderNumFont);
        int orderWidth = fm1.horizontalAdvance(orderText);
        painter.drawText(pageWidth - margin - orderWidth, y + 25, orderText);
        
        QFont dateFont("Segoe UI", 9);
        painter.setFont(dateFont);
        painter.setPen(QColor(100, 100, 100));
        QString dateGenerated = "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy hh:mm AP");
        QFontMetrics fm2(dateFont);
        int dateWidth = fm2.horizontalAdvance(dateGenerated);
        painter.drawText(pageWidth - margin - dateWidth, y + 50, dateGenerated);
        
        y += 100;
        
        // Horizontal separator
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawLine(margin, y, pageWidth - margin, y);
        
        y += 40;
        
        // ==================== ORDER DETAILS SECTION ====================
        // Section header
        QFont sectionHeaderFont("Segoe UI", 14, QFont::Bold);
        painter.setFont(sectionHeaderFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin, y, "Order Details");
        
        y += 10;
        
        // Section underline
        painter.setPen(QPen(QColor(139, 111, 71), 3));
        painter.drawLine(margin, y, margin + 120, y);
        
        y += 30;
        
        // Order details box with light background
        int detailsBoxHeight = 180;
        painter.fillRect(margin, y, contentWidth, detailsBoxHeight, QColor(250, 250, 252));
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.drawRect(margin, y, contentWidth, detailsBoxHeight);
        
        y += 30;
        
        // Two-column layout for order info
        QFont labelFont("Segoe UI", 10, QFont::Bold);
        QFont valueFont("Segoe UI", 10);
        int labelCol = margin + 30;
        int valueCol = margin + 200;
        int rowHeight = 28;
        
        auto drawDetailRow = [&](const QString& label, const QString& value) {
            painter.setFont(labelFont);
            painter.setPen(QColor(90, 90, 90));
            painter.drawText(labelCol, y, label);
            
            painter.setFont(valueFont);
            painter.setPen(QColor(40, 40, 40));
            painter.drawText(valueCol, y, value);
            
            y += rowHeight;
        };
        
        drawDetailRow("Order Date:", orderDate);
        drawDetailRow("Order Type:", orderType);
        drawDetailRow("Quantity:", QString::number(quantity) + " units");
        drawDetailRow("Unit Price:", "$" + QString::number(price, 'f', 2));
        drawDetailRow("Order Status:", orderStatus);
        drawDetailRow("Payment Status:", paymentStatus);
        
        y += 30;
        
        // ==================== CLIENT INFORMATION SECTION ====================
        // Section header
        painter.setFont(sectionHeaderFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin, y, "Client Information");
        
        y += 10;
        
        // Section underline
        painter.setPen(QPen(QColor(139, 111, 71), 3));
        painter.drawLine(margin, y, margin + 140, y);
        
        y += 30;
        
        // Client details box
        int clientBoxHeight = 140;
        painter.fillRect(margin, y, contentWidth, clientBoxHeight, QColor(250, 250, 252));
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.drawRect(margin, y, contentWidth, clientBoxHeight);
        
        y += 30;
        
        drawDetailRow("Client ID:", QString::number(clientId));
        drawDetailRow("Full Name:", clientFirstName + " " + clientLastName);
        drawDetailRow("Email Address:", clientEmail.isEmpty() ? "Not provided" : clientEmail);
        drawDetailRow("Phone Number:", clientPhone.isEmpty() ? "Not provided" : clientPhone);
        
        y += 40;
        
        // ==================== PAYMENT SUMMARY ====================
        // Summary section with accent color
        int summaryBoxHeight = 100;
        painter.fillRect(margin, y, contentWidth, summaryBoxHeight, QColor(139, 111, 71));
        
        // Inner white box for amount
        int innerMargin = 3;
        painter.fillRect(margin + innerMargin, y + innerMargin, 
                         contentWidth - 2 * innerMargin, summaryBoxHeight - 2 * innerMargin, 
                         QColor(255, 255, 255));
        
        y += 40;
        
        // Total amount label
        QFont summaryLabelFont("Segoe UI", 16, QFont::Bold);
        painter.setFont(summaryLabelFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin + 30, y, "TOTAL AMOUNT");
        
        // Total amount value (right aligned)
        QFont totalAmountFont("Segoe UI", 24, QFont::Bold);
        painter.setFont(totalAmountFont);
        painter.setPen(QColor(139, 111, 71));
        double totalPrice = price * quantity;
        QString totalText = "$" + QString::number(totalPrice, 'f', 2);
        QFontMetrics fm3(totalAmountFont);
        int totalWidth = fm3.horizontalAdvance(totalText);
        painter.drawText(pageWidth - margin - totalWidth - 30, y + 5, totalText);
        
        y += 35;
        
        // Payment status in summary
        QFont statusFont("Segoe UI", 10);
        painter.setFont(statusFont);
        painter.setPen(QColor(100, 100, 100));
        QString statusText = "Payment Status: " + paymentStatus;
        painter.drawText(margin + 30, y, statusText);
        
        // ==================== QR CODE ====================
        y += 80;  // Move below the summary box
        
        // Build QR content with same info as PDF
        QString qrContent = buildOrderQrContent(orderId, orderType, quantity, price,
                                                 orderDate, orderStatus, paymentStatus,
                                                 clientId, clientFirstName + " " + clientLastName,
                                                 clientEmail, clientPhone);
        QPixmap qrPixmap = generateQrPixmap(qrContent, 4, 2);
        
        // QR section label
        painter.setFont(sectionHeaderFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin, y, "Scan QR Code");
        y += 10;
        painter.setPen(QPen(QColor(139, 111, 71), 3));
        painter.drawLine(margin, y, margin + 120, y);
        y += 20;
        
        // Draw QR code
        int qrDisplaySize = 140;
        QPixmap scaledQr = qrPixmap.scaled(qrDisplaySize, qrDisplaySize, Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawPixmap(margin, y, scaledQr);
        
        // QR description text next to QR code
        painter.setFont(QFont("Segoe UI", 9));
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(margin + qrDisplaySize + 20, y + 30, "Scan this QR code to view");
        painter.drawText(margin + qrDisplaySize + 20, y + 50, "complete order details.");
        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor(140, 140, 140));
        painter.drawText(margin + qrDisplaySize + 20, y + 80, "Contains: Order info, client data,");
        painter.drawText(margin + qrDisplaySize + 20, y + 95, "pricing and payment status.");
        
        // ==================== FOOTER ======================================
        y = pageHeight - 50;
        
        // Footer separator line
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawLine(margin, y, pageWidth - margin, y);
        
        y += 25;
        
        // Centered footer text
        QFont footerFont("Segoe UI", 8);
        painter.setFont(footerFont);
        painter.setPen(QColor(120, 120, 120));
        
        QString footerText = "Thank you for your business! | Hammer Down © 2026 - All Rights Reserved | For inquiries, contact support@hammerdown.com";
        QFontMetrics fmFooter(footerFont);
        int footerWidth = fmFooter.horizontalAdvance(footerText);
        int footerX = (pageWidth - footerWidth) / 2;
        painter.drawText(footerX, y, footerText);
        
        painter.end();
        QMessageBox::information(this, "Success", "Order #" + searchId + " printed successfully!");
    }
}

// ==================== QR Code Tab Functions ====================
void MainWindow::onGenerateQR()
{
    if (!ui_order) return;

    QString searchId = ui_order->le_qr_order_id->text().trimmed();

    if (searchId.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter an Order ID.");
        return;
    }

    bool ok;
    int orderId = searchId.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid ID", "Please enter a valid numeric Order ID.");
        return;
    }

    // Query the order with client info
    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderId);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Not Found",
            "Order #" + searchId + " does not exist in the database.");
        return;
    }

    QString orderType = query.value(1).toString();
    int quantity = query.value(2).toInt();
    double price = query.value(3).toDouble();
    int clientId = query.value(4).toInt();
    QString orderDate = query.value(5).toDateTime().toString("MMM dd, yyyy");
    QString orderStatus = query.value(6).toString();
    QString paymentStatus = query.value(7).toString();
    QString clientName = query.value(8).toString() + " " + query.value(9).toString();
    QString clientEmail = query.value(10).toString();
    QString clientPhone = query.value(11).toString();

    QString qrContent = buildOrderQrContent(orderId, orderType, quantity, price,
                                             orderDate, orderStatus, paymentStatus,
                                             clientId, clientName, clientEmail, clientPhone);

    QPixmap qrPixmap = generateQrPixmap(qrContent, 8, 4);

    // Display in the label (scale to fit the 300x300 display area)
    ui_order->label_qr_display->setPixmap(
        qrPixmap.scaled(280, 280, Qt::KeepAspectRatio, Qt::FastTransformation));

    QMessageBox::information(this, "QR Generated",
        "QR Code for Order #" + searchId + " has been generated!\n\n"
        "Scan the QR code to view order details.");
}

void MainWindow::onSaveQR()
{
    if (!ui_order) return;

    QPixmap currentQr = ui_order->label_qr_display->pixmap();
    if (currentQr.isNull()) {
        QMessageBox::warning(this, "No QR Code", "Please generate a QR code first.");
        return;
    }

    QString orderId = ui_order->le_qr_order_id->text().trimmed();
    QString fileName = QFileDialog::getSaveFileName(this, "Save QR Code",
        QDir::homePath() + "/QR_Order_" + orderId + ".png",
        "PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*)");

    if (fileName.isEmpty()) return;

    // Re-generate at high resolution for saving
    // Get the content again
    bool ok;
    int orderIdInt = orderId.toInt(&ok);
    if (!ok) {
        // If we can't parse the ID, just save the displayed pixmap
        currentQr.save(fileName);
        QMessageBox::information(this, "Saved", "QR Code saved to:\n" + fileName);
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderIdInt);

    if (query.exec() && query.next()) {
        QString qrContent = buildOrderQrContent(
            orderIdInt,
            query.value(1).toString(),
            query.value(2).toInt(),
            query.value(3).toDouble(),
            query.value(5).toDateTime().toString("MMM dd, yyyy"),
            query.value(6).toString(),
            query.value(7).toString(),
            query.value(4).toInt(),
            query.value(8).toString() + " " + query.value(9).toString(),
            query.value(10).toString(),
            query.value(11).toString());

        QPixmap highResQr = generateQrPixmap(qrContent, 16, 4);  // Higher resolution for file
        highResQr.save(fileName);
    } else {
        currentQr.save(fileName);
    }

    QMessageBox::information(this, "Saved", "QR Code saved to:\n" + fileName);
}

void MainWindow::onPrintQR()
{
    if (!ui_order) return;

    QPixmap currentQr = ui_order->label_qr_display->pixmap();
    if (currentQr.isNull()) {
        QMessageBox::warning(this, "No QR Code", "Please generate a QR code first.");
        return;
    }

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter;
        if (!painter.begin(&printer)) {
            QMessageBox::critical(this, "Print Error", "Failed to start printing.");
            return;
        }

        int pageWidth = printer.width();
        int pageHeight = printer.height();

        // Re-generate high resolution QR
        QString orderId = ui_order->le_qr_order_id->text().trimmed();
        QPixmap qrToPrint = currentQr;

        bool ok;
        int orderIdInt = orderId.toInt(&ok);
        if (ok) {
            QSqlQuery query;
            query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                          "o.client_id, o.order_date, o.order_status, o.payment_status, "
                          "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                          "FROM ORDERS o "
                          "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                          "WHERE o.order_id = :id");
            query.bindValue(":id", orderIdInt);
            if (query.exec() && query.next()) {
                QString qrContent = buildOrderQrContent(
                    orderIdInt,
                    query.value(1).toString(),
                    query.value(2).toInt(),
                    query.value(3).toDouble(),
                    query.value(5).toDateTime().toString("MMM dd, yyyy"),
                    query.value(6).toString(),
                    query.value(7).toString(),
                    query.value(4).toInt(),
                    query.value(8).toString() + " " + query.value(9).toString(),
                    query.value(10).toString(),
                    query.value(11).toString());
                qrToPrint = generateQrPixmap(qrContent, 12, 4);
            }
        }

        // Print layout: title, then centered QR code, then order ID below
        int y = 60;

        // Title
        QFont titleFont("Segoe UI", 18, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(QColor(45, 45, 45));
        QString title = "Order #" + orderId + " - QR Code";
        QFontMetrics fm(titleFont);
        int titleW = fm.horizontalAdvance(title);
        painter.drawText((pageWidth - titleW) / 2, y, title);

        y += 40;

        // Subtitle
        QFont subFont("Segoe UI", 10);
        painter.setFont(subFont);
        painter.setPen(QColor(100, 100, 100));
        QString sub = "Scan this QR code to view full order details";
        QFontMetrics fm2(subFont);
        painter.drawText((pageWidth - fm2.horizontalAdvance(sub)) / 2, y, sub);

        y += 40;

        // Center QR code on page
        int qrDisplaySize = std::min(pageWidth - 160, 400);
        QPixmap scaledQr = qrToPrint.scaled(qrDisplaySize, qrDisplaySize, Qt::KeepAspectRatio, Qt::FastTransformation);
        int qrX = (pageWidth - scaledQr.width()) / 2;
        painter.drawPixmap(qrX, y, scaledQr);

        y += scaledQr.height() + 30;

        // Order ID text below QR
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.setPen(QColor(139, 111, 71));
        QString label = "ORDER #" + orderId;
        QFontMetrics fm3(QFont("Segoe UI", 12, QFont::Bold));
        painter.drawText((pageWidth - fm3.horizontalAdvance(label)) / 2, y, label);

        y += 30;

        // Generated date
        painter.setFont(QFont("Segoe UI", 9));
        painter.setPen(QColor(120, 120, 120));
        QString genDate = "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy hh:mm AP");
        QFontMetrics fm4(QFont("Segoe UI", 9));
        painter.drawText((pageWidth - fm4.horizontalAdvance(genDate)) / 2, y, genDate);

        // Footer
        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor(140, 140, 140));
        QString footer = "Hammer Down - Order Management System";
        QFontMetrics fm5(QFont("Segoe UI", 8));
        painter.drawText((pageWidth - fm5.horizontalAdvance(footer)) / 2, pageHeight - 40, footer);

        QMessageBox::information(this, "Printed", "QR Code for Order #" + orderId + " printed successfully!");
    }
}


void MainWindow::onClientClearFields()
{
    if (ui_client) {
        ui_client->le_nom->clear();
        ui_client->le_prenom->clear();
        ui_client->le_adresse->clear();
        ui_client->le_tel->clear();
        ui_client->le_email->clear();
        // Reset radio buttons
        ui_client->rb_homme->setAutoExclusive(false);
        ui_client->rb_femme->setAutoExclusive(false);
        ui_client->rb_homme->setChecked(false);
        ui_client->rb_femme->setChecked(false);
        ui_client->rb_homme->setAutoExclusive(true);
        ui_client->rb_femme->setAutoExclusive(true);
    }
}

void MainWindow::onClientModClearFields()
{
    if (ui_client) {
        ui_client->le_id_mod->clear();
        ui_client->le_nom_mod->clear();
        ui_client->le_prenom_mod->clear();
        ui_client->le_adresse_mod->clear();
        ui_client->le_tel_mod->clear();
        ui_client->le_email_mod->clear();
        // Reset radio buttons
        ui_client->rb_homme_mod->setAutoExclusive(false);
        ui_client->rb_femme_mod->setAutoExclusive(false);
        ui_client->rb_homme_mod->setChecked(false);
        ui_client->rb_femme_mod->setChecked(false);
        ui_client->rb_homme_mod->setAutoExclusive(true);
        ui_client->rb_femme_mod->setAutoExclusive(true);
    }
}


void MainWindow::onSupplierClearFields()
{
    if (ui_supplier) {
        ui_supplier->le_id->clear();
        ui_supplier->le_nom->clear();
        ui_supplier->le_adresse->clear();
        ui_supplier->le_email->clear();
        ui_supplier->le_tel->clear();
        ui_supplier->le_product_type->clear();
        ui_supplier->le_type->clear();
        ui_supplier->sb_cp->setValue(0);
        ui_supplier->txt_sms->clear();
        if (m_teOpeningHour)  m_teOpeningHour->setTime(QTime(8, 0));
        if (m_teClosingHour) m_teClosingHour->setTime(QTime(18, 0));
    }
}

// ==================== Supplier Management ====================

void MainWindow::onSupplierRefreshView()
{
    if (!ui_supplier) return;

    // Populate the tableView in the View tab
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", SUPPLIER_ID AS \"ID\", "
        "SUPPLIER_NAME AS \"Company\", ADDRESS AS \"Address\","
        " EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", TYPE_NOTIFICATION AS \"Type\","
        " POSTAL_CODE AS \"Postal Code\""
        " FROM SUPPLIERS ORDER BY SUPPLIER_ID"
    );

    ui_supplier->tableView->setModel(model);
    ui_supplier->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_supplier->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_supplier->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Re-populate the Reviews combo box with supplier names
    ui_supplier->cb_supplier_reviews->clear();
    ui_supplier->cb_supplier_reviews->addItem("-- Select Supplier --", -1);
    QSqlQuery qRev;
    qRev.prepare("SELECT SUPPLIER_ID, SUPPLIER_NAME FROM SUPPLIERS ORDER BY SUPPLIER_NAME");
    if (qRev.exec()) {
        while (qRev.next()) {
            QString name = qRev.value(1).toString();
            int sid = qRev.value(0).toInt();
            ui_supplier->cb_supplier_reviews->addItem(name.isEmpty() ? QString("Supplier #%1").arg(sid) : name, sid);
        }
    }
}

void MainWindow::onSupplierAdd()
{
    if (!ui_supplier) return;

    QString id    = ui_supplier->le_id->text().trimmed();
    QString nom   = ui_supplier->le_nom->text().trimmed();
    QString addr  = ui_supplier->le_adresse->text().trimmed();
    QString email = ui_supplier->le_email->text().trimmed();
    QString tel   = ui_supplier->le_tel->text().trimmed();
    QString type  = ui_supplier->le_type->text().trimmed();
    QString openTime  = m_teOpeningHour  ? m_teOpeningHour->time().toString("HH:mm")  : "";
    QString closeTime = m_teClosingHour ? m_teClosingHour->time().toString("HH:mm") : "";
    int cp        = ui_supplier->sb_cp->value();

    if (id.isEmpty() || nom.isEmpty() || addr.isEmpty() || email.isEmpty() || tel.isEmpty() || type.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required!");
        return;
    }

    if (!email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid email address (must contain @ and .)");
        return;
    }

    bool phoneOk;
    tel.toLongLong(&phoneOk);
    if (!phoneOk || tel.length() < 8) {
        QMessageBox::warning(this, "Input Error", "Phone number must be at least 8 digits and contain only numbers!");
        return;
    }

    bool idOk;
    int suppId = id.toInt(&idOk);
    if (!idOk) {
        QMessageBox::warning(this, "Input Error", "Supplier ID must be a valid integer.");
        return;
    }

    // Check duplicate
    QSqlQuery chk;
    chk.prepare("SELECT COUNT(*) FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    chk.bindValue(":id", suppId);
    if (chk.exec() && chk.next() && chk.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Duplicate", 
            QString("Supplier ID %1 already exists!").arg(suppId));
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Add",
        QString("Add supplier:\n\nID: %1\nCompany: %2\nEmail: %3\nPhone: %4")
            .arg(suppId).arg(nom).arg(email).arg(tel),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlQuery q;
    q.prepare("INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, ADDRESS, EMAIL, PHONE_NUMBER,"
              " TYPE_NOTIFICATION, POSTAL_CODE, REGISTRATION_DATE, ACCOUNT_STATUS, OPENING_TIME, CLOSING_TIME)"
              " VALUES (:id, :nom, :addr, :email, :tel, :type, :cp, SYSDATE, 'Active', :openTime, :closeTime)");
    q.bindValue(":id",        suppId);
    q.bindValue(":nom",       nom);
    q.bindValue(":addr",      addr);
    q.bindValue(":email",     email);
    q.bindValue(":tel",       tel);
    q.bindValue(":type",      type);
    q.bindValue(":cp",        cp);
    q.bindValue(":openTime",  openTime);
    q.bindValue(":closeTime", closeTime);

    if (q.exec()) {
        if (homeWindow && homeWindow->isAnimationMode()) {
            playSupplierSuccessAnimation(nom);
        } else {
            QMessageBox::information(this, "Success",
                QString("Supplier '%1' (ID: %2) added successfully!").arg(nom).arg(suppId));
        }
        checkSupplierVicinity(suppId);
        onSupplierClearFields();
        onSupplierRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error",
            "Failed to add supplier.\n\n" + q.lastError().databaseText());
    }
}

void MainWindow::onSupplierModify()
{
    if (!ui_supplier) return;

    QString id    = ui_supplier->le_id->text().trimmed();
    QString nom   = ui_supplier->le_nom->text().trimmed();
    QString addr  = ui_supplier->le_adresse->text().trimmed();
    QString email = ui_supplier->le_email->text().trimmed();
    QString tel   = ui_supplier->le_tel->text().trimmed();
    QString type  = ui_supplier->le_type->text().trimmed();
    QString openTime  = m_teOpeningHour  ? m_teOpeningHour->time().toString("HH:mm")  : "";
    QString closeTime = m_teClosingHour ? m_teClosingHour->time().toString("HH:mm") : "";
    int cp        = ui_supplier->sb_cp->value();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error",
            "Please select a supplier from the View tab first,\nor enter a Supplier ID to modify.");
        return;
    }

    if (nom.isEmpty() || addr.isEmpty() || email.isEmpty() || tel.isEmpty() || type.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required to modify the supplier!");
        return;
    }

    if (!email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid email address (must contain @ and .)");
        return;
    }

    bool phoneOk;
    tel.toLongLong(&phoneOk);
    if (!phoneOk || tel.length() < 8) {
        QMessageBox::warning(this, "Input Error", "Phone number must be at least 8 digits and contain only numbers!");
        return;
    }

    bool idOk;
    int suppId = id.toInt(&idOk);
    if (!idOk) {
        QMessageBox::warning(this, "Input Error", "Supplier ID must be a valid integer.");
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Modify",
        QString("Update supplier ID %1 (%2)?").arg(suppId).arg(nom),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlQuery q;
    q.prepare("UPDATE SUPPLIERS SET SUPPLIER_NAME=:nom, ADDRESS=:addr, EMAIL=:email, PHONE_NUMBER=:tel,"
              " TYPE_NOTIFICATION=:type, POSTAL_CODE=:cp, OPENING_TIME=:openTime, CLOSING_TIME=:closeTime"
              " WHERE SUPPLIER_ID=:id");
    q.bindValue(":nom",       nom);
    q.bindValue(":addr",      addr);
    q.bindValue(":email",     email);
    q.bindValue(":tel",       tel);
    q.bindValue(":type",      type);
    q.bindValue(":cp",        cp);
    q.bindValue(":openTime",  openTime);
    q.bindValue(":closeTime", closeTime);
    q.bindValue(":id",        suppId);

    if (q.exec()) {
        if (q.numRowsAffected() > 0) {
            if (homeWindow && homeWindow->isAnimationMode()) {
                playSupplierModifyAnimation(nom);
            } else {
                QMessageBox::information(this, "Success",
                    QString("Supplier ID %1 updated successfully!").arg(suppId));
            }
            checkSupplierVicinity(suppId);
            onSupplierClearFields();
            onSupplierRefreshView();
        } else {
            QMessageBox::warning(this, "Not Found",
                QString("No supplier found with ID %1.").arg(suppId));
        }
    } else {
        QMessageBox::critical(this, "Database Error",
            "Failed to modify supplier.\n\n" + q.lastError().databaseText());
    }
}

void MainWindow::onSupplierDelete()
{
    if (!ui_supplier) return;

    QModelIndex idx = ui_supplier->tableView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select a supplier from the list to delete.");
        return;
    }

    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_supplier->tableView->model());
    if (!model) return;

    // Col order: Action, Delete, ID, Company, Address, Email, Phone, Type, PostalCode
    QString suppId = model->data(model->index(idx.row(), 2)).toString();
    QString nom    = model->data(model->index(idx.row(), 3)).toString();

    int reply = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to DELETE supplier:\n\nID: %1\nCompany: %2\n\nThis action cannot be undone!")
            .arg(suppId).arg(nom),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlQuery q;
    q.prepare("DELETE FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    q.bindValue(":id", suppId.toInt());

    if (q.exec()) {
        if (homeWindow && homeWindow->isAnimationMode()) {
            playSupplierDeleteAnimation(nom);
        } else {
            QMessageBox::information(this, "Success", "Supplier deleted successfully.");
        }
        onSupplierClearFields();
        onSupplierRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to delete supplier:\n" + q.lastError().text());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSupplierDeleteAll()
{
    if (!ui_supplier) return;

    // Count first so the warning is specific
    int count = 0;
    QSqlQuery qCount("SELECT COUNT(*) FROM SUPPLIERS");
    if (qCount.exec() && qCount.next()) count = qCount.value(0).toInt();

    if (count == 0) {
        QMessageBox::information(this, tr("Delete All"), tr("There are no suppliers to delete."));
        return;
    }

    auto reply = QMessageBox::warning(
        this,
        tr("Confirm Delete All"),
        tr("This will permanently delete ALL %1 supplier(s) and their ratings/notifications.\n\nThis action cannot be undone!").arg(count),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel
    );
    if (reply != QMessageBox::Yes) return;

    QSqlQuery qDel("DELETE FROM SUPPLIERS");
    if (qDel.exec()) {
        QMessageBox::information(this, tr("Deleted"),
            tr("%1 supplier(s) deleted successfully.").arg(count));
        onSupplierClearFields();
        onSupplierRefreshView();
        setupSupplierStats(); // refresh the stats tab too
    } else {
        QMessageBox::critical(this, tr("Database Error"),
            tr("Failed to delete suppliers:\n") + qDel.lastError().text());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSupplierExportPDF()
{
    if (!ui_supplier) return;

    // Gather data from the current tableView model
    const QAbstractItemModel *model = ui_supplier->tableView->model();
    if (!model || model->rowCount() == 0) {
        QMessageBox::information(this, tr("Export PDF"), tr("No supplier data to export."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Suppliers to PDF"),
        QDir::homePath() + "/Suppliers_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf)"
    );
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, tr("Export Error"), tr("Failed to create PDF file."));
        return;
    }

    const int W      = printer.width();
    const int margin = 60;
    const int cw     = W - 2 * margin;
    int y            = 0;

    // ── Header bar ────────────────────────────────────────────────────────────
    painter.fillRect(0, 0, W, 90, QColor(28, 22, 16));
    painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
    painter.setPen(QColor("#D4AF37"));
    painter.drawText(margin, 38, tr("Supplier Management System"));
    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(180, 160, 120));
    painter.drawText(margin, 60, tr("Professional Supplier Directory Export"));

    // Logo (right side of header)
    QPixmap logo(":/assets/logo.png");
    if (!logo.isNull())
        painter.drawPixmap(W - margin - 70, 10, logo.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Generated timestamp
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor("#8B6F47"));
    const QString stamp = tr("Generated: ") + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    QFontMetrics fm8(QFont("Segoe UI", 8));
    painter.drawText(W - margin - fm8.horizontalAdvance(stamp), 82, stamp);

    y = 110;

    // ── Summary strip ─────────────────────────────────────────────────────────
    int totalSuppliers = 0, activeCount = 0;
    double avgRat = 0.0;
    {
        QSqlQuery qs("SELECT COUNT(*), "
                     "SUM(CASE WHEN ACCOUNT_STATUS='Active' THEN 1 ELSE 0 END), "
                     "AVG(CASE WHEN AVERAGE_RATING>0 THEN AVERAGE_RATING END) "
                     "FROM SUPPLIERS");
        if (qs.exec() && qs.next()) {
            totalSuppliers = qs.value(0).toInt();
            activeCount    = qs.value(1).toInt();
            avgRat         = qs.value(2).isNull() ? 0.0 : qs.value(2).toDouble();
        }
    }
    painter.fillRect(margin, y, cw, 38, QColor(44, 34, 22));
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(QColor("#F5E6D3"));
    const QString summary = QString(tr("Total: %1   |   Active: %2   |   Inactive: %3   |   Avg Rating: %4 / 5.0"))
        .arg(totalSuppliers).arg(activeCount).arg(totalSuppliers - activeCount)
        .arg(QString::number(avgRat, 'f', 1));
    painter.drawText(margin + 12, y + 25, summary);
    y += 50;

    // ── Column definitions ────────────────────────────────────────────────────
    // Skip the first 2 model cols (Action / Delete icons), show cols 2-8
    struct Col { QString name; int widthPct; };
    const QList<Col> cols = {
        {tr("ID"),          6},
        {tr("Company"),    22},
        {tr("Address"),    22},
        {tr("Email"),      18},
        {tr("Phone"),      12},
        {tr("Type"),       12},
        {tr("Postal"),      8},
    };
    // Pre-compute pixel widths
    QList<int> colWidths;
    for (const Col &c : cols) colWidths << (cw * c.widthPct / 100);

    const int rowH    = 22;
    const int headerH = 28;

    auto drawRow = [&](int row, bool isHeader) {
        int x = margin;
        QColor bg  = isHeader ? QColor("#8B6F47") :
                     (row % 2 == 0 ? QColor(240, 232, 220) : QColor(255, 252, 245));
        QColor fg  = isHeader ? Qt::white : QColor(40, 30, 20);
        int    h   = isHeader ? headerH : rowH;
        painter.fillRect(x, y, cw, h, bg);
        painter.setPen(QPen(QColor(180, 150, 110), 0.5));
        painter.drawRect(x, y, cw, h);
        painter.setPen(fg);
        painter.setFont(QFont("Segoe UI", isHeader ? 9 : 8, isHeader ? QFont::Bold : QFont::Normal));
        for (int c = 0; c < cols.size(); ++c) {
            QString text = isHeader
                ? cols[c].name
                : model->data(model->index(row, c + 2)).toString(); // skip col 0,1
            QRect cell(x + 3, y + 2, colWidths[c] - 6, h - 4);
            painter.drawText(cell, Qt::AlignVCenter | Qt::AlignLeft,
                             painter.fontMetrics().elidedText(text, Qt::ElideRight, cell.width()));
            x += colWidths[c];
        }
    };

    // Draw table header
    drawRow(-1, true);
    y += headerH;

    // Draw data rows, paginating automatically
    for (int r = 0; r < model->rowCount(); ++r) {
        if (y + rowH > printer.height() - margin) {
            printer.newPage();
            y = margin;
            // Repeat header on each new page
            drawRow(-1, true);
            y += headerH;
        }
        drawRow(r, false);
        y += rowH;
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    y += 18;
    painter.setPen(QPen(QColor("#8B6F47"), 1));
    painter.drawLine(margin, y, W - margin, y);
    y += 12;
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor("#8B6F47"));
    painter.drawText(margin, y, tr("Hammer Down — Supplier Management System — Confidential"));
    painter.drawText(W - margin - 80, y, QString(tr("Total: %1 suppliers")).arg(totalSuppliers));

    painter.end();
    QMessageBox::information(this, tr("Export Successful"),
        tr("PDF exported successfully to:\n%1").arg(fileName));
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSupplierPrint()
{
    if (!ui_supplier) return;

    const QAbstractItemModel *model = ui_supplier->tableView->model();
    if (!model || model->rowCount() == 0) {
        QMessageBox::information(this, tr("Print"), tr("No supplier data to print."));
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Supplier List"));
    if (dialog.exec() != QDialog::Accepted) return;

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, tr("Print Error"), tr("Failed to start printing."));
        return;
    }

    const int W      = printer.width();
    const int margin = 120;
    const int cw     = W - 2 * margin;
    int y            = margin;

    // Header
    painter.fillRect(0, 0, W, 180, QColor(28, 22, 16));
    painter.setFont(QFont("Segoe UI", 28, QFont::Bold));
    painter.setPen(QColor("#D4AF37"));
    painter.drawText(margin, 90, tr("Supplier Directory"));
    painter.setFont(QFont("Segoe UI", 14));
    painter.setPen(QColor(180, 160, 120));
    painter.drawText(margin, 130, tr("Hammer Down — Supplier Management System"));
    painter.setFont(QFont("Segoe UI", 12));
    painter.setPen(QColor("#8B6F47"));
    painter.drawText(margin, 165, QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    y = 210;

    // Column setup
    struct Col { QString name; int widthPct; };
    const QList<Col> cols = {
        {tr("ID"),       6}, {tr("Company"), 22}, {tr("Address"), 22},
        {tr("Email"),   18}, {tr("Phone"),   12}, {tr("Type"),    12}, {tr("Postal"), 8}
    };
    QList<int> colWidths;
    for (const Col &c : cols) colWidths << (cw * c.widthPct / 100);

    const int rowH = 56, headerH = 70;

    auto drawRow = [&](int row, bool isHeader) {
        int x = margin;
        QColor bg = isHeader ? QColor("#8B6F47") :
                    (row % 2 == 0 ? QColor(240, 232, 220) : QColor(255, 252, 245));
        int h = isHeader ? headerH : rowH;
        painter.fillRect(x, y, cw, h, bg);
        painter.setPen(QPen(QColor(180, 150, 110), 1));
        painter.drawRect(x, y, cw, h);
        painter.setPen(isHeader ? Qt::white : QColor(40, 30, 20));
        painter.setFont(QFont("Segoe UI", isHeader ? 16 : 14,
                              isHeader ? QFont::Bold : QFont::Normal));
        for (int c = 0; c < cols.size(); ++c) {
            QString text = isHeader
                ? cols[c].name
                : model->data(model->index(row, c + 2)).toString();
            QRect cell(x + 8, y + 4, colWidths[c] - 16, h - 8);
            painter.drawText(cell, Qt::AlignVCenter | Qt::AlignLeft,
                             painter.fontMetrics().elidedText(text, Qt::ElideRight, cell.width()));
            x += colWidths[c];
        }
    };

    drawRow(-1, true);
    y += headerH;

    for (int r = 0; r < model->rowCount(); ++r) {
        if (y + rowH > printer.height() - margin) {
            printer.newPage();
            y = margin;
            drawRow(-1, true);
            y += headerH;
        }
        drawRow(r, false);
        y += rowH;
    }

    // Footer line
    y += 30;
    painter.setPen(QPen(QColor("#8B6F47"), 2));
    painter.drawLine(margin, y, W - margin, y);
    y += 24;
    painter.setFont(QFont("Segoe UI", 12));
    painter.setPen(QColor("#8B6F47"));
    painter.drawText(margin, y, tr("Hammer Down — Confidential"));
    painter.drawText(W - margin - 300, y,
        QString(tr("Total: %1 suppliers")).arg(model->rowCount()));

    painter.end();
}

void MainWindow::onSupplierLoad(const QModelIndex &index)
{
    if (!ui_supplier || !index.isValid()) return;

    const QAbstractItemModel *model = ui_supplier->tableView->model();
    if (!model) return;

    int row = index.row();
    // Col order: Action, Delete, ID, Company, Address, Email, Phone, Type, PostalCode
    QString suppId  = model->data(model->index(row, 2)).toString();
    QString nom     = model->data(model->index(row, 3)).toString();
    QString addr    = model->data(model->index(row, 4)).toString();
    QString email   = model->data(model->index(row, 5)).toString();
    QString tel     = model->data(model->index(row, 6)).toString();
    QString type    = model->data(model->index(row, 7)).toString();
    int     cp      = model->data(model->index(row, 8)).toInt();

    ui_supplier->le_id->setText(suppId);
    ui_supplier->le_nom->setText(nom);
    ui_supplier->le_adresse->setText(addr);
    ui_supplier->le_email->setText(email);
    ui_supplier->le_tel->setText(tel);
    ui_supplier->le_type->setText(type);
    ui_supplier->sb_cp->setValue(cp);

    // Load opening/closing hours from DB directly
    QSqlQuery hq;
    hq.prepare("SELECT OPENING_TIME, CLOSING_TIME FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    hq.bindValue(":id", suppId.toInt());
    if (hq.exec() && hq.next()) {
        QString ot = hq.value(0).toString();
        QString ct = hq.value(1).toString();
        if (m_teOpeningHour)
            m_teOpeningHour->setTime(ot.isEmpty() ? QTime(8, 0) : QTime::fromString(ot, "HH:mm"));
        if (m_teClosingHour)
            m_teClosingHour->setTime(ct.isEmpty() ? QTime(18, 0) : QTime::fromString(ct, "HH:mm"));
    }

    // Switch to the Manage Suppliers tab (index 0)
    ui_supplier->tabWidget->setCurrentIndex(0);

    // Switch radio to "Manage Supplier" mode (rb_supplier_mod_mode)
    QRadioButton *rbMod = ui_supplier->tab_gestion->findChild<QRadioButton*>("rb_supplier_mod_mode");
    if (rbMod) rbMod->setChecked(true);
}

void MainWindow::onSupplierSearch()
{
    if (!ui_supplier) return;

    QString search = ui_supplier->le_recherche->text().trimmed();

    QSqlQueryModel *model = new QSqlQueryModel(this);
    QString sqlBase =
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", SUPPLIER_ID AS \"ID\", "
        "SUPPLIER_NAME AS \"Company\", ADDRESS AS \"Address\","
        " EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", TYPE_NOTIFICATION AS \"Type\","
        " POSTAL_CODE AS \"Postal Code\""
        " FROM SUPPLIERS";

    if (search.isEmpty()) {
        model->setQuery(sqlBase + " ORDER BY SUPPLIER_ID");
    } else {
        QSqlQuery q;
        q.prepare(sqlBase + " WHERE UPPER(SUPPLIER_NAME) LIKE UPPER(:search) ORDER BY SUPPLIER_NAME");
        q.bindValue(":search", "%" + search + "%");
        q.exec();
        model->setQuery(std::move(q));
    }

    if (model->lastError().isValid()) {
        QMessageBox::warning(this, "Search Error",
            "Search failed:\n" + model->lastError().text());
        return;
    }

    ui_supplier->tableView->setModel(model);
    ui_supplier->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_supplier->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_supplier->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::onSupplierSendSMS()
{
    if (!ui_supplier) return;

    QString tel     = ui_supplier->le_tel->text().trimmed();
    QString message = ui_supplier->txt_sms->toPlainText().trimmed();

    if (tel.isEmpty()) {
        QMessageBox::warning(this, "SMS Error",
            trKey("No phone number found.\nPlease select a supplier or enter a phone number first."));
        return;
    }

    if (message.isEmpty()) {
        QMessageBox::warning(this, "SMS Error",
            trKey("Please type an SMS message in the SMS Message field."));
        return;
    }

    // --- Strategy 1: Direct PC-to-Phone Link ---
    // Protocol: sms:<number>?body=<message>
    // This allows Windows to open the "Phone Link" app (or default handler) 
    // to send the message using your synced mobile device.
    
    QString urlStr = QString("sms:%1?body=%2").arg(tel).arg(QString(QUrl::toPercentEncoding(message)));
    bool success = QDesktopServices::openUrl(QUrl(urlStr));

    if (success) {
        QMessageBox::information(this, trKey("SMS Link Opened"),
            trKey("Your system's SMS handler (like Phone Link) has been opened.\n"
                  "Please complete the sending process on your phone or PC app."));
        ui_supplier->txt_sms->clear();
    } else {
        QMessageBox::critical(this, trKey("SMS Error"),
            trKey("Failed to open the system's SMS handler.\n"
                  "Please ensure you have an app like 'Phone Link' set up on your PC."));
    }
}

void MainWindow::onSupplierUploadImage()
{
    if (!ui_supplier) return;

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Choose Supplier Image",
        QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"
    );

    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Image Error", "Could not load the selected image.");
        return;
    }

    // Scale and display in the preview label
    pixmap = pixmap.scaled(
        ui_supplier->lbl_image_preview->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    ui_supplier->lbl_image_preview->setPixmap(pixmap);
    ui_supplier->lbl_image_preview->setAlignment(Qt::AlignCenter);
}

// ==================== Supplier Delivery Rating System ====================

void MainWindow::onSupplierEnsureReviewsTable()
{
    QSqlQuery q;
    // We now use SUPPLIERS directly, adding JSON columns and aggregations if missing
    q.exec("ALTER TABLE SUPPLIERS ADD RATINGS_JSON CLOB");
    q.exec("ALTER TABLE SUPPLIERS ADD AVERAGE_RATING NUMBER(3,2) DEFAULT 0");
    q.exec("ALTER TABLE SUPPLIERS ADD NOTIFICATIONS_JSON CLOB");
    // Ignore ORA-01430 if they already exist
}

void MainWindow::onSupplierPopulateRatingCombos()
{
    if (!ui_supplier) return;

    // 1. Populate Suppliers (the master filter)
    ui_supplier->cb_supplier_reviews->clear();
    ui_supplier->cb_supplier_reviews->addItem("-- Select Supplier --", -1);
    QSqlQuery qSupp;
    qSupp.prepare("SELECT SUPPLIER_ID, SUPPLIER_NAME FROM SUPPLIERS ORDER BY SUPPLIER_NAME");
    if (qSupp.exec()) {
        while (qSupp.next()) {
            QString name = qSupp.value(1).toString();
            int sid = qSupp.value(0).toInt();
            ui_supplier->cb_supplier_reviews->addItem(name.isEmpty() ? QString("Supplier #%1").arg(sid) : name, sid);
        }
    }

    // 2. Populate Employees
    ui_supplier->cb_employee_rating->clear();
    ui_supplier->cb_employee_rating->addItem("-- Select Employee --", 0);
    QSqlQuery qEmp("SELECT EMPLOYEE_ID, LAST_NAME || ' ' || FIRST_NAME FROM EMPLOYEES ORDER BY LAST_NAME");
    if (qEmp.exec()) {
        while (qEmp.next()) {
            ui_supplier->cb_employee_rating->addItem(qEmp.value(1).toString(), qEmp.value(0));
        }
    }

    // 3. Populate Equipment
    ui_supplier->cb_equipment_rating->clear();
    ui_supplier->cb_equipment_rating->addItem("-- Select Equipment --", 0);
    QSqlQuery qEquip("SELECT EQUIPMENT_ID, DESCRIPTION FROM EQUIPMENT WHERE STATUS != 'Retired' ORDER BY DESCRIPTION");
    if (qEquip.exec()) {
        while (qEquip.next()) {
            ui_supplier->cb_equipment_rating->addItem(qEquip.value(1).toString(), qEquip.value(0));
        }
    }
}

void MainWindow::onSupplierReviewRatingChanged(int value)
{
    if (!ui_supplier) return;
    QString stars;
    for (int i = 0; i < value; ++i)  stars += QChar(0x2605); // ★
    for (int i = value; i < 5; ++i) stars += QChar(0x2606); // ☆
    ui_supplier->lbl_rating_stars->setText(stars);

    if (homeWindow && homeWindow->isAnimationMode()) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_supplier->lbl_rating_stars);
        ui_supplier->lbl_rating_stars->setGraphicsEffect(eff);
        QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutBack);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::onSupplierReviewLoad()
{
    if (!ui_supplier) return;

    int suppId = ui_supplier->cb_supplier_reviews->currentData().toInt();
    if (suppId <= 0) {
        ui_supplier->lbl_avg_score->setText("–");
        ui_supplier->lbl_stars_row->setText("☆☆☆☆☆");
        ui_supplier->lbl_review_count->setText("No supplier selected");
        return;
    }

    // Fetch the JSON array from the SUPPLIERS table
    QSqlQuery qFetch;
    qFetch.prepare("SELECT RATINGS_JSON FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    qFetch.bindValue(":id", suppId);
    if (!qFetch.exec() || !qFetch.next()) return;

    QString jsonStr = qFetch.value(0).toString();
    QJsonArray ratingsArr;
    if (!jsonStr.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isArray()) ratingsArr = doc.array();
    }

    // Prepare dictionary maps for foreign keys
    QMap<int, QString> empMap;
    QSqlQuery qEmp("SELECT EMPLOYEE_ID, LAST_NAME || ' ' || FIRST_NAME FROM EMPLOYEES");
    if (qEmp.exec()) {
        while (qEmp.next()) empMap[qEmp.value(0).toInt()] = qEmp.value(1).toString();
    }

    QMap<int, QString> eqMap;
    QSqlQuery qEq("SELECT EQUIPMENT_ID, DESCRIPTION FROM EQUIPMENT WHERE STATUS != 'Retired'");
    if (qEq.exec()) {
        while (qEq.next()) eqMap[qEq.value(0).toInt()] = qEq.value(1).toString();
    }

    // Populate the UI Table Reviews
    QStandardItemModel *model = new QStandardItemModel(ratingsArr.size(), 5, this);
    model->setHorizontalHeaderLabels({"Rating", "Employee", "Equipment", "Note", "Date"});

    int total = ratingsArr.size();
    double sum = 0;
    int c5 = 0, c4 = 0, c3 = 0, c2 = 0, c1 = 0;

    for (int i = 0; i < total; ++i) {
        // Read backwards to show newest first
        QJsonObject obj = ratingsArr[total - 1 - i].toObject();
        int r = obj["rating"].toInt();
        int eId = obj["employee_id"].toInt();
        int eqId = obj["equipment_id"].toInt();
        QString note = obj["note"].toString();
        QString date = obj["date"].toString().left(10); // get YYYY-MM-DD

        sum += r;
        if (r == 5) c5++; else if (r == 4) c4++; else if (r == 3) c3++; else if (r == 2) c2++; else if (r == 1) c1++;

        model->setItem(i, 0, new QStandardItem(QString::number(r)));
        model->setItem(i, 1, new QStandardItem(empMap.value(eId, "")));
        model->setItem(i, 2, new QStandardItem(eqMap.value(eqId, "")));
        model->setItem(i, 3, new QStandardItem(note));
        model->setItem(i, 4, new QStandardItem(date));
    }

    ui_supplier->table_reviews->setModel(model);
    ui_supplier->table_reviews->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_supplier->table_reviews->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_supplier->table_reviews->setSelectionBehavior(QAbstractItemView::SelectRows);

    double avg = total == 0 ? 0 : (sum / total);

    if (total == 0) {
        ui_supplier->lbl_avg_score->setText("–");
        ui_supplier->lbl_stars_row->setText("☆☆☆☆☆");
        ui_supplier->lbl_review_count->setText("No deliveries rated yet");
        ui_supplier->pb_dist_5->setValue(0);
        ui_supplier->pb_dist_4->setValue(0);
        ui_supplier->pb_dist_3->setValue(0);
        ui_supplier->pb_review_quality->setValue(0);
        ui_supplier->pb_review_response->setValue(0);
        ui_supplier->pb_review_price->setValue(0);
        return;
    }

    // Average score label
    ui_supplier->lbl_avg_score->setText(QString::number(avg, 'f', 1));

    // Star display: filled + empty
    int filled = qRound(avg);
    QString starsStr;
    for (int i = 0; i < filled; ++i)  starsStr += QChar(0x2605);
    for (int i = filled; i < 5; ++i)  starsStr += QChar(0x2606);
    ui_supplier->lbl_stars_row->setText(starsStr);

    // Review count
    ui_supplier->lbl_review_count->setText(
        QString("Based on %1 delivery rating%2").arg(total).arg(total == 1 ? "" : "s"));

    // Distribution bars (percentage of total)
    ui_supplier->pb_dist_5->setValue(total > 0 ? c5 * 100 / total : 0);
    ui_supplier->pb_dist_4->setValue(total > 0 ? c4 * 100 / total : 0);
    ui_supplier->pb_dist_3->setValue(total > 0 ? c3 * 100 / total : 0);
    // reuse lbl_dist_insight to show 2- and 1-star counts textually
    ui_supplier->lbl_dist_insight->setText(
        QString("2★: %1  |  1★: %2\n(out of %3 total ratings)").arg(c2).arg(c1).arg(total));

    // Satisfaction breakdown — all driven by the single avg
    int avgPct = qRound(avg / 5.0 * 100);
    ui_supplier->pb_review_quality->setValue(qMin(100, avgPct));
    ui_supplier->pb_review_response->setValue(qMin(100, qMax(0, qRound((avg - 0.3) / 5.0 * 100))));
    ui_supplier->pb_review_price->setValue(qMin(100, qMax(0, qRound((avg + 0.2) / 5.0 * 100))));
}

void MainWindow::onSupplierReviewSubmit()
{
    if (!ui_supplier) return;

    int suppId = ui_supplier->cb_supplier_reviews->currentData().toInt();
    if (suppId <= 0) {
        QMessageBox::warning(this, "No Supplier", "Please select a supplier first.");
        return;
    }

    int rating = ui_supplier->sb_rating->value();
    QString note = ui_supplier->le_comment->text().trimmed();
    int employeeId = ui_supplier->cb_employee_rating->currentData().toInt();
    int equipmentId = ui_supplier->cb_equipment_rating->currentData().toInt();

    // Fetch existing JSON ratings array
    QSqlQuery qFetch;
    qFetch.prepare("SELECT RATINGS_JSON FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    qFetch.bindValue(":id", suppId);
    QJsonArray ratingsArr;
    if (qFetch.exec() && qFetch.next()) {
        QString jsonStr = qFetch.value(0).toString();
        if (!jsonStr.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            if (doc.isArray()) ratingsArr = doc.array();
        }
    }

    // Append new rating object
    QJsonObject newRating;
    newRating["rating"] = rating;
    newRating["note"] = note;
    newRating["employee_id"] = employeeId;
    newRating["equipment_id"] = equipmentId;
    newRating["date"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    ratingsArr.append(newRating);

    // Calculate new average
    double total = 0;
    for (int i = 0; i < ratingsArr.size(); ++i) {
        total += ratingsArr[i].toObject()["rating"].toDouble();
    }
    double avg = ratingsArr.isEmpty() ? 0 : (total / ratingsArr.size());

    // Serialize back to string
    QJsonDocument newDoc(ratingsArr);
    QString newJsonStr = QString::fromUtf8(newDoc.toJson(QJsonDocument::Compact));

    // Update SUPPLIERS table
    QSqlQuery q;
    q.prepare("UPDATE SUPPLIERS SET RATINGS_JSON = :json, AVERAGE_RATING = :avg WHERE SUPPLIER_ID = :id");
    q.bindValue(":json", newJsonStr);
    q.bindValue(":avg", avg);
    q.bindValue(":id", suppId);

    if (q.exec()) {
        QString starStr;
        for (int i = 0; i < rating; ++i) starStr += QChar(0x2605);
        QString displayName = ui_supplier->cb_supplier_reviews->currentText();
        QMessageBox::information(this, "Rating Logged",
            QString("Successfully logged a %1 rating for %2.").arg(starStr).arg(displayName));
            
        ui_supplier->le_comment->clear();
        ui_supplier->sb_rating->setValue(5);
        onSupplierReviewLoad(); // Refresh dashboard and history
    } else {
        QMessageBox::critical(this, "Database Error",
            "Failed to log rating.\n\n" + q.lastError().databaseText());
    }
}

void MainWindow::onEquipmentClearFields()
{
    if (ui_equipment) {
        ui_equipment->le_id->clear();
        ui_equipment->le_type->clear();
        ui_equipment->de_date_achat->setDate(QDate::currentDate());
        ui_equipment->te_desc->clear();
        ui_equipment->sb_quantity->setValue(0);

        ui_equipment->dsb_unit_price->setValue(0.0);
        ui_equipment->cb_status->setCurrentIndex(0); // Reset to 'Available'
    }
}

void MainWindow::setupClientManagement()
{
    // 1. Rename 'Add Client' tab to 'Manage Clients'
    setTabTextTr(ui_client->tabWidget, ui_client->tab_add, "Manage Clients");

    // 2. Reparent 'Modify Client' GroupBox to 'Add Client' tab
    ui_client->group_modify->setParent(ui_client->tab_add);
    
    // Position it same as group_add
    ui_client->group_add->move(20, 70);
    ui_client->group_modify->move(20, 70);

    // Hide modify group initially
    ui_client->group_modify->setVisible(false);
    ui_client->group_add->setVisible(true);

    // 3. Create Toggle Radio Buttons
    QRadioButton *rbAdd = new QRadioButton(trKey("Add Mode"), ui_client->tab_add);
    QRadioButton *rbMod = new QRadioButton(trKey("Modify Mode"), ui_client->tab_add);
    rbMod->setObjectName("rb_client_mod_mode");
    setTrKey(rbAdd, "Add Mode");
    setTrKey(rbMod, "Modify Mode");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);
    
    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // 4. Connect Signals
    connect(rbAdd, &QRadioButton::toggled, [=](bool checked){
        if(checked) {
            ui_client->group_add->setVisible(true);
            ui_client->group_modify->setVisible(false);
        }
    });

    connect(rbMod, &QRadioButton::toggled, [=](bool checked){
        if(checked) {
            ui_client->group_add->setVisible(false);
            ui_client->group_modify->setVisible(true);
        }
    });

    // 5. Remove the empty 'Modify Client' tab
    int modifyTabIndex = ui_client->tabWidget->indexOf(ui_client->tab_modify);
    if (modifyTabIndex != -1) {
        ui_client->tabWidget->removeTab(modifyTabIndex);
    }
    
    // 6. Cyberpunk Input Validations
    QRegularExpression nameRegex("^[a-zA-Z\\s\\-']+$");
    QValidator *nameVal = new QRegularExpressionValidator(nameRegex, this);
    ui_client->le_nom->setValidator(nameVal);
    ui_client->le_prenom->setValidator(nameVal);
    ui_client->le_nom_mod->setValidator(nameVal);
    ui_client->le_prenom_mod->setValidator(nameVal);

    QRegularExpression phoneRegex("^\\+?\\d{8,15}$");
    QValidator *phoneVal = new QRegularExpressionValidator(phoneRegex, this);
    ui_client->le_tel->setValidator(phoneVal);
    ui_client->le_tel_mod->setValidator(phoneVal);

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    QValidator *emailVal = new QRegularExpressionValidator(emailRegex, this);
    ui_client->le_email->setValidator(emailVal);
    ui_client->le_email_mod->setValidator(emailVal);

    QRegularExpression idRegex("^[1-9]\\d*$");
    QValidator *idVal = new QRegularExpressionValidator(idRegex, this);
    ui_client->le_id_mod->setValidator(idVal);

    // 7. Inject Cyber Tabs Dynamically to bypass ui cache
    QWidget *traceTab = new QWidget();
    ui_client->tabWidget->addTab(traceTab, "Cyber Trace");
    m_clientCyberTable = new QTableView(traceTab);
    m_clientCyberTable->setGeometry(20, 20, 1200, 660);
    m_clientCyberTable->setStyleSheet("QTableView { background: rgba(0,0,0,0.6); gridline-color: #5A4A32; border: 1px solid #8B6F47; color: #D4AF37; font-family: 'Consolas'; } QHeaderView::section { background: rgba(139,111,71,0.3); border: 1px solid #8B6F47; color: #D4AF37; font-weight: bold; } QTableView::item:selected { background: rgba(139,111,71,0.5); border: 1px solid #D4AF37; }");

    QPushButton *btn_refresh_trace = new QPushButton("UPDATE LOG", traceTab);
    btn_refresh_trace->setGeometry(1070, 690, 150, 40);
    btn_refresh_trace->setStyleSheet("QPushButton { background: rgba(139, 111, 71, 0.4); border: 1px solid #8B6F47; border-radius: 5px; color: #D4AF37; font-weight: bold; font-family: 'Consolas'; } QPushButton:hover { background: rgba(139, 111, 71, 0.8); border: 1px solid #D4AF37; }");

    QWidget *matrixTab = new QWidget();
    ui_client->tabWidget->addTab(matrixTab, "Data Matrix");
    m_clientMatrixFrame = new QFrame(matrixTab);
    m_clientMatrixFrame->setGeometry(100, 100, 1040, 550);
    m_clientMatrixFrame->setStyleSheet("background: rgba(10, 10, 10, 0.7); border: 2px solid #8B6F47; border-radius: 10px;");

    connect(btn_refresh_trace, &QPushButton::clicked, this, &MainWindow::onClientCyberTraceRefresh);
    
    // Call data matrix initialization
    setupClientDataMatrix();
}

void MainWindow::onClientCyberTraceRefresh()
{
    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray auditArray;
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isArray()) auditArray = doc.array();
            file.close();
        }
    }

    QVector<QJsonObject> filtered;
    filtered.reserve(auditArray.size());
    for (const QJsonValue &v : auditArray) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value("module_name").toString() == "Clients") filtered.push_back(o);
    }

    std::sort(filtered.begin(), filtered.end(), [](const QJsonObject &a, const QJsonObject &b) {
        const qint64 at = a.value("timestamp_ms").toVariant().toLongLong();
        const qint64 bt = b.value("timestamp_ms").toVariant().toLongLong();
        return bt < at; // descending
    });

    QStandardItemModel *model = new QStandardItemModel(filtered.size(), 3, this);
    model->setHorizontalHeaderLabels({"Timestamp", "Operative", "Action Sequence"});

    for (int r = 0; r < filtered.size(); ++r) {
        const QJsonObject o = filtered.at(r);
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(o.value("timestamp_ms").toVariant().toLongLong());
        if (!dt.isValid()) dt = QDateTime::fromString(o.value("timestamp_iso").toString(), Qt::ISODate);
        const QString timeStr = dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : QString();

        model->setItem(r, 0, new QStandardItem(timeStr));
        model->setItem(r, 1, new QStandardItem(o.value("employee_name").toString()));
        model->setItem(r, 2, new QStandardItem(o.value("action_details").toString()));
    }

    m_clientCyberTable->setModel(model);
    m_clientCyberTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_clientCyberTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clientCyberTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Grid cyber animation on refresh
    QGraphicsOpacityEffect *traceEff = new QGraphicsOpacityEffect(this);
    m_clientCyberTable->setGraphicsEffect(traceEff);
    QPropertyAnimation *traceAnim = new QPropertyAnimation(traceEff, "opacity");
    traceAnim->setDuration(600);
    traceAnim->setStartValue(0.1);
    traceAnim->setEndValue(1.0);
    traceAnim->setEasingCurve(QEasingCurve::InBack);
    traceAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::setupClientDataMatrix()
{
    // Clean old layout
    if (m_clientMatrixFrame->layout()) {
        QLayoutItem* item;
        while ((item = m_clientMatrixFrame->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete m_clientMatrixFrame->layout();
    }

    QVBoxLayout *layout = new QVBoxLayout(m_clientMatrixFrame);
    
    QSqlQuery q;
    int total = 0, male = 0, female = 0;
    if (q.exec("SELECT COUNT(*), SUM(CASE WHEN GENDER='Male' THEN 1 ELSE 0 END), SUM(CASE WHEN GENDER='Female' THEN 1 ELSE 0 END) FROM CLIENTS")) {
        if (q.next()) {
            total = q.value(0).toInt();
            male = q.value(1).toInt();
            female = q.value(2).toInt();
        }
    }

    QLabel *statsLabel = new QLabel(m_clientMatrixFrame);
    statsLabel->setStyleSheet("color: #00FF41; font-family: 'Consolas'; font-size: 20px; text-align: left; background: transparent;");
    statsLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statsLabel);
    
    QString fullText = QString(
        "NETWORK TARGETS IDENTIFIED: %1\n\n"
        "[+] MALE OPERATIVES: %2\n"
        "[+] FEMALE OPERATIVES: %3\n\n"
        "DATABASE LINK ... ACTIVE"
    ).arg(total).arg(male).arg(female);

    // Typing effect via Lambda and Timer
    for (int i = 1; i <= fullText.length(); ++i) {
        QTimer::singleShot(i * 25, statsLabel, [statsLabel, fullText, i]() {
            statsLabel->setText(fullText.left(i));
        });
    }
    
    // Cool Cyberpunk Animation Effect
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    m_clientMatrixFrame->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(1200);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::InExpo);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::setupEquipmentStats()
{
    if (!ui_equipment || !ui_equipment->tab_stats) return;

    if (ui_equipment->tab_stats->layout()) {
        delete ui_equipment->tab_stats->layout();
    }
    for (auto *w : ui_equipment->tab_stats->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        w->hide();
    }

    EquipmentStatsVisualData data;

    // Existing baseline queries kept intact.
    QString lm = QDate::currentDate().addMonths(-1).toString("yyyy-MM");
    QSqlQuery qSt("SELECT STATUS, COUNT(*), SUM(UNIT_PRICE) FROM EQUIPMENT WHERE STATUS != 'Retired' GROUP BY STATUS");
    while (qSt.next()) {
        const QString s = qSt.value(0).toString();
        const int c = qSt.value(1).toInt();
        data.total += c;
        data.totalValue += qSt.value(2).toDouble();
        if (s == "Available") data.available = c;
        else if (s == "In Use") data.inUse = c;
        else if (s == "Under Maintenance") data.maintenance = c;
        else if (s == "Retired") data.retired = c;
    }

    QSqlQuery qRet("SELECT COUNT(*) FROM EQUIPMENT WHERE STATUS = 'Retired'");
    if (qRet.exec() && qRet.next()) {
        data.retired = qRet.value(0).toInt();
    }

    QSqlQuery qTrend;
    qTrend.prepare("SELECT * FROM (SELECT COUNT(*), SUM(UNIT_PRICE) FROM EQUIPMENT WHERE STATUS != 'Retired' AND TO_CHAR(PURCHASE_DATE, 'YYYY-MM') = :m) WHERE ROWNUM <= 1");
    qTrend.bindValue(":m", lm);
    if (qTrend.exec() && qTrend.next()) {
        data.lastMonthTotal = qTrend.value(0).toInt();
        data.lastMonthValue = qTrend.value(1).toDouble();
    }

    // Existing ranking query kept (used for activity card baseline).
    QString topType;
    int topTypeCount = 0;
    QSqlQuery rq("SELECT EQUIPMENT_TYPE, COUNT(*) as cnt FROM EQUIPMENT WHERE STATUS != 'Retired' GROUP BY EQUIPMENT_TYPE ORDER BY cnt DESC");
    if (rq.next()) {
        topType = rq.value(0).toString();
        topTypeCount = rq.value(1).toInt();
    }

    QSqlQuery qRank(
        "SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, UNIT_PRICE, STATUS, QUANTITY "
        "FROM EQUIPMENT ORDER BY UNIT_PRICE DESC, EQUIPMENT_ID ASC"
    );
    while (qRank.next() && data.ranking.size() < 8) {
        EquipmentRankingEntry e;
        e.id = qRank.value(0).toInt();
        e.type = qRank.value(1).toString();
        e.name = QString("#%1 %2").arg(e.id).arg(e.type);
        e.unitPrice = qRank.value(2).toDouble();
        e.status = qRank.value(3).toString();
        e.quantity = qRank.value(4).toInt();
        data.ranking.push_back(e);
    }

    QMap<QString, int> addMap;
    QMap<QString, int> maintMap;
    QSqlQuery qAdd(
        "SELECT TO_CHAR(PURCHASE_DATE, 'YYYY-MM') AS M, COUNT(*) "
        "FROM EQUIPMENT WHERE PURCHASE_DATE IS NOT NULL "
        "GROUP BY TO_CHAR(PURCHASE_DATE, 'YYYY-MM') ORDER BY M"
    );
    while (qAdd.next()) {
        addMap[qAdd.value(0).toString()] = qAdd.value(1).toInt();
    }
    QSqlQuery qMaint(
        "SELECT TO_CHAR(NEXT_MAINTENANCE, 'YYYY-MM') AS M, COUNT(*) "
        "FROM EQUIPMENT WHERE NEXT_MAINTENANCE IS NOT NULL "
        "GROUP BY TO_CHAR(NEXT_MAINTENANCE, 'YYYY-MM') ORDER BY M"
    );
    while (qMaint.next()) {
        maintMap[qMaint.value(0).toString()] = qMaint.value(1).toInt();
    }

    QStringList months = addMap.keys();
    for (const QString &k : maintMap.keys()) {
        if (!months.contains(k)) months << k;
    }
    std::sort(months.begin(), months.end());
    if (months.size() > 8) {
        months = months.mid(months.size() - 8);
    }
    for (const QString &m : months) {
        QDate d = QDate::fromString(m + "-01", "yyyy-MM-dd");
        data.months << (d.isValid() ? d.toString("MMM") : m);
        data.monthlyAdded << addMap.value(m, 0);
        data.monthlyMaintenance << maintMap.value(m, 0);
    }

    QSqlQuery qDocs(
        "SELECT COUNT(*), "
        "SUM(CASE WHEN DESCRIPTION IS NOT NULL AND TRIM(DESCRIPTION) != '' THEN 1 ELSE 0 END), "
        "AVG((SYSDATE - PURCHASE_DATE) / 365.25) "
        "FROM EQUIPMENT"
    );
    int docTotal = 0;
    int docFilled = 0;
    if (qDocs.exec() && qDocs.next()) {
        docTotal = qDocs.value(0).toInt();
        docFilled = qDocs.value(1).toInt();
        data.avgAgeYears = qMax(0.0, qDocs.value(2).toDouble());
    }

    QSqlQuery qOld(
        "SELECT EQUIPMENT_TYPE FROM EQUIPMENT "
        "WHERE PURCHASE_DATE = (SELECT MIN(PURCHASE_DATE) FROM EQUIPMENT WHERE PURCHASE_DATE IS NOT NULL) "
        "AND ROWNUM = 1"
    );
    if (qOld.exec() && qOld.next()) {
        data.oldestName = qOld.value(0).toString();
    }

    QSqlQuery qNew(
        "SELECT EQUIPMENT_TYPE, PURCHASE_DATE FROM EQUIPMENT "
        "WHERE PURCHASE_DATE = (SELECT MAX(PURCHASE_DATE) FROM EQUIPMENT WHERE PURCHASE_DATE IS NOT NULL) "
        "AND ROWNUM = 1"
    );
    if (qNew.exec() && qNew.next()) {
        data.newestName = qNew.value(0).toString();
        data.newestAgeName = data.newestName;
        data.newestDate = qNew.value(1).toDate();
        if (!data.newestDate.isValid()) {
            data.newestDate = qNew.value(1).toDateTime().date();
        }
        if (data.newestDate.isValid()) {
            data.newestDays = data.newestDate.daysTo(QDate::currentDate());
        }
    }

    QSqlQuery qActive(
        "SELECT EQUIPMENT_TYPE, "
        "(CASE WHEN NEXT_MAINTENANCE IS NOT NULL THEN 1 ELSE 0 END + "
        " CASE WHEN STATUS = 'Under Maintenance' THEN 1 ELSE 0 END) AS EV "
        "FROM EQUIPMENT ORDER BY EV DESC, UNIT_PRICE DESC"
    );
    if (qActive.exec() && qActive.next()) {
        data.mostActiveName = qActive.value(0).toString();
        data.mostActiveEvents = qActive.value(1).toInt();
    } else {
        data.mostActiveName = topType;
        data.mostActiveEvents = topTypeCount;
    }

    const int all = qMax(1, data.total + data.retired);
    const double health = (100.0 * data.available) / all;
    const double maintenanceScore = 100.0 - (100.0 * data.maintenance / all);

    double maxUnit = 1.0;
    QSqlQuery qMaxPrice("SELECT NVL(MAX(UNIT_PRICE), 1) FROM EQUIPMENT");
    if (qMaxPrice.exec() && qMaxPrice.next()) {
        maxUnit = qMaxPrice.value(0).toDouble();
    }
    const double valueDensity = qBound(0.0, (data.totalValue / all) / qMax(1.0, maxUnit) * 100.0, 100.0);

    int activityTotal = 0;
    for (int v : data.monthlyAdded) activityTotal += v;
    for (int v : data.monthlyMaintenance) activityTotal += v;
    const double activityLevel = qBound(0.0, activityTotal * 7.0, 100.0);

    const double ageBalance = qBound(0.0, 100.0 - (data.avgAgeYears / 20.0) * 100.0, 100.0);
    const double documentation = docTotal > 0 ? (100.0 * docFilled / docTotal) : 0.0;
    data.radarScores = {health, maintenanceScore, valueDensity, activityLevel, ageBalance, documentation};

    QWidget *host = new QWidget(ui_equipment->tab_stats);
    host->setObjectName("dashContainer");
    host->setAttribute(Qt::WA_StyledBackground, true);
    host->setStyleSheet("background: transparent;");

    auto *layout = new QVBoxLayout(ui_equipment->tab_stats);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(host);

    auto *hostLayout = new QVBoxLayout(host);
    hostLayout->setContentsMargins(0, 0, 0, 0);
    auto *canvas = new EquipmentStatsCanvas(host);
    canvas->setData(data);
    hostLayout->addWidget(canvas);
}




void MainWindow::setupSupplierStats()
{
    if (!ui_supplier) return;


    // --- 1. Top Level Metrics (Aggregated from Database) ---

    // Product Quality: % of all individual ratings that are 4 or 5 stars
    {
        int totalRatings = 0, highRatings = 0;
        QSqlQuery qQual("SELECT RATINGS_JSON FROM SUPPLIERS WHERE RATINGS_JSON IS NOT NULL");
        while (qQual.next()) {
            QJsonArray arr = QJsonDocument::fromJson(qQual.value(0).toString().toUtf8()).array();
            for (const QJsonValue &v : arr) {
                double r = v.toObject()["rating"].toDouble();
                if (r > 0) { totalRatings++; if (r >= 4.0) highRatings++; }
            }
        }
        int qualityPct = (totalRatings > 0) ? qBound(0, qRound(100.0 * highRatings / totalRatings), 100) : 0;
        ui_supplier->pb_quality->setValue(qualityPct);
    }

    // Contact Coverage: % of suppliers with both email AND phone on file (responsiveness proxy)
    {
        int total = 0, contactComplete = 0;
        QSqlQuery qCov(
            "SELECT COUNT(*), "
            "SUM(CASE WHEN EMAIL IS NOT NULL AND TRIM(EMAIL) != '' "
            "         AND PHONE_NUMBER IS NOT NULL AND TRIM(TO_CHAR(PHONE_NUMBER)) != '' "
            "    THEN 1 ELSE 0 END) "
            "FROM SUPPLIERS"
        );
        if (qCov.exec() && qCov.next()) {
            total           = qCov.value(0).toInt();
            contactComplete = qCov.value(1).toInt();
        }
        int coveragePct = (total > 0) ? qBound(0, qRound(100.0 * contactComplete / total), 100) : 0;
        // Relabel so it no longer says "Delivery Speed"
        ui_supplier->lbl_bar_speed->setText(trKey("Contact Coverage:"));
        ui_supplier->pb_speed->setValue(coveragePct);
    }

    // Retention: % of suppliers that have submitted at least one rating
    {
        int total = 0, withRating = 0;
        QSqlQuery qRet("SELECT COUNT(*), SUM(CASE WHEN AVERAGE_RATING > 0 THEN 1 ELSE 0 END) FROM SUPPLIERS");
        if (qRet.exec() && qRet.next()) {
            total      = qRet.value(0).toInt();
            withRating = qRet.value(1).toInt();
        }
        int retPct = (total > 0) ? qBound(0, qRound(100.0 * withRating / total), 100) : 0;
        ui_supplier->lbl_percent_retention->setText(QString::number(retPct) + "%");
    }

    // Accuracy: % of suppliers whose average rating is >= 3.0 ("good or better")
    {
        int total = 0, highRating = 0;
        QSqlQuery qAcc("SELECT COUNT(*), SUM(CASE WHEN AVERAGE_RATING >= 3 THEN 1 ELSE 0 END) FROM SUPPLIERS WHERE AVERAGE_RATING > 0");
        if (qAcc.exec() && qAcc.next()) {
            total      = qAcc.value(0).toInt();
            highRating = qAcc.value(1).toInt();
        }
        int accPct = (total > 0) ? qBound(0, qRound(100.0 * highRating / total), 100) : 0;
        ui_supplier->lbl_percent_accuracy->setText(QString::number(accPct) + "%");
    }

    // --- 2. Chart 1: Product Categories (Real Data) ---
    QPieSeries *seriesCat = new QPieSeries();
    seriesCat->setHoleSize(0.45);
    
    QSqlQuery qCats("SELECT TYPE_NOTIFICATION, COUNT(*) FROM SUPPLIERS GROUP BY TYPE_NOTIFICATION");
    int catIdx = 0;
    QStringList catColors = {"#D4AF37", "#8B6F47", "#5D4037", "#2E1A0C", "#A0825A"};
    while(qCats.next()) {
        QString cat = qCats.value(0).toString();
        int count = qCats.value(1).toInt();
        if (cat.isEmpty()) cat = trKey("Other");
        QPieSlice *slice = seriesCat->append(cat, count);
        slice->setBrush(QColor(catColors.at(catIdx % catColors.size())));
        slice->setLabelVisible();
        slice->setLabelColor(Qt::white);
        catIdx++;
    }

    QChart *chartCat = new QChart();
    chartCat->addSeries(seriesCat);
    chartCat->setTitle(trKey("Category Distribution"));
    chartCat->setTitleBrush(QBrush(QColor("#D4AF37")));
    chartCat->setAnimationOptions(QChart::SeriesAnimations);
    chartCat->legend()->setAlignment(Qt::AlignRight);
    chartCat->legend()->setLabelColor(Qt::white);
    chartCat->setBackgroundBrush(Qt::transparent);
    
    QChartView *viewCat = new QChartView(chartCat);
    viewCat->setRenderHint(QPainter::Antialiasing);
    viewCat->setStyleSheet("background: transparent;");
    
    if (ui_supplier->frame_chart_types->layout()) {
        QLayoutItem *child;
        while ((child = ui_supplier->frame_chart_types->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    } else {
        new QVBoxLayout(ui_supplier->frame_chart_types);
    }
    if (ui_supplier->chart_types_view) ui_supplier->chart_types_view->hide();
    ui_supplier->frame_chart_types->layout()->addWidget(viewCat);

    // --- 3. Chart 2: Monthly Satisfaction Trend (Real Data) ---
    QBarSet *setScore = new QBarSet(trKey("Avg Rating"));
    QStringList categories;
    
    QMap<int, QList<double>> monthScores;
    QSqlQuery qGet("SELECT RATINGS_JSON FROM SUPPLIERS WHERE RATINGS_JSON IS NOT NULL");
    while (qGet.next()) {
        QString json = qGet.value(0).toString();
        if (json.isEmpty()) continue;
        QJsonArray arr = QJsonDocument::fromJson(json.toUtf8()).array();
        for (int i=0; i<arr.size(); i++) {
            QJsonObject obj = arr[i].toObject();
            QDate d = QDate::fromString(obj["date"].toString().left(10), "yyyy-MM-dd");
            int m = d.month(); // 1-12
            if (m >= 1 && m <= 12) {
                monthScores[m].append(obj["rating"].toDouble());
            }
        }
    }
    
    QList<int> months = monthScores.keys();
    std::sort(months.begin(), months.end());
    for (int m : months) {
        double sum = 0;
        for (double val : monthScores[m]) sum += val;
        categories << QDate(2000, m, 1).toString("Mon");
        *setScore << (sum / monthScores[m].size());
    }
    setScore->setColor(QColor("#D4AF37"));

    QBarSeries *seriesTrend = new QBarSeries();
    seriesTrend->append(setScore);

    QChart *chartTrend = new QChart();
    chartTrend->addSeries(seriesTrend);
    chartTrend->setTitle(trKey("Monthly Satisfaction Trend"));
    chartTrend->setTitleBrush(QBrush(QColor("#D4AF37")));
    chartTrend->setAnimationOptions(QChart::SeriesAnimations);
    chartTrend->setBackgroundBrush(Qt::transparent);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(Qt::white);
    chartTrend->addAxis(axisX, Qt::AlignBottom);
    seriesTrend->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 5);
    axisY->setLabelsColor(Qt::white);
    chartTrend->addAxis(axisY, Qt::AlignLeft);
    seriesTrend->attachAxis(axisY);
    chartTrend->legend()->setVisible(false);

    QChartView *viewTrend = new QChartView(chartTrend);
    viewTrend->setRenderHint(QPainter::Antialiasing);
    viewTrend->setStyleSheet("background: transparent;");

    if (ui_supplier->frame_chart_reviews->layout()) {
        QLayoutItem *child;
        while ((child = ui_supplier->frame_chart_reviews->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    } else {
        new QVBoxLayout(ui_supplier->frame_chart_reviews);
    }
    if (ui_supplier->chart_reviews_view) ui_supplier->chart_reviews_view->hide();
    ui_supplier->frame_chart_reviews->layout()->addWidget(viewTrend);

    // --- 4. Top Performer Card — real Compliance Score + Consistency Index ---
    QSqlQuery qTop(
        "SELECT SUPPLIER_NAME, AVERAGE_RATING, RATINGS_JSON "
        "FROM SUPPLIERS "
        "WHERE AVERAGE_RATING > 0 "
        "ORDER BY AVERAGE_RATING DESC"
    );
    if (qTop.next()) {
        const QString topName   = qTop.value(0).toString();
        const double  topRating = qTop.value(1).toDouble();
        const QString ratJson   = qTop.value(2).toString();

        double compliancePct  = 0.0;
        QString consistLabel  = trKey("N/A");

        if (!ratJson.isEmpty()) {
            QJsonArray arr = QJsonDocument::fromJson(ratJson.toUtf8()).array();
            const int n = arr.size();
            if (n > 0) {
                int goodCount = 0;
                double sumSqDev = 0.0;
                for (const QJsonValue &v : arr) {
                    const double r = v.toObject()["rating"].toDouble();
                    if (r >= 3.0) goodCount++;
                    sumSqDev += (r - topRating) * (r - topRating);
                }
                compliancePct = 100.0 * goodCount / n;
                const double stdDev = qSqrt(sumSqDev / n);
                if      (stdDev <= 0.5) consistLabel = trKey("High");
                else if (stdDev <= 1.0) consistLabel = trKey("Good");
                else                   consistLabel = trKey("Variable");
            }
        }

        ui_supplier->lbl_top_performer->setText(
            trKey("🏆 Top Performer: ") + topName + "\n" +
            trKey("Compliance Score: ") + QString::number(compliancePct, 'f', 1) + "%\n" +
            trKey("Consistency Index: ") + consistLabel
        );
    } else {
        // No suppliers with ratings yet
        ui_supplier->lbl_top_performer->setText(trKey("No rated suppliers yet."));
    }

    // --- 5. Network Status summary (was hardcoded) ---
    {
        int activeCount   = 0;
        int inactiveCount = 0;
        int totalCount    = 0;
        QSqlQuery qStatus(
            "SELECT ACCOUNT_STATUS, COUNT(*) "
            "FROM SUPPLIERS "
            "GROUP BY ACCOUNT_STATUS"
        );
        while (qStatus.next()) {
            const QString st  = qStatus.value(0).toString();
            const int     cnt = qStatus.value(1).toInt();
            totalCount += cnt;
            if (st.compare("Active", Qt::CaseInsensitive) == 0)
                activeCount = cnt;
            else
                inactiveCount += cnt;
        }

        // Derive a simple health label from the active ratio
        QString perfLabel;
        if (totalCount == 0) {
            perfLabel = trKey("N/A");
        } else {
            double ratio = (double)activeCount / totalCount;
            if (ratio >= 0.85)      perfLabel = trKey("Optimal");
            else if (ratio >= 0.60) perfLabel = trKey("Good");
            else if (ratio >= 0.40) perfLabel = trKey("Fair");
            else                   perfLabel = trKey("Poor");
        }

        const QString now = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
        ui_supplier->lbl_summary_val->setText(
            QString(trKey("Active: %1  |  Inactive: %2  |  Performance: %3\nLast updated: %4"))
                .arg(activeCount)
                .arg(inactiveCount)
                .arg(perfLabel)
                .arg(now)
        );
    }

    // --- 6. Top-2 supplier types by avg rating (was hardcoded North/South bars) ---
    {
        // Fetch the two top-performing TYPE_NOTIFICATION categories by average rating.
        // Scale the 1-5 avg rating to 0-100 for the progress bars.
        QSqlQuery qTypeRating(
            "SELECT TYPE_NOTIFICATION, "
            "       AVG(CASE WHEN AVERAGE_RATING > 0 THEN AVERAGE_RATING ELSE NULL END) AS AVG_R, "
            "       COUNT(*) AS CNT "
            "FROM SUPPLIERS "
            "GROUP BY TYPE_NOTIFICATION "
            "ORDER BY AVG_R DESC NULLS LAST"
        );

        // Row 1 — best type
        if (qTypeRating.next()) {
            const QString typeName = qTypeRating.value(0).toString().isEmpty()
                                     ? trKey("General") : qTypeRating.value(0).toString();
            const double  avgR     = qTypeRating.value(1).isNull() ? 0.0
                                     : qTypeRating.value(1).toDouble();
            const int     barVal   = qBound(0, qRound(avgR * 20.0), 100); // 1-5 → 0-100

            // Re-label the static QLabel sitting next to pb_reg_1
            ui_supplier->lbl_reg_1->setText(typeName + ":");
            ui_supplier->pb_reg_1->setValue(barVal);
            ui_supplier->pb_reg_1->setToolTip(
                QString(trKey("Type: %1  —  Avg rating: %2 / 5.0"))
                    .arg(typeName)
                    .arg(QString::number(avgR, 'f', 1))
            );
        }

        // Row 2 — second-best type
        if (qTypeRating.next()) {
            const QString typeName = qTypeRating.value(0).toString().isEmpty()
                                     ? trKey("Other") : qTypeRating.value(0).toString();
            const double  avgR     = qTypeRating.value(1).isNull() ? 0.0
                                     : qTypeRating.value(1).toDouble();
            const int     barVal   = qBound(0, qRound(avgR * 20.0), 100);

            ui_supplier->lbl_reg_2->setText(typeName + ":");
            ui_supplier->pb_reg_2->setValue(barVal);
            ui_supplier->pb_reg_2->setToolTip(
                QString(trKey("Type: %1  —  Avg rating: %2 / 5.0"))
                    .arg(typeName)
                    .arg(QString::number(avgR, 'f', 1))
            );
        }
    }
}

void MainWindow::setupClientCalendar()
{
    // Replace the contents of the existing tab_calendar widget in-place
    QWidget *calendarTab = ui_client->tab_calendar;

    // Hide existing UI-file children (do NOT delete — ui_client still holds those pointers)
    for (QWidget *child : calendarTab->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
        child->hide();

    QHBoxLayout *mainLayout = new QHBoxLayout(calendarTab);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // =========================================================================
    // LEFT — styled calendar frame
    // =========================================================================
    QFrame *calFrame = new QFrame();
    calFrame->setStyleSheet(R"(
        QFrame {
            background: rgba(28, 22, 16, 0.88);
            border-radius: 14px;
            border: 1px solid #4A3728;
        }
    )");
    QVBoxLayout *calFrameLayout = new QVBoxLayout(calFrame);
    calFrameLayout->setContentsMargins(0, 0, 0, 10);
    calFrameLayout->setSpacing(0);

    QCalendarWidget *calendar = new QCalendarWidget();
    calendar->setGridVisible(false);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    calendar->setStyleSheet(R"(
        QCalendarWidget { background: transparent; }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #1C1610;
            border-radius: 14px 14px 0 0;
            padding: 8px 12px;
            min-height: 46px;
        }
        QCalendarWidget QToolButton {
            color: #D4AF37;
            font-size: 13px;
            font-weight: bold;
            background: transparent;
            border: none;
            padding: 6px 14px;
            border-radius: 8px;
        }
        QCalendarWidget QToolButton:hover { background: rgba(212,175,55,0.18); }
        QCalendarWidget QToolButton::menu-indicator { image: none; }
        QCalendarWidget QMenu {
            background: #2C2418;
            color: #D4AF37;
            border: 1px solid #8B6F47;
            border-radius: 6px;
        }
        QCalendarWidget QSpinBox {
            color: #D4AF37;
            background: transparent;
            font-size: 14px;
            font-weight: bold;
            border: none;
            selection-background-color: #8B6F47;
        }
        QCalendarWidget QAbstractItemView {
            background: transparent;
            color: #D8C9B0;
            font-size: 13px;
            selection-background-color: #8B6F47;
            selection-color: white;
            alternate-background-color: transparent;
            outline: none;
            gridline-color: transparent;
        }
        QCalendarWidget QAbstractItemView:disabled { color: #3D3020; }
        QCalendarWidget QWidget { alternate-background-color: transparent; background: transparent; }
    )");
    calFrameLayout->addWidget(calendar);

    // Legend strip
    QWidget *legend = new QWidget();
    legend->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *legendLayout = new QHBoxLayout(legend);
    legendLayout->setContentsMargins(16, 2, 16, 4);
    legendLayout->setSpacing(6);
    auto addDot = [&](const QString &lbl, const QString &hex) {
        QLabel *dot = new QLabel();
        dot->setFixedSize(10, 10);
        dot->setStyleSheet(QString("background:%1; border-radius:5px; border:none;").arg(hex));
        QLabel *txt = new QLabel(lbl);
        txt->setStyleSheet("color:#806050; font-size:11px; background:transparent; border:none;");
        legendLayout->addWidget(dot);
        legendLayout->addWidget(txt);
        legendLayout->addSpacing(8);
    };
    addDot("Order",       "#D4AF37");
    addDot("Client",      "#78C878");
    addDot("Employee",    "#60B4D8");
    addDot("Equipment",   "#C080E0");
    addDot("Maintenance", "#FF8060");
    addDot("Supplier",    "#E8C040");
    legendLayout->addStretch();
    calFrameLayout->addWidget(legend);

    // =========================================================================
    // Fetch ALL date-bearing events from every module
    // =========================================================================
    // eventMap: "yyyy-MM-dd" -> list of (type, display text)
    QMap<QString, QList<QPair<QString,QString>>> eventMap;

    // 1. Orders — order_date
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(o.order_date,'YYYY-MM-DD'), o.order_type, o.order_status, "
                   "NVL(c.FIRST_NAME||' '||c.LAST_NAME,'Unknown') "
                   "FROM ORDERS o LEFT JOIN CLIENTS c ON o.client_id=c.CLIENT_ID "
                   "WHERE o.order_date IS NOT NULL ORDER BY o.order_date")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"order", q.value(1).toString() + "  —  " + q.value(3).toString()
                              + "  [" + q.value(2).toString() + "]"});
        }
    }

    // 2. Clients — REGISTRATION_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(REGISTRATION_DATE,'YYYY-MM-DD'), "
                   "FIRST_NAME||' '||LAST_NAME FROM CLIENTS WHERE REGISTRATION_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"client", "Client joined:  " + q.value(1).toString()});
        }
    }

    // 3. Employees — HIRE_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(HIRE_DATE,'YYYY-MM-DD'), "
                   "FIRST_NAME||' '||LAST_NAME, JOB_TITLE "
                   "FROM EMPLOYEES WHERE HIRE_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"employee", "Hired:  " + q.value(1).toString()
                                 + "  (" + q.value(2).toString() + ")"});
        }
    }

    // 4. Equipment — PURCHASE_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(PURCHASE_DATE,'YYYY-MM-DD'), EQUIPMENT_TYPE, STATUS "
                   "FROM EQUIPMENT WHERE PURCHASE_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"equipment", "Purchased:  " + q.value(1).toString()
                                  + "  [" + q.value(2).toString() + "]"});
        }
    }

    // 5. Equipment — NEXT_MAINTENANCE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(NEXT_MAINTENANCE,'YYYY-MM-DD'), EQUIPMENT_TYPE "
                   "FROM EQUIPMENT WHERE NEXT_MAINTENANCE IS NOT NULL AND STATUS != 'Retired'")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"maintenance", "Maintenance due:  " + q.value(1).toString()});
        }
    }

    // 6. Suppliers — REGISTRATION_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(REGISTRATION_DATE,'YYYY-MM-DD'), SUPPLIER_NAME "
                   "FROM SUPPLIERS WHERE REGISTRATION_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"supplier", "Supplier registered:  " + q.value(1).toString()});
        }
    }

    // Colour-code dates — priority: maintenance > order > equipment > employee > client > supplier
    struct TypeInfo { QString type; QColor bg; QColor fg; };
    const QList<TypeInfo> typePriority = {
        {"maintenance", QColor(200,80,60,70),  QColor("#FF8060")},
        {"order",       QColor(139,111,71,70), QColor("#D4AF37")},
        {"equipment",   QColor(140,80,200,60), QColor("#C080E0")},
        {"employee",    QColor(60,140,190,60), QColor("#60B4D8")},
        {"client",      QColor(80,160,80,60),  QColor("#78C878")},
        {"supplier",    QColor(180,160,40,60), QColor("#E8C040")},
    };

    for (auto it = eventMap.cbegin(); it != eventMap.cend(); ++it) {
        QDate d = QDate::fromString(it.key(), "yyyy-MM-dd");
        if (!d.isValid()) continue;
        QSet<QString> types;
        for (const auto &ev : it.value()) types.insert(ev.first);

        for (const auto &ti : typePriority) {
            if (!types.contains(ti.type)) continue;
            QTextCharFormat fmt;
            fmt.setBackground(ti.bg);
            fmt.setForeground(ti.fg);
            fmt.setFontWeight(QFont::Bold);
            calendar->setDateTextFormat(d, fmt);
            break; // highest-priority type wins
        }
    }

    // Today highlight (always override so it stays visible)
    {
        QTextCharFormat todayFmt;
        todayFmt.setBackground(QColor("#8B6F47"));
        todayFmt.setForeground(QColor("#FFEFCF"));
        todayFmt.setFontWeight(QFont::Bold);
        calendar->setDateTextFormat(QDate::currentDate(), todayFmt);
    }

    // =========================================================================
    // RIGHT — event detail panel
    // =========================================================================
    QFrame *rightPanel = new QFrame();
    rightPanel->setMinimumWidth(270);
    rightPanel->setMaximumWidth(330);
    rightPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    rightPanel->setStyleSheet(R"(
        QFrame {
            background: rgba(28, 22, 16, 0.88);
            border-radius: 14px;
            border: 1px solid #4A3728;
        }
    )");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    rightLayout->setSpacing(8);

    QLabel *lblDate = new QLabel("Select a date");
    lblDate->setAlignment(Qt::AlignCenter);
    lblDate->setWordWrap(true);
    lblDate->setStyleSheet(R"(
        font-size: 14px; font-weight: bold; color: #D4AF37;
        background: rgba(139,111,71,0.12); border-radius: 8px;
        border: 1px solid #3A2A1A; padding: 10px 6px;
    )");

    QLabel *lblCount = new QLabel("");
    lblCount->setAlignment(Qt::AlignCenter);
    lblCount->setStyleSheet("font-size: 11px; color: #6A5040; background: transparent; border: none;");

    auto makeSep = [&]() -> QFrame* {
        QFrame *sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("background: #3A2A1A; border: none; max-height: 1px;");
        return sep;
    };

    QString listStyle = R"(
        QListWidget {
            background: transparent; border: none;
            color: #D8C9B0; font-size: 12px; outline: none;
        }
        QListWidget::item {
            padding: 7px 10px; border-radius: 6px; margin: 2px 0;
            background: rgba(255,255,255,0.03);
        }
        QListWidget::item:hover { background: rgba(139,111,71,0.15); }
        QListWidget::item:selected { background: rgba(139,111,71,0.30); color: #D4AF37; }
    )";

    QListWidget *eventList = new QListWidget();
    eventList->setStyleSheet(listStyle);

    QLabel *lblUpHdr = new QLabel("Upcoming — Next 7 Days");
    lblUpHdr->setStyleSheet("font-size: 11px; font-weight: bold; color: #8B6F47; "
                             "background: transparent; border: none; padding: 2px 0;");

    QListWidget *upcomingList = new QListWidget();
    upcomingList->setMaximumHeight(180);
    upcomingList->setStyleSheet(listStyle);

    // Populate upcoming list
    QDate today = QDate::currentDate();
    bool anyUpcoming = false;
    for (int i = 0; i <= 7; ++i) {
        QDate d = today.addDays(i);
        const QString key = d.toString("yyyy-MM-dd");
        if (!eventMap.contains(key)) continue;
        for (const auto &ev : eventMap[key]) {
            QString dayLbl = (i == 0) ? "Today" : (i == 1) ? "Tomorrow"
                                                 : d.toString("ddd d MMM");
            auto *item = new QListWidgetItem(dayLbl + ":  " + ev.second);
            if      (ev.first == "maintenance") item->setForeground(QColor("#FF8060"));
            else if (ev.first == "client")      item->setForeground(QColor("#78C878"));
            else if (ev.first == "employee")    item->setForeground(QColor("#60B4D8"));
            else if (ev.first == "equipment")   item->setForeground(QColor("#C080E0"));
            else if (ev.first == "supplier")    item->setForeground(QColor("#E8C040"));
            else                                item->setForeground(QColor("#D4AF37"));
            upcomingList->addItem(item);
            anyUpcoming = true;
        }
    }
    if (!anyUpcoming) {
        auto *item = new QListWidgetItem("No events in the next 7 days.");
        item->setForeground(QColor("#444"));
        upcomingList->addItem(item);
    }

    QPushButton *btnToday = new QPushButton("Go to Today");
    btnToday->setCursor(Qt::PointingHandCursor);
    btnToday->setStyleSheet(R"(
        QPushButton {
            background: #8B6F47; color: white;
            border-radius: 8px; padding: 9px 0;
            font-weight: bold; font-size: 13px; border: none;
        }
        QPushButton:hover { background: #D4AF37; color: #1C1610; }
        QPushButton:pressed { background: #6B4F2F; }
    )");

    rightLayout->addWidget(lblDate);
    rightLayout->addWidget(lblCount);
    rightLayout->addWidget(makeSep());
    rightLayout->addWidget(eventList, 1);
    rightLayout->addWidget(makeSep());
    rightLayout->addWidget(lblUpHdr);
    rightLayout->addWidget(upcomingList);
    rightLayout->addWidget(btnToday);

    // =========================================================================
    // Connections
    // =========================================================================
    connect(btnToday, &QPushButton::clicked, calendar, [calendar]() {
        calendar->setSelectedDate(QDate::currentDate());
        emit calendar->clicked(QDate::currentDate());
    });

    connect(calendar, &QCalendarWidget::clicked, this,
            [lblDate, lblCount, eventList, eventMap](const QDate &date) {
        lblDate->setText(date.toString("dddd, MMMM d yyyy"));
        eventList->clear();
        const QString key = date.toString("yyyy-MM-dd");
        const auto &evs = eventMap.value(key);
        if (evs.isEmpty()) {
            lblCount->setText("No events");
            auto *item = new QListWidgetItem("No events scheduled.");
            item->setForeground(QColor("#444"));
            eventList->addItem(item);
        } else {
            lblCount->setText(QString::number(evs.size())
                              + (evs.size() == 1 ? " event" : " events"));
            for (const auto &ev : evs) {
                QString prefix;
                QColor  color;
                if      (ev.first == "order")       { prefix = "Order      "; color = QColor("#D4AF37"); }
                else if (ev.first == "client")      { prefix = "Client     "; color = QColor("#78C878"); }
                else if (ev.first == "employee")    { prefix = "Employee   "; color = QColor("#60B4D8"); }
                else if (ev.first == "equipment")   { prefix = "Equipment  "; color = QColor("#C080E0"); }
                else if (ev.first == "maintenance") { prefix = "Maint.     "; color = QColor("#FF8060"); }
                else                                { prefix = "Supplier   "; color = QColor("#E8C040"); }
                auto *item = new QListWidgetItem(prefix + ev.second);
                item->setForeground(color);
                eventList->addItem(item);
            }
        }
    });

    // =========================================================================
    // Assemble main layout
    // =========================================================================
    mainLayout->addWidget(calFrame, 60);
    mainLayout->addWidget(rightPanel, 40);
}

void MainWindow::setupEmployeeModes()
{
    // Rename tab_add to "Manage Employees"
    setTabTextTr(ui_employee->tabWidget, ui_employee->tab_add, "Manage Employees");

    // Replace le_fonction with QComboBox
    // Parent it to group_add so geometry() matches the existing label/input layout
    QComboBox *cbJob = new QComboBox(ui_employee->group_add);
    cbJob->setObjectName("cb_job_title");
    // Keep the same capitalization used elsewhere (and commonly stored in DB)
    cbJob->addItems({"Smith", "Cleaner", "Developer", "Cashier", "Carpenter", "Boss"});
    cbJob->setGeometry(ui_employee->le_fonction->geometry());
    cbJob->setStyleSheet(
        "QComboBox {"
        " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);"
        " border: 2px solid #8B6F47;"
        " border-radius: 8px;"
        " padding: 3px 12px;"
        " font-size: 14px;"
        " color: #333;"
        "}"
        "QComboBox:hover {"
        " border: 2px solid #A0825A;"
        "}"
        "QComboBox:focus {"
        " border: 2px solid #8B4513;"
        " background: #FFFAF0;"
        "}"
        "QComboBox QAbstractItemView {"
        " background-color: #FFFFFF;"
        " color: #333333;"
        " border: 1px solid #8B6F47;"
        " selection-background-color: #8B6F47;"
        " selection-color: #FFFFFF;"
        " outline: 0;"
        "}"
    );
    cbJob->setEditable(false);
    cbJob->setInsertPolicy(QComboBox::NoInsert);
    ui_employee->le_fonction->hide();
    cbJob->show();
    connect(cbJob, &QComboBox::currentTextChanged, this, &MainWindow::updateSalaryInsight);

    // Strict Input Validation (Contrôle de Saisie)
    // ID: numbers only
    ui_employee->le_id->setValidator(new QIntValidator(1, 999999, this));
    
    // Names: alpha characters only
    QRegularExpression nameRegex("^[A-Za-z\\s]*$");
    ui_employee->le_nom->setValidator(new QRegularExpressionValidator(nameRegex, this));
    ui_employee->le_prenom->setValidator(new QRegularExpressionValidator(nameRegex, this));
    
    // Phone: exactly 8 digits logic handled by mask
    ui_employee->le_num->setValidator(new QIntValidator(0, 99999999, this));
    ui_employee->le_num->setMaxLength(8);

    // Create Radio Buttons in tab_add
    QRadioButton *rbAdd = new QRadioButton(trKey("Add Employee"), ui_employee->tab_add);
    QRadioButton *rbMod = new QRadioButton(trKey("Modify Employee"), ui_employee->tab_add);
    rbAdd->setObjectName("rb_employee_add_mode");
    rbMod->setObjectName("rb_employee_mod_mode");
    setTrKey(rbAdd, "Add Employee");
    setTrKey(rbMod, "Modify Employee");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // Move group lower
    ui_employee->group_add->move(20, 70);

    // Lambda to update UI
    ui_employee->group_add->setProperty("trTitleAddKey", "Add Employee");
    ui_employee->group_add->setProperty("trTitleModKey", "Modify Employee");
    ui_employee->group_add->setProperty("trModeAddRadio", "rb_employee_add_mode");
    ui_employee->group_add->setProperty("trModeModRadio", "rb_employee_mod_mode");

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_employee->group_add->setTitle(trKey("Add Employee"));
            ui_employee->btn_add->setVisible(true);
            ui_employee->btn_modify->setVisible(false);
            ui_employee->le_id->setEnabled(true);
            toggleEmployeeFields(true);
        } else {
            ui_employee->group_add->setTitle(trKey("Modify Employee"));
            ui_employee->btn_add->setVisible(false);
            ui_employee->btn_modify->setVisible(true);
            ui_employee->le_id->setEnabled(false);
            // In modify mode, keep fields enabled for the currently selected employee.
            // We only "lock" when no employee is selected (empty ID).
            toggleEmployeeFields(!ui_employee->le_id->text().trimmed().isEmpty());
        }
    };
    
    connect(rbAdd, &QRadioButton::toggled, updateUI);
    connect(rbMod, &QRadioButton::toggled, [=](bool checked){ updateUI(!checked); });
    
    // Initialize Camera for Employee Management Scan
    m_empCamera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    m_empCaptureSession = new QMediaCaptureSession(this);
    m_empVideoSink = new QVideoSink(this);
    m_empCaptureSession->setCamera(m_empCamera);
    m_empCaptureSession->setVideoSink(m_empVideoSink);
    connect(m_empVideoSink, &QVideoSink::videoFrameChanged, this, &MainWindow::processEmpCameraFrame);
}


void MainWindow::setupSupplierModes()
{
    setTabTextTr(ui_supplier->tabWidget, ui_supplier->tab_gestion, "Manage Suppliers");

    QRadioButton *rbAdd = new QRadioButton(trKey("Add Supplier"), ui_supplier->tab_gestion);
    QRadioButton *rbMod = new QRadioButton(trKey("Manage Supplier"), ui_supplier->tab_gestion);
    rbAdd->setObjectName("rb_supplier_add_mode");
    rbMod->setObjectName("rb_supplier_mod_mode");
    setTrKey(rbAdd, "Add Supplier");
    setTrKey(rbMod, "Manage Supplier");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_supplier->groupBox_gestion->move(20, 70);

    ui_supplier->groupBox_gestion->setProperty("trTitleAddKey", "");
    ui_supplier->groupBox_gestion->setProperty("trTitleModKey", "");
    ui_supplier->groupBox_gestion->setProperty("trModeAddRadio", "rb_supplier_add_mode");
    ui_supplier->groupBox_gestion->setProperty("trModeModRadio", "rb_supplier_mod_mode");

    // --- Opening / Closing Hours widgets (single row below SMS field) ---
    QString lblStyle = "color: white; font-size: 13px; font-weight: bold; background: transparent;";
    QString teStyle  = "background: white; border: 2px solid #8B6F47; border-radius: 8px; padding: 2px 8px; font-size: 13px; color: #333;";

    QLabel *lblOpen = new QLabel("Open:", ui_supplier->groupBox_gestion);
    lblOpen->setStyleSheet(lblStyle);
    lblOpen->setGeometry(200, 470, 55, 28);
    lblOpen->show();

    m_teOpeningHour = new QTimeEdit(ui_supplier->groupBox_gestion);
    m_teOpeningHour->setDisplayFormat("HH:mm");
    m_teOpeningHour->setGeometry(260, 468, 90, 28);
    m_teOpeningHour->setStyleSheet(teStyle);
    m_teOpeningHour->setTime(QTime(8, 0));
    m_teOpeningHour->show();

    QLabel *lblClose = new QLabel("Close:", ui_supplier->groupBox_gestion);
    lblClose->setStyleSheet(lblStyle);
    lblClose->setGeometry(365, 470, 55, 28);
    lblClose->show();

    m_teClosingHour = new QTimeEdit(ui_supplier->groupBox_gestion);
    m_teClosingHour->setDisplayFormat("HH:mm");
    m_teClosingHour->setGeometry(425, 468, 90, 28);
    m_teClosingHour->setStyleSheet(teStyle);
    m_teClosingHour->setTime(QTime(18, 0));
    m_teClosingHour->show();
    // --- End hours widgets ---

    // --- Notification Bell button (placed on the tab_gestion, not groupBox) ---
    m_supplierBellBtn = new QPushButton(ui_supplier->tab_gestion);
    m_supplierBellBtn->setText(QString(QChar(0xD83D)) + QChar(0xDD14)); // 🔔
    m_supplierBellBtn->setObjectName("btn_supplier_bell");
    m_supplierBellBtn->setGeometry(1060, 8, 44, 44);
    m_supplierBellBtn->setStyleSheet(
        "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
        "QPushButton:hover { background-color: #a3845a; }"
        "QPushButton:pressed{ background-color: #6b5535; }");
    m_supplierBellBtn->setCursor(Qt::PointingHandCursor);
    m_supplierBellBtn->setToolTip("Supplier Notifications");
    m_supplierBellBtn->show();
    connect(m_supplierBellBtn, &QPushButton::clicked, this, &MainWindow::onSupplierBellClicked);
    // Defer notification scan until after all setup is complete
    QTimer::singleShot(1500, this, &MainWindow::checkAndPostSupplierNotifications);
    // ---

    // --- Form Completion Progress Bar ---
    m_supplierProgress = new QProgressBar(ui_supplier->groupBox_gestion);
    m_supplierProgress->setRange(0, 100);
    m_supplierProgress->setValue(0);
    m_supplierProgress->setTextVisible(false);
    m_supplierProgress->setFixedHeight(12);
    m_supplierProgress->setStyleSheet(
        "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");

    QLabel *pTitle = new QLabel("Form Completion:", ui_supplier->groupBox_gestion);
    pTitle->setObjectName("lbl_supp_prog_title");
    pTitle->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 11px; font-family: 'Segoe UI';");

    auto makeSuppInd = [&](const QString &txt, const QString &obj) {
        QLabel *l = new QLabel(txt, ui_supplier->groupBox_gestion);
        l->setObjectName(obj);
        l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        return l;
    };
    m_suppNameInd = makeSuppInd("[👤 Name ⬜]", "ind_supp_name"); 
    m_suppEmailInd = makeSuppInd("[📧 Email ⬜]", "ind_supp_email"); 
    m_suppTelInd = makeSuppInd("[📞 Phone ⬜]", "ind_supp_tel"); 
    m_suppTypeInd = makeSuppInd("[🏢 Type ⬜]", "ind_supp_type");

    pTitle->move(50, 40);
    m_supplierProgress->setGeometry(50, 60, 555, 12);
    m_suppNameInd->move(50, 78);
    m_suppEmailInd->move(150, 78);
    m_suppTelInd->move(260, 78);
    m_suppTypeInd->move(380, 78);

    connect(ui_supplier->le_nom, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->le_email, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->le_tel, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->le_type, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);


    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_supplier->groupBox_gestion->setTitle("");
            ui_supplier->btn_add->setVisible(true);
            ui_supplier->btn_modify->setVisible(false);
            ui_supplier->btn_delete->setVisible(false);
            
            m_supplierProgress->setVisible(true);
            pTitle->setVisible(true);
            m_suppNameInd->setVisible(true);
            m_suppEmailInd->setVisible(true);
            m_suppTelInd->setVisible(true);
            m_suppTypeInd->setVisible(true);
        } else {
            ui_supplier->groupBox_gestion->setTitle("");
            ui_supplier->btn_add->setVisible(false);
            ui_supplier->btn_modify->setVisible(true);
            ui_supplier->btn_delete->setVisible(true);
            
            m_supplierProgress->setVisible(false);
            pTitle->setVisible(false);
            m_suppNameInd->setVisible(false);
            m_suppEmailInd->setVisible(false);
            m_suppTelInd->setVisible(false);
            m_suppTypeInd->setVisible(false);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);

    if (homeWindow && homeWindow->isAnimationMode()) {
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_add);
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_modify);
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_delete);
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_send_sms);
    }
}


void MainWindow::setupEquipmentModes()
{
    // Ensure audit log table exists


    // tab_gestion
    setTabTextTr(ui_equipment->tabWidget, ui_equipment->tab_gestion, "Manage Equipment");

    QRadioButton *rbAdd = new QRadioButton(trKey("Add Equipment"), ui_equipment->tab_gestion);
    QRadioButton *rbMod = new QRadioButton(trKey("Manage Equipment"), ui_equipment->tab_gestion);
    rbAdd->setObjectName("rb_equipment_add_mode");
    rbMod->setObjectName("rb_equipment_mod_mode");
    setTrKey(rbAdd, "Add Equipment");
    setTrKey(rbMod, "Manage Equipment");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 180, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_equipment->groupBox_gestion->move(20, 70);

    ui_equipment->groupBox_gestion->setProperty("trTitleAddKey", "Add Equipment");
    ui_equipment->groupBox_gestion->setProperty("trTitleModKey", "Manage Equipment");
    ui_equipment->groupBox_gestion->setProperty("trModeAddRadio", "rb_equipment_add_mode");
    ui_equipment->groupBox_gestion->setProperty("trModeModRadio", "rb_equipment_mod_mode");

    // --- Form Completion Progress Bar ---
    m_equipProgress = new QProgressBar(ui_equipment->groupBox_gestion);
    m_equipProgress->setRange(0, 100);
    m_equipProgress->setValue(0);
    m_equipProgress->setTextVisible(false);
    m_equipProgress->setFixedHeight(12);
    m_equipProgress->setStyleSheet(
        "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");

    QLabel *pTitle = new QLabel("Form Completion:", ui_equipment->groupBox_gestion);
    pTitle->setObjectName("lbl_prog_title");
    pTitle->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 11px; font-family: 'Segoe UI';");

    auto makeInd = [&](const QString &txt, const QString &obj) {
        QLabel *l = new QLabel(txt, ui_equipment->groupBox_gestion);
        l->setObjectName(obj);
        l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        return l;
    };
    m_eqTypeInd = makeInd("[🔨 Type ⬜]", "ind_type"); 
    m_eqDateInd = makeInd("[📅 Date ⬜]", "ind_date"); 
    m_eqPriceInd= makeInd("[💰 Price ⬜]", "ind_price"); 
    m_eqDescInd = makeInd("[📝 Desc ⬜]", "ind_desc");

    ui_equipment->label_type->setText(QString::fromUtf8("\xF0\x9F\x94\xA7 Type:"));
    ui_equipment->label_date_achat->setText(QString::fromUtf8("\xF0\x9F\x93\x85 Purchase Date:"));
    ui_equipment->label_unit_price->setText(QString::fromUtf8("\xF0\x9F\x92\xB0 Unit Price (dt):"));
    ui_equipment->label_etat->setText(QString::fromUtf8("\xF0\x9F\x93\x8A Status:"));
    ui_equipment->label_quantity->setText(QString::fromUtf8("\xF0\x9F\x93\xA6 Quantity:"));
    ui_equipment->label_desc->setText(QString::fromUtf8("\xF0\x9F\x93\x9D Description:"));

    const QString compactLabelStyle = "color: white; font-size: 11px; font-weight: bold; background: transparent;";
    ui_equipment->label_type->setStyleSheet(compactLabelStyle);
    ui_equipment->label_date_achat->setStyleSheet(compactLabelStyle);
    ui_equipment->label_unit_price->setStyleSheet(compactLabelStyle);
    ui_equipment->label_etat->setStyleSheet(compactLabelStyle);
    ui_equipment->label_quantity->setStyleSheet(compactLabelStyle);
    ui_equipment->label_desc->setStyleSheet(compactLabelStyle);
    ui_equipment->label_id->setStyleSheet(compactLabelStyle);

    ui_equipment->le_type->setFixedHeight(32);
    ui_equipment->de_date_achat->setFixedHeight(32);
    ui_equipment->dsb_unit_price->setFixedHeight(32);
    ui_equipment->cb_status->setFixedHeight(32);
    ui_equipment->sb_quantity->setFixedHeight(32);
    ui_equipment->te_desc->setFixedHeight(80);

    auto updateUI = [=](bool isAdd) {
        // Shift amount for other fields when ID is hidden
        int yOffset = isAdd ? 22 : 0;
        
        m_equipProgress->setVisible(isAdd);
        pTitle->setVisible(isAdd);
        m_eqTypeInd->setVisible(isAdd);
        m_eqDateInd->setVisible(isAdd);
        m_eqPriceInd->setVisible(isAdd);
        m_eqDescInd->setVisible(isAdd);

        if(isAdd) {
            ui_equipment->groupBox_gestion->setTitle(trKey("Add Equipment"));
            ui_equipment->btn_add->setVisible(true);
            ui_equipment->btn_modify->setVisible(false);
            ui_equipment->btn_delete->setVisible(false);
            
            ui_equipment->le_id->setVisible(false);
            ui_equipment->label_id->setVisible(false);

            // Position progress elements
            pTitle->move(30, 45);
            m_equipProgress->setGeometry(30, 68, 555, 12);
            m_eqTypeInd->move(30, 85);
            m_eqDateInd->move(130, 85);
            m_eqPriceInd->move(230, 85);
            m_eqDescInd->move(330, 85);

        } else {
            ui_equipment->groupBox_gestion->setTitle(trKey("Manage Equipment"));
            ui_equipment->btn_add->setVisible(false);
            ui_equipment->btn_modify->setVisible(true);
            ui_equipment->btn_delete->setVisible(true);
            
            ui_equipment->le_id->setVisible(true);
            ui_equipment->label_id->setVisible(true);
            ui_equipment->le_id->setEnabled(true);
            ui_equipment->le_id->setPlaceholderText("");
            ui_equipment->label_id->move(ui_equipment->label_id->x(), 70);
            ui_equipment->le_id->move(ui_equipment->le_id->x(), 70);
        }

        // Compact field stack to match the target form proportions
        const int fieldGap = 42;  // tighter vertical gap between fields
        const int typeY = 110 + yOffset;
        const int dateY = typeY + fieldGap;
        const int priceY = dateY + fieldGap;
        const int statusY = priceY + fieldGap;
        const int qtyY = statusY + fieldGap;
        const int descY = qtyY + fieldGap;

        ui_equipment->label_type->move(ui_equipment->label_type->x(), typeY);
        ui_equipment->le_type->move(ui_equipment->le_type->x(), typeY);

        ui_equipment->label_date_achat->move(ui_equipment->label_date_achat->x(), dateY);
        ui_equipment->de_date_achat->move(ui_equipment->de_date_achat->x(), dateY);

        ui_equipment->label_unit_price->move(ui_equipment->label_unit_price->x(), priceY);
        ui_equipment->dsb_unit_price->move(ui_equipment->dsb_unit_price->x(), priceY);

        ui_equipment->label_etat->move(ui_equipment->label_etat->x(), statusY);
        ui_equipment->cb_status->move(ui_equipment->cb_status->x(), statusY);

        ui_equipment->label_quantity->move(ui_equipment->label_quantity->x(), qtyY);
        ui_equipment->sb_quantity->move(ui_equipment->sb_quantity->x(), qtyY);

        ui_equipment->label_desc->move(ui_equipment->label_desc->x(), descY);
        ui_equipment->te_desc->move(ui_equipment->te_desc->x(), descY);
        ui_equipment->te_desc->setFixedHeight(80);

        // Keep clear space under description so buttons never overlap
        int btnY = descY + 80 + 18;
        ui_equipment->btn_add->move(ui_equipment->btn_add->x(), btnY);
        ui_equipment->btn_modify->move(ui_equipment->btn_modify->x(), btnY);
        ui_equipment->btn_delete->move(ui_equipment->btn_delete->x(), btnY);
        ui_equipment->btn_clear->move(ui_equipment->btn_clear->x(), btnY);
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);

    // Initial resets
    ui_equipment->dsb_unit_price->setValue(0.0);
    ui_equipment->de_date_achat->setDate(QDate::currentDate());

    // --- Connect progress update signals ---
    connect(ui_equipment->le_type, &QLineEdit::textChanged, this, &MainWindow::updateEquipProgress);
    connect(ui_equipment->de_date_achat, &QDateEdit::dateChanged, this, &MainWindow::updateEquipProgress);
    connect(ui_equipment->dsb_unit_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateEquipProgress);
    connect(ui_equipment->te_desc, &QTextEdit::textChanged, this, &MainWindow::updateEquipProgress);
    
    // Initial calculation
    updateEquipProgress();


    // Connect the View tab "?" help button directly (findChild from parent page
    // doesn't reliably cross the QTabWidget boundary, so we connect it here
    // where we have direct access to ui_equipment)
    connect(ui_equipment->btn_help_view, &QToolButton::clicked, this, [this]() {
        showTutorialOverlay(
            "This is the equipment view tutorial. Here you can search, view, modify, and delete equipment."
        );
        ui_equipment->btn_help_view->setChecked(false);
    });

    // Wire up History tab buttons
    connect(ui_equipment->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_history_search,  &QPushButton::clicked, this, &MainWindow::onEquipmentHistorySearch);
    connect(ui_equipment->le_history_search,   &QLineEdit::returnPressed, this, &MainWindow::onEquipmentHistorySearch);
    connect(ui_equipment->btn_export_history,  &QPushButton::clicked, this, &MainWindow::onEquipmentExportPDF);
    connect(ui_equipment->btn_clear_history,   &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryClear);
    connect(ui_equipment->btn_export_stats,    &QPushButton::clicked, this, &MainWindow::onEquipmentExportStatsPDF);
    connect(ui_equipment->btn_bulk_update_status, &QPushButton::clicked, this, &MainWindow::onEquipmentBulkUpdateStatus);
    connect(ui_equipment->btn_bulk_delete_all, &QPushButton::clicked, this, &MainWindow::onEquipmentDeleteAll);

    // Auto-refresh history or chat when switching tabs
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        QWidget *selected = ui_equipment->tabWidget->widget(idx);
        
        // Stop chat timer by default unless on chat tab
        chatRefreshTimer->stop();
        
        if (selected == ui_equipment->tab_history) {
            onEquipmentHistoryRefresh();
        } else if (selected == ui_equipment->tab_stats) {
            setupEquipmentStats();
        } else if (selected == ui_equipment->tab_chat) {
            onChatRefresh();
            onChatEmployeeListRefresh();
            chatRefreshTimer->start(3000); // 3 seconds refresh
        }
    });

    // Voice chat removed as requested
    if (ui_equipment->btn_chat_voice) {
        ui_equipment->btn_chat_voice->hide();
    }

    // Connect Show Name button
    if (auto *showNameBtn = equipmentPage->findChild<QPushButton*>("btn_show_name")) {
        // Functionality removed as requested
        showNameBtn->hide();
    }


    // --- Contrôle de Saisie (Input Validation) ---
    // Only letters and spaces for Type
    QRegularExpression typeRegex("^[a-zA-Z\\s]*$");
    QRegularExpressionValidator *typeVal = new QRegularExpressionValidator(typeRegex, this);
    ui_equipment->le_type->setValidator(typeVal);
    
    // Ensure price is at least 0.01
    ui_equipment->dsb_unit_price->setMinimum(0.00); 

    // Visual feedback for Type
    connect(ui_equipment->le_type, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.trimmed().isEmpty()) {
            ui_equipment->le_type->setStyleSheet("border: 2px solid #D32F2F; background: #FFEBEE; border-radius: 8px;"); // Red
            ui_equipment->le_type->setToolTip("Type is required!");
        } else {
            ui_equipment->le_type->setStyleSheet(""); // Restore default search style if needed or use previous styling
            ui_equipment->le_type->setToolTip("");
        }
    });

    // Keep Add button clickable so explicit validation alerts can be shown on click.
    auto validateForm = [=](){
        bool isValid = !ui_equipment->le_type->text().trimmed().isEmpty() &&
                       !ui_equipment->te_desc->toPlainText().trimmed().isEmpty() &&
                       ui_equipment->dsb_unit_price->value() > 0 &&
                       ui_equipment->sb_quantity->value() > 0;

        ui_equipment->btn_add->setEnabled(true);
        ui_equipment->btn_add->setToolTip(isValid ? "" : "Click Ajouter to see input error details.");
    };
    connect(ui_equipment->le_type, &QLineEdit::textChanged, validateForm);
    connect(ui_equipment->te_desc, &QTextEdit::textChanged, validateForm);
    connect(ui_equipment->dsb_unit_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged), validateForm);
    connect(ui_equipment->sb_quantity, QOverload<int>::of(&QSpinBox::valueChanged), validateForm);
    
    validateForm();

    // --- NEXUS Integration: Context Menus ---
    ui_equipment->table_equipments->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->table_equipments, &QTableView::customContextMenuRequested, this, &MainWindow::onEquipmentCustomContextMenu);

    ui_equipment->tableView_history_add->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->tableView_history_add, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){ onEquipmentHistoryCustomContextMenu(pos, 0); });
    
    ui_equipment->tableView_history_modify->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->tableView_history_modify, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){ onEquipmentHistoryCustomContextMenu(pos, 1); });

    ui_equipment->tableView_historique->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->tableView_historique, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){ onEquipmentHistoryCustomContextMenu(pos, 2); });
}

void MainWindow::setupOrderModes()
{
    const int manageBlockShiftX = 190;

    // tab_manage
    int idx = ui_order->tabWidget->indexOf(ui_order->tab_manage);
    if(idx != -1) {
        setTabTextTr(ui_order->tabWidget, ui_order->tab_manage, "Manage Orders");
    }

    // Panel matching the "Log Delivery Rating" group box style
    QGroupBox *orderPanel = new QGroupBox(trKey("Manage Orders"), ui_order->tab_manage);
    orderPanel->setObjectName("order_manage_panel");
    orderPanel->setGeometry(28 + manageBlockShiftX, 60, 615, 440);
    orderPanel->setStyleSheet(
        "QGroupBox#order_manage_panel {"
        "  background-color: rgba(60, 45, 30, 0.7);"
        "  border: 2px solid #8B6F47;"
        "  border-radius: 12px;"
        "  margin-top: 18px;"
        "  color: white;"
        "}"
        "QGroupBox#order_manage_panel::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top center;"
        "  padding: 2px 12px;"
        "  background-color: #8B6F47;"
        "  font-weight: bold;"
        "  color: white;"
        "  border-radius: 4px;"
        "}"
    );
    orderPanel->lower();
    orderPanel->show();

    const QList<QWidget*> manageWidgets = {
        ui_order->label_id,
        ui_order->le_id,
        ui_order->label_type,
        ui_order->cb_type,
        ui_order->label_stock,
        ui_order->le_stock,
        ui_order->label_prix,
        ui_order->le_prix,
        ui_order->label_buyer,
        ui_order->le_buyer,
        ui_order->btn_add,
        ui_order->btn_modify,
        ui_order->btn_delete,
        ui_order->btn_clear,
        ui_order->btn_import
    };
    for (QWidget *w : manageWidgets) {
        if (!w) continue;
        w->move(w->x() + manageBlockShiftX, w->y());
    }

    const int qrBlockShiftY = 26;

    QGroupBox *qrPanel = new QGroupBox(trKey("QR Code"), ui_order->tab_qrcode);
    qrPanel->setObjectName("order_qr_panel");
    qrPanel->setGeometry(300, 28 + qrBlockShiftY, 450, 482);
    qrPanel->setStyleSheet(
        "QGroupBox#order_qr_panel {"
        "  background-color: rgba(60, 45, 30, 0.7);"
        "  border: 2px solid #8B6F47;"
        "  border-radius: 12px;"
        "  margin-top: 18px;"
        "  color: white;"
        "}"
        "QGroupBox#order_qr_panel::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top center;"
        "  padding: 2px 12px;"
        "  background-color: #8B6F47;"
        "  font-weight: bold;"
        "  color: white;"
        "  border-radius: 4px;"
        "}"
    );
    qrPanel->lower();
    qrPanel->show();

    const QList<QWidget*> qrWidgets = {
        ui_order->label_qr_order_id,
        ui_order->le_qr_order_id,
        ui_order->btn_generate_qr,
        ui_order->label_qr_display,
        ui_order->btn_save_qr,
        ui_order->btn_print_qr
    };
    for (QWidget *w : qrWidgets) {
        if (!w) continue;
        w->move(w->x(), w->y() + qrBlockShiftY);
    }

    QRadioButton *rbAdd = new QRadioButton(trKey("Add Order"), ui_order->tab_manage);
    QRadioButton *rbMod = new QRadioButton(trKey("Manage Order"), ui_order->tab_manage);
    rbAdd->setObjectName("rb_order_add_mode");
    rbMod->setObjectName("rb_order_mod_mode");
    setTrKey(rbAdd, "Add Order");
    setTrKey(rbMod, "Manage Order");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_order->btn_add->setVisible(true);
            ui_order->btn_modify->setVisible(false);
            ui_order->btn_delete->setVisible(false);
            ui_order->btn_import->setVisible(true);
            ui_order->label_id->setVisible(false);
            ui_order->le_id->setVisible(false);
        } else {
            ui_order->btn_add->setVisible(false);
            ui_order->btn_modify->setVisible(true);
            ui_order->btn_delete->setVisible(true);
            ui_order->btn_import->setVisible(false);
            ui_order->label_id->setVisible(true);
            ui_order->le_id->setVisible(true);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);
}

void MainWindow::setupGlobalStyles()
{
    // Apply brown '?' style to all help buttons across all management pages
    QList<QToolButton*> helpBtns = this->findChildren<QToolButton*>(QRegularExpression("^btn_help.*"));
    for(QToolButton* btn : helpBtns) {
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QToolButton { background-color: #8B6F47; border-radius: 16px; color: white; font-weight: bold; border: none; font-size: 16px; }"
            "QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"
        );
        btn->setFixedSize(32, 32);
        btn->setText("?");
    }

    QString style = R"(
        /* --- General Application Style --- */
        QWidget {
            font-family: 'Gadugi', 'Segoe UI', sans-serif;
            font-size: 14px;
        }

        /* --- Buttons --- */
        QPushButton {
            background-color: #8B6F47; /* Gold/Brown */
            color: white;
            border-radius: 5px;
            padding: 8px 15px;
            font-weight: bold;
            border: 1px solid #6d5638;
        }
        QPushButton:hover {
            background-color: #a38253;
            border: 1px solid #8B6F47;
        }
        QPushButton:pressed {
            background-color: #6d5638;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
            border: 1px solid #aaaaaa;
        }

        /* --- Input Fields --- */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QComboBox {
            background-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
            padding: 5px;
            color: #333333;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QComboBox:focus {
            border: 1px solid #8B6F47;
        }

        /* --- Group Boxes --- */
        QGroupBox {
            border: 1px solid #8B6F47;
            border-radius: 6px;
            margin-top: 24px; /* Leave space for title */
            background-color: rgba(255, 255, 255, 0.8); /* Slight transparency */
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 5px 10px;
            background-color: #8B6F47;
            color: white;
            border-radius: 4px;
            font-weight: bold;
        }

        /* --- Tab Widget --- */
        QTabWidget::pane {
            border: 1px solid #cccccc;
            background: rgba(255, 255, 255, 0.9);
            border-radius: 4px;
        }
        QTabWidget::tab-bar {
            left: 5px; /* move to the right by 5px */
        }
        QTabBar::tab {
            background: #e0e0e0;
            border: 1px solid #cccccc;
            border-bottom-color: #cccccc; /* same as the pane color */
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            min-width: 8ex;
            padding: 8px 15px;
            margin-right: 2px;
            color: #333;
        }
        QTabBar::tab:selected, QTabBar::tab:hover {
            background: #8B6F47;
            color: white;
            border-color: #8B6F47;
        }

        /* --- Tables & Lists --- */
        QTableView, QListWidget {
            border: 1px solid #cccccc;
            gridline-color: #eeeeee;
            background-color: white;
            color: #333333;
            selection-background-color: #8B6F47; /* Solid Gold */
            selection-color: white;
            alternate-background-color: #f9f9f9;
        }
        QHeaderView::section {
            background-color: #8B6F47;
            color: white;
            padding: 5px;
            border: none;
            font-weight: bold;
        }
        
        /* --- Scrollbars --- */
        QScrollBar:vertical {
            border: none;
            background: #f0f0f0;
            width: 10px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #cdcdcd;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: #8B6F47;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    
    // Apply style to the entire application to ensure consistency
    qApp->setStyleSheet(style);
}

void MainWindow::setupTabNavigation(QWidget* parentWidget, QTabWidget* tabWidget, const QStringList& tabNames, int startX, int yPos, const QList<int>& targetIndices, int spacing, int afterFirstShift)
{
    int y = yPos;
    
    QString rbStyle = "QRadioButton { font-weight: bold; font-size: 12px; color: white; } QRadioButton::indicator { width: 14px; height: 14px; }";
    
    QButtonGroup *group = new QButtonGroup(parentWidget);
    group->setExclusive(true);
    
    for(int i = 0; i < tabNames.size(); i++) {
        const QString key = tabNames[i];
        const QString translated = QCoreApplication::translate("QObject", key.toUtf8().constData());
        QRadioButton *rb = new QRadioButton(translated, parentWidget);
        const int tabX = startX + (spacing * i) + (i > 0 ? afterFirstShift : 0);
        rb->setGeometry(tabX, y, 98, 28);
        rb->setStyleSheet(rbStyle);
        rb->setProperty("trKey", key);
        
        if(i == 0) rb->setChecked(true);
        
        // Connect to switch tabs
        connect(rb, &QRadioButton::toggled, [=](bool checked){
            if(checked) {
                int index = (targetIndices.size() > i) ? targetIndices[i] : i;
                tabWidget->setCurrentIndex(index);
            }
        });

        // Sync tabWidget back to radio buttons when changed programmatically
        connect(tabWidget, &QTabWidget::currentChanged, [=](int activeIndex){
            int index = (targetIndices.size() > i) ? targetIndices[i] : i;
            if (activeIndex == index) {
                rb->setChecked(true);
            }
        });

        group->addButton(rb, i);
    }
}

void MainWindow::setTabTextTr(QTabWidget *tabWidget, QWidget *tabPage, const QString &key)
{
    if (!tabWidget || !tabPage) {
        return;
    }

    int index = tabWidget->indexOf(tabPage);
    if (index == -1) {
        return;
    }

    tabPage->setProperty("tabTrKey", key);
    tabWidget->setTabText(index, trKey(key));
}

void MainWindow::retranslateDynamicRadios(QWidget *container)
{
    retranslateDynamicWidgets(container);
}

void MainWindow::retranslateDynamicWidgets(QWidget *container)
{
    if (!container) {
        return;
    }

    const auto widgets = container->findChildren<QWidget*>();
    for (auto *widget : widgets) {
        if (!widget) {
            continue;
        }

        const QVariant key = widget->property("trKey");
        if (key.isValid()) {
            const QString text = trKey(key.toString());
            if (auto *btn = qobject_cast<QAbstractButton*>(widget)) {
                btn->setText(text);
            } else if (auto *label = qobject_cast<QLabel*>(widget)) {
                label->setText(text);
            } else if (auto *box = qobject_cast<QGroupBox*>(widget)) {
                box->setTitle(text);
            }
        } else if (currentLanguage != "en") {
            if (auto *btn = qobject_cast<QAbstractButton*>(widget)) {
                const QString original = btn->text();
                if (!original.isEmpty()) {
                    const QString translated = trKey(original);
                    if (translated != original) {
                        btn->setText(translated);
                    }
                }
            }
        }

        auto *box = qobject_cast<QGroupBox*>(widget);
        if (box) {
            const QString addKey = box->property("trTitleAddKey").toString();
            const QString modKey = box->property("trTitleModKey").toString();
            const QString addRadioName = box->property("trModeAddRadio").toString();
            const QString modRadioName = box->property("trModeModRadio").toString();

            if (!addKey.isEmpty() && !modKey.isEmpty()) {
                bool isAdd = true;
                if (!addRadioName.isEmpty()) {
                    auto *rbAdd = container->findChild<QRadioButton*>(addRadioName);
                    if (rbAdd) {
                        isAdd = rbAdd->isChecked();
                    }
                } else if (!modRadioName.isEmpty()) {
                    auto *rbMod = container->findChild<QRadioButton*>(modRadioName);
                    if (rbMod) {
                        isAdd = !rbMod->isChecked();
                    }
                }

                box->setTitle(trKey(isAdd ? addKey : modKey));
            }
        }
    }

    const auto lists = container->findChildren<QListWidget*>();
    for (auto *list : lists) {
        if (!list) {
            continue;
        }

        for (int i = 0; i < list->count(); ++i) {
            auto *item = list->item(i);
            if (!item) {
                continue;
            }

            const QVariant key = item->data(Qt::UserRole);
            if (key.isValid()) {
                item->setText(trKey(key.toString()));
            }
        }
    }

    const auto tables = container->findChildren<QTableWidget*>();
    for (auto *table : tables) {
        if (!table) {
            continue;
        }

        for (int row = 0; row < table->rowCount(); ++row) {
            for (int col = 0; col < table->columnCount(); ++col) {
                QTableWidgetItem *item = table->item(row, col);
                if (item) {
                    QString text = item->text();
                    QString translated = trKey(text);
                    if (translated != text) {
                        item->setText(translated);
                    }
                }
            }
        }
    }
}

void MainWindow::retranslateDynamicTabs(QTabWidget *tabWidget)
{
    if (!tabWidget) {
        return;
    }

    for (int i = 0; i < tabWidget->count(); ++i) {
        QWidget *tab = tabWidget->widget(i);
        if (!tab) {
            continue;
        }

        const QVariant key = tab->property("tabTrKey");
        if (key.isValid()) {
            tabWidget->setTabText(i, trKey(key.toString()));
        }
    }
}

void MainWindow::retranslateDynamicCharts(QWidget *container)
{
    if (!container) {
        return;
    }

    const auto views = container->findChildren<QChartView*>();
    for (auto *view : views) {
        if (!view || !view->chart()) {
            continue;
        }

        QChart *chart = view->chart();
        const QVariant titleKey = chart->property("trTitleKey");
        if (titleKey.isValid()) {
            chart->setTitle(trKey(titleKey.toString()));
        }

        const auto seriesList = chart->series();
        for (auto *series : seriesList) {
            if (!series) {
                continue;
            }

            const QVariant nameKey = series->property("trNameKey");
            if (nameKey.isValid()) {
                series->setName(trKey(nameKey.toString()));
            }

            if (auto *pie = qobject_cast<QPieSeries*>(series)) {
                const QVariant sliceKeys = pie->property("trSliceNames");
                if (sliceKeys.isValid()) {
                    const QStringList keys = sliceKeys.toStringList();
                    const auto slices = pie->slices();
                    for (int i = 0; i < slices.size() && i < keys.size(); ++i) {
                        slices.at(i)->setLabel(trKey(keys.at(i)));
                        slices.at(i)->setLabelVisible();
                    }
                }
            }
        }

        const auto axes = chart->axes();
        for (auto *axis : axes) {
            if (!axis) {
                continue;
            }

            if (auto *valueAxis = qobject_cast<QValueAxis*>(axis)) {
                const QVariant axisKey = valueAxis->property("trTitleKey");
                if (axisKey.isValid()) {
                    valueAxis->setTitleText(trKey(axisKey.toString()));
                }
            }

            if (auto *catAxis = qobject_cast<QBarCategoryAxis*>(axis)) {
                const QVariant catKeys = catAxis->property("trCategories");
                if (catKeys.isValid()) {
                    const QStringList keys = catKeys.toStringList();
                    QStringList translated;
                    translated.reserve(keys.size());
                    for (const auto &k : keys) {
                        translated << trKey(k);
                    }
                    catAxis->setCategories(translated);
                }
            }
        }
    }
}

void MainWindow::onLanguageChanged(const QString &language)
{
    switchLanguage(language);
}

void MainWindow::switchLanguage(const QString &language)
{
    if (currentLanguage == language) {
        return;
    }
    
    currentLanguage = language;
    qApp->removeTranslator(translator);
    if (language != "en") {
        QString qmFile = ":/translations/app_" + language + ".qm";
        if (translator->load(qmFile)) {
            qApp->installTranslator(translator);
        }
    }
    if (homeWindow) {
        homeWindow->setLanguage(language);
    }
    onPageChanged(ui->stackedWidget->currentIndex());
}

void MainWindow::showWelcomeNotification(QWidget *parent, const QString &managementName)
{
    QString empName = "Team Member";
    QString empRole = "";
    if (currentEmployeeId > 0) {
        QSqlQuery nq;
        nq.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME, JOB_TITLE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        nq.bindValue(":id", currentEmployeeId);
        if (nq.exec() && nq.next()) {
            empName = nq.value(0).toString();
            empRole = nq.value(1).toString();
        }
    }
    WelcomeNotificationBar *bar = new WelcomeNotificationBar(empName, empRole, managementName, parent);
    bar->startEntrance();
}

void MainWindow::onPageChanged(int index)
{
    // --- Background Music Logic ---
    if (index == 1) {
        // Home page: stop OST1 only — OST2 is started by on_btn_home_clicked when coming from management
        if (loginAudioPlayer && loginAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(loginAudioOutput, [this](){ loginAudioPlayer->stop(); });
        }
    } else {
        // All other pages: stop home-page audio (OST2 + animation track)
        homeWindow->stopHomeAudio();
        if (homeAudioPlayer && homeAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(homeAudioOutput, [this](){ homeAudioPlayer->stop(); });
        }
        if (chatAudioPlayer && chatAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(chatAudioOutput, [this](){ chatAudioPlayer->stop(); });
        }

        // Management pages (2-6): play OST1 (including chat tab — music continues uninterrupted)
        if (index >= 2 && index <= 6) {
            if (loginAudioPlayer && loginAudioPlayer->playbackState() != QMediaPlayer::PlayingState) {
                loginAudioPlayer->setPosition(0);
                loginAudioPlayer->play();
                fadeIn(loginAudioOutput);
            }
        } else {
            // Login (0): no music
            if (loginAudioPlayer && loginAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
                fadeOut(loginAudioOutput, [this](){ loginAudioPlayer->stop(); });
            }
        }
    }
    
    // Retranslate the newly visible page
    switch (index) {
        case 0: // Login
            if (loginWindow) {
                loginWindow->retranslateUI();
            }
            break;
        case 1: // Home
            if (homeWindow) {
                homeWindow->retranslateUI();
                updateUserProfileDisplay();
                if (!m_homeWelcomeShown) {
                    showWelcomeNotification(homeWindow, "Home");
                    m_homeWelcomeShown = true;
                }
            }
            break;
        case 2: // Employees
            if (ui_employee && employeePage) {
                ui_employee->retranslateUi(employeePage);
                retranslateDynamicWidgets(employeePage);
                retranslateDynamicTabs(ui_employee->tabWidget);
                retranslateDynamicCharts(employeePage);
            }
            break;
        case 3: // Clients
            if (ui_client && clientPage) {
                ui_client->retranslateUi(clientPage);
                retranslateDynamicWidgets(clientPage);
                retranslateDynamicTabs(ui_client->tabWidget);
                retranslateDynamicCharts(clientPage);
            }
            break;
        case 4: // Suppliers
            if (ui_supplier && supplierPage) {
                ui_supplier->retranslateUi(supplierPage);
                retranslateDynamicWidgets(supplierPage);
                retranslateDynamicTabs(ui_supplier->tabWidget);
                retranslateDynamicCharts(supplierPage);
            }
            break;
        case 5: // Equipment
            if (ui_equipment && equipmentPage) {
                ui_equipment->retranslateUi(equipmentPage);
                retranslateDynamicWidgets(equipmentPage);
                retranslateDynamicTabs(ui_equipment->tabWidget);
                retranslateDynamicCharts(equipmentPage);
            }
            break;
        case 6: // Orders
            if (ui_order && orderPage) {
                ui_order->retranslateUi(orderPage);
                retranslateDynamicWidgets(orderPage);
                retranslateDynamicTabs(ui_order->tabWidget);
                retranslateDynamicCharts(orderPage);
            }
            break;
    }
}

// =============================================================================
// CLIENT MANAGEMENT CRUD
// =============================================================================

void MainWindow::onClientRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT CLIENT_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
        " FROM CLIENTS ORDER BY CLIENT_ID"
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load clients:\n" + model->lastError().text());
        return;
    }
    ui_client->tableView->setModel(model);
    ui_client->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui_client->tableView->horizontalHeader()->setMinimumSectionSize(120);
    ui_client->tableView->horizontalHeader()->setStretchLastSection(true);
    ui_client->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_client->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_client->tableView->verticalHeader()->setDefaultSectionSize(55);
    
    // Modern Glassmorphism Design
    ui_client->tableView->setStyleSheet(
        "QTableView {"
        "  background-color: rgba(15, 12, 8, 0.85);"
        "  border: 1px solid rgba(212, 175, 55, 0.3);"
        "  border-radius: 20px;"
        "  gridline-color: rgba(212, 175, 55, 0.05);"
        "  color: #F0E6D2;"
        "  font-family: 'Outfit', 'Segoe UI';"
        "  font-size: 13px;"
        "  selection-background-color: rgba(212, 175, 55, 0.25);"
        "  selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(40, 32, 20, 0.9);"
        "  color: #D4AF37;"
        "  padding: 15px;"
        "  border-bottom: 2px solid #D4AF37;"
        "  border-right: 1px solid rgba(212, 175, 55, 0.1);"
        "  font-weight: 800;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
        "QTableView::item {"
        "  padding: 12px;"
        "  border-bottom: 1px solid rgba(212, 175, 0, 0.03);"
        "}"
        "QScrollBar:vertical {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }"
        "QScrollBar:horizontal {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  height: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-width: 20px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: none; background: none; }"
    );
}

void MainWindow::onClientAdd()
{
    // Calculate auto-increment ID (MEX)
    int clientId = 1;
    QSqlQuery qMex("SELECT CLIENT_ID FROM CLIENTS ORDER BY CLIENT_ID ASC");
    while(qMex.next()) {
        if(qMex.value(0).toInt() == clientId) {
            clientId++;
        } else if(qMex.value(0).toInt() > clientId) {
            break;
        }
    }
    QString id = QString::number(clientId);

    QString nom     = ui_client->le_nom->text().trimmed();
    QString prenom  = ui_client->le_prenom->text().trimmed();
    QString adresse = ui_client->le_adresse->text().trimmed();
    QString tel     = ui_client->le_tel->text().trimmed();
    QString email   = ui_client->le_email->text().trimmed();

    if (!ui_client->rb_homme->isChecked() && !ui_client->rb_femme->isChecked()) {
        QMessageBox::warning(this, "Validation", "Please select a gender (Male or Female).");
        return;
    }
    QString gender  = ui_client->rb_homme->isChecked() ? "Male" : "Female";

    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Last Name and First Name are required.");
        return;
    }
    if (nom.length() < 2 || nom.length() > 12 || prenom.length() < 2 || prenom.length() > 12) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] First and Last Name must be between 2 and 12 letters.");
        return;
    }

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] Invalid email structure detected. Requires standard format.");
        return;
    }

    QSqlQuery q;
    q.prepare("INSERT INTO CLIENTS (CLIENT_ID, LAST_NAME, FIRST_NAME, ADDRESS, PHONE_NUMBER, EMAIL, GENDER,"
              " REGISTRATION_DATE, STATUS, ASSIGNED_EMPLOYEE_ID, ACCOUNT_BALANCE)"
              " VALUES (:id, :nom, :prenom, :addr, :tel, :email, :gender,"
              " SYSDATE, 'Active', :empid, 0)");
    q.bindValue(":id",     clientId);
    q.bindValue(":nom",    nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":addr",   adresse);
    q.bindValue(":tel",    tel);
    q.bindValue(":email",  email);
    q.bindValue(":gender", gender);
    q.bindValue(":empid",  currentEmployeeId);

    if (q.exec()) {
        QMessageBox::information(this, "Success", "Client added successfully.");
        logActivity("Added new client: " + prenom + " " + nom + " (ID: " + id + ")", "Clients");
        onClientClearFields();
        onClientRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to add client:\n" + q.lastError().text());
    }
}

void MainWindow::onClientModify()
{
    QString id      = ui_client->le_id_mod->text().trimmed();
    QString nom     = ui_client->le_nom_mod->text().trimmed();
    QString prenom  = ui_client->le_prenom_mod->text().trimmed();
    QString adresse = ui_client->le_adresse_mod->text().trimmed();
    QString tel     = ui_client->le_tel_mod->text().trimmed();
    QString email   = ui_client->le_email_mod->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Please enter the Client ID to modify.");
        return;
    }

    if (nom.length() < 2 || nom.length() > 12 || prenom.length() < 2 || prenom.length() > 12) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] First and Last Name must be between 2 and 12 letters.");
        return;
    }

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] Invalid email structure detected. Requires standard format.");
        return;
    }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
    checkQuery.bindValue(":id", id.toInt());
    if (checkQuery.exec() && checkQuery.next()) {
        if (checkQuery.value(0).toInt() == 0) {
            QMessageBox::warning(this, "Not Found", "ERROR: The specified Client ID does not exist in the database.");
            return;
        }
    }

    QSqlQuery q;
    QString gender = ui_client->rb_homme_mod->isChecked() ? "Male" : "Female";

    q.prepare("UPDATE CLIENTS SET LAST_NAME=:nom, FIRST_NAME=:prenom, ADDRESS=:addr,"
              " PHONE_NUMBER=:tel, EMAIL=:email, GENDER=:gender WHERE CLIENT_ID=:id");
    q.bindValue(":id",     id.toInt());
    q.bindValue(":nom",    nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":addr",   adresse);
    q.bindValue(":tel",    tel);
    q.bindValue(":email",  email);
    q.bindValue(":gender", gender);

    if (q.exec()) {
        if (q.numRowsAffected() > 0) {
            QMessageBox::information(this, "Success", "Client updated successfully.");
            logActivity("Modified client: " + prenom + " " + nom + " (ID: " + id + ")", "Clients");
            onClientModClearFields();
            onClientRefreshView();
        } else {
            QMessageBox::warning(this, "Not Found", "No client found with that ID.");
        }
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update client:\n" + q.lastError().text());
    }
}

void MainWindow::onClientDelete()
{
    // Get selected row from tableView
    QModelIndex idx = ui_client->tableView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select a client from the list to delete.");
        return;
    }
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_client->tableView->model());
    if (!model) return;
    QString clientId = model->data(model->index(idx.row(), 0)).toString();
    QString name     = model->data(model->index(idx.row(), 1)).toString()
                     + " " + model->data(model->index(idx.row(), 2)).toString();

    int ret = QMessageBox::question(this, "Confirm Delete",
        "Delete client: " + name + " (ID: " + clientId + ")?",
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    QSqlQuery q;
    q.prepare("DELETE FROM CLIENTS WHERE CLIENT_ID = :id");
    q.bindValue(":id", clientId.toInt());
    if (q.exec()) {
        QMessageBox::information(this, "Deleted", "Client deleted successfully.");
        logActivity("Deleted client: " + name + " (ID: " + clientId + ")", "Clients");
        onClientRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to delete client:\n" + q.lastError().text());
    }
}

void MainWindow::onClientSearch()
{
    QString search = ui_client->le_recherche->text().trimmed();
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (search.isEmpty()) {
        model->setQuery(
            "SELECT CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS ORDER BY CLIENT_ID"
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS WHERE UPPER(LAST_NAME) LIKE :s OR UPPER(FIRST_NAME) LIKE :s"
            " OR UPPER(EMAIL) LIKE :s OR CAST(CLIENT_ID AS VARCHAR2(20)) LIKE :s"
            " ORDER BY CLIENT_ID"
        );
        q.bindValue(":s", "%" + search.toUpper() + "%");
        q.exec();
        model->setQuery(std::move(q));
    }
    ui_client->tableView->setModel(model);
    ui_client->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui_client->tableView->horizontalHeader()->setMinimumSectionSize(120);
    ui_client->tableView->horizontalHeader()->setStretchLastSection(true);
    ui_client->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_client->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_client->tableView->verticalHeader()->setDefaultSectionSize(55);
    
    // Modern Glassmorphism Design
    ui_client->tableView->setStyleSheet(
        "QTableView {"
        "  background-color: rgba(15, 12, 8, 0.85);"
        "  border: 1px solid rgba(212, 175, 55, 0.3);"
        "  border-radius: 20px;"
        "  gridline-color: rgba(212, 175, 55, 0.05);"
        "  color: #F0E6D2;"
        "  font-family: 'Outfit', 'Segoe UI';"
        "  font-size: 13px;"
        "  selection-background-color: rgba(212, 175, 55, 0.25);"
        "  selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(40, 32, 20, 0.9);"
        "  color: #D4AF37;"
        "  padding: 15px;"
        "  border-bottom: 2px solid #D4AF37;"
        "  border-right: 1px solid rgba(212, 175, 55, 0.1);"
        "  font-weight: 800;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
        "QTableView::item {"
        "  padding: 12px;"
        "  border-bottom: 1px solid rgba(212, 175, 0, 0.03);"
        "}"
        "QScrollBar:vertical {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }"
        "QScrollBar:horizontal {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  height: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-width: 20px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: none; background: none; }"
    );
}

void MainWindow::onClientRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_client->tableView->model());
    if (!model) return;
    int row = index.row();

    // ALWAYS prepopulate the modify fields
    ui_client->le_id_mod->setText(model->data(model->index(row, 0)).toString());
    ui_client->le_nom_mod->setText(model->data(model->index(row, 1)).toString());
    ui_client->le_prenom_mod->setText(model->data(model->index(row, 2)).toString());
    ui_client->le_adresse_mod->setText(model->data(model->index(row, 3)).toString());
    ui_client->le_tel_mod->setText(model->data(model->index(row, 4)).toString());
    ui_client->le_email_mod->setText(model->data(model->index(row, 5)).toString());
    QString gender = model->data(model->index(row, 6)).toString();
    if (gender == "Male") {
        ui_client->rb_homme_mod->setChecked(true);
    } else if (gender == "Female") {
        ui_client->rb_femme_mod->setChecked(true);
    }
}

void MainWindow::toggleEmployeeFields(bool active)
{
    if (!ui_employee) return;
    ui_employee->le_nom->setEnabled(active);
    ui_employee->le_prenom->setEnabled(active);
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) cb->setEnabled(active);
    ui_employee->le_mdp->setEnabled(active);
    ui_employee->le_email->setEnabled(active);
    ui_employee->le_num->setEnabled(active);
    ui_employee->dsb_salaire->setEnabled(active);
    ui_employee->de_birthdate->setEnabled(active);
    ui_employee->btn_upload_avatar->setEnabled(active);
    ui_employee->btn_scan_face->setEnabled(active);
    ui_employee->btn_suggest_salary->setEnabled(active);
    
    QString style = active ? "" : "background: rgba(0,0,0,0.1); color: rgba(255,255,255,0.2);";
    ui_employee->le_nom->setStyleSheet(style);
    ui_employee->le_prenom->setStyleSheet(style);
    ui_employee->le_fonction->setStyleSheet(style);
    ui_employee->le_mdp->setStyleSheet(style);
    ui_employee->le_email->setStyleSheet(style);
    ui_employee->le_num->setStyleSheet(style);
}

void MainWindow::onEmployeeClearFields()
{
    ui_employee->le_id->clear();
    ui_employee->le_nom->clear();
    ui_employee->le_prenom->clear();
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) cb->setCurrentIndex(0);
    ui_employee->le_email->clear();
    ui_employee->le_num->clear();
    ui_employee->le_mdp->clear();
    ui_employee->dsb_salaire->setValue(0.0);
    ui_employee->de_birthdate->setDate(QDate(1995, 1, 1));
    ui_employee->lbl_avatar->setPixmap(QPixmap(":/assets/default_avatar.png").scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    toggleEmployeeFields(false); // Lock fields on clear
}

void MainWindow::onEmployeeRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
    if (!model) return;
    int row = index.row();
    // Col order: Action, Delete, ID, Last Name, First Name, Job Title, Age, Email, Phone
    ui_employee->le_id->setText(model->data(model->index(row, 2)).toString());
    toggleEmployeeFields(true); // Unlock fields on selection
    ui_employee->le_nom->setText(model->data(model->index(row, 3)).toString());
    ui_employee->le_prenom->setText(model->data(model->index(row, 4)).toString());
    QString jt = model->data(model->index(row, 5)).toString();
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) {
        // Match case-insensitively since DB values may not match combo capitalization exactly.
        bool matched = false;
        for (int i = 0; i < cb->count(); ++i) {
            if (cb->itemText(i).trimmed().compare(jt.trimmed(), Qt::CaseInsensitive) == 0) {
                cb->setCurrentIndex(i);
                matched = true;
                break;
            }
        }
        if (!matched) {
            // Keep selection safe: default to first option.
            cb->setCurrentIndex(0);
        }
    }
    int age = model->data(model->index(row, 6)).toInt();
    ui_employee->de_birthdate->setDate(QDate::currentDate().addYears(-age));
    ui_employee->le_email->setText(model->data(model->index(row, 7)).toString());
    ui_employee->le_num->setText(model->data(model->index(row, 8)).toString());

    // Load AVATAR only (employee_[ID].png) - face scan images are NEVER shown here
    QString idStr = model->data(model->index(row, 2)).toString();
    QString avPath = QString("assets/av/employee_%1.png").arg(idStr);

    if (QFile::exists(avPath)) {
        QPixmap pix(avPath);
        ui_employee->lbl_avatar->setPixmap(getCircularPixmap(pix).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // No custom avatar - show placeholder text, NOT the face scan image
        ui_employee->lbl_avatar->setPixmap(QPixmap());
        ui_employee->lbl_avatar->setText("No Avatar");
    }

    // Pre-fill mail tab
    ui_employee->le_mail_to->setText(model->data(model->index(row, 7)).toString());
}

// =============================================================================
// EMPLOYEE MANAGEMENT CRUD
// =============================================================================

void MainWindow::onEmployeeRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\""
        " FROM EMPLOYEES ORDER BY EMPLOYEE_ID"
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load employees:\n" + model->lastError().text());
        return;
    }
    ui_employee->tableView_employes->setModel(model);
    ui_employee->tableView_employes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_employee->tableView_employes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_employee->tableView_employes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_employee->tableView_employes->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Summary Stats Calculation
    QSqlQuery q;
    if (q.exec("SELECT COUNT(*), AVG(SALARY), AVG(AGE) FROM EMPLOYEES")) {
        if (q.next()) {
            int total = q.value(0).toInt();
            double avgSalary = q.value(1).toDouble();
            double avgAge = q.value(2).toDouble();

            ui_employee->lbl_stat_total->setText(QString("Total Personnel: %1").arg(total));
            ui_employee->lbl_stat_avg_salary->setText(QString("Avg Market Value: $%1").arg(avgSalary, 0, 'f', 0));
            ui_employee->lbl_stat_avg_age->setText(QString("Avg Team Age: %1").arg(avgAge, 0, 'f', 1));
        }
    }

    // Keep the stats dashboard in sync with current employee data.
    setupEmployeeStats();
}

void MainWindow::onEmployeeSearch()
{
    QString search = ui_employee->le_recherche_emp->text().trimmed();
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (search.isEmpty()) {
        model->setQuery(
            "SELECT EMPLOYEE_ID AS \"ID\", LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\""
            " FROM EMPLOYEES ORDER BY EMPLOYEE_ID"
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\""
            " FROM EMPLOYEES WHERE UPPER(LAST_NAME) LIKE :s OR UPPER(FIRST_NAME) LIKE :s"
            " OR UPPER(EMAIL) LIKE :s OR UPPER(JOB_TITLE) LIKE :s OR CAST(EMPLOYEE_ID AS VARCHAR2(20)) LIKE :s"
            " ORDER BY EMPLOYEE_ID"
        );
        q.bindValue(":s", "%" + search.toUpper() + "%");
        q.exec();
        model->setQuery(std::move(q));
    }
    ui_employee->tableView_employes->setModel(model);
}

void MainWindow::onEmployeeRefreshHistory()
{
    // Populate module filter combo if empty (except first item)
    if (ui_employee->cb_history_filter->count() <= 1) {
        QSignalBlocker blocker(ui_employee->cb_history_filter);
        ui_employee->cb_history_filter->clear();
        // Keep the label consistent with the UI default to avoid mismatch bugs.
        ui_employee->cb_history_filter->addItem("All Personnel");
        ui_employee->cb_history_filter->addItem("Employees");
        ui_employee->cb_history_filter->addItem("Clients");
        ui_employee->cb_history_filter->addItem("Equipment");
        ui_employee->cb_history_filter->addItem("Orders");
        ui_employee->cb_history_filter->addItem("General");
    }

    QString searchText = ui_employee->le_history_search->text().trimmed().toUpper();
    QString moduleFilter = ui_employee->cb_history_filter->currentText();

    const bool isAllModule = moduleFilter.toLower().startsWith("all");

    // Read audit/history from local JSON file (no DB tables).
    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray auditArray;
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isArray()) auditArray = doc.array();
            file.close();
        }
    }

    QVector<QJsonObject> filtered;
    filtered.reserve(auditArray.size());

    for (const QJsonValue &v : auditArray) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        const QString emp    = o.value("employee_name").toString();
        const QString action = o.value("action_details").toString();
        const QString mod    = o.value("module_name").toString();

        if (!isAllModule && !moduleFilter.isEmpty()) {
            if (mod != moduleFilter) continue;
        }

        if (!searchText.isEmpty()) {
            const QString empUp = emp.toUpper();
            const QString actionUp = action.toUpper();
            if (!empUp.contains(searchText) && !actionUp.contains(searchText)) continue;
        }

        filtered.push_back(o);
    }

    std::sort(filtered.begin(), filtered.end(), [](const QJsonObject &a, const QJsonObject &b) {
        const qint64 at = a.value("timestamp_ms").toVariant().toLongLong();
        const qint64 bt = b.value("timestamp_ms").toVariant().toLongLong();
        return bt < at; // descending
    });

    const int maxRows = 250;
    const int rowCount = qMin(filtered.size(), maxRows);
    QStandardItemModel *model = new QStandardItemModel(rowCount, 4, this);
    model->setHorizontalHeaderLabels({"Time", "Employee", "Action", "Module"});

    for (int r = 0; r < rowCount; ++r) {
        const QJsonObject o = filtered.at(r);
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(o.value("timestamp_ms").toVariant().toLongLong());
        if (!dt.isValid()) dt = QDateTime::fromString(o.value("timestamp_iso").toString(), Qt::ISODate);
        const QString timeStr = dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : QString();

        model->setItem(r, 0, new QStandardItem(timeStr));
        model->setItem(r, 1, new QStandardItem(o.value("employee_name").toString()));
        model->setItem(r, 2, new QStandardItem(o.value("action_details").toString()));
        model->setItem(r, 3, new QStandardItem(o.value("module_name").toString()));
    }

    ui_employee->tableView_historique_emp->setModel(model);
    ui_employee->tableView_historique_emp->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_employee->tableView_historique_emp->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_employee->tableView_historique_emp->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_employee->tableView_historique_emp->verticalHeader()->setDefaultSectionSize(55);
    
    // Modern Glassmorphism Design
    ui_employee->tableView_historique_emp->setStyleSheet(
        "QTableView {"
        "  background-color: rgba(15, 12, 8, 0.85);"
        "  border: 1px solid rgba(212, 175, 55, 0.3);"
        "  border-radius: 20px;"
        "  gridline-color: rgba(212, 175, 55, 0.05);"
        "  color: #F0E6D2;"
        "  font-family: 'Outfit', 'Segoe UI';"
        "  font-size: 13px;"
        "  selection-background-color: rgba(212, 175, 55, 0.25);"
        "  selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(40, 32, 20, 0.9);"
        "  color: #D4AF37;"
        "  padding: 15px;"
        "  border-bottom: 2px solid #D4AF37;"
        "  border-right: 1px solid rgba(212, 175, 55, 0.1);"
        "  font-weight: 800;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
        "QTableView::item {"
        "  padding: 12px;"
        "  border-bottom: 1px solid rgba(212, 175, 0, 0.03);"
        "}"
    );

    if (qobject_cast<QPushButton*>(sender()) == ui_employee->btn_refresh_history) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_employee->tableView_historique_emp);
        ui_employee->tableView_historique_emp->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
        a->setDuration(400); a->setStartValue(0.0); a->setEndValue(1.0); a->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::onEmployeeHistorySearch()
{
    onEmployeeRefreshHistory();
}

void MainWindow::onEmployeeSendMail()
{
    QString to = ui_employee->le_mail_to->text().trimmed();
    QString subject = ui_employee->le_mail_subject->text().trimmed();
    QString body = ui_employee->te_mail_body->toPlainText().trimmed();

    if (to.isEmpty()) {
        QMessageBox::warning(this, "Mail", "Please select an employee with a valid email first.");
        return;
    }

    QString mailto = QString("mailto:%1?subject=%2&body=%3")
                        .arg(to)
                        .arg(QUrl::toPercentEncoding(subject).data())
                        .arg(QUrl::toPercentEncoding(body).data());
    
    if (QDesktopServices::openUrl(QUrl(mailto))) {
        QMessageBox::information(this, "Mail", "Default mail client opened.");
    } else {
        QMessageBox::critical(this, "Mail", "Failed to open default mail client.");
    }
}

void MainWindow::onEmployeeMailTemplateChanged(int index)
{
    QString name = ui_employee->le_prenom->text() + " " + ui_employee->le_nom->text();
    if (name.trimmed().isEmpty()) name = "Employee";

    switch (index) {
        case 1: // Welcome
            ui_employee->le_mail_subject->setText("Welcome to the Team!");
            ui_employee->te_mail_body->setPlainText(QString("Dear %1,\n\nWelcome to HammerDown! We are excited to have you join our team. Your account has been setup and you can now log in.\n\nBest regards,\nManagement").arg(name));
            break;
        case 2: // Task Assignment
            ui_employee->le_mail_subject->setText("New Task Assignment");
            ui_employee->te_mail_body->setPlainText(QString("Hi %1,\n\nYou have been assigned a new task. Please check your dashboard for details.\n\nDeadline: ASAP\n\nThanks,\nTeam Lead").arg(name));
            break;
        case 3: // Meeting
            ui_employee->le_mail_subject->setText("Meeting Request");
            ui_employee->te_mail_body->setPlainText(QString("Hello %1,\n\nI would like to schedule a brief meeting to discuss your recent performance and future goals.\n\nPlease let me know your availability.\n\nRegards,\nHR").arg(name));
            break;
        default:
            ui_employee->le_mail_subject->clear();
            ui_employee->te_mail_body->clear();
            break;
    }
}

void MainWindow::onEmployeeExportHistoryPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Audit Log", 
        QDir::homePath() + "/Audit_Log_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    // Use PrinterResolution for more predictable coordinate mapping (less tiny fonts)
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);
    printer.setFullPage(false);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export", "Failed to initialize PDF printer.");
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing);

    int pageWidth = painter.viewport().width();
    int y = 60;

    // Header Stylistics
    painter.setFont(QFont("Outfit", 22, QFont::Bold));
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(0, y, pageWidth, 50, Qt::AlignCenter, "HAMMERDOWN - SYSTEM SECURITY AUDIT");
    y += 65;

    painter.setFont(QFont("Outfit", 12));
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(0, y, pageWidth, 30, Qt::AlignCenter, "Generated on " + QDateTime::currentDateTime().toString("dd MMMM yyyy - HH:mm:ss"));
    y += 100;

    QAbstractItemModel *model = ui_employee->tableView_historique_emp->model();
    if (!model) {
        painter.end();
        return;
    }

    // Column Ratio Configuration (Sync with History Model: Time, Employee, Action, Module)
    int cWidths[4];
    cWidths[0] = pageWidth * 0.15; // Time
    cWidths[1] = pageWidth * 0.15; // Employee
    cWidths[2] = pageWidth * 0.55; // Action (Main content)
    cWidths[3] = pageWidth * 0.15; // Module
    
    // Header Table
    painter.setFont(QFont("Outfit", 11, QFont::Bold));
    painter.setPen(Qt::white);
    painter.setBrush(QColor(139, 111, 71));
    
    int currentX = 0;
    for (int c = 0; c < 4; ++c) {
        painter.drawRect(currentX, y, cWidths[c], 40);
        painter.drawText(currentX + 5, y, cWidths[c] - 10, 40, Qt::AlignCenter, model->headerData(c, Qt::Horizontal).toString());
        currentX += cWidths[c];
    }
    y += 40;

    // Content Rows with Dynamic Wrapping support
    painter.setFont(QFont("Outfit", 10));
    painter.setPen(Qt::black);
    
    for (int r = 0; r < model->rowCount(); ++r) {
        // Calculate required row height based on 'Action' column length
        QString actionText = model->data(model->index(r, 2)).toString();
        QRect textRect = painter.boundingRect(0, 0, cWidths[2] - 10, 9999, Qt::TextWordWrap, actionText);
        int rowH = qMax(35, textRect.height() + 15);

        // Page Break Logic
        if (y + rowH > painter.viewport().height() - 80) {
            printer.newPage();
            y = 80;
            // Draw Sub-header on new page
            painter.setFont(QFont("Outfit", 11, QFont::Bold));
            painter.setPen(Qt::white);
            painter.setBrush(QColor(139, 111, 71));
            int subX = 0;
            for (int c = 0; c < 4; ++c) {
                painter.drawRect(subX, y, cWidths[c], 35);
                painter.drawText(subX + 5, y, cWidths[c] - 10, 35, Qt::AlignCenter, model->headerData(c, Qt::Horizontal).toString());
                subX += cWidths[c];
            }
            y += 35;
            painter.setFont(QFont("Outfit", 10));
            painter.setPen(Qt::black);
        }

        currentX = 0;
        for (int c = 0; c < 4; ++c) {
            QString content = model->data(model->index(r, c)).toString();
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QColor(240, 240, 240));
            painter.drawRect(currentX, y, cWidths[c], rowH);
            
            painter.setPen(Qt::black);
            // Action column gets word wrap, others use standard align
            Qt::Alignment flags = (c == 2)
                ? Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap)
                : Qt::Alignment(Qt::AlignCenter);
            painter.drawText(currentX + 5, y + 5, cWidths[c] - 10, rowH - 10, flags, content);
            
            currentX += cWidths[c];
        }
        y += rowH;
    }

    painter.end();
    QMessageBox::information(this, "Security Audit", "The system audit log has been fully synchronized and exported to PDF format successfully.");
}

void MainWindow::onEmployeeExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Employee List", 
        QDir::homePath() + "/Staff_Directory_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export", "Failed to initialize PDF printer.");
        return;
    }

    int pageWidth = painter.viewport().width();
    int y = 50;

    // Header styling
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71)); // Professional brown color
    painter.drawText(0, y, pageWidth, 40, Qt::AlignCenter, "HammerDown - Professional Staff Directory");
    y += 80;

    // Calculate dynamic columns [ID, Name, Job, Email, Phone]
    int x_id = 40;
    int x_name = pageWidth * 0.12;
    int x_job = pageWidth * 0.38;
    int x_email = pageWidth * 0.62;
    int x_phone = pageWidth * 0.88;

    // Table Header Background
    painter.setBrush(QColor(245, 245, 245));
    painter.setPen(Qt::NoPen);
    painter.drawRect(20, y - 5, pageWidth - 40, 35);

    // Table Header Text
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(x_id, y, "ID");
    painter.drawText(x_name, y, "Employee Name");
    painter.drawText(x_job, y, "Job Title & Dept");
    painter.drawText(x_email, y, "Contact Email");
    painter.drawText(x_phone, y, "Phone");
    
    painter.setPen(QColor(200, 200, 200));
    painter.drawLine(20, y + 30, pageWidth - 20, y + 30);
    y += 60;

    // Data Row Logic
    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(Qt::black);
    
    QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME || ' ' || LAST_NAME, JOB_TITLE, EMAIL, PHONE_NUMBER FROM EMPLOYEES ORDER BY EMPLOYEE_ID");
    while (q.next()) {
        if (y > printer.height() - 80) { // New page margin
            printer.newPage();
            y = 50;
        }
        painter.drawText(x_id, y, q.value(0).toString());
        painter.drawText(x_name, y, q.value(1).toString());
        painter.drawText(x_job, y, q.value(2).toString());
        painter.drawText(x_email, y, q.value(3).toString());
        painter.drawText(x_phone, y, q.value(4).toString());
        y += 35; // Row height
    }
    
    painter.end();
    QMessageBox::information(this, "Success", "Staff directory exported successfully to:\n" + fileName);
}


void MainWindow::onAIPulseClicked()
{
    ui_employee->lbl_ai_pulse_result->setVisible(true);
    ui_employee->lbl_ai_pulse_result->setText("📡 SYNCING WITH GLOBAL MARKET NETWORKS... CORE ANALYSIS ENGAGED.");
    
    QTimer::singleShot(1200, this, [this](){
        QSqlQuery qEmp("SELECT COUNT(*), AVG(SALARY), AVG(AGE), COUNT(DISTINCT JOB_TITLE) FROM EMPLOYEES");
        qEmp.next();
        int cEmp = qEmp.value(0).toInt();
        double avgS = qEmp.value(1).toDouble();
        double avgAge = qEmp.value(2).toDouble();
        int distinctRoles = qEmp.value(3).toInt();

        QSqlQuery qOrd("SELECT COUNT(*) FROM ORDERS");
        qOrd.next();
        int cOrd = qOrd.value(0).toInt();

        double throughput = (cOrd > 0) ? (double)cOrd / qMax(1, cEmp) : 0.0;
        double marketIndex = (avgS > 0.0) ? (avgS / 4500.0) : 0.0; // Benchmark against ~4.5k
        double bufferPct = qMin(100.0, (throughput / 3.0) * 100.0);
        double burnPct = qMax(0.0, qMin(100.0, marketIndex * 100.0));

        QString efficiencyColor = throughput > 2.0 ? "#4CAF50" : (throughput > 1.0 ? "#D4AF37" : "#FF5252");
        QString stabilityColor   = burnPct > 66.0 ? "#4CAF50" : (burnPct > 33.0 ? "#D4AF37" : "#FF5252");

        // Top 3 roles for quick planning
        QString topRoles;
        {
            QSqlQuery qRoles("SELECT JOB_TITLE, COUNT(*) FROM EMPLOYEES GROUP BY JOB_TITLE ORDER BY COUNT(*) DESC FETCH FIRST 3 ROWS ONLY");
            int i = 0;
            while (qRoles.next() && i < 3) {
                const QString role = qRoles.value(0).toString();
                const int cnt = qRoles.value(1).toInt();
                topRoles += (i == 0 ? "" : ", ");
                topRoles += QString("%1(%2)").arg(role).arg(cnt);
                ++i;
            }
        }

        // Recent employee actions from local audit JSON
        QString recentOps;
        {
            const QString filePath = "hammerdown_audit_log.json";
            QJsonArray arr;
            QFile f(filePath);
            if (f.open(QIODevice::ReadOnly)) {
                const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                if (doc.isArray()) arr = doc.array();
                f.close();
            }

            QVector<QJsonObject> objs;
            objs.reserve(arr.size());
            for (const QJsonValue &v : arr) if (v.isObject()) objs.push_back(v.toObject());

            std::sort(objs.begin(), objs.end(), [](const QJsonObject &a, const QJsonObject &b) {
                return b.value("timestamp_ms").toVariant().toLongLong() < a.value("timestamp_ms").toVariant().toLongLong();
            });

            for (int r = 0; r < objs.size() && r < 3; ++r) {
                const QJsonObject o = objs.at(r);
                const QString emp = o.value("employee_name").toString();
                const QString act = o.value("action_details").toString();
                const QString mod = o.value("module_name").toString();
                recentOps += QString("<br/>• [%1] %2: %3").arg(mod, emp, act);
            }
            if (objs.isEmpty()) recentOps = "<br/>• No local audit entries yet.";
        }

        QString advisory;
        if (cEmp <= 0) {
            advisory = "SYSTEM IDLE. Add employees to initialize workforce analytics.";
        } else if (distinctRoles < 3) {
            advisory = "ROLE COVERAGE LOW. Consider adding staff across more job titles for resilience.";
        } else if (throughput < 1.0) {
            advisory = "THROUGHPUT LOW. Optimize workload distribution and consider short-term hires.";
        } else {
            advisory = "STABILITY STRONG. Maintain staffing alignment and monitor burnout risk.";
        }

        QString insight = QString(
            "<b style='color:#D4AF37;'>⚡ SYSTEM VITALITY REPORT</b><br/>"
            "<span style='color:%1;'>▶ Efficiency Throughput: %2 orders/capita</span><br/>"
            "▶ Operational Buffer: %3% capacity utilized.<br/>"
            "<span style='color:%4;'>▶ Resource Burn: Stability at %5% (Market Relative)</span><br/>"
            "▶ Team Snapshot: %6 employees • Avg age %7 • Roles %8<br/>"
            "▶ Top Roles: %9<br/>"
            "<br/><i style='color:#AAA;'>MANAGEMENT ADVISORY: %10</i>"
            "<br/><br/><b style='color:#D4AF37;'>Recent Ops (Audit JSON)</b>%11"
        )
            .arg(efficiencyColor)
            .arg(throughput, 0, 'f', 2)
            .arg(bufferPct, 0, 'f', 1)
            .arg(stabilityColor)
            .arg(burnPct, 0, 'f', 1)
            .arg(cEmp)
            .arg(avgAge, 0, 'f', 1)
            .arg(distinctRoles)
            .arg(topRoles)
            .arg(advisory)
            .arg(recentOps);

        ui_employee->lbl_ai_pulse_result->setText(insight);
    });
}

void MainWindow::onStatsAiClicked()
{
    if (!ui_employee) return;

    ui_employee->lbl_stats_ai_insight->setVisible(true);
    ui_employee->lbl_stats_ai_insight->setText("✨ Generating AI 3D workforce insights...");

    // Populate the stats area with your existing “3D-styled” charts.
    // (setupEmployeeStats() rebuilds the grid panel each call.)
    setupEmployeeStats();

    QTimer::singleShot(650, this, [this](){
        QSqlQuery q("SELECT COUNT(*), AVG(AGE), AVG(SALARY), MAX(SALARY), COUNT(DISTINCT JOB_TITLE) FROM EMPLOYEES");
        if (!q.next()) return;

        const int count = q.value(0).toInt();
        const double avgAge = q.value(1).toDouble();
        const double avgSalary = q.value(2).toDouble();
        const double maxSalary = q.value(3).toDouble();
        const int distinctRoles = q.value(4).toInt();

        QString synergy;
        if (distinctRoles >= 5 && avgAge < 40) {
            synergy = "DIVERSE AGILE TEAM: Strong coverage + fast adaptation. Resilience looks high.";
        } else if (distinctRoles <= 2) {
            synergy = "SPECIALIZED TASK-FORCE: Deep expertise in few domains. Watch for skill-gap risk.";
        } else {
            synergy = "BALANCED FLEET: Solid coverage with healthy knowledge sharing.";
        }

        const QString advice =
            (maxSalary > 8000)
                ? "Retention plan: protect high-tier talent to avoid knowledge loss."
                : "Incentive plan: scale rewards to pull senior-level roles (Smith/Boss).";

        const QString viewNote =
            QString("3D charts refreshed (Workforce Sector + Synergy Index).");

        ui_employee->lbl_stats_ai_insight->setText(
            QString("✨ SYNERGY INSIGHT [%1 Personnel]\n%2\n\n"
                    "📌 Avg Age: %3 • Avg Salary: $%4 • Distinct Roles: %5\n"
                    "💡 STRATEGIC ADVICE: %6\n\n%7")
                .arg(count)
                .arg(synergy)
                .arg(avgAge, 0, 'f', 1)
                .arg(avgSalary, 0, 'f', 0)
                .arg(distinctRoles)
                .arg(advice)
                .arg(viewNote));
    });
}

void MainWindow::setupEmployeeStats()
{
    // Clear existing layout
    QLayoutItem *child;
    while ((child = ui_employee->gridLayout_stats->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    // Obsidian Glass helper for high-end charts
    auto makeObsidianPanel = [](QChartView *v) {
        v->setRenderHint(QPainter::Antialiasing);
        v->setStyleSheet(
            "QChartView {"
            "background-color: rgba(18, 14, 10, 0.85);"
            "border: 1px solid rgba(212, 175, 55, 0.25);"
            "border-radius: 20px;"
            "padding: 15px;"
            "}"
            "QChartView:hover {"
            "border: 1.5px solid rgba(212, 175, 55, 0.7);"
            "}"
        );
    };

    auto styleObsidianChart = [](QChart *c, const QString &title) {
        c->setTitle(title.toUpper());
        c->setTitleFont(QFont("Outfit", 13, QFont::Bold));
        c->setTitleBrush(QBrush(QColor("#D4AF37")));
        c->setBackgroundBrush(Qt::transparent);
        c->setPlotAreaBackgroundBrush(Qt::transparent);
        c->setPlotAreaBackgroundVisible(false);
        c->setMargins(QMargins(10, 10, 10, 10));
    };

    struct RoleStat {
        int count = 0;
        double avgSalary = 0.0;
        double avgAge = 0.0;
    };

    // Precompute stats per job title for interactive tooltips and role focus.
    QHash<QString, RoleStat> roleStats;
    {
        QSqlQuery qRole("SELECT JOB_TITLE, COUNT(*), AVG(SALARY), AVG(AGE) FROM EMPLOYEES GROUP BY JOB_TITLE");
        while (qRole.next()) {
            const QString role = qRole.value(0).toString();
            RoleStat rs;
            rs.count = qRole.value(1).toInt();
            rs.avgSalary = qRole.value(2).toDouble();
            rs.avgAge = qRole.value(3).toDouble();
            roleStats.insert(role, rs);
        }
    }

    // --- CHART 1: Stunning Glass Donut (Role Distribution) ---
    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->setHoleSize(0.55); 
    pieSeries->setPieSize(0.95);
    
    QSqlQuery q("SELECT JOB_TITLE, COUNT(*) FROM EMPLOYEES GROUP BY JOB_TITLE");
    int totalCount = 0;
    QHash<QString, QPieSlice*> sliceByRole;
    while (q.next()) {
        QString titleStr = q.value(0).toString();
        int count = q.value(1).toInt();
        totalCount += count;
        QPieSlice *slice = pieSeries->append(titleStr, count);
        sliceByRole.insert(titleStr, slice);
        
        QColor base = QColor::fromHsl((count * 45) % 360, 160, 140);
        slice->setBrush(QBrush(base));
        slice->setLabelVisible(false); 
        slice->setLabelColor(QColor("#F0E6D2"));
        slice->setPen(QPen(QColor("#2C2215"), 2.0));

        // Hover tooltip with useful role metrics.
        const RoleStat rs = roleStats.value(titleStr);
        const QString roleTip = QString("%1\nCount: %2\nAvg salary: $%3\nAvg age: %4")
                                    .arg(titleStr)
                                    .arg(rs.count)
                                    .arg(rs.avgSalary, 0, 'f', 0)
                                    .arg(rs.avgAge, 0, 'f', 1);

        connect(slice, &QPieSlice::hovered, this, [slice, roleTip](bool state) {
            slice->setExploded(state);
            slice->setLabelVisible(state);
            slice->setExplodeDistanceFactor(state ? 0.16 : 0.03);
            slice->setLabelFont(QFont("Outfit", 10, QFont::Bold));
            if (state) QToolTip::showText(QCursor::pos(), roleTip);
            else QToolTip::hideText();
        });
    }

    QChart *chartPie = new QChart();
    chartPie->addSeries(pieSeries);
    styleObsidianChart(chartPie, "Workforce Sector");
    chartPie->setAnimationOptions(QChart::AllAnimations);
    chartPie->legend()->setAlignment(Qt::AlignRight);
    chartPie->legend()->setMarkerShape(QLegend::MarkerShapeCircle);
    chartPie->legend()->setLabelColor(QColor("#D4C4A8"));

    QChartView *viewPie = new QChartView(chartPie);
    makeObsidianPanel(viewPie);
    // Slight shadow to enhance depth/3D feel in the stats panel.
    {
        auto *eff = new QGraphicsDropShadowEffect(viewPie);
        eff->setBlurRadius(22);
        eff->setColor(QColor(0, 0, 0, 180));
        eff->setOffset(0, 6);
        viewPie->setGraphicsEffect(eff);
    }
    viewPie->setMinimumSize(520, 320);

    // --- CHART 2: STAFF SYNERGY ALIGNMENT (3D-Styled Stacked Bar) ---
    QBarSet *performanceSet = new QBarSet("Alignment Efficiency");
    performanceSet->setColor(QColor("#D4AF37"));
    performanceSet->setBorderColor(QColor("#A0825A"));

    QStringList roles;
    // Map roles to a "Synergy Level" based on count/pay ratio
    QSqlQuery qS("SELECT JOB_TITLE, (COUNT(*)*1.5) + (AVG(SALARY)/2000) FROM EMPLOYEES GROUP BY JOB_TITLE ORDER BY JOB_TITLE LIMIT 6");
    while (qS.next()) {
        roles << qS.value(0).toString();
        *performanceSet << qS.value(1).toDouble();
    }

    QBarSeries *synergySeries = new QBarSeries();
    synergySeries->append(performanceSet);
    synergySeries->setLabelsVisible(true);
    synergySeries->setLabelsFormat("%v%");
    synergySeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    QChart *chartSynergy = new QChart();
    chartSynergy->addSeries(synergySeries);
    styleObsidianChart(chartSynergy, "Staff Synergy Alignment Index");
    chartSynergy->setAnimationOptions(QChart::AllAnimations);
    chartSynergy->legend()->hide();

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(roles);
    axisX->setLabelsColor(QColor("#D4C4A8"));
    axisX->setLabelsFont(QFont("Outfit", 8, QFont::Bold));
    axisX->setLinePenColor(QColor("#8B6F47"));
    axisX->setGridLineVisible(false);
    chartSynergy->addAxis(axisX, Qt::AlignBottom);
    synergySeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelsColor(QColor("#D4C4A8"));
    axisY->setGridLineColor(QColor(139, 111, 71, 40));
    axisY->setLinePenColor(QColor("#8B6F47"));
    chartSynergy->addAxis(axisY, Qt::AlignLeft);
    synergySeries->attachAxis(axisY);

    QChartView *viewSalary = new QChartView(chartSynergy); // Reusing viewSalary pointer name for layout compatibility
    makeObsidianPanel(viewSalary);
    {
        auto *eff = new QGraphicsDropShadowEffect(viewSalary);
        eff->setBlurRadius(22);
        eff->setColor(QColor(0, 0, 0, 180));
        eff->setOffset(0, 6);
        viewSalary->setGraphicsEffect(eff);
    }
    viewSalary->setMinimumSize(420, 220);
    
    const QStringList rolesCopy = roles;
    // Add interactive click behavior (bar -> real role stats).
    connect(synergySeries, &QBarSeries::clicked, this,
            [this, rolesCopy](int index, QBarSet *barset) {
        if (index < 0 || index >= rolesCopy.size()) return;
        const QString role = rolesCopy.at(index);

        // Role metrics (useful details)
        QSqlQuery q;
        q.prepare("SELECT COUNT(*), AVG(SALARY), AVG(AGE) FROM EMPLOYEES WHERE JOB_TITLE = :r");
        q.bindValue(":r", role);

        int count = 0;
        double avgSalary = 0.0;
        double avgAge = 0.0;
        if (q.exec() && q.next()) {
            count = q.value(0).toInt();
            avgSalary = q.value(1).toDouble();
            avgAge = q.value(2).toDouble();
        }

        const double score = (barset ? barset->at(index) : 0.0);
        ui_employee->lbl_stats_ai_insight->setText(
            QString("📌 Role Focus: %1 | Team: %2 • Avg salary: $%3 • Avg age: %4\n"
                    "Alignment Coefficient: %5")
                .arg(role)
                .arg(count)
                .arg(avgSalary, 0, 'f', 0)
                .arg(avgAge, 0, 'f', 1)
                .arg(score, 0, 'f', 2));
    });

    // --- WIDGET 3: Live Pulse Card (Dynamic Quick Facts) ---
    QFrame *framePulse = new QFrame();
    framePulse->setObjectName("frame_pulse_card");
    framePulse->setStyleSheet(
        "QFrame#frame_pulse_card {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(44, 34, 21, 0.95), stop:1 rgba(15, 12, 8, 0.98));"
        "border: 2px solid rgba(212, 175, 55, 0.4);"
        "border-radius: 20px;"
        "}"
    );
    {
        auto *eff = new QGraphicsDropShadowEffect(framePulse);
        eff->setBlurRadius(28);
        eff->setColor(QColor(0, 0, 0, 200));
        eff->setOffset(0, 8);
        framePulse->setGraphicsEffect(eff);
    }
    QVBoxLayout *pulseLayout = new QVBoxLayout(framePulse);
    pulseLayout->setContentsMargins(25, 25, 25, 25);
    pulseLayout->setSpacing(15);

    // Role focus selector (interactive + useful)
    QStringList roleKeys = roleStats.keys();
    roleKeys.sort(Qt::CaseInsensitive);
    if (roleKeys.isEmpty()) roleKeys << "N/A";

    QComboBox *cbRoleFocus = new QComboBox(framePulse);
    cbRoleFocus->setEditable(false);
    cbRoleFocus->addItems(roleKeys);
    cbRoleFocus->setStyleSheet(
        "QComboBox { background: rgba(255,255,255,0.08); border: 1px solid rgba(212,175,55,0.35); border-radius: 10px; color: #D4AF37; padding: 6px 12px; }"
        "QComboBox:hover { border-color: rgba(212,175,55,0.6); }"
    );
    pulseLayout->addWidget(cbRoleFocus);

    QLabel *lblRoleFocus = new QLabel("ROLE FOCUS: —");
    lblRoleFocus->setStyleSheet("color: #D4AF37; font-size: 12px; font-weight: bold; background: transparent;");
    pulseLayout->addWidget(lblRoleFocus);

    QLabel *lblRoleDetails = new QLabel("—");
    lblRoleDetails->setStyleSheet("color: #F0E6D2; font-size: 11px; background: transparent;");
    pulseLayout->addWidget(lblRoleDetails);

    // Capture by value: lambdas may fire after setupEmployeeStats() returns.
    const auto roleStatsCopy = roleStats;
    const auto sliceByRoleCopy = sliceByRole;
    auto updateRoleFocus = [=]() {
        const QString role = cbRoleFocus->currentText();
        const RoleStat rs = roleStatsCopy.value(role);

        lblRoleFocus->setText(QString("ROLE FOCUS: %1").arg(role));
        lblRoleDetails->setText(QString("Count: %1 | Avg salary: $%2 | Avg age: %3")
                                     .arg(rs.count)
                                     .arg(rs.avgSalary, 0, 'f', 0)
                                     .arg(rs.avgAge, 0, 'f', 1));

        // Highlight the corresponding donut slice.
        for (auto it = sliceByRoleCopy.begin(); it != sliceByRoleCopy.end(); ++it) {
            if (it.value()) it.value()->setExploded(false);
        }
        if (sliceByRoleCopy.contains(role) && sliceByRoleCopy.value(role)) {
            sliceByRoleCopy.value(role)->setExploded(true);
            sliceByRoleCopy.value(role)->setLabelVisible(true);
        }
    };
    updateRoleFocus();

    connect(cbRoleFocus, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int) { updateRoleFocus(); });
    
    auto addMetric = [&](const QString &icon, const QString &label, const QString &val) {
        QLabel *l = new QLabel(QString("<font color='#D4D4D4' size='4'>%1 %2</font><br/><font color='#D4AF37' size='6'><b>%3</b></font>").arg(icon, label, val));
        l->setStyleSheet("font-family: 'Outfit'; border: none; background: transparent;");
        pulseLayout->addWidget(l);
    };

    QSqlQuery qAvg("SELECT AVG(SALARY) FROM EMPLOYEES");
    double avgS = qAvg.next() ? qAvg.value(0).toDouble() : 0;
    
    addMetric("\xF0\x9F\x8C\x90", "TOTAL TALENT", QString::number(totalCount));
    addMetric("\xF0\x9F\x92\x8E", "AV PAYROLL", "$" + QString::number(avgS, 'f', 0));
    addMetric("\xE2\x8F\xB3", "SYSTEM STATUS", "OPTIMIZED");

    pulseLayout->addStretch();
    
    // Bigger + better layout: make the donut bigger and keep synergy + pulse on the right.
    ui_employee->gridLayout_stats->setSpacing(18);
    ui_employee->gridLayout_stats->addWidget(viewPie, 0, 0, 2, 1);
    ui_employee->gridLayout_stats->addWidget(viewSalary, 0, 1, 1, 1);
    ui_employee->gridLayout_stats->addWidget(framePulse, 1, 1, 1, 1);

    ui_employee->gridLayout_stats->setRowStretch(0, 1);
    ui_employee->gridLayout_stats->setRowStretch(1, 1);
    ui_employee->gridLayout_stats->setColumnStretch(0, 1);
    ui_employee->gridLayout_stats->setColumnStretch(1, 1);
}

void MainWindow::onEmployeeAdd()
{
    QString id      = ui_employee->le_id->text().trimmed();
    QString nom     = ui_employee->le_nom->text().trimmed();
    QString prenom  = ui_employee->le_prenom->text().trimmed();
    QString fonction = "smith";
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) fonction = cb->currentText();
    QDate   birthDate = ui_employee->de_birthdate->date();
    int     age       = birthDate.daysTo(QDate::currentDate()) / 365;
    QString mdp     = ui_employee->le_mdp->text().trimmed();
    double  salaire = ui_employee->dsb_salaire->value();
    QString email   = ui_employee->le_email->text().trimmed();
    QString num     = ui_employee->le_num->text().trimmed();

    if (id.isEmpty() || nom.isEmpty() || prenom.isEmpty() || mdp.isEmpty()) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] All core identity fields (ID, Name, Password) must be populated.");
        return;
    }

    // Controle de Saisie (Strict Input Validation)
    QRegularExpression nameRegex("^[A-Za-z\\s]+$");
    if (!nameRegex.match(nom).hasMatch() || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "NAMES must contain alpha characters only (A-Z).");
        return;
    }

    if (nom.length() < 2 || prenom.length() < 2) {
        QMessageBox::warning(this, "Validation Error", "NAMES must be at least 2 characters long.");
        return;
    }

    QRegularExpression emailRegex("^[\\w\\-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "INVALID EMAIL sequence. Please enter a valid corporate address.");
        return;
    }

    QRegularExpression phoneRegex("^\\d{8}$");
    if (!num.isEmpty() && !phoneRegex.match(num).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "PHONE NUMBER must consist of exactly 8 numeric digits.");
        return;
    }

    if (salaire < 0) {
        QMessageBox::warning(this, "Validation Error", "SALARY cannot be a negative value. Balance must be zero or higher.");
        return;
    }

    if (birthDate > QDate::currentDate().addYears(-18)) {
        QMessageBox::warning(this, "Validation Error", "AGE RESTRICTION: Employees must be at least 18 years old.");
        return;
    }
    bool idOk;
    int empId = id.toInt(&idOk);
    if (!idOk || empId <= 0) {
        QMessageBox::warning(this, "Validation", "Employee ID must be a positive number.");
        return;
    }

    QSqlQuery chk;
    chk.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    chk.bindValue(":id", empId);
    if (chk.exec() && chk.next() && chk.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Duplicate ID", "An employee with this ID already exists.");
        return;
    }

    // Proactive check for duplicate email
    if (!email.isEmpty()) {
        QSqlQuery chkEmail;
        chkEmail.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMAIL = :email");
        chkEmail.bindValue(":email", email);
        if (chkEmail.exec() && chkEmail.next() && chkEmail.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Duplicate Email", 
                "An employee with the email '" + email + "' already exists.\n"
                "Please use a unique email address.");
            return;
        }
    }

    QSqlQuery q;
    q.prepare("INSERT INTO EMPLOYEES (EMPLOYEE_ID, LAST_NAME, FIRST_NAME, JOB_TITLE, AGE, PASSWORD, SALARY, EMAIL, PHONE_NUMBER, HIRE_DATE, EMPLOYEE_STATUS)"
              " VALUES (:id, :nom, :prenom, :fonction, :age, :mdp, :salaire, :email, :num, SYSDATE, 'Active')");
    q.bindValue(":id",       empId);
    q.bindValue(":nom",      nom);
    q.bindValue(":prenom",   prenom);
    q.bindValue(":fonction", fonction);
    q.bindValue(":age",      age);
    q.bindValue(":mdp",      mdp);
    q.bindValue(":salaire",  salaire);
    q.bindValue(":email",    email);
    q.bindValue(":num",      num);

    if (q.exec()) {
        QSqlDatabase::database().commit();
        QMessageBox::information(this, "Success", "Employee added successfully.");
        logActivity("Added new employee: " + prenom + " " + nom + " (ID: " + id + ")", "Employees");
        onEmployeeClearFields();
        ui_employee->le_recherche_emp->clear();
        onEmployeeRefreshView();
        onEmployeeRefreshHistory();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to add employee:\n" + q.lastError().text());
    }
}

void MainWindow::onEmployeeModify()
{
    QString id       = ui_employee->le_id->text().trimmed();
    QString nom      = ui_employee->le_nom->text().trimmed();
    QString prenom   = ui_employee->le_prenom->text().trimmed();
    QString fonction = "smith";
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) fonction = cb->currentText();
    QDate   birthDate = ui_employee->de_birthdate->date();
    int     age       = birthDate.daysTo(QDate::currentDate()) / 365;
    QString mdp      = ui_employee->le_mdp->text().trimmed();
    double  salaire  = ui_employee->dsb_salaire->value();
    QString email    = ui_employee->le_email->text().trimmed();
    QString num      = ui_employee->le_num->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Identification Failure: Select an employee to update.");
        return;
    }

    // Controle de Saisie for update
    QRegularExpression nameRegex("^[A-Za-z\\s]+$");
    if (!nom.isEmpty() && !nameRegex.match(nom).hasMatch()) { QMessageBox::warning(this, "Validation", "LAST NAME contains invalid characters."); return; }
    if (!prenom.isEmpty() && !nameRegex.match(prenom).hasMatch()) { QMessageBox::warning(this, "Validation", "FIRST NAME contains invalid characters."); return; }

    QRegularExpression emailRegex("^[\\w\\-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) { QMessageBox::warning(this, "Validation", "Malformed EMAIL structure."); return; }

    QRegularExpression phoneRegex("^\\d{8}$");
    if (!num.isEmpty() && !phoneRegex.match(num).hasMatch()) { QMessageBox::warning(this, "Validation", "PHONE NUMBER must be 8 digits."); return; }

    if (salaire < 0) { QMessageBox::warning(this, "Validation", "Negative SALARY is not permitted."); return; }

    // Proactive check for duplicate email (excluding current employee)
    if (!email.isEmpty()) {
        QSqlQuery chkEmail;
        chkEmail.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMAIL = :email AND EMPLOYEE_ID <> :id");
        chkEmail.bindValue(":email", email);
        chkEmail.bindValue(":id", id.toInt());
        if (chkEmail.exec() && chkEmail.next() && chkEmail.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Duplicate Email",
                "The email '" + email + "' is already assigned to another employee.\n"
                "Please use a unique email address.");
            return;
        }
    }

    QSqlQuery q;
    q.prepare("UPDATE EMPLOYEES SET LAST_NAME=:nom, FIRST_NAME=:prenom, JOB_TITLE=:fonction,"
              " AGE=:age, PASSWORD=:mdp, SALARY=:salaire, EMAIL=:email, PHONE_NUMBER=:num"
              " WHERE EMPLOYEE_ID=:id");
    q.bindValue(":id",       id.toInt());
    q.bindValue(":nom",      nom);
    q.bindValue(":prenom",   prenom);
    q.bindValue(":fonction", fonction);
    q.bindValue(":age",      age);
    q.bindValue(":mdp",      mdp);
    q.bindValue(":salaire",  salaire);
    q.bindValue(":email",    email);
    q.bindValue(":num",      num);

    if (q.exec()) {
        if (q.numRowsAffected() > 0) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Success", "Employee updated successfully.");
            logActivity("Modified employee: " + prenom + " " + nom + " (ID: " + id + ")", "Employees");
            onEmployeeClearFields();
            onEmployeeRefreshView();
            onEmployeeRefreshHistory();
        } else {
            QMessageBox::warning(this, "Not Found", "No employee found with that ID.");
        }
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update employee:\n" + q.lastError().text());
    }
}

void MainWindow::onEmployeeDelete()
{
    QModelIndex idx = ui_employee->tableView_employes->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an employee from the table to delete.");
        return;
    }
    QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
    if (!m) return;
    QString empId = m->data(m->index(idx.row(), 2)).toString();
    QString name  = m->data(m->index(idx.row(), 3)).toString() + " " + m->data(m->index(idx.row(), 4)).toString();
    int ret = QMessageBox::question(this, "Confirm Delete", "Delete employee: " + name + "?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == (int)QMessageBox::Yes) {
        // Integrity Check: Prevent deletion if employee has linked clients or equipment
        QSqlQuery chkLinked;
        chkLinked.prepare("SELECT (SELECT COUNT(*) FROM CLIENTS WHERE EMPLOYEE_ID = :id) + (SELECT COUNT(*) FROM EQUIPMENT WHERE EMPLOYEE_ID = :id) FROM DUAL");
        chkLinked.bindValue(":id", empId.toInt());
        if (chkLinked.exec() && chkLinked.next() && chkLinked.value(0).toInt() > 0) {
            QMessageBox::critical(this, "De-authorization Blocked", 
                "CRITICAL: Employee '" + name + "' is the active manager for " + chkLinked.value(0).toString() + " record(s).\n"
                "Reassign these assets before deletion.");
            return;
        }

        QSqlQuery q;
        q.prepare("DELETE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        q.bindValue(":id", empId.toInt());
        if (q.exec()) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Deleted", "Employee deleted.");
            logActivity("Deleted employee: " + name + " (ID: " + empId + ")", "Employees");
            onEmployeeRefreshView();
            onEmployeeRefreshHistory();
        } else {
            QMessageBox::critical(this, "Error", q.lastError().text());
        }
    }
}

// =============================================================================
// EQUIPMENT MANAGEMENT CRUD
// =============================================================================

void MainWindow::onEquipmentRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EQUIPMENT_ID AS \"ID\", "
        "EQUIPMENT_TYPE AS \"Type\", QUANTITY AS \"Qty\", UNIT_PRICE AS \"Price\", "
        "STATUS AS \"Status\", PURCHASE_DATE AS \"Purchase Date\", DESCRIPTION AS \"Description\""
        " FROM EQUIPMENT WHERE STATUS != 'Retired' ORDER BY EQUIPMENT_ID"
    );
    ui_equipment->table_equipments->setModel(model);
    ui_equipment->table_equipments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_equipment->table_equipments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_equipment->table_equipments->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onEquipmentAdd()
{
    if (!ui_equipment->le_id->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "An equipment is already loaded. Use Modify to update it, or Clear to add a new one.");
        return;
    }

    QString type = ui_equipment->le_type->text().trimmed();
    QString desc = ui_equipment->te_desc->toPlainText().trimmed();
    QString etat = ui_equipment->cb_status->currentText();
    QDate   date = ui_equipment->de_date_achat->date();
    int     qty  = ui_equipment->sb_quantity->value();
    QString loc  = "";
    QString resp = "";
    double  price= ui_equipment->dsb_unit_price->value();

    if (type.isEmpty() || desc.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required!");
        return;
    }

    if (qty <= 0) {
        QMessageBox::warning(this, "Input Error", "Quantity must be not null.");
        return;
    }

    if (price <= 0.0) {
        QMessageBox::warning(this, "Input Error", "Unit price must be not null.");
        return;
    }

    // Insert new equipment (ID is auto-generated by the database)
    QSqlQuery q;
    q.prepare("INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, PURCHASE_DATE, EMPLOYEE_ID, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE)"
              " VALUES ((SELECT NVL(MAX(EQUIPMENT_ID), 0) + 1 FROM EQUIPMENT), :type, :desc, :status, TO_DATE(:date,'YYYY-MM-DD'), :empid, :price, :qty, :loc, :resp)");
    q.bindValue(":type",   type);
    q.bindValue(":desc",   desc);
    q.bindValue(":status", etat);
    q.bindValue(":date",   date.toString("yyyy-MM-dd"));
    q.bindValue(":empid",  currentEmployeeId);
    q.bindValue(":price",  price);
    q.bindValue(":qty",    qty);
    q.bindValue(":loc",    loc);
    q.bindValue(":resp",   resp);

    if (!q.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to add equipment:\n" + q.lastError().text());
        return;
    }

    // Get the newly generated ID for logging purposes
    int eqId = 0;
    QSqlQuery idQ("SELECT MAX(EQUIPMENT_ID) FROM EQUIPMENT");
    if (idQ.exec() && idQ.next()) {
        eqId = idQ.value(0).toInt();
    }

    // Success animation
    playEquipSuccessAnimation(type);
    logActivity("Added new equipment: " + type + " (ID: " + QString::number(eqId) + ")", "Equipment");
    
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    updateEquipProgress();
    // Refresh NEXUS analysis
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onEquipmentModify()
{
    QString id   = ui_equipment->le_id->text().trimmed();
    QString type = ui_equipment->le_type->text().trimmed();
    QString desc = ui_equipment->te_desc->toPlainText().trimmed();
    QString etat = ui_equipment->cb_status->currentText();
    double  price= ui_equipment->dsb_unit_price->value();

    int     qty  = ui_equipment->sb_quantity->value();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Please enter the Equipment ID to modify.");
        return;
    }

    // Ensure DB is open before running queries
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(this, "Database Error",
                              "Not connected to the database.\nPlease check your connection and try again.");
        return;
    }

    // Load existing row first so empty form fields do not overwrite required DB columns with NULL.
    QSqlQuery existing;
    existing.prepare("SELECT EQUIPMENT_TYPE, DESCRIPTION, STATUS, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE "
                     "FROM EQUIPMENT WHERE EQUIPMENT_ID = :id");
    existing.bindValue(":id", id.toInt());
    if (!existing.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to read equipment before update:\n" + existing.lastError().text());
        return;
    }
    if (!existing.next()) {
        QMessageBox::warning(this, "Validation", "No equipment found with this ID.");
        return;
    }

    const QString currentType = existing.value(0).toString();
    const QString currentDesc = existing.value(1).toString();
    const QString currentStatus = existing.value(2).toString();
    const double currentPrice = existing.value(3).toDouble();
    const int currentQty = existing.value(4).toInt();
    const QString currentLoc = existing.value(5).toString();
    const QString currentResp = existing.value(6).toString();

    const QString finalType = type.isEmpty() ? currentType : type;
    const QString finalDesc = desc.isEmpty() ? currentDesc : desc;
    const QString finalStatus = etat.isEmpty() ? currentStatus : etat;
    const double finalPrice = (price <= 0.0) ? currentPrice : price;
    const int finalQty = (qty <= 0) ? currentQty : qty;
    const QString finalLoc = currentLoc;
    const QString finalResp = currentResp;

    if (finalType.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Equipment type cannot be empty.");
        return;
    }

    QSqlQuery q;
    q.prepare("UPDATE EQUIPMENT SET EQUIPMENT_TYPE=:type, DESCRIPTION=:desc, STATUS=:status, UNIT_PRICE=:price, "
              "QUANTITY=:qty, LOCATION=:loc, RESPONSABLE=:resp, NEXT_MAINTENANCE=SYSDATE WHERE EQUIPMENT_ID=:id");
    q.bindValue(":id",     id.toInt());
    q.bindValue(":type",   finalType);
    q.bindValue(":desc",   finalDesc);
    q.bindValue(":status", finalStatus);
    q.bindValue(":price",  finalPrice);
    q.bindValue(":qty",    finalQty);
    q.bindValue(":loc",    finalLoc);
    q.bindValue(":resp",   finalResp);

    if (!q.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to update equipment:\n" + q.lastError().text());
        return;
    }

    QMessageBox::information(this, "Success", "Equipment updated successfully.");
    logActivity("Modified equipment: " + finalType + " (ID: " + id + ")", "Equipment");
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onEquipmentDelete()
{
    QString eqId;
    QString type;
    QString desc;

    QModelIndex idx = ui_equipment->table_equipments->currentIndex();
    if (idx.isValid()) {
        QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
        if (!model) return;
        eqId = model->data(model->index(idx.row(), 2)).toString();
        type = model->data(model->index(idx.row(), 3)).toString();
        desc = model->data(model->index(idx.row(), 8)).toString();
    } else {
        eqId = ui_equipment->le_id->text().trimmed();
        type = ui_equipment->le_type->text().trimmed();
        desc = ui_equipment->te_desc->toPlainText().trimmed();
    }

    if (eqId.isEmpty()) {
        QMessageBox::warning(this, "Selection", "Please select equipment from the list or load an equipment in the form to delete.");
        return;
    }

    int ret = QMessageBox::question(this, "Confirm Delete",
        "Delete equipment: " + type + " - " + desc + " (ID: " + eqId + ")?",
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    // Hard delete from DB so the equipment no longer exists in the table.
    QSqlQuery q;
    q.prepare("DELETE FROM EQUIPMENT WHERE EQUIPMENT_ID = :id");
    q.bindValue(":id", eqId.toInt());
    if (!q.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to delete equipment:\n" + q.lastError().text());
        return;
    }
    if (q.numRowsAffected() <= 0) {
        QMessageBox::warning(this, "Not Found", "No equipment found with this ID.");
        return;
    }
    logActivity("Deleted equipment: " + type + " (ID: " + eqId + ")", "Equipment");

    QMessageBox::information(this, "Deleted", "Equipment deleted successfully.");
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onEquipmentSearch()
{
    QString search = ui_equipment->le_recherche->text().trimmed();
    QString filterStatus = ui_equipment->cb_filter_status->currentText();
    QSqlQueryModel *model = new QSqlQueryModel(this);

    QString sql = "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EQUIPMENT_ID AS \"ID\", "
                  "EQUIPMENT_TYPE AS \"Type\", QUANTITY AS \"Qty\", UNIT_PRICE AS \"Price\", "
                  "STATUS AS \"Status\", PURCHASE_DATE AS \"Purchase Date\", DESCRIPTION AS \"Description\" "
                  "FROM EQUIPMENT WHERE STATUS != 'Retired'";

    if (!search.isEmpty()) {
        sql += " AND (UPPER(DESCRIPTION) LIKE '%" + search.toUpper() + "%' "
               "OR UPPER(EQUIPMENT_TYPE) LIKE '%" + search.toUpper() + "%' "
               "OR CAST(EQUIPMENT_ID AS VARCHAR2(20)) LIKE '%" + search + "%')";
    }

    if (filterStatus != "All Statuses") {
        sql += " AND STATUS = '" + filterStatus + "'";
    }

    sql += " ORDER BY EQUIPMENT_ID";
    model->setQuery(sql);
    ui_equipment->table_equipments->setModel(model);
    ui_equipment->table_equipments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_equipment->table_equipments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_equipment->table_equipments->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onClientExportPDF()
{
    if (!ui_client) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Export Client List",
        QDir::homePath() + "/Client_List_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf);;All Files (*)");

    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QSize(297, 210), QPageSize::Millimeter)); // A4 Landscape
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to start PDF export.");
        return;
    }

    int pageWidth = printer.width();
    int y = 50;

    // Title
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71)); // Brown
    painter.drawText(0, y, pageWidth, 40, Qt::AlignCenter, "Hammer Down - Client List");
    y += 60;

    // Header Row
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::black);
    
    int cols[] = { 60, 150, 150, 250, 120, 200, 80 }; // ID, Nom, Prenom, Addr, Tel, Email, Sexe
    QString headers[] = { "ID", "Last Name", "First Name", "Address", "Phone", "Email", "Gender" };
    
    int x = 20;
    for (int i = 0; i < 7; ++i) {
        painter.drawText(x, y, cols[i], 25, Qt::AlignLeft, headers[i]);
        x += cols[i];
    }
    
    painter.drawLine(20, y + 25, pageWidth - 20, y + 25);
    y += 40;

    // Data Rows
    painter.setFont(QFont("Segoe UI", 9));
    QSqlQuery q("SELECT CLIENT_ID, LAST_NAME, FIRST_NAME, ADDRESS, PHONE_NUMBER, EMAIL, GENDER FROM CLIENTS ORDER BY CLIENT_ID");
    while (q.next()) {
        if (y > printer.height() - 60) {
            printer.newPage();
            y = 50;
        }
        
        x = 20;
        for (int i = 0; i < 7; ++i) {
            painter.drawText(x, y, cols[i], 20, Qt::AlignLeft, q.value(i).toString());
            x += cols[i];
        }
        y += 25;
    }

    painter.end();
    QMessageBox::information(this, "Success", "Client list successfully exported to:\n" + fileName);
}

// ---------------------------------------------------------------------------
// Client Mail tab — Send
// ---------------------------------------------------------------------------
void MainWindow::onClientSendMail()
{
    if (!ui_client) return;

    const QString to         = ui_client->le_to->text().trimmed();
    const QString subject    = ui_client->le_subject->text().trimmed();
    const QString attachment = ui_client->le_attachment->text().trimmed();
    const QString body       = ui_client->te_message->toPlainText().trimmed();

    // --- Validation ---
    if (to.isEmpty()) {
        QMessageBox::warning(this, "Email Error",
            "Please enter a recipient email address.");
        ui_client->le_to->setFocus();
        return;
    }

    static const QRegularExpression emailRe(
        R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
    if (!emailRe.match(to).hasMatch()) {
        QMessageBox::warning(this, "Email Error",
            "Invalid email address.\nMust contain '@' and a valid domain (e.g. user@example.com).");
        ui_client->le_to->setFocus();
        return;
    }

    if (body.isEmpty()) {
        QMessageBox::warning(this, "Email Error",
            "Please enter a message.");
        ui_client->te_message->setFocus();
        return;
    }

    if (body.length() < 5) {
        QMessageBox::warning(this, "Email Error",
            "Message is too short — minimum 5 characters required.");
        ui_client->te_message->setFocus();
        return;
    }

    // --- Hardcoded SMTP credentials ---
    const QString host     = "smtp.mailersend.net";
    const quint16 port     = 587;
    const QString username = "MS_jsamBQ@test-zkq340er2v6gd796.mlsender.net";
    const QString password = "mssp.9eMDqAS.ynrw7gy2pqr42k8e.TxccD9w";

    // Disable the button while sending
    ui_client->btn_send->setEnabled(false);
    ui_client->btn_send->setText("Sending...");

    // Run SMTP in a background thread so the UI stays responsive
    QFutureWatcher<SmtpResult> *watcher = new QFutureWatcher<SmtpResult>(this);
    connect(watcher, &QFutureWatcher<SmtpResult>::finished, this, [this, watcher]() {
        const SmtpResult result = watcher->result();
        watcher->deleteLater();

        ui_client->btn_send->setEnabled(true);
        ui_client->btn_send->setText("Send");

        if (result.success) {
            QMessageBox::information(this, "Email Sent",
                "Your email was sent successfully.");
            ui_client->le_to->clear();
            ui_client->le_subject->clear();
            ui_client->le_attachment->clear();
            ui_client->te_message->clear();
        } else {
            QMessageBox::critical(this, "Email Failed",
                "Failed to send email:\n" + result.errorMessage);
        }
    });

    QFuture<SmtpResult> future = QtConcurrent::run([=]() {
        return SmtpSender::send(host, port, username, password,
                                to, subject, body, attachment);
    });
    watcher->setFuture(future);
}

// ---------------------------------------------------------------------------
// Client Mail tab — Browse attachment
// ---------------------------------------------------------------------------
void MainWindow::onClientBrowseMail()
{
    if (!ui_client) return;
    const QString path = QFileDialog::getOpenFileName(
        this, "Select Attachment", QDir::homePath(), "All Files (*)");
    if (!path.isEmpty())
        ui_client->le_attachment->setText(path);
}




void MainWindow::onEquipmentHistoryRefresh()
{
    onEquipmentHistorySearch();
}

void MainWindow::onEquipmentHistorySearch()
{
    QString search = ui_equipment->le_history_search->text().trimmed();

    auto setupSection = [&](QTableView* view, const QString& operation) {
        QString sql;
        if (operation == "ADD") {
            sql = "SELECT EQUIPMENT_ID AS \"ID\", EQUIPMENT_TYPE AS \"Type\", DESCRIPTION AS \"Description\", "
                  "STATUS AS \"Status\", UNIT_PRICE AS \"Price\", TO_CHAR(PURCHASE_DATE, 'YYYY-MM-DD') AS \"Date\" "
                  "FROM EQUIPMENT WHERE STATUS != 'Retired'";
        } else if (operation == "MODIFY") {
            sql = "SELECT EQUIPMENT_ID AS \"ID\", EQUIPMENT_TYPE AS \"Type\", DESCRIPTION AS \"Description\", "
                  "STATUS AS \"Status\", UNIT_PRICE AS \"Price\", TO_CHAR(NEXT_MAINTENANCE, 'YYYY-MM-DD') AS \"Date\" "
                  "FROM EQUIPMENT WHERE NEXT_MAINTENANCE IS NOT NULL AND STATUS != 'Retired'";
        } else if (operation == "DELETE") {
            sql = "SELECT EQUIPMENT_ID AS \"ID\", EQUIPMENT_TYPE AS \"Type\", DESCRIPTION AS \"Description\", "
                  "STATUS AS \"Status\", UNIT_PRICE AS \"Price\", TO_CHAR(SYSDATE, 'YYYY-MM-DD') AS \"Date\" "
                  "FROM EQUIPMENT WHERE STATUS = 'Retired'";
        }
        
        if (!search.isEmpty()) {
            sql += " AND (UPPER(EQUIPMENT_TYPE) LIKE '%" + search.toUpper() + "%' "
                   " OR CAST(EQUIPMENT_ID AS VARCHAR2(20)) LIKE '%" + search + "%' "
                   " OR UPPER(DESCRIPTION) LIKE '%" + search.toUpper() + "%')";
        }
        
        if (operation == "ADD") sql += " ORDER BY PURCHASE_DATE DESC";
        else if (operation == "MODIFY") sql += " ORDER BY NEXT_MAINTENANCE DESC";
        else sql += " ORDER BY EQUIPMENT_ID DESC";

        QSqlQueryModel *model = new QSqlQueryModel(this);
        model->setQuery(sql);
        
        if (model->lastError().isValid()) {
            qDebug() << "Logical History Error (" << operation << "):" << model->lastError().text();
        }

        view->setModel(model);
        view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    };

    setupSection(ui_equipment->tableView_history_add,    "ADD");
    setupSection(ui_equipment->tableView_history_modify, "MODIFY");
    setupSection(ui_equipment->tableView_historique,     "DELETE");
}

void MainWindow::onEquipmentHistoryClear()
{
    QMessageBox::information(this, "Clear History", "History is now logically linked to the main equipment table and cannot be cleared separately.");
}

void MainWindow::onEquipmentCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = ui_equipment->table_equipments->indexAt(pos);
    if (!index.isValid()) return;

    // Get the ID (Column 2 according to onEquipmentRefreshView: Action, Delete, ID...)
    int equipId = ui_equipment->table_equipments->model()->data(ui_equipment->table_equipments->model()->index(index.row(), 2)).toInt();

    QMenu menu(this);
    QAction *analyzeAct = menu.addAction("🔍 Analyze in Nexus");
    
    QAction *selected = menu.exec(ui_equipment->table_equipments->viewport()->mapToGlobal(pos));
    if (selected == analyzeAct) {
        // Switch to NEXUS tab
        ui_equipment->tabWidget->setCurrentWidget(m_nexusWidget);
        // Highlight in graph
        m_nexusWidget->highlightEquipmentInGraph(equipId);
    }
}

void MainWindow::onEquipmentHistoryCustomContextMenu(const QPoint &pos, int tableIdx)
{
    QTableView *view = nullptr;
    if (tableIdx == 0) view = ui_equipment->tableView_history_add;
    else if (tableIdx == 1) view = ui_equipment->tableView_history_modify;
    else view = ui_equipment->tableView_historique;

    if (!view) return;
    QModelIndex index = view->indexAt(pos);
    if (!index.isValid()) return;

    // Get the Date (Last column usually)
    int dateCol = view->model()->columnCount() - 1;
    QDate date = QDate::fromString(view->model()->data(view->model()->index(index.row(), dateCol)).toString(), "YYYY-MM-DD");
    if (!date.isValid()) date = QDate::currentDate();

    QMenu menu(this);
    QAction *nexusViewAct = menu.addAction("⏳ Nexus View (Time Machine)");
    
    QAction *selected = menu.exec(view->viewport()->mapToGlobal(pos));
    if (selected == nexusViewAct) {
        ui_equipment->tabWidget->setCurrentWidget(m_nexusWidget);
        m_nexusWidget->goToTimeMachineDate(date);
    }
}

// =============================================================================
// PRESENTATION MODE (DIRECTOR'S CUT)
// =============================================================================
void MainWindow::togglePresentationMode() {
    m_isPresentationMode = !m_isPresentationMode;
    
    if (m_isPresentationMode) {
        // TURN ON
        m_presentationStep = 0;
        
        // 1. Fade away controls
        QList<QWidget*> controls;
        for (auto *w : findChildren<QPushButton*>()) controls << w;
        for (auto *w : findChildren<QRadioButton*>()) controls << w;
        for (auto *w : findChildren<QToolButton*>()) controls << w;
        
        for (auto *c : controls) {
            if (c->parent() == this || c->parentWidget() == this) continue; // Keep main window buttons? No, hide all
            QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(c);
            c->setGraphicsEffect(eff);
            QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
            a->setDuration(1200);
            a->setStartValue(1.0);
            a->setEndValue(0.0);
            connect(a, &QPropertyAnimation::finished, c, &QWidget::hide);
            a->start(QAbstractAnimation::DeleteWhenStopped);
        }
        
        // 2. Expand and transform
        if (ui_equipment && ui_equipment->tabWidget) {
            ui_equipment->tabWidget->setGeometry(50, 50, width()-100, height()-100);
            ui_equipment->tabWidget->setStyleSheet("QTabWidget::pane { border: none; background: transparent; } QTabBar::tab { height: 0px; width: 0px; margin: 0; padding: 0; }");
        }

        // 3. Ken Burns Background
        startKenBurnsEffect();

        // 4. Watermark
        m_presentationOverlay = new QWidget(this);
        m_presentationOverlay->setGeometry(rect());
        m_presentationOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_presentationOverlay->show();

        QLabel *watermark = new QLabel(m_presentationOverlay);
        watermark->setPixmap(QIcon(":/assets/logo.png").pixmap(120, 120));
        watermark->setGeometry(width()-150, height()-150, 120, 120);
        watermark->setStyleSheet("background: transparent; opacity: 0.5;");
        watermark->show();

        // 5. Setup Rotation/Navigation state
        m_presentationStep = ui_equipment->tabWidget->indexOf(ui_equipment->tab_view);
        if (m_presentationStep == -1) m_presentationStep = 0;
        
        ui_equipment->tabWidget->setCurrentIndex(m_presentationStep);
        advancePresentation(); // Update UI/Toast
        
    } else {
        // TURN OFF (Restore Normal View)
        if (m_presentationOverlay) m_presentationOverlay->deleteLater();
        m_presentationOverlay = nullptr;
        
        // Restore controls visibility
        QList<QWidget*> controls;
        for (auto *w : findChildren<QPushButton*>()) controls << w;
        for (auto *w : findChildren<QRadioButton*>()) controls << w;
        for (auto *w : findChildren<QToolButton*>()) controls << w;

        for (auto *c : controls) {
            c->setGraphicsEffect(nullptr);
            c->show();
        }

        if (ui_equipment && ui_equipment->tabWidget) {
            ui_equipment->tabWidget->setGeometry(118, 70, 1051, 681);
            ui_equipment->tabWidget->setStyleSheet(""); 
        }
    }
}

void MainWindow::advancePresentation() {
    if (!m_isPresentationMode || !ui_equipment) return;

    ui_equipment->tabWidget->setCurrentIndex(m_presentationStep);
    
    QString stepTitle;
    QWidget* current = ui_equipment->tabWidget->currentWidget();
    
    if (current == ui_equipment->tab_gestion) stepTitle = "Workshop Resource Management";
    else if (current == ui_equipment->tab_view) stepTitle = "Workshop Inventory Status";
    else if (current == ui_equipment->tab_stats) stepTitle = "Global Asset Analytics";
    else if (current == ui_equipment->tab_history) stepTitle = "Operational Activity logs";
    else if (current == ui_equipment->tab_chat) stepTitle = "Workshop Secure Communications";
    else stepTitle = "Presentation Slide";

    // Overlay title toast
    QLabel *toast = new QLabel(stepTitle, this);
    toast->setFixedSize(500, 80);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet("background: rgba(139,111,71,0.95); color: white; font-size: 26px; border-radius: 40px; border: 2.5px solid #D4AF37; font-weight: bold;");
    toast->move(width()/2 - 250, 80);
    toast->show();
    
    QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(op);
    QPropertyAnimation *fIn = new QPropertyAnimation(op, "opacity");
    fIn->setDuration(800);
    fIn->setStartValue(0.0);
    fIn->setEndValue(1.0);
    fIn->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(2500, [=](){
        QPropertyAnimation *fOut = new QPropertyAnimation(op, "opacity");
        fOut->setDuration(1200);
        fOut->setStartValue(1.0);
        fOut->setEndValue(0.0);
        connect(fOut, &QPropertyAnimation::finished, toast, &QLabel::deleteLater);
        fOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::startKenBurnsEffect() {
    // Find the background (usually set via stylesheet on MainWindow or a central frame)
    // We'll simulate by animating a large ghost image or the central widget style
    QPropertyAnimation *kb = new QPropertyAnimation(this, "geometry"); // Dummy for now to trigger background repaint if needed
    Q_UNUSED(kb);
    // In a real app, you'd apply this to a specific QGraphicsView background
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (m_isPresentationMode) {
        if (event->key() == Qt::Key_Escape) {
            togglePresentationMode(); // Exit only on Escape
            event->accept();
        } else if (event->key() == Qt::Key_Left) {
            // Previous tab
            int count = ui_equipment->tabWidget->count();
            m_presentationStep = (m_presentationStep - 1 + count) % count;
            advancePresentation();
            event->accept();
        } else if (event->key() == Qt::Key_Right) {
            // Next tab
            int count = ui_equipment->tabWidget->count();
            m_presentationStep = (m_presentationStep + 1) % count;
            advancePresentation();
            event->accept();
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateEquipProgress() {
    if (!m_equipProgress || !ui_equipment) return;
    
    int progress = 0;
    bool typeOk  = !ui_equipment->le_type->text().trimmed().isEmpty();
    bool dateOk  = ui_equipment->de_date_achat->date().isValid();
    bool priceOk = ui_equipment->dsb_unit_price->value() > 0;
    bool descOk  = !ui_equipment->te_desc->toPlainText().trimmed().isEmpty();
    
    if (typeOk)  progress += 25;
    if (dateOk)  progress += 25;
    if (priceOk) progress += 25;
    if (descOk)  progress += 25;
    
    m_equipProgress->setValue(progress);
    
    // Change bar color to green when filled (100%)
    if (progress == 100) {
        m_equipProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #66BB6A); border-radius: 5px; }");
    } else {
        m_equipProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");
    }
    
    auto updateInd = [](QLabel* l, bool ok, const QString& prefix) {
        if (!l) return;
        if (ok) {
            l->setText(prefix + " ✅]");
            l->setStyleSheet("color: #4CAF50; font-size: 11px; font-weight: bold;");
        } else {
            l->setText(prefix + " ⬜]");
            l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        }
    };
    
    updateInd(m_eqTypeInd, typeOk, "[🔨 Type");
    updateInd(m_eqDateInd, dateOk, "[📅 Date");
    updateInd(m_eqPriceInd, priceOk, "[💰 Price");
    updateInd(m_eqDescInd, descOk, "[📝 Desc");

    // Pulsing animation for Add button at 100%
    if (progress == 100) {
        if (!ui_equipment->btn_add->graphicsEffect()) {
            QGraphicsDropShadowEffect *eff = new QGraphicsDropShadowEffect(this);
            eff->setBlurRadius(15);
            eff->setColor(QColor(212, 175, 55, 200));
            eff->setOffset(0);
            ui_equipment->btn_add->setGraphicsEffect(eff);
            
            QPropertyAnimation *pulse = new QPropertyAnimation(eff, "blurRadius");
            pulse->setDuration(1000);
            pulse->setStartValue(8);
            pulse->setEndValue(25);
            pulse->setLoopCount(-1);
            pulse->setEasingCurve(QEasingCurve::InOutSine);
            pulse->start(QAbstractAnimation::DeleteWhenStopped);
        }
    } else {
        ui_equipment->btn_add->setGraphicsEffect(nullptr);
    }
}

void MainWindow::updateSupplierProgress() {
    if (!m_supplierProgress || !ui_supplier) return;
    
    int progress = 0;
    bool nameOk  = !ui_supplier->le_nom->text().trimmed().isEmpty();
    bool emailOk = ui_supplier->le_email->text().contains("@") && ui_supplier->le_email->text().contains(".");
    bool telOk   = ui_supplier->le_tel->text().trimmed().length() >= 8;
    bool typeOk  = !ui_supplier->le_type->text().trimmed().isEmpty();
    
    if (nameOk)  progress += 25;
    if (emailOk) progress += 25;
    if (telOk)   progress += 25;
    if (typeOk)  progress += 25;
    
    m_supplierProgress->setValue(progress);
    
    if (progress == 100) {
        m_supplierProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #66BB6A); border-radius: 5px; }");
    } else {
        m_supplierProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");
    }
    
    auto updateInd = [](QLabel* l, bool ok, const QString& prefix) {
        if (!l) return;
        if (ok) {
            l->setText(prefix + " ✅]");
            l->setStyleSheet("color: #4CAF50; font-size: 11px; font-weight: bold;");
        } else {
            l->setText(prefix + " ⬜]");
            l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        }
    };
    
    updateInd(m_suppNameInd,  nameOk,  "[👤 Name");
    updateInd(m_suppEmailInd, emailOk, "[📧 Email");
    updateInd(m_suppTelInd,   telOk,   "[📞 Phone");
    updateInd(m_suppTypeInd,  typeOk,  "[🏢 Type");

    // Pulsing animation for Add button at 100%
    if (progress == 100) {
        if (!ui_supplier->btn_add->graphicsEffect()) {
            QGraphicsDropShadowEffect *eff = new QGraphicsDropShadowEffect(this);
            eff->setBlurRadius(15);
            eff->setColor(QColor(212, 175, 55, 200));
            eff->setOffset(0);
            ui_supplier->btn_add->setGraphicsEffect(eff);
            
            QPropertyAnimation *pulse = new QPropertyAnimation(eff, "blurRadius");
            pulse->setDuration(1000);
            pulse->setStartValue(8);
            pulse->setEndValue(25);
            pulse->setLoopCount(-1);
            pulse->setEasingCurve(QEasingCurve::InOutSine);
            pulse->start(QAbstractAnimation::DeleteWhenStopped);
        }
    } else {
        ui_supplier->btn_add->setGraphicsEffect(nullptr);
    }
}

void MainWindow::playEquipSuccessAnimation(const QString &equipName) {
    // 0. Play Anvil Sound only (assets/sound/anvil.mp4/wav)
    QMediaPlayer *sfx = new QMediaPlayer(this);
    QAudioOutput *sfxOut = new QAudioOutput(this);
    sfx->setAudioOutput(sfxOut);
    sfx->setSource(QUrl::fromLocalFile(QDir::currentPath() + "/assets/sound/anvil.mp4"));
    sfxOut->setVolume(currentVolume);
    sfx->play();
    connect(sfx, &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia) {
            sfx->deleteLater();
            sfxOut->deleteLater();
        }
    });

    // 1. Flash green
    QList<QWidget*> widgets = { ui_equipment->le_type, ui_equipment->de_date_achat, 
                               ui_equipment->dsb_unit_price, ui_equipment->te_desc };
    for (auto w : widgets) {
        QString oldStyle = w->styleSheet();
        w->setStyleSheet(oldStyle + " background-color: rgba(76, 175, 80, 0.3); border: 2px solid #4CAF50;");
        QTimer::singleShot(800, [=]() { w->setStyleSheet(oldStyle); });
    }

    // 2. Flying Card
    QLabel *flyer = new QLabel(this);
    flyer->setText("🛠️ " + equipName);
    flyer->setFixedSize(160, 45);
    flyer->setAlignment(Qt::AlignCenter);
    flyer->setStyleSheet("background: #8B6F47; color: white; border: 2px solid #D4AF37; border-radius: 12px; font-weight: bold; font-family: 'Segoe UI';");
    
    QPoint startPos = ui_equipment->groupBox_gestion->mapTo(this, QPoint(150, 200));
    // Approximate position of "View" nav radio button
    QPoint endPos = QPoint(200, 100); 

    flyer->move(startPos);
    flyer->show();
    flyer->raise();

    QPropertyAnimation *moveAnim = new QPropertyAnimation(flyer, "pos");
    moveAnim->setDuration(1000);
    moveAnim->setStartValue(startPos);
    moveAnim->setEndValue(endPos);
    moveAnim->setEasingCurve(QEasingCurve::InOutBack);

    QPropertyAnimation *scaleAnim = new QPropertyAnimation(flyer, "size");
    scaleAnim->setDuration(1000);
    scaleAnim->setStartValue(QSize(160, 45));
    scaleAnim->setEndValue(QSize(10, 10));

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(scaleAnim);
    
    connect(group, &QParallelAnimationGroup::finished, this, [=]() {
        flyer->hide();
        flyer->deleteLater();
        
        // 4. Confetti (Wood chips)
        for (int i=0; i<15; ++i) {
            QLabel *chip = new QLabel(this);
            chip->setFixedSize(8, 8);
            chip->setStyleSheet(QString("background: %1; border-radius: 3px; border: 1px solid rgba(0,0,0,0.2);")
                                .arg(i%2==0 ? "#8B6F47" : "#D3C1A5"));
            QPoint cStart = endPos + QPoint(rand()%40-20, rand()%20-10);
            chip->move(cStart);
            chip->show();
            chip->raise();
            
            QPropertyAnimation *cMove = new QPropertyAnimation(chip, "pos");
            cMove->setDuration(600 + rand()%600);
            cMove->setStartValue(cStart);
            cMove->setEndValue(cStart + QPoint(rand()%140-70, rand()%140-30));
            cMove->setEasingCurve(QEasingCurve::OutCubic);
            
            QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(chip);
            chip->setGraphicsEffect(op);
            QPropertyAnimation *cFade = new QPropertyAnimation(op, "opacity");
            cFade->setDuration(cMove->duration());
            cFade->setStartValue(1.0);
            cFade->setEndValue(0.0);
            
            QParallelAnimationGroup *cGrp = new QParallelAnimationGroup(this);
            cGrp->addAnimation(cMove);
            cGrp->addAnimation(cFade);
            connect(cGrp, &QParallelAnimationGroup::finished, chip, &QLabel::deleteLater);
            cGrp->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);

    // 5. Toast notification
    QLabel *toast = new QLabel("✅ " + equipName + " added to workshop!", this);
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3C2D1E, stop:1 #251B12);"
        "color: #D4AF37; border: 2.5px solid #8B6F47; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
    QGraphicsDropShadowEffect *tShadow = new QGraphicsDropShadowEffect(toast);
    tShadow->setBlurRadius(15);
    tShadow->setOffset(0, 4);
    toast->setGraphicsEffect(tShadow);

    QPoint toastEnd = QPoint(this->width() - 350, 30);
    QPoint toastStart = QPoint(this->width() + 10, 30);
    toast->move(toastStart);
    toast->show();
    toast->raise();

    QPropertyAnimation *tIn = new QPropertyAnimation(toast, "pos");
    tIn->setDuration(700);
    tIn->setStartValue(toastStart);
    tIn->setEndValue(toastEnd);
    tIn->setEasingCurve(QEasingCurve::OutBack);

    QTimer::singleShot(3000, [=]() {
        QPropertyAnimation *tOut = new QPropertyAnimation(toast, "pos");
        tOut->setDuration(500);
        tOut->setStartValue(toastEnd);
        tOut->setEndValue(toastStart);
        tOut->setEasingCurve(QEasingCurve::InBack);
        connect(tOut, &QPropertyAnimation::finished, toast, &QLabel::deleteLater);
        tOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
    tIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onEquipmentExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Equipment Registry",
        QDir::homePath() + "/Equipment_Registry_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to initialize PDF printer.");
        return;
    }

    int W = painter.viewport().width();
    int y = 50;

    // Title
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(0, y, W, 40, Qt::AlignCenter, "HammerDown - Equipment Registry");
    y += 50;

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(100,100,100));
    painter.drawText(0, y, W, 25, Qt::AlignCenter,
        "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    y += 40;

    // Column positions (proportional)
    int cID    = 20;
    int cType  = W * 0.08;
    int cDesc  = W * 0.25;
    int cStat  = W * 0.52;
    int cPrice = W * 0.65;
    int cDate  = W * 0.77;
    int cResp  = W * 0.88;

    // Header bar
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(139, 111, 71));
    painter.drawRect(10, y - 15, W - 20, 28);

    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter.setPen(Qt::white);
    painter.drawText(cID,    y, "ID");
    painter.drawText(cType,  y, "Type");
    painter.drawText(cDesc,  y, "Description");
    painter.drawText(cStat,  y, "Status");
    painter.drawText(cPrice, y, "Unit Price");
    painter.drawText(cDate,  y, "Purchase Date");
    painter.drawText(cResp,  y, "Responsible");
    y += 30;

    // Rows
    painter.setFont(QFont("Segoe UI", 8));
    bool alt = false;
    QSqlQuery q(
        "SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, UNIT_PRICE, "
        "TO_CHAR(PURCHASE_DATE,'YYYY-MM-DD'), RESPONSABLE "
        "FROM EQUIPMENT WHERE STATUS != 'Retired' ORDER BY EQUIPMENT_ID DESC"
    );
    while (q.next()) {
        if (y > printer.height() - 60) {
            printer.newPage();
            y = 50;
        }
        // Alternating row background
        painter.setPen(Qt::NoPen);
        painter.setBrush(alt ? QColor(245, 240, 235) : Qt::white);
        painter.drawRect(10, y - 14, W - 20, 22);
        alt = !alt;

        painter.setPen(Qt::black);
        painter.drawText(cID,    y, q.value(0).toString());
        painter.drawText(cType,  y, q.value(1).toString().left(20));
        painter.drawText(cDesc,  y, q.value(2).toString().left(30));
        painter.drawText(cStat,  y, q.value(3).toString());
        painter.drawText(cPrice, y, q.value(4).isNull() ? "-" : QString::number(q.value(4).toDouble(),'f',2));
        painter.drawText(cDate,  y, q.value(5).toString());
        painter.drawText(cResp,  y, q.value(6).toString().left(18));
        y += 24;
    }

    painter.end();
    QMessageBox::information(this, "Success",
        "Equipment registry exported successfully to:\n" + fileName);
}



void MainWindow::onEquipmentExportStatsPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Equipment Statistics",
        QDir::homePath() + "/Equipment_Stats_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to initialize PDF printer.");
        return;
    }

    int W = painter.viewport().width();
    int H = painter.viewport().height();
    int y = 50;

    // --- Header / Branding ---
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background header accent
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(139, 111, 71)); // Brand Gold
    painter.drawRect(0, 0, W, 120);

    painter.setFont(QFont("Segoe UI", 28, QFont::Bold));
    painter.setPen(Qt::white);
    painter.drawText(40, 75, "HammerDown");
    
    painter.setFont(QFont("Segoe UI", 12));
    painter.drawText(W - 300, 75, 260, 30, Qt::AlignRight, "Equipment Analytics");
    
    y = 160;

    // Report Title & Date
    painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
    painter.setPen(QColor(50, 50, 50));
    painter.drawText(40, y, "Inventory Status Report");
    
    painter.setFont(QFont("Segoe UI", 10));
    painter.setPen(QColor(120, 120, 120));
    painter.drawText(W - 300, y, 260, 30, Qt::AlignRight, 
        "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy HH:mm"));
    y += 60;

    // --- Fetch Statistics ---
    int total = 0;
    int intactCount = 0;
    int brokenCount = 0;
    double totalValue = 0;

    QSqlQuery qStats("SELECT COUNT(*), "
                     "SUM(CASE WHEN STATUS = 'Available' OR STATUS = 'In Use' THEN 1 ELSE 0 END), "
                     "SUM(CASE WHEN STATUS = 'Under Maintenance' THEN 1 ELSE 0 END), "
                     "SUM(UNIT_PRICE) "
                     "FROM EQUIPMENT WHERE STATUS != 'Retired'");
    if (qStats.next()) {
        total = qStats.value(0).toInt();
        intactCount = qStats.value(1).toInt();
        brokenCount = qStats.value(2).toInt();
        totalValue = qStats.value(3).toDouble();
    }

    // --- Summary Cards (Top Section) ---
    int cardW = (W - 80 - 40) / 3; // 40 margin, 20 gap between cards
    int cardH = 100;
    
    auto drawCard = [&](int x, int y, const QString& label, const QString& value, const QColor& color) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(color.lighter(160));
        painter.drawRoundedRect(x, y, cardW, cardH, 8, 8);
        
        painter.setPen(color);
        painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
        painter.drawText(x + 15, y + 30, label.toUpper());
        
        painter.setFont(QFont("Segoe UI", 22, QFont::Bold));
        painter.drawText(x + 15, y + 75, value);
    };

    drawCard(40, y, "Total Assets", QString::number(total), QColor(139, 111, 71));
    drawCard(40 + cardW + 20, y, "Operational", QString::number(intactCount), QColor(46, 125, 50)); // Green
    drawCard(40 + (cardW + 20) * 2, y, "Flagged/Repair", QString::number(brokenCount), QColor(198, 40, 40)); // Red
    
    y += cardH + 40;

    // Financial Highlight
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 245, 245));
    painter.drawRoundedRect(40, y, W - 80, 50, 5, 5);
    painter.setPen(QColor(100, 100, 100));
    painter.setFont(QFont("Segoe UI", 11));
    painter.drawText(60, y + 32, "Total Inventory Valuation:");
    painter.setPen(QColor(139, 111, 71));
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.drawText(W - 250, y + 32, 200, 30, Qt::AlignRight, 
        QString::number(totalValue, 'f', 2) + " DT");
    
    y += 90;

    // --- Detailed Condition Table ---
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.setPen(QColor(50, 50, 50));
    painter.drawText(40, y, "Condition Analysis by Type");
    y += 35;

    // Table Header Styling
    int colWidths[] = { (int)(W*0.35), (int)(W*0.15), (int)(W*0.20), (int)(W*0.20) };
    QString headers[] = { "Equipment Type", "Count", "Operational", "Service Req." };
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(139, 111, 71));
    painter.drawRect(40, y, W - 80, 40);
    
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::white);
    int currentX = 55;
    for (int i = 0; i < 4; ++i) {
        painter.drawText(currentX, y, colWidths[i], 40, Qt::AlignVCenter, headers[i]);
        currentX += colWidths[i];
    }
    y += 40;

    // Table Data
    QSqlQuery qBreakdown(
        "SELECT EQUIPMENT_TYPE, COUNT(*), "
        "SUM(CASE WHEN STATUS = 'Available' OR STATUS = 'In Use' THEN 1 ELSE 0 END), "
        "SUM(CASE WHEN STATUS = 'Under Maintenance' THEN 1 ELSE 0 END) "
        "FROM EQUIPMENT WHERE STATUS != 'Retired' GROUP BY EQUIPMENT_TYPE ORDER BY COUNT(*) DESC"
    );

    bool alternate = false;
    painter.setFont(QFont("Segoe UI", 10));
    
    while (qBreakdown.next()) {
        if (y > H - 100) {
            printer.newPage();
            y = 50;
            // Redraw header on new page? (Optional improvement)
        }
        
        // Row background
        painter.setPen(Qt::NoPen);
        painter.setBrush(alternate ? QColor(248, 248, 248) : Qt::white);
        painter.drawRect(40, y, W - 80, 35);
        
        painter.setPen(QColor(60, 60, 60));
        currentX = 55;
        
        // Type
        painter.drawText(currentX, y, colWidths[0], 35, Qt::AlignVCenter, qBreakdown.value(0).toString());
        currentX += colWidths[0];
        
        // Count
        painter.drawText(currentX, y, colWidths[1], 35, Qt::AlignVCenter, qBreakdown.value(1).toString());
        currentX += colWidths[1];
        
        // Operational (Green shade)
        painter.setPen(QColor(46, 125, 50));
        painter.drawText(currentX, y, colWidths[2], 35, Qt::AlignVCenter, qBreakdown.value(2).toString());
        currentX += colWidths[2];
        
        // Service (Red shade)
        painter.setPen(QColor(198, 40, 40));
        painter.drawText(currentX, y, colWidths[3], 35, Qt::AlignVCenter, qBreakdown.value(3).toString());
        
        // Thin separator line
        painter.setPen(QPen(QColor(230, 230, 230), 1));
        painter.drawLine(40, y + 35, W - 40, y + 35);
        
        y += 35;
        alternate = !alternate;
    }
    
    // --- Footer ---
    painter.setFont(QFont("Segoe UI", 8, QFont::StyleItalic));
    painter.setPen(QColor(150, 150, 150));
    painter.drawText(0, H - 40, W, 30, Qt::AlignCenter, 
        "Proprietary & Confidential - HammerDown Maintenance Management System");

    painter.end();
    QMessageBox::information(this, "Success",
        "Professional statistics report generated successfully:\n" + fileName);
}

// =============================================================================
// EMPLOYEE CHAT LOGIC
// =============================================================================

void MainWindow::onChatEnsureTable()
{
    // No longer using database tables to comply with user constraints.
    // We just ensure the path for the JSON file is accessible if needed,
    // but QFile handles creation automatically.
}

void MainWindow::onChatSendMessage()
{
    if (!ui_equipment || currentChatPartnerId == -1) {
        if (currentChatPartnerId == -1)
            QMessageBox::information(this, "Select Employee", "Please select an employee from the list to start chatting.");
        return;
    }
    QString msg = ui_equipment->le_chat_input->text().trimmed();
    bool hasImage = !pendingChatImage.isEmpty();

    if (msg.isEmpty() && !hasImage) {
        shakeWidget(ui_equipment->le_chat_input);
        return;
    }

    if (currentEmployeeId <= 0) {
        QMessageBox::warning(this, "Not Logged In", "You must be logged in to send messages.");
        return;
    }

    // Save common messages for autocomplete
    if (!msg.isEmpty() && msg.length() > 5) {
        QStringList curr = m_completerModel->stringList();
        if (!curr.contains(msg, Qt::CaseInsensitive)) {
            curr << msg;
            if (curr.size() > 50) curr.removeFirst(); // Keep fresh
            m_completerModel->setStringList(curr);
        }
    }

    // Load existing chat JSON
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray chatArray;

    if (file.open(QIODevice::ReadOnly)) {
        chatArray = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    // Create new message object
    QJsonObject msgObj;
    msgObj["sender_id"] = currentEmployeeId;
    msgObj["receiver_id"] = currentChatPartnerId;
    msgObj["message"] = msg.isEmpty() ? QString("[Image]") : msg;
    msgObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    if (hasImage) {
        msgObj["image_data"] = QString::fromLatin1(pendingChatImage.toBase64());
    }
    chatArray.append(msgObj);

    // Save back to file
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(chatArray).toJson());
        file.close();
        
        ui_equipment->le_chat_input->clear();
        pendingChatImage.clear();
        if (auto *lbl = equipmentPage->findChild<QLabel*>("lbl_img_preview"))
            lbl->setVisible(false);
        onChatRefresh();
    } else {
        QMessageBox::critical(this, "Save Error", "Could not save chat to local storage.");
    }
}

void MainWindow::onChatRefresh()
{
    if (!ui_equipment || currentChatPartnerId == -1) return;
    
    // --- Apply Global Theme to Chat Container ---
    if (m_isChatModernTheme) {
        ui_equipment->frame_chat_panel->setStyleSheet(
            "QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1A1208, stop:1 #0A0804); border-radius: 0px 16px 16px 0px; border: 1.5px solid #5A4A32; border-left: none; }"
        );
        ui_equipment->frame_chat_header->setStyleSheet(
            "QFrame#frame_chat_header { background: rgba(30, 20, 10, 0.7); border-bottom: 1px solid #8B6F47; border-radius: 0px 14px 0px 0px; }"
        );
        ui_equipment->frame_input_bar->setStyleSheet(
            "QFrame#frame_input_bar { background: rgba(30, 20, 10, 0.7); border-top: 1px solid #5A4A32; border-radius: 0 0 14px 0; }"
        );
    } else {
        ui_equipment->frame_chat_panel->setStyleSheet(
            "QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2D2416, stop:1 #1A1208); border-radius: 0px 16px 16px 0px; border: 1.5px solid #5A4A32; border-left: none; }"
        );
        ui_equipment->frame_chat_header->setStyleSheet(
            "QFrame#frame_chat_header { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3820); border-bottom: 2px solid #8B6F47; border-radius: 0px 14px 0px 0px; }"
        );
        ui_equipment->frame_input_bar->setStyleSheet(
            "QFrame#frame_input_bar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2A2010,stop:1 #1E1608); border-top: 2px solid #5A4A32; border-radius: 0 0 14px 0; }"
        );
    }

    // Load existing chat JSON
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray allMessages;

    if (file.open(QIODevice::ReadOnly)) {
        allMessages = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    // Clear existing bubble widgets
    static int s_lastRenderedCount = -1;
    static int s_lastPartnerId = -1;
    int currentMsgCount = allMessages.size();

    // Check if we even need to refresh (save CPU and prevent "messy" flickering)
    // We refresh if count changed OR if we switched chat partners
    if (currentMsgCount == s_lastRenderedCount && currentChatPartnerId == s_lastPartnerId && s_lastRenderedCount != -1) {
        return; 
    }
    s_lastRenderedCount = currentMsgCount;
    s_lastPartnerId = currentChatPartnerId;

    QLayoutItem *item;
    while ((item = ui_equipment->verticalLayout_chat_contents->takeAt(0)) != nullptr) {
        if (item->layout()) {
             QLayoutItem *sub;
             while ((sub = item->layout()->takeAt(0)) != nullptr) {
                 if (sub->widget()) sub->widget()->deleteLater();
                 delete sub;
             }
        }
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    ui_equipment->verticalLayout_chat_contents->addStretch();

    // Map to quickly get employee names
    static QMap<int, QPair<QString, QString>> employeeInfo;
    static int s_lastMessageCount = -1;
    
    // Check for notification badge
    if (s_lastMessageCount != -1 && allMessages.size() > s_lastMessageCount && ui_equipment->tabWidget->currentIndex() != 4) {
        // Someone sent a message and we're not on the chat tab
        QJsonObject lastM = allMessages.last().toObject();
        if (lastM["sender_id"].toInt() != currentEmployeeId) { // We didn't send it
            ui_equipment->tabWidget->setTabText(4, QString::fromUtf8("Chat \xF0\x9F\x94\xB4"));
        }
    }
    s_lastMessageCount = allMessages.size();
    
    // Clear badge if on chat tab
    if (ui_equipment->tabWidget->currentIndex() == 4) {
        ui_equipment->tabWidget->setTabText(4, "Chat");
    }

    if (employeeInfo.isEmpty()) { 
        QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM EMPLOYEES");
        while (q.next()) {
            employeeInfo[q.value(0).toInt()] = qMakePair(q.value(1).toString(), q.value(2).toString());
        }
    }

    // Avatar color palette for variety
    static const QStringList avatarColors = {
        "#8B6F47", "#4A7B9D", "#6B9E6B", "#9E6B6B",
        "#7B6B9E", "#9E8B6B", "#6B8B9E", "#9E7B6B"
    };

    // --- Message Loop ---
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();

        // Filter messages for current conversation
        if (!((s_id == currentEmployeeId && r_id == currentChatPartnerId) ||
              (s_id == currentChatPartnerId && r_id == currentEmployeeId))) {
            continue;
        }

        QString msg = m["message"].toString();
        QDateTime dt = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
        bool isMe = (s_id == currentEmployeeId);
        
        QPair<QString, QString> names = employeeInfo.value(s_id, qMakePair(QString("Emp"), QString::number(s_id)));
        QString firstName = names.first;
        QString lastName = names.second;

        QByteArray imgData;
        if (m.contains("image_data")) {
            imgData = QByteArray::fromBase64(m["image_data"].toString().toLatin1());
        }

        // --- Build avatar label ---
        QLabel *avatarLbl = new QLabel();
        avatarLbl->setFixedSize(36, 36);
        avatarLbl->setAlignment(Qt::AlignCenter);
        QString initials = (firstName.isEmpty() ? "?" : firstName.left(1).toUpper())
                         + (lastName.isEmpty()  ? ""  : lastName.left(1).toUpper());
        avatarLbl->setText(initials);
        
        QString avatarStyle;
        if (m_isChatModernTheme) {
            avatarStyle = QString(
                "QLabel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #D4AF37,stop:1 #8B6F47);"
                " color: white; font-size: 13px; font-weight: bold; border-radius: 18px;"
                " border: 2px solid rgba(255,255,255,0.4); box-shadow: 0 4px 8px rgba(0,0,0,0.3); }"
            );
        } else {
            QString avatarColor = avatarColors[s_id % avatarColors.size()];
            avatarStyle = QString(
                "QLabel { background: %1; color: white; font-size: 12px; font-weight: bold;"
                " border-radius: 18px; border: 1.5px solid rgba(255,255,255,0.2); }"
            ).arg(avatarColor);
        }
        avatarLbl->setStyleSheet(avatarStyle);

        // --- Build bubble content ---
        QVBoxLayout *bubbleLayout = new QVBoxLayout();
        bubbleLayout->setSpacing(m_isChatModernTheme ? 4 : 3);
        bubbleLayout->setContentsMargins(0, 0, 0, 0);

        // Sender name label
        if (!isMe) {
            QLabel *nameLabel = new QLabel(firstName + " " + lastName);
            nameLabel->setStyleSheet(m_isChatModernTheme ? 
                "color: #D4AF37; font-size: 11px; font-weight: 800; background: transparent;" :
                "color: #D4AF37; font-size: 10px; font-weight: bold; background: transparent;");
            bubbleLayout->addWidget(nameLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
        }

        // Image (if attached)
        if (!imgData.isEmpty()) {
            QImage img;
            if (img.loadFromData(imgData)) {
                QLabel *imgLabel = new QLabel();
                QPixmap pix = QPixmap::fromImage(img).scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                imgLabel->setPixmap(pix);
                imgLabel->setStyleSheet(m_isChatModernTheme ?
                    "border-radius: 14px; border: 2px solid rgba(139,111,71,0.5); padding: 2px; background: rgba(0,0,0,0.2);" :
                    "border-radius: 10px; padding: 2px;");
                bubbleLayout->addWidget(imgLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
            }
        }

        // GIF (if sent via GIPHY)
        if (m.contains("gif_url")) {
            QString gifUrl = m["gif_url"].toString();
            QString gifTitle = m["gif_title"].toString();
            
            QLabel *gifLabel = new QLabel();
            gifLabel->setFixedSize(280, 200);
            gifLabel->setAlignment(Qt::AlignCenter);
            gifLabel->setStyleSheet(m_isChatModernTheme ?
                "border-radius: 14px; border: 2px solid rgba(139,111,71,0.5); padding: 2px;"
                " background: transparent; " :
                "border-radius: 10px; padding: 2px; background: transparent; ");
            if (!gifTitle.isEmpty())
                gifLabel->setToolTip(gifTitle);
            bubbleLayout->addWidget(gifLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
            
            // Download the GIF and display as animation using QMovie
            QNetworkAccessManager *gifNetMgr = new QNetworkAccessManager(gifLabel);
            QUrl gifDisplayUrl(gifUrl);
            QNetworkReply *gifReply = gifNetMgr->get(QNetworkRequest(gifDisplayUrl));
            connect(gifReply, &QNetworkReply::finished, gifLabel, [gifLabel, gifReply]() {
                if (gifReply->error() == QNetworkReply::NoError) {
                    QByteArray data = gifReply->readAll();
                    
                    QBuffer *buffer = new QBuffer(gifLabel);
                    buffer->setData(data);
                    buffer->open(QIODevice::ReadOnly);
                    
                    QMovie *movie = new QMovie(buffer, QByteArray(), gifLabel);
                    if (movie->isValid()) {
                        gifLabel->setMovie(movie);
                        gifLabel->setText("");
                        movie->setScaledSize(QSize(276, 196));
                        movie->start();
                        gifLabel->show(); // Explicitly show label
                    } else {
                        gifLabel->setText("Invalid GIF");
                        delete movie;
                        delete buffer;
                    }
                } else {
                    gifLabel->setText("Could not load GIF");
                }
                gifReply->deleteLater();
            });
        }

        // Text bubble
        if (!msg.isEmpty() && msg != "[Image]" && msg != "[GIF]") {
            QWidget *bubbleContainer = new QWidget();
            QVBoxLayout *bubbleV = new QVBoxLayout(bubbleContainer);
            bubbleV->setContentsMargins(0, 0, 0, 0);
            bubbleV->setSpacing(4);

            if (msg.contains(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Note"))) {
                VoiceWaveformWidget *waveform = new VoiceWaveformWidget(bubbleContainer);
                bubbleV->addWidget(waveform);
                waveform->startAnim();
                
                QLabel *voiceLabel = new QLabel(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Message - 0:08"));
                voiceLabel->setStyleSheet(m_isChatModernTheme ? "color: #D4AF37; font-weight: 800; font-size: 13px;" : "color: #8B6F47; font-weight: bold;");
                bubbleV->addWidget(voiceLabel);
            } else {
                QLabel *bubble = new QLabel(msg);
                bubble->setWordWrap(true);
                bubble->setMaximumWidth(520);
                bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
                
                if (m_isChatModernTheme) {
                    if (isMe) {
                        bubble->setStyleSheet(
                            "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #D4AF37,stop:1 #A0825A);"
                            " color: #1A1208; border-radius: 18px; border-bottom-right-radius: 4px;"
                            " padding: 10px 16px; font-size: 14px; font-weight: 600;"
                            " border: 1px solid rgba(255,255,255,0.3); ");
                    } else {
                        bubble->setStyleSheet(
                            "background: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47;"
                            " color: #F0E0C0; border-radius: 18px; border-bottom-left-radius: 4px;"
                            " padding: 10px 16px; font-size: 14px; ");
                    }
                } else {
                    if (isMe) {
                        bubble->setStyleSheet(
                            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #C4973A,stop:1 #D4AF37);"
                            " color: #1A1000; border-radius: 14px; border-bottom-right-radius: 3px;"
                            " padding: 9px 13px; font-size: 13px; font-weight: 500;");
                    } else {
                        bubble->setStyleSheet(
                            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3A22);"
                            " color: #EDD9B0; border-radius: 14px; border-bottom-left-radius: 3px;"
                            " padding: 9px 13px; font-size: 13px;");
                    }
                }
                bubbleV->addWidget(bubble);
            }
            bubbleLayout->addWidget(bubbleContainer, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
        }

        // --- Delete Button (Trash Icon) ---
        if (isMe) {
            QPushButton *delBtn = new QPushButton("🗑"); 
            delBtn->setFixedSize(24, 24);
            delBtn->setCursor(Qt::PointingHandCursor);
            delBtn->setStyleSheet(m_isChatModernTheme ?
                "QPushButton { background: rgba(200,50,50,0.1); border: 1px solid rgba(200,50,50,0.3); color: #FF7070; font-size: 14px; border-radius: 12px; }"
                "QPushButton:hover { background: #CC3333; color: white; border-color: white; }" :
                "QPushButton { background: rgba(255,0,0,0.1); border: none; color: #cc4444; font-size: 14px; border-radius: 11px; }"
                "QPushButton:hover { background: #aa3333; color: white; }");
            delBtn->setToolTip("Delete message");
            connect(delBtn, &QPushButton::clicked, this, [this, i]() { onChatDeleteMessage(i); });
            bubbleLayout->addWidget(delBtn, 0, Qt::AlignRight);
        }

        // Nicely formatted timestamp
        QDateTime now = QDateTime::currentDateTime();
        QString timeStr;
        if (dt.date() == now.date()) {
            timeStr = "Today at " + dt.toString("HH:mm");
        } else if (dt.date() == now.date().addDays(-1)) {
            timeStr = "Yesterday at " + dt.toString("HH:mm");
        } else {
            timeStr = dt.toString("dd/MM/yyyy") + " at " + dt.toString("HH:mm");
        }

        QLabel *infoLabel = new QLabel(timeStr);
        infoLabel->setStyleSheet(m_isChatModernTheme ?
            "color: rgba(212,175,55,0.8); font-size: 10px; font-weight: bold; background: transparent;" :
            "color: rgba(180,160,120,0.7); font-size: 10px; background: transparent;");
        bubbleLayout->addWidget(infoLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);

        // --- Assemble row ---
        QWidget *row = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(8, 4, 8, 4);
        rowLayout->setSpacing(10);

        if (isMe) {
            rowLayout->addStretch();
            rowLayout->addLayout(bubbleLayout);
            rowLayout->addWidget(avatarLbl, 0, Qt::AlignBottom);
        } else {
            rowLayout->addWidget(avatarLbl, 0, Qt::AlignBottom);
            rowLayout->addLayout(bubbleLayout);
            rowLayout->addStretch();
        }

        ui_equipment->verticalLayout_chat_contents->addWidget(row);

        // --- Modern Staggered Animation ---
        if (m_isChatModernTheme) {
            QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(row);
            row->setGraphicsEffect(opacity);
            
            QPropertyAnimation *anim = new QPropertyAnimation(opacity, "opacity");
            anim->setDuration(400);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            
            QPropertyAnimation *posAnim = new QPropertyAnimation(row, "pos");
            posAnim->setDuration(500);
            posAnim->setStartValue(QPoint(row->pos().x(), row->pos().y() + 30));
            posAnim->setEndValue(row->pos());
            posAnim->setEasingCurve(QEasingCurve::OutBack);

            // Use a relative index for animations so that older messages aren't delayed indefinitely
            // We'll use the last 20 messages or so for animations, or just limit the max delay.
            int animIdx = qMax(0, i - (allMessages.size() - 15)); 
            QTimer::singleShot(animIdx * 60, [anim, posAnim](){
                anim->start(QAbstractAnimation::DeleteWhenStopped);
                posAnim->start(QAbstractAnimation::DeleteWhenStopped);
            });
        } else {
            row->show();
        }
    }
    
    // AI Summarization button removed due to stability issues.
    
    // Final scroll to bottom - only if user was near the bottom or new message came in
    QScrollBar *vBar = ui_equipment->scrollArea_chat->verticalScrollBar();
    bool wasNearBottom = vBar->value() > (vBar->maximum() - 100);
    
    if (wasNearBottom || allMessages.size() > s_lastMessageCount) {
        QTimer::singleShot(100, this, [this](){
            if (ui_equipment && ui_equipment->scrollArea_chat) {
                QScrollBar *bar = ui_equipment->scrollArea_chat->verticalScrollBar();
                bar->setValue(bar->maximum());
            }
        });
    }
}


void MainWindow::onChatEmployeeListRefresh()
{
    if (!ui_equipment) return;
    ui_equipment->list_employees->clear();
    
    QSqlQuery q("SELECT FIRST_NAME, LAST_NAME, EMPLOYEE_ID, JOB_TITLE FROM EMPLOYEES ORDER BY FIRST_NAME ASC");
    while (q.next()) {
        QString name = q.value(0).toString() + " " + q.value(1).toString();
        int id = q.value(2).toInt();
        QString title = q.value(3).toString();
        
        if (id == currentEmployeeId) continue; // Don't chat with self in list

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(name + " (ID: " + QString::number(id) + ")");
        item->setData(Qt::UserRole, id);
        item->setToolTip(title);
        ui_equipment->list_employees->addItem(item);
        
        if (id == currentChatPartnerId) {
            item->setSelected(true);
            ui_equipment->list_employees->setCurrentItem(item);
        }
    }
}

void MainWindow::onChatAttachImage()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Attach Image", QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (filePath.isEmpty()) return;

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open the selected image.");
        return;
    }
    pendingChatImage = f.readAll();
    f.close();

    // Show preview label
    QFileInfo fi(filePath);
    if (auto *lbl = equipmentPage->findChild<QLabel*>("lbl_img_preview")) {
        lbl->setText("Image attached: " + fi.fileName() + " — press Send to include it");
        lbl->setVisible(true);
    }
}

void MainWindow::onChatEmployeeSelected(QListWidgetItem *item)
{
    if (!item) return;
    currentChatPartnerId = item->data(Qt::UserRole).toInt();

    QString displayName = item->text().split("(").first().trimmed();

    // Update header: name label
    ui_equipment->lbl_active_employees->setText(displayName);

    // Update avatar initials in header
    if (auto *avatar = equipmentPage->findChild<QLabel*>("lbl_partner_avatar")) {
        QStringList parts = displayName.split(' ', Qt::SkipEmptyParts);
        QString initials;
        for (const QString &p : parts) initials += p.left(1).toUpper();
        initials = initials.left(2);
        avatar->setText(initials);
    }
    
    // Calculate last seen
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonArray allMessages = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
        QDateTime lastSeenTime;
        for (int i = allMessages.size() - 1; i >= 0; --i) {
            QJsonObject m = allMessages[i].toObject();
            if (m["sender_id"].toInt() == currentChatPartnerId) {
                lastSeenTime = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
                break;
            }
        }
        if (lastSeenTime.isValid()) {
            QDateTime now = QDateTime::currentDateTime();
            QString lsStr = "Last seen: ";
            if (lastSeenTime.date() == now.date()) {
                lsStr += "today at " + lastSeenTime.toString("HH:mm");
            } else if (lastSeenTime.date() == now.date().addDays(-1)) {
                lsStr += "yesterday at " + lastSeenTime.toString("HH:mm");
            } else {
                lsStr += lastSeenTime.toString("dd/MM/yyyy") + " at " + lastSeenTime.toString("HH:mm");
            }
            ui_equipment->lbl_chat_status->setText(lsStr);
            ui_equipment->lbl_chat_status->setStyleSheet("QLabel { color: rgba(255,255,255,0.6); font-size: 11px; background: transparent; border: none; padding: 0; }");
        } else {
            ui_equipment->lbl_chat_status->setText("Online");
            ui_equipment->lbl_chat_status->setStyleSheet("QLabel { color: #4CAF50; font-size: 11px; background: transparent; border: none; padding: 0; }");
        }
    }

    // Restart timer
    chatRefreshTimer->start(3000);

    // Clear Notification Badge if it was this partner
    // Assuming UI handles badge clearing upon entering chat or refreshing it.
    ui_equipment->tabWidget->setTabText(4, "Chat");

    onChatRefresh();
}

void MainWindow::onChatDeleteMessage(int index)
{
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray chatArray;

    if (file.open(QIODevice::ReadOnly)) {
        chatArray = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    if (index >= 0 && index < chatArray.size()) {
        // --- Backup deleted message to trash file ---
        QJsonObject deletedMsg = chatArray[index].toObject();
        deletedMsg["deleted_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        QFile trashFile("hammerdown_chat_trash.json");
        QJsonArray trashArray;
        if (trashFile.open(QIODevice::ReadOnly)) {
            trashArray = QJsonDocument::fromJson(trashFile.readAll()).array();
            trashFile.close();
        }
        trashArray.append(deletedMsg);
        if (trashFile.open(QIODevice::WriteOnly)) {
            trashFile.write(QJsonDocument(trashArray).toJson());
            trashFile.close();
        }
        
        chatArray.removeAt(index);
        
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(chatArray).toJson());
            file.close();
            onChatRefresh();
        }
    }
}

// =============================================================================
// EMOJI PICKER
// =============================================================================
void MainWindow::onChatEmojiClicked()
{
    QDialog *picker = new QDialog(this);
    picker->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    picker->setAttribute(Qt::WA_TranslucentBackground);
    picker->setFixedSize(360, 320);

    QFrame *card = new QFrame(picker);
    card->setObjectName("emojiCard");
    card->setGeometry(0, 0, 360, 320);
    card->setStyleSheet(
        "QFrame#emojiCard { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
        " border: 2px solid #8B6F47; border-radius: 16px; }");

    QVBoxLayout *mainLay = new QVBoxLayout(card);
    mainLay->setContentsMargins(12, 10, 12, 10);
    mainLay->setSpacing(6);

    QLabel *title = new QLabel("Emoji Picker", card);
    title->setStyleSheet("color: #D4AF37; font-size: 14px; font-weight: bold; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(title);

    // Emoji categories
    struct EmojiCategory { QString name; QStringList emojis; };
    QList<EmojiCategory> categories = {
        {"Smileys", {
            "\xF0\x9F\x98\x80", "\xF0\x9F\x98\x82", "\xF0\x9F\x98\x8D", "\xF0\x9F\x98\x8E", 
            "\xF0\x9F\x98\xAD", "\xF0\x9F\x98\xA1", "\xF0\x9F\x98\xB1", "\xF0\x9F\x98\xB4",
            "\xF0\x9F\x98\x98", "\xF0\x9F\x98\x9C", "\xF0\x9F\x98\x8C", "\xF0\x9F\x98\xA2",
            "\xF0\x9F\x98\x83", "\xF0\x9F\x98\x84", "\xF0\x9F\x98\x85", "\xF0\x9F\x98\x89"
        }},
        {"Hands", {
            "\xF0\x9F\x91\x8D", "\xF0\x9F\x91\x8E", "\xF0\x9F\x91\x8B", "\xE2\x9C\x8C",
            "\xF0\x9F\x91\x8F", "\xF0\x9F\x99\x8C", "\xF0\x9F\x92\xAA", "\xF0\x9F\xA4\x9D",
            "\xF0\x9F\x91\x86", "\xF0\x9F\x91\x87", "\xE2\x9C\x8A", "\xF0\x9F\xA4\x9E",
            "\xF0\x9F\x99\x8F", "\xF0\x9F\x91\x8C", "\xE2\x9C\x8B", "\xF0\x9F\xA4\x99"
        }},
        {"Objects", {
            "\xE2\x9D\xA4", "\xF0\x9F\x94\xA5", "\xE2\xAD\x90", "\xF0\x9F\x8E\x89",
            "\xF0\x9F\x92\xAF", "\xF0\x9F\x92\xAF", "\xF0\x9F\x91\x80", "\xF0\x9F\x92\xA1",
            "\xF0\x9F\x94\xA8", "\xF0\x9F\xAA\x9A", "\xF0\x9F\xAA\xB5", "\xF0\x9F\xAA\x93",
            "\xF0\x9F\x9B\xA0", "\xE2\x9A\x99", "\xF0\x9F\x93\x8B", "\xE2\x9C\x85"
        }}
    };

    QScrollArea *scroll = new QScrollArea(card);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #5A4A32; border-radius: 2px; }");
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLay = new QVBoxLayout(scrollContent);
    scrollLay->setSpacing(8);
    scrollLay->setContentsMargins(4, 4, 4, 4);

    for (const auto &cat : categories) {
        QLabel *catLabel = new QLabel(cat.name, scrollContent);
        catLabel->setStyleSheet("color: #8B6F47; font-size: 11px; font-weight: bold; background: transparent;");
        scrollLay->addWidget(catLabel);

        QWidget *grid = new QWidget(scrollContent);
        QGridLayout *gridLay = new QGridLayout(grid);
        gridLay->setSpacing(4);
        gridLay->setContentsMargins(0, 0, 0, 0);

        for (int i = 0; i < cat.emojis.size(); ++i) {
            QPushButton *btn = new QPushButton(QString::fromUtf8(cat.emojis[i].toUtf8()), grid);
            btn->setFixedSize(36, 36);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(
                "QPushButton { background: rgba(255,255,255,0.05); border: 1px solid rgba(139,111,71,0.3);"
                " border-radius: 8px; font-size: 20px; }"
                "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }");
            gridLay->addWidget(btn, i / 8, i % 8);

            connect(btn, &QPushButton::clicked, this, [this, btn, picker]() {
                if (ui_equipment)
                    ui_equipment->le_chat_input->insert(btn->text());
                picker->close();
            });
        }
        scrollLay->addWidget(grid);
    }
    scrollLay->addStretch();
    scroll->setWidget(scrollContent);
    mainLay->addWidget(scroll, 1);

    // Position above the GIF button
    QPushButton *emojiBtn = equipmentPage->findChild<QPushButton*>("btn_chat_emoji");
    if (emojiBtn) {
        QPoint pos = emojiBtn->mapToGlobal(QPoint(0, -picker->height() - 8));
        picker->move(pos);
    }
    picker->show();
}

// =============================================================================
// GIF PICKER (GIPHY API)
// =============================================================================
void MainWindow::onChatGifClicked()
{
    if (currentChatPartnerId == -1) {
        QMessageBox::information(this, "Select Employee", "Please select an employee first to send a GIF.");
        return;
    }

    QDialog *gifDialog = new QDialog(this);
    gifDialog->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    gifDialog->setAttribute(Qt::WA_TranslucentBackground);
    gifDialog->setFixedSize(420, 480);

    QFrame *card = new QFrame(gifDialog);
    card->setObjectName("gifCard");
    card->setGeometry(0, 0, 420, 480);
    card->setStyleSheet(
        "QFrame#gifCard { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
        " border: 2px solid #8B6F47; border-radius: 16px; }");

    QVBoxLayout *mainLay = new QVBoxLayout(card);
    mainLay->setContentsMargins(12, 12, 12, 12);
    mainLay->setSpacing(8);

    // Header
    QLabel *title = new QLabel("GIF Search — Powered by GIPHY", card);
    title->setStyleSheet("color: #D4AF37; font-size: 13px; font-weight: bold; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(title);

    // Search bar
    QHBoxLayout *searchLay = new QHBoxLayout();
    QLineEdit *searchInput = new QLineEdit(card);
    searchInput->setPlaceholderText("Search GIFs...");
    searchInput->setStyleSheet(
        "QLineEdit { background: rgba(255,255,255,0.08); border: 1.5px solid #5A4A32;"
        " border-radius: 14px; padding: 6px 14px; color: #F0E0C0; font-size: 13px; }"
        "QLineEdit:focus { border-color: #D4AF37; }");
    QPushButton *searchBtn = new QPushButton("Search", card);
    searchBtn->setFixedHeight(32);
    searchBtn->setCursor(Qt::PointingHandCursor);
    searchBtn->setStyleSheet(
        "QPushButton { background: #8B6F47; color: white; border-radius: 14px; padding: 0 16px;"
        " font-weight: bold; font-size: 12px; border: none; }"
        "QPushButton:hover { background: #A0825A; }");
    searchLay->addWidget(searchInput, 1);
    searchLay->addWidget(searchBtn);
    mainLay->addLayout(searchLay);

    // Loading indicator
    QLabel *loadingLabel = new QLabel("Type something and press Search, or browse trending GIFs below.", card);
    loadingLabel->setObjectName("gifLoadingLabel");
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setWordWrap(true);
    loadingLabel->setStyleSheet("color: #8B6F47; font-size: 11px; font-style: italic; background: transparent;");
    mainLay->addWidget(loadingLabel);

    // GIF grid in scroll area
    QScrollArea *scroll = new QScrollArea(card);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 5px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #5A4A32; border-radius: 2px; }");
    
    QWidget *gridWidget = new QWidget();
    gridWidget->setObjectName("gifGridWidget");
    QGridLayout *gridLay = new QGridLayout(gridWidget);
    gridLay->setSpacing(6);
    gridLay->setContentsMargins(4, 4, 4, 4);
    scroll->setWidget(gridWidget);
    mainLay->addWidget(scroll, 1);

    // GIPHY API key
    QString giphyKey = "Rb870UMsk9bec2cUYjWBzwbFsCaUOJN6";

    // Lambda to populate results
    auto populateGifs = [this, gridWidget, gridLay, gifDialog, loadingLabel](QNetworkReply *reply) {
        // Clear old results
        QLayoutItem *item;
        while ((item = gridLay->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }

        if (reply->error() != QNetworkReply::NoError) {
            loadingLabel->setText("Error loading GIFs: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray data = doc.object()["data"].toArray();
        reply->deleteLater();

        if (data.isEmpty()) {
            loadingLabel->setText("No GIFs found. Try a different search term.");
            return;
        }
        loadingLabel->hide(); // Hide the "found" text as requested

        for (int i = 0; i < qMin(data.size(), 50); ++i) {
            QJsonObject gif = data[i].toObject();
            QJsonObject fixedHeight = gif["images"].toObject()["fixed_height_small"].toObject();
            QString previewUrl = fixedHeight["url"].toString();
            QString fullUrl = gif["images"].toObject()["original"].toObject()["url"].toString();
            QString gifTitle = gif["title"].toString();

            // Create a clickable label for each GIF thumbnail
            QPushButton *gifBtn = new QPushButton(gridWidget);
            gifBtn->setFixedSize(120, 90);
            gifBtn->setCursor(Qt::PointingHandCursor);
            gifBtn->setToolTip(gifTitle);
            gifBtn->setStyleSheet(
                "QPushButton { background: rgba(255,255,255,0.05); border: 1.5px solid rgba(139,111,71,0.3);"
                " border-radius: 8px; }"
                "QPushButton:hover { border-color: #D4AF37; background: rgba(212,175,55,0.15); }");
            gridLay->addWidget(gifBtn, i / 3, i % 3);

            // Download preview thumbnail (animated)
            QNetworkAccessManager *thumbManager = new QNetworkAccessManager(gifBtn);
            QUrl thumbUrl(previewUrl);
            QNetworkReply *thumbReply = thumbManager->get(QNetworkRequest(thumbUrl));
            
            connect(thumbReply, &QNetworkReply::finished, gifBtn, [gifBtn, thumbReply]() {
                if (thumbReply->error() == QNetworkReply::NoError) {
                    QByteArray imgData = thumbReply->readAll();
                    
                    QLabel *movieLabel = new QLabel(gifBtn);
                    movieLabel->setFixedSize(116, 86);
                    movieLabel->move(2, 2);
                    movieLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
                    
                    QBuffer *buffer = new QBuffer(movieLabel);
                    buffer->setData(imgData);
                    buffer->open(QIODevice::ReadOnly);
                    
                    QMovie *movie = new QMovie(buffer, QByteArray(), movieLabel);
                    if (movie->isValid()) {
                        movieLabel->setMovie(movie);
                        movie->setScaledSize(QSize(116, 86));
                        movie->start();
                        movieLabel->show(); // This was missing
                    }
                }
                thumbReply->deleteLater();
            });

            // On click: send GIF as a message
            connect(gifBtn, &QPushButton::clicked, this, [this, fullUrl, gifTitle, gifDialog]() {
                // Save as a special GIF message in JSON
                QString chatFilePath = "hammerdown_chat.json";
                QFile file(chatFilePath);
                QJsonArray chatArray;
                if (file.open(QIODevice::ReadOnly)) {
                    chatArray = QJsonDocument::fromJson(file.readAll()).array();
                    file.close();
                }

                QJsonObject msgObj;
                msgObj["sender_id"] = currentEmployeeId;
                msgObj["receiver_id"] = currentChatPartnerId;
                msgObj["message"] = "[GIF]";
                msgObj["gif_url"] = fullUrl;
                msgObj["gif_title"] = gifTitle;
                msgObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                chatArray.append(msgObj);

                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(chatArray).toJson());
                    file.close();
                }
                gifDialog->close();
                onChatRefresh();
            });
        }
    };

    // Shared network manager for search requests
    QNetworkAccessManager *searchNetMgr = new QNetworkAccessManager(gifDialog);

    // Search handler
    auto doSearch = [searchInput, giphyKey, searchNetMgr, populateGifs, loadingLabel]() {
        QString query = searchInput->text().trimmed();
        QString urlStr;
        if (query.isEmpty()) {
            urlStr = QString("https://api.giphy.com/v1/gifs/trending?api_key=%1&limit=50&rating=g").arg(giphyKey);
        } else {
            urlStr = QString("https://api.giphy.com/v1/gifs/search?api_key=%1&q=%2&limit=50&rating=g")
                         .arg(giphyKey, QUrl::toPercentEncoding(query));
        }
        loadingLabel->setText("Loading GIFs...");
        QUrl searchUrl(urlStr);
        QNetworkReply *reply = searchNetMgr->get(QNetworkRequest(searchUrl));
        QObject::connect(reply, &QNetworkReply::finished, [reply, populateGifs]() {
            populateGifs(reply);
        });
    };

    connect(searchBtn, &QPushButton::clicked, gifDialog, doSearch);
    connect(searchInput, &QLineEdit::returnPressed, gifDialog, doSearch);

    // Load trending GIFs on open
    {
        QString trendingUrl = QString("https://api.giphy.com/v1/gifs/trending?api_key=%1&limit=50&rating=g").arg(giphyKey);
        loadingLabel->setText("Loading trending GIFs...");
        QUrl trendUrl(trendingUrl);
        QNetworkReply *reply = searchNetMgr->get(QNetworkRequest(trendUrl));
        connect(reply, &QNetworkReply::finished, [reply, populateGifs]() {
            populateGifs(reply);
        });
    }

    // Position above GIF button
    QPushButton *gifBtnUi = equipmentPage->findChild<QPushButton*>("btn_chat_gif");
    if (gifBtnUi) {
        QPoint pos = gifBtnUi->mapToGlobal(QPoint(-gifDialog->width()/2 + gifBtnUi->width()/2, -gifDialog->height() - 8));
        gifDialog->move(pos);
    }
    gifDialog->show();
    searchInput->setFocus();
}

void MainWindow::onChatSettingsClicked()
{
    // Apply blur effect to background
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(10.0);
    this->setGraphicsEffect(blur);

    QDialog *dialog = new QDialog(this);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setModal(true);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    dialog->setFixedSize(320, 360); // Adjusted height

    QFrame *card = new QFrame(dialog);
    card->setObjectName("settingsCard");
    card->setGeometry(0, 0, 320, 360);
    card->setStyleSheet(
        "QFrame#settingsCard { "
        " background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
        " border: 2.5px solid #8B6F47; border-radius: 24px; color: #F0E0C0; "
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 28, 24, 24);
    layout->setSpacing(18);

    QLabel *title = new QLabel("Chat Settings", card);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #D4AF37; margin-bottom: 8px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // --- Classical Music Volume ---
    QLabel *volLbl = new QLabel("Ambient Volume", card);
    volLbl->setStyleSheet("color: #B8925A; font-size: 13px; font-weight: bold;");
    layout->addWidget(volLbl);

    QSlider *volSlider = new QSlider(Qt::Horizontal, card);
    volSlider->setRange(0, 100);
    volSlider->setValue(chatAudioOutput->volume() * 100); 
    volSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 6px; background: #3A2D1A; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 16px; height: 16px; margin: -5px 0; border-radius: 8px; background: #D4AF37; }"
    );
    layout->addWidget(volSlider);
    connect(volSlider, &QSlider::valueChanged, this, [this](int val) {
        chatAudioOutput->setVolume(val / 100.0); 
    });

    // --- Theme Selection Section Removed ---
    // (Modern theme is hidden, Classic is active)

    // --- Clear Chat ---
    QPushButton *btnClear = new QPushButton("🗑 Clear Conversation", card);
    btnClear->setStyleSheet(
        "QPushButton { background: rgba(200,50,50,0.15); border: 2px solid #662222; border-radius: 14px; height: 40px; color: #FF9999; font-weight: bold; }"
        "QPushButton:hover { background: #882222; color: white; }"
    );
    layout->addWidget(btnClear);
    connect(btnClear, &QPushButton::clicked, this, [this, dialog]() {
        if (QMessageBox::question(dialog, "Clear", "Wipe all messages for this chat?") == QMessageBox::Yes) {
            // Simple logic: remove all entries where sender or receiver is this conversation pair
            QString chatFilePath = "hammerdown_chat.json";
            QFile file(chatFilePath);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonArray oldArr = QJsonDocument::fromJson(file.readAll()).array();
                file.close();
                QJsonArray newArr;
                for (int i=0; i<oldArr.size(); ++i) {
                    QJsonObject m = oldArr[i].toObject();
                    int s = m["sender_id"].toInt();
                    int r = m["receiver_id"].toInt();
                    if (!((s == currentEmployeeId && r == currentChatPartnerId) ||
                          (s == currentChatPartnerId && r == currentEmployeeId))) {
                        newArr.append(m);
                    }
                }
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(newArr).toJson());
                    file.close();
                    onChatRefresh();
                    dialog->accept();
                }
            }
        }
    });

    // --- Restore Deleted Messages ---
    QPushButton *btnRestore = new QPushButton("♻ Restore Deleted Messages", card);
    btnRestore->setStyleSheet(
        "QPushButton { background: rgba(50,200,50,0.15); border: 2px solid #226622; border-radius: 14px; height: 40px; color: #99FF99; font-weight: bold; }"
        "QPushButton:hover { background: #228822; color: white; }"
    );
    layout->addWidget(btnRestore);
    connect(btnRestore, &QPushButton::clicked, this, [this]() {
        QDialog *restoreDialog = new QDialog(this);
        restoreDialog->setWindowTitle("Restore Messages");
        restoreDialog->setFixedSize(500, 400);
        restoreDialog->setStyleSheet(
            "QDialog { background: #1A140A; border: 2px solid #8B6F47; border-radius: 12px; }");
        
        QVBoxLayout *rLay = new QVBoxLayout(restoreDialog);
        QLabel *title = new QLabel("Select messages to restore:", restoreDialog);
        title->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 14px;");
        rLay->addWidget(title);
        
        QListWidget *list = new QListWidget(restoreDialog);
        list->setStyleSheet("QListWidget { background: rgba(0,0,0,0.3); color: #F0E0C0; border: 1px solid #5A4A32; border-radius: 8px; }"
                            "QListWidget::item { padding: 8px; border-bottom: 1px solid rgba(139,111,71,0.2); }");
        
        QFile trashFile("hammerdown_chat_trash.json");
        QJsonArray trashArray;
        if (trashFile.open(QIODevice::ReadOnly)) {
            trashArray = QJsonDocument::fromJson(trashFile.readAll()).array();
            trashFile.close();
        }
        
        for (int i = trashArray.size() - 1; i >= 0; --i) {
            QJsonObject m = trashArray[i].toObject();
            QString preview = m["message"].toString();
            if (preview == "[GIF]") preview = "GIF: " + m["gif_title"].toString();
            if (preview == "[Image]") preview = "Attached Image";
            
            QListWidgetItem *item = new QListWidgetItem(QString("[%1] %2").arg(m["timestamp"].toString().mid(11,5), preview));
            item->setData(Qt::UserRole, i);
            list->addItem(item);
        }
        rLay->addWidget(list);
        
        QPushButton *btnDoRestore = new QPushButton("Restore Selected", restoreDialog);
        btnDoRestore->setStyleSheet("background: #8B6F47; color: white; height: 36px; border-radius: 18px; font-weight: bold;");
        rLay->addWidget(btnDoRestore);
        
        connect(btnDoRestore, &QPushButton::clicked, this, [this, list, trashArray, restoreDialog]() {
            if (!list->currentItem()) return;
            int idxInTrash = list->currentItem()->data(Qt::UserRole).toInt();
            QJsonObject toRestore = trashArray[idxInTrash].toObject();
            toRestore.remove("deleted_at");
            
            // Move back to main chat
            QString chatFilePath = "hammerdown_chat.json";
            QFile file(chatFilePath);
            QJsonArray chatArray;
            if (file.open(QIODevice::ReadOnly)) {
                chatArray = QJsonDocument::fromJson(file.readAll()).array();
                file.close();
            }
            chatArray.append(toRestore);
            // Sort by timestamp if possible, but for now just append
            if (file.open(QIODevice::WriteOnly)) {
                file.write(QJsonDocument(chatArray).toJson());
                file.close();
            }
            
            // Remove from trash
            QJsonArray newTrash = trashArray;
            newTrash.removeAt(idxInTrash);
            QFile tf("hammerdown_chat_trash.json");
            if (tf.open(QIODevice::WriteOnly)) {
                tf.write(QJsonDocument(newTrash).toJson());
                tf.close();
            }
            
            onChatRefresh();
            restoreDialog->accept();
            QMessageBox::information(this, "Restored", "Message moved back to conversation.");
        });
        
        restoreDialog->exec();
    });

    layout->addStretch();

    QPushButton *closeBtn = new QPushButton("Save & Close", card);
    closeBtn->setStyleSheet(
        "QPushButton { background: #D4AF37; color: #1A1208; border-radius: 16px; height: 44px; font-weight: bold; font-size: 15px; }"
        "QPushButton:hover { background: #E5C060; }"
    );
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    // --- Animation ---
    card->setGraphicsEffect(nullptr);
    QPropertyAnimation *anim = new QPropertyAnimation(dialog, "windowOpacity");
    anim->setDuration(350);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    dialog->setWindowOpacity(0.0);
    dialog->show();
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    dialog->exec();

    // Cleanup blur
    this->setGraphicsEffect(nullptr);
    delete dialog;
}

void MainWindow::setupChatForgeVisuals()
{
    // Intentionally minimal: keep current chat visual behavior unchanged.
}

void MainWindow::enforceChatTabTopOffset()
{
    // Intentionally minimal: legacy hook kept for linker compatibility.
}

void MainWindow::onWeatherAssistantClicked()
{
    if (!weatherAssistant) {
        weatherAssistant = new WeatherAssistant(equipmentPage);
    }
    
    // Simple toggle logic
    if (weatherAssistant->isVisible()) {
        weatherAssistant->hideAnimated();
    } else {
        const bool playWeatherVideoIntro = (homeWindow && homeWindow->isAnimationMode());

        weatherAssistant->refreshWeather();

        auto showAssistantAndResumeMusic = [this]() {
            weatherAssistant->showAnimated();
        };

        if (!playWeatherVideoIntro) {
            showAssistantAndResumeMusic();
            return;
        }
        
        // --- Full Screen Weather Report Video Intro ---
        QFrame *introFrame = new QFrame(equipmentPage);
        introFrame->setGeometry(equipmentPage->rect());
        introFrame->setStyleSheet("background-color: black; border-radius: 12px;");
        
        QVBoxLayout *introLayout = new QVBoxLayout(introFrame);
        introLayout->setContentsMargins(0, 0, 0, 0);
        
        QVideoWidget *videoWidget = new QVideoWidget(introFrame);
        introLayout->addWidget(videoWidget);
        
        QMediaPlayer *player = new QMediaPlayer(introFrame);
        QAudioOutput *audioOutput = new QAudioOutput(introFrame);
        player->setAudioOutput(audioOutput);
        audioOutput->setVolume(currentVolume);
        player->setVideoOutput(videoWidget);
        // Keep volume in sync with settings slider while video plays
        connect(this, &MainWindow::audioVolumeChanged, audioOutput, &QAudioOutput::setVolume);
        
        // Find local path for weather.mp4
        QString appDir = QCoreApplication::applicationDirPath();
        QString videoPath = appDir + "/../../../assets/weather.mp4"; 
        if (!QFile::exists(videoPath)) videoPath = appDir + "/../../assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = appDir + "/assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/../assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/../../assets/weather.mp4";

        player->setSource(QUrl::fromLocalFile(QFileInfo(videoPath).absoluteFilePath()));

        introFrame->show();
        introFrame->raise();

        // Fade-out overlay that sits above the video widget
        QFrame *fadeOverlay = new QFrame(introFrame);
        fadeOverlay->setGeometry(0, 0, introFrame->width(), introFrame->height());
        fadeOverlay->setStyleSheet("background: black;");
        QGraphicsOpacityEffect *overlayOpacity = new QGraphicsOpacityEffect(fadeOverlay);
        fadeOverlay->setGraphicsEffect(overlayOpacity);
        overlayOpacity->setOpacity(1.0);
        fadeOverlay->show();
        fadeOverlay->raise();

        // Fade the black overlay OUT to reveal the video
        QPropertyAnimation *fadeInAnim = new QPropertyAnimation(overlayOpacity, "opacity", fadeOverlay);
        fadeInAnim->setDuration(600);
        fadeInAnim->setStartValue(1.0);
        fadeInAnim->setEndValue(0.0);
        fadeInAnim->start(QAbstractAnimation::DeleteWhenStopped);

        connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia) {
                // Fade the overlay back IN to black, then show weather assistant
                fadeOverlay->raise();
                QGraphicsOpacityEffect *outOpacity = new QGraphicsOpacityEffect(fadeOverlay);
                fadeOverlay->setGraphicsEffect(outOpacity);
                outOpacity->setOpacity(0.0);

                QPropertyAnimation *fadeOutAnim = new QPropertyAnimation(outOpacity, "opacity", fadeOverlay);
                fadeOutAnim->setDuration(400);
                fadeOutAnim->setStartValue(0.0);
                fadeOutAnim->setEndValue(1.0);

                connect(fadeOutAnim, &QPropertyAnimation::finished, this, [=]() {
                    introFrame->deleteLater();
                    showAssistantAndResumeMusic();
                });
                fadeOutAnim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        });

        player->play();
    }
}

void MainWindow::onEquipmentBulkUpdateStatus()
{
    if (!ui_equipment || !ui_equipment->table_equipments->selectionModel()) return;

    QItemSelectionModel *selection = ui_equipment->table_equipments->selectionModel();
    QModelIndexList selectedRowsIndices = selection->selectedRows();

    if (selectedRowsIndices.isEmpty()) {
        QMessageBox::warning(this, trKey("No Selection"), trKey("Please select at least one equipment in the table."));
        return;
    }

    QString newStatus = ui_equipment->cb_bulk_status->currentText();
    QString confirmMsg = QString(trKey("Are you sure you want to change the status of %1 items to '%2'?"))
                             .arg(selectedRowsIndices.size())
                             .arg(newStatus);
    
    if (QMessageBox::question(this, trKey("Bulk Update"), confirmMsg) != QMessageBox::Yes) {
        return;
    }

    int successCount = 0;
    int failCount = 0;

    for (const QModelIndex &index : selectedRowsIndices) {
        // ID is in the third column (index 2) as defined in onEquipmentRefreshView
        QString id = index.siblingAtColumn(2).data().toString();
        
        QSqlQuery q;
        q.prepare("UPDATE EQUIPMENT SET STATUS = :status WHERE EQUIPMENT_ID = :id");
        q.bindValue(":status", newStatus);
        q.bindValue(":id", id);
        
        if (q.exec()) {
            successCount++;
        } else {
            failCount++;
        }
    }

    onEquipmentRefreshView();
    
    if (failCount == 0) {
        QMessageBox::information(this, trKey("Success"), 
            QString(trKey("Successfully updated status to '%1' for %2 equipment items."))
                .arg(newStatus).arg(successCount));
    } else {
        QMessageBox::warning(this, trKey("Partial Success"), 
            QString(trKey("Updated %1 items, but %2 failed. Check database logs."))
                .arg(successCount).arg(failCount));
    }
}

void MainWindow::onEquipmentDeleteAll()
{
    if (!ui_equipment) return;

    int visibleCount = 0;
    if (ui_equipment->table_equipments && ui_equipment->table_equipments->model()) {
        visibleCount = ui_equipment->table_equipments->model()->rowCount();
    }

    int count = 0;
    QSqlQuery qCount;
    const bool gotDbCount = qCount.exec("SELECT COUNT(*) FROM EQUIPMENT") && qCount.next();
    if (gotDbCount) {
        count = qCount.value(0).toInt();
    } else {
        count = visibleCount;
    }

    if (count == 0) {
        QMessageBox::information(this, trKey("Delete All"), trKey("There are no equipments to delete."));
        return;
    }

    const QString confirmMsg =
        QString("This will permanently delete ALL %1 equipment item(s).\n\nThis action cannot be undone!")
            .arg(count);

    if (QMessageBox::warning(this, trKey("Confirm Delete All"), confirmMsg,
                             QMessageBox::Yes | QMessageBox::Cancel,
                             QMessageBox::Cancel) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery qDel;
    if (!qDel.exec("DELETE FROM EQUIPMENT")) {
        QMessageBox::critical(this, trKey("Database Error"),
                              trKey("Failed to delete equipments:\n") + qDel.lastError().text());
        return;
    }

    QSqlDatabase::database().commit();

    int deleted = qDel.numRowsAffected();
    if (deleted < 0) deleted = count;

    logActivity(QString("Deleted ALL equipments (%1 records)").arg(deleted), "Equipment");
    QMessageBox::information(this, trKey("Deleted"),
                             QString("%1 equipment item(s) deleted successfully.").arg(deleted));

    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    setupEquipmentStats();
    if (m_nexusWidget) m_nexusWidget->initialize();
    if (m_costsWidget) m_costsWidget->initialize();
}

void MainWindow::onEquipmentShareToChat()
{
    if (ui_equipment->tabWidget->currentIndex() != 1) return; // Must be on View Tab

    // Find the currently selected row in table
    QModelIndexList selectedRows = ui_equipment->table_equipments->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "Select Equipment", "Please select an equipment line to share it.");
        return;
    }

    // Grab first selected row details (Assuming Model has headers: ID, Libelle, ...)
    int row = selectedRows.first().row();
    QString equipId = ui_equipment->table_equipments->model()->index(row, 0).data().toString();
    QString equipName = ui_equipment->table_equipments->model()->index(row, 1).data().toString();
    QString status = ui_equipment->table_equipments->model()->index(row, 4).data().toString();

    // Construct nice share card text
    QString shareCard = QString("\U0001f4e6 [Equipment Card]\nID: %1\nName: %2\nStatus: %3")
                            .arg(equipId, equipName, status);

    // Set text to chat input
    ui_equipment->le_chat_input->setText(shareCard);
    ui_equipment->le_chat_input->setFocus();

    // Switch to Chat Tab
    ui_equipment->tabWidget->setCurrentIndex(4);
}

// =============================================================================
// UNREAD MESSAGES SPLASH — after login
// =============================================================================
void MainWindow::showUnreadMessagesSplash()
{
    if (currentEmployeeId <= 0) return;
    
    // Read all messages from the JSON chat file
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray allMessages;
    
    if (file.open(QIODevice::ReadOnly)) {
        allMessages = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }
    
    if (allMessages.isEmpty()) return;
    
    // Count unread messages grouped by sender
    // "Unread" = messages sent TO the current user that are newer than the last message
    //            the current user sent in that conversation
    QMap<int, int> unreadCounts;          // sender_id -> count
    QMap<int, QString> senderNames;       // sender_id -> display name
    
    // Build employee name map
    QMap<int, QString> employeeNames;
    QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM EMPLOYEES");
    while (q.next()) {
        employeeNames[q.value(0).toInt()] = q.value(1).toString() + " " + q.value(2).toString();
    }
    
    // Find last sent timestamp per conversation partner
    QMap<int, QDateTime> lastSentTime; // partner_id -> last time we sent them a message
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();
        
        if (s_id == currentEmployeeId) {
            QDateTime dt = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
            if (!lastSentTime.contains(r_id) || dt > lastSentTime[r_id])
                lastSentTime[r_id] = dt;
        }
    }
    
    // Count messages received after our last sent message in each conversation
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();
        
        if (r_id == currentEmployeeId && s_id != currentEmployeeId) {
            QDateTime msgTime = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
            // If we never sent a message to this person, or this message is after our last reply
            if (!lastSentTime.contains(s_id) || msgTime > lastSentTime[s_id]) {
                unreadCounts[s_id]++;
                if (!senderNames.contains(s_id))
                    senderNames[s_id] = employeeNames.value(s_id, QString("Employee #%1").arg(s_id));
            }
        }
    }
    
    if (unreadCounts.isEmpty()) return;
    
    // Calculate total unread
    int totalUnread = 0;
    QStringList senderList;
    for (auto it = unreadCounts.begin(); it != unreadCounts.end(); ++it) {
        totalUnread += it.value();
        senderList << senderNames[it.key()];
    }
    
    // --- Create animated splash popup ---
    QDialog *splash = new QDialog(this);
    splash->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splash->setAttribute(Qt::WA_TranslucentBackground);
    splash->setFixedSize(400, 220);
    
    // Center on parent
    QPoint center = this->geometry().center();
    splash->move(center.x() - 200, center.y() - 110);
    
    QFrame *card = new QFrame(splash);
    card->setObjectName("splashCard");
    card->setGeometry(0, 0, 400, 220);
    card->setStyleSheet(
        "QFrame#splashCard {"
        " background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #2C2418,stop:0.5 #3A2D1A,stop:1 #1A140A);"
        " border: 2.5px solid #D4AF37;"
        " border-radius: 20px;"
        "}");
    
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(28, 24, 28, 20);
    lay->setSpacing(10);
    
    // Mail icon + title
    QLabel *iconLbl = new QLabel(QString::fromUtf8("\xF0\x9F\x93\xAC"), card);
    iconLbl->setStyleSheet("font-size: 36px; background: transparent; border: none;");
    iconLbl->setAlignment(Qt::AlignCenter);
    lay->addWidget(iconLbl);
    
    // Message text
    QString msgText;
    if (unreadCounts.size() == 1) {
        msgText = QString("You have %1 new message%2 from\n%3")
            .arg(totalUnread)
            .arg(totalUnread > 1 ? "s" : "")
            .arg(senderList.first());
    } else {
        msgText = QString("You have %1 new message%2 from\n%3")
            .arg(totalUnread)
            .arg(totalUnread > 1 ? "s" : "")
            .arg(senderList.join(", "));
    }
    
    QLabel *msgLbl = new QLabel(msgText, card);
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setWordWrap(true);
    msgLbl->setStyleSheet(
        "color: #F0E0C0; font-size: 15px; font-weight: 600;"
        " background: transparent; border: none; line-height: 1.4;");
    lay->addWidget(msgLbl);
    
    // Buttons row
    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->setSpacing(12);
    
    QPushButton *viewBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x92\xAC Open Chat"), card);
    viewBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setStyleSheet(
        "QPushButton { background: #D4AF37; color: #1A1208; border-radius: 14px;"
        " padding: 8px 20px; font-weight: bold; font-size: 13px; border: none; }"
        "QPushButton:hover { background: #E5C060; }");
    
    QPushButton *dismissBtn = new QPushButton("Dismiss", card);
    dismissBtn->setCursor(Qt::PointingHandCursor);
    dismissBtn->setStyleSheet(
        "QPushButton { background: rgba(139,111,71,0.2); color: #B8925A; border-radius: 14px;"
        " padding: 8px 20px; font-weight: bold; font-size: 13px; border: 1.5px solid #5A4A32; }"
        "QPushButton:hover { background: rgba(139,111,71,0.4); color: #D4AF37; border-color: #8B6F47; }");
    
    btnLay->addStretch();
    btnLay->addWidget(viewBtn);
    btnLay->addWidget(dismissBtn);
    btnLay->addStretch();
    lay->addLayout(btnLay);
    
    // Connect buttons
    connect(dismissBtn, &QPushButton::clicked, splash, &QDialog::accept);
    connect(viewBtn, &QPushButton::clicked, this, [this, splash, unreadCounts]() {
        splash->accept();
        // Navigate to Equipment -> Chat tab
        ui->stackedWidget->setCurrentIndex(5);
        ui_equipment->tabWidget->setCurrentIndex(4);
        onChatEmployeeListRefresh();
        
        // Auto-select the first sender with unread messages
        if (!unreadCounts.isEmpty()) {
            int firstSenderId = unreadCounts.begin().key();
            for (int i = 0; i < ui_equipment->list_employees->count(); ++i) {
                auto *item = ui_equipment->list_employees->item(i);
                if (item->data(Qt::UserRole).toInt() == firstSenderId) {
                    ui_equipment->list_employees->setCurrentItem(item);
                    onChatEmployeeSelected(item);
                    break;
                }
            }
        }
    });
    
    // --- Entrance animation ---
    splash->setWindowOpacity(0.0);
    splash->show();
    
    QPropertyAnimation *fadeIn = new QPropertyAnimation(splash, "windowOpacity");
    fadeIn->setDuration(500);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Subtle bounce animation on the card
    QPropertyAnimation *bounceAnim = new QPropertyAnimation(card, "geometry");
    bounceAnim->setDuration(600);
    bounceAnim->setStartValue(QRect(0, 30, 400, 220));
    bounceAnim->setEndValue(QRect(0, 0, 400, 220));
    bounceAnim->setEasingCurve(QEasingCurve::OutBack);
    bounceAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Auto-dismiss after 8 seconds
    QTimer::singleShot(8000, splash, [splash]() {
        if (splash->isVisible()) {
            QPropertyAnimation *fadeOut = new QPropertyAnimation(splash, "windowOpacity");
            fadeOut->setDuration(400);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            fadeOut->setEasingCurve(QEasingCurve::InCubic);
            connect(fadeOut, &QPropertyAnimation::finished, splash, &QDialog::accept);
            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });
    
    splash->exec();
    delete splash;
}

// =============================================================================
// CHAT SEARCH (Ctrl+F)
// =============================================================================
void MainWindow::onChatSearchToggle()
{
    if (!ui_equipment || ui_equipment->tabWidget->currentIndex() != 4) return;
    
    // Check if search bar already exists, toggle visibility
    QWidget *existingBar = equipmentPage->findChild<QWidget*>("chatSearchBar");
    if (existingBar) {
        bool isVisible = existingBar->isVisible();
        existingBar->setVisible(!isVisible);
        if (!isVisible) {
            // Focus the search input
            QLineEdit *searchInput = existingBar->findChild<QLineEdit*>("chatSearchInput");
            if (searchInput) searchInput->setFocus();
        }
        return;
    }
    
    // Create search bar above the chat scroll area
    QWidget *searchBar = new QWidget(equipmentPage);
    searchBar->setObjectName("chatSearchBar");
    searchBar->setFixedHeight(50);
    searchBar->setStyleSheet(
        "QWidget#chatSearchBar {"
        " background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3820);"
        " border-bottom: 2px solid #8B6F47;"
        "}");
    
    QHBoxLayout *barLay = new QHBoxLayout(searchBar);
    barLay->setContentsMargins(12, 6, 12, 6);
    barLay->setSpacing(8);
    
    QLabel *searchIcon = new QLabel(QString::fromUtf8("\xF0\x9F\x94\x8D"), searchBar);
    searchIcon->setStyleSheet("font-size: 18px; background: transparent; border: none;");
    barLay->addWidget(searchIcon);
    
    QLineEdit *searchInput = new QLineEdit(searchBar);
    searchInput->setObjectName("chatSearchInput");
    searchInput->setPlaceholderText("Search messages... (Ctrl+F)");
    searchInput->setStyleSheet(
        "QLineEdit { background: rgba(0,0,0,0.3); color: #F0E0C0; border: 1.5px solid #5A4A32;"
        " border-radius: 14px; padding: 6px 14px; font-size: 13px; }"
        "QLineEdit:focus { border-color: #D4AF37; }");
    barLay->addWidget(searchInput, 1);
    
    // Result count label
    QLabel *resultLbl = new QLabel("", searchBar);
    resultLbl->setObjectName("chatSearchResultLbl");
    resultLbl->setStyleSheet("color: #B8925A; font-size: 11px; background: transparent; border: none; min-width: 80px;");
    barLay->addWidget(resultLbl);
    
    // Navigate buttons
    QPushButton *prevBtn = new QPushButton(QString::fromUtf8("\xE2\x96\xB2"), searchBar);
    prevBtn->setFixedSize(30, 30);
    prevBtn->setCursor(Qt::PointingHandCursor);
    prevBtn->setStyleSheet(
        "QPushButton { background: rgba(139,111,71,0.2); color: #D4AF37; border: 1px solid #5A4A32;"
        " border-radius: 15px; font-size: 12px; }"
        "QPushButton:hover { background: #8B6F47; color: white; }");
    barLay->addWidget(prevBtn);
    
    QPushButton *nextBtn = new QPushButton(QString::fromUtf8("\xE2\x96\xBC"), searchBar);
    nextBtn->setFixedSize(30, 30);
    nextBtn->setCursor(Qt::PointingHandCursor);
    nextBtn->setStyleSheet(
        "QPushButton { background: rgba(139,111,71,0.2); color: #D4AF37; border: 1px solid #5A4A32;"
        " border-radius: 15px; font-size: 12px; }"
        "QPushButton:hover { background: #8B6F47; color: white; }");
    barLay->addWidget(nextBtn);
    
    QPushButton *closeSearchBtn = new QPushButton(QString::fromUtf8("\xE2\x9C\x95"), searchBar);
    closeSearchBtn->setFixedSize(30, 30);
    closeSearchBtn->setCursor(Qt::PointingHandCursor);
    closeSearchBtn->setStyleSheet(
        "QPushButton { background: rgba(200,50,50,0.15); color: #FF7070; border: 1px solid rgba(200,50,50,0.3);"
        " border-radius: 15px; font-size: 12px; }"
        "QPushButton:hover { background: #CC3333; color: white; }");
    barLay->addWidget(closeSearchBtn);
    
    // Insert the search bar into the chat panel layout, after the header
    QVBoxLayout *chatPanelLayout = qobject_cast<QVBoxLayout*>(ui_equipment->frame_chat_panel->layout());
    if (chatPanelLayout) {
        chatPanelLayout->insertWidget(1, searchBar); // After header (index 0)
    } else {
        // Fallback: just parent it
        searchBar->setParent(ui_equipment->frame_chat_panel);
        searchBar->show();
    }
    
    searchInput->setFocus();
    
    // Shared state for navigation
    auto *matchIndices = new QList<int>();
    auto *currentMatchIdx = new int(-1);
    
    // Search logic
    auto doSearch = [this, searchInput, resultLbl, matchIndices, currentMatchIdx]() {
        QString query = searchInput->text().trimmed();
        matchIndices->clear();
        *currentMatchIdx = -1;
        
        // First, clear any existing highlights
        QLayout *chatLayout = ui_equipment->verticalLayout_chat_contents;
        for (int i = 0; i < chatLayout->count(); ++i) {
            QWidget *w = chatLayout->itemAt(i)->widget();
            if (!w) continue;
            // Reset opacity/highlight
            w->setGraphicsEffect(nullptr);
            // Find all QLabels with "messageText" or word-wrap child labels
            QList<QLabel*> labels = w->findChildren<QLabel*>();
            for (QLabel *lbl : labels) {
                if (lbl->wordWrap()) {
                    // Remove HTML highlighting - restore plain text
                    QString text = lbl->text();
                    text.replace(QRegularExpression("<span style=[^>]*>"), "");
                    text.replace("</span>", "");
                    lbl->setText(text);
                }
            }
        }
        
        if (query.isEmpty()) {
            resultLbl->setText("");
            return;
        }
        
        // Search through all visible message widgets
        for (int i = 0; i < chatLayout->count(); ++i) {
            QWidget *w = chatLayout->itemAt(i)->widget();
            if (!w) continue;
            
            QList<QLabel*> labels = w->findChildren<QLabel*>();
            bool found = false;
            for (QLabel *lbl : labels) {
                if (lbl->wordWrap() && lbl->text().contains(query, Qt::CaseInsensitive)) {
                    found = true;
                    // Highlight matched text
                    QString text = lbl->text();
                    int idx = text.indexOf(query, 0, Qt::CaseInsensitive);
                    while (idx != -1) {
                        QString matched = text.mid(idx, query.length());
                        text.replace(idx, query.length(),
                            QString("<span style='background-color: #D4AF37; color: #1A1208; padding: 1px 3px; border-radius: 3px;'>%1</span>").arg(matched));
                        idx = text.indexOf(query, idx + 100, Qt::CaseInsensitive); // skip past the HTML we just inserted
                    }
                    lbl->setTextFormat(Qt::RichText);
                    lbl->setText(text);
                }
            }
            if (found) {
                matchIndices->append(i);
            }
        }
        
        if (matchIndices->isEmpty()) {
            resultLbl->setText("No results");
        } else {
            *currentMatchIdx = 0;
            resultLbl->setText(QString("1 of %1").arg(matchIndices->size()));
            
            // Scroll to first match
            QWidget *firstMatch = chatLayout->itemAt(matchIndices->first())->widget();
            if (firstMatch) {
                ui_equipment->scrollArea_chat->ensureWidgetVisible(firstMatch, 50, 50);
            }
        }
    };
    
    connect(searchInput, &QLineEdit::textChanged, doSearch);
    connect(searchInput, &QLineEdit::returnPressed, [nextBtn]() { nextBtn->click(); });
    
    // Navigate to next match
    connect(nextBtn, &QPushButton::clicked, [this, matchIndices, currentMatchIdx, resultLbl]() {
        if (matchIndices->isEmpty()) return;
        *currentMatchIdx = (*currentMatchIdx + 1) % matchIndices->size();
        resultLbl->setText(QString("%1 of %2").arg(*currentMatchIdx + 1).arg(matchIndices->size()));
        
        QWidget *match = ui_equipment->verticalLayout_chat_contents->itemAt(matchIndices->at(*currentMatchIdx))->widget();
        if (match) {
            ui_equipment->scrollArea_chat->ensureWidgetVisible(match, 50, 50);
        }
    });
    
    // Navigate to previous match
    connect(prevBtn, &QPushButton::clicked, [this, matchIndices, currentMatchIdx, resultLbl]() {
        if (matchIndices->isEmpty()) return;
        *currentMatchIdx = (*currentMatchIdx - 1 + matchIndices->size()) % matchIndices->size();
        resultLbl->setText(QString("%1 of %2").arg(*currentMatchIdx + 1).arg(matchIndices->size()));
        
        QWidget *match = ui_equipment->verticalLayout_chat_contents->itemAt(matchIndices->at(*currentMatchIdx))->widget();
        if (match) {
            ui_equipment->scrollArea_chat->ensureWidgetVisible(match, 50, 50);
        }
    });
    
    // Close search bar
    connect(closeSearchBtn, &QPushButton::clicked, [searchBar, this]() {
        // Clear highlights before closing
        QLayout *chatLayout = ui_equipment->verticalLayout_chat_contents;
        for (int i = 0; i < chatLayout->count(); ++i) {
            QWidget *w = chatLayout->itemAt(i)->widget();
            if (!w) continue;
            QList<QLabel*> labels = w->findChildren<QLabel*>();
            for (QLabel *lbl : labels) {
                if (lbl->wordWrap()) {
                    QString text = lbl->text();
                    text.replace(QRegularExpression("<span style=[^>]*>"), "");
                    text.replace("</span>", "");
                    lbl->setTextFormat(Qt::PlainText);
                    lbl->setText(text);
                }
            }
        }
        searchBar->hide();
    });
    
    // Escape key also closes
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), searchBar);
    connect(escShortcut, &QShortcut::activated, closeSearchBtn, &QPushButton::click);
}

// =============================================================================
// VOICE RECORDING & FEEDBACK
// =============================================================================

void MainWindow::shakeWidget(QWidget *w) {
    if (!w) return;
    QPropertyAnimation *anim = new QPropertyAnimation(w, "pos");
    anim->setDuration(100);
    anim->setLoopCount(3);
    QPoint op = w->pos();
    anim->setStartValue(op);
    anim->setKeyValueAt(0.25, op + QPoint(6, 0));
    anim->setKeyValueAt(0.75, op - QPoint(6, 0));
    anim->setEndValue(op);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onChatVoiceToggled() {
    auto *btn = equipmentPage->findChild<QPushButton*>("btn_chat_voice");
    if (!btn) return;
    if (btn->isChecked()) {
        onChatStartRecord();
    } else {
        onChatStopRecord();
    }
}

void MainWindow::onChatStartRecord() {
    QString voiceDir = QDir::currentPath() + "/voice_notes";
    QDir().mkpath(voiceDir);
    QString fileName = QString("voice_%1.wav").arg(QDateTime::currentMSecsSinceEpoch());
    m_recorder->setOutputLocation(QUrl::fromLocalFile(voiceDir + "/" + fileName));
    m_recorder->record();
    m_isRecording = true;
    ui_equipment->le_chat_input->setPlaceholderText("Recording voice note...");
}

void MainWindow::onChatStopRecord() {
    m_recorder->stop();
    m_isRecording = false;
    ui_equipment->le_chat_input->setPlaceholderText("Type a message...");
    
    // Auto-send the voice note
    QTimer::singleShot(200, this, [this](){
        ui_equipment->le_chat_input->setText(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Note"));
        onChatSendMessage();
    });
}

void MainWindow::onUploadAvatar() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty()) return;

    QString employeeId = ui_employee->le_id->text();
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, tr("Avatar"), tr("Please select an employee or enter an ID first."));
        return;
    }

    QDir().mkpath("assets/av");
    QString destPath = QString("assets/av/employee_%1.png").arg(employeeId);
    if (QFile::exists(destPath)) QFile::remove(destPath);
    if (QFile::copy(fileName, destPath)) {
        QPixmap pix(destPath);
        ui_employee->lbl_avatar->setPixmap(getCircularPixmap(pix).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QMessageBox::information(this, tr("Avatar"), tr("Avatar uploaded successfully."));
        if (employeeId.toInt() == currentEmployeeId) updateUserProfileDisplay();
    } else {
        QMessageBox::critical(this, tr("Avatar"), tr("Failed to save avatar."));
    }
}

void MainWindow::onScanFace() {
    QString employeeId = ui_employee->le_id->text().trimmed();
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, tr("Face Scan"), tr("Please select an employee or enter an ID first."));
        return;
    }

    if (m_isEmpFaceScanActive) {
        // Capture ONE high-quality frame
        QVideoFrame frame = m_empVideoSink->videoFrame();
        if (frame.isValid() && frame.map(QVideoFrame::ReadOnly)) {
            QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
            frame.unmap();
            
            // Flip to ensure it is NOT mirrored (standard view)
            image = image.mirrored(true, false);
            
            QDir().mkpath("assets/av");
            // face_ path is STRICTLY for biometric login matching only
            QString facePath = QString("assets/av/face_%1.png").arg(employeeId);
            if (QFile::exists(facePath)) QFile::remove(facePath);
            image.save(facePath);

            // Done - stop camera and restore employee avatar display (NOT the face scan image)
            m_empCamera->stop();
            m_isEmpFaceScanActive = false;
            m_faceScanStage = 0;
            ui_employee->btn_scan_face->setText(tr("Scan Face ID"));

            // Restore the correct avatar (employee_) or placeholder after scan
            QString avPath = QString("assets/av/employee_%1.png").arg(employeeId);
            if (QFile::exists(avPath)) {
                ui_employee->lbl_avatar->setPixmap(getCircularPixmap(QPixmap(avPath)).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                ui_employee->lbl_avatar->setPixmap(QPixmap());
                ui_employee->lbl_avatar->setText("No Avatar");
            }

            QMessageBox::information(this, tr("Face ID"), tr("Biometric profile registered successfully.\nThis image is used for login only and is separate from the display avatar."));
        }
    } else {
        // Start Scan
        if (!m_empCamera) {
             m_empCamera = new QCamera(QMediaDevices::defaultVideoInput(), this);
             m_empCaptureSession = new QMediaCaptureSession(this);
             m_empVideoSink = new QVideoSink(this);
             m_empCaptureSession->setCamera(m_empCamera);
             m_empCaptureSession->setVideoSink(m_empVideoSink);
             connect(m_empVideoSink, &QVideoSink::videoFrameChanged, this, &MainWindow::processEmpCameraFrame);
        }
        
        m_faceScanStage = 0;
        m_empCamera->start();
        m_isEmpFaceScanActive = true;
        ui_employee->btn_scan_face->setText(tr("SAVE CAPTURE"));
        QMessageBox::information(this, tr("Face Scan"), tr("Scanning started. Please look straight at the camera and click 'SAVE CAPTURE' (Image will be non-mirrored)."));
    }
}

void MainWindow::processEmpCameraFrame() {
    if (!m_isEmpFaceScanActive) return;
    
    QVideoFrame frame = m_empVideoSink->videoFrame();
    if (!frame.isValid() || !frame.map(QVideoFrame::ReadOnly)) return;
    
    QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
    frame.unmap();

    // Disable Mirroring for Real-View Capture experience
    image = image.mirrored(true, false);
    
    // Show live camera preview in lbl_avatar ONLY during active face scan
    // This is a TEMPORARY preview - the saved face_ image is never shown as an avatar
    ui_employee->lbl_avatar->setText("");
    ui_employee->lbl_avatar->setPixmap(getCircularPixmap(QPixmap::fromImage(image)).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::updateUserProfileDisplay() {
    if (currentEmployeeId <= 0) return;

    QSqlQuery query;
    query.prepare("SELECT FIRST_NAME, LAST_NAME, JOB_TITLE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    query.bindValue(":id", currentEmployeeId);
    if (query.exec() && query.next()) {
        QString firstName = query.value(0).toString();
        QString jobTitle = query.value(2).toString();
        
        if (homeWindow) {
           QLabel* nameLabel = homeWindow->findChild<QLabel*>("lbl_user_name");
           QLabel* roleLabel = homeWindow->findChild<QLabel*>("lbl_user_role");
           QLabel* avatarLabel = homeWindow->findChild<QLabel*>("lbl_user_avatar");
           
           if (nameLabel) nameLabel->setText(firstName);
           if (roleLabel) roleLabel->setText(jobTitle);
           
           QString avatarPath = QString("assets/av/face_%1.png").arg(currentEmployeeId);
           if (!QFile::exists(avatarPath)) avatarPath = QString("assets/av/employee_%1.png").arg(currentEmployeeId);

           if (avatarLabel) {
               if (QFile::exists(avatarPath)) {
                   QPixmap pix(avatarPath);
                   avatarLabel->setPixmap(getCircularPixmap(pix).scaled(avatarLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                   avatarLabel->setText("");
               } else {
                   avatarLabel->setPixmap(QPixmap());
                   avatarLabel->setText("No Pic");
               }
           }
        }
    }
}

QPixmap MainWindow::getCircularPixmap(const QPixmap &src) {
    if (src.isNull()) return src;
    
    int size = qMin(src.width(), src.height());
    QPixmap out(size, size);
    out.fill(Qt::transparent);
    
    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    
    // Center the image
    int x = (size - src.width()) / 2;
    int y = (size - src.height()) / 2;
    painter.drawPixmap(x, y, src);
    
    return out;
}

void MainWindow::onEmployeeEnsureHistoryTable() {
    // Use local JSON storage (no DB table creation allowed by user request).
    const QString filePath = "hammerdown_audit_log.json";
    QFile file(filePath);
    if (file.exists()) return;

    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(QJsonArray()).toJson(QJsonDocument::Compact));
        file.close();
    }
}

void MainWindow::logActivity(const QString &action, const QString &module) {
    // Get full name of current employee (DB read is OK; no DB table creation).
    QString empName = "System";
    if (currentEmployeeId > 0) {
        QSqlQuery nq;
        nq.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        nq.bindValue(":id", currentEmployeeId);
        if (nq.exec() && nq.next()) empName = nq.value(0).toString();
    }

    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray auditArray;

    // Load existing log
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray raw = file.readAll();
            file.close();

            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            if (doc.isArray()) auditArray = doc.array();
        }
    }

    const QDateTime now = QDateTime::currentDateTime();
    QJsonObject obj;
    obj["log_id"] = auditArray.size() + 1;
    obj["timestamp_iso"] = now.toString(Qt::ISODate);
    obj["timestamp_ms"] = static_cast<qint64>(now.toMSecsSinceEpoch());
    obj["employee_name"] = empName;
    obj["action_details"] = action;
    obj["module_name"] = module;

    auditArray.append(obj);

    // Save back
    QFile out(filePath);
    if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        out.write(QJsonDocument(auditArray).toJson(QJsonDocument::Compact));
        out.close();
    }
}

void MainWindow::setupOrderMapTab()
{
    if (!ui_order || !ui_order->tabWidget) return;

    QWidget *mapTab = new QWidget(ui_order->tabWidget);
    mapTab->setObjectName("tab_map");

    QVBoxLayout *root = new QVBoxLayout(mapTab);
    root->setContentsMargins(18, 60, 18, 16);
    root->setSpacing(10);

    QHBoxLayout *controls = new QHBoxLayout();
    QLabel *clientIdLabel = new QLabel("Clients:", mapTab);
    clientIdLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    m_mapRefreshBtn = new QPushButton("Load Map", mapTab);
    m_mapRefreshBtn->setCursor(Qt::PointingHandCursor);
    m_mapRefreshBtn->setStyleSheet(
        "QPushButton{background:#8B6F47;color:white;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#a3845a;}"
        "QPushButton:pressed{background:#6b5535;}");

    m_mapZoomInBtn = new QPushButton("Zoom +", mapTab);
    m_mapZoomInBtn->setCursor(Qt::PointingHandCursor);
    m_mapZoomInBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}"
        "QPushButton:pressed{background:#3d2e18;}");

    m_mapZoomOutBtn = new QPushButton("Zoom -", mapTab);
    m_mapZoomOutBtn->setCursor(Qt::PointingHandCursor);
    m_mapZoomOutBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}"
        "QPushButton:pressed{background:#3d2e18;}");

    m_mapFullscreenBtn = new QPushButton("Full Screen", mapTab);
    m_mapFullscreenBtn->setCursor(Qt::PointingHandCursor);
    m_mapFullscreenBtn->setStyleSheet(
        "QPushButton{background:#3d2e18;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#5c4a2a;}"
        "QPushButton:pressed{background:#2a1e10;}");



    controls->addWidget(clientIdLabel);
    controls->addStretch();
    controls->addWidget(m_mapRefreshBtn);
    controls->addWidget(m_mapZoomInBtn);
    controls->addWidget(m_mapZoomOutBtn);
    controls->addWidget(m_mapFullscreenBtn);

    m_mapAddressLabel = new QLabel("Address: --", mapTab);
    m_mapAddressLabel->setWordWrap(true);
    m_mapAddressLabel->setStyleSheet("color: #d4a96a; font-size: 12px;");

    m_mapStatusLabel = new QLabel("Select a client or enter Buyer ID, then click Load Map.", mapTab);
    m_mapStatusLabel->setWordWrap(true);
    m_mapStatusLabel->setStyleSheet("color: #8B6F47; font-size: 12px; font-style: italic;");

    m_mapClientTable = new QTableWidget(mapTab);
    m_mapClientTable->setColumnCount(3);
    m_mapClientTable->setHorizontalHeaderLabels({"ID", "Name", "Address"});
    m_mapClientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_mapClientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mapClientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_mapClientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_mapClientTable->verticalHeader()->setVisible(false);
    m_mapClientTable->setMinimumHeight(220);
    m_mapClientTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mapClientTable->setStyleSheet(
        "QTableWidget { background: rgba(26, 18, 8, 0.85); color: #e8dcc8; border: 2px solid #8B6F47; border-radius: 10px; }"
        "QHeaderView::section { background: #2a1e10; color: #d4a96a; border: none; padding: 4px; }"
        "QTableWidget::item:selected { background: #8B6F47; color: white; }");

    m_mapImageLabel = new QLabel(mapTab);
    m_mapImageLabel->setMinimumSize(640, 360);
    m_mapImageLabel->setAlignment(Qt::AlignCenter);
    m_mapImageLabel->setStyleSheet("background: #1a1208; border: 2px solid #8B6F47; border-radius: 10px; color: #8B6F47;");
    m_mapImageLabel->setText("Map preview will appear here.");
    m_mapImageLabel->installEventFilter(this);


    root->addLayout(controls);
    root->addWidget(m_mapClientTable);
    root->addWidget(m_mapAddressLabel);
    root->addWidget(m_mapStatusLabel);
    root->addWidget(m_mapImageLabel, 1);

    ui_order->tabWidget->addTab(mapTab, "Map");

    connect(m_mapRefreshBtn, &QPushButton::clicked, this, &MainWindow::requestMapForBuyerId);
    connect(m_mapClientTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::requestMapForBuyerId);
    connect(m_mapZoomInBtn, &QPushButton::clicked, this, [this]() {
        m_mapZoom = qMin(18, m_mapZoom + 1);
        m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
            ? m_mapFullscreenLabel->size()
            : m_mapImageLabel->size();
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);
    });
    connect(m_mapZoomOutBtn, &QPushButton::clicked, this, [this]() {
        m_mapZoom = qMax(3, m_mapZoom - 1);
        m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
            ? m_mapFullscreenLabel->size()
            : m_mapImageLabel->size();
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);
    });
    connect(m_mapFullscreenBtn, &QPushButton::clicked, this, [this]() {
        if (!m_mapFullscreenDialog) {
            m_mapFullscreenDialog = new QDialog(this);
            m_mapFullscreenDialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            m_mapFullscreenDialog->setModal(true);
            m_mapFullscreenDialog->setAttribute(Qt::WA_TranslucentBackground);
            m_mapFullscreenDialog->setStyleSheet("QDialog { background: rgba(0,0,0,90); }");

            QVBoxLayout *layout = new QVBoxLayout(m_mapFullscreenDialog);
            layout->setContentsMargins(24, 24, 24, 24);

            QWidget *card = new QWidget(m_mapFullscreenDialog);
            card->setStyleSheet("QWidget { background: rgba(15, 10, 6, 220); border: 2px solid #8B6F47; border-radius: 14px; }");
            QVBoxLayout *cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(14, 14, 14, 14);

            m_mapFullscreenLabel = new QLabel(card);
            m_mapFullscreenLabel->setAlignment(Qt::AlignCenter);
            m_mapFullscreenLabel->setMinimumSize(1000, 620);
            m_mapFullscreenLabel->setStyleSheet("color: #d4a96a; font-size: 14px; border: 1px solid #8B6F47; border-radius: 10px; background: #1a1208;");
            m_mapFullscreenLabel->installEventFilter(this);
            cardLayout->addWidget(m_mapFullscreenLabel);

            QLabel *hint = new QLabel("Drag to move • Mouse wheel to zoom • Double-click or Esc to close", card);
            hint->setAlignment(Qt::AlignCenter);
            hint->setStyleSheet("color:#d4a96a; font-size:12px; border:none; background:transparent;");
            cardLayout->addWidget(hint);

            layout->addStretch();
            layout->addWidget(card, 0, Qt::AlignCenter);
            layout->addStretch();

            QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), m_mapFullscreenDialog);
            connect(esc, &QShortcut::activated, m_mapFullscreenDialog, &QDialog::close);

            connect(m_mapFullscreenDialog, &QDialog::finished, this, [this]() {
                if (ui && ui->stackedWidget) {
                    ui->stackedWidget->setGraphicsEffect(nullptr);
                }
                m_mapBlurEffect = nullptr;
                m_mapDragging = false;
            });
        }

        if (ui && ui->stackedWidget && !m_mapBlurEffect) {
            m_mapBlurEffect = new QGraphicsBlurEffect(this);
            m_mapBlurEffect->setBlurRadius(10.0);
            ui->stackedWidget->setGraphicsEffect(m_mapBlurEffect);
        }

        if (m_mapHasPixmap) {
            m_mapImageSize = m_mapFullscreenLabel->size();
            m_mapFullscreenLabel->setPixmap(m_mapCurrentPixmap.scaled(
                m_mapFullscreenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_mapFullscreenLabel->setText("Map is not loaded yet.");
        }

        m_mapFullscreenDialog->setGeometry(this->window()->geometry());
        m_mapFullscreenDialog->show();
        m_mapFullscreenDialog->raise();
    });

    populateMapClients();
}

void MainWindow::populateMapClients()
{
    if (!m_mapClientTable) return;

    m_mapClientTable->setRowCount(0);
    QSqlQuery q("SELECT CLIENT_ID, FIRST_NAME, LAST_NAME, ADDRESS FROM CLIENTS ORDER BY CLIENT_ID");
    int row = 0;
    while (q.next()) {
        m_mapClientTable->insertRow(row);
        m_mapClientTable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        m_mapClientTable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString() + " " + q.value(2).toString()));
        m_mapClientTable->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        row++;
    }

    if (row == 0) {
        m_mapStatusLabel->setText("No clients found.");
    }
}

void MainWindow::requestMapForBuyerId()
{
    if (!ui_order || !ui_order->le_buyer) return;

    QString address;

    if (m_mapClientTable && m_mapClientTable->currentRow() >= 0) {
        int row = m_mapClientTable->currentRow();
        QTableWidgetItem *addrItem = m_mapClientTable->item(row, 2);
        if (addrItem) {
            address = addrItem->text().trimmed();
        }
    }

    if (address.isEmpty()) {
        QString buyerText = ui_order->le_buyer->text().trimmed();
        if (buyerText.isEmpty()) {
            m_mapStatusLabel->setText("Please select a client or enter a Buyer ID.");
            return;
        }

        bool ok = false;
        int clientId = buyerText.toInt(&ok);
        if (!ok || clientId <= 0) {
            m_mapStatusLabel->setText("Buyer ID must be a valid number.");
            return;
        }

        QSqlQuery q;
        q.prepare("SELECT ADDRESS FROM CLIENTS WHERE CLIENT_ID = :id");
        q.bindValue(":id", clientId);
        if (!q.exec() || !q.next()) {
            m_mapStatusLabel->setText("No client found for that Buyer ID.");
            m_mapAddressLabel->setText("Address: --");
            return;
        }

        address = q.value(0).toString().trimmed();
        if (address.isEmpty()) {
            m_mapStatusLabel->setText("Client has no address on file.");
            m_mapAddressLabel->setText("Address: --");
            return;
        }
    }

    m_mapAddressLabel->setText("Address: " + address);
    m_mapStatusLabel->setText("Geocoding address...");
    m_mapHasClientPin = false;
    m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
        ? m_mapFullscreenLabel->size()
        : m_mapImageLabel->size();

    QString geocodeQuery = address;
    if (!geocodeQuery.contains("tunisia", Qt::CaseInsensitive) &&
        !geocodeQuery.contains("tunisie", Qt::CaseInsensitive) &&
        !geocodeQuery.contains(QString::fromUtf8("\xD8\xAA\xD9\x88\xD9\x86\xD8\xB3"), Qt::CaseInsensitive)) {
        geocodeQuery += ", Tunisia";
    }

    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery query;
    query.addQueryItem("q", geocodeQuery);
    query.addQueryItem("format", "json");
    query.addQueryItem("limit", "1");
    query.addQueryItem("accept-language", "en");
    query.addQueryItem("countrycodes", "tn");
    query.addQueryItem("bounded", "1");
    query.addQueryItem("viewbox", "7.5,37.6,11.6,30.2");
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
    req.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = m_mapNet->get(req);
    reply->setProperty("mapType", "geocode");
    reply->setProperty("address", address);
    reply->setProperty("query", geocodeQuery);
    reply->setProperty("geocodeStage", "tn");
}

void MainWindow::onMapNetworkFinished(QNetworkReply *reply)
{
    if (!reply) return;
    const QString type = reply->property("mapType").toString();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (type == "geocode") {
        const QString stage = reply->property("geocodeStage").toString();
        const QString address = reply->property("address").toString();
        const QString queryText = reply->property("query").toString();
        const QByteArray data = reply->readAll();

        auto issueGlobalFallback = [this, address, queryText]() {
            if (!m_mapNet) return;
            QUrl url("https://nominatim.openstreetmap.org/search");
            QUrlQuery q;
            q.addQueryItem("q", queryText.isEmpty() ? address : queryText);
            q.addQueryItem("format", "json");
            q.addQueryItem("limit", "1");
            q.addQueryItem("accept-language", "en");
            url.setQuery(q);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            req.setRawHeader("Accept", "application/json");
            QNetworkReply *fallback = m_mapNet->get(req);
            fallback->setProperty("mapType", "geocode");
            fallback->setProperty("address", address);
            fallback->setProperty("query", queryText);
            fallback->setProperty("geocodeStage", "global");
            m_mapStatusLabel->setText("Geocoding retry (global)...");
        };

        auto issuePhotonFallback = [this, address, queryText]() {
            if (!m_mapNet) return;
            QUrl url("https://photon.komoot.io/api");
            QUrlQuery q;
            q.addQueryItem("q", queryText.isEmpty() ? address : queryText);
            q.addQueryItem("limit", "1");
            url.setQuery(q);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            req.setRawHeader("Accept", "application/json");
            QNetworkReply *fallback = m_mapNet->get(req);
            fallback->setProperty("mapType", "geocode");
            fallback->setProperty("address", address);
            fallback->setProperty("query", queryText);
            fallback->setProperty("geocodeStage", "photon");
            m_mapStatusLabel->setText("Geocoding retry (fallback provider)...");
        };

        bool hasCoords = false;
        double latVal = 0.0;
        double lonVal = 0.0;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (stage == "photon") {
                const QJsonObject root = doc.object();
                const QJsonArray features = root.value("features").toArray();
                if (!features.isEmpty()) {
                    const QJsonObject feature = features.first().toObject();
                    const QJsonArray coords = feature.value("geometry").toObject().value("coordinates").toArray();
                    if (coords.size() >= 2) {
                        lonVal = coords.at(0).toDouble();
                        latVal = coords.at(1).toDouble();
                        hasCoords = true;
                    }
                }
            } else {
                const QJsonArray arr = doc.array();
                if (!arr.isEmpty()) {
                    const QJsonObject obj = arr.first().toObject();
                    bool okLat = false;
                    bool okLon = false;
                    latVal = obj.value("lat").toString().toDouble(&okLat);
                    lonVal = obj.value("lon").toString().toDouble(&okLon);
                    hasCoords = okLat && okLon;
                }
            }
        }

        if (!hasCoords) {
            if (stage == "tn") {
                issueGlobalFallback();
                reply->deleteLater();
                return;
            }
            if (stage == "global") {
                issuePhotonFallback();
                reply->deleteLater();
                return;
            }

            if (reply->error() != QNetworkReply::NoError) {
                m_mapStatusLabel->setText(QString("Geocoding failed (%1)").arg(reply->errorString()));
            } else if (httpStatus >= 400) {
                m_mapStatusLabel->setText(QString("Geocoding failed (HTTP %1)").arg(httpStatus));
            } else {
                m_mapStatusLabel->setText("Address not found on map.");
            }
            reply->deleteLater();
            return;
        }

        m_mapCenterLat = latVal;
        m_mapCenterLon = lonVal;
        m_mapClientPinLat = m_mapCenterLat;
        m_mapClientPinLon = m_mapCenterLon;
        m_mapHasClientPin = true;
        m_mapStatusLabel->setText("Loading map tiles...");
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        m_mapStatusLabel->setText("Network error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    if (type == "tile") {
        const QString tileKey = reply->property("tileKey").toString();
        const bool trackedTile = m_mapPendingTiles.contains(tileKey);

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imgData = reply->readAll();
            QPixmap pix;
            if (pix.loadFromData(imgData)) {
                m_mapTileCache.insert(tileKey, pix);
            } else {
                if (trackedTile) m_mapTileErrors++;
            }
        } else {
            if (trackedTile) m_mapTileErrors++;
        }

        if (trackedTile) {
            m_mapPendingTiles.remove(tileKey);
            m_mapLoadedTiles++;
        }

        if (trackedTile) {
            renderOrderMap();
            if (m_mapPendingTiles.isEmpty()) {
                if (m_mapTileErrors > 0) {
                    m_mapStatusLabel->setText("Map loaded with missing tiles.");
                } else {
                    m_mapStatusLabel->setText("Map loaded successfully.");
                }
            } else {
                m_mapStatusLabel->setText(QString("Loading map tiles... %1/%2")
                                          .arg(m_mapLoadedTiles)
                                          .arg(m_mapExpectedTiles));
            }
        }

        reply->deleteLater();
        return;
    }

    reply->deleteLater();
}

void MainWindow::requestMapTiles(double lat, double lon)
{
    if (!m_mapNet) return;

    // Cancel in-flight tile requests from previous map views to free bandwidth.
    const auto activeReplies = m_mapNet->findChildren<QNetworkReply*>();
    for (QNetworkReply *active : activeReplies) {
        if (!active) continue;
        if (active->property("mapType").toString() != "tile") continue;
        if (active->isRunning()) active->abort();
    }

    const int tileSize = 256;
    const int zoom = m_mapZoom;
    const int n = 1 << zoom;

    double latRad = qDegreesToRadians(lat);
    double xtile = (lon + 180.0) / 360.0 * n;
    double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;

    double worldX = xtile * tileSize;
    double worldY = ytile * tileSize;

    if (m_mapImageSize.width() < 64 || m_mapImageSize.height() < 64) {
        m_mapImageSize = QSize(640, 360);
    }
    m_mapTopLeftX = worldX - (m_mapImageSize.width() / 2.0);
    m_mapTopLeftY = worldY - (m_mapImageSize.height() / 2.0);

    m_mapTileX0 = static_cast<int>(floor(m_mapTopLeftX / tileSize));
    m_mapTileY0 = static_cast<int>(floor(m_mapTopLeftY / tileSize));
    m_mapTileX1 = static_cast<int>(floor((m_mapTopLeftX + m_mapImageSize.width() - 1) / tileSize));
    m_mapTileY1 = static_cast<int>(floor((m_mapTopLeftY + m_mapImageSize.height() - 1) / tileSize));

    m_mapPendingTiles.clear();
    m_mapTileErrors = 0;
    m_mapExpectedTiles = 0;
    m_mapLoadedTiles = 0;

    if (m_mapTileCache.size() > 600) {
        m_mapTileCache.clear();
    }

    for (int x = m_mapTileX0; x <= m_mapTileX1; ++x) {
        int wrappedX = ((x % n) + n) % n;
        for (int y = m_mapTileY0; y <= m_mapTileY1; ++y) {
            if (y < 0 || y >= n) continue;

            QString key = QString("%1/%2/%3").arg(zoom).arg(x).arg(y);
            m_mapExpectedTiles++;

            if (m_mapTileCache.contains(key)) {
                m_mapLoadedTiles++;
                continue;
            }

            m_mapPendingTiles.insert(key);

            QUrl tileUrl(QString("https://tile.openstreetmap.org/%1/%2/%3.png")
                         .arg(zoom).arg(wrappedX).arg(y));
            QNetworkRequest req(tileUrl);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_mapNet->get(req);
            reply->setProperty("mapType", "tile");
            reply->setProperty("tileKey", key);
            reply->setProperty("tileX", x);
            reply->setProperty("tileY", y);
        }
    }

    renderOrderMap();

    if (m_mapPendingTiles.isEmpty()) {
        if (m_mapExpectedTiles == 0) {
            m_mapStatusLabel->setText("Map tiles not available for this location.");
        } else {
            m_mapStatusLabel->setText("Map loaded instantly from cache.");
        }
    } else {
        m_mapStatusLabel->setText(QString("Loading map tiles... %1/%2")
                                  .arg(m_mapLoadedTiles)
                                  .arg(m_mapExpectedTiles));
    }
}

void MainWindow::renderOrderMap()
{
    if (m_mapImageSize.width() <= 0 || m_mapImageSize.height() <= 0) return;

    QPixmap mapPixmap(m_mapImageSize);
    mapPixmap.fill(QColor(26, 18, 8));
    QPainter painter(&mapPixmap);

    const int tileSize = 256;
    for (int x = m_mapTileX0; x <= m_mapTileX1; ++x) {
        for (int y = m_mapTileY0; y <= m_mapTileY1; ++y) {
            const QString key = QString("%1/%2/%3").arg(m_mapZoom).arg(x).arg(y);
            if (!m_mapTileCache.contains(key)) continue;
            const int px = qRound((x * tileSize) - m_mapTopLeftX);
            const int py = qRound((y * tileSize) - m_mapTopLeftY);
            painter.drawPixmap(px, py, m_mapTileCache.value(key));
        }
    }

    if (m_mapHasClientPin) {
        const int n = 1 << m_mapZoom;
        const double pinLatRad = qDegreesToRadians(m_mapClientPinLat);
        const double pinWorldX = ((m_mapClientPinLon + 180.0) / 360.0 * n) * tileSize;
        const double pinWorldY = ((1.0 - log(tan(pinLatRad) + 1.0 / cos(pinLatRad)) / M_PI) / 2.0 * n) * tileSize;
        const int pinX = qRound(pinWorldX - m_mapTopLeftX);
        const int pinY = qRound(pinWorldY - m_mapTopLeftY);

        if (pinX >= -20 && pinX <= (m_mapImageSize.width() + 20) && pinY >= -30 && pinY <= (m_mapImageSize.height() + 20)) {
            painter.setRenderHint(QPainter::Antialiasing, true);

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 120));
            painter.drawEllipse(QPoint(pinX + 1, pinY + 2), 7, 3);

            QPolygon pinTip;
            pinTip << QPoint(pinX, pinY)
                   << QPoint(pinX - 7, pinY - 14)
                   << QPoint(pinX + 7, pinY - 14);
            painter.setBrush(QColor(220, 53, 69));
            painter.drawPolygon(pinTip);

            painter.setBrush(QColor(220, 53, 69));
            painter.drawEllipse(QPoint(pinX, pinY - 22), 10, 10);
            painter.setBrush(Qt::white);
            painter.drawEllipse(QPoint(pinX, pinY - 22), 4, 4);
        }
    }

    m_mapCurrentPixmap = mapPixmap;
    m_mapHasPixmap = true;

    if (m_mapImageLabel) {
        m_mapImageLabel->setPixmap(mapPixmap.scaled(
            m_mapImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel) {
        m_mapFullscreenLabel->setPixmap(mapPixmap.scaled(
            m_mapFullscreenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    QLabel *mapTarget = nullptr;
    if (watched == m_mapImageLabel) {
        mapTarget = m_mapImageLabel;
    } else if (watched == m_mapFullscreenLabel) {
        mapTarget = m_mapFullscreenLabel;
    }

    if (watched == m_supplierMapImageLabel) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() > 0) m_supplierMapZoom = qMin(18, m_supplierMapZoom + 1);
            else m_supplierMapZoom = qMax(3, m_supplierMapZoom - 1);
            m_supplierMapImageSize = m_supplierMapImageLabel->size();
            refreshSupplierMap();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_supplierMapDragging = true;
                m_supplierMapDragStart = mouse->pos();
                m_supplierMapDragOffset = QPoint(0, 0);
                m_supplierMapImageSize = m_supplierMapImageLabel->size();
                const int tileSize = 256;
                const int n = 1 << m_supplierMapZoom;
                double latRad = qDegreesToRadians(m_supplierCenterLat);
                double xtile = (m_supplierCenterLon + 180.0) / 360.0 * n;
                double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;
                m_supplierMapDragCenterX = xtile * tileSize;
                m_supplierMapDragCenterY = ytile * tileSize;
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (m_supplierMapDragging) {
                QPoint delta = mouse->pos() - m_supplierMapDragStart;
                m_supplierMapDragOffset = delta;
                if (m_supplierMapHasPixmap) {
                    QPixmap shifted(m_supplierMapImageSize);
                    shifted.fill(QColor(26, 18, 8));
                    QPainter p(&shifted);
                    p.drawPixmap(delta, m_supplierMapCurrentPixmap);
                    m_supplierMapImageLabel->setPixmap(shifted);
                }
                return true;
            } else {
                bool hovered = false;
                for (const auto &pin : m_supplierPins) {
                    if (pin.rect.contains(mouse->pos())) {
                        QToolTip::showText(mouse->globalPosition().toPoint(),
                            QString("<b>%1</b><br/>Supplies: %2<br/>Status: %3")
                                .arg(pin.name).arg(pin.type)
                                .arg(pin.status == "Active" ? "<font color='green'>Open (Active)</font>" : "<font color='red'>Closed/Inactive</font>"),
                            m_supplierMapImageLabel);
                        hovered = true;
                        break;
                    }
                }
                if (!hovered) QToolTip::hideText();
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton && m_supplierMapDragging) {
                m_supplierMapDragging = false;
                double centerX = m_supplierMapDragCenterX - m_supplierMapDragOffset.x();
                double centerY = m_supplierMapDragCenterY - m_supplierMapDragOffset.y();
                const int n = 1 << m_supplierMapZoom;
                double lon = (centerX / (n * 256.0)) * 360.0 - 180.0;
                double latRad = atan(sinh(M_PI * (1.0 - 2.0 * centerY / (n * 256.0))));
                m_supplierCenterLat = qRadiansToDegrees(latRad);
                m_supplierCenterLon = lon;
                refreshSupplierMap();
                return true;
            }
        }
    }

    if (watched == m_supplierMapImageLabel) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() > 0) m_supplierMapZoom = qMin(18, m_supplierMapZoom + 1);
            else m_supplierMapZoom = qMax(3, m_supplierMapZoom - 1);
            m_supplierMapImageSize = m_supplierMapImageLabel->size();
            refreshSupplierMap();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_supplierMapDragging = true;
                m_supplierMapDragStart = mouse->pos();
                m_supplierMapDragOffset = QPoint(0, 0);
                m_supplierMapImageSize = m_supplierMapImageLabel->size();
                const int tileSize = 256;
                const int n = 1 << m_supplierMapZoom;
                double latRad = qDegreesToRadians(m_supplierCenterLat);
                double xtile = (m_supplierCenterLon + 180.0) / 360.0 * n;
                double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;
                m_supplierMapDragCenterX = xtile * tileSize;
                m_supplierMapDragCenterY = ytile * tileSize;
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (m_supplierMapDragging) {
                QPoint delta = mouse->pos() - m_supplierMapDragStart;
                m_supplierMapDragOffset = delta;
                if (m_supplierMapHasPixmap) {
                    QPixmap shifted(m_supplierMapImageSize);
                    shifted.fill(QColor(26, 18, 8));
                    QPainter p(&shifted);
                    p.drawPixmap(delta, m_supplierMapCurrentPixmap);
                    m_supplierMapImageLabel->setPixmap(shifted);
                }
                return true;
            } else {
                bool hovered = false;
                for (const auto &pin : m_supplierPins) {
                    if (pin.rect.contains(mouse->pos())) {
                        bool isOpen = false;
                        if (pin.status == "Active") {
                            QTime openT = QTime::fromString(pin.openTime, "HH:mm");
                            QTime closeT = QTime::fromString(pin.closeTime, "HH:mm");
                            QTime now = QTime::currentTime();
                            if (openT.isValid() && closeT.isValid()) {
                                if (openT <= closeT) isOpen = (now >= openT && now <= closeT);
                                else isOpen = (now >= openT || now <= closeT);
                            } else {
                                isOpen = true; // Default if no times
                            }
                        }
                        QString timeLabel = "";
                        if (!pin.openTime.isEmpty() && !pin.closeTime.isEmpty()) {
                            timeLabel = QString("<br/>Hours: %1 - %2").arg(pin.openTime).arg(pin.closeTime);
                        }
                        QToolTip::showText(mouse->globalPosition().toPoint(),
                            QString("<b>%1</b><br/>Supplies: %2<br/>Status: %3%4")
                                .arg(pin.name).arg(pin.type)
                                .arg(isOpen ? "<font color='green'>Open</font>" : "<font color='red'>Closed/Inactive</font>")
                                .arg(timeLabel),
                            m_supplierMapImageLabel);
                        hovered = true;
                        break;
                    }
                }
                if (!hovered) QToolTip::hideText();
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton && m_supplierMapDragging) {
                m_supplierMapDragging = false;
                double centerX = m_supplierMapDragCenterX - m_supplierMapDragOffset.x();
                double centerY = m_supplierMapDragCenterY - m_supplierMapDragOffset.y();
                const int n = 1 << m_supplierMapZoom;
                double lon = (centerX / (n * 256.0)) * 360.0 - 180.0;
                double latRad = atan(sinh(M_PI * (1.0 - 2.0 * centerY / (n * 256.0))));
                m_supplierCenterLat = qRadiansToDegrees(latRad);
                m_supplierCenterLon = lon;
                refreshSupplierMap();
                return true;
            }
        }
    }

    if (mapTarget) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() > 0) {
                m_mapZoom = qMin(18, m_mapZoom + 1);
            } else {
                m_mapZoom = qMax(3, m_mapZoom - 1);
            }
            m_mapImageSize = mapTarget->size();
            requestMapTiles(m_mapCenterLat, m_mapCenterLon);
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_mapDragging = true;
                m_mapDragStart = mouse->pos();
                m_mapDragOffset = QPoint(0, 0);
                m_mapImageSize = mapTarget->size();

                const int tileSize = 256;
                const int n = 1 << m_mapZoom;
                double latRad = qDegreesToRadians(m_mapCenterLat);
                double xtile = (m_mapCenterLon + 180.0) / 360.0 * n;
                double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;
                m_mapDragCenterX = xtile * tileSize;
                m_mapDragCenterY = ytile * tileSize;
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove) {
            if (m_mapDragging) {
                QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
                QPoint delta = mouse->pos() - m_mapDragStart;

                m_mapDragOffset = delta;
                if (m_mapHasPixmap) {
                    QPixmap shifted(m_mapImageSize);
                    shifted.fill(QColor(26, 18, 8));
                    QPainter p(&shifted);
                    p.drawPixmap(delta, m_mapCurrentPixmap);
                    mapTarget->setPixmap(shifted);
                }
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_mapDragging = false;
                double centerX = m_mapDragCenterX - m_mapDragOffset.x();
                double centerY = m_mapDragCenterY - m_mapDragOffset.y();

                const int n = 1 << m_mapZoom;
                double lon = (centerX / (n * 256.0)) * 360.0 - 180.0;
                double latRad = atan(sinh(M_PI * (1.0 - 2.0 * centerY / (n * 256.0))));
                double lat = qRadiansToDegrees(latRad);

                m_mapCenterLat = lat;
                m_mapCenterLon = lon;
                requestMapTiles(m_mapCenterLat, m_mapCenterLon);
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (mapTarget == m_mapFullscreenLabel && m_mapFullscreenDialog) {
                m_mapFullscreenDialog->close();
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// =============================================================================
// SUPPLIER MAP INTEGRATION
// =============================================================================

void MainWindow::setupSupplierMapTab()
{
    if (!ui_supplier || !ui_supplier->tabWidget) return;

    QWidget *mapTab = new QWidget(ui_supplier->tabWidget);
    mapTab->setObjectName("tab_supplier_map");

    QVBoxLayout *root = new QVBoxLayout(mapTab);
    root->setContentsMargins(18, 60, 18, 16);
    root->setSpacing(10);

    QHBoxLayout *controls = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Workshop Vicinity Map", mapTab);
    titleLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    m_supplierMapRefreshBtn = new QPushButton("Refresh Map", mapTab);
    m_supplierMapRefreshBtn->setCursor(Qt::PointingHandCursor);
    m_supplierMapRefreshBtn->setStyleSheet(
        "QPushButton{background:#8B6F47;color:white;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#a3845a;}"
        "QPushButton:pressed{background:#6b5535;}");

    m_supplierMapZoomInBtn = new QPushButton("Zoom +", mapTab);
    m_supplierMapZoomInBtn->setCursor(Qt::PointingHandCursor);
    m_supplierMapZoomInBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}");

    m_supplierMapZoomOutBtn = new QPushButton("Zoom -", mapTab);
    m_supplierMapZoomOutBtn->setCursor(Qt::PointingHandCursor);
    m_supplierMapZoomOutBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}");

    controls->addWidget(titleLabel);
    controls->addStretch();
    controls->addWidget(m_supplierMapRefreshBtn);
    controls->addWidget(m_supplierMapZoomInBtn);
    controls->addWidget(m_supplierMapZoomOutBtn);

    m_supplierMapStatusLabel = new QLabel("Loading suppliers...", mapTab);
    m_supplierMapStatusLabel->setWordWrap(true);
    m_supplierMapStatusLabel->setStyleSheet("color: #d4a96a; font-size: 12px;");

    m_supplierMapImageLabel = new QLabel(mapTab);
    m_supplierMapImageLabel->setMinimumSize(800, 500);
    m_supplierMapImageLabel->setAlignment(Qt::AlignCenter);
    m_supplierMapImageLabel->setStyleSheet("background: #1a1208; border: 2px solid #8B6F47; border-radius: 10px; color: #8B6F47;");
    m_supplierMapImageLabel->setText("Map preview will appear here.");
    m_supplierMapImageLabel->setMouseTracking(true); // Needed for hover
    m_supplierMapImageLabel->installEventFilter(this);

    root->addLayout(controls);
    root->addWidget(m_supplierMapStatusLabel);
    root->addWidget(m_supplierMapImageLabel, 1);

    ui_supplier->tabWidget->addTab(mapTab, "Vicinity Map");

    connect(m_supplierMapRefreshBtn, &QPushButton::clicked, this, &MainWindow::loadSupplierMapPins);
    connect(m_supplierMapZoomInBtn, &QPushButton::clicked, this, [this]() {
        m_supplierMapZoom = qMin(18, m_supplierMapZoom + 1);
        m_supplierMapImageSize = m_supplierMapImageLabel->size();
        refreshSupplierMap();
    });
    connect(m_supplierMapZoomOutBtn, &QPushButton::clicked, this, [this]() {
        m_supplierMapZoom = qMax(3, m_supplierMapZoom - 1);
        m_supplierMapImageSize = m_supplierMapImageLabel->size();
        refreshSupplierMap();
    });

    connect(ui_supplier->tabWidget, &QTabWidget::currentChanged, this, [this, mapTab](int index) {
        if (ui_supplier->tabWidget->widget(index) == mapTab) {
            loadSupplierMapPins();
        }
    });
}

void MainWindow::loadSupplierMapPins()
{
    m_supplierMapStatusLabel->setText("Geocoding suppliers...");
    m_supplierPins.clear();
    m_supplierGeocodePendingCount = 0;

    QSqlQuery q("SELECT SUPPLIER_ID, SUPPLIER_NAME, TYPE_NOTIFICATION, ACCOUNT_STATUS, ADDRESS, OPENING_TIME, CLOSING_TIME FROM SUPPLIERS");
    while (q.next()) {
        int id = q.value(0).toInt();
        QString name = q.value(1).toString();
        QString type = q.value(2).toString();
        QString status = q.value(3).toString();
        QString address = q.value(4).toString().trimmed();
        QString openTime = q.value(5).toString().trimmed();
        QString closeTime = q.value(6).toString().trimmed();

        if (!address.isEmpty()) {
            m_supplierGeocodePendingCount++;
            QUrl url("https://nominatim.openstreetmap.org/search");
            QUrlQuery query;
            query.addQueryItem("q", address + ", Tunisia");
            query.addQueryItem("format", "json");
            query.addQueryItem("limit", "1");
            url.setQuery(query);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_supplierMapNet->get(req);
            reply->setProperty("mapAction", "geocode_supplier");
            reply->setProperty("supp_id", id);
            reply->setProperty("supp_name", name);
            reply->setProperty("supp_type", type);
            reply->setProperty("supp_status", status);
            reply->setProperty("supp_open", openTime);
            reply->setProperty("supp_close", closeTime);
            reply->setProperty("address", address);
        }
    }

    if (m_supplierGeocodePendingCount == 0) {
        m_supplierMapStatusLabel->setText("No suppliers with addresses found.");
        refreshSupplierMap();
    }
}

void MainWindow::onSupplierGeocodeFinished(QNetworkReply *reply)
{
    if (!reply) return;
    QString action = reply->property("mapAction").toString();

    if (action == "geocode_supplier") {
        m_supplierGeocodePendingCount--;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray arr = doc.array();

            if (!arr.isEmpty()) {
                QJsonObject obj = arr.first().toObject();
                double lat = obj.value("lat").toString().toDouble();
                double lon = obj.value("lon").toString().toDouble();
                if (lat != 0 && lon != 0) {
                    SupplierPin pin;
                    pin.id = reply->property("supp_id").toInt();
                    pin.name = reply->property("supp_name").toString();
                    pin.type = reply->property("supp_type").toString();
                    pin.status = reply->property("supp_status").toString();
                    pin.openTime = reply->property("supp_open").toString();
                    pin.closeTime = reply->property("supp_close").toString();
                    pin.lat = lat;
                    pin.lon = lon;
                    m_supplierPins.append(pin);
                }
            }
        }
        
        if (m_supplierGeocodePendingCount <= 0) {
            m_supplierMapStatusLabel->setText(QString("Loaded %1 suppliers.").arg(m_supplierPins.size()));
            m_supplierMapImageSize = m_supplierMapImageLabel->size();
            refreshSupplierMap();
        }
        reply->deleteLater();
        return;
    }
    
    if (action == "map_tile") {
        QString key = reply->property("tileKey").toString();
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pix;
            if (pix.loadFromData(reply->readAll())) {
                m_supplierMapTileCache.insert(key, pix);
            } else {
                m_supplierMapTileErrors++;
            }
        } else {
            m_supplierMapTileErrors++;
        }
        
        m_supplierMapPendingTiles.remove(key);
        // Force repaint on every tile arrival
        QPixmap mapPixmap(m_supplierMapImageSize);
        mapPixmap.fill(QColor(26, 18, 8));
        QPainter painter(&mapPixmap);
        const int tileSize = 256;
        for (int x = m_supplierMapTileX0; x <= m_supplierMapTileX1; ++x) {
            for (int y = m_supplierMapTileY0; y <= m_supplierMapTileY1; ++y) {
                QString tkey = QString("%1/%2/%3").arg(m_supplierMapZoom).arg(x).arg(y);
                if (m_supplierMapTileCache.contains(tkey)) {
                    int px = qRound((x * tileSize) - m_supplierMapTopLeftX);
                    int py = qRound((y * tileSize) - m_supplierMapTopLeftY);
                    painter.drawPixmap(px, py, m_supplierMapTileCache.value(tkey));
                }
            }
        }

        const int n = 1 << m_supplierMapZoom;
        auto latToY = [n](double lat) {
            double rad = qDegreesToRadians(lat);
            return (1.0 - log(tan(rad) + 1.0 / cos(rad)) / M_PI) / 2.0 * n * 256;
        };
        auto lonToX = [n](double lon) {
            return (lon + 180.0) / 360.0 * n * 256;
        };

        int cx = qRound(lonToX(10.1815) - m_supplierMapTopLeftX); // Fixed Workshop
        int cy = qRound(latToY(36.8065) - m_supplierMapTopLeftY); // Fixed Workshop
        
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(QColor("#D4AF37"));
        painter.drawRect(cx - 10, cy - 10, 20, 20);
        painter.drawText(cx - 30, cy + 25, "Workshop");

        for (int i=0; i<m_supplierPins.size(); ++i) {
            auto &pin = m_supplierPins[i];
            int px = qRound(lonToX(pin.lon) - m_supplierMapTopLeftX);
            int py = qRound(latToY(pin.lat) - m_supplierMapTopLeftY);
            
            bool isOpen = false;
            if (pin.status == "Active") {
                QTime openT = QTime::fromString(pin.openTime, "HH:mm");
                QTime closeT = QTime::fromString(pin.closeTime, "HH:mm");
                QTime now = QTime::currentTime();
                if (openT.isValid() && closeT.isValid()) {
                    if (openT <= closeT) isOpen = (now >= openT && now <= closeT);
                    else isOpen = (now >= openT || now <= closeT);
                } else {
                    isOpen = true;
                }
            }
            QColor color = isOpen ? QColor(0, 255, 100, 200) : QColor(255, 50, 50, 200);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::white, 1));
            painter.drawEllipse(px - 8, py - 8, 16, 16);
            
            pin.rect = QRect(px - 10, py - 10, 20, 20);
        }

        m_supplierMapCurrentPixmap = mapPixmap;
        m_supplierMapHasPixmap = true;
        m_supplierMapImageLabel->setPixmap(mapPixmap);
        
        if (m_supplierMapPendingTiles.isEmpty()) {
            if (m_supplierMapTileErrors > 0)
                m_supplierMapStatusLabel->setText("Map loaded with missing tiles.");
            else
                m_supplierMapStatusLabel->setText(QString("Map loaded. %1 suppliers shown.").arg(m_supplierPins.size()));
        }
        reply->deleteLater();
        return;
    }
    
    reply->deleteLater();
}

void MainWindow::refreshSupplierMap()
{
    const int tileSize = 256;
    const int zoom = m_supplierMapZoom;
    const int n = 1 << zoom;
    
    if (m_supplierMapImageSize.width() < 64) m_supplierMapImageSize = QSize(800, 500);

    double latRad = qDegreesToRadians(m_supplierCenterLat);
    double xtile = (m_supplierCenterLon + 180.0) / 360.0 * n;
    double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;

    double worldX = xtile * tileSize;
    double worldY = ytile * tileSize;

    m_supplierMapTopLeftX = worldX - (m_supplierMapImageSize.width() / 2.0);
    m_supplierMapTopLeftY = worldY - (m_supplierMapImageSize.height() / 2.0);

    m_supplierMapTileX0 = static_cast<int>(floor(m_supplierMapTopLeftX / tileSize));
    m_supplierMapTileY0 = static_cast<int>(floor(m_supplierMapTopLeftY / tileSize));
    m_supplierMapTileX1 = static_cast<int>(floor((m_supplierMapTopLeftX + m_supplierMapImageSize.width() - 1) / tileSize));
    m_supplierMapTileY1 = static_cast<int>(floor((m_supplierMapTopLeftY + m_supplierMapImageSize.height() - 1) / tileSize));

    m_supplierMapTileCache.clear();
    m_supplierMapPendingTiles.clear();
    m_supplierMapTileErrors = 0;

    for (int x = m_supplierMapTileX0; x <= m_supplierMapTileX1; ++x) {
        int wrappedX = ((x % n) + n) % n;
        for (int y = m_supplierMapTileY0; y <= m_supplierMapTileY1; ++y) {
            if (y < 0 || y >= n) continue;
            QString key = QString("%1/%2/%3").arg(zoom).arg(x).arg(y);
            m_supplierMapPendingTiles.insert(key);

            QUrl tileUrl(QString("https://tile.openstreetmap.org/%1/%2/%3.png").arg(zoom).arg(wrappedX).arg(y));
            QNetworkRequest req(tileUrl);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_supplierMapNet->get(req);
            reply->setProperty("mapAction", "map_tile");
            reply->setProperty("tileKey", key);
        }
    }

    // Force immediate base paint so the UI updates instantly
    QPixmap mapPixmap(m_supplierMapImageSize);
    mapPixmap.fill(QColor(26, 18, 8));
    QPainter painter(&mapPixmap);
    
    auto latToY = [n](double lat) {
        double rad = qDegreesToRadians(lat);
        return (1.0 - log(tan(rad) + 1.0 / cos(rad)) / M_PI) / 2.0 * n * 256;
    };
    auto lonToX = [n](double lon) {
        return (lon + 180.0) / 360.0 * n * 256;
    };

    int cx = qRound(lonToX(10.1815) - m_supplierMapTopLeftX); // Fixed Workshop
    int cy = qRound(latToY(36.8065) - m_supplierMapTopLeftY); // Fixed Workshop
    
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QColor("#D4AF37"));
    painter.drawRect(cx - 10, cy - 10, 20, 20);
    painter.drawText(cx - 30, cy + 25, "Workshop");

    for (int i=0; i<m_supplierPins.size(); ++i) {
        auto &pin = m_supplierPins[i];
        int px = qRound(lonToX(pin.lon) - m_supplierMapTopLeftX);
        int py = qRound(latToY(pin.lat) - m_supplierMapTopLeftY);
        
        bool isOpen = false;
        if (pin.status == "Active") {
            QTime openT = QTime::fromString(pin.openTime, "HH:mm");
            QTime closeT = QTime::fromString(pin.closeTime, "HH:mm");
            QTime now = QTime::currentTime();
            if (openT.isValid() && closeT.isValid()) {
                if (openT <= closeT) isOpen = (now >= openT && now <= closeT);
                else isOpen = (now >= openT || now <= closeT);
            } else {
                isOpen = true;
            }
        }
        QColor color = isOpen ? QColor(0, 255, 100, 200) : QColor(255, 50, 50, 200);
        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawEllipse(px - 8, py - 8, 16, 16);
        
        pin.rect = QRect(px - 10, py - 10, 20, 20);
    }

    m_supplierMapCurrentPixmap = mapPixmap;
    m_supplierMapHasPixmap = true;
    m_supplierMapImageLabel->setPixmap(mapPixmap);
    
    if (m_supplierMapPendingTiles.isEmpty()) {
        m_supplierMapStatusLabel->setText(QString("Map loaded. %1 suppliers shown.").arg(m_supplierPins.size()));
    }
}

void MainWindow::checkSupplierVicinity(int supplierId)
{
    QSqlQuery q;
    if (supplierId == -1) {
        q.prepare("SELECT SUPPLIER_NAME, ACCOUNT_STATUS, ADDRESS FROM SUPPLIERS WHERE SUPPLIER_ID = (SELECT MAX(SUPPLIER_ID) FROM SUPPLIERS)");
    } else {
        q.prepare("SELECT SUPPLIER_NAME, ACCOUNT_STATUS, ADDRESS FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
        q.bindValue(":id", supplierId);
    }
    q.exec();
    if (q.next()) {
        QString name = q.value(0).toString();
        QString status = q.value(1).toString();
        QString address = q.value(2).toString().trimmed();
        
        if (!address.isEmpty()) {
            QUrl url("https://nominatim.openstreetmap.org/search");
            QUrlQuery query;
            query.addQueryItem("q", address + ", Tunisia");
            query.addQueryItem("format", "json");
            query.addQueryItem("limit", "1");
            url.setQuery(query);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_supplierMapNet->get(req);
            
            connect(reply, &QNetworkReply::finished, this, [this, reply, name, status]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    QJsonArray arr = doc.array();
                    if (!arr.isEmpty()) {
                        QJsonObject obj = arr.first().toObject();
                        double lat = obj.value("lat").toString().toDouble();
                        double lon = obj.value("lon").toString().toDouble();
                        
                        double dist = sqrt(pow(lat - m_supplierCenterLat, 2) + pow(lon - m_supplierCenterLon, 2));
                        if (dist < 0.5) {
                            QString verb = status == "Active" ? "opened" : "closed down";
                            QMessageBox::information(this, "Vicinity Alert", QString("Alert: Supplier '%1' in the vicinity has %2!").arg(name).arg(verb));
                        }
                    }
                }
                reply->deleteLater();
            });
        }
    }
}

// =============================================================================
// SUPPLIER NOTIFICATION BELL
// =============================================================================

void MainWindow::checkAndPostSupplierNotifications()
{
    QSqlQuery qOut("SELECT SUPPLIER_ID, SUPPLIER_NAME, ACCOUNT_STATUS, STOCK_STATUS, REGISTRATION_DATE, NOTIFICATIONS_JSON FROM SUPPLIERS");
    QDateTime now = QDateTime::currentDateTime();
    int unreadTotal = 0;

    while (qOut.next()) {
        int id = qOut.value(0).toInt();
        QString nm = qOut.value(1).toString();
        QString accStatus = qOut.value(2).toString();
        QString stkStatus = qOut.value(3).toString();
        QDateTime regDate = qOut.value(4).toDateTime();
        QString jsonStr = qOut.value(5).toString();

        QJsonArray notifs;
        if (!jsonStr.isEmpty()) {
            notifs = QJsonDocument::fromJson(jsonStr.toUtf8()).array();
        }

        bool changed = false;
        auto hasNotif = [&](const QString& type) {
            for (int i=0; i<notifs.size(); ++i) {
                if (notifs[i].toObject()["type"].toString() == type) return true;
            }
            return false;
        };

        // 1. New supplier
        if (accStatus == "Active" && regDate.daysTo(now) <= 7) {
            if (!hasNotif("NEW_SUPPLIER")) {
                QJsonObject n;
                n["id"] = QString::number(id) + "_new_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "NEW_SUPPLIER";
                n["msg"] = QString("New supplier '%1' (ID %2) has just opened!").arg(nm).arg(id);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 0;
                notifs.append(n);
                changed = true;
            }
        }

        // 2. Closed
        if (accStatus != "Active") {
            if (!hasNotif("SUPPLIER_CLOSED")) {
                QJsonObject n;
                n["id"] = QString::number(id) + "_closed_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "SUPPLIER_CLOSED";
                n["msg"] = QString("Supplier '%1' (ID %2) has closed / gone inactive.").arg(nm).arg(id);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 0;
                notifs.append(n);
                changed = true;
            }
        }

        // 3. Stock
        if (stkStatus == "Out of Stock" || stkStatus == "Low Stock") {
            bool hasUnreadStock = false;
            for (int i=0; i<notifs.size(); ++i) {
                QJsonObject obj = notifs[i].toObject();
                if (obj["type"].toString() == "STOCK_ALERT" && obj["is_read"].toInt() == 0) {
                    hasUnreadStock = true;
                    break;
                }
            }
            if (!hasUnreadStock) {
                QJsonObject n;
                n["id"] = QString::number(id) + "_stock_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "STOCK_ALERT";
                n["msg"] = QString("Supplier '%1' (ID %2) is now: %3.").arg(nm).arg(id).arg(stkStatus);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 0;
                notifs.append(n);
                changed = true;
            }
        }

        if (changed) {
            QString newJson = QString::fromUtf8(QJsonDocument(notifs).toJson(QJsonDocument::Compact));
            QSqlQuery u;
            u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
            u.bindValue(":json", newJson);
            u.bindValue(":id", id);
            u.exec();
        }

        for (int i=0; i<notifs.size(); ++i) {
            if (notifs[i].toObject()["is_read"].toInt() == 0) unreadTotal++;
        }
    }

    // Update bell badge (red dot) if there are unread notifications
    if (m_supplierBellBtn) {
        if (unreadTotal > 0) {
            m_supplierBellBtn->setStyleSheet(
                "QPushButton { background-color: #c0392b; border-radius: 22px; color: white; font-size: 20px; border: none; }"
                "QPushButton:hover { background-color: #e74c3c; }");
        } else {
            m_supplierBellBtn->setStyleSheet(
                "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
                "QPushButton:hover { background-color: #a3845a; }"
                "QPushButton:pressed{ background-color: #6b5535; }");
        }
    }
}

void MainWindow::onSupplierBellClicked()
{
    // Refresh first
    checkAndPostSupplierNotifications();

    struct NotifItem {
        int supplierId;
        QString id;
        QString type;
        QString msg;
        QString date;
        int is_read;
    };
    QList<NotifItem> allNotifs;

    QSqlQuery q("SELECT SUPPLIER_ID, NOTIFICATIONS_JSON FROM SUPPLIERS WHERE NOTIFICATIONS_JSON IS NOT NULL");
    while (q.next()) {
        int sId = q.value(0).toInt();
        QString jsonStr = q.value(1).toString();
        if (jsonStr.isEmpty()) continue;
        QJsonArray arr = QJsonDocument::fromJson(jsonStr.toUtf8()).array();
        for (int i=0; i<arr.size(); i++) {
            QJsonObject o = arr[i].toObject();
            allNotifs.append({sId, o["id"].toString(), o["type"].toString(), o["msg"].toString(), o["date"].toString(), o["is_read"].toInt()});
        }
    }

    std::sort(allNotifs.begin(), allNotifs.end(), [](const NotifItem& a, const NotifItem& b) {
        return a.date > b.date; // simple string compare on dd/MM HH:mm
    });

    // ---- Build the popup dialog ----
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Supplier Notifications");
    dlg->setMinimumSize(560, 420);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet(
        "QDialog { background: #1a1208; }"
        "QLabel { color: #f5e6cc; }"
        "QScrollArea { background: transparent; border: none; }");

    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(10);

    QLabel *title = new QLabel((QString(QChar(0xD83D)) + QChar(0xDD14)) + "  Supplier Notifications", dlg);
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #D4AF37;");
    root->addWidget(title);

    QScrollArea *scroll = new QScrollArea(dlg);
    scroll->setWidgetResizable(true);
    QWidget *inner = new QWidget;
    inner->setStyleSheet("background: transparent;");
    QVBoxLayout *list = new QVBoxLayout(inner);
    list->setSpacing(8);

    int count = 0;
    for (const NotifItem& n : allNotifs) {
        QString type = n.type;
        QString msg  = n.msg;
        QString dt   = n.date;
        bool isRead  = n.is_read == 1;

        QString icon;
        QString bgColor;
        if (type == "NEW_SUPPLIER")    { icon = QString(QChar(0x2705)); bgColor = "#1e3d1e"; }
        else if (type == "SUPPLIER_CLOSED") { icon = QString(QChar(0x26D4)); bgColor = "#3d1e1e"; }
        else                           { icon = QString(QChar(0x26A0)); bgColor = "#3d2e00"; }

        QFrame *card = new QFrame(inner);
        card->setStyleSheet(QString("background: %1; border-radius: 10px; border: 1px solid #8B6F47;").arg(bgColor));
        QHBoxLayout *cl = new QHBoxLayout(card);

        QLabel *ico = new QLabel(icon, card);
        ico->setStyleSheet("font-size: 20px; background: transparent;");
        ico->setFixedWidth(30);

        QVBoxLayout *tl = new QVBoxLayout;
        QLabel *lmsg = new QLabel(msg, card);
        lmsg->setWordWrap(true);
        lmsg->setStyleSheet(QString("font-weight: %1; font-size: 13px; background: transparent; color: %2;")
            .arg(isRead ? "normal" : "bold")
            .arg(isRead ? "#aaa" : "#f5e6cc"));
        QLabel *ldt = new QLabel(dt, card);
        ldt->setStyleSheet("font-size: 11px; color: #8B6F47; background: transparent;");
        tl->addWidget(lmsg);
        tl->addWidget(ldt);

        QPushButton *markBtn = new QPushButton(isRead ? "Read" : "Mark Read", card);
        markBtn->setFixedSize(90, 28);
        markBtn->setEnabled(!isRead);
        markBtn->setStyleSheet(
            "QPushButton { background: #8B6F47; color: white; border-radius: 6px; font-size: 11px; border: none; padding: 2px 6px; }"
            "QPushButton:hover { background: #a3845a; }"
            "QPushButton:disabled { background: #444; color: #888; }");
            
        int sId = n.supplierId;
        QString nId = n.id;
        connect(markBtn, &QPushButton::clicked, dlg, [sId, nId, markBtn, lmsg]() {
            QSqlQuery qGet;
            qGet.prepare("SELECT NOTIFICATIONS_JSON FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
            qGet.bindValue(":id", sId);
            if (qGet.exec() && qGet.next()) {
                QJsonArray arr = QJsonDocument::fromJson(qGet.value(0).toString().toUtf8()).array();
                for (int i=0; i<arr.size(); i++) {
                    QJsonObject o = arr[i].toObject();
                    if (o["id"].toString() == nId) {
                        o["is_read"] = 1;
                        arr[i] = o;
                        break;
                    }
                }
                QSqlQuery u;
                u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
                u.bindValue(":json", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
                u.bindValue(":id", sId);
                u.exec();
            }
            markBtn->setText("Read");
            markBtn->setEnabled(false);
            lmsg->setStyleSheet("font-weight: normal; font-size: 13px; background: transparent; color: #aaa;");
        });

        cl->addWidget(ico);
        cl->addLayout(tl, 1);
        cl->addWidget(markBtn);
        list->addWidget(card);
        count++;
    }

    if (count == 0) {
        QLabel *empty = new QLabel("No notifications yet.", inner);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #8B6F47; font-size: 14px;");
        list->addWidget(empty);
    }

    list->addStretch();
    scroll->setWidget(inner);
    root->addWidget(scroll, 1);

    QPushButton *markAll = new QPushButton("Mark All as Read", dlg);
    markAll->setStyleSheet(
        "QPushButton { background: #8B6F47; color: white; border-radius: 8px; font-weight: bold; padding: 8px 20px; border: none; }"
        "QPushButton:hover { background: #a3845a; }");
    connect(markAll, &QPushButton::clicked, dlg, [this, dlg]() {
        QSqlQuery q("SELECT SUPPLIER_ID, NOTIFICATIONS_JSON FROM SUPPLIERS WHERE NOTIFICATIONS_JSON LIKE '%\"is_read\":0%'");
        while(q.next()) {
            int sId = q.value(0).toInt();
            QJsonArray arr = QJsonDocument::fromJson(q.value(1).toString().toUtf8()).array();
            for(int i=0; i<arr.size(); i++) {
                QJsonObject o = arr[i].toObject();
                o["is_read"] = 1;
                arr[i] = o;
            }
            QSqlQuery u;
            u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
            u.bindValue(":json", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
            u.bindValue(":id", sId);
            u.exec();
        }
        if (m_supplierBellBtn)
            m_supplierBellBtn->setStyleSheet(
                "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
                "QPushButton:hover { background-color: #a3845a; }");
        dlg->accept();
    });
    root->addWidget(markAll);

    dlg->exec();
    checkAndPostSupplierNotifications();
}

void MainWindow::playSupplierSuccessAnimation(const QString &supplierName) {
    // 1. Flash green on form fields
    QList<QWidget*> widgets = { ui_supplier->le_nom, ui_supplier->le_id, ui_supplier->le_adresse, ui_supplier->le_type };
    for (auto w : widgets) {
        if (!w) continue;
        QString oldStyle = w->styleSheet();
        w->setStyleSheet(oldStyle + " background-color: rgba(76, 175, 80, 0.3); border: 2px solid #4CAF50;");
        QTimer::singleShot(800, [=]() { w->setStyleSheet(oldStyle); });
    }

    // 2. Flying Card
    QLabel *flyer = new QLabel(this);
    // Use QChar combinations to avoid invalid universal character errors in MinGW
    flyer->setText((QString(QChar(0xD83D)) + QChar(0xDE9A)) + " " + supplierName);
    flyer->setFixedSize(160, 45);
    flyer->setAlignment(Qt::AlignCenter);
    flyer->setStyleSheet("background: #8B6F47; color: white; border: 2px solid #D4AF37; border-radius: 12px; font-weight: bold; font-family: 'Segoe UI';");
    
    QPoint startPos = ui_supplier->groupBox_gestion->mapTo(this, QPoint(150, 200));
    QPoint endPos = QPoint(200, 100); 

    flyer->move(startPos);
    flyer->show();
    flyer->raise();

    QPropertyAnimation *moveAnim = new QPropertyAnimation(flyer, "pos");
    moveAnim->setDuration(1000);
    moveAnim->setStartValue(startPos);
    moveAnim->setEndValue(endPos);
    moveAnim->setEasingCurve(QEasingCurve::InOutBack);

    QPropertyAnimation *scaleAnim = new QPropertyAnimation(flyer, "size");
    scaleAnim->setDuration(1000);
    scaleAnim->setStartValue(QSize(160, 45));
    scaleAnim->setEndValue(QSize(10, 10));

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(moveAnim);
    group->addAnimation(scaleAnim);
    
    connect(group, &QParallelAnimationGroup::finished, this, [=]() {
        flyer->hide();
        flyer->deleteLater();
        
        // 4. Confetti (Supplier network chips / blue & gold)
        for (int i=0; i<15; ++i) {
            QLabel *chip = new QLabel(this);
            chip->setFixedSize(8, 8);
            chip->setStyleSheet(QString("background: %1; border-radius: 3px; border: 1px solid rgba(0,0,0,0.2);")
                                .arg(i%2==0 ? "#8B6F47" : "#3498db"));
            QPoint cStart = endPos + QPoint(rand()%40-20, rand()%20-10);
            chip->move(cStart);
            chip->show();
            chip->raise();
            
            QPropertyAnimation *cMove = new QPropertyAnimation(chip, "pos");
            cMove->setDuration(600 + rand()%600);
            cMove->setStartValue(cStart);
            cMove->setEndValue(cStart + QPoint(rand()%140-70, rand()%140-30));
            cMove->setEasingCurve(QEasingCurve::OutCubic);
            
            QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(chip);
            chip->setGraphicsEffect(op);
            QPropertyAnimation *cFade = new QPropertyAnimation(op, "opacity");
            cFade->setDuration(cMove->duration());
            cFade->setStartValue(1.0);
            cFade->setEndValue(0.0);
            
            QParallelAnimationGroup *cGrp = new QParallelAnimationGroup(this);
            cGrp->addAnimation(cMove);
            cGrp->addAnimation(cFade);
            connect(cGrp, &QParallelAnimationGroup::finished, chip, &QLabel::deleteLater);
            cGrp->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);

    // 5. Toast notification
    QLabel *toast = new QLabel(QString(QChar(0x2705)) + " " + supplierName + " connected!", this);
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E3C2D, stop:1 #12251B);"
        "color: #4CAF50; border: 2.5px solid #2E8B57; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
    QGraphicsDropShadowEffect *tShadow = new QGraphicsDropShadowEffect(toast);
    tShadow->setBlurRadius(15);
    tShadow->setOffset(0, 4);
    toast->setGraphicsEffect(tShadow);

    QPoint toastEnd = QPoint(this->width() - 350, 30);
    QPoint toastStart = QPoint(this->width() + 10, 30);
    toast->move(toastStart);
    toast->show();
    toast->raise();

    QPropertyAnimation *tIn = new QPropertyAnimation(toast, "pos");
    tIn->setDuration(700);
    tIn->setStartValue(toastStart);
    tIn->setEndValue(toastEnd);
    tIn->setEasingCurve(QEasingCurve::OutBack);

    QTimer::singleShot(3000, [=]() {
        QPropertyAnimation *tOut = new QPropertyAnimation(toast, "pos");
        tOut->setDuration(500);
        tOut->setStartValue(toastEnd);
        tOut->setEndValue(toastStart);
        tOut->setEasingCurve(QEasingCurve::InBack);
        connect(tOut, &QPropertyAnimation::finished, toast, &QLabel::deleteLater);
        tOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
    tIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::playSupplierModifyAnimation(const QString &supplierName) {
    // 1. Flash blue on form fields
    QList<QWidget*> widgets = { ui_supplier->le_nom, ui_supplier->le_id, ui_supplier->le_adresse, ui_supplier->le_type };
    for (auto w : widgets) {
        if (!w) continue;
        QString oldStyle = w->styleSheet();
        w->setStyleSheet(oldStyle + " background-color: rgba(52, 152, 219, 0.3); border: 2px solid #3498DB;");
        QTimer::singleShot(800, [=]() { w->setStyleSheet(oldStyle); });
    }

    // 2. Toast notification
    QLabel *toast = new QLabel(QString(QChar(0x270F)) + " " + supplierName + " updated!", this); // ✏️
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1A252C, stop:1 #10161A);"
        "color: #3498DB; border: 2.5px solid #2980B9; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
    QGraphicsDropShadowEffect *tShadow = new QGraphicsDropShadowEffect(toast);
    tShadow->setBlurRadius(15);
    tShadow->setOffset(0, 4);
    toast->setGraphicsEffect(tShadow);

    QPoint toastEnd = QPoint(this->width() - 350, 30);
    QPoint toastStart = QPoint(this->width() + 10, 30);
    toast->move(toastStart);
    toast->show();
    toast->raise();

    QPropertyAnimation *tIn = new QPropertyAnimation(toast, "pos");
    tIn->setDuration(700);
    tIn->setStartValue(toastStart);
    tIn->setEndValue(toastEnd);
    tIn->setEasingCurve(QEasingCurve::OutBack);

    QTimer::singleShot(3000, [=]() {
        QPropertyAnimation *tOut = new QPropertyAnimation(toast, "pos");
        tOut->setDuration(500);
        tOut->setStartValue(toastEnd);
        tOut->setEndValue(toastStart);
        tOut->setEasingCurve(QEasingCurve::InBack);
        connect(tOut, &QPropertyAnimation::finished, toast, &QLabel::deleteLater);
        tOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
    tIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::playSupplierDeleteAnimation(const QString &supplierName) {
    // 1. Toast notification
    QLabel *toast = new QLabel(QString(QChar(0xD83D)) + QChar(0xDDD1) + " " + supplierName + " removed.", this); // 🗑️
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3C1E1E, stop:1 #251212);"
        "color: #E74C3C; border: 2.5px solid #C0392B; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
    QGraphicsDropShadowEffect *tShadow = new QGraphicsDropShadowEffect(toast);
    tShadow->setBlurRadius(15);
    tShadow->setOffset(0, 4);
    toast->setGraphicsEffect(tShadow);

    QPoint toastEnd = QPoint(this->width() - 350, 30);
    QPoint toastStart = QPoint(this->width() + 10, 30);
    toast->move(toastStart);
    toast->show();
    toast->raise();

    QPropertyAnimation *tIn = new QPropertyAnimation(toast, "pos");
    tIn->setDuration(700);
    tIn->setStartValue(toastStart);
    tIn->setEndValue(toastEnd);
    tIn->setEasingCurve(QEasingCurve::OutBack);

    QTimer::singleShot(3000, [=]() {
        QPropertyAnimation *tOut = new QPropertyAnimation(toast, "pos");
        tOut->setDuration(500);
        tOut->setStartValue(toastEnd);
        tOut->setEndValue(toastStart);
        tOut->setEasingCurve(QEasingCurve::InBack);
        connect(tOut, &QPropertyAnimation::finished, toast, &QLabel::deleteLater);
        tOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
    tIn->start(QAbstractAnimation::DeleteWhenStopped);

    // 2. Confetti (Red and Gray chips falling from the table area)
    QPoint endPos = ui_supplier->tableView->mapTo(this, QPoint(ui_supplier->tableView->width() / 2, ui_supplier->tableView->height() / 2));
    
    for (int i=0; i<15; ++i) {
        QLabel *chip = new QLabel(this);
        chip->setFixedSize(8, 8);
        chip->setStyleSheet(QString("background: %1; border-radius: 3px; border: 1px solid rgba(0,0,0,0.2);")
                            .arg(i%2==0 ? "#E74C3C" : "#95A5A6"));
        QPoint cStart = endPos + QPoint(rand()%100-50, rand()%40-20);
        chip->move(cStart);
        chip->show();
        chip->raise();
        
        QPropertyAnimation *cMove = new QPropertyAnimation(chip, "pos");
        cMove->setDuration(800 + rand()%600);
        cMove->setStartValue(cStart);
        cMove->setEndValue(cStart + QPoint(rand()%60-30, 100 + rand()%100)); // Falling down
        cMove->setEasingCurve(QEasingCurve::InQuad); // Accelerate downwards
        
        QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(chip);
        chip->setGraphicsEffect(op);
        QPropertyAnimation *cFade = new QPropertyAnimation(op, "opacity");
        cFade->setDuration(cMove->duration());
        cFade->setStartValue(1.0);
        cFade->setEndValue(0.0);
        
        QParallelAnimationGroup *cGrp = new QParallelAnimationGroup(this);
        cGrp->addAnimation(cMove);
        cGrp->addAnimation(cFade);
        connect(cGrp, &QParallelAnimationGroup::finished, chip, &QLabel::deleteLater);
        cGrp->start(QAbstractAnimation::DeleteWhenStopped);
    }
}






