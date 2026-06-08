#include "costswidget.h"
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QDebug>
#include <QtMath>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <algorithm>

// ============================================================================
// MAIN COSTS WIDGET
// ============================================================================
CostsWidget::CostsWidget(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    buildUI();
}

void CostsWidget::buildUI() {
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16); // 60fps
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_globalTime += 0.016f;
        update();
    });

    // Loading timer
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(30);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        runLoadingAnimation();
    });

    // KPI count-up timer
    m_kpiAnimTimer = new QTimer(this);
    m_kpiAnimTimer->setInterval(16);
    connect(m_kpiAnimTimer, &QTimer::timeout, this, [this]() {
        m_kpiAnimProgress += 0.012f;
        if (m_kpiAnimProgress >= 1.0f) {
            m_kpiAnimProgress = 1.0f;
            m_kpiAnimTimer->stop();
        }
        update();
    });

    // Create sub-tab widgets
    m_tcoWidget = new TCOWidget(this);
    m_repairWidget = new RepairReplaceWidget(this);
    m_forecastWidget = new ForecastWidget(this);
    m_roiWidget = new ROIWidget(this);
    m_timelineWidget = new TimelineWidget(this);
    m_reportWidget = new ReportWidget(this);

    // Stack
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(m_tcoWidget);
    m_stack->addWidget(m_repairWidget);
    m_stack->addWidget(m_forecastWidget);
    m_stack->addWidget(m_roiWidget);
    m_stack->addWidget(m_timelineWidget);
    m_stack->addWidget(m_reportWidget);
    m_stack->hide();

    // Sub-tab buttons
    QStringList tabNames = {
        QString::fromUtf8("\xF0\x9F\x92\xB0 Ownership"),
        QString::fromUtf8("\xF0\x9F\x94\xA7 Repair/Replace"),
        QString::fromUtf8("\xF0\x9F\x93\x85 Forecast"),
        QString::fromUtf8("\xF0\x9F\x93\x8A ROI"),
        QString::fromUtf8("\xF0\x9F\x97\x93 Timeline"),
        QString::fromUtf8("\xF0\x9F\x93\x84 Report")
    };
    for (int i = 0; i < tabNames.size(); i++) {
        QPushButton *btn = new QPushButton(tabNames[i], this);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(34);
        if (i == 0) btn->setChecked(true);
        connect(btn, &QPushButton::clicked, this, [this, i]() { switchSubTab(i); });
        m_tabButtons.append(btn);
    }

    // Groq client
    m_groqClient = new CostGroqClient(this);
    m_groqClient->setApiKey("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");
    connect(m_groqClient, &CostGroqClient::insightReady, this, [this](const QString &insight) {
        m_currentInsight = insight;
        m_insightCharIndex = 0;
        m_displayedInsight.clear();
        update();
    });

    // Engine
    m_engine = new CostCalculationEngine();
}

void CostsWidget::initialize() {
    if (m_isLoaded) return;
    m_loadingProgress = 0.0f;
    m_loadingPhase = 0;
    m_animTimer->start();
    m_loadingTimer->start();
    update();
}

void CostsWidget::runLoadingAnimation() {
    m_loadingProgress += 0.02f;
    if (m_loadingProgress >= 1.0f) {
        m_loadingProgress = 1.0f;
        m_loadingTimer->stop();
        m_isLoaded = true;

        // Run calculations
        m_engine->calculate();
        onDataReady();
    }
    update();
}

void CostsWidget::onDataReady() {
    auto data = m_engine->results();
    m_totalInventory = m_engine->totalInventoryValue();
    m_totalTCO = m_engine->totalTCO();
    m_forecast90 = m_engine->forecast90Days();
    m_topPerformer = m_engine->topPerformer();
    m_urgentActions = m_engine->urgentActions();

    // Distribute data to sub-tabs
    m_tcoWidget->setData(data);
    m_repairWidget->setData(data);
    m_forecastWidget->setData(data);
    m_roiWidget->setData(data);
    m_timelineWidget->setData(data);
    m_reportWidget->setData(data);

    // Show stack
    m_stack->show();
    m_stack->setGeometry(0, 138, width(), height() - 138 - 56);

    // Position sub-tab buttons
    int btnX = 20;
    for (auto *btn : m_tabButtons) {
        QFontMetrics fm(btn->font());
        int w = fm.horizontalAdvance(btn->text()) + 44;
        btn->setGeometry(btnX, 93, w, 34);
        btn->show();
        btnX += w + 8;
    }

    // Start KPI animation
    m_kpiAnimProgress = 0.0f;
    m_kpiAnimTimer->start();

    // Request initial Groq insight
    requestGroqInsight();

    emit costsReady();
}

