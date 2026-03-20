// nexuswidget_ui.cpp — Display widgets + Main NexusWidget container

#include "nexuswidget.h"
#include <QPainterPath>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QtMath>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QScrollArea>
#include <QButtonGroup>
#include <algorithm>

extern QList<NexusEquipment> loadAllEquipment();
extern QList<NexusEmployee> loadAllEmployees();

// ============================================================================
// INFERENCE DISPLAY WIDGET
// ============================================================================
InferenceDisplayWidget::InferenceDisplayWidget(QWidget *parent) : QWidget(parent), m_visibleCount(0) {
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(10, 10, 10, 10);

    QLabel *title = new QLabel("INFERENCE ENGINE", this);
    title->setStyleSheet("color: #D4AF37; font-size: 20px; font-weight: bold; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(title);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_container = new QWidget();
    m_container->setStyleSheet("background: transparent;");
    m_layout = new QVBoxLayout(m_container);
    m_layout->setSpacing(8);
    m_layout->addStretch();
    m_scrollArea->setWidget(m_container);
    mainLay->addWidget(m_scrollArea, 1);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(120);
    connect(m_animTimer, &QTimer::timeout, this, [this](){
        if (m_visibleCount < m_cards.size()) {
            m_cards[m_visibleCount]->setVisible(true);
            QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(m_cards[m_visibleCount]);
            m_cards[m_visibleCount]->setGraphicsEffect(eff);
            QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
            anim->setDuration(400);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            m_visibleCount++;
        } else {
            m_animTimer->stop();
        }
    });
}

void InferenceDisplayWidget::setInsights(const QList<NexusInsight> &insights) {
    m_animTimer->stop(); // Force stop any active reveal sequence
    m_visibleCount = 0;   // Reset counter for the new set of insights
    
    // Clear old
    for (auto *c : m_cards) { 
        m_layout->removeWidget(c); 
        c->deleteLater(); 
    }
    m_cards.clear();

    for (const auto &insight : insights) {
        QFrame *card = createInsightCard(insight);
        card->setVisible(true); // Always visible as requested
        m_layout->insertWidget(m_layout->count() - 1, card);
        m_cards.append(card);
    }
}

QFrame* InferenceDisplayWidget::createInsightCard(const NexusInsight &insight) {
    QFrame *card = new QFrame();
    card->setMinimumHeight(100); // Allow it to grow if text is long

    QColor borderColor;
    QString icon, severityText;
    switch (insight.severity) {
    case NexusInsight::CRITICAL: borderColor = QColor(244, 67, 54); icon = "🔴"; severityText = "CRITICAL"; break;
    case NexusInsight::WARNING:  borderColor = QColor(255, 152, 0); icon = "🟠"; severityText = "WARNING"; break;
    case NexusInsight::INFO:     borderColor = QColor(33, 150, 243); icon = "🔵"; severityText = "INFO"; break;
    case NexusInsight::INSIGHT:  borderColor = QColor(255, 215, 0); icon = "🟡"; severityText = "INSIGHT"; break;
    }

    card->setStyleSheet(QString(
        "QFrame { background: rgba(50, 40, 30, 0.85); border-left: 5px solid %1; "
        "border-radius: 8px; border-top: 1px solid rgba(139,111,71,0.3); "
        "border-right: 1px solid rgba(139,111,71,0.2); border-bottom: 1px solid rgba(139,111,71,0.2); }")
        .arg(borderColor.name()));

    QHBoxLayout *lay = new QHBoxLayout(card);
    lay->setContentsMargins(12, 8, 12, 8);

    // Icon
    QLabel *iconLbl = new QLabel(icon, card);
    iconLbl->setFixedSize(30, 30);
    iconLbl->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    lay->addWidget(iconLbl);

    // Content
    QVBoxLayout *contentLay = new QVBoxLayout();
    contentLay->setSpacing(2);

    QLabel *titleLbl = new QLabel(QString("[%1] %2").arg(severityText, insight.title), card);
    titleLbl->setStyleSheet("color: white; font-weight: bold; font-size: 13px; background: transparent; border: none;");
    contentLay->addWidget(titleLbl);

    QLabel *descLbl = new QLabel(insight.explanation, card);
    descLbl->setWordWrap(true);
    descLbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    descLbl->setStyleSheet("color: #C0B090; font-size: 11px; background: transparent; border: none; line-height: 1.25;");
    contentLay->addWidget(descLbl);

    QLabel *confLbl = new QLabel(QString("Confidence: %1%").arg(insight.confidence), card);
    confLbl->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; background: transparent; border: none;").arg(borderColor.name()));
    contentLay->addWidget(confLbl);

    lay->addLayout(contentLay, 1);

    return card;
}

// ============================================================================
// INTELLIGENCE REPORT WIDGET
// ============================================================================
IntelligenceReportWidget::IntelligenceReportWidget(QWidget *parent) : QWidget(parent),
    m_charIndex(0), m_engine(nullptr)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(15, 15, 15, 15);

    m_reportView = new QTextEdit(this);
    m_reportView->setReadOnly(true);
    m_reportView->setStyleSheet(
        "QTextEdit { background: rgba(20, 15, 8, 0.9); color: #E0D0B0; font-family: 'Consolas', monospace; "
        "font-size: 13px; border: 2px solid #8B6F47; border-radius: 12px; padding: 15px; }");
    mainLay->addWidget(m_reportView, 1);

    QHBoxLayout *btnLay = new QHBoxLayout();
    m_regenerateBtn = new QPushButton("Regenerate Report", this);
    m_regenerateBtn->setStyleSheet(
        "QPushButton { background: #8B6F47; color: white; border-radius: 16px; padding: 10px 24px; font-weight: bold; }"
        "QPushButton:hover { background: #A0825A; }");
    m_regenerateBtn->setCursor(Qt::PointingHandCursor);
    btnLay->addWidget(m_regenerateBtn);

    m_exportBtn = new QPushButton("Export as PDF", this);
    m_exportBtn->setStyleSheet(
        "QPushButton { background: #5A4A32; color: white; border-radius: 16px; padding: 10px 24px; font-weight: bold; }"
        "QPushButton:hover { background: #8B6F47; }");
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    btnLay->addWidget(m_exportBtn);
    btnLay->addStretch();
    mainLay->addLayout(btnLay);

    m_typewriterTimer = new QTimer(this);
    m_typewriterTimer->setInterval(2);
    connect(m_typewriterTimer, &QTimer::timeout, this, [this](){
        if (m_charIndex < m_fullReport.length()) {
            int batch = qMin(5, m_fullReport.length() - m_charIndex);
            m_charIndex += batch;
            m_reportView->setHtml(m_fullReport.left(m_charIndex));
            QScrollBar *sb = m_reportView->verticalScrollBar();
            sb->setValue(sb->maximum());
        } else {
            m_typewriterTimer->stop();
        }
    });

    connect(m_regenerateBtn, &QPushButton::clicked, this, [this](){
        if (m_engine) {
            // Correctly find the NexusWidget up the hierarchy (Report is in Stack, Stack is in Nexus)
            QWidget *p = this->parentWidget();
            NexusWidget *parentNexus = nullptr;
            while (p) {
                parentNexus = qobject_cast<NexusWidget*>(p);
                if (parentNexus) break;
                p = p->parentWidget();
            }

            if (parentNexus) parentNexus->runAiReport();
            else generateReport(m_engine); 
        }
    });

    connect(m_exportBtn, &QPushButton::clicked, this, [this](){
        QString filename = QFileDialog::getSaveFileName(this, "Export Report", "nexus_report.pdf", "PDF (*.pdf)");
        if (!filename.isEmpty()) {
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(filename);
            QTextDocument doc;
            doc.setHtml(m_fullReport);
            doc.print(&printer);
        }
    });
}

