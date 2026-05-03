#include "equipment.h"
#include "creative_components.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_equipment_management.h"
#include "nexuswidget.h"
#include "costswidget.h"
#include "weatherassistant.h"
#include <QShortcut>
#include <QToolTip>
#include <QDateTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMovie>
#include <QBuffer>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QStringListModel>
#include <QCompleter>
#include <QFileInfo>
#include <QDir>

#include <QJsonArray>

#include <QJsonDocument>

#include <QLineEdit>

#include <QMenu>

#include <QPrinter>

#include <QRandomGenerator>

#include <QRegularExpressionValidator>

#include <QSqlQueryModel>

#include <QStandardItemModel>

#include <QStyledItemDelegate>
#include <QStyle>
#include <QFontMetrics>
#include <QHeaderView>
#include <QItemSelectionModel>

#include <QtMath>

#include <algorithm>
#include <QEasingCurve>
#include <QRandomGenerator>

// AiSuggestPanel implementation moved to aisuggestpanel.cpp



namespace {

QString trKey(const QString &key)

{

    return QCoreApplication::translate("QObject", key.toUtf8().constData());

}

class TimelineCardDelegate final : public QStyledItemDelegate {
public:
    explicit TimelineCardDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        Q_UNUSED(index);
        return QSize(option.rect.width(), 90);
    }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (!index.isValid() || index.column() != 0) {
            return;
        }

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        p->save();
        p->setRenderHint(QPainter::Antialiasing);

        const QRect base = opt.rect.adjusted(12, 6, -12, -6);
        const int lineX = base.left() + 12;
        const int dotY = base.center().y();
        const int dotBaseR = 4;

        const auto *view = qobject_cast<const QAbstractItemView*>(opt.widget);
        const int activeRow = view ? view->property("timelineSelectedRow").toInt() : -1;
        double pulse = view ? view->property("timelinePulse").toDouble() : 0.0;
        double expand = view ? view->property("timelineExpand").toDouble() : 0.0;
        if (pulse < 0.0) pulse = 0.0;
        if (pulse > 1.0) pulse = 1.0;
        if (expand < 0.0) expand = 0.0;
        if (expand > 1.0) expand = 1.0;
        const double pulseEase = (pulse <= 0.5) ? (pulse * 2.0) : (2.0 - pulse * 2.0);
        const bool isActive = (activeRow == index.row());
        const double expandFactor = isActive ? expand : 0.0;
        const int lift = isActive ? static_cast<int>(3 * pulseEase + 2 * expandFactor) : 0;

        QColor accent(212, 175, 55);

        QColor lineColor(139, 111, 71, 180);
        if (isActive) {
            lineColor = QColor(accent.red(), accent.green(), accent.blue(), 200);
        }
        QColor cardFill(255, 255, 255, 245);
        QColor cardBorder(210, 180, 140, 200);
        if (opt.state & QStyle::State_Selected) {
            cardBorder = QColor(139, 111, 71, 255);
            cardFill = QColor(255, 248, 235, 255);
        }

        if (isActive && pulseEase > 0.0) {
            const int glowAlpha = static_cast<int>(90 * pulseEase);
            QRect glowRect = base.adjusted(20, -lift - 2, -2, -lift + 2);
            QPainterPath glowPath;
            glowPath.addRoundedRect(glowRect, 12, 12);
            p->setPen(Qt::NoPen);
            QColor glowColor = accent;
            glowColor.setAlpha(glowAlpha);
            p->setBrush(glowColor);
            p->drawPath(glowPath);

            const int borderAlpha = qMin(255, static_cast<int>(200 + 55 * pulseEase));
            cardBorder = QColor(accent.red(), accent.green(), accent.blue(), borderAlpha);
        }

        p->setPen(QPen(lineColor, 2));
        p->drawLine(QPoint(lineX, base.top() + 4), QPoint(lineX, base.bottom() - 4));

        p->setBrush(lineColor);
        p->setPen(Qt::NoPen);
        const int dotR = dotBaseR + (isActive ? static_cast<int>(2 * pulseEase + 1 * expandFactor) : 0);
        p->drawEllipse(QPoint(lineX, dotY), dotR, dotR);

        QRect cardRect = base.adjusted(24, -lift, 0, -lift);
        QPainterPath cardPath;
        cardPath.addRoundedRect(cardRect, 10, 10);
        p->setBrush(cardFill);
        p->setPen(QPen(cardBorder, 1));
        p->drawPath(cardPath);

        const QAbstractItemModel *model = index.model();
        const int row = index.row();
        const QString id = model->data(model->index(row, 0)).toString();
        const QString type = model->data(model->index(row, 1)).toString();
        const QString desc = model->data(model->index(row, 2)).toString();
        const QString status = model->data(model->index(row, 3)).toString();
        const QString price = model->data(model->index(row, 4)).toString();
        const QString date = model->data(model->index(row, 5)).toString();
        const QString event = model->data(model->index(row, 6)).toString();
        const QString eventLower = event.toLower();
        if (eventLower.contains("delete")) {
            accent = QColor(176, 60, 60);
        } else if (eventLower.contains("modify")) {
            accent = QColor(70, 120, 160);
        } else if (eventLower.contains("add")) {
            accent = QColor(60, 150, 95);
        }

        const QString title = QString("Equipment %1 - %2")
                                  .arg(id.isEmpty() ? "-" : id, type.isEmpty() ? "-" : type);
        const QString meta = QString("Status: %1 | Price: %2")
                                 .arg(status.isEmpty() ? "-" : status, price.isEmpty() ? "-" : price);
        const QString descLine = desc.isEmpty() ? "-" : desc;

        QFont titleFont = opt.font;
        titleFont.setBold(true);
        titleFont.setPointSize(qMax(8, titleFont.pointSize() + 1));
        QFont metaFont = opt.font;
        metaFont.setPointSize(qMax(8, metaFont.pointSize() - 1));

        const int textLeft = cardRect.left() + 12;
        const int textRight = cardRect.right() - 12;
        QRect titleRect(textLeft, cardRect.top() + 10, textRight - textLeft, 18);
        QRect metaRect(textLeft, cardRect.top() + 30, textRight - textLeft, 16);
        const int descHeight = 16 + static_cast<int>(24 * expandFactor);
        QRect descRect(textLeft, cardRect.top() + 48, textRight - textLeft, descHeight);

        p->setFont(titleFont);
        p->setPen(QColor(42, 30, 16));
        QFontMetrics titleMetrics(titleFont);
        p->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter,
                    titleMetrics.elidedText(title, Qt::ElideRight, titleRect.width()));

        p->setFont(metaFont);
        p->setPen(QColor(90, 70, 50));
        QFontMetrics metaMetrics(metaFont);
        p->drawText(metaRect, Qt::AlignLeft | Qt::AlignVCenter,
                    metaMetrics.elidedText(meta, Qt::ElideRight, metaRect.width()));
        if (expandFactor < 0.35) {
            p->drawText(descRect, Qt::AlignLeft | Qt::AlignVCenter,
                        metaMetrics.elidedText(descLine, Qt::ElideRight, descRect.width()));
        } else {
            p->drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, descLine);
        }

        p->setPen(QColor(125, 95, 70));
        p->drawText(titleRect, Qt::AlignRight | Qt::AlignVCenter, date);

        if (isActive && expandFactor > 0.2) {
            double scan = view ? view->property("timelineScan").toDouble() : 0.0;
            if (scan < 0.0) scan = 0.0;
            if (scan > 1.0) scan = 1.0;
            const int bandY = cardRect.top() + 8 + static_cast<int>((cardRect.height() - 16) * scan);
            QRect bandRect(cardRect.left() + 8, bandY - 5, cardRect.width() - 16, 10);
            QLinearGradient bandGrad(bandRect.left(), bandRect.top(), bandRect.right(), bandRect.top());
            QColor edge = accent;
            edge.setAlpha(0);
            QColor center = accent;
            center.setAlpha(90);
            bandGrad.setColorAt(0.0, edge);
            bandGrad.setColorAt(0.5, center);
            bandGrad.setColorAt(1.0, edge);
            p->setPen(Qt::NoPen);
            p->setBrush(bandGrad);
            p->drawRoundedRect(bandRect, 6, 6);

            const QString badgeText = event.isEmpty() ? "EVENT" : event.toUpper();
            const int badgeW = 90;
            const int badgeH = 18;
            QRect badgeRect(cardRect.right() - badgeW - 8, metaRect.top() - 2, badgeW, badgeH);
            p->setBrush(accent);
            p->setPen(Qt::NoPen);
            p->drawRoundedRect(badgeRect, 8, 8);
            QFont badgeFont = metaFont;
            badgeFont.setBold(true);
            badgeFont.setPointSize(qMax(8, badgeFont.pointSize() - 1));
            p->setFont(badgeFont);
            QColor badgeTextColor = (accent.lightness() > 140) ? QColor(30, 20, 12) : QColor(255, 255, 255);
            p->setPen(badgeTextColor);
            p->drawText(badgeRect, Qt::AlignCenter, badgeText);
        }

        if (expandFactor > 0.05) {
            const int extraTop = descRect.bottom() + 6;
            const int extraBottom = cardRect.bottom() - 8;
            if (extraBottom > extraTop) {
                QRect barRect(textLeft, extraTop, textRight - textLeft - 90, 10);
                if (barRect.width() > 40) {
                    const QString key = id + type + status;
                    const int h = qAbs(static_cast<int>(qHash(key))) % 100;
                    double score = qMax(0.18, h / 100.0);
                    const double anim = 0.6 + 0.4 * expandFactor;
                    int fillW = static_cast<int>(barRect.width() * score * anim);

                    p->setPen(Qt::NoPen);
                    p->setBrush(QColor(225, 215, 200, 200));
                    p->drawRoundedRect(barRect, 4, 4);

                    QLinearGradient grad(barRect.topLeft(), barRect.topRight());
                    grad.setColorAt(0, QColor(212, 175, 55, 220));
                    grad.setColorAt(1, QColor(139, 111, 71, 220));
                    p->setBrush(grad);
                    QRect fillRect = barRect;
                    fillRect.setWidth(fillW);
                    p->drawRoundedRect(fillRect, 4, 4);

                    if (isActive) {
                        const int orbX = barRect.left() + static_cast<int>(barRect.width() * (0.15 + 0.7 * pulseEase));
                        const int orbR = 3 + static_cast<int>(2 * pulseEase);
                        p->setBrush(QColor(255, 240, 200, 220));
                        p->drawEllipse(QPoint(orbX, barRect.center().y()), orbR, orbR);
                    }

                    const QDate parsed = QDate::fromString(date, "yyyy-MM-dd");
                    QString ageText = "Age: -";
                    if (parsed.isValid()) {
                        const int days = parsed.daysTo(QDate::currentDate());
                        ageText = QString("Age: %1d").arg(days);
                    }
                    QRect ageRect(barRect.right() + 8, barRect.top() - 4,
                                  cardRect.right() - barRect.right() - 12, 16);
                    p->setFont(metaFont);
                    p->setPen(QColor(110, 85, 65));
                    p->drawText(ageRect, Qt::AlignRight | Qt::AlignVCenter, ageText);
                }
            }
        }

        p->restore();
    }
};