void CostsWidget::switchSubTab(int index) {
    m_activeSubTab = index;
    m_stack->setCurrentIndex(index);
    for (int i = 0; i < m_tabButtons.size(); i++) {
        m_tabButtons[i]->setChecked(i == index);
    }
    requestGroqInsight();
    update();
}

void CostsWidget::requestGroqInsight() {
    if (m_engine->results().isEmpty()) return;

    QString sys = "You are a senior financial analyst specializing in industrial equipment ROI. "
                  "Be specific, use the numbers provided, give actionable advice. Direct and brief. "
                  "Under 45 words. No generic statements.";
    QString usr;
    auto data = m_engine->results();

    switch (m_activeSubTab) {
    case 0: // TCO
        usr = QString("Total workshop TCO: %1dt across %2 equipment. Average cost/yr: %3dt. One insight.")
              .arg(m_totalTCO, 0, 'f', 0).arg(data.size()).arg(data.isEmpty() ? 0 : m_totalTCO / data.size(), 0, 'f', 0);
        break;
    case 1: // Repair/Replace
        if (!data.isEmpty()) {
            auto &e = data[0];
            usr = QString("Repair %1dt vs replace %2dt for %3. Remaining life %4yr. One recommendation.")
                  .arg(e.repairCost, 0, 'f', 0).arg(e.replacementCost, 0, 'f', 0).arg(e.name).arg(e.remainingYears);
        }
        break;
    case 2: // Forecast
        usr = QString("Next 3 months total forecast: %1dt. %2 equipment tracked. One budget advice.")
              .arg(m_forecast90, 0, 'f', 0).arg(data.size());
        break;
    case 3: { // ROI
        auto sorted = data;
        std::sort(sorted.begin(), sorted.end(), [](const EquipmentFinancials &a, const EquipmentFinancials &b) {
            return a.roiScore > b.roiScore;
        });
        if (sorted.size() >= 2)
            usr = QString("Best ROI: %1 %2/100. Worst: %3 %4/100. Gap insight in one sentence.")
                  .arg(sorted.first().name).arg(sorted.first().roiScore, 0, 'f', 0)
                  .arg(sorted.last().name).arg(sorted.last().roiScore, 0, 'f', 0);
        break;
    }
    case 4: { // Timeline
        auto sorted = data;
        std::sort(sorted.begin(), sorted.end(), [](const EquipmentFinancials &a, const EquipmentFinancials &b) {
            return a.remainingYears < b.remainingYears;
        });
        if (!sorted.isEmpty())
            usr = QString("Next replacement: %1 in %2yr costing %3dt. Monthly saving needed: %4dt. One planning insight.")
                  .arg(sorted.first().name).arg(sorted.first().remainingYears)
                  .arg(sorted.first().replacementCost, 0, 'f', 0).arg(sorted.first().monthlyBudgetNeeded, 0, 'f', 0);
        break;
    }
    case 5: // Report
        usr = QString("Prepare executive cost report focus: inventory %1dt, TCO %2dt, urgent actions %3. One board-level recommendation.")
              .arg(m_totalInventory, 0, 'f', 0).arg(m_totalTCO, 0, 'f', 0).arg(m_urgentActions);
        break;
    default:
        usr = QString("Workshop has %1 equipment worth %2dt total. One general financial insight.")
              .arg(data.size()).arg(m_totalInventory, 0, 'f', 0);
        break;
    }

    if (!usr.isEmpty())
        m_groqClient->requestInsight(sys, usr);
    else {
        m_currentInsight = "Select equipment data for AI analysis.";
        m_insightCharIndex = 0;
        m_displayedInsight.clear();
    }
}

QString CostsWidget::formatNumber(double val) {
    if (val >= 1000000) return QString("%1M").arg(val / 1000000.0, 0, 'f', 1);
    if (val >= 1000) return QString("%1K").arg(val / 1000.0, 0, 'f', 1);
    return QString::number((int)val);
}