void IntelligenceReportWidget::generateReport(InferenceEngine *engine) {
    m_engine = engine;
    m_fullReport = buildReportText();
    m_charIndex = 0;
    m_reportView->clear();
    m_typewriterTimer->start();
}

QString IntelligenceReportWidget::buildReportText() {
    if (!m_engine) return "";
    QString r;
    QDateTime now = QDateTime::currentDateTime();

    r += "<h1 style='color:#D4AF37; text-align:center;'>WORKSHOP INTELLIGENCE REPORT</h1>";
    r += QString("<p style='color:#B8925A; text-align:center;'>Generated: %1<br>"
        "Analysis confidence: %2%<br>"
        "Data points analyzed: %3</p>").arg(now.toString("yyyy-MM-dd HH:mm:ss")).arg(m_engine->overallConfidence()).arg(m_engine->dataPointsAnalyzed());

    r += "<hr style='border-color:#8B6F47;'>";

    // Executive Summary
    r += "<h2 style='color:#D4AF37;'>A — EXECUTIVE SUMMARY</h2>";
    double health = m_engine->healthScore();
    QString worstFinding = "No critical issues detected.";
    QString bestFinding = "Workshop operating normally.";
    for (const auto &i : m_engine->insights()) {
        if (i.severity == NexusInsight::CRITICAL) { worstFinding = i.explanation; break; }
    }
    for (int idx = m_engine->insights().size() - 1; idx >= 0; --idx) {
        if (m_engine->insights()[idx].severity == NexusInsight::INSIGHT) {
            bestFinding = m_engine->insights()[idx].explanation;
            break;
        }
    }
    r += QString("<p style='color:#E0D0B0;'>The workshop currently operates at <b>%1%</b> optimal efficiency. "
        "The primary concern is: <i>%2</i>. The strongest asset: <i>%3</i>.</p>")
        .arg(health, 0, 'f', 0).arg(worstFinding).arg(bestFinding);

    // Critical Findings
    r += "<h2 style='color:#F44336;'>B — CRITICAL FINDINGS</h2>";
    bool hasCritical = false;
    for (const auto &i : m_engine->insights()) {
        if (i.severity == NexusInsight::CRITICAL || i.severity == NexusInsight::WARNING) {
            r += QString("<p style='color:#E0D0B0;'><b style='color:%1;'>[%2]</b> %3 — %4</p>")
                .arg(i.severity == NexusInsight::CRITICAL ? "#F44336" : "#FF9800")
                .arg(i.severity == NexusInsight::CRITICAL ? "CRITICAL" : "WARNING")
                .arg(i.title, i.explanation);
            hasCritical = true;
        }
    }
    if (!hasCritical) r += "<p style='color:#4CAF50;'>No critical findings. Workshop is in good condition.</p>";

    // Hidden Discoveries
    r += "<h2 style='color:#FFD700;'>C — HIDDEN DISCOVERIES</h2>";
    int discNum = 1;
    for (const auto &i : m_engine->insights()) {
        if (i.severity == NexusInsight::INSIGHT) {
            r += QString("<p style='color:#E0D0B0;'>%1. %2 — %3</p>").arg(discNum++).arg(i.title, i.explanation);
        }
    }

    // Workforce Analysis
    r += "<h2 style='color:#D4AF37;'>D — WORKFORCE ANALYSIS</h2>";
    auto employees = loadAllEmployees();
    auto equipment = loadAllEquipment();
    for (const auto &emp : employees) {
        int count = 0;
        QMap<QString, int> types;
        for (const auto &eq : equipment) {
            if (eq.employeeId == emp.id) { count++; types[eq.type]++; }
        }
        if (count > 0) {
            QString mainType = types.isEmpty() ? "various" : types.begin().key();
            r += QString("<p style='color:#E0D0B0;'>%1 %2 has been involved with %3 equipment, primarily with %4 equipment.</p>")
                .arg(emp.firstName, emp.lastName).arg(count).arg(mainType);
        }
    }

    // Temporal Analysis
    r += "<h2 style='color:#D4AF37;'>E — TEMPORAL ANALYSIS</h2>";
    QMap<QString, int> yearCounts;
    for (const auto &eq : equipment) {
        if (eq.purchaseDate.isValid()) yearCounts[eq.purchaseDate.toString("yyyy")]++;
    }
    for (auto it = yearCounts.begin(); it != yearCounts.end(); ++it) {
        r += QString("<p style='color:#E0D0B0;'>Year %1: %2 equipment acquired.</p>").arg(it.key()).arg(it.value());
    }

    // Strategic Conclusion
    r += "<h2 style='color:#D4AF37;'>F — STRATEGIC CONCLUSION</h2>";
    r += "<p style='color:#E0D0B0;'>Based on the analysis, the three most important actions are:</p>";
    r += "<ol style='color:#E0D0B0;'>";
    int actionCount = 0;
    for (const auto &i : m_engine->insights()) {
        if (actionCount >= 3) break;
        if (i.severity <= NexusInsight::WARNING) {
            r += QString("<li>%1: %2</li>").arg(i.title, i.explanation);
            actionCount++;
        }
    }
    if (actionCount == 0) r += "<li>Continue current maintenance schedule.</li>";
    r += "</ol>";

    return r;
}