void setTrKey(QWidget *widget, const QString &key)

{

    if (widget) {

        widget->setProperty("trKey", key);

    }

}

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

static qreal clamp01(qreal v) { return qBound<qreal>(0.0, v, 1.0); }
static qreal easeSin(qreal t) { return qSin(clamp01(t) * M_PI_2); }

// ─────────────────────────────────────────────────────────────────────────
// NEW: Clean percentage-card based stats dashboard
// ─────────────────────────────────────────────────────────────────────────
class EquipmentStatsCanvas final : public QWidget {
public:
    explicit EquipmentStatsCanvas(QWidget *parent = nullptr) : QWidget(parent) {
        setMouseTracking(true);
        m_frameTimer.setInterval(16);
        connect(&m_frameTimer, &QTimer::timeout, this, [this]() { m_globalTime += 0.016; update(); });
        m_frameTimer.start();
        m_bootMs = QDateTime::currentMSecsSinceEpoch();
    }
    void setData(const EquipmentStatsVisualData &data) { m_data = data; m_bootMs = QDateTime::currentMSecsSinceEpoch(); update(); }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        const QRectF canvas = rect();
        drawBg(p, canvas);
        const qreal anim = easeSin(elapsed() / 1.2);
        const int all = qMax(1, m_data.total + m_data.retired);

        // ── Title bar ──
        QRectF hdr(20, 16, canvas.width() - 40, 40);
        p.setPen(QColor(193, 127, 62, 200));
        p.setFont(QFont("Outfit", 12, QFont::Bold));
        p.drawText(hdr, Qt::AlignRight | Qt::AlignVCenter, "EQUIPMENT ANALYTICS");

        // ── Row 1: High-Density Metric Strips ──
        const qreal cw = (canvas.width() - 54) / 3.0;
        const qreal cy = 64;
        const qreal ch = 100;
        struct Metric { QString label; double pct; QColor col; QString val; };
        double availPct = 100.0 * m_data.available / all;
        double inUsePct = 100.0 * m_data.inUse / all;
        double maintPct = 100.0 * m_data.maintenance / all;

        QVector<Metric> metrics = {
            {"AVAILABLE", availPct, QColor("#4CAF7D"), QString::number(m_data.available)},
            {"IN USE",    inUsePct, QColor("#C17F3E"), QString::number(m_data.inUse)},
            {"MAINTENANCE", maintPct, QColor("#F59E0B"), QString::number(m_data.maintenance)}
        };
        for (int i = 0; i < 3; ++i) {
            QRectF card(20 + i * (cw + 7), cy, cw, ch);
            qreal cardAnim = easeSin(qBound(0.0, (elapsed() - 0.1 - i * 0.1) / 0.6, 1.0));
            p.save(); 
            p.translate(0, (1.0 - cardAnim) * 15);
            p.setOpacity(cardAnim);
            drawGlassCard(p, card, QColor(20, 16, 12, 235));
            
            // Layout: Title (Top Left), Count (Middle), Percentage (Right)
            p.setPen(QColor(193, 127, 62, 180));
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            p.drawText(card.adjusted(14, 10, -14, 0), Qt::AlignLeft | Qt::AlignTop, metrics[i].label);
            
            p.setPen(Qt::white);
            p.setFont(QFont("Outfit", 24, QFont::Black));
            p.drawText(card.adjusted(14, 30, -14, 0), Qt::AlignLeft | Qt::AlignTop, metrics[i].val);
            
            p.setPen(metrics[i].col);
            p.setFont(QFont("Outfit", 12, QFont::Bold));
            p.drawText(card.adjusted(14, 30, -14, -14), Qt::AlignRight | Qt::AlignBottom, 
                       QString::number((int)(metrics[i].pct * anim)) + "%");
            
            // Progress Bar
            QRectF bar(card.left() + 14, card.bottom() - 12, card.width() - 28, 4);
            p.setPen(Qt::NoPen); p.setBrush(QColor(60, 50, 40)); p.drawRect(bar);
            p.setBrush(metrics[i].col); p.drawRect(QRectF(bar.left(), bar.top(), bar.width() * (metrics[i].pct/100.0) * anim, 4));
            p.restore();
        }

        // ── Main Intelligence Matrix (Detailed Asset Ledger) ──
        qreal matrixY = cy + ch + 14;
        qreal footerH = 40;
        qreal matrixH = canvas.height() - matrixY - footerH - 24;
        QRectF matrixCard(20, matrixY, canvas.width() - 40, matrixH);
        
        qreal matA = easeSin(qBound(0.0, (elapsed() - 0.35) / 0.8, 1.0));
        p.save();
        p.translate(0, (1.0 - matA) * 25);
        p.setOpacity(matA);
        drawGlassCard(p, matrixCard, QColor(22, 19, 16, 252));
        
        p.setPen(QColor(193, 127, 62, 150));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(matrixCard.adjusted(16, 12, 0, 0), Qt::AlignLeft | Qt::AlignTop, "DETAILED ASSET PERFORMANCE LEDGER");

        // Data Ledger Columns
        qreal rowH = 34;
        int listSize = qMin(5, m_data.ranking.size());
        for (int i = 0; i < listSize; ++i) {
            const auto &item = m_data.ranking[i];
            qreal ry = matrixCard.top() + 44 + i * rowH;
            QRectF rowRect(matrixCard.left() + 24, ry, matrixCard.width() - 44, rowH - 4);
            
            p.setPen(QPen(QColor("#FFD700"), 1.5));
            p.drawLine(matrixCard.left() + 14, ry, matrixCard.left() + 14, ry + rowH - 6);
            p.setBrush(QColor("#FFD700"));
            p.drawEllipse(QPointF(matrixCard.left() + 14, ry + (rowH-6)/2.0), 3, 3);

            p.setPen(Qt::NoPen);
            p.setBrush(QColor(193, 127, 62, 15));
            p.drawRoundedRect(rowRect, 4, 4);
            
            p.setPen(Qt::white);
            p.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
            p.drawText(rowRect.adjusted(12, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, item.name);
            
            p.setPen(QColor("#FFD700"));
            p.setFont(QFont("Consolas", 9, QFont::Bold));
            p.drawText(rowRect.adjusted(0, 0, -12, 0), Qt::AlignRight | Qt::AlignVCenter, 
                       QString::number(item.unitPrice, 'f', 0) + " dt");
            
            qreal scaleW = 80 * anim;
            QRectF scaleRect(rowRect.right() - 200, rowRect.center().y() - 2, scaleW, 4);
            p.setPen(Qt::NoPen); p.setBrush(QColor(60, 50, 40)); p.drawRect(scaleRect);
            p.setBrush(QColor("#3B82F6")); 
            p.drawRect(QRectF(scaleRect.left(), scaleRect.top(), scaleRect.width() * (0.5 + 0.1 * qSin(m_globalTime*2.0 + i)), 4));
        }

        // ── Row 3: Expandable Mini-Pulse Metrics (Shiny & Interactive) ──
        qreal miniY = matrixCard.top() + 44 + listSize * rowH + 10;
        qreal miniW = (matrixCard.width() - 40) / 3.0;
        struct MiniStat { QString title; QColor col; QString detail; };
        QVector<MiniStat> miniStats = {
            {"SEC OPS", QColor("#F87171"), "FIREWALL: ACTIVE\nTHREATS: 0"},
            {"NET LINK", QColor("#3B82F6"), "LATENCY: 12ms\nBANDWIDTH: 1.2G"},
            {"CORE LOAD", QColor("#4CAF7D"), "CPU: 24%\nRAM: 4.8GB"}
        };

        m_miniRects.clear();
        for(int i=0; i<3; ++i) {
            // Animation handling for expansion
            qreal target = (m_expandedIndex == i) ? 1.0 : 0.0;
            m_expansionFactors[i] += (target - m_expansionFactors[i]) * 0.12;
            
            qreal currentH = 45 + m_expansionFactors[i] * 60;
            QRectF mCard(matrixCard.left() + 14 + i*(miniW+6), miniY, miniW, currentH);
            m_miniRects.append(mCard);
            
            // Hover Gold Effect
            QColor cardBg = (m_hoveredMini == i) ? QColor(60, 50, 20, 220) : QColor(35, 30, 25, 200);
            drawGlassCard(p, mCard, cardBg);
            if(m_hoveredMini == i) { // Shine
                p.setPen(QPen(QColor(255, 215, 0, 100), 1.5));
                p.drawRoundedRect(mCard, 14, 14);
            }
            
            // Pulse Animation
            qreal pPulse = (qSin(m_globalTime * 4.0 + i) + 1.0) * 0.5;
            p.setPen(Qt::NoPen); p.setBrush(QColor(miniStats[i].col.red(), miniStats[i].col.green(), miniStats[i].col.blue(), 40 + pPulse*100));
            p.drawEllipse(mCard.left() + 12, miniY + 22.5, 6 + pPulse*6, 6 + pPulse*6);
            p.setBrush(miniStats[i].col);
            p.drawEllipse(mCard.left() + 12, miniY + 22.5, 3, 3);
            
            p.setPen(QColor(200, 190, 180));
            p.setFont(QFont("Segoe UI", 7, QFont::Bold));
            p.drawText(QRectF(mCard.left() + 24, miniY, mCard.width()-24, 45), Qt::AlignLeft | Qt::AlignVCenter, miniStats[i].title);
            
            // Expanded Content
            if(m_expansionFactors[i] > 0.1) {
                p.save();
                p.setOpacity(m_expansionFactors[i]);
                p.setPen(QColor(200, 190, 180, 150));
                p.setFont(QFont("Consolas", 7));
                p.drawText(mCard.adjusted(12, 45, -12, -10), Qt::AlignLeft | Qt::AlignTop, miniStats[i].detail);
                
                // Close 'X'
                p.setPen(QColor(255, 100, 100, 200));
                p.setFont(QFont("Arial", 8, QFont::Bold));
                p.drawText(mCard.adjusted(0, 5, -10, 0), Qt::AlignRight | Qt::AlignTop, "×");
                p.restore();
            } else {
                // Closed State: Neural Sparks
                for(int s=0; s<3; ++s) {
                    qreal sparkX = mCard.left() + 40 + s*20 + qSin(m_globalTime*2.0 + s)*5;
                    p.setBrush(QColor(193, 127, 62, 50));
                    p.drawEllipse(QPointF(sparkX, mCard.bottom() - 10), 1, 1);
                }
            }
            
            // Shiny Decoration
            qreal sweepX = std::fmod(m_globalTime * 350.0, mCard.width() * 6.0) - mCard.width();
            if(sweepX > -20 && sweepX < mCard.width() + 20) {
                QLinearGradient sweep(mCard.left() + sweepX, 0, mCard.left() + sweepX + 40, 0);
                sweep.setColorAt(0, Qt::transparent); 
                sweep.setColorAt(0.5, (m_hoveredMini == i) ? QColor(255, 215, 0, 40) : QColor(255, 255, 255, 20)); 
                sweep.setColorAt(1, Qt::transparent);
                p.fillRect(mCard, sweep);
            }
        }
        p.restore();

        // ── Row 4: Tactical Node Activity & Fleet Vitality (New Animated Section) ──
        qreal nodeY = miniY + 55; // Base position below mini-stats
        qreal nodeH = matrixCard.bottom() - nodeY - 10;
        if (nodeH > 20) { // Only draw if there's enough space
            QRectF nodeArea(matrixCard.left() + 14, nodeY, matrixCard.width() - 28, nodeH);
            qreal nodeA = easeSin(qBound(0.0, (elapsed() - 0.7) / 0.6, 1.0));
            p.save();
            p.setOpacity(nodeA * (1.0 - m_expansionFactors[0]*0.5 - m_expansionFactors[1]*0.5 - m_expansionFactors[2]*0.5)); // Fade slightly on expansion
            
            p.setPen(QPen(QColor(193, 127, 62, 40), 1, Qt::DashLine));
            p.drawLine(nodeArea.left(), nodeArea.top() + 5, nodeArea.right(), nodeArea.top() + 5);

            // Fleet Vitality Scan (Wavy "Heartbeat" Line)
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath wave;
            wave.moveTo(nodeArea.left(), nodeArea.center().y());
            for(int x=0; x<nodeArea.width(); x += 5) {
                qreal phase = (x * 0.05) - (m_globalTime * 3.0);
                qreal y = nodeArea.center().y() + qSin(phase) * 8.0 * qCos(phase * 0.5);
                wave.lineTo(nodeArea.left() + x, y);
            }
            p.setPen(QPen(QColor(59, 130, 246, 80), 1.5));
            p.drawPath(wave);

            // Glowing Status Nodes
            for(int i=0; i<8; ++i) {
                qreal nx = nodeArea.left() + 20 + i * (nodeArea.width()/8.0);
                qreal ny = nodeArea.center().y();
                qreal nPulse = (qSin(m_globalTime * 2.0 + i) + 1.0) * 0.5;
                
                // Outer Glow
                p.setBrush(QColor(193, 127, 62, 20 * nPulse));
                p.setPen(Qt::NoPen);
                p.drawEllipse(QPointF(nx, ny), 6, 6);
                
                // Core
                p.setBrush(nPulse > 0.8 ? QColor("#FFD700") : QColor(193, 127, 62, 150));
                p.drawEllipse(QPointF(nx, ny), 2, 2);
                
                // Holographic ID (Mini shiny text - dynamic hex)
                p.setPen(QColor(255, 215, 0, 100 + nPulse * 155));
                p.setFont(QFont("Consolas", 6, QFont::Bold));
                QString hexId = QString("0x%1%2").arg(i+1).arg((int)qAbs(qSin(m_globalTime*5.0 + i)*255), 2, 16, QChar('0')).toUpper();
                p.drawText(QRectF(nx - 20, ny - 15, 40, 10), Qt::AlignCenter, hexId);

                if (nPulse > 0.95) { // Occasional "Spark"
                    p.setPen(QColor(255, 215, 0, 150));
                    p.drawLine(nx-4, ny-4, nx+4, ny+4);
                    p.drawLine(nx+4, ny-4, nx-4, ny+4);
                }
            }

            p.setPen(QColor(193, 127, 62, 100));
            p.setFont(QFont("Segoe UI", 6, QFont::Bold));
            p.drawText(nodeArea.adjusted(0, 0, 0, -2), Qt::AlignLeft | Qt::AlignBottom, "LIVE NODE TELEMETRY");
            p.drawText(nodeArea.adjusted(0, 0, 0, -2), Qt::AlignRight | Qt::AlignBottom, "VITALITY: NOMINAL");

            // Regional Hub Capacity (Small Vertical Bars)
            for(int i=0; i<12; ++i) {
                qreal bx = nodeArea.right() - 120 + i*8;
                qreal bh = 10 + qAbs(qSin(m_globalTime*3.0 + i*0.5)) * 15;
                QRectF bar(bx, nodeArea.bottom() - bh - 15, 4, bh);
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(193, 127, 62, 30));
                p.drawRect(QRectF(bx, nodeArea.bottom()-40, 4, 25)); // Background track
                p.setBrush(QColor("#3B82F6"));
                p.drawRect(bar);
                // Glowing Top
                p.setBrush(QColor(255, 215, 0, 150));
                p.drawRect(QRectF(bx, bar.top(), 4, 2));
            }
            p.restore();

            // Global Holographic Sweep (Occasional)
            qreal globalSweepX = std::fmod(m_globalTime * 200.0, matrixCard.width() * 8.0) - matrixCard.width();
            if(globalSweepX > -100 && globalSweepX < matrixCard.width() + 100) {
                QLinearGradient g(matrixCard.left() + globalSweepX, 0, matrixCard.left() + globalSweepX + 60, 0);
                g.setColorAt(0, Qt::transparent);
                g.setColorAt(0.5, QColor(193, 127, 62, 8));
                g.setColorAt(1, Qt::transparent);
                p.fillRect(matrixCard, g);
            }
        }