// ============================================================================
// PAINT EVENT
// ============================================================================
void CostsWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::TextAntialiasing);

    drawBackground(p);

    if (!m_isLoaded) {
        // Loading animation
        int cx = width() / 2, cy = height() / 2;
        // Spinning arc
        p.setPen(QPen(QColor("#C17F3E"), 3));
        int arcAngle = (int)(m_globalTime * 200) % 360;
        p.drawArc(cx - 30, cy - 30, 60, 60, arcAngle * 16, 90 * 16);

        p.setPen(QColor("#C17F3E"));
        QFont f = p.font(); f.setPixelSize(12); f.setBold(true); f.setLetterSpacing(QFont::AbsoluteSpacing, 2);
        p.setFont(f);
        p.drawText(QRect(0, cy + 50, width(), 30), Qt::AlignCenter,
                   QString("COST INTELLIGENCE INITIALIZING... %1%").arg((int)(m_loadingProgress * 100)));

        // Progress bar
        int bw = 300, bh = 4;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(26, 17, 8));
        p.drawRoundedRect(cx - bw/2, cy + 80, bw, bh, 2, 2);
        p.setBrush(QColor("#C17F3E"));
        p.drawRoundedRect(cx - bw/2, cy + 80, (int)(bw * m_loadingProgress), bh, 2, 2);
        return;
    }

    drawHeader(p);
    drawSubTabBar(p);
    drawInsightBar(p);

    // Position stack
    if (m_stack) {
        m_stack->setGeometry(0, 138, width(), height() - 138 - 56);
    }
}

// ============================================================================
// BACKGROUND
// ============================================================================
void CostsWidget::drawBackground(QPainter &p) {
    // Layer 1 — Deep void
    QRadialGradient voidGrad(width() * 0.4, height() * 0.35, qMax(width(), height()) * 0.7);
    voidGrad.setColorAt(0, QColor(26, 14, 6));
    voidGrad.setColorAt(0.4, QColor(15, 8, 4));
    voidGrad.setColorAt(1.0, QColor(0, 0, 0));
    p.fillRect(rect(), voidGrad);

    // Layer 2 — Grid
    float gridOffset = fmod(m_globalTime * 8.0f, 28.0f);
    p.setPen(QPen(QColor(193, 127, 62, 13), 1));
    for (int y = -(int)gridOffset; y < height(); y += 28) p.drawLine(0, y, width(), y);
    p.setPen(QPen(QColor(193, 127, 62, 8), 1));
    for (int x = 0; x < width(); x += 28) p.drawLine(x, 0, x, height());

    // Layer 3 — Ambient lights
    float breath1 = 0.7f + 0.3f * sinf(m_globalTime * 0.25f);
    QRadialGradient light1(width() * 0.85, height() * 0.1, width() * 0.4);
    light1.setColorAt(0, QColor(193, 127, 62, (int)(13 * breath1)));
    light1.setColorAt(1, QColor(193, 127, 62, 0));
    p.fillRect(rect(), light1);

    float breath2 = 0.7f + 0.3f * sinf(m_globalTime * 0.3f + 1.5f);
    QRadialGradient light2(width() * 0.15, height() * 0.9, width() * 0.35);
    light2.setColorAt(0, QColor(139, 90, 43, (int)(8 * breath2)));
    light2.setColorAt(1, QColor(139, 90, 43, 0));
    p.fillRect(rect(), light2);

    // Layer 4 — Particles (40 for performance)
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 40; i++) {
        float speed = 8.0f + (i % 5) * 3.0f;
        float seedX = ((i * 137) % width());
        float seedY = ((i * 251) % height());
        float px = seedX + sinf(m_globalTime * 0.15f + i * 0.8f) * 1.2f;
        float py = fmod(m_globalTime * speed + seedY, (float)height());
        float sz = 1.0f + (i % 3) * 0.5f;
        p.setBrush(QColor(193, 127, 62, 15));
        p.drawEllipse(QPointF(px, py), sz, sz);
    }

    // Layer 5 — Scanlines
    p.setPen(QPen(QColor(0, 0, 0, 15), 1));
    for (int y = 0; y < height(); y += 2) p.drawLine(0, y, width(), y);
}