// ============================================================================
// PATTERN ARCHAEOLOGY WIDGET
// ============================================================================
PatternArchaeologyWidget::PatternArchaeologyWidget(QWidget *parent) : QWidget(parent),
    m_revealedLayers(0), m_revealProgress(0), m_expandedLayer(-1)
{
    m_revealTimer = new QTimer(this);
    m_revealTimer->setInterval(16);
    connect(m_revealTimer, &QTimer::timeout, this, [this](){
        m_revealProgress += 0.02;
        if (m_revealProgress >= 1.0) {
            m_revealProgress = 0;
            m_revealedLayers++;
            if (m_revealedLayers >= m_layers.size()) m_revealTimer->stop();
        }
        update();
    });
}

void PatternArchaeologyWidget::loadData(InferenceEngine *engine) {
    m_layers.clear();

    // Layer 1: Surface
    Layer l1;
    l1.title = "SURFACE"; l1.subtitle = "What happened";
    l1.color = QColor(212, 175, 55);
    auto equip = loadAllEquipment();
    int count = 0;
    for (const auto &e : equip) {
        if (count >= 10) break;
        l1.entries.append(QString("[%1] %2 — Status: %3").arg(e.id).arg(e.type, e.status));
        count++;
    }
    l1.expanded = false;
    m_layers.append(l1);

    // Layer 2: Patterns
    Layer l2;
    l2.title = "PATTERNS"; l2.subtitle = "Recurring behaviors detected";
    l2.color = QColor(184, 146, 90);
    QMap<QString, int> typeCounts;
    for (const auto &e : equip) typeCounts[e.type]++;
    for (auto it = typeCounts.begin(); it != typeCounts.end(); ++it) {
        l2.entries.append(QString("%1: %2 units").arg(it.key()).arg(it.value()));
    }
    l2.expanded = false;
    m_layers.append(l2);

    // Layer 3: Correlations
    Layer l3;
    l3.title = "CORRELATIONS"; l3.subtitle = "Relationships between events";
    l3.color = QColor(139, 111, 71);
    QMap<int, int> empEquipCount;
    for (const auto &e : equip) {
        if (e.employeeId > 0) empEquipCount[e.employeeId]++;
    }
    auto employees = loadAllEmployees();
    for (auto it = empEquipCount.begin(); it != empEquipCount.end(); ++it) {
        for (const auto &emp : employees) {
            if (emp.id == it.key()) {
                l3.entries.append(QString("%1 %2 ↔ %3 equipment").arg(emp.firstName, emp.lastName).arg(it.value()));
                break;
            }
        }
    }
    l3.expanded = false;
    m_layers.append(l3);

    // Layer 4: Causations
    Layer l4;
    l4.title = "CAUSATIONS"; l4.subtitle = "Why things happen";
    l4.color = QColor(90, 74, 50);
    if (engine) {
        for (const auto &i : engine->insights()) {
            if (i.severity <= NexusInsight::WARNING) {
                l4.entries.append(i.explanation);
                if (l4.entries.size() >= 5) break;
            }
        }
    }
    l4.expanded = false;
    m_layers.append(l4);

    // Layer 5: Bedrock
    Layer l5;
    l5.title = "BEDROCK"; l5.subtitle = "Fundamental workshop truths";
    l5.color = QColor(50, 40, 28);
    if (engine) {
        for (const auto &i : engine->insights()) {
            if (i.severity == NexusInsight::INSIGHT && i.confidence >= 70) {
                l5.entries.append(i.explanation);
                if (l5.entries.size() >= 3) break;
            }
        }
    }
    l5.expanded = false;
    m_layers.append(l5);

    m_revealedLayers = 0;
    m_revealProgress = 0;
    m_revealTimer->start();
}

void PatternArchaeologyWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(20, 15, 8));
    bg.setColorAt(1, QColor(35, 25, 15));
    p.fillRect(rect(), bg);

    if (m_layers.isEmpty()) return;

    int totalLayers = m_layers.size();
    int layerH = (m_expandedLayer >= 0) ? 50 : (height() - 40) / totalLayers;
    int expandedH = height() - 40 - (totalLayers - 1) * 50;
    int y = 20;

    for (int i = totalLayers - 1; i >= 0; --i) {
        if (i > m_revealedLayers) continue;

        int h = (m_expandedLayer == i) ? expandedH : layerH;
        qreal layerOpacity = 1.0;
        if (i == m_revealedLayers) layerOpacity = m_revealProgress;

        p.setOpacity(layerOpacity);

        QRectF r(20, y, width() - 40, h);
        m_layers[i].rect = r;

        // Layer background
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        lg.setColorAt(0, m_layers[i].color.lighter(120));
        lg.setColorAt(1, m_layers[i].color.darker(130));
        p.setBrush(lg);
        p.setPen(QPen(m_layers[i].color.lighter(150), 1));
        p.drawRoundedRect(r, 8, 8);

        // Title
        QFont f = p.font();
        f.setPixelSize(14); f.setBold(true);
        p.setFont(f);
        QColor textCol = (i >= 3) ? QColor(255, 215, 0) : Qt::white;
        p.setPen(textCol);
        p.drawText(r.adjusted(15, 5, 0, 0), Qt::AlignTop | Qt::AlignLeft,
            QString("LAYER %1 — %2").arg(i + 1).arg(m_layers[i].title));

        f.setPixelSize(11); f.setBold(false);
        p.setFont(f);
        p.setPen(QColor(200, 190, 170));
        p.drawText(r.adjusted(15, 22, 0, 0), Qt::AlignTop | Qt::AlignLeft, m_layers[i].subtitle);

        // Entries if expanded or enough space
        if (h > 60) {
            int ey = 42;
            f.setPixelSize(11);
            p.setFont(f);
            for (const auto &entry : m_layers[i].entries) {
                if (ey + 16 > h - 5) break;
                p.setPen(textCol);
                p.drawText(r.adjusted(20, ey, -10, 0), Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, "• " + entry);
                ey += 18;
            }
        }

        y += h + 4;
        p.setOpacity(1.0);
    }
}

void PatternArchaeologyWidget::mousePressEvent(QMouseEvent *event) {
    for (int i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i].rect.contains(event->position())) {
            m_expandedLayer = (m_expandedLayer == i) ? -1 : i;
            update();
            return;
        }
    }
}

// ============================================================================
// DECISION MAPPER WIDGET
// ============================================================================
DecisionMapperWidget::DecisionMapperWidget(QWidget *parent) : QWidget(parent),
    m_engine(nullptr), m_rippleProgress(0), m_hasResult(false)
{
    QHBoxLayout *mainLay = new QHBoxLayout(this);
    mainLay->setContentsMargins(15, 15, 15, 15);
    mainLay->setSpacing(15);

    // Left panel
    QFrame *leftPanel = new QFrame(this);
    leftPanel->setFixedWidth(280);
    leftPanel->setStyleSheet("QFrame { background: rgba(50,40,30,0.7); border: 2px solid #8B6F47; border-radius: 12px; }");
    QVBoxLayout *leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(15, 15, 15, 15);
    leftLay->setSpacing(10);

    QLabel *title = new QLabel("Action Selector", leftPanel);
    title->setStyleSheet("color: #D4AF37; font-size: 16px; font-weight: bold; background: transparent; border: none;");
    leftLay->addWidget(title);

    QLabel *lbl1 = new QLabel("Action Type:", leftPanel);
    lbl1->setStyleSheet("color: white; font-size: 12px; background: transparent; border: none;");
    leftLay->addWidget(lbl1);

    m_actionCombo = new QComboBox(leftPanel);
    m_actionCombo->addItems({"Delete equipment", "Change status", "Retire equipment", "Add new (simulate)"});
    m_actionCombo->setStyleSheet("QComboBox { background: white; border: 1px solid #8B6F47; border-radius: 6px; padding: 5px; }");
    leftLay->addWidget(m_actionCombo);

    QLabel *lbl2 = new QLabel("Equipment:", leftPanel);
    lbl2->setStyleSheet("color: white; font-size: 12px; background: transparent; border: none;");
    leftLay->addWidget(lbl2);

    m_equipCombo = new QComboBox(leftPanel);
    m_equipCombo->setStyleSheet("QComboBox { background: white; border: 1px solid #8B6F47; border-radius: 6px; padding: 5px; }");
    leftLay->addWidget(m_equipCombo);

    QLabel *lbl3 = new QLabel("New Status:", leftPanel);
    lbl3->setStyleSheet("color: white; font-size: 12px; background: transparent; border: none;");
    leftLay->addWidget(lbl3);

    m_statusCombo = new QComboBox(leftPanel);
    m_statusCombo->addItems({"Available", "In Use", "Under Maintenance", "Retired"});
    m_statusCombo->setStyleSheet("QComboBox { background: white; border: 1px solid #8B6F47; border-radius: 6px; padding: 5px; }");
    leftLay->addWidget(m_statusCombo);

    m_analyzeBtn = new QPushButton("ANALYZE CONSEQUENCES", leftPanel);
    m_analyzeBtn->setStyleSheet(
        "QPushButton { background: #D4AF37; color: #1A140A; border-radius: 16px; padding: 12px; font-weight: bold; font-size: 13px; border: none; }"
        "QPushButton:hover { background: #E0C060; }");
    m_analyzeBtn->setCursor(Qt::PointingHandCursor);
    leftLay->addWidget(m_analyzeBtn);
    leftLay->addStretch();

    mainLay->addWidget(leftPanel);

    // Right panel
    QVBoxLayout *rightLay = new QVBoxLayout();

    // Result area ON TOP (as requested)
    m_resultArea = new QScrollArea(this);
    m_resultArea->setWidgetResizable(true);
    m_resultArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_resultContainer = new QWidget();
    m_resultContainer->setStyleSheet("background: transparent;");
    new QVBoxLayout(m_resultContainer); // CRITICAL: Initialize layout to prevent crash
    m_resultArea->setWidget(m_resultContainer);
    rightLay->addWidget(m_resultArea, 1);

    // Ripple visualization moved to BOTTOM and minimized
    m_rippleWidget = new QWidget(this);
    m_rippleWidget->setFixedHeight(120);
    m_rippleWidget->setStyleSheet("background: transparent; border-top: 1px solid #5A4A32;");
    rightLay->addWidget(m_rippleWidget);

    m_rippleWidget->installEventFilter(this);

    mainLay->addLayout(rightLay, 1);

    connect(m_analyzeBtn, &QPushButton::clicked, this, [this](){ analyzeConsequences(); });
}