        // ── Compact Footer (Bottom Right Only) ──
        qreal footW = 160;
        QRectF footer(canvas.width() - footW - 20, canvas.height() - footerH - 12, footW, footerH);
        qreal footA = easeSin(qBound(0.0, (elapsed() - 0.85) / 0.6, 1.0));
        p.save();
        p.setOpacity(footA);
        drawGlassCard(p, footer, QColor(18, 15, 12, 255));
        
        QFont fLabel("Segoe UI", 7, QFont::Bold);
        fLabel.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
        p.setFont(fLabel);
        p.setPen(QColor(193, 127, 62, 180));
        p.drawText(footer.adjusted(14, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, "TOTAL ASSETS:");
        
        p.setPen(Qt::white);
        p.setFont(QFont("Outfit", 11, QFont::Black));
        p.drawText(footer.adjusted(0, 0, -14, 0), Qt::AlignRight | Qt::AlignVCenter, QString::number(all));
        p.restore();
    }

private:
    qreal elapsed() const { return (QDateTime::currentMSecsSinceEpoch() - m_bootMs) / 1000.0; }

    void drawBg(QPainter &p, const QRectF &r) {
        p.fillRect(r, QColor("#0A0806"));
        
        // High-Tech Grid
        p.setPen(QPen(QColor(193, 127, 62, 15), 1));
        int spacing = 40;
        for (int x = 0; x <= r.width(); x += spacing) p.drawLine(x, 0, x, r.height());
        for (int y = 0; y <= r.height(); y += spacing) p.drawLine(0, y, r.width(), y);

        // Animated Scanline
        qreal scanY = std::fmod(m_globalTime * 80.0, r.height());
        QLinearGradient sg(0, scanY, 0, scanY + 60);
        sg.setColorAt(0, Qt::transparent);
        sg.setColorAt(0.5, QColor(193, 127, 62, 10));
        sg.setColorAt(1, Qt::transparent);
        p.fillRect(QRectF(0, scanY, r.width(), 60), sg);

        // Dynamic Floating Particles
        for (int i = 0; i < 50; ++i) {
            qreal seed = i * 137.5;
            qreal speed = 10.0 + (i % 7) * 4.0;
            qreal t = m_globalTime * speed * 0.1;
            qreal x = std::fmod(seed + t * 50, r.width());
            qreal y = std::fmod(seed * 0.8 + t * 30, r.height());
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(193, 127, 62, 15 + (i % 20)));
            p.drawEllipse(QPointF(x, y), 1.0, 1.0);
        }

        // Vignette
        QRadialGradient vig(r.center(), r.width() * 0.8);
        vig.setColorAt(0, Qt::transparent);
        vig.setColorAt(1, QColor(0, 0, 0, 180));
        p.fillRect(r, vig);
    }

    void mousePressEvent(QMouseEvent *e) override {
        for(int i=0; i<m_miniRects.size(); ++i) {
            if(m_miniRects[i].contains(e->pos())) {
                if(m_expandedIndex == i) m_expandedIndex = -1;
                else m_expandedIndex = i;
                update();
                return;
            }
        }
        m_expandedIndex = -1;
        update();
    }

    void mouseMoveEvent(QMouseEvent *e) override {
        int oldHover = m_hoveredMini;
        m_hoveredMini = -1;
        for(int i=0; i<m_miniRects.size(); ++i) {
            if(m_miniRects[i].contains(e->pos())) {
                m_hoveredMini = i;
                break;
            }
        }
        if(oldHover != m_hoveredMini) update();
    }

    void leaveEvent(QEvent *) override {
        m_hoveredMini = -1;
        update();
    }

    void drawGlassCard(QPainter &p, const QRectF &r, const QColor &bg) {
        // Shadow
        p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, 100));
        p.drawRoundedRect(r.translated(3, 4), 14, 14);
        // Card
        QLinearGradient g(r.topLeft(), r.bottomRight());
        g.setColorAt(0, bg); g.setColorAt(1, bg.darker(120));
        p.setBrush(g); p.setPen(QPen(QColor(193, 127, 62, 50), 1));
        p.drawRoundedRect(r, 14, 14);
        // Top accent
        QLinearGradient ag(r.left() + 10, 0, r.right() - 10, 0);
        ag.setColorAt(0, Qt::transparent); ag.setColorAt(0.5, QColor(193, 127, 62, 80)); ag.setColorAt(1, Qt::transparent);
        p.fillRect(QRectF(r.left() + 10, r.top() + 2, r.width() - 20, 2), ag);
    }

    EquipmentStatsVisualData m_data;
    QTimer m_frameTimer;
    qreal m_globalTime = 0.0;
    qint64 m_bootMs = 0;

    int m_expandedIndex = -1;
    qreal m_expansionFactors[3] = {0,0,0};
    int m_hoveredMini = -1;
    QVector<QRectF> m_miniRects;

    QVector<QRectF> m_panelRects;
    int m_hoveredPanel = -1;
    int m_hoveredSegment = -1;

    QPointF m_donutCenter;
    qreal m_donutInner = 0.0;
    qreal m_donutOuter = 0.0;
    QVector<QPair<qreal, qreal>> m_segmentRanges;
};

}

void MainWindow::on_gs_equipment_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
    ui_equipment->tabWidget->setCurrentIndex(0);
    onEquipmentRefreshView();
    onEquipmentHistorySearch();
}