// ============================================================================
// HEADER (top 90px)
// ============================================================================
void CostsWidget::drawHeader(QPainter &p) {
    // Header background
    QLinearGradient hbg(0, 0, 0, 90);
    hbg.setColorAt(0, QColor(30, 17, 8));
    hbg.setColorAt(1, QColor(13, 8, 5));
    p.fillRect(0, 0, width(), 90, hbg);

    // Bottom edge line
    float edgeOp = 0.4f + 0.2f * sinf(m_globalTime * 1.2f);
    QLinearGradient edgeGrad(0, 0, width(), 0);
    edgeGrad.setColorAt(0, Qt::transparent);
    edgeGrad.setColorAt(0.3, QColor(193, 127, 62, (int)(255 * edgeOp)));
    edgeGrad.setColorAt(0.5, QColor(255, 102, 0, (int)(255 * edgeOp)));
    edgeGrad.setColorAt(0.7, QColor(193, 127, 62, (int)(255 * edgeOp)));
    edgeGrad.setColorAt(1, Qt::transparent);
    p.setPen(QPen(QBrush(edgeGrad), 1.5));
    p.drawLine(0, 89, width(), 89);

        // Title (top-right)
    QFont titleFont = p.font();
    titleFont.setPixelSize(22); titleFont.setBold(true);
    p.setFont(titleFont);
        QString title = "COST INTELLIGENCE";
        QRect titleRect(width() - 560, 16, 540, 32);

    // Shadow passes
        p.setPen(QColor(0, 0, 0, 217)); p.drawText(titleRect.translated(2, 2), Qt::AlignRight | Qt::AlignVCenter, title);
        p.setPen(QColor(107, 58, 26, 166)); p.drawText(titleRect.translated(1, 1), Qt::AlignRight | Qt::AlignVCenter, title);
        p.setPen(QColor("#C17F3E")); p.drawText(titleRect, Qt::AlignRight | Qt::AlignVCenter, title);

    // Subtitle
    QFont subFont = p.font(); subFont.setPixelSize(10); subFont.setBold(false);
    subFont.setLetterSpacing(QFont::AbsoluteSpacing, 3);
    p.setFont(subFont);
    float subOp = 0.4f + 0.15f * sinf(m_globalTime * 0.35f);
    p.setPen(QColor(245, 230, 211, (int)(255 * subOp)));
        QRect subRect(width() - 620, 48, 600, 18);
        p.drawText(subRect, Qt::AlignRight | Qt::AlignVCenter, "PROFESSIONAL FINANCIAL INTELLIGENCE");

    // Animated underline
    QFontMetrics fm(titleFont);
    int tw = fm.horizontalAdvance(title);
    float ulWidth = tw * (0.5f + 0.5f * sinf(m_globalTime * 0.7f));
        float rightEdge = width() - 20;
        float ulX = rightEdge - tw + (tw - ulWidth) / 2.0f;
    QLinearGradient ulGrad(ulX, 0, ulX + ulWidth, 0);
    ulGrad.setColorAt(0, Qt::transparent);
    ulGrad.setColorAt(0.3, QColor("#C17F3E"));
    ulGrad.setColorAt(0.5, QColor("#FFD700"));
    ulGrad.setColorAt(0.7, QColor("#C17F3E"));
    ulGrad.setColorAt(1, Qt::transparent);
    p.setPen(QPen(QBrush(ulGrad), 1.5));
        p.drawLine(QPointF(ulX, 40), QPointF(ulX + ulWidth, 40));
}

void CostsWidget::drawKPI(QPainter &p, const QRectF &rect, const QString &value,
                           const QString &label, const QColor &color, int kpiIndex) {
    float scaleY = 1.0f + 0.015f * sinf(m_globalTime * 1.2f + kpiIndex * 0.7f);
    p.save();
    p.translate(rect.center());
    p.scale(1.0, scaleY);
    p.translate(-rect.center());

    // Shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 153));
    p.drawRoundedRect(rect.adjusted(2, 3, 2, 3), 10, 10);

    // Box background
    QLinearGradient bg(rect.topLeft(), rect.bottomRight());
    bg.setColorAt(0, QColor(30, 21, 16)); bg.setColorAt(1, QColor(13, 8, 5));
    p.setBrush(bg);
    QPen border(QColor(color.red(), color.green(), color.blue(), 80), 1);
    p.setPen(border);
    p.drawRoundedRect(rect, 10, 10);

    // Top accent
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    QPainterPath accent;
    accent.addRoundedRect(rect.x() + 1, rect.y() + 1, rect.width() - 2, 2, 1, 1);
    p.drawPath(accent);

    // Value
    QFont vf = p.font(); vf.setPixelSize(18); vf.setBold(true); p.setFont(vf);
    p.setPen(color);
    p.drawText(rect.adjusted(10, 12, -10, -20), Qt::AlignLeft | Qt::AlignVCenter, value);

    // Label
    QFont lf = p.font(); lf.setPixelSize(8); lf.setBold(false);
    lf.setLetterSpacing(QFont::AbsoluteSpacing, 2); lf.setCapitalization(QFont::AllUppercase);
    p.setFont(lf);
    p.setPen(QColor(245, 230, 211, 115));
    p.drawText(rect.adjusted(10, 0, -10, -6), Qt::AlignLeft | Qt::AlignBottom, label);

    p.restore();
}