void DecisionMapperWidget::loadData(InferenceEngine *engine) {
    m_engine = engine;
    m_equipCombo->clear();
    auto equip = loadAllEquipment();
    for (const auto &e : equip) {
        m_equipCombo->addItem(QString("[%1] %2").arg(e.id).arg(e.type), e.id);
    }
}

void DecisionMapperWidget::analyzeConsequences() {
    if (!m_engine) return;
    auto equip = loadAllEquipment();
    int selectedId = m_equipCombo->currentData().toInt();
    QString action = m_actionCombo->currentText();
    QString newStatus = m_statusCombo->currentText();

    ConsequenceResult result;
    result.equipCountBefore = equip.size();
    result.valueBefore = 0;
    int goodBefore = 0;
    for (const auto &e : equip) {
        result.valueBefore += e.unitPrice;
        if (e.status == "Available" || e.status == "In Use") goodBefore++;
    }
    result.healthBefore = equip.isEmpty() ? 100 : (double)goodBefore / equip.size() * 100;

    // Simulate action
    QList<NexusEquipment> simulated = equip;
    if (action.startsWith("Delete")) {
        for (int i = 0; i < simulated.size(); ++i) {
            if (simulated[i].id == selectedId) { simulated.removeAt(i); break; }
        }
        result.equipCountAfter = simulated.size();
    } else if (action.startsWith("Retire")) {
        for (auto &e : simulated) { if (e.id == selectedId) e.status = "Retired"; }
        result.equipCountAfter = simulated.size();
    } else if (action.startsWith("Change")) {
        for (auto &e : simulated) { if (e.id == selectedId) e.status = newStatus; }
        result.equipCountAfter = simulated.size();
    } else {
        result.equipCountAfter = simulated.size() + 1;
    }

    result.valueAfter = 0;
    int goodAfter = 0;
    for (const auto &e : simulated) {
        result.valueAfter += e.unitPrice;
        if (e.status == "Available" || e.status == "In Use") goodAfter++;
    }
    result.healthAfter = simulated.isEmpty() ? 100 : (double)goodAfter / simulated.size() * 100;

    // Check risks
    QMap<QString, int> typeCounts;
    for (const auto &e : simulated) {
        if (e.status != "Retired") typeCounts[e.type]++;
    }
    result.singlePointRisk = false;
    for (auto v : typeCounts) { if (v <= 1) { result.singlePointRisk = true; break; } }

    double totalVal = result.valueAfter;
    QList<double> prices;
    for (const auto &e : simulated) prices.append(e.unitPrice);
    std::sort(prices.begin(), prices.end(), std::greater<double>());
    double top3v = 0;
    for (int i = 0; i < qMin(3, (int)prices.size()); ++i) top3v += prices[i];
    result.valueConcentrationRisk = (totalVal > 0 && top3v / totalVal > 0.7);
    result.dependencyRisk = false;

    // Recommendation
    int riskCount = (result.singlePointRisk ? 1 : 0) + (result.valueConcentrationRisk ? 1 : 0);
    bool healthDown = result.healthAfter < result.healthBefore;
    if (riskCount >= 2 || (healthDown && riskCount >= 1)) {
        result.recommendation = "DO NOT PROCEED";
        result.explanation = "This action creates multiple risk factors.";
    } else if (riskCount == 1 || healthDown) {
        result.recommendation = "PROCEED WITH CAUTION";
        result.explanation = "This action has some risk factors to consider.";
    } else {
        result.recommendation = "PROCEED";
        result.explanation = "This action appears safe.";
    }

    m_lastResult = result;
    m_hasResult = true;

    // Trigger AI prediction
    QWidget *p = this->parentWidget();
    NexusWidget *parentNexus = nullptr;
    while (p) {
        parentNexus = qobject_cast<NexusWidget*>(p);
        if (parentNexus) break;
        p = p->parentWidget();
    }
    
    if (parentNexus) {
        parentNexus->runAiDecision(action, selectedId);
    }

    // Animate ripple
    QPropertyAnimation *anim = new QPropertyAnimation(this, "rippleProgress");
    anim->setDuration(1500);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    displayResults(result);
}