void MainWindow::on_nav_equipments_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
    ui_equipment->tabWidget->setCurrentIndex(0);
    onEquipmentRefreshView();
    onEquipmentHistorySearch();
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
    m_eqTypeInd = makeInd("[ÃƒÂ°Ã…Â¸Ã¢â‚¬ÂÃ‚Â¨ Type ÃƒÂ¢Ã‚Â¬Ã…â€œ]", "ind_type"); 
    m_eqDateInd = makeInd("[ÃƒÂ°Ã…Â¸Ã¢â‚¬Å“Ã¢â‚¬Â¦ Date ÃƒÂ¢Ã‚Â¬Ã…â€œ]", "ind_date"); 
    m_eqPriceInd= makeInd("[ÃƒÂ°Ã…Â¸Ã¢â‚¬â„¢Ã‚Â° Price ÃƒÂ¢Ã‚Â¬Ã…â€œ]", "ind_price"); 
    m_eqDescInd = makeInd("[ÃƒÂ°Ã…Â¸Ã¢â‚¬Å“Ã‚Â Desc ÃƒÂ¢Ã‚Â¬Ã…â€œ]", "ind_desc");

    ui_equipment->label_type->setText(QString::fromUtf8("\xF0\x9F\x94\xA7 Type:"));
    ui_equipment->label_date_achat->setText(QString::fromUtf8("\xF0\x9F\x93\x85 Purchase Date:"));
    ui_equipment->label_unit_price->setText(QString::fromUtf8("\xF0\x9F\x92\xB0 Unit Price (dt):"));
    ui_equipment->label_etat->setText(QString::fromUtf8("\xF0\x9F\x93\x8A Status:"));
    ui_equipment->label_quantity->setText(QString::fromUtf8("\xF0\x9F\x93\xA6 Quantity:"));
    ui_equipment->label_desc->setText(QString::fromUtf8("\xF0\x9F\x93\x9D Description:"));

    // --- APPLY FORGE DESIGN SYSTEM (FINAL BOSS PROMPT) ---
    ui_equipment->groupBox_gestion->setStyleSheet(
        "QGroupBox#groupBox_gestion { background-color: rgba(60, 45, 30, 0.7); border: 2px solid #8B6F47; border-radius: 12px; margin-top: 18px; color: white; }"
        "QGroupBox#groupBox_gestion::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 2px 12px; background-color: #8B6F47; font-weight: bold; color: white; border-radius: 4px; }"
    );

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
    connect(ui_equipment->btn_refresh_history, &QPushButton::clicked,
            this, &MainWindow::onEquipmentHistoryRefresh, Qt::UniqueConnection);
    connect(ui_equipment->btn_history_search, &QPushButton::clicked,
            this, &MainWindow::onEquipmentHistorySearch, Qt::UniqueConnection);
    connect(ui_equipment->le_history_search, &QLineEdit::returnPressed,
            this, &MainWindow::onEquipmentHistorySearch, Qt::UniqueConnection);
    connect(ui_equipment->btn_export_history, &QPushButton::clicked,
            this, &MainWindow::onEquipmentExportPDF, Qt::UniqueConnection);
    connect(ui_equipment->btn_clear_history, &QPushButton::clicked,
            this, &MainWindow::onEquipmentHistoryClear, Qt::UniqueConnection);
    connect(ui_equipment->btn_export_stats,    &QPushButton::clicked, this, &MainWindow::onEquipmentExportStatsPDF);
    connect(ui_equipment->btn_bulk_update_status, &QPushButton::clicked, this, &MainWindow::onEquipmentBulkUpdateStatus);
    connect(ui_equipment->btn_bulk_delete_all, &QPushButton::clicked, this, &MainWindow::onEquipmentDeleteAll);

    // Handle module tab switches.
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        QWidget *selected = ui_equipment->tabWidget->widget(idx);
        
        // Stop chat timer by default unless on chat tab
        chatRefreshTimer->stop();
        
        if (selected == ui_equipment->tab_view) {
            onEquipmentRefreshView();
        } else if (selected == ui_equipment->tab_history) {
            onEquipmentHistoryRefresh();
        } else if (selected == ui_equipment->tab_stats) {
            setupEquipmentStats();
        } else if (selected == ui_equipment->tab_chat) {
            ui_equipment->list_employees->setItemDelegate(new ChatEmployeeDelegate(this));
            ui_equipment->list_employees->setSpacing(2);
            onChatRefresh();
            onChatEmployeeListRefresh();
            chatRefreshTimer->start(3000); // 3 seconds refresh
        }
    });

    // --- ContrÃƒÆ’Ã‚Â´le de Saisie (Input Validation) ---
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

    // --- Creative Suite Initialization ---
    m_radialMenu = new RadialCommandMenu(this);
    connect(m_radialMenu, &RadialCommandMenu::actionSelected, this, &MainWindow::onRadialAction);

    m_identityCard = new IdentityCard(ui_equipment->tab_view);
    m_identityCard->hide();
}