// ============================================================================
// SUB-TAB BAR
// ============================================================================
void CostsWidget::drawSubTabBar(QPainter &p) {
    // Background bar
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(13, 8, 5));
    p.drawRect(0, 90, width(), 48);

    // Bottom border
    p.setPen(QPen(QColor(26, 18, 8), 1));
    p.drawLine(0, 137, width(), 137);

    // Style buttons
    for (int i = 0; i < m_tabButtons.size(); i++) {
        QPushButton *btn = m_tabButtons[i];
        bool active = btn->isChecked();
        if (active) {
            btn->setStyleSheet(
                "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #C17F3E,stop:1 #8B4A1E); "
                "border: 1px solid rgba(255,176,96,0.6); color: white; font-weight: bold; font-size: 11px; "
                "border-radius: 17px; padding: 0px 22px; }"
            );
        } else {
            btn->setStyleSheet(
                "QPushButton { background: transparent; border: 1px solid rgba(193,127,62,0.35); "
                "color: rgba(193,127,62,0.7); font-weight: bold; font-size: 11px; "
                "border-radius: 17px; padding: 0px 22px; }"
                "QPushButton:hover { background: rgba(193,127,62,0.12); border-color: rgba(193,127,62,0.7); "
                "color: #C17F3E; }"
            );
        }
    }
}

// ============================================================================
// INSIGHT BAR (bottom 56px)
// ============================================================================
void CostsWidget::drawInsightBar(QPainter &p) {
    int by = height() - 56;

    // Background
    QLinearGradient bg(0, by, 0, height());
    bg.setColorAt(0, QColor(13, 8, 5)); bg.setColorAt(1, QColor(8, 5, 3));
    p.fillRect(0, by, width(), 56, bg);

    // Top border
    float borderOp = 0.3f + 0.2f * sinf(m_globalTime * 1.2f);
    p.setPen(QPen(QColor(193, 127, 62, (int)(255 * borderOp)), 1));
    p.drawLine(0, by, width(), by);

    // Pulsing dot
    float dotR = 6.0f + 2.0f * sinf(m_globalTime * 2.5f);
    p.setPen(Qt::NoPen);
    QRadialGradient dotGlow(22, by + 28, 12);
    dotGlow.setColorAt(0, QColor("#C17F3E"));
    dotGlow.setColorAt(1, QColor(193, 127, 62, 0));
    p.setBrush(dotGlow);
    p.drawEllipse(QPointF(22, by + 28), 12, 12);
    p.setBrush(QColor("#C17F3E"));
    p.drawEllipse(QPointF(22, by + 28), dotR, dotR);

    // Label
    QFont lf = p.font(); lf.setPixelSize(10); lf.setBold(true); p.setFont(lf);
    p.setPen(QColor("#C17F3E"));
    p.drawText(36, by + 33, QString::fromUtf8("\xF0\x9F\x92\xA1 FINANCIAL INSIGHT:"));

    // Typewriter insight text
    if (!m_currentInsight.isEmpty()) {
        m_insightCharIndex += 0.6f;
        int showChars = qMin((int)m_insightCharIndex, m_currentInsight.length());
        m_displayedInsight = m_currentInsight.left(showChars);
    }

    QFont tf = p.font(); tf.setPixelSize(12); tf.setItalic(true); tf.setBold(false); p.setFont(tf);
    p.setPen(QColor(245, 230, 211));
    QRect textRect(200, by + 8, width() - 350, 40);
    p.drawText(textRect, Qt::AlignVCenter | Qt::TextWordWrap, m_displayedInsight);

    // Loading dots if Groq is loading
    if (m_groqClient && m_groqClient->isLoading()) {
        for (int i = 0; i < 3; i++) {
            float dotOp = (sinf(m_globalTime * 4.0f + i * 1.0f) + 1.0f) / 2.0f;
            p.setBrush(QColor(193, 127, 62, (int)(255 * dotOp)));
            p.drawEllipse(QPointF(width() - 80 + i * 12, by + 28), 3, 3);
        }
    }

    // "powered by Groq AI"
    QFont pf = p.font(); pf.setPixelSize(8); pf.setItalic(false); p.setFont(pf);
    p.setPen(QColor(245, 230, 211, 64));
    p.drawText(width() - 110, by + 48, "powered by Groq AI");
}