void DecisionMapperWidget::displayResults(const ConsequenceResult &result) {
    if (m_resultContainer->layout()) {
        QLayoutItem *child;
        while ((child = m_resultContainer->layout()->takeAt(0)) != nullptr) {
            delete child->widget(); delete child;
        }
        delete m_resultContainer->layout();
    }

    QVBoxLayout *lay = new QVBoxLayout(m_resultContainer);
    lay->setSpacing(10);

    auto addSection = [&](const QString &title, const QString &content, const QColor &col) {
        QFrame *f = new QFrame();
        f->setStyleSheet(QString("QFrame { background: rgba(50,40,30,0.8); border-left: 4px solid %1; border-radius: 8px; }").arg(col.name()));
        QVBoxLayout *fl = new QVBoxLayout(f);
        fl->setContentsMargins(12, 8, 12, 8);
        QLabel *t = new QLabel(title, f);
        t->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 13px; background: transparent; border: none;");
        fl->addWidget(t);
        QLabel *c = new QLabel(content, f);
        c->setWordWrap(true);
        c->setStyleSheet("color: #E0D0B0; font-size: 12px; background: transparent; border: none;");
        fl->addWidget(c);
        lay->addWidget(f);
    };

    // Immediate
    addSection("IMMEDIATE CONSEQUENCES",
        QString("Equipment count: %1 → %2\nWorkshop value: %3 dt → %4 dt\nHealth score: %5% → %6%")
            .arg(result.equipCountBefore).arg(result.equipCountAfter)
            .arg(result.valueBefore, 0, 'f', 0).arg(result.valueAfter, 0, 'f', 0)
            .arg(result.healthBefore, 0, 'f', 1).arg(result.healthAfter, 0, 'f', 1),
        QColor(33, 150, 243));

    // Risks
    QString riskText;
    riskText += QString("Single point of failure: %1\n").arg(result.singlePointRisk ? "⚠️ Risk" : "✅ Safe");
    riskText += QString("Value concentration: %1\n").arg(result.valueConcentrationRisk ? "⚠️ Risk" : "✅ Safe");
    riskText += QString("Dependency risk: %1").arg(result.dependencyRisk ? "⚠️ Risk" : "✅ Safe");
    addSection("RISK ASSESSMENT", riskText, QColor(255, 152, 0));

    // Recommendation
    QColor recCol = result.recommendation == "PROCEED" ? QColor(76,175,80) :
                    result.recommendation == "DO NOT PROCEED" ? QColor(244,67,54) : QColor(255,152,0);
    addSection("NEXUS RECOMMENDATION",
        QString("%1\n%2").arg(result.recommendation, result.explanation), recCol);

    lay->addStretch();
}

void DecisionMapperWidget::displayAiPrediction(const QString &aiText) {
    if (!m_resultContainer->layout()) return;
    
    QFrame *aiFrame = new QFrame();
    aiFrame->setStyleSheet("QFrame { background: rgba(0, 50, 100, 0.4); border: 2px solid #3498db; border-radius: 12px; }");
    QVBoxLayout *ailay = new QVBoxLayout(aiFrame);
    
    QLabel *aiTitle = new QLabel("🤖 NEXUS AI PREDICTION", aiFrame);
    aiTitle->setStyleSheet("color: #3498db; font-weight: bold; font-size: 14px; background: transparent;");
    ailay->addWidget(aiTitle);
    
    QLabel *aiContent = new QLabel(aiText, aiFrame);
    aiContent->setWordWrap(true);
    aiContent->setStyleSheet("color: white; font-style: italic; font-size: 12px; background: transparent;");
    ailay->addWidget(aiContent);
    
    // Insert at top of scroll area results
    static_cast<QVBoxLayout*>(m_resultContainer->layout())->insertWidget(0, aiFrame);
}

// ============================================================================
// MAIN NEXUS WIDGET
// ============================================================================
NexusWidget::NexusWidget(QWidget *parent) : QWidget(parent),
    m_loadingPhase(0), m_loadingProgress(0), m_isLoaded(false)
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 55, 0, 0); // Clear top radio buttons
    mainLay->setSpacing(0);

    // Sub-tab bar (Standardized version)
    QHBoxLayout *tabLayMain = new QHBoxLayout();
    tabLayMain->setSpacing(5);
    tabLayMain->setContentsMargins(15, 5, 15, 5);

    QStringList tabsLabels = {"Graph", "Time", "Report", "Decisions", "Maintenance"};
    QStringList tabsIcons = {":/assets/graph.png", ":/assets/time.png", ":/assets/report.png", ":/assets/predict.png", ":/assets/maintenance.png"};
    
    for (int i = 0; i < tabsLabels.size(); ++i) {
        QPushButton *btn = new QPushButton(tabsLabels[i], this);
        btn->setIcon(QIcon(tabsIcons[i]));
        btn->setIconSize(QSize(18, 18));
        btn->setCheckable(true);
        btn->setFixedHeight(36); // Slightly taller for better icon alignment
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { background: transparent; color: #B8925A; border: 1px solid #5A4A32; "
            "border-radius: 18px; padding: 0 18px; font-weight: bold; font-size: 11px; text-align: left; }"
            "QPushButton:checked { background: #E0C060; color: #1A140A; border-color: #E0C060; }"
            "QPushButton:hover { border: 1px solid #D4AF37; background: rgba(212, 175, 55, 0.15); }");
        if(i==0) btn->setChecked(true);
        connect(btn, &QPushButton::clicked, this, [this, i](){ switchTab(i); });
        m_tabButtons.append(btn);
        tabLayMain->addWidget(btn);
    }
    tabLayMain->addStretch();
    mainLay->insertLayout(0, tabLayMain);

    // Stack
    m_stack = new QStackedWidget(this);
    m_stack->setStyleSheet("background: transparent;");

    m_graphWidget = new KnowledgeGraphWidget(m_stack);
    m_timeMachine = new TimeMachineWidget(m_stack);
    m_inferenceDisplay = new InferenceDisplayWidget(m_stack);
    m_reportWidget = new IntelligenceReportWidget(m_stack);
    m_archaeologyWidget = new PatternArchaeologyWidget(m_stack);
    m_decisionMapper = new DecisionMapperWidget(m_stack);
    m_maintenanceWidget = new MaintenanceOrganismWidget(m_stack);

    m_stack->addWidget(m_graphWidget);       // 0
    m_stack->addWidget(m_timeMachine);       // 1
    m_stack->addWidget(m_reportWidget);      // 2
    m_stack->addWidget(m_decisionMapper);    // 3
    m_stack->addWidget(m_maintenanceWidget); // 4

    mainLay->addWidget(m_stack, 1);

    m_groqApiKey = "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";

    // Engine
    m_engine = new InferenceEngine(this);
    m_networkManager = new QNetworkAccessManager(this);
    m_groqClient = new NexusGroqClient(this);
    m_groqClient->setApiKey(m_groqApiKey);

    connect(m_engine, &InferenceEngine::insightsReady, this, [this](){
        // Generate report and load decision mapper
        m_reportWidget->generateReport(m_engine);
        m_decisionMapper->loadData(m_engine);
        updateStatusBar();
    });

    // Status bar Initialization
    m_statusBar = new QLabel(this);
    m_statusBar->setFixedHeight(30);
    m_statusBar->setStyleSheet(
        "QLabel { background: rgba(20, 15, 8, 0.95); color: #B8925A; font-size: 12px; "
        "font-weight: bold; padding-left: 15px; border-top: 1px solid #5A4A32; }");
    m_statusBar->setText("🧠 Nexus Readiness Alpha... All Systems Active.");
    mainLay->addWidget(m_statusBar);

    // Loading timer
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(16);
    connect(m_loadingTimer, &QTimer::timeout, this, [this](){ runLoadingAnimation(); });
}