void MainWindow::onEquipmentRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT 'Edit' AS \"Action\", 'Delete' AS \"Delete\", EQUIPMENT_ID AS \"ID\", "
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

    QJsonObject equipMeta;
    equipMeta["equipment_operation"] = "ADD";
    equipMeta["equipment_id"] = eqId;
    equipMeta["equipment_type"] = type;
    equipMeta["equipment_description"] = desc;
    equipMeta["equipment_status"] = etat;
    equipMeta["equipment_price"] = price;
    logActivity("Added new equipment: " + type + " (ID: " + QString::number(eqId) + ")", "Equipment", equipMeta);
    
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    updateEquipProgress();
    // Refresh NEXUS analysis
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onRadialAction(const QString &action, int id) {
    // Find row index for this ID
    QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
    int row = -1;
    for (int i = 0; i < m->rowCount(); ++i) {
        if (m->data(m->index(i, 2)).toInt() == id) {
            row = i;
            break;
        }
    }
    if (row == -1) return;

    if (action == "EDIT") {
        ui_equipment->table_equipments->clicked(m->index(row, 0));
    } else if (action == "DELETE") {
        ui_equipment->table_equipments->setCurrentIndex(m->index(row, 1));
        onEquipmentDelete();
    } else if (action == "STATS") {
        ui_equipment->tabWidget->setCurrentWidget(ui_equipment->tab_stats);
    }
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

    QJsonObject equipMeta;
    equipMeta["equipment_operation"] = "MODIFY";
    equipMeta["equipment_id"] = id.toInt();
    equipMeta["equipment_type"] = finalType;
    equipMeta["equipment_description"] = finalDesc;
    equipMeta["equipment_status"] = finalStatus;
    equipMeta["equipment_price"] = finalPrice;
    logActivity("Modified equipment: " + finalType + " (ID: " + id + ")", "Equipment", equipMeta);
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
    QString status;
    double unitPrice = 0.0;

    QModelIndex idx = ui_equipment->table_equipments->currentIndex();
    if (idx.isValid()) {
        QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
        if (!model) return;
        eqId = model->data(model->index(idx.row(), 2)).toString();
        type = model->data(model->index(idx.row(), 3)).toString();
        unitPrice = model->data(model->index(idx.row(), 5)).toDouble();
        status = model->data(model->index(idx.row(), 6)).toString();
        desc = model->data(model->index(idx.row(), 8)).toString();
    } else {
        eqId = ui_equipment->le_id->text().trimmed();
        type = ui_equipment->le_type->text().trimmed();
        desc = ui_equipment->te_desc->toPlainText().trimmed();
        status = ui_equipment->cb_status->currentText();
        unitPrice = ui_equipment->dsb_unit_price->value();
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
    QString deleteLabel = type;
    if (!desc.isEmpty()) {
        deleteLabel += " - " + desc;
    }

    QJsonObject equipMeta;
    equipMeta["equipment_operation"] = "DELETE";
    equipMeta["equipment_id"] = eqId.toInt();
    equipMeta["equipment_type"] = type;
    equipMeta["equipment_description"] = desc;
    equipMeta["equipment_status"] = status;
    equipMeta["equipment_price"] = unitPrice;
    logActivity("Deleted equipment: " + deleteLabel + " (ID: " + eqId + ")", "Equipment", equipMeta);

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

    QString sql = "SELECT 'Edit' AS \"Action\", 'Delete' AS \"Delete\", EQUIPMENT_ID AS \"ID\", "
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

void MainWindow::onEquipmentHistoryRefresh()
{
    onEquipmentHistorySearch();
}

void MainWindow::onEquipmentHistorySearch()
{
    if (!ui_equipment) {
        return;
    }

    const QString search = ui_equipment->le_history_search->text().trimmed();
    const QString searchUpper = search.toUpper();

    auto readHistoryState = []() -> QJsonObject {
        QFile f("hammerdown_history_state.json");
        if (!f.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }
        const QJsonDocument d = QJsonDocument::fromJson(f.readAll());
        f.close();
        return d.isObject() ? d.object() : QJsonObject();
    };

    auto writeHistoryState = [](const QJsonObject &obj) {
        QFile f("hammerdown_history_state.json");
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
            f.close();
        }
    };

    auto getString = [](const QJsonObject &obj, const QString &a, const QString &b = QString()) -> QString {
        QString v = obj.value(a).toString().trimmed();
        if (v.isEmpty() && !b.isEmpty()) {
            v = obj.value(b).toString().trimmed();
        }
        return v;
    };

    auto getInt = [](const QJsonObject &obj, const QString &a, const QString &b = QString()) -> int {
        int v = obj.value(a).toVariant().toInt();
        if (v <= 0 && !b.isEmpty()) {
            v = obj.value(b).toVariant().toInt();
        }
        return v;
    };

    QJsonObject historyState = readHistoryState();
    int addedMinId = qMax(0, historyState.value("equipment_added_min_id").toInt(0));

    const QJsonArray previousSnapshot = historyState.value("equipment_snapshot").toArray();
    QJsonObject previousById;
    for (const QJsonValue &v : previousSnapshot) {
        if (!v.isObject()) {
            continue;
        }
        const QJsonObject obj = v.toObject();
        const int id = getInt(obj, "id", "equipment_id");
        if (id > 0) {
            previousById[QString::number(id)] = obj;
        }
    }

    QJsonArray modifyHistory = historyState.value("equipment_modify_history").toArray();
    QJsonArray deleteHistory = historyState.value("equipment_delete_history").toArray();

    QJsonArray currentRows;
    QJsonObject currentById;
    int currentMaxId = 0;

    QSqlQuery q;
    const bool queryOk = q.exec(
        "SELECT "
        "EQUIPMENT_ID, "
        "NVL(EQUIPMENT_TYPE, '-') AS EQUIPMENT_TYPE, "
        "NVL(DESCRIPTION, '-') AS DESCRIPTION, "
        "NVL(STATUS, '-') AS STATUS, "
        "NVL(TO_CHAR(UNIT_PRICE, 'FM9999999990.00'), '-') AS PRICE, "
        "NVL(TO_CHAR(PURCHASE_DATE, 'YYYY-MM-DD'), '-') AS PURCHASE_DATE "
        "FROM EQUIPMENT "
        "ORDER BY EQUIPMENT_ID DESC");

    if (queryOk) {
        while (q.next()) {
            const int id = q.value(0).toInt();
            if (id <= 0) {
                continue;
            }

            QJsonObject rowObj;
            rowObj["id"] = id;
            rowObj["type"] = q.value(1).toString();
            rowObj["description"] = q.value(2).toString();
            rowObj["status"] = q.value(3).toString();
            rowObj["price"] = q.value(4).toString();
            rowObj["date"] = q.value(5).toString();

            currentRows.append(rowObj);
            currentById[QString::number(id)] = rowObj;
            if (id > currentMaxId) {
                currentMaxId = id;
            }
        }
    } else {
        qDebug() << "Failed to load equipment rows for history:" << q.lastError().text();
    }

    if (currentMaxId < addedMinId) {
        addedMinId = 0;
        historyState["equipment_added_min_id"] = 0;
    }

    if (queryOk) {
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        const QString nowDate = QDate::currentDate().toString("yyyy-MM-dd");

        const QStringList currentKeys = currentById.keys();
        for (const QString &idKey : currentKeys) {
            if (!previousById.contains(idKey)) {
                continue;
            }

            const QJsonObject cur = currentById.value(idKey).toObject();
            const QJsonObject old = previousById.value(idKey).toObject();

            const bool changed =
                getString(cur, "type") != getString(old, "type") ||
                getString(cur, "description") != getString(old, "description") ||
                getString(cur, "status") != getString(old, "status") ||
                getString(cur, "price") != getString(old, "price") ||
                getString(cur, "date") != getString(old, "date");

            if (!changed) {
                continue;
            }

            QJsonObject ev = cur;
            ev["status"] = "Modified";
            ev["timestamp_ms"] = QString::number(nowMs);
            ev["date"] = nowDate;
            modifyHistory.append(ev);
            while (modifyHistory.size() > 800) {
                modifyHistory.removeAt(0);
            }
        }

        const QStringList previousKeys = previousById.keys();
        for (const QString &idKey : previousKeys) {
            if (currentById.contains(idKey)) {
                continue;
            }

            const QJsonObject old = previousById.value(idKey).toObject();
            QJsonObject ev = old;
            ev["status"] = "Deleted";
            ev["timestamp_ms"] = QString::number(nowMs);
            ev["date"] = nowDate;
            deleteHistory.append(ev);
            while (deleteHistory.size() > 800) {
                deleteHistory.removeAt(0);
            }
        }

        historyState["equipment_snapshot"] = currentRows;
        historyState["equipment_modify_history"] = modifyHistory;
        historyState["equipment_delete_history"] = deleteHistory;
        writeHistoryState(historyState);
    }

    auto buildHaystack = [&](const QJsonObject &obj, const QString &fallbackStatus) {
        const int idNum = getInt(obj, "id", "equipment_id");
        const QString id = (idNum > 0) ? QString::number(idNum) : QString("-");
        const QString type = getString(obj, "type", "equipment_type");
        const QString description = getString(obj, "description", "equipment_description");
        QString status = getString(obj, "status", "equipment_status");
        if (status.isEmpty()) {
            status = fallbackStatus;
        }
        const QString price = getString(obj, "price", "equipment_price");
        QString dateText = getString(obj, "date");
        if (dateText.isEmpty()) {
            QDateTime dt = QDateTime::fromMSecsSinceEpoch(getString(obj, "timestamp_ms").toLongLong());
            if (dt.isValid()) {
                dateText = dt.date().toString("yyyy-MM-dd");
            }
        }
        return (id + " " + type + " " + description + " " + status + " " + price + " " + dateText).toUpper();
    };

    auto applyTimelineView = [&](QTableView *view) {
        if (!view) {
            return;
        }
        const int baseHeight = 90;
        const int expandedHeight = 150;

        if (!dynamic_cast<TimelineCardDelegate*>(view->itemDelegate())) {
            view->setItemDelegate(new TimelineCardDelegate(view));
        }
        if (!view->property("timelinePulse").isValid()) {
            view->setProperty("timelinePulse", 0.0);
            view->setProperty("timelineExpand", 0.0);
            view->setProperty("timelineScan", 0.0);
            view->setProperty("timelineSelectedRow", -1);
            view->setProperty("timelineSelectedKey", QString());
        }

        auto *pulseAnim = view->findChild<QPropertyAnimation*>("timelinePulseAnim");
        if (!pulseAnim) {
            pulseAnim = new QPropertyAnimation(view, "timelinePulse", view);
            pulseAnim->setObjectName("timelinePulseAnim");
            pulseAnim->setDuration(520);
            pulseAnim->setKeyValueAt(0.0, 0.0);
            pulseAnim->setKeyValueAt(0.5, 1.0);
            pulseAnim->setKeyValueAt(1.0, 0.0);
            pulseAnim->setEasingCurve(QEasingCurve::OutCubic);
            QObject::connect(pulseAnim, &QPropertyAnimation::valueChanged, view, [view]() {
                view->viewport()->update();
            });
        }

        auto *expandAnim = view->findChild<QPropertyAnimation*>("timelineExpandAnim");
        if (!expandAnim) {
            expandAnim = new QPropertyAnimation(view, "timelineExpand", view);
            expandAnim->setObjectName("timelineExpandAnim");
            expandAnim->setDuration(260);
            expandAnim->setStartValue(0.0);
            expandAnim->setEndValue(1.0);
            expandAnim->setEasingCurve(QEasingCurve::OutCubic);
            QObject::connect(expandAnim, &QPropertyAnimation::valueChanged, view, [view, baseHeight, expandedHeight]() {
                const int row = view->property("timelineSelectedRow").toInt();
                if (row < 0 || !view->model()) {
                    return;
                }
                double factor = view->property("timelineExpand").toDouble();
                if (factor < 0.0) factor = 0.0;
                if (factor > 1.0) factor = 1.0;
                const int height = baseHeight + static_cast<int>((expandedHeight - baseHeight) * factor);
                view->setRowHeight(row, height);
                view->viewport()->update();
            });
        }

        auto *scanAnim = view->findChild<QPropertyAnimation*>("timelineScanAnim");
        if (!scanAnim) {
            scanAnim = new QPropertyAnimation(view, "timelineScan", view);
            scanAnim->setObjectName("timelineScanAnim");
            scanAnim->setDuration(900);
            scanAnim->setStartValue(0.0);
            scanAnim->setEndValue(1.0);
            scanAnim->setEasingCurve(QEasingCurve::OutCubic);
            scanAnim->setLoopCount(1);
            QObject::connect(scanAnim, &QPropertyAnimation::valueChanged, view, [view]() {
                view->viewport()->update();
            });
        }

        view->setShowGrid(false);
        view->setAlternatingRowColors(false);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->verticalHeader()->setVisible(false);
        view->horizontalHeader()->setVisible(false);
        view->setStyleSheet("QTableView { background: transparent; border: none; }");
        view->verticalHeader()->setDefaultSectionSize(baseHeight);

        if (view->model()) {
            for (int r = 0; r < view->model()->rowCount(); ++r) {
                view->setRowHeight(r, baseHeight);
            }
        }

        if (view->selectionModel() && view->model()) {
            const QString key = view->property("timelineSelectedKey").toString();
            if (!key.isEmpty()) {
                const QStringList parts = key.split('|');
                const QString wantedId = parts.value(0);
                const QString wantedEvent = parts.value(1);
                const int rowCount = view->model()->rowCount();
                for (int r = 0; r < rowCount; ++r) {
                    const QString rowId = view->model()->index(r, 0).data().toString();
                    const QString rowEvent = view->model()->index(r, 6).data().toString();
                    if (rowId == wantedId && rowEvent == wantedEvent) {
                        QSignalBlocker blocker(view->selectionModel());
                        view->selectionModel()->setCurrentIndex(view->model()->index(r, 0),
                                                                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                        view->setProperty("timelineSelectedRow", r);
                        view->setProperty("timelineExpand", 1.0);
                        view->setRowHeight(r, expandedHeight);
                        view->viewport()->update();
                        break;
                    }
                }
            }
        }

        if (view->selectionModel()) {
            const quintptr selPtr = reinterpret_cast<quintptr>(view->selectionModel());
            if (view->property("timelineSelModel").toULongLong() != selPtr) {
                view->setProperty("timelineSelModel", QVariant::fromValue<qulonglong>(selPtr));
                QObject::connect(view->selectionModel(), &QItemSelectionModel::currentRowChanged,
                                 view, [view, pulseAnim, expandAnim, scanAnim, baseHeight](const QModelIndex &current, const QModelIndex &) {
                    const int prevRow = view->property("timelineSelectedRow").toInt();
                    if (prevRow >= 0) {
                        view->setRowHeight(prevRow, baseHeight);
                    }

                    if (!current.isValid()) {
                        view->setProperty("timelineSelectedRow", -1);
                        view->setProperty("timelineExpand", 0.0);
                        view->setProperty("timelineScan", 0.0);
                        view->setProperty("timelineSelectedKey", QString());
                        view->viewport()->update();
                        return;
                    }

                    const QString selId = current.model()->index(current.row(), 0).data().toString();
                    const QString selEvent = current.model()->index(current.row(), 6).data().toString();
                    view->setProperty("timelineSelectedKey", selId + "|" + selEvent);

                    view->setProperty("timelineSelectedRow", current.row());
                    view->setProperty("timelineExpand", 0.0);
                    view->setProperty("timelineScan", 0.0);
                    view->setRowHeight(current.row(), baseHeight);

                    expandAnim->stop();
                    expandAnim->start();
                    pulseAnim->stop();
                    pulseAnim->start();
                    scanAnim->stop();
                    scanAnim->start();
                });
            }
        }
    };

    auto renderRows = [&](QTableView *view, const QJsonArray &rows, const QString &fallbackStatus, int minIdExclusive, bool iterateReversed) {
        if (!view) {
            return;
        }

        QStandardItemModel *model = new QStandardItemModel(this);
        model->setHorizontalHeaderLabels({"ID", "Type", "Description", "Status", "Price", "Date", "Event"});

        constexpr int kMaxRowsPerSection = 300;
        int appended = 0;

        auto appendRowFromObject = [&](const QJsonObject &obj) {
            if (appended >= kMaxRowsPerSection) {
                return;
            }

            const int idNum = getInt(obj, "id", "equipment_id");
            if (minIdExclusive >= 0 && idNum > 0 && idNum <= minIdExclusive) {
                return;
            }

            if (!searchUpper.isEmpty()) {
                const QString haystack = buildHaystack(obj, fallbackStatus);
                if (!haystack.contains(searchUpper)) {
                    return;
                }
            }

            const QString id = (idNum > 0) ? QString::number(idNum) : QString("-");
            QString type = getString(obj, "type", "equipment_type");
            QString description = getString(obj, "description", "equipment_description");
            QString status = getString(obj, "status", "equipment_status");
            QString price = getString(obj, "price", "equipment_price");
            QString dateText = getString(obj, "date");
            const QString eventType = fallbackStatus;

            if (status.isEmpty()) status = fallbackStatus;
            if (type.isEmpty()) type = "-";
            if (description.isEmpty()) description = "-";
            if (price.isEmpty()) price = "-";

            if (dateText.isEmpty()) {
                QDateTime dt = QDateTime::fromMSecsSinceEpoch(getString(obj, "timestamp_ms").toLongLong());
                if (dt.isValid()) {
                    dateText = dt.date().toString("yyyy-MM-dd");
                }
            }
            if (dateText.isEmpty()) dateText = "-";

            QList<QStandardItem*> row;
            row << new QStandardItem(id)
                << new QStandardItem(type)
                << new QStandardItem(description)
                << new QStandardItem(status)
                << new QStandardItem(price)
                << new QStandardItem(dateText)
                << new QStandardItem(eventType);
            model->appendRow(row);
            ++appended;
        };

        if (iterateReversed) {
            for (int i = rows.size() - 1; i >= 0 && appended < kMaxRowsPerSection; --i) {
                if (!rows.at(i).isObject()) {
                    continue;
                }
                appendRowFromObject(rows.at(i).toObject());
            }
        } else {
            for (int i = 0; i < rows.size() && appended < kMaxRowsPerSection; ++i) {
                if (!rows.at(i).isObject()) {
                    continue;
                }
                appendRowFromObject(rows.at(i).toObject());
            }
        }

        view->setModel(model);
        view->setSortingEnabled(true);
        if (model->rowCount() > 0) {
            view->sortByColumn(5, Qt::DescendingOrder);
        }

        applyTimelineView(view);
        view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        for (int c = 1; c < model->columnCount(); ++c) {
            view->setColumnHidden(c, true);
        }
    };

    renderRows(ui_equipment->tableView_history_add, currentRows, "Added", addedMinId, false);
    renderRows(ui_equipment->tableView_history_modify, modifyHistory, "Modified", -1, true);
    renderRows(ui_equipment->tableView_historique, deleteHistory, "Deleted", -1, true);

}

void MainWindow::onEquipmentHistoryClear()
{
    if (!ui_equipment) {
        return;
    }

    QTableView *targetView = ui_equipment->tableView_history_add;
    QString sectionLabel = "Added";
    QString operation = "ADD";

    if (ui_equipment->tabWidget_history_sections) {
        const int currentIdx = ui_equipment->tabWidget_history_sections->currentIndex();
        if (currentIdx == ui_equipment->tabWidget_history_sections->indexOf(ui_equipment->tab_history_modify)) {
            targetView = ui_equipment->tableView_history_modify;
            sectionLabel = "Modified";
            operation = "MODIFY";
        } else if (currentIdx == ui_equipment->tabWidget_history_sections->indexOf(ui_equipment->tab_history_delete)) {
            targetView = ui_equipment->tableView_historique;
            sectionLabel = "Deleted";
            operation = "DELETE";
        }
    }

    if (!targetView) {
        return;
    }

    auto readHistoryState = []() -> QJsonObject {
        QFile f("hammerdown_history_state.json");
        if (!f.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }
        const QJsonDocument d = QJsonDocument::fromJson(f.readAll());
        f.close();
        return d.isObject() ? d.object() : QJsonObject();
    };

    auto writeHistoryState = [](const QJsonObject &obj) {
        QFile f("hammerdown_history_state.json");
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
            f.close();
        }
    };

    int addedCutoffId = -1;
    if (operation == "ADD") {
        QSqlQuery qMax;
        if (qMax.exec("SELECT NVL(MAX(EQUIPMENT_ID), 0) FROM EQUIPMENT") && qMax.next()) {
            addedCutoffId = qMax.value(0).toInt();
        } else {
            addedCutoffId = 0;
        }

        QJsonObject state = readHistoryState();
        state["equipment_added_min_id"] = addedCutoffId;
        writeHistoryState(state);
    }

    int removedRows = 0;
    if (operation == "MODIFY" || operation == "DELETE") {
        QJsonObject state = readHistoryState();
        const QString key = (operation == "MODIFY") ? "equipment_modify_history" : "equipment_delete_history";
        const QJsonArray oldRows = state.value(key).toArray();
        removedRows = oldRows.size();
        state[key] = QJsonArray();
        writeHistoryState(state);
    }

    onEquipmentHistorySearch();

    if (operation == "ADD") {
        QMessageBox::information(this,
                                 "Clear Logs",
                                 QString("Cleared %1 history view. Existing equipment up to ID %2 is hidden; only newly inserted equipment will appear after refresh.")
                                     .arg(sectionLabel, QString::number(addedCutoffId)));
    } else {
        QMessageBox::information(this,
                                 "Clear Logs",
                                 QString("Cleared %1 logs. Removed rows: %2.")
                                     .arg(sectionLabel, QString::number(removedRows)));
    }
}

void MainWindow::onEquipmentCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = ui_equipment->table_equipments->indexAt(pos);
    if (!index.isValid()) return;

    // Get the ID (Column 2 according to onEquipmentRefreshView: Action, Delete, ID...)
    int equipId = ui_equipment->table_equipments->model()->data(ui_equipment->table_equipments->model()->index(index.row(), 2)).toInt();

    QMenu menu(this);
        QAction *analyzeAct = menu.addAction("Analyze in Nexus");
    
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

    // Date is stored in visible column 5 (with optional hidden metadata columns after it).
    int dateCol = (view->model()->columnCount() > 5) ? 5 : (view->model()->columnCount() - 1);
    QDate date = QDate::fromString(view->model()->data(view->model()->index(index.row(), dateCol)).toString(), "yyyy-MM-dd");

    if (!date.isValid() && view->model()->columnCount() > 6) {
        const QString ts = view->model()->data(view->model()->index(index.row(), 6)).toString();
        const QDateTime dt = QDateTime::fromString(ts, "yyyy-MM-dd HH:mm:ss");
        if (dt.isValid()) {
            date = dt.date();
        }
    }
    if (!date.isValid()) date = QDate::currentDate();

    QMenu menu(this);
    QAction *nexusViewAct = menu.addAction("ÃƒÂ¢Ã‚ÂÃ‚Â³ Nexus View (Time Machine)");
    
    QAction *selected = menu.exec(view->viewport()->mapToGlobal(pos));
    if (selected == nexusViewAct) {
        ui_equipment->tabWidget->setCurrentWidget(m_nexusWidget);
        m_nexusWidget->goToTimeMachineDate(date);
    }
}