void NexusWidget::callGroq(const QString &prompt, std::function<void(QString)> callback) {
    if (m_groqApiKey.isEmpty()) { callback("API Key Missing"); return; }
    
    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_groqApiKey).toUtf8());

    QJsonObject obj;
    obj["model"] = "llama-3.3-70b-versatile";
    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = prompt;
    messages.append(msg);
    obj["messages"] = messages;
    obj["temperature"] = 0.7;

    QNetworkReply *reply = m_networkManager->post(req, QJsonDocument(obj).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, callback](){
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString content = doc.object()["choices"].toArray().at(0).toObject()["message"].toObject()["content"].toString();
            callback(content);
        } else {
            callback(QString("AI Error: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
    });
}

void NexusWidget::runAiReport() {
    m_statusBar->setText("🧠 Nexus AI is drafting a high-fidelity intelligence report...");
    QString prompt = "You are the NEXUS Workshop AI. Analyze this workshop data and write a PROFESSIONAL, STRATEGIC report in HTML format. "
                     "STRICT STYLE GUIDE (Amber/Wood/Premium Theme): "
                     "- Headers (h1, h2): #D4AF37 (Gold) "
                     "- DATA HIGHLIGHT TABLES: Use HTML <table> with width:100% and background:rgba(40,30,20,0.5). "
                     "  Example: <table style='border-left: 5px solid #D4AF37; margin:15px 0; padding:10px;'>...</table> "
                     "- Critical Risk Warnings: Use <div> with background:rgba(211,47,47,0.15) and border:2px solid #D32F2F. "
                     "- Recommended Actions Table: Include a detailed breakdown. "
                     "- Body Text: #E0D0B0 (Premium Creme). "
                     "Structure: Full tactical overview, Health metrics table, Risk matrix, and Future projection. "
                     "Data Summary: ";
    
    auto equips = loadAllEquipment();
    prompt += QString("Total equipments: %1. ").arg(equips.size());
    for(int i=0; i<qMin(15, (int)equips.size()); i++) {
        prompt += QString("[%1: %2, Status: %3, Price: %4]. ").arg(equips[i].id).arg(equips[i].type, equips[i].status).arg(equips[i].unitPrice);
    }

    callGroq(prompt, [this](QString result){
        m_reportWidget->setFullReport(result);
        m_statusBar->setText("✅ AI Intelligence Report Ready.");
    });
}