// =============================================================================
// PRESENTATION MODE (DIRECTOR'S CUT)
// =============================================================================

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
            l->setText(prefix + " \u2705]");
            l->setStyleSheet("color: #4CAF50; font-size: 11px; font-weight: bold;");
        } else {
            l->setText(prefix + " \u2B1C]");
            l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        }
    };
    
    updateInd(m_eqTypeInd, typeOk, "[\U0001F528 Type");
    updateInd(m_eqDateInd, dateOk, "[\U0001F4C5 Date");
    updateInd(m_eqPriceInd, priceOk, "[\U0001F4B0 Price");
    updateInd(m_eqDescInd, descOk, "[\U0001F4DD Desc");

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
    flyer->setText("ÃƒÂ°Ã…Â¸Ã¢â‚¬ÂºÃ‚Â ÃƒÂ¯Ã‚Â¸Ã‚Â " + equipName);
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
    QLabel *toast = new QLabel("ÃƒÂ¢Ã…â€œÃ¢â‚¬Â¦ " + equipName + " added to workshop!", this);
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
// UNREAD MESSAGES SPLASH ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â after login
// =============================================================================

void MainWindow::ensureEquipmentHistoryDatabaseObjects()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "Equipment history bootstrap skipped: no open database connection.";
        return;
    }

    auto execStep = [](const QString &sql, const QString &stepName) {
        QSqlQuery q;
        if (!q.exec(sql)) {
            qDebug() << "Equipment history bootstrap failed at" << stepName << ":" << q.lastError().text();
            return false;
        }
        return true;
    };

    const QString createTableBlock = R"SQL(
BEGIN
    EXECUTE IMMEDIATE '
        CREATE TABLE EQUIPMENT_HISTORY (
            HISTORY_ID NUMBER PRIMARY KEY,
            EQUIPMENT_ID NUMBER,
            OPERATION_TYPE VARCHAR2(10 CHAR) NOT NULL,
            EQUIPMENT_TYPE VARCHAR2(255 CHAR),
            DESCRIPTION VARCHAR2(4000 CHAR),
            STATUS VARCHAR2(100 CHAR),
            UNIT_PRICE NUMBER(12,2),
            QUANTITY NUMBER,
            CHANGE_DATE DATE DEFAULT SYSDATE NOT NULL,
            CHANGED_BY VARCHAR2(128 CHAR)
        )';
EXCEPTION
    WHEN OTHERS THEN
        IF SQLCODE != -955 THEN
            RAISE;
        END IF;
END;
)SQL";

    const QString createSequenceBlock = R"SQL(
BEGIN
    EXECUTE IMMEDIATE 'CREATE SEQUENCE EQUIPMENT_HISTORY_SEQ START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE';
EXCEPTION
    WHEN OTHERS THEN
        IF SQLCODE != -955 THEN
            RAISE;
        END IF;
END;
)SQL";

    const QString createIndexBlock = R"SQL(
BEGIN
    EXECUTE IMMEDIATE 'CREATE INDEX IDX_EQUIPMENT_HISTORY_OP_DATE ON EQUIPMENT_HISTORY (OPERATION_TYPE, CHANGE_DATE)';
EXCEPTION
    WHEN OTHERS THEN
        IF SQLCODE != -955 THEN
            RAISE;
        END IF;
END;
)SQL";

    const QString createTriggerSql = R"SQL(
CREATE OR REPLACE TRIGGER TRG_EQUIPMENT_HISTORY_AUDIT
AFTER INSERT OR UPDATE OR DELETE ON EQUIPMENT
FOR EACH ROW
BEGIN
    IF INSERTING THEN
        INSERT INTO EQUIPMENT_HISTORY (
            HISTORY_ID, EQUIPMENT_ID, OPERATION_TYPE, EQUIPMENT_TYPE, DESCRIPTION,
            STATUS, UNIT_PRICE, QUANTITY, CHANGE_DATE, CHANGED_BY
        ) VALUES (
            EQUIPMENT_HISTORY_SEQ.NEXTVAL,
            :NEW.EQUIPMENT_ID,
            'ADD',
            :NEW.EQUIPMENT_TYPE,
            :NEW.DESCRIPTION,
            :NEW.STATUS,
            :NEW.UNIT_PRICE,
            :NEW.QUANTITY,
            SYSDATE,
            USER
        );
    ELSIF UPDATING THEN
        INSERT INTO EQUIPMENT_HISTORY (
            HISTORY_ID, EQUIPMENT_ID, OPERATION_TYPE, EQUIPMENT_TYPE, DESCRIPTION,
            STATUS, UNIT_PRICE, QUANTITY, CHANGE_DATE, CHANGED_BY
        ) VALUES (
            EQUIPMENT_HISTORY_SEQ.NEXTVAL,
            :NEW.EQUIPMENT_ID,
            'MODIFY',
            :NEW.EQUIPMENT_TYPE,
            :NEW.DESCRIPTION,
            :NEW.STATUS,
            :NEW.UNIT_PRICE,
            :NEW.QUANTITY,
            SYSDATE,
            USER
        );
    ELSIF DELETING THEN
        INSERT INTO EQUIPMENT_HISTORY (
            HISTORY_ID, EQUIPMENT_ID, OPERATION_TYPE, EQUIPMENT_TYPE, DESCRIPTION,
            STATUS, UNIT_PRICE, QUANTITY, CHANGE_DATE, CHANGED_BY
        ) VALUES (
            EQUIPMENT_HISTORY_SEQ.NEXTVAL,
            :OLD.EQUIPMENT_ID,
            'DELETE',
            :OLD.EQUIPMENT_TYPE,
            :OLD.DESCRIPTION,
            :OLD.STATUS,
            :OLD.UNIT_PRICE,
            :OLD.QUANTITY,
            SYSDATE,
            USER
        );
    END IF;