void NexusWidget::runAiInference() {
    m_statusBar->setText("🧠 Nexus AI is analyzing deep patterns...");
    QString prompt = "Analyze the following workshop equipment data and provide 5 technical tactical insights. "
                     "Return ONLY a JSON array of objects. Example format: [{\"title\":\"X\", \"explanation\":\"Y\", \"severity\":1, \"confidence\":90}]. "
                     "Severity: 0=CRITICAL, 1=WARNING, 2=INFO, 3=INSIGHT. Data: ";
    
    auto equips = loadAllEquipment();
    for(int i=0; i<qMin(40, (int)equips.size()); i++) {
        prompt += QString("[%1: %2, Status: %3]. ").arg(equips[i].id).arg(equips[i].type, equips[i].status);
    }

    callGroq(prompt, [this](QString result){
        // Clean JSON if needed (AI sometimes wraps in ```json)
        if(result.contains("```json")) {
            result = result.split("```json").last().split("```").first().trimmed();
        } else if(result.contains("```")) {
            result = result.split("```").last().split("```").first().trimmed();
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
        if (doc.isArray()) {
            QList<NexusInsight> insights;
            QJsonArray arr = doc.array();
            for (int i = 0; i < arr.size(); ++i) {
                QJsonObject obj = arr[i].toObject();
                NexusInsight in;
                in.title = obj["title"].toString();
                in.explanation = obj["explanation"].toString();
                in.severity = (NexusInsight::Severity)obj["severity"].toInt();
                in.confidence = obj["confidence"].toInt();
                if(in.confidence == 0) in.confidence = 85; // Fallback
                insights.append(in);
            }
            m_inferenceDisplay->setInsights(insights);
            m_statusBar->setText("✅ AI Deep Inference Complete.");
        } else {
            m_statusBar->setText("❌ AI Inference Error: Invalid JSON context.");
        }
    });
}

void NexusWidget::runAiDecision(const QString &action, int equipId) {
    m_statusBar->setText("🧠 Nexus AI is calculating future ripples...");
    QString prompt = QString("Predict the 2-year operational consequence of %1 on equipment ID %2 in a carpentry workshop context. ").arg(action).arg(equipId);
    
    callGroq(prompt, [this](QString result){
        // We can display this in the decision mapper
        m_statusBar->setText("✅ AI Predictive Analysis Complete.");
        // Notify mapper
        m_decisionMapper->displayAiPrediction(result);
    });
}

void IntelligenceReportWidget::setFullReport(const QString &html) {
    m_fullReport = html;
    m_charIndex = 0;
    m_reportView->clear();
    m_typewriterTimer->start();
}

void NexusWidget::switchTab(int index) {
    for (int i = 0; i < m_tabButtons.size(); ++i) {
        m_tabButtons[i]->setChecked(i == index);
    }
    m_stack->setCurrentIndex(index);
}




void NexusWidget::initialize() {
    if (m_isLoaded) {
        // Already loaded — just silently refresh data
        m_graphWidget->loadData();
        m_timeMachine->loadData();
        m_engine->runAllRules();
        m_maintenanceWidget->loadData(m_engine);
        return;
    }
    if (m_loadingTimer->isActive()) {
        // Already initializing — don't restart
        return;
    }
    m_loadingPhase = 0;
    m_loadingProgress = 0;
    m_isLoaded = false;

    // Allow graph to breathe and fill the space
    m_graphWidget->setMinimumHeight(500);

    m_loadingTimer->start();
}

void NexusWidget::runLoadingAnimation() {
    m_loadingProgress += 0.02;
    if (m_loadingProgress >= 1.0) {
        m_loadingTimer->stop();
        m_isLoaded = true;

        // Load all data
        m_engine->loadData();
        m_graphWidget->loadData();
        m_timeMachine->loadData();
        m_engine->runAllRules();
        m_maintenanceWidget->loadData(m_engine);
        m_archaeologyWidget->loadData(m_engine);
        m_decisionMapper->loadData(m_engine);

        m_statusBar->setText(QString("🧠 Nexus Active | %1 insights found | Confidence: %2% | Last analysis: just now")
            .arg(m_engine->insights().size()).arg(m_engine->overallConfidence()));

        emit nexusReady();
    }
    update();
}

void NexusWidget::updateStatusBar() {
    m_statusBar->setText(QString("🧠 Nexus Active | %1 insights found | Confidence: %2% | Health: %3% | Last analysis: just now")
        .arg(m_engine->insights().size()).arg(m_engine->overallConfidence())
        .arg(m_engine->healthScore(), 0, 'f', 1));
}

void NexusWidget::highlightEquipmentInGraph(int equipId) {
    switchTab(0);
    m_graphWidget->highlightEquipment(equipId);
}

void NexusWidget::goToTimeMachineDate(const QDate &date) {
    switchTab(1);
    m_timeMachine->goToDate(date);
}

void NexusWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background gradient
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(20, 15, 8));
    bg.setColorAt(1, QColor(35, 25, 15));
    p.fillRect(rect(), bg);

    // Loading animation
    if (!m_isLoaded) {
        p.save();
        QPointF center = QRectF(rect()).center();

        // Dark overlay
        p.setBrush(QColor(0, 0, 0, (int)(200 * (1.0 - m_loadingProgress))));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());

        // Central point
        QColor white(255, 255, 255, (int)(255 * (1.0 - m_loadingProgress * 0.5)));
        p.setBrush(white);
        p.drawEllipse(center, 4, 4);

        // Extending lines
        int lineCount = qMax(1, (int)(m_loadingProgress * 20));
        for (int i = 0; i < lineCount; ++i) {
            qreal angle = (2.0 * M_PI * i) / 20.0;
            qreal len = m_loadingProgress * 200;
            QPointF end(center.x() + qCos(angle) * len, center.y() + qSin(angle) * len);
            QPen pen(QColor(212, 175, 55, (int)(200 * m_loadingProgress)), 1.5);
            p.setPen(pen);
            p.drawLine(center, end);
        }

        // Progress text
        QFont f = p.font();
        f.setPixelSize(14); f.setBold(true);
        p.setFont(f);
        p.setPen(QColor(212, 175, 55));
        p.drawText(QRectF(0, center.y() + 40, width(), 30), Qt::AlignCenter,
            QString("NEXUS INITIALIZING... %1%").arg((int)(m_loadingProgress * 100)));

        p.restore();
    }
}

// ============================================================================
// NEXUS GROQ CLIENT
// ============================================================================
NexusGroqClient::NexusGroqClient(QObject *parent) : QObject(parent) {
    m_network = new QNetworkAccessManager(this);
}

void NexusGroqClient::sendPrompt(const QString &sys, const QString &usr, QObject* rc, const char* sl) {
    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QJsonObject obj;
    obj["model"] = "llama-3.3-70b-versatile"; // High-fidelity Groq model
    QJsonArray msgs;
    msgs.append(QJsonObject{{"role", "system"}, {"content", sys}});
    msgs.append(QJsonObject{{"role", "user"}, {"content", usr}});
    obj["messages"] = msgs;
    obj["max_tokens"] = 1000;
    obj["temperature"] = 0.7;

    QNetworkReply *rep = m_network->post(req, QJsonDocument(obj).toJson());
    connect(rep, &QNetworkReply::finished, this, [rep, rc, sl](){
        QString res;
        QByteArray data = rep->readAll();
        if (rep->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            res = doc.object()["choices"].toArray().at(0).toObject()["message"].toObject()["content"].toString();
        } else {
            res = QString("AI_ERROR (%1): %2").arg(rep->error()).arg(QString::fromUtf8(data));
        }
        QMetaObject::invokeMethod(rc, sl, Q_ARG(QString, res));
        rep->deleteLater();
    });
}