END;
)SQL";

    if (!execStep(createTableBlock, "create table")) return;
    if (!execStep(createSequenceBlock, "create sequence")) return;
    if (!execStep(createIndexBlock, "create index")) return;
    if (!execStep(createTriggerSql, "create trigger")) return;
}
void MainWindow::setupEquipmentConnections()
{
    // --- NEXUS TAB: Add programmatically as tab index 5 ---
    m_nexusWidget = new NexusWidget(equipmentPage);
    ui_equipment->tabWidget->addTab(m_nexusWidget, "NEXUS");

    // --- COSTS TAB: Add programmatically as tab index 6 ---
    m_costsWidget = new CostsWidget(equipmentPage);
    ui_equipment->tabWidget->addTab(m_costsWidget, "COSTS");

    // --- Equipment CRUD Connections ---
    connect(ui_equipment->btn_clear,  &QPushButton::clicked, this, &MainWindow::onEquipmentClearFields);
    connect(ui_equipment->btn_add,    &QPushButton::clicked, this, &MainWindow::onEquipmentAdd);

    connect(ui_equipment->btn_modify, &QPushButton::clicked, this, &MainWindow::onEquipmentModify);
    connect(ui_equipment->btn_delete, &QPushButton::clicked, this, &MainWindow::onEquipmentDelete);
    connect(ui_equipment->btn_delete_confirm, &QPushButton::clicked, this, &MainWindow::onEquipmentDelete);
    connect(ui_equipment->btn_share_chat, &QPushButton::clicked, this, &MainWindow::onEquipmentShareToChat);
    connect(ui_equipment->btn_search, &QPushButton::clicked, this, &MainWindow::onEquipmentSearch);
    connect(ui_equipment->le_recherche, &QLineEdit::returnPressed, this, &MainWindow::onEquipmentSearch);

    // Load row into form and show Identity Card
    connect(ui_equipment->table_equipments, &QAbstractItemView::clicked, this, [this](const QModelIndex &idx){
        QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
        if (!m) return;

        QString id      = m->data(m->index(idx.row(), 2)).toString();
        QString type    = m->data(m->index(idx.row(), 3)).toString();
        QString cond    = m->data(m->index(idx.row(), 6)).toString();
        double  price   = m->data(m->index(idx.row(), 5)).toDouble();
        QString desc    = m->data(m->index(idx.row(), 8)).toString();

        // Show Identity Card only in animation mode
        if (m_identityCard) {
            const bool animMode = (homeWindow && homeWindow->isAnimationMode());
            if (animMode) {
                m_identityCard->setup(desc, type, cond, price, id.toInt());
                m_identityCard->animateOpen();
            } else {
                m_identityCard->hide();
            }
        }

        if (idx.column() == 0) { // Edit Action
            QString dateStr = m->data(m->index(idx.row(), 7)).toString();
            int     qty     = m->data(m->index(idx.row(), 4)).toInt();

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

            ui_equipment->tabWidget->setCurrentWidget(ui_equipment->tab_gestion);
            if (QRadioButton *rbMod = ui_equipment->tab_gestion->findChild<QRadioButton*>("rb_equipment_mod_mode")) {
                rbMod->setChecked(true);
            }
        } else if (idx.column() == 1) { // Delete Action
            ui_equipment->table_equipments->setCurrentIndex(idx);
            onEquipmentDelete();
        }
    });

    // Radial Menu Activation
    ui_equipment->table_equipments->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->table_equipments, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){
        QModelIndex idx = ui_equipment->table_equipments->indexAt(pos);
        if (idx.isValid()) {
            bool animMode = (homeWindow && homeWindow->isAnimationMode());
            if (animMode) {
                QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
                int id = m->data(m->index(idx.row(), 2)).toInt();
                // Map to a fixed position like right side.
                // We let the menu position itself by passing a dummy pos or global geometry.
                m_radialMenu->showMenu(ui_equipment->tab_view->mapToGlobal(QPoint(ui_equipment->tab_view->width() - 320, 50)), id);
            }
        }
    });

    // Auto-refresh equipment views/stats when switching tabs
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (m_radialMenu) m_radialMenu->hide();
        if (m_identityCard) m_identityCard->hide();

        if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_view) {
            onEquipmentRefreshView();
        } else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_history) {
            onEquipmentHistoryRefresh();
        } else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_stats) {
            setupEquipmentStats();
        } else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_chat) {
            onChatEmployeeListRefresh();
            chatRefreshTimer->start(3000);
        } else if (ui_equipment->tabWidget->widget(idx) == m_nexusWidget) {
            m_nexusWidget->initialize();
            chatRefreshTimer->stop();
        } else if (ui_equipment->tabWidget->widget(idx) == m_costsWidget) {
            m_costsWidget->initialize();
            chatRefreshTimer->stop();
        } else {
            chatRefreshTimer->stop();
        }
    });

    // History Connections
    connect(ui_equipment->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_clear_history, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryClear);
    connect(ui_equipment->le_history_search, &QLineEdit::returnPressed, this, &MainWindow::onEquipmentHistorySearch);
    connect(ui_equipment->btn_history_search, &QPushButton::clicked, this, &MainWindow::onEquipmentHistorySearch);

    // Tab Navigation for Equipment
    ui_equipment->tabWidget->tabBar()->hide();
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
    
    // Ensure History Table Exists
    onChatEnsureTable();
    
    chatRefreshTimer = new QTimer(this);
    connect(chatRefreshTimer, &QTimer::timeout, this, &MainWindow::onChatRefresh);

    // Keep equipment pages in sync
    equipmentSyncTimer = new QTimer(this);
    equipmentSyncTimer->setInterval(2000);
    connect(equipmentSyncTimer, &QTimer::timeout, this, [this]() {
        if (!ui || !ui->stackedWidget || !ui_equipment || !ui_equipment->tabWidget) return;
        const bool onEquipmentModule = (ui->stackedWidget->currentWidget() == equipmentPage);
        QWidget *currentTab = ui_equipment->tabWidget->currentWidget();
        onEquipmentHistorySearch();
        if (onEquipmentModule && (currentTab == ui_equipment->tab_view || currentTab == ui_equipment->tab_history)) {
            onEquipmentRefreshView();
        }
    });
    equipmentSyncTimer->start();
    
    // Connect Chat buttons
    connect(ui_equipment->btn_chat_send, &QPushButton::clicked, this, &MainWindow::onChatSendMessage);
    connect(ui_equipment->le_chat_input, &QLineEdit::returnPressed, this, &MainWindow::onChatSendMessage);
    connect(ui_equipment->list_employees, &QListWidget::itemClicked, this, &MainWindow::onChatEmployeeSelected);
    connect(ui_equipment->btn_chat_img, &QPushButton::clicked, this, &MainWindow::onChatAttachImage);
    
    // Add Emoji & GIF buttons
    {
        QHBoxLayout *inputLayout = ui_equipment->horizontalLayout_input;
        QPushButton *emojiBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x98\x80"), equipmentPage);
        emojiBtn->setObjectName("btn_chat_emoji");
        emojiBtn->setFixedSize(44, 44);
        emojiBtn->setCursor(Qt::PointingHandCursor);
        emojiBtn->setStyleSheet("QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; } QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }");
        
        QPushButton *gifBtn = new QPushButton("GIF", equipmentPage);
        gifBtn->setObjectName("btn_chat_gif");
        gifBtn->setFixedSize(50, 44);
        gifBtn->setCursor(Qt::PointingHandCursor);
        gifBtn->setStyleSheet("QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 13px; font-weight: bold; } QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }");
        
        inputLayout->insertWidget(1, emojiBtn);
        inputLayout->insertWidget(2, gifBtn);
        
        connect(emojiBtn, &QPushButton::clicked, this, &MainWindow::onChatEmojiClicked);
        connect(gifBtn, &QPushButton::clicked, this, &MainWindow::onChatGifClicked);
    }
    
    aiNetworkManager = new QNetworkAccessManager(this);
    aiApiKey = "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";
    giphyNetworkManager = new QNetworkAccessManager(this);
    chatSummaryNetManager = new QNetworkAccessManager(this);
    
    m_hoverCard = new EquipmentHoverCard(equipmentPage);
    m_completerModel = new QStringListModel(this);
    m_chatCompleter = new QCompleter(m_completerModel, this);
    ui_equipment->le_chat_input->setCompleter(m_chatCompleter);
    
    connect(ui_equipment->le_chat_input, &QLineEdit::textChanged, this, [this](const QString &text){
        if (!ui_equipment || !m_hoverCard) return;
        static QRegularExpression reg("#(\\d+)");
        QRegularExpressionMatchIterator it = reg.globalMatch(text);
        QRegularExpressionMatch lastMatch;
        while (it.hasNext()) lastMatch = it.next();
        if (lastMatch.hasMatch()) {
            int id = lastMatch.captured(1).toInt();
            QPoint pos = ui_equipment->le_chat_input->mapTo(equipmentPage, QPoint(120, -145));
            m_hoverCard->showCard(id, pos);
        } else {
            m_hoverCard->hide();
        }
    });

    if (ui_equipment->btn_chat_voice) {
        ui_equipment->btn_chat_voice->setVisible(false);
    }
    
    QShortcut *chatSearchShortcut = new QShortcut(QKeySequence("Ctrl+F"), equipmentPage);
    connect(chatSearchShortcut, &QShortcut::activated, this, &MainWindow::onChatSearchToggle);
    
    if (auto *refreshBtn = equipmentPage->findChild<QPushButton*>("btn_chat_refresh"))
        connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onChatRefresh);
    
    if (auto *searchEdit = equipmentPage->findChild<QLineEdit*>("le_chat_search"))
        connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &txt){
            if (!ui_equipment) return;
            for (int i = 0; i < ui_equipment->list_employees->count(); ++i) {
                ui_equipment->list_employees->item(i)->setHidden(!ui_equipment->list_employees->item(i)->text().contains(txt, Qt::CaseInsensitive));
            }
        });

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
    
    connect(ui_equipment->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
}
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
    
    QString typeTag = "[Message]";
    if (hasImage) typeTag = "[Image]";
    
    msgObj["message"] = msg.isEmpty() ? typeTag : msg;
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
            "QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1A1208, stop:1 #0A0804); border-radius: 16px; border: 1.5px solid #5A4A32; }"
        );
        ui_equipment->frame_chat_header->setStyleSheet(
            "QFrame#frame_chat_header { background: rgba(30, 20, 10, 0.85); border-bottom: 2px solid #D4AF37; border-radius: 14px 14px 0px 0px; }"
        );
        ui_equipment->frame_input_bar->setStyleSheet(
            "QFrame#frame_input_bar { background: rgba(25, 20, 15, 0.9); border-top: 1px solid #D4AF37; border-radius: 0 0 14px 14px; }"
        );
    } else {
        ui_equipment->frame_chat_panel->setStyleSheet(
            "QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2D2416, stop:1 #1A1208); border-radius: 16px; border: 1.5px solid #5A4A32; }"
        );
        ui_equipment->frame_chat_header->setStyleSheet(
            "QFrame#frame_chat_header { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3820); border-bottom: 2px solid #8B6F47; border-radius: 14px 14px 0px 0px; }"
        );
        ui_equipment->frame_input_bar->setStyleSheet(
            "QFrame#frame_input_bar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2A2010,stop:1 #1E1608); border-top: 2px solid #5A4A32; border-radius: 0 0 14px 14px; }"
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
    ui_equipment->verticalLayout_chat_contents->setSpacing(15);
    ui_equipment->verticalLayout_chat_contents->setContentsMargins(10, 10, 10, 10);
    ui_equipment->scrollArea_chat->setWidgetResizable(true);

    // Map to quickly get employee names
    static QMap<int, QPair<QString, QString>> employeeInfo;
    static int s_lastMessageCount = -1;
    
    // Check for notification badge
    if (s_lastMessageCount != -1 && allMessages.size() > s_lastMessageCount && ui_equipment->tabWidget->currentIndex() != 4) {
        QJsonObject lastM = allMessages.last().toObject();
        if (lastM["sender_id"].toInt() != currentEmployeeId) { 
             ui_equipment->tabWidget->setTabText(4, QString::fromUtf8("Chat \xF0\x9F\x94\xB4"));
        }
    }
    
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

    // Filter first to determine relevant history size
    QJsonArray filteredMessages;
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();
        if ((s_id == currentEmployeeId && r_id == currentChatPartnerId) ||
            (s_id == currentChatPartnerId && r_id == currentEmployeeId)) {
            filteredMessages.append(m);
        }
    }

    // --- Optimization: Render only the last 40 messages to prevent crash/resource exhaustion ---
    int startIdx = qMax(0, filteredMessages.size() - 40);
    if (startIdx > 0) {
        QLabel *limitHint = new QLabel("--- Older messages hidden for performance ---");
        limitHint->setAlignment(Qt::AlignCenter);
        limitHint->setStyleSheet("color: rgba(255,255,255,0.3); font-style: italic; font-size: 10px; padding: 10px;");
        ui_equipment->verticalLayout_chat_contents->addWidget(limitHint);
    }

    for (int i = startIdx; i < filteredMessages.size(); ++i) {
        QJsonObject m = filteredMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        
        QString msg = m["message"].toString();
        QDateTime dt = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
        bool isMe = (s_id == currentEmployeeId);
        
        QPair<QString, QString> names = employeeInfo.value(s_id, qMakePair(QString("Emp"), QString::number(s_id)));
        QString senderName = names.first + " " + names.second;

        QWidget *msgContainer = new QWidget();
        QHBoxLayout *msgHBox = new QHBoxLayout(msgContainer);
        msgHBox->setContentsMargins(5, 2, 5, 2);
        msgHBox->setSpacing(10);

        // Avatar
        QLabel *avatarLbl = new QLabel();
        avatarLbl->setFixedSize(36, 36);
        avatarLbl->setAlignment(Qt::AlignCenter);
        avatarLbl->setStyleSheet(QString("background: %1; color: white; border-radius: 18px; font-weight: bold; border: 1.5px solid rgba(255,255,255,0.2);")
                                 .arg(avatarColors[s_id % avatarColors.size()]));
        avatarLbl->setText(senderName.at(0).toUpper());

        // Bubble
        QFrame *bubble = new QFrame();
        bubble->setObjectName("chat_bubble");
        QString bubbleStyle = isMe ? 
            "QFrame#chat_bubble { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3A2D1A, stop:1 #2A2010); border: 1.2px solid #D4AF37; border-radius: 14px; }" :
            "QFrame#chat_bubble { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1A222A, stop:1 #0A1218); border: 1.2px solid #3B82F6; border-radius: 14px; }";
        bubble->setStyleSheet(bubbleStyle);
        
        QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
        bubbleLayout->setContentsMargins(12, 8, 12, 8);
        bubbleLayout->setSpacing(4);

        QLabel *nameLbl = new QLabel(senderName.toUpper());
        nameLbl->setStyleSheet("font-size: 9px; font-weight: bold; color: rgba(255,255,255,0.6);");
        bubbleLayout->addWidget(nameLbl);

        // Media Content (Images)
        if (m.contains("image_data")) {
            QLabel *imgLbl = new QLabel();
            QPixmap pix;
            pix.loadFromData(QByteArray::fromBase64(m["image_data"].toString().toLatin1()));
            imgLbl->setPixmap(pix.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            bubbleLayout->addWidget(imgLbl);
        }

        // Media Content (GIFs - Fixed Playback & Stability)
        if (m.contains("gif_url")) {
            QLabel *gifLbl = new QLabel();
            gifLbl->setMinimumSize(200, 150);
            
            // Use existing manager or create one if null
            if (!aiNetworkManager) aiNetworkManager = new QNetworkAccessManager(this);
            
            QNetworkReply *rep = giphyNetworkManager->get(QNetworkRequest(QUrl(m["gif_url"].toString())));
            connect(rep, &QNetworkReply::finished, gifLbl, [gifLbl, rep]() {
                if (rep->error() == QNetworkReply::NoError) {
                    QByteArray data = rep->readAll();
                    if (!data.isEmpty()) {
                        QBuffer *buffer = new QBuffer(gifLbl);
                        buffer->setData(data);
                        buffer->open(QIODevice::ReadOnly);
                        
                        QMovie *movie = new QMovie(buffer, QByteArray(), gifLbl);
                        gifLbl->setMovie(movie);
                        movie->setScaledSize(QSize(200, 150));
                        movie->start();
                    }
                }
                rep->deleteLater();
            });
            bubbleLayout->addWidget(gifLbl);
        }

        if (msg.contains(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Note"))) {
            VoiceWaveformWidget *wave = new VoiceWaveformWidget();
            wave->startAnim();
            bubbleLayout->addWidget(wave);
        } else if (msg != "[Image]" && msg != "[GIF]") {
            QLabel *contentLbl = new QLabel(msg);
            contentLbl->setWordWrap(true);
            contentLbl->setStyleSheet("color: white; font-size: 13px;");
            bubbleLayout->addWidget(contentLbl);
        }

        QLabel *timeLbl = new QLabel(dt.toString("HH:mm"));
        timeLbl->setAlignment(Qt::AlignRight);
        timeLbl->setStyleSheet("font-size: 8px; color: rgba(255,255,255,0.4);");
        bubbleLayout->addWidget(timeLbl);

        if (isMe) {
            msgHBox->addStretch();
            msgHBox->addWidget(bubble);
            msgHBox->addWidget(avatarLbl);
        } else {
            msgHBox->addWidget(avatarLbl);
            msgHBox->addWidget(bubble);
            msgHBox->addStretch();
        }

        ui_equipment->verticalLayout_chat_contents->insertWidget(ui_equipment->verticalLayout_chat_contents->count() - 1, msgContainer);
    }

    // Smooth Scroll to bottom
    QTimer::singleShot(50, this, [this](){
        if(ui_equipment->scrollArea_chat->verticalScrollBar()) {
            QScrollBar *sb = ui_equipment->scrollArea_chat->verticalScrollBar();
            QPropertyAnimation *anim = new QPropertyAnimation(sb, "value");
            anim->setDuration(400);
            anim->setStartValue(sb->value());
            anim->setEndValue(sb->maximum());
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });
    
    s_lastMessageCount = allMessages.size();
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
        lbl->setText("Image attached: " + fi.fileName() + " ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â press Send to include it");
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
    QLabel *title = new QLabel("GIF Search ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Powered by GIPHY", card);
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
    dialog->setFixedSize(320, 360);

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

    // --- Clear Chat ---
    QPushButton *btnClear = new QPushButton("Clear Conversation", card);
    btnClear->setStyleSheet(
        "QPushButton { background: rgba(200,50,50,0.15); border: 2px solid #662222; border-radius: 14px; height: 40px; color: #FF9999; font-weight: bold; }"
        "QPushButton:hover { background: #882222; color: white; }"
    );
    layout->addWidget(btnClear);
    connect(btnClear, &QPushButton::clicked, this, [this, dialog]() {
        if (QMessageBox::question(dialog, "Clear", "Wipe all messages for this chat?") == QMessageBox::Yes) {
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
    QPushButton *btnRestore = new QPushButton("Restore Deleted Messages", card);
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
        QLabel *rTitle = new QLabel("Select messages to restore:", restoreDialog);
        rTitle->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 14px;");
        rLay->addWidget(rTitle);
        
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
            
            QString chatFilePath = "hammerdown_chat.json";
            QFile file(chatFilePath);
            QJsonArray chatArray;
            if (file.open(QIODevice::ReadOnly)) {
                chatArray = QJsonDocument::fromJson(file.readAll()).array();
                file.close();
            }
            chatArray.append(toRestore);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(QJsonDocument(chatArray).toJson());
                file.close();
            }
            
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
        
        // --- Audio Sync: Pause home music ---
        pauseHomeAudioForWeather();

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
                    resumeHomeAudioAfterWeather();
                    showAssistantAndResumeMusic();
                });
                fadeOutAnim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        });

        player->play();
    }
}

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





