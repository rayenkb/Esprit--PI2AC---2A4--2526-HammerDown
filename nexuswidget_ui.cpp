// nexuswidget_ui.cpp — Display widgets + Main NexusWidget container

#include "nexuswidget.h"
#include <QPainterPath>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QDebug>
#include <QtMath>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QScrollArea>
#include <QPixmap>
#include <QUrlQuery>
#include <QButtonGroup>
#include <QRegularExpression>
#include <QThread>
#include <QPointer>
#include <algorithm>
#include <utility>

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
    m_engine(nullptr), m_charIndex(0)
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
// AI FORGE WIDGET
// ============================================================================
AiForgeWidget::AiForgeWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
    buildUi();
    refreshStatsFromDb();
    startEntrance();

    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(16);
    connect(m_tickTimer, &QTimer::timeout, this, &AiForgeWidget::onTick);
    m_tickTimer->start();

    m_typingTimer = new QTimer(this);
    m_typingTimer->setInterval(20);
    connect(m_typingTimer, &QTimer::timeout, this, &AiForgeWidget::updateTypingFrame);

    m_thinkingTimer = new QTimer(this);
    m_thinkingTimer->setInterval(220);
    connect(m_thinkingTimer, &QTimer::timeout, this, &AiForgeWidget::updateThinkingFrame);
}

void AiForgeWidget::buildUi() {
    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_leftPanel = new QWidget(this);
    m_centerPanel = new QWidget(this);
    m_rightPanel = new QWidget(this);
    m_leftPanel->setFixedWidth(260);
    m_rightPanel->setFixedWidth(220);
    m_leftPanel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_centerPanel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_rightPanel->setAttribute(Qt::WA_TranslucentBackground, true);

    root->addWidget(m_leftPanel);
    root->addWidget(m_centerPanel, 1);
    root->addWidget(m_rightPanel);

    // Left sidebar
    QVBoxLayout *leftLay = new QVBoxLayout(m_leftPanel);
    leftLay->setContentsMargins(10, 10, 10, 10);
    leftLay->setSpacing(8);

    QLabel *ctxTitle = new QLabel(QString::fromUtf8("◈ WORKSHOP CONTEXT"), m_leftPanel);
    ctxTitle->setStyleSheet(
        "QLabel{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1E1208,stop:1 #0D0805);"
        "color:#C17F3E;font-weight:800;font-size:10px;letter-spacing:2px;padding:10px;border-bottom:1px solid rgba(193,127,62,64);}"
    );
    leftLay->addWidget(ctxTitle);

    auto createStatBlock = [&](const QString &name, QFrame **box, QLabel **value) {
        QFrame *f = new QFrame(m_leftPanel);
        f->setStyleSheet("QFrame{background:#0D0805;border:1px solid #1E1208;border-radius:8px;}");
        QVBoxLayout *vl = new QVBoxLayout(f);
        vl->setContentsMargins(10, 8, 10, 8);
        vl->setSpacing(3);

        QLabel *nameLbl = new QLabel(name, f);
        nameLbl->setStyleSheet("QLabel{color:rgba(245,230,211,112);font-size:9px;font-weight:700;letter-spacing:1px;}");
        QLabel *valLbl = new QLabel("0", f);
        valLbl->setStyleSheet("QLabel{color:#C17F3E;font-size:26px;font-weight:800;}");

        vl->addWidget(nameLbl);
        vl->addWidget(valLbl);
        *box = f;
        *value = valLbl;
        return f;
    };

    leftLay->addWidget(createStatBlock("HEALTH", &m_healthBox, &m_healthValue));
    leftLay->addWidget(createStatBlock("EQUIPMENT", &m_equipmentBox, &m_equipmentValue));
    leftLay->addWidget(createStatBlock("ACTIVE ALERTS", &m_alertBox, &m_alertValue));
    leftLay->addWidget(createStatBlock("TOTAL VALUE", &m_totalValueBox, &m_totalValueValue));

    QLabel *recentTitle = new QLabel("RECENT ACTIVITY", m_leftPanel);
    recentTitle->setStyleSheet("QLabel{color:#C17F3E;font-size:9px;font-weight:800;letter-spacing:1px;padding:4px 2px;}");
    leftLay->addWidget(recentTitle);

    m_recentHost = new QWidget(m_leftPanel);
    m_recentLayout = new QVBoxLayout(m_recentHost);
    m_recentLayout->setContentsMargins(0, 0, 0, 0);
    m_recentLayout->setSpacing(6);
    m_recentLayout->addStretch(1);
    leftLay->addWidget(m_recentHost);
    leftLay->addStretch(1);

    m_contextStatus = new QLabel(m_leftPanel);
    m_contextStatus->setTextFormat(Qt::RichText);
    m_contextStatus->setStyleSheet("QLabel{background:#0A1A0A;border:1px solid rgba(76,175,125,102);border-radius:11px;padding:5px 8px;font-size:8px;color:#4CAF7D;}");
    leftLay->addWidget(m_contextStatus);

    // Center panel
    QVBoxLayout *centerLay = new QVBoxLayout(m_centerPanel);
    centerLay->setContentsMargins(10, 10, 10, 10);
    centerLay->setSpacing(8);

    QFrame *header = new QFrame(m_centerPanel);
    header->setFixedHeight(56);
    header->setStyleSheet(
        "QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1E1208,stop:1 #0D0805);"
        "border:1px solid rgba(193,127,62,90);border-radius:9px;}"
    );
    QHBoxLayout *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(12, 8, 12, 8);

    QLabel *title = new QLabel("AI FORGE\nWorkshop Intelligence", header);
    title->setStyleSheet("QLabel{color:#F5E6D3;font-size:14px;font-weight:800;}");

    m_statusLabel = new QLabel("Ready", header);
    m_statusLabel->setStyleSheet("QLabel{color:#4CAF7D;background:#0A1A0A;border:1px solid rgba(76,175,125,128);border-radius:10px;padding:4px 10px;font-size:9px;font-weight:800;}");

    headerLay->addWidget(title);
    headerLay->addStretch(1);
    headerLay->addWidget(m_statusLabel);
    centerLay->addWidget(header);

    m_messagesScroll = new QScrollArea(m_centerPanel);
    m_messagesScroll->setWidgetResizable(true);
    m_messagesScroll->setFrameShape(QFrame::NoFrame);
    m_messagesScroll->setStyleSheet(
        "QScrollArea{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(13,8,5,0.8),stop:1 rgba(30,18,8,0.8));border:1px solid rgba(193,127,62,30);border-radius:12px;}"
        "QScrollBar:vertical{width:6px;background:rgba(13,8,5,0.5);border-radius:3px;}"
        "QScrollBar::handle:vertical{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #C17F3E,stop:1 #D4AF37);border-radius:3px;min-height:20px;}"
        "QScrollBar::handle:vertical:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #D4AF37,stop:1 #F59E0B);box-shadow:0px 0px 8px rgba(212,175,55,0.6);}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical{height:0px;}"
    );
    m_messagesHost = new QWidget(m_messagesScroll);
    m_messagesLayout = new QVBoxLayout(m_messagesHost);
    m_messagesLayout->setContentsMargins(6, 6, 6, 6);
    m_messagesLayout->setSpacing(10);
    m_messagesLayout->addStretch(1);
    m_messagesScroll->setWidget(m_messagesHost);
    centerLay->addWidget(m_messagesScroll, 1);

    QFrame *inputFrame = new QFrame(m_centerPanel);
    inputFrame->setFixedHeight(72);
    inputFrame->setStyleSheet(
        "QFrame{"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(30,18,8,0.95),stop:0.5 rgba(21,13,6,0.98),stop:1 rgba(13,8,5,0.95));"
        "border:2px solid rgba(193,127,62,60);border-radius:18px;"
        "box-shadow:0px 4px 20px rgba(0,0,0,0.4), inset 0px 1px 0px rgba(255,255,255,0.1);"
        "}"
    );
    QHBoxLayout *inputLay = new QHBoxLayout(inputFrame);
    inputLay->setContentsMargins(16, 12, 16, 12);
    inputLay->setSpacing(10);

    m_input = new QLineEdit(inputFrame);
    m_input->setPlaceholderText("Ask anything about your workshop...");
    m_input->setStyleSheet(
        "QLineEdit{"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(13,8,5,0.9),stop:1 rgba(21,13,6,0.9));"
        "color:#F5E6D3;border:2px solid rgba(193,127,62,45);border-radius:23px;"
        "padding:0 14px;font-size:13px;selection-background-color:rgba(193,127,62,120);"
        "box-shadow:inset 0px 2px 8px rgba(0,0,0,0.3), 0px 1px 0px rgba(255,255,255,0.1);"
        "}"
        "QLineEdit:focus{"
        "border:2px solid rgba(212,175,55,180);"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(21,13,6,0.95),stop:1 rgba(30,18,8,0.95));"
        "box-shadow:inset 0px 2px 8px rgba(0,0,0,0.4), 0px 0px 12px rgba(212,175,55,0.3);"
        "}"
        "QLineEdit:disabled{color:rgba(245,230,211,128);background:rgba(18,13,8,0.7);}"
    );

    m_sendBtn = new QPushButton("Send  -->", inputFrame);
    m_sendBtn->setFixedSize(100, 46);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton{"
        "color:#FFFFFF;font-weight:800;font-size:13px;border-radius:23px;"
        "border:2px solid rgba(193,127,62,80);"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(232,145,74,0.95),stop:0.3 rgba(193,127,62,0.98),stop:0.7 rgba(139,74,30,0.98),stop:1 rgba(107,68,35,0.95));"
        "box-shadow:0px 4px 15px rgba(0,0,0,0.4), inset 0px 1px 0px rgba(255,255,255,0.2), inset 0px -1px 0px rgba(0,0,0,0.3);"
        "}"
        "QPushButton:hover{"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(245,158,11,0.95),stop:0.3 rgba(232,145,74,0.98),stop:0.7 rgba(193,127,62,0.98),stop:1 rgba(139,74,30,0.95));"
        "border:2px solid rgba(212,175,55,120);"
        "box-shadow:0px 6px 20px rgba(245,158,11,0.4), inset 0px 1px 0px rgba(255,255,255,0.3), inset 0px -1px 0px rgba(0,0,0,0.2);"
        "}"
        "QPushButton:pressed{"
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(160,130,90,0.95),stop:0.3 rgba(139,111,71,0.98),stop:0.7 rgba(107,68,35,0.98),stop:1 rgba(85,52,19,0.95));"
        "border:2px solid rgba(139,111,71,150);"
        "box-shadow:inset 0px 2px 8px rgba(0,0,0,0.5), 0px 1px 0px rgba(255,255,255,0.1);"
        "}"
        "QPushButton:disabled{background:rgba(107,68,35,0.7);color:rgba(160,130,90,0.7);border:2px solid rgba(139,111,71,50);}"
    );

    inputLay->addWidget(m_input, 1);
    inputLay->addWidget(m_sendBtn);
    centerLay->addWidget(inputFrame);

    // Right sidebar
    QVBoxLayout *rightLay = new QVBoxLayout(m_rightPanel);
    rightLay->setContentsMargins(10, 10, 10, 10);
    rightLay->setSpacing(8);

    QLabel *ctrlTitle = new QLabel(QString::fromUtf8("◈ AI CONTROLS"), m_rightPanel);
    ctrlTitle->setStyleSheet(
        "QLabel{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1E1208,stop:1 #0D0805);"
        "color:#C17F3E;font-weight:800;font-size:10px;letter-spacing:2px;padding:10px;border-bottom:1px solid rgba(193,127,62,64);}"
    );
    rightLay->addWidget(ctrlTitle);

    m_modeAdvisorBtn = new QPushButton("⚙  Advisor", m_rightPanel);
    m_modeDeepBtn = new QPushButton("🔎  Deep Analysis", m_rightPanel);
    m_modeBriefBtn = new QPushButton("☀  Daily Brief", m_rightPanel);
    for (QPushButton *b : {m_modeAdvisorBtn, m_modeDeepBtn, m_modeBriefBtn}) {
        b->setFixedHeight(48);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton{background:#1A1208;border:1px solid rgba(193,127,62,90);border-radius:10px;"
            "color:#F5E6D3;font-weight:800;font-size:12px;text-align:left;padding-left:12px;}"
            "QPushButton:hover{background:#1E1508;border:1px solid rgba(193,127,62,153);color:#FFFFFF;}"
        );
        rightLay->addWidget(b);
    }

    QLabel *quickAskLbl = new QLabel("QUICK ASK", m_rightPanel);
    quickAskLbl->setStyleSheet("QLabel{color:#C17F3E;font-size:9px;font-weight:800;letter-spacing:1px;padding:2px 1px;}");
    rightLay->addWidget(quickAskLbl);

    auto makeQuick = [&](const QString &txt) {
        QPushButton *b = new QPushButton(QString("✦  ") + txt, m_rightPanel);
        b->setFixedHeight(42);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton{background:#0D0805;border:1px solid rgba(193,127,62,64);border-radius:8px;"
            "color:#F5E6D3;font-weight:800;font-size:11px;text-align:left;padding-left:10px;}"
            "QPushButton:hover{background:#1A1208;border:1px solid rgba(193,127,62,153);color:#FFFFFF;border-left:2px solid #C17F3E;padding-left:12px;}"
            "QPushButton:pressed{color:#FFD700;}"
        );
        rightLay->addWidget(b);
        return b;
    };

    m_quickFixBtn = makeQuick("What needs fixing now?");
    m_quickHealthBtn = makeQuick("Workshop health summary");
    m_quickRoiBtn = makeQuick("Best ROI equipment?");
    m_quickActionBtn = makeQuick("Today's action plan");

    m_clearBtn = new QPushButton("🗑️ Clear Chat", m_rightPanel);
    m_clearBtn->setFixedHeight(38);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    m_clearBtn->setStyleSheet(
        "QPushButton{background:#1A0808;border:1px solid rgba(204,34,0,102);border-radius:8px;"
        "color:#F5E6D3;font-weight:800;font-size:11px;}"
        "QPushButton:hover{background:#2A0808;border:1px solid rgba(204,34,0,204);color:#FFFFFF;}"
    );
    rightLay->addWidget(m_clearBtn);
    rightLay->addStretch(1);

    connect(m_sendBtn, &QPushButton::clicked, this, &AiForgeWidget::onSend);
    connect(m_input, &QLineEdit::returnPressed, this, &AiForgeWidget::onSend);

    connect(m_modeAdvisorBtn, &QPushButton::clicked, this, [this]() { setMode(Advisor); });
    connect(m_modeDeepBtn, &QPushButton::clicked, this, [this]() {
        setMode(DeepAnalysis);
        sendQuestion("Perform complete deep analysis covering health, top 3 risks, financial overview, and 3 action recommendations.");
    });
    connect(m_modeBriefBtn, &QPushButton::clicked, this, [this]() {
        setMode(DailyBrief);
        sendQuestion("Give today's workshop briefing: health status, most urgent task, who should do what, estimated costs, one thing not to forget.");
    });

    connect(m_quickFixBtn, &QPushButton::clicked, this, [this]() { sendQuestion("What needs fixing now?"); });
    connect(m_quickHealthBtn, &QPushButton::clicked, this, [this]() { sendQuestion("Workshop health summary"); });
    connect(m_quickRoiBtn, &QPushButton::clicked, this, [this]() { sendQuestion("Best ROI equipment?"); });
    connect(m_quickActionBtn, &QPushButton::clicked, this, [this]() { sendQuestion("Today's action plan"); });

    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        m_waitingForAi = false;
        m_pendingQuestion.clear();
        m_activeQuestion.clear();
        updateInteractiveState();
        m_msgs.clear();
        startEntrance();
        m_clearBtn->setText("Cleared ✓");
        m_clearBtn->setStyleSheet(
            "QPushButton{background:#2A1408;border:1px solid rgba(193,127,62,200);border-radius:8px;color:#FFFFFF;font-weight:800;font-size:11px;}"
        );
        QTimer::singleShot(650, this, [this]() {
            m_clearBtn->setText("🗑️ Clear Chat");
            m_clearBtn->setStyleSheet(
                "QPushButton{background:#1A0808;border:1px solid rgba(204,34,0,102);border-radius:8px;color:#F5E6D3;font-weight:800;font-size:11px;}"
                "QPushButton:hover{background:#2A0808;border:1px solid rgba(204,34,0,204);color:#FFFFFF;}"
            );
        });
    });

    updateModeButtons();
    updateInteractiveState();
}

void AiForgeWidget::startEntrance() {
    m_entrance = 1.0f;
    m_totalValueDisplay = 0.0;
    m_pendingQuestion.clear();
    m_activeQuestion.clear();
    m_waitingForAi = false;
    m_typingMessageIndex = -1;
    if (m_typingTimer) m_typingTimer->stop();
    if (m_thinkingTimer) m_thinkingTimer->stop();
    m_thinkingAnimPhase = 0;
    m_msgs.clear();
    clearMessageWidgets();

    Msg welcome;
    welcome.user = false;
    welcome.thinking = false;
    welcome.cps = 50; // 20ms per char
    welcome.ts = QDateTime::currentDateTime();
    welcome.dataPoints = m_dataPoints;
    welcome.text = QString("I have analyzed %1 equipment,\n%2 history events, and %3 active maintenance alerts. "
                           "Your workshop health is at %4%.\n\n"
                           "Ask me anything about your workshop.\n"
                           "I know every tool, every record, every pattern in your data.")
                       .arg(m_equipmentCount)
                       .arg(m_historyCount)
                       .arg(m_activeAlerts)
                       .arg(m_healthScore);
    welcome.shownChars = 0;
    m_msgs.append(welcome);
    appendMessageWidget(m_msgs.size() - 1);
    startTypewriterForLastMessage();
    scrollMessagesToBottom();
    updateInteractiveState();

    // Refresh context silently in background; conversation remains stable.
    refreshStatsFromDb();
}

void AiForgeWidget::setMode(Mode mode) {
    m_mode = mode;
    updateModeButtons();
}

void AiForgeWidget::updateModeButtons() {
    auto styleFor = [](bool active, const QString &accent) {
        if (active) {
            return QString(
                "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2A1A08,stop:.5 #3A2A10,stop:1 #2A1A08);"
                "border:1px solid rgba(193,127,62,204);border-left:4px solid #C17F3E;border-radius:10px;"
                "color:#FFFFFF;font-weight:800;font-size:12px;text-align:left;padding-left:10px;}"
                "QPushButton:hover{border:1px solid %1;border-left:4px solid #C17F3E;color:#FFFFFF;}"
            ).arg(accent);
        }
        return QString(
            "QPushButton{background:#1A1208;border:1px solid rgba(193,127,62,90);border-radius:10px;"
            "color:#F5E6D3;font-weight:800;font-size:12px;text-align:left;padding-left:12px;}"
            "QPushButton:hover{background:#1E1508;border:1px solid rgba(193,127,62,153);color:#FFFFFF;}"
        );
    };

    m_modeAdvisorBtn->setStyleSheet(styleFor(m_mode == Advisor, "rgba(193,127,62,220)"));
    m_modeDeepBtn->setStyleSheet(styleFor(m_mode == DeepAnalysis, "rgba(59,130,246,220)"));
    m_modeBriefBtn->setStyleSheet(styleFor(m_mode == DailyBrief, "rgba(76,175,125,220)"));
}

void AiForgeWidget::updateInteractiveState() {
    bool enabled = !m_waitingForAi;
    m_sendBtn->setEnabled(enabled);
    m_input->setEnabled(enabled);
    m_sendBtn->setCursor(enabled ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
}

void AiForgeWidget::refreshStatsFromDb() {
    if (m_dbRefreshInFlight) {
        m_dbRefreshQueued = true;
        return;
    }
    m_dbRefreshInFlight = true;

    QSqlDatabase baseDb = QSqlDatabase::database();
    const QString driver = baseDb.driverName();
    const QString hostName = baseDb.hostName();
    const int port = baseDb.port();
    const QString databaseName = baseDb.databaseName();
    const QString userName = baseDb.userName();
    const QString password = baseDb.password();
    const QString connectOptions = baseDb.connectOptions();

    QPointer<AiForgeWidget> self(this);

    QThread *worker = QThread::create([self, driver, hostName, port, databaseName, userName, password, connectOptions]() {
        int equipmentCount = 0;
        int historyCount = 0;
        int activeAlerts = 0;
        double totalValue = 0.0;
        int dataPoints = 0;

        int availableCount = 0;
        int inUseCount = 0;
        int maintenanceCount = 0;
        int retiredCount = 0;
        QString urgentMaintenanceEquipment;
        int urgentMaintenanceDays = 0;

        double totalAgeYears = 0.0;
        int oldestAgeYears = 0;

        QStringList equipmentNames;
        QList<AiForgeWidget::ActivityRow> recentActivity;
        QString smartContext;

        struct EqInfo {
            int id = 0;
            QString type;
            QString status;
            double price = 0.0;
            QDate purchaseDate;
            QString description;
            int quantity = 0;
            int ageYears = 0;
            int daysInStatus = 0;
            int eventCount = 0;
            QString valueTier;
        };

        struct HistInfo {
            int equipmentId = -1;
            QString equipmentType;
            QString action;
            QString description;
            QString changedBy;
            QDateTime when;
        };

        QList<EqInfo> equipments;
        QList<HistInfo> histories;
        QMap<int, int> eventCountByEquipment;
        QMap<int, QDateTime> lastEventByEquipment;
        QMap<int, QString> equipTypeById;
        QMap<int, QString> equipNameById;
        QMap<int, double> equipPriceById;
        QMap<QString, int> employeeEvents;
        QMap<QString, QMap<QString, int>> employeeTypeTouches;
        QMap<int, QString> employeeNameById;
        QMap<QString, QString> employeeCanonicalName;
        QStringList alertLines;
        QStringList recentLines;

        int eventsThisMonth = 0;
        int eventsLastMonth = 0;
        QDateTime newestEvent;
        QDateTime oldestEvent;

        QString connName = QString("aiforge_ctx_%1_%2")
            .arg((quintptr)QThread::currentThreadId())
            .arg(QDateTime::currentMSecsSinceEpoch());

        {
            QSqlDatabase db = QSqlDatabase::addDatabase(driver, connName);
            if (!hostName.isEmpty()) db.setHostName(hostName);
            if (port > 0) db.setPort(port);
            db.setDatabaseName(databaseName);
            db.setUserName(userName);
            db.setPassword(password);
            if (!connectOptions.isEmpty()) db.setConnectOptions(connectOptions);

            if (db.open()) {
                QSqlQuery empQ(db);
                if (empQ.exec("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM EMPLOYEES")) {
                    while (empQ.next()) {
                        int id = empQ.value(0).toInt();
                        QString first = empQ.value(1).toString().trimmed();
                        QString last = empQ.value(2).toString().trimmed();
                        QString full = (first + " " + last).trimmed();
                        if (full.isEmpty()) full = QString("Employee #%1").arg(id);
                        employeeNameById[id] = full;
                        employeeCanonicalName[full.toLower()] = full;
                        dataPoints += 3;
                    }
                }

                QSqlQuery eqQ(db);
                if (eqQ.exec("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, STATUS, UNIT_PRICE, PURCHASE_DATE, DESCRIPTION, QUANTITY FROM EQUIPMENT")) {
                    while (eqQ.next()) {
                        EqInfo e;
                        e.id = eqQ.value(0).toInt();
                        e.type = eqQ.value(1).toString().trimmed();
                        e.status = eqQ.value(2).toString().trimmed();
                        e.price = eqQ.value(3).toDouble();
                        e.purchaseDate = eqQ.value(4).toDate();
                        e.description = eqQ.value(5).toString().trimmed();
                        e.quantity = eqQ.value(6).toInt();

                        e.ageYears = e.purchaseDate.isValid() ? qMax(0, e.purchaseDate.daysTo(QDate::currentDate()) / 365) : 0;
                        e.valueTier = e.price >= 2500.0 ? "high" : (e.price >= 1000.0 ? "mid" : "low");

                        equipmentCount++;
                        totalValue += e.price;
                        totalAgeYears += e.ageYears;
                        oldestAgeYears = qMax(oldestAgeYears, e.ageYears);
                        dataPoints += 7;

                        QString s = e.status.toLower();
                        if (s.contains("available")) availableCount++;
                        else if (s.contains("in use") || s.contains("use")) inUseCount++;
                        else if (s.contains("maint")) maintenanceCount++;
                        else if (s.contains("retir")) retiredCount++;

                        QString equipName = QString("%1 #%2").arg(e.type.isEmpty() ? QString("Equipment") : e.type).arg(e.id);
                        equipTypeById[e.id] = e.type;
                        equipNameById[e.id] = equipName;
                        equipPriceById[e.id] = e.price;

                        if (!e.type.isEmpty()) equipmentNames.append(e.type);
                        equipmentNames.append(equipName);

                        equipments.append(e);
                    }
                }

                QSqlQuery histQ(db);
                bool ok = histQ.exec("SELECT * FROM HISTORY ORDER BY CHANGE_DATE DESC FETCH FIRST 50 ROWS ONLY");
                if (!ok) {
                    ok = histQ.exec("SELECT * FROM history ORDER BY change_date DESC LIMIT 50");
                }
                if (ok) {
                    const QDate today = QDate::currentDate();
                    const QDate lastMonthRef = today.addMonths(-1);

                    while (histQ.next()) {
                        historyCount++;
                        dataPoints++;

                        QSqlRecord rec = histQ.record();
                        int equipmentIdx = rec.indexOf("EQUIPMENT_ID");
                        if (equipmentIdx < 0) equipmentIdx = rec.indexOf("equipment_id");
                        int actionIdx = rec.indexOf("ACTION_TYPE");
                        if (actionIdx < 0) actionIdx = rec.indexOf("action_type");
                        int descIdx = rec.indexOf("DESCRIPTION");
                        if (descIdx < 0) descIdx = rec.indexOf("description");
                        int dateIdx = rec.indexOf("CHANGE_DATE");
                        if (dateIdx < 0) dateIdx = rec.indexOf("change_date");
                        int changedIdx = rec.indexOf("CHANGED_BY");
                        if (changedIdx < 0) changedIdx = rec.indexOf("changed_by");

                        int equipmentId = equipmentIdx >= 0 ? histQ.value(equipmentIdx).toInt() : -1;
                        QString action = actionIdx >= 0 ? histQ.value(actionIdx).toString().trimmed() : QString("UPDATED");
                        QString description = descIdx >= 0 ? histQ.value(descIdx).toString().trimmed() : action;
                        QVariant dateVar = dateIdx >= 0 ? histQ.value(dateIdx) : QVariant();
                        QDateTime when = dateVar.toDateTime();
                        if (!when.isValid()) {
                            when = QDateTime::fromString(dateVar.toString(), Qt::ISODate);
                        }
                        QString changedBy = changedIdx >= 0 ? histQ.value(changedIdx).toString().trimmed() : QString();

                        QString actor = changedBy;
                        bool idOk = false;
                        int actorId = actor.toInt(&idOk);
                        if (idOk && employeeNameById.contains(actorId)) {
                            actor = employeeNameById.value(actorId);
                        } else if (!actor.isEmpty()) {
                            QString key = actor.toLower();
                            actor = employeeCanonicalName.contains(key) ? employeeCanonicalName.value(key) : actor;
                        }
                        if (actor.isEmpty()) actor = "Unknown";

                        QString equipType = equipTypeById.value(equipmentId, QString("Equipment"));
                        QString equipName = equipNameById.value(equipmentId, equipmentId > 0 ? QString("Equipment #%1").arg(equipmentId) : QString("Workshop"));

                        HistInfo h;
                        h.equipmentId = equipmentId;
                        h.equipmentType = equipType;
                        h.action = action;
                        h.description = description;
                        h.changedBy = actor;
                        h.when = when;
                        histories.append(h);

                        eventCountByEquipment[equipmentId]++;
                        employeeEvents[actor]++;
                        employeeTypeTouches[actor][equipType]++;

                        if (!lastEventByEquipment.contains(equipmentId) && when.isValid()) {
                            lastEventByEquipment[equipmentId] = when;
                        }

                        if (when.isValid()) {
                            if (!newestEvent.isValid() || when > newestEvent) newestEvent = when;
                            if (!oldestEvent.isValid() || when < oldestEvent) oldestEvent = when;
                            QDate d = when.date();
                            if (d.year() == today.year() && d.month() == today.month()) eventsThisMonth++;
                            if (d.year() == lastMonthRef.year() && d.month() == lastMonthRef.month()) eventsLastMonth++;
                        }

                        if (action.compare("ORGANISM_BORN", Qt::CaseInsensitive) == 0) {
                            activeAlerts++;
                            QString severity = "medium";
                            QString descLower = description.toLower();
                            if (descLower.contains("critical") || descLower.contains("urgent")) severity = "critical";
                            else if (descLower.contains("high")) severity = "high";
                            int days = when.isValid() ? qMax(0, when.daysTo(QDateTime::currentDateTime())) : 0;
                            alertLines.append(QString("- %1: %2, %3 days").arg(equipName, severity).arg(days));
                        }

                        if (recentActivity.size() < 5) {
                            AiForgeWidget::ActivityRow row;
                            row.equipment = equipName;
                            row.action = description.isEmpty() ? action : description;
                            row.at = when.isValid() ? when.toString("HH:mm") : QString("--:--");
                            if (action.contains("ADD", Qt::CaseInsensitive) || action.contains("NEW", Qt::CaseInsensitive)) row.color = QColor("#4CAF7D");
                            else if (action.contains("MAINT", Qt::CaseInsensitive) || action.contains("ORGANISM", Qt::CaseInsensitive)) row.color = QColor("#F59E0B");
                            else row.color = QColor("#C17F3E");
                            recentActivity.append(row);
                        }

                        if (recentLines.size() < 10) {
                            QString whenText = when.isValid() ? when.toString("yyyy-MM-dd HH:mm") : QString("unknown date");
                            recentLines.append(QString("- %1: %2 %3 by %4").arg(whenText, equipName, action, actor));
                        }
                    }
                }
            }

            db.close();
        }
        QSqlDatabase::removeDatabase(connName);

        for (EqInfo &e : equipments) {
            e.eventCount = eventCountByEquipment.value(e.id, 0);
            if (lastEventByEquipment.contains(e.id)) {
                e.daysInStatus = qMax(0, lastEventByEquipment.value(e.id).daysTo(QDateTime::currentDateTime()));
            } else {
                e.daysInStatus = e.purchaseDate.isValid() ? qMax(0, e.purchaseDate.daysTo(QDate::currentDate())) : 0;
            }
            if (e.status.contains("maint", Qt::CaseInsensitive) && e.daysInStatus >= urgentMaintenanceDays) {
                urgentMaintenanceDays = e.daysInStatus;
                urgentMaintenanceEquipment = QString("%1 #%2")
                    .arg(e.type.isEmpty() ? QString("Equipment") : e.type)
                    .arg(e.id);
            }
        }

        int workshopHealth = equipmentCount > 0
            ? int((double(availableCount) / double(equipmentCount)) * 100.0)
            : 0;
        double avgEquipmentAge = equipmentCount > 0 ? (totalAgeYears / double(equipmentCount)) : 0.0;

        int mostUsedId = -1;
        int mostUsedEvents = -1;
        for (auto it = eventCountByEquipment.begin(); it != eventCountByEquipment.end(); ++it) {
            if (it.value() > mostUsedEvents) {
                mostUsedEvents = it.value();
                mostUsedId = it.key();
            }
        }

        QString mostActiveEmployee = "Unknown";
        int mostActiveCount = -1;
        for (auto it = employeeEvents.begin(); it != employeeEvents.end(); ++it) {
            if (it.value() > mostActiveCount) {
                mostActiveCount = it.value();
                mostActiveEmployee = it.key();
            }
        }

        int monthsSpan = 1;
        if (newestEvent.isValid() && oldestEvent.isValid()) {
            monthsSpan = qMax(1,
                (newestEvent.date().year() - oldestEvent.date().year()) * 12
                + (newestEvent.date().month() - oldestEvent.date().month()) + 1);
        }
        double avgEventsPerMonth = monthsSpan > 0 ? (double(historyCount) / double(monthsSpan)) : 0.0;

        EqInfo mostExpensive;
        EqInfo oldestEquipment;
        EqInfo newestEquipment;
        bool firstEq = true;
        for (const EqInfo &e : std::as_const(equipments)) {
            if (firstEq || e.price > mostExpensive.price) mostExpensive = e;
            if (firstEq || e.ageYears > oldestEquipment.ageYears) oldestEquipment = e;
            if (firstEq || (newestEquipment.purchaseDate.isValid() ? e.purchaseDate > newestEquipment.purchaseDate : e.purchaseDate.isValid())) newestEquipment = e;
            firstEq = false;
        }

        EqInfo longestMaintenance;
        bool hasLongest = false;
        for (const EqInfo &e : std::as_const(equipments)) {
            if (e.status.contains("maint", Qt::CaseInsensitive)) {
                if (!hasLongest || e.daysInStatus > longestMaintenance.daysInStatus) {
                    longestMaintenance = e;
                    hasLongest = true;
                }
            }
        }

        QStringList lines;
        lines << "WORKSHOP OVERVIEW:";
        auto pct = [&](int n) { return equipmentCount > 0 ? (double(n) * 100.0 / double(equipmentCount)) : 0.0; };
        lines << QString(" Total equipment: %1").arg(equipmentCount);
        lines << QString(" Available: %1 (%2%)").arg(availableCount).arg(QString::number(pct(availableCount), 'f', 1));
        lines << QString(" In Use: %1 (%2%)").arg(inUseCount).arg(QString::number(pct(inUseCount), 'f', 1));
        lines << QString(" Maintenance: %1 (%2%)").arg(maintenanceCount).arg(QString::number(pct(maintenanceCount), 'f', 1));
        lines << QString(" Retired: %1 (%2%)").arg(retiredCount).arg(QString::number(pct(retiredCount), 'f', 1));
        lines << QString(" Total value: %1dt").arg(QString::number(totalValue, 'f', 2));
        lines << QString(" Health score: %1%").arg(workshopHealth);
        lines << QString(" Workshop age: %1 years").arg(oldestAgeYears);
        lines << "";

        lines << "EQUIPMENT LIST:";
        for (const EqInfo &e : std::as_const(equipments)) {
            QString equipName = equipNameById.value(e.id, QString("Equipment #%1").arg(e.id));
            QString veteran = e.ageYears > 15 ? "veteran" : "standard";
            lines << QString(" - [%1] %2: %3, %4dt, %5yr old, %6 days in status, %7 interactions, qty %8, %9, %10")
                        .arg(e.id)
                        .arg(equipName)
                        .arg(e.status)
                        .arg(QString::number(e.price, 'f', 2))
                        .arg(e.ageYears)
                        .arg(e.daysInStatus)
                        .arg(e.eventCount)
                        .arg(e.quantity)
                        .arg(veteran)
                        .arg(e.valueTier);
        }
        lines << "";

        lines << "MAINTENANCE ALERTS:";
        if (alertLines.isEmpty()) lines << " - none";
        else lines << alertLines;
        lines << "";

        lines << "RECENT ACTIVITY (last 10 events):";
        if (recentLines.isEmpty()) lines << " - none";
        else lines << recentLines;
        lines << "";

        lines << "EMPLOYEES:";
        if (employeeNameById.isEmpty()) {
            lines << " - none";
        } else {
            for (auto it = employeeNameById.begin(); it != employeeNameById.end(); ++it) {
                const QString emp = it.value();
                int interactions = employeeEvents.value(emp, 0);
                QString topType = "none";
                int topCount = -1;
                auto touches = employeeTypeTouches.value(emp);
                for (auto tt = touches.begin(); tt != touches.end(); ++tt) {
                    if (tt.value() > topCount) {
                        topCount = tt.value();
                        topType = tt.key();
                    }
                }
                lines << QString(" - %1: %2 total interactions, most active with %3")
                            .arg(emp)
                            .arg(interactions)
                            .arg(topType);
            }
        }
        lines << "";

        lines << "KEY INSIGHTS:";
        lines << QString(" - Most valuable: %1 at %2dt")
                    .arg(mostExpensive.type.isEmpty() ? QString("unknown") : QString("%1 #%2").arg(mostExpensive.type).arg(mostExpensive.id))
                    .arg(QString::number(mostExpensive.price, 'f', 2));
        lines << QString(" - Most used: %1 with %2 events")
                    .arg(mostUsedId > 0 ? equipNameById.value(mostUsedId, QString("Equipment #%1").arg(mostUsedId)) : QString("unknown"))
                    .arg(qMax(0, mostUsedEvents));
        lines << QString(" - Longest maintenance: %1 %2 days")
                    .arg(hasLongest ? QString("%1 #%2").arg(longestMaintenance.type).arg(longestMaintenance.id) : QString("none"))
                    .arg(hasLongest ? QString::number(longestMaintenance.daysInStatus) : QString("0"));
        lines << QString(" - Newest equipment: %1 %2")
                    .arg(newestEquipment.type.isEmpty() ? QString("unknown") : QString("%1 #%2").arg(newestEquipment.type).arg(newestEquipment.id))
                    .arg(newestEquipment.purchaseDate.isValid() ? newestEquipment.purchaseDate.toString("yyyy-MM-dd") : QString("unknown"));
        lines << QString(" - Oldest equipment: %1 %2 years")
                    .arg(oldestEquipment.type.isEmpty() ? QString("unknown") : QString("%1 #%2").arg(oldestEquipment.type).arg(oldestEquipment.id))
                    .arg(oldestEquipment.ageYears);
        lines << QString(" - Most modified equipment: %1")
                    .arg(mostUsedId > 0 ? equipNameById.value(mostUsedId, QString("Equipment #%1").arg(mostUsedId)) : QString("unknown"));
        lines << QString(" - Most active employee: %1 (%2 events)")
                    .arg(mostActiveEmployee)
                    .arg(qMax(0, mostActiveCount));
        lines << QString(" - Avg events/month: %1")
                    .arg(QString::number(avgEventsPerMonth, 'f', 2));
        lines << QString(" - Events this month vs last month: %1 vs %2")
                    .arg(eventsThisMonth)
                    .arg(eventsLastMonth);
        lines << QString(" - Avg equipment age: %1 years")
                    .arg(QString::number(avgEquipmentAge, 'f', 1));

        smartContext = lines.join("\n");

        if (!self) return;

        QMetaObject::invokeMethod(self.data(),
            [self, equipmentCount, historyCount, activeAlerts, totalValue, dataPoints, workshopHealth,
             availableCount, inUseCount, maintenanceCount, urgentMaintenanceEquipment, urgentMaintenanceDays,
             equipmentNames, recentActivity, smartContext]() {
                if (!self) return;

                self->m_equipmentCount = equipmentCount;
                self->m_historyCount = historyCount;
                self->m_activeAlerts = activeAlerts;
                self->m_availableCount = availableCount;
                self->m_inUseCount = inUseCount;
                self->m_maintenanceCount = maintenanceCount;
                self->m_totalValue = totalValue;
                self->m_dataPoints = dataPoints;
                self->m_healthScore = workshopHealth;
                self->m_urgentMaintenanceEquipment = urgentMaintenanceEquipment;
                self->m_urgentMaintenanceDays = urgentMaintenanceDays;
                self->m_equipmentNames = equipmentNames;
                self->m_recentActivity = recentActivity;
                self->m_cachedSmartContext = smartContext;

                self->rebuildRecentActivity();
                self->updateStatsAnimation();

                if (self->m_msgs.size() == 1 && !self->m_msgs.first().user && !self->m_msgs.first().thinking) {
                    bool finishedTyping = self->m_msgs.first().shownChars >= self->m_msgs.first().text.size();
                    self->m_msgs.first().text = QString(
                        "I have analyzed %1 equipment,\n%2 history events, and %3 active maintenance alerts. "
                        "Your workshop health is at %4%.\n\n"
                        "Ask me anything about your workshop.\n"
                        "I know every tool, every record, every pattern in your data.")
                        .arg(self->m_equipmentCount)
                        .arg(self->m_historyCount)
                        .arg(self->m_activeAlerts)
                        .arg(self->m_healthScore);
                    if (finishedTyping) {
                        self->m_msgs.first().shownChars = self->m_msgs.first().text.size();
                    }
                    if (self->m_msgs.first().bodyLabel) {
                        self->m_msgs.first().bodyLabel->setText(
                            self->formatAiRichText(self->m_msgs.first().text.left(self->m_msgs.first().shownChars)));
                    }
                }

                self->m_dbRefreshInFlight = false;
                if (self->m_dbRefreshQueued) {
                    self->m_dbRefreshQueued = false;
                    self->refreshStatsFromDb();
                    return;
                }

                if (self->m_waitingForAi && !self->m_pendingQuestion.isEmpty()) {
                    QString question = self->m_pendingQuestion;
                    self->m_pendingQuestion.clear();

                    if (!self->m_groqClient) {
                        self->onGroqResponse("AI_ERROR: Groq client unavailable");
                        return;
                    }

                    const QString sys = self->buildSystemPrompt();
                    const QString usr = self->buildUserPrompt(question);
                    self->m_groqClient->sendPrompt(sys, usr, self.data(), "onGroqResponse");
                }
            },
            Qt::QueuedConnection);
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void AiForgeWidget::rebuildRecentActivity() {
    while (m_recentLayout->count() > 1) {
        QLayoutItem *it = m_recentLayout->takeAt(0);
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }

    for (int i = 0; i < m_recentActivity.size(); ++i) {
        const ActivityRow &row = m_recentActivity[i];
        QFrame *card = new QFrame(m_recentHost);
        card->setStyleSheet(QString(
            "QFrame{background:#0A0604;border-left:2px solid %1;border-radius:6px;}"
        ).arg(row.color.name()));

        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(8, 5, 8, 5);
        cl->setSpacing(1);

        QLabel *equip = new QLabel(row.equipment, card);
        equip->setStyleSheet("QLabel{color:#F5E6D3;font-size:9px;font-weight:800;}");
        QLabel *meta = new QLabel(QString("%1  ·  %2").arg(row.action.left(36), row.at), card);
        meta->setStyleSheet("QLabel{color:rgba(245,230,211,132);font-size:8px;font-weight:600;}");
        meta->setWordWrap(true);

        cl->addWidget(equip);
        cl->addWidget(meta);
        card->setVisible(false);
        m_recentLayout->insertWidget(m_recentLayout->count() - 1, card);

        QTimer::singleShot(i * 55, this, [card]() {
            card->setVisible(true);
            QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(card);
            card->setGraphicsEffect(eff);
            QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity", card);
            anim->setDuration(220);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
}

QString AiForgeWidget::buildSmartContext() const {
    if (!m_cachedSmartContext.trimmed().isEmpty()) {
        return m_cachedSmartContext;
    }
    return QString("WORKSHOP OVERVIEW:\n"
                   " Total equipment: %1\n"
                   " Total value: %2dt\n"
                   " Health score: %3%\n"
                   " Active alerts: %4\n"
                   " Context refreshing in background.")
        .arg(m_equipmentCount)
        .arg(QString::number(m_totalValue, 'f', 2))
        .arg(m_healthScore)
        .arg(m_activeAlerts);
}

QString AiForgeWidget::buildSystemPrompt() const {
    QString sys =
        "You are AI Forge - the intelligent management system for this specific carpentry workshop. "
        "You have complete real-time data about every piece of equipment, every maintenance event, every employee interaction, "
        "and every financial metric in this workshop.\n\n"
        "YOUR PERSONALITY:\n"
        "- Expert workshop manager with 20 years experience in carpentry operations\n"
        "- Direct, practical, specific\n"
        "- You know THIS workshop intimately\n"
        "- You use real names and real numbers\n"
        "- You prioritize safety and efficiency\n\n"
        "ANSWER RULES:\n"
        "- Always use specific equipment names from the data\n"
        "- Always cite real numbers: prices, ages, days, percentages\n"
        "- Maximum 120 words per response\n"
        "- Use line breaks for readability\n"
        "- Start with the most important point\n"
        "- End with one clear action if relevant\n"
        "- If asked about something not in data, say what you do know that is related\n";

    if (m_mode == Advisor) {
        sys += "- Answer conversationally as an advisor.\n";
    } else if (m_mode == DeepAnalysis) {
        sys += "- Use deeper structured analysis for risk and planning decisions.\n";
    } else if (m_mode == DailyBrief) {
        sys += "- Use morning standup style with immediate priorities.\n";
    }

    sys += QString("\nCONTEXT:\n%1").arg(buildSmartContext());
    return sys;
}

QString AiForgeWidget::buildUserPrompt(const QString &question) const {
    return question.trimmed();
}

QString AiForgeWidget::buildFallbackResponse(const QString &reason, const QString &question) const {
    QString prompt = question.trimmed();
    if (prompt.isEmpty()) {
        prompt = m_activeQuestion.trimmed();
    }
    const QString lower = prompt.toLower();

    QString urgentTarget = m_urgentMaintenanceEquipment.trimmed();
    if (urgentTarget.isEmpty()) {
        urgentTarget = "Maintenance queue";
    }

    QString mostUsed = "unknown";
    QRegularExpression mostUsedRe("- Most used: ([^\\n]+?) with");
    QRegularExpressionMatch mostUsedMatch = mostUsedRe.match(m_cachedSmartContext);
    if (mostUsedMatch.hasMatch()) {
        mostUsed = mostUsedMatch.captured(1).trimmed();
    }

    QString mostValuable = "unknown";
    QString mostValuablePrice = QString::number((int)m_totalValue);
    QRegularExpression mostValuableRe("- Most valuable: ([^\\n]+?) at ([0-9.]+)dt");
    QRegularExpressionMatch mostValuableMatch = mostValuableRe.match(m_cachedSmartContext);
    if (mostValuableMatch.hasMatch()) {
        mostValuable = mostValuableMatch.captured(1).trimmed();
        mostValuablePrice = mostValuableMatch.captured(2).trimmed();
    }

    const int estimatedCost = qMax(0, m_maintenanceCount * 180 + (m_maintenanceCount > 0 ? 120 : 0));

    QString report;
    if (lower.contains("fix") || lower.contains("urgent") || lower.contains("maintenance")) {
        report = QString(
            "Immediate Fix Priorities:\n"
            "1) Urgent: %1 (%2 days in maintenance)\n"
            "2) Preventive checks: %3 additional equipment in maintenance queue\n"
            "3) Keep production stable: %4 equipment currently in use\n"
            "Estimated immediate maintenance spend: %5dt")
            .arg(urgentTarget)
            .arg(qMax(1, m_urgentMaintenanceDays))
            .arg(qMax(0, m_maintenanceCount - 1))
            .arg(m_inUseCount)
            .arg(estimatedCost);
    } else if (lower.contains("health") || lower.contains("summary")) {
        report = QString(
            "Workshop Health Summary:\n"
            "Health: %1%\n"
            "Equipment: %2 total\n"
            "Available: %3 | In Use: %4 | Maintenance: %5\n"
            "Total value protected: %6dt\n"
            "%7")
            .arg(m_healthScore)
            .arg(m_equipmentCount)
            .arg(m_availableCount)
            .arg(m_inUseCount)
            .arg(m_maintenanceCount)
            .arg((int)m_totalValue)
            .arg(m_maintenanceCount > 0
                ? QString("Urgent attention: %1 (%2 days).")
                      .arg(urgentTarget)
                      .arg(qMax(1, m_urgentMaintenanceDays))
                : QString("No urgent maintenance delay detected."));
    } else if (lower.contains("roi") || lower.contains("return") || lower.contains("value")) {
        report = QString(
            "ROI Focus:\n"
            "Top utilization candidate: %1\n"
            "Highest-value asset: %2 (%3dt)\n"
            "Current uptime profile: %4 available, %5 in use\n"
            "Recommendation: prioritize uptime on %2 and clear urgent maintenance on %6 to protect revenue.")
            .arg(mostUsed)
            .arg(mostValuable)
            .arg(mostValuablePrice)
            .arg(m_availableCount)
            .arg(m_inUseCount)
            .arg(urgentTarget);
    } else if (lower.contains("today") || lower.contains("action") || lower.contains("brief")) {
        report = QString(
            "Today's Workshop Briefing:\n"
            "Health status: %1%\n"
            "Most urgent task: %2 (%3 days waiting)\n"
            "Who should do what:\n"
            "- Maintenance lead: start intervention on %2 now\n"
            "- Operations lead: keep %4 in-use assets running without downtime\n"
            "- Inventory coordinator: prepare parts for %5 maintenance items\n"
            "Estimated costs today: %6dt\n"
            "One thing not to forget: log every intervention in HISTORY before end of shift.")
            .arg(m_healthScore)
            .arg(urgentTarget)
            .arg(qMax(1, m_urgentMaintenanceDays))
            .arg(m_inUseCount)
            .arg(m_maintenanceCount)
            .arg(estimatedCost);
    } else {
        report = QString(
            "Workshop Status Report:\n"
            "Health: %1%\n"
            "Equipment: %2 total\n"
            "Available: %3 | In Use: %4\n"
            "Maintenance needed: %5\n"
            "Total value: %6dt\n"
            "%7")
            .arg(m_healthScore)
            .arg(m_equipmentCount)
            .arg(m_availableCount)
            .arg(m_inUseCount)
            .arg(m_maintenanceCount)
            .arg((int)m_totalValue)
            .arg(m_maintenanceCount > 0
                ? QString("Urgent: %1 (%2 days in maintenance).")
                      .arg(urgentTarget)
                      .arg(qMax(1, m_urgentMaintenanceDays))
                : QString("All systems operating normally."));
    }

    Q_UNUSED(reason);

    return report;
}

QString AiForgeWidget::formatAiRichText(const QString &text) const {
    QString out = text.toHtmlEscaped();
    out.replace("\n", "<br>");

    out.replace(QRegularExpression("(\\b\\d+(?:\\.\\d+)?(?:\\s*dt)?\\b)"),
                "<span style='color:#FFD700;font-weight:800;'>\\1</span>");
    out.replace(QRegularExpression("URGENT:"),
                "<span style='color:#CC2200;font-weight:800;'>URGENT:</span>");
    out.replace(QRegularExpression("\\b(healthy|good|stable)\\b", QRegularExpression::CaseInsensitiveOption),
                "<span style='color:#4CAF7D;font-weight:700;'>\\1</span>");

    for (const QString &equipName : m_equipmentNames) {
        QString t = equipName.trimmed();
        if (t.size() < 3) continue;
        QRegularExpression re(QString("(%1)").arg(QRegularExpression::escape(t)), QRegularExpression::CaseInsensitiveOption);
        out.replace(re, "<span style='color:#C17F3E;font-weight:800;'>\\1</span>");
    }

    return out;
}

void AiForgeWidget::addUserMessage(const QString &text) {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return;

    Msg m;
    m.user = true;
    m.text = trimmed;
    m.shownChars = trimmed.size();
    m.ts = QDateTime::currentDateTime();
    m.dataPoints = 0;
    m.cps = 1000;
    m_msgs.append(m);

    appendMessageWidget(m_msgs.size() - 1);
    scrollMessagesToBottom();
}

void AiForgeWidget::addThinkingMessage() {
    removeThinkingMessage();

    Msg m;
    m.user = false;
    m.thinking = true;
    m.text = "Analyzing...";
    m.shownChars = m.text.size();
    m.ts = QDateTime::currentDateTime();
    m.dataPoints = m_dataPoints;
    m.cps = 1000;
    m_msgs.append(m);

    appendMessageWidget(m_msgs.size() - 1);
    m_thinkingAnimPhase = 0;
    if (m_thinkingTimer && !m_thinkingTimer->isActive()) m_thinkingTimer->start();
    scrollMessagesToBottom();
}

void AiForgeWidget::resolveThinkingMessage(const QString &text) {
    removeThinkingMessage();

    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return;

    Msg m;
    m.user = false;
    m.thinking = false;
    m.text = trimmed;
    m.shownChars = 1;
    m.ts = QDateTime::currentDateTime();
    m.dataPoints = m_dataPoints;
    m.cps = 50;
    m_msgs.append(m);

    appendMessageWidget(m_msgs.size() - 1);
    startTypewriterForLastMessage();
    scrollMessagesToBottom();
}

void AiForgeWidget::removeThinkingMessage() {
    for (int i = m_msgs.size() - 1; i >= 0; --i) {
        if (!m_msgs[i].thinking) continue;
        if (m_msgs[i].rowWidget) {
            if (m_messagesLayout) m_messagesLayout->removeWidget(m_msgs[i].rowWidget);
            m_msgs[i].rowWidget->deleteLater();
            m_msgs[i].rowWidget = nullptr;
        }
        m_msgs.removeAt(i);
        break;
    }

    bool stillThinking = false;
    for (const Msg &m : std::as_const(m_msgs)) {
        if (m.thinking) {
            stillThinking = true;
            break;
        }
    }
    if (!stillThinking && m_thinkingTimer) m_thinkingTimer->stop();
}

void AiForgeWidget::appendMessageWidget(int index) {
    if (index < 0 || index >= m_msgs.size() || !m_messagesLayout || !m_messagesHost) return;

    Msg &m = m_msgs[index];
    if (!m.user && !m.thinking && m.text.trimmed().isEmpty()) return;

    QWidget *row = new QWidget(m_messagesHost);
    QHBoxLayout *rowLay = new QHBoxLayout(row);
    rowLay->setContentsMargins(4, 0, 4, 0);
    rowLay->setSpacing(0);

    const int convoW = (m_messagesScroll && m_messagesScroll->viewport())
        ? m_messagesScroll->viewport()->width()
        : qMax(420, m_centerPanel ? m_centerPanel->width() : width());
    const int bubbleMaxWidth = qMax(260, int(convoW * 0.65));

    if (m.user) {
        QFrame *bubble = new QFrame(row);
        bubble->setMaximumWidth(bubbleMaxWidth);
        bubble->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        bubble->setStyleSheet(
            "QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #2E1A0A,stop:1 #1A0E06);"
            "border:1px solid rgba(193,127,62,102);border-radius:16px 16px 4px 16px;}"
        );

        QVBoxLayout *bl = new QVBoxLayout(bubble);
        bl->setContentsMargins(12, 10, 12, 10);
        bl->setSpacing(0);

        QLabel *txt = new QLabel(m.text.toHtmlEscaped().replace("\n", "<br>"), bubble);
        txt->setTextFormat(Qt::RichText);
        txt->setWordWrap(true);
        txt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        txt->setStyleSheet("QLabel{color:#F5E6D3;font-size:13px;}");
        txt->adjustSize();
        bl->addWidget(txt);

        m.bodyLabel = txt;
        rowLay->addStretch(1);
        rowLay->addWidget(bubble);
    } else {
        QFrame *bubble = new QFrame(row);
        bubble->setMinimumHeight(50);
        bubble->setMaximumWidth(bubbleMaxWidth);
        bubble->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        bubble->setStyleSheet(
            "QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #0A1018,stop:1 #060A12);"
            "border:1px solid rgba(193,127,62,90);border-radius:4px 16px 16px 16px;}"
        );

        QHBoxLayout *outer = new QHBoxLayout(bubble);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->setSpacing(0);

        QFrame *accent = new QFrame(bubble);
        accent->setFixedWidth(4);
        accent->setStyleSheet(
            "QFrame{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #FFD700,stop:.5 #C17F3E,stop:1 #8B4A1E);"
            "border-top-left-radius:4px;border-bottom-left-radius:16px;}"
        );
        outer->addWidget(accent);

        QWidget *content = new QWidget(bubble);
        content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        QVBoxLayout *cl = new QVBoxLayout(content);
        cl->setContentsMargins(0, 0, 0, 0);
        cl->setSpacing(0);

        QFrame *topBand = new QFrame(content);
        topBand->setFixedHeight(22);
        topBand->setStyleSheet("QFrame{background:rgba(13,21,32,204);border-bottom:1px solid rgba(193,127,62,38);}");
        QHBoxLayout *tb = new QHBoxLayout(topBand);
        tb->setContentsMargins(10, 0, 10, 0);
        QLabel *aiIcon = new QLabel(topBand);
        aiIcon->setFixedSize(14, 14);
        QPixmap aiPx(":/assets/aiforge.png");
        if (!aiPx.isNull()) {
            aiIcon->setPixmap(aiPx.scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        QLabel *ttl = new QLabel("AI FORGE", topBand);
        ttl->setStyleSheet("QLabel{color:#C17F3E;font-size:8px;font-weight:800;letter-spacing:1px;}");
        tb->addWidget(aiIcon);
        tb->addWidget(ttl);
        tb->addStretch(1);
        cl->addWidget(topBand);

        QWidget *body = new QWidget(content);
        body->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        QVBoxLayout *bodyLay = new QVBoxLayout(body);
        bodyLay->setContentsMargins(12, 10, 14, 8);
        bodyLay->setSpacing(5);

        if (m.thinking) {
            QLabel *dotsLbl = new QLabel("●  ●  ●", body);
            dotsLbl->setTextFormat(Qt::RichText);
            dotsLbl->setAlignment(Qt::AlignCenter);
            dotsLbl->setWordWrap(true);
            dotsLbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
            dotsLbl->setStyleSheet("QLabel{color:rgba(193,127,62,180);font-size:11px;font-weight:800;}");
            bodyLay->addWidget(dotsLbl);

            QLabel *waitLbl = new QLabel("Analyzing...", body);
            waitLbl->setAlignment(Qt::AlignCenter);
            waitLbl->setWordWrap(true);
            waitLbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
            waitLbl->setStyleSheet("QLabel{color:rgba(245,230,211,125);font-size:10px;font-style:italic;}");
            bodyLay->addWidget(waitLbl);

            m.thinkingDotsLabel = dotsLbl;
            m.thinkingCaptionLabel = waitLbl;
        } else {
            QLabel *txt = new QLabel(formatAiRichText(m.text.left(qMax(1, m.shownChars))), body);
            txt->setTextFormat(Qt::RichText);
            txt->setWordWrap(true);
            txt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
            txt->setStyleSheet("QLabel{color:#F5E6D3;font-size:13px;line-height:1.6;}");
            txt->adjustSize();
            bodyLay->addWidget(txt);
            m.bodyLabel = txt;
        }

        QLabel *meta = new QLabel(QString("%1    Based on %2 data points")
                                   .arg(m.ts.time().toString("HH:mm"))
                                   .arg(m.dataPoints), body);
        meta->setWordWrap(true);
        meta->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        meta->setStyleSheet("QLabel{color:rgba(245,230,211,85);font-size:8px;font-style:italic;}");
        bodyLay->addWidget(meta);

        cl->addWidget(body);
        outer->addWidget(content, 1);

        rowLay->addWidget(bubble);
        rowLay->addStretch(1);
    }

    m.rowWidget = row;
    m_messagesLayout->insertWidget(m_messagesLayout->count() - 1, row);
}

void AiForgeWidget::clearMessageWidgets() {
    while (m_messagesLayout && m_messagesLayout->count() > 1) {
        QLayoutItem *it = m_messagesLayout->takeAt(0);
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }

    for (Msg &m : m_msgs) {
        m.rowWidget = nullptr;
        m.bodyLabel = nullptr;
        m.thinkingDotsLabel = nullptr;
        m.thinkingCaptionLabel = nullptr;
    }
}

void AiForgeWidget::rebuildMessageWidgets() {
    clearMessageWidgets();
    for (int i = 0; i < m_msgs.size(); ++i) {
        appendMessageWidget(i);
    }
    scrollMessagesToBottom();
}

void AiForgeWidget::startTypewriterForLastMessage() {
    if (m_msgs.isEmpty()) return;

    const int idx = m_msgs.size() - 1;
    Msg &m = m_msgs[idx];
    if (m.user || m.thinking || m.text.isEmpty()) return;

    m.shownChars = qMax(1, m.shownChars);
    if (m.bodyLabel) {
        m.bodyLabel->setText(formatAiRichText(m.text.left(m.shownChars)));
        m.bodyLabel->adjustSize();
    }

    if (m.shownChars >= m.text.size()) {
        m_typingMessageIndex = -1;
        if (m_typingTimer) m_typingTimer->stop();
        return;
    }

    m_typingMessageIndex = idx;
    if (m_typingTimer && !m_typingTimer->isActive()) m_typingTimer->start();
}

void AiForgeWidget::updateTypingFrame() {
    if (m_typingMessageIndex < 0 || m_typingMessageIndex >= m_msgs.size()) {
        if (m_typingTimer) m_typingTimer->stop();
        m_typingMessageIndex = -1;
        return;
    }

    Msg &m = m_msgs[m_typingMessageIndex];
    if (m.user || m.thinking || !m.bodyLabel) {
        if (m_typingTimer) m_typingTimer->stop();
        m_typingMessageIndex = -1;
        return;
    }

    if (m.shownChars < m.text.size()) {
        int add = qMax(1, int(qRound(float(m.cps) * 0.02f)));
        m.shownChars = qMin(m.text.size(), m.shownChars + add);
        m.bodyLabel->setText(formatAiRichText(m.text.left(m.shownChars)));
        m.bodyLabel->adjustSize();
        scrollMessagesToBottom();
    }

    if (m.shownChars >= m.text.size()) {
        if (m_typingTimer) m_typingTimer->stop();
        m_typingMessageIndex = -1;
    }
}

void AiForgeWidget::updateThinkingFrame() {
    if (!m_waitingForAi) {
        if (m_thinkingTimer) m_thinkingTimer->stop();
        return;
    }

    int idx = -1;
    for (int i = m_msgs.size() - 1; i >= 0; --i) {
        if (m_msgs[i].thinking) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        if (m_thinkingTimer) m_thinkingTimer->stop();
        return;
    }

    Msg &m = m_msgs[idx];
    m_thinkingAnimPhase = (m_thinkingAnimPhase + 1) % 3;

    if (m.thinkingDotsLabel) {
        QString dots;
        for (int i = 0; i < 3; ++i) {
            const int alpha = (i == m_thinkingAnimPhase) ? 255 : 120;
            dots += QString("<span style='color:rgba(193,127,62,%1);font-size:11px;font-weight:800;'>●</span>")
                        .arg(alpha);
            if (i < 2) dots += "&nbsp;&nbsp;";
        }
        m.thinkingDotsLabel->setText(dots);
    }
    if (m.thinkingCaptionLabel) {
        m.thinkingCaptionLabel->setText(QString("Analyzing%1").arg(QString(".").repeated(m_thinkingAnimPhase + 1)));
    }
}

void AiForgeWidget::scrollMessagesToBottom() {
    QTimer::singleShot(50, this, [this]() {
        if (m_messagesScroll && m_messagesScroll->verticalScrollBar()) {
            m_messagesScroll->verticalScrollBar()->setValue(m_messagesScroll->verticalScrollBar()->maximum());
        }
    });
}

void AiForgeWidget::sendQuestion(const QString &question) {
    if (question.trimmed().isEmpty()) return;
    if (m_waitingForAi) return;
    m_input->setText(question);
    onSend();
}

void AiForgeWidget::onSend() {
    const QString enteredQuestion = m_input->text().trimmed();
    if (enteredQuestion.isEmpty() || m_waitingForAi) return;

    QString effectiveQuestion = enteredQuestion;
    QString normalized = enteredQuestion.trimmed();
    if (normalized.startsWith('/')) {
        normalized.remove(0, 1);
    }
    normalized = normalized.simplified();

    const QString cmd = normalized.section(' ', 0, 0).toLower();
    const QString arg = normalized.section(' ', 1).trimmed();

    if ((cmd == "help" || cmd == "commands" || cmd == "?") && arg.isEmpty()) {
        addUserMessage(enteredQuestion);
        resolveThinkingMessage(
            "AI Forge Commands:\n"
            "- help : show this command list\n"
            "- fix : what needs fixing now\n"
            "- health : workshop health summary\n"
            "- roi : best ROI equipment\n"
            "- plan : today's action plan\n"
            "- brief : daily briefing mode + summary\n"
            "- deep : deep analysis mode + full analysis\n"
            "- advisor : switch to advisor mode\n"
            "- ask <question> : send a custom question\n"
            "\nYou can also type any normal question directly."
        );
        m_input->clear();
        return;
    }

    if (cmd == "ask") {
        if (arg.isEmpty()) {
            addUserMessage(enteredQuestion);
            resolveThinkingMessage("Usage: ask <your question>");
            m_input->clear();
            return;
        }
        effectiveQuestion = arg;
    } else if (cmd == "fix" && arg.isEmpty()) {
        effectiveQuestion = "What needs fixing now?";
    } else if (cmd == "health" && arg.isEmpty()) {
        effectiveQuestion = "Workshop health summary";
    } else if (cmd == "roi" && arg.isEmpty()) {
        effectiveQuestion = "Best ROI equipment?";
    } else if ((cmd == "plan" || cmd == "action") && arg.isEmpty()) {
        effectiveQuestion = "Today's action plan";
    } else if (cmd == "brief" && arg.isEmpty()) {
        setMode(DailyBrief);
        effectiveQuestion = "Give today's workshop briefing: health status, most urgent task, who should do what, estimated costs, one thing not to forget.";
    } else if (cmd == "deep" && arg.isEmpty()) {
        setMode(DeepAnalysis);
        effectiveQuestion = "Perform complete deep analysis covering health, top 3 risks, financial overview, and 3 action recommendations.";
    } else if (cmd == "advisor" && arg.isEmpty()) {
        setMode(Advisor);
        addUserMessage(enteredQuestion);
        resolveThinkingMessage("Advisor mode enabled. Ask your workshop question.");
        m_input->clear();
        return;
    }

    m_waitingForAi = true;
    updateInteractiveState();
    updateStatsAnimation();

    addUserMessage(enteredQuestion);
    addThinkingMessage();

    m_input->clear();
    m_pendingQuestion = effectiveQuestion;
    m_activeQuestion = effectiveQuestion;

    // Guard against indefinite "Analyzing..." if callback delivery fails.
    const QString reqToken = QString::number(QDateTime::currentMSecsSinceEpoch())
        + "_" + QString::number(QRandomGenerator::global()->generate());
    setProperty("ai_forge_req_token", reqToken);
    QTimer::singleShot(25000, this, [this, reqToken]() {
        if (!m_waitingForAi) return;
        if (property("ai_forge_req_token").toString() != reqToken) return;
        onGroqResponse("AI_ERROR: Request timed out");
    });

    if (!m_groqClient) {
        onGroqResponse("AI_ERROR: Groq client unavailable");
        return;
    }

    // Build complete smart context in worker thread, then send Groq request.
    refreshStatsFromDb();
}

void AiForgeWidget::onGroqResponse(const QString &text) {
    setProperty("ai_forge_req_token", QString());

    const QString questionForFallback = m_activeQuestion;
    QString response = text.trimmed();
    const QString lower = response.toLower();
    const bool isFailure = response.isEmpty()
        || response.startsWith("AI_ERROR")
        || response.startsWith("AI Error", Qt::CaseInsensitive)
        || lower.contains("exceeded your current quota")
        || lower.contains("plan and billing")
        || lower.contains("api key")
        || lower.contains("permission denied");

    if (isFailure) {
        response = buildFallbackResponse("live unavailable", questionForFallback);
    }

    m_pendingQuestion.clear();
    m_activeQuestion.clear();
    resolveThinkingMessage(response);

    m_waitingForAi = false;
    if (m_thinkingTimer) m_thinkingTimer->stop();
    updateInteractiveState();
    updateStatsAnimation();

    // Silent sidebar/context refresh in background.
    refreshStatsFromDb();
}

void AiForgeWidget::updateStatsAnimation() {
    qreal healthScale = 1.0 + 0.018 * std::sin(m_globalTime * 1.1);
    qreal equipmentScale = 1.0 + 0.015 * std::sin(m_globalTime * 1.1 + 1.2);
    qreal alertScale = m_activeAlerts > 0 ? (1.0 + 0.08 * std::sin(m_globalTime * 3.0)) : 1.0;

    QString healthColor = m_healthScore > 80 ? "#4CAF7D" : (m_healthScore > 60 ? "#C17F3E" : "#CC2200");
    QString alertColor = m_activeAlerts > 0 ? "#CC2200" : "#4CAF7D";

    m_healthValue->setText(QString::number(m_healthScore));
    m_healthValue->setStyleSheet(QString("QLabel{color:%1;font-size:%2px;font-weight:800;}")
                                 .arg(healthColor)
                                 .arg(int(26.0 * healthScale)));

    m_equipmentValue->setText(QString::number(m_equipmentCount));
    m_equipmentValue->setStyleSheet(QString("QLabel{color:#C17F3E;font-size:%1px;font-weight:800;}")
                                    .arg(int(25.0 * equipmentScale)));

    m_alertValue->setText(QString::number(m_activeAlerts));
    m_alertValue->setStyleSheet(QString("QLabel{color:%1;font-size:%2px;font-weight:800;}")
                                .arg(alertColor)
                                .arg(int(25.0 * alertScale)));

    m_totalValueDisplay += (m_totalValue - m_totalValueDisplay) * 0.08;
    if (qAbs(m_totalValue - m_totalValueDisplay) < 1.0) m_totalValueDisplay = m_totalValue;
    m_totalValueValue->setText(QString::number((int)m_totalValueDisplay));
    m_totalValueValue->setStyleSheet("QLabel{color:#FFD700;font-size:26px;font-weight:800;}");

    int dotAlpha = 120 + int((std::sin(m_globalTime * 3.0) + 1.0) * 50.0);
    m_contextStatus->setText(QString("<span style='color:rgba(76,175,125,%1);'>●</span> Context loaded · %2 data points")
                             .arg(dotAlpha)
                             .arg(m_dataPoints));

    int pulse = 20 + int((std::sin(m_globalTime * 2.5) + 1.0) * 35.0);
    if (m_waitingForAi) {
        m_statusLabel->setText("Analyzing...");
        m_statusLabel->setStyleSheet(QString("QLabel{color:#C17F3E;background:#1A1008;border:1px solid rgba(193,127,62,%1);border-radius:10px;padding:4px 10px;font-size:9px;font-weight:800;}")
                                     .arg(120 + pulse));
    } else {
        m_statusLabel->setText("Ready");
        m_statusLabel->setStyleSheet("QLabel{color:#4CAF7D;background:#0A1A0A;border:1px solid rgba(76,175,125,128);border-radius:10px;padding:4px 10px;font-size:9px;font-weight:800;}");
    }
}

void AiForgeWidget::onTick() {
    m_globalTime += 0.016f;
    if (m_entrance < 1.0f) {
        m_entrance = qMin(1.0f, m_entrance + 0.02f);
    }
    update();
}

void AiForgeWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

    // Layer 1: Deep AI void
    QRadialGradient rg(QPointF(width() * 0.45, height() * 0.40), qMax(width(), height()) * 0.9);
    rg.setColorAt(0.0, QColor("#0A0F1A"));
    rg.setColorAt(0.4, QColor("#080B14"));
    rg.setColorAt(1.0, QColor("#000000"));
    p.fillRect(rect(), rg);

    // Layer 2: Neural grid
    float drift = std::fmod(m_globalTime * 5.0f, 32.0f);
    p.setPen(QPen(QColor(193, 127, 62, 10), 1)); // 4%
    for (int y = -32; y < height() + 32; y += 32) {
        int lineIndex = (y + 32) / 32;
        QPainterPath path;
        path.moveTo(0, y - drift);
        for (int x = 0; x <= width(); x += 14) {
            qreal wy = (y - drift) + std::sin(x * 0.008 + m_globalTime * 0.30 + lineIndex * 0.2) * 1.5;
            path.lineTo(x, wy);
        }
        p.drawPath(path);
    }

    p.setPen(QPen(QColor(193, 127, 62, 6), 1)); // 2.5%
    for (int x = 0; x <= width(); x += 32) {
        p.drawLine(x, 0, x, height());
    }

    // Layer 3: Ambient particles (150)
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(193, 127, 62, 13));
    for (int i = 0; i < 150; ++i) {
        float seedY = float((i * 37) % qMax(1, height()));
        float rise = m_globalTime * (16.0f + float(i % 5) * 3.0f);
        float py = std::fmod(seedY - rise, float(qMax(1, height())));
        if (py < 0.0f) py += float(height());
        float seedX = float((i * 89) % qMax(1, width()));
        float px = seedX + std::sin(m_globalTime * 0.18f + i * 0.85f) * 1.2f;
        p.drawEllipse(QPointF(px, py), 1.7, 1.7);
    }

    // Layer 4: Ambient breathing glows
    auto glow = [&](const QPointF &center, const QColor &color, qreal radius, qreal pulse) {
        QRadialGradient g(center, radius);
        g.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), int(16 * pulse)));
        g.setColorAt(1.0, Qt::transparent);
        p.setBrush(g);
        p.drawEllipse(center, radius, radius);
    };
    glow(QPointF(width() * 0.1, height() * 0.1), QColor("#C17F3E"), width() * 0.30, 1.0 + 0.4 * std::sin(m_globalTime * 0.28));
    glow(QPointF(width() * 0.9, height() * 0.88), QColor("#3B82F6"), width() * 0.25, 1.0 + 0.4 * std::sin(m_globalTime * 0.35 + 1.8));
    glow(QPointF(width() * 0.35, height() * 0.55), QColor("#8B4A1E"), width() * 0.35, 1.0 + 0.4 * std::sin(m_globalTime * 0.22 + 0.9));
    glow(QPointF(width() * 0.86, height() * 0.14), QColor("#C17F3E"), width() * 0.20, 1.0 + 0.25 * std::sin(m_globalTime * 0.31));

    // Scanlines
    p.setPen(QPen(QColor(0, 0, 0, 13), 1));
    for (int y = 0; y < height(); y += 3) {
        p.drawLine(0, y, width(), y);
    }

    // Panel separators
    auto drawSep = [&](int x) {
        QLinearGradient g(x, 0, x, height());
        g.setColorAt(0.0, Qt::transparent);
        g.setColorAt(0.5, QColor(193, 127, 62, 64));
        g.setColorAt(1.0, Qt::transparent);
        p.setPen(QPen(g, 1));
        p.drawLine(x, 0, x, height());
    };
    drawSep(260);
    drawSep(width() - 220);

}

// ============================================================================
// PARALLEL FUTURES WIDGET
// ============================================================================
ParallelFuturesWidget::ParallelFuturesWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, true);
    buildUi();

    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(16);
    connect(m_tickTimer, &QTimer::timeout, this, &ParallelFuturesWidget::onTick);
    m_tickTimer->start();

    refreshMetrics();
}

void ParallelFuturesWidget::bindEngine(InferenceEngine *engine) {
    m_engine = engine;
    refreshMetrics();
}

void ParallelFuturesWidget::buildUi() {
    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(10);

    m_leftPanel = new QWidget(this);
    m_leftPanel->setFixedWidth(260);
    m_leftPanel->setStyleSheet("QWidget{background:#0A0806;border:1px solid rgba(193,127,62,75);border-radius:12px;}");
    QVBoxLayout *leftLay = new QVBoxLayout(m_leftPanel);
    leftLay->setContentsMargins(12, 12, 12, 12);
    leftLay->setSpacing(8);

    QLabel *leftTitle = new QLabel("PARALLEL\nFUTURES", m_leftPanel);
    leftTitle->setStyleSheet("QLabel{color:#D4AF37;font-size:16px;font-weight:900;letter-spacing:1px;}");
    leftLay->addWidget(leftTitle);

    QLabel *leftSub = new QLabel("Digital-twin scenario simulator", m_leftPanel);
    leftSub->setStyleSheet("QLabel{color:#B8925A;font-size:10px;}");
    leftLay->addWidget(leftSub);

    auto makeChip = [&](const QString &name, QLabel **valueLabel) {
        QFrame *chip = new QFrame(m_leftPanel);
        chip->setStyleSheet("QFrame{background:#120C07;border:1px solid rgba(193,127,62,65);border-radius:8px;}");
        QVBoxLayout *cl = new QVBoxLayout(chip);
        cl->setContentsMargins(9, 7, 9, 7);
        cl->setSpacing(2);

        QLabel *nameLbl = new QLabel(name, chip);
        nameLbl->setStyleSheet("QLabel{color:#8A7A6A;font-size:9px;font-weight:800;letter-spacing:1px;}");
        QLabel *value = new QLabel("--", chip);
        value->setStyleSheet("QLabel{color:#F5E6D3;font-size:18px;font-weight:900;}");
        cl->addWidget(nameLbl);
        cl->addWidget(value);
        *valueLabel = value;
        return chip;
    };

    leftLay->addWidget(makeChip("WORKSHOP HEALTH", &m_healthChip));
    leftLay->addWidget(makeChip("ASSETS", &m_assetsChip));
    leftLay->addWidget(makeChip("MAINTENANCE LOAD", &m_maintenanceChip));
    leftLay->addWidget(makeChip("TOTAL VALUE", &m_valueChip));

    QLabel *quickLbl = new QLabel("HIGH-IMPACT STARTERS", m_leftPanel);
    quickLbl->setStyleSheet("QLabel{color:#C17F3E;font-size:9px;font-weight:900;letter-spacing:1px;padding-top:4px;}");
    leftLay->addWidget(quickLbl);

    auto addStarter = [&](const QString &title, const QString &prompt) {
        QPushButton *btn = new QPushButton(title, m_leftPanel);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton{background:#0D0905;border:1px solid rgba(193,127,62,75);border-radius:8px;"
            "color:#F5E6D3;font-size:10px;font-weight:700;padding:8px;text-align:left;}"
            "QPushButton:hover{background:#1A1208;border:1px solid rgba(193,127,62,155);color:#FFFFFF;}"
        );
        connect(btn, &QPushButton::clicked, this, [this, prompt]() {
            if (m_questionInput) m_questionInput->setText(prompt);
            onGenerate();
        });
        leftLay->addWidget(btn);
    };

    addStarter("Budget Shock", "What is the safest plan if maintenance budget is cut by 20% for the next quarter?");
    addStarter("Downtime Crisis", "If two critical machines fail this month, what is the best operational recovery path?");
    addStarter("Growth Move", "If we buy 3 new high-value machines, how does risk, ROI, and downtime change over 12 months?");
    leftLay->addStretch(1);

    m_centerPanel = new QWidget(this);
    m_centerPanel->setStyleSheet("QWidget{background:#090705;border:1px solid rgba(193,127,62,80);border-radius:12px;}");
    QVBoxLayout *centerLay = new QVBoxLayout(m_centerPanel);
    centerLay->setContentsMargins(12, 12, 12, 12);
    centerLay->setSpacing(8);

    QLabel *title = new QLabel("PARALLEL FUTURES AI LAB", m_centerPanel);
    title->setStyleSheet("QLabel{color:#D4AF37;font-size:22px;font-weight:900;letter-spacing:1px;}");
    centerLay->addWidget(title);

    QLabel *sub = new QLabel("Simulate multiple futures, compare impact, and get AI council verdict", m_centerPanel);
    sub->setStyleSheet("QLabel{color:#B8925A;font-size:11px;}");
    centerLay->addWidget(sub);

    QFrame *controlBar = new QFrame(m_centerPanel);
    controlBar->setStyleSheet("QFrame{background:#120C07;border:1px solid rgba(193,127,62,85);border-radius:10px;}");
    QHBoxLayout *controlLay = new QHBoxLayout(controlBar);
    controlLay->setContentsMargins(8, 8, 8, 8);
    controlLay->setSpacing(6);

    m_questionInput = new QLineEdit(controlBar);
    m_questionInput->setPlaceholderText("Describe a strategic decision to simulate...");
    m_questionInput->setStyleSheet(
        "QLineEdit{background:#090705;border:1px solid rgba(193,127,62,100);border-radius:7px;"
        "color:#F5E6D3;padding:7px;font-size:11px;}"
        "QLineEdit:focus{border:1px solid #D4AF37;}"
    );

    m_horizonCombo = new QComboBox(controlBar);
    m_horizonCombo->addItems({"3 months", "6 months", "12 months", "24 months"});
    m_horizonCombo->setCurrentIndex(2);
    m_horizonCombo->setStyleSheet(
        "QComboBox{background:#0D0905;border:1px solid rgba(193,127,62,120);border-radius:7px;color:#F5E6D3;padding:6px;}"
        "QComboBox::drop-down{border:none;width:20px;}"
        "QComboBox QAbstractItemView{background:#0D0905;color:#F5E6D3;border:1px solid #5A4A32;}"
    );

    m_generateBtn = new QPushButton("Simulate", controlBar);
    m_generateBtn->setCursor(Qt::PointingHandCursor);
    m_generateBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #D4AF37,stop:1 #8B4A1E);"
        "border:none;border-radius:8px;color:#1A140A;font-weight:900;padding:8px 16px;font-size:11px;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #E7C867,stop:1 #A8622A);}"
        "QPushButton:disabled{background:#5A4A32;color:#D9C2A7;}"
    );

    m_randomizeBtn = new QPushButton("Shock Test", controlBar);
    m_randomizeBtn->setCursor(Qt::PointingHandCursor);
    m_randomizeBtn->setStyleSheet(
        "QPushButton{background:#1A1208;border:1px solid rgba(193,127,62,130);border-radius:8px;color:#F5E6D3;"
        "font-weight:800;padding:8px 12px;font-size:10px;}"
        "QPushButton:hover{background:#2A1A08;color:#FFFFFF;border:1px solid #D4AF37;}"
    );

    controlLay->addWidget(m_questionInput, 1);
    controlLay->addWidget(m_horizonCombo);
    controlLay->addWidget(m_generateBtn);
    controlLay->addWidget(m_randomizeBtn);
    centerLay->addWidget(controlBar);

    connect(m_generateBtn, &QPushButton::clicked, this, &ParallelFuturesWidget::onGenerate);
    connect(m_questionInput, &QLineEdit::returnPressed, this, &ParallelFuturesWidget::onGenerate);
    connect(m_randomizeBtn, &QPushButton::clicked, this, [this]() {
        static const QStringList prompts = {
            "If we delay all non-critical maintenance by one month, what is the real risk?",
            "How can we maximize throughput next quarter without increasing long-term failure risk?",
            "What is the smartest maintenance and investment mix for best ROI over 12 months?"
        };
        int idx = QRandomGenerator::global()->bounded(prompts.size());
        if (m_questionInput) m_questionInput->setText(prompts[idx]);
        onGenerate();
    });

    m_statusLabel = new QLabel("Ready to simulate futures", m_centerPanel);
    m_statusLabel->setStyleSheet("QLabel{color:#4CAF7D;font-size:10px;font-weight:800;padding:2px 4px;}");
    centerLay->addWidget(m_statusLabel);

    m_cardsScroll = new QScrollArea(m_centerPanel);
    m_cardsScroll->setWidgetResizable(true);
    m_cardsScroll->setStyleSheet("QScrollArea{background:#050403;border:1px solid rgba(193,127,62,70);border-radius:10px;}");
    m_cardsHost = new QWidget();
    m_cardsHost->setStyleSheet("background:transparent;");
    m_cardsLayout = new QVBoxLayout(m_cardsHost);
    m_cardsLayout->setContentsMargins(8, 8, 8, 8);
    m_cardsLayout->setSpacing(8);
    m_cardsLayout->addStretch(1);
    m_cardsScroll->setWidget(m_cardsHost);
    centerLay->addWidget(m_cardsScroll, 1);

    m_rightPanel = new QWidget(this);
    m_rightPanel->setFixedWidth(340);
    m_rightPanel->setStyleSheet("QWidget{background:#0A0806;border:1px solid rgba(193,127,62,75);border-radius:12px;}");
    QVBoxLayout *rightLay = new QVBoxLayout(m_rightPanel);
    rightLay->setContentsMargins(12, 12, 12, 12);
    rightLay->setSpacing(8);

    QLabel *debateTitle = new QLabel("AI COUNCIL DEBATE", m_rightPanel);
    debateTitle->setStyleSheet("QLabel{color:#D4AF37;font-size:14px;font-weight:900;letter-spacing:1px;}");
    rightLay->addWidget(debateTitle);

    QLabel *debateSub = new QLabel("CFO AI · Maintenance Chief AI · Risk Auditor AI", m_rightPanel);
    debateSub->setStyleSheet("QLabel{color:#8A7A6A;font-size:9px;}");
    rightLay->addWidget(debateSub);

    m_debateView = new QTextEdit(m_rightPanel);
    m_debateView->setReadOnly(true);
    m_debateView->setStyleSheet(
        "QTextEdit{background:#050403;border:1px solid rgba(193,127,62,65);border-radius:8px;"
        "color:#F5E6D3;padding:8px;font-family:'Consolas';font-size:11px;}"
    );
    m_debateView->setPlainText("Run a simulation to start AI council debate.");
    rightLay->addWidget(m_debateView, 1);

    QFrame *verdictFrame = new QFrame(m_rightPanel);
    verdictFrame->setStyleSheet(
        "QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1A1208,stop:1 #100B06);"
        "border:1px solid rgba(212,175,55,140);border-radius:10px;}"
    );
    QVBoxLayout *vlay = new QVBoxLayout(verdictFrame);
    vlay->setContentsMargins(10, 8, 10, 8);
    QLabel *vTitle = new QLabel("FINAL VERDICT", verdictFrame);
    vTitle->setStyleSheet("QLabel{color:#D4AF37;font-size:10px;font-weight:900;letter-spacing:1px;}");
    m_verdictLabel = new QLabel("No verdict yet", verdictFrame);
    m_verdictLabel->setWordWrap(true);
    m_verdictLabel->setStyleSheet("QLabel{color:#F5E6D3;font-size:11px;font-weight:700;}");
    vlay->addWidget(vTitle);
    vlay->addWidget(m_verdictLabel);
    rightLay->addWidget(verdictFrame);

    root->addWidget(m_leftPanel);
    root->addWidget(m_centerPanel, 1);
    root->addWidget(m_rightPanel);
}

void ParallelFuturesWidget::refreshMetrics() {
    QList<NexusEquipment> equips = loadAllEquipment();

    m_equipmentCount = equips.size();
    m_maintenanceCount = 0;
    m_totalValue = 0.0;
    int available = 0;

    for (const NexusEquipment &eq : std::as_const(equips)) {
        m_totalValue += eq.unitPrice * qMax(1, eq.quantity);
        QString s = eq.status.toLower();
        if (s.contains("maint")) m_maintenanceCount++;
        if (s.contains("available")) available++;
    }

    m_healthScore = (m_engine != nullptr)
        ? int(qRound(m_engine->healthScore()))
        : (m_equipmentCount > 0 ? int((double(available) * 100.0) / double(m_equipmentCount)) : 0);

    m_activeAlerts = 0;
    QSqlQuery q;
    bool ok = q.exec("SELECT COUNT(*) FROM HISTORY WHERE UPPER(ACTION_TYPE)='ORGANISM_BORN'");
    if (!ok) {
        ok = q.exec("SELECT COUNT(*) FROM history WHERE UPPER(action_type)='ORGANISM_BORN'");
    }
    if (ok && q.next()) {
        m_activeAlerts = q.value(0).toInt();
    }

    if (m_healthChip) m_healthChip->setText(QString("%1%").arg(m_healthScore));
    if (m_assetsChip) m_assetsChip->setText(QString::number(m_equipmentCount));
    if (m_maintenanceChip) m_maintenanceChip->setText(QString::number(m_maintenanceCount));
    if (m_valueChip) m_valueChip->setText(QString::number(int(m_totalValue)) + "dt");
}

QList<ParallelFuturesWidget::FutureScenario> ParallelFuturesWidget::buildScenarios(const QString &question) const {
    QList<FutureScenario> out;

    int horizonMonths = 12;
    if (m_horizonCombo) {
        QString t = m_horizonCombo->currentText();
        horizonMonths = t.section(' ', 0, 0).toInt();
        if (horizonMonths <= 0) horizonMonths = 12;
    }

    const uint seed = qHash(question.toLower() + "|" + QString::number(horizonMonths)
                            + "|" + QString::number(m_equipmentCount)
                            + "|" + QString::number(m_maintenanceCount));
    auto jitter = [&](int low, int high, int salt) {
        if (high <= low) return low;
        uint mixed = seed ^ (0x9e3779b9u + uint(salt) + (seed << 6) + (seed >> 2));
        return low + int(mixed % uint(high - low + 1));
    };

    const int pressure = qMax(1, m_maintenanceCount + m_activeAlerts);
    const int baseHealth = qBound(0, m_healthScore, 100);
    const int valueScale = qMax(250, int(m_totalValue * 0.01));

    FutureScenario shield;
    shield.name = "Shield Strategy";
    shield.posture = "Stability first";
    shield.cost = qMax(600, valueScale + pressure * 240 + jitter(120, 460, 3));
    shield.downtime = qMax(2, pressure + jitter(2, 6, 5));
    shield.risk = qBound(8, 20 + pressure * 2 + jitter(-2, 6, 7), 70);
    shield.health = qBound(45, baseHealth + 10 + jitter(-2, 3, 9), 98);
    shield.narrative = "Aggressive maintenance now to lock reliability and reduce surprise failures.";

    FutureScenario balanced;
    balanced.name = "Balanced Momentum";
    balanced.posture = "Cost vs uptime";
    balanced.cost = qMax(450, int(valueScale * 0.75) + pressure * 180 + jitter(80, 320, 11));
    balanced.downtime = qMax(1, qMax(2, pressure - 1) + jitter(1, 4, 13));
    balanced.risk = qBound(12, 28 + pressure * 3 + jitter(-3, 7, 15), 84);
    balanced.health = qBound(35, baseHealth + 4 + jitter(-3, 2, 17), 93);
    balanced.narrative = "Phased interventions while preserving throughput and cash discipline.";

    FutureScenario aggressive;
    aggressive.name = "Production Surge";
    aggressive.posture = "Output first";
    aggressive.cost = qMax(300, int(valueScale * 0.55) + pressure * 120 + jitter(60, 260, 19));
    aggressive.downtime = qMax(1, qMax(1, pressure - 3) + jitter(0, 2, 21));
    aggressive.risk = qBound(20, 44 + pressure * 4 + jitter(-4, 9, 23), 96);
    aggressive.health = qBound(25, baseHealth - 6 + jitter(-4, 1, 25), 88);
    aggressive.narrative = "Minimize immediate interruptions but accept compounding reliability risk.";

    out << shield << balanced << aggressive;
    return out;
}

void ParallelFuturesWidget::rebuildScenarioCards() {
    while (m_cardsLayout && m_cardsLayout->count() > 1) {
        QLayoutItem *item = m_cardsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    for (int i = 0; i < m_scenarios.size(); ++i) {
        const FutureScenario &s = m_scenarios[i];

        QFrame *card = new QFrame(m_cardsHost);
        card->setStyleSheet(
            "QFrame{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #120C07,stop:1 #0A0806);"
            "border:1px solid rgba(193,127,62,115);border-radius:10px;}"
        );

        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(10, 9, 10, 9);
        cl->setSpacing(6);

        QLabel *name = new QLabel(QString("%1  ·  %2").arg(s.name, s.posture), card);
        name->setStyleSheet("QLabel{color:#D4AF37;font-size:12px;font-weight:900;}");
        cl->addWidget(name);

        QGridLayout *grid = new QGridLayout();
        grid->setHorizontalSpacing(10);
        grid->setVerticalSpacing(3);

        auto metric = [&](const QString &k, const QString &v, int r, int c) {
            QLabel *kLbl = new QLabel(k, card);
            kLbl->setStyleSheet("QLabel{color:#8A7A6A;font-size:9px;font-weight:800;}");
            QLabel *vLbl = new QLabel(v, card);
            vLbl->setStyleSheet("QLabel{color:#F5E6D3;font-size:12px;font-weight:800;}");
            grid->addWidget(kLbl, r, c * 2);
            grid->addWidget(vLbl, r, c * 2 + 1);
        };

        metric("Cost", QString::number(s.cost) + "dt", 0, 0);
        metric("Downtime", QString::number(s.downtime) + "d", 0, 1);
        metric("Risk", QString::number(s.risk) + "%", 1, 0);
        metric("Health", QString::number(s.health) + "%", 1, 1);
        cl->addLayout(grid);

        QLabel *riskLbl = new QLabel("Failure Risk", card);
        riskLbl->setStyleSheet("QLabel{color:#B8925A;font-size:9px;font-weight:700;}");
        cl->addWidget(riskLbl);
        QProgressBar *riskBar = new QProgressBar(card);
        riskBar->setRange(0, 100);
        riskBar->setValue(s.risk);
        riskBar->setTextVisible(false);
        riskBar->setStyleSheet(
            "QProgressBar{background:#0A0907;border:1px solid rgba(193,127,62,70);height:9px;border-radius:4px;}"
            "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #CC2200,stop:1 #F59E0B);border-radius:3px;}"
        );
        cl->addWidget(riskBar);

        QLabel *healthLbl = new QLabel("Projected Health", card);
        healthLbl->setStyleSheet("QLabel{color:#B8925A;font-size:9px;font-weight:700;}");
        cl->addWidget(healthLbl);
        QProgressBar *healthBar = new QProgressBar(card);
        healthBar->setRange(0, 100);
        healthBar->setValue(s.health);
        healthBar->setTextVisible(false);
        healthBar->setStyleSheet(
            "QProgressBar{background:#0A0907;border:1px solid rgba(193,127,62,70);height:9px;border-radius:4px;}"
            "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4CAF7D,stop:1 #22C55E);border-radius:3px;}"
        );
        cl->addWidget(healthBar);

        QLabel *desc = new QLabel(s.narrative, card);
        desc->setWordWrap(true);
        desc->setStyleSheet("QLabel{color:#E0D0B0;font-size:10px;}");
        cl->addWidget(desc);

        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(card);
        eff->setOpacity(0.0);
        card->setGraphicsEffect(eff);

        m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, card);

        QTimer::singleShot(i * 100, this, [eff, this]() {
            QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity", this);
            anim->setDuration(380);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }
}

QString ParallelFuturesWidget::buildDebatePrompt() const {
    QString prompt;
    prompt += QString("Decision question: %1\n").arg(m_lastQuestion);
    prompt += QString("Horizon: %1\n\n").arg(m_horizonCombo ? m_horizonCombo->currentText() : QString("12 months"));

    prompt += QString("Workshop baseline:\n- health: %1%%\n- assets: %2\n- maintenance backlog: %3\n- active alerts: %4\n- total value: %5dt\n\n")
        .arg(m_healthScore)
        .arg(m_equipmentCount)
        .arg(m_maintenanceCount)
        .arg(m_activeAlerts)
        .arg(int(m_totalValue));

    prompt += "Scenarios:\n";
    for (const FutureScenario &s : m_scenarios) {
        prompt += QString("- %1 (%2): cost=%3dt, downtime=%4d, risk=%5%%, projected_health=%6%%. %7\n")
            .arg(s.name, s.posture)
            .arg(s.cost)
            .arg(s.downtime)
            .arg(s.risk)
            .arg(s.health)
            .arg(s.narrative);
    }

    prompt += "\nReturn plain text only with this exact structure:\n"
              "CFO Perspective:\n"
              "Maintenance Chief Perspective:\n"
              "Risk Auditor Perspective:\n"
              "Final Verdict:\n"
              "Backup Plan:\n"
              "Confidence: <0-100>%\n"
              "Keep it practical and specific for a carpentry workshop.";
    return prompt;
}

QString ParallelFuturesWidget::buildLocalDebate() const {
    if (m_scenarios.isEmpty()) {
        return "CFO Perspective:\nNo scenario data available.\n\n"
               "Maintenance Chief Perspective:\nNo scenario data available.\n\n"
               "Risk Auditor Perspective:\nNo scenario data available.\n\n"
               "Final Verdict:\nRun a simulation first.\n\n"
               "Backup Plan:\nRun a simulation first.\n\n"
               "Confidence: 0%";
    }

    int best = 0;
    int backup = 0;
    double bestScore = -1e9;
    double backupScore = -1e9;

    for (int i = 0; i < m_scenarios.size(); ++i) {
        const FutureScenario &s = m_scenarios[i];
        double score = (100.0 - s.risk) * 0.48 + s.health * 0.34 - s.cost * 0.0009 - s.downtime * 2.3;
        if (score > bestScore) {
            backupScore = bestScore;
            backup = best;
            bestScore = score;
            best = i;
        } else if (score > backupScore) {
            backupScore = score;
            backup = i;
        }
    }

    const FutureScenario &winner = m_scenarios[best];
    const FutureScenario &runner = m_scenarios[backup];

    return QString(
        "CFO Perspective:\n"
        "%1 gives the strongest cost-to-stability ratio at %2dt with controlled downside.\n\n"
        "Maintenance Chief Perspective:\n"
        "%1 protects workshop health at %3%% while keeping downtime at %4 days.\n\n"
        "Risk Auditor Perspective:\n"
        "Primary hidden risk remains backlog acceleration; monitor %5%% risk threshold weekly.\n\n"
        "Final Verdict:\n"
        "Choose %1 now for the next %6 horizon.\n\n"
        "Backup Plan:\n"
        "%7 if budget pressure increases or downtime must be reduced immediately.\n\n"
        "Confidence: 74%%")
        .arg(winner.name)
        .arg(winner.cost)
        .arg(winner.health)
        .arg(winner.downtime)
        .arg(winner.risk)
        .arg(m_horizonCombo ? m_horizonCombo->currentText() : QString("12 months"))
        .arg(runner.name);
}

void ParallelFuturesWidget::setLoadingState(bool loading) {
    m_loading = loading;
    if (m_generateBtn) {
        m_generateBtn->setEnabled(!loading);
        m_generateBtn->setText(loading ? "Running..." : "Simulate");
    }
    if (m_questionInput) m_questionInput->setEnabled(!loading);
    if (m_horizonCombo) m_horizonCombo->setEnabled(!loading);
    if (!loading && m_statusLabel) {
        m_statusLabel->setText("Simulation complete");
        m_statusLabel->setStyleSheet("QLabel{color:#4CAF7D;font-size:10px;font-weight:800;padding:2px 4px;}");
    }
}

void ParallelFuturesWidget::onGenerate() {
    QString question = m_questionInput ? m_questionInput->text().trimmed() : QString();
    if (question.isEmpty()) {
        question = "What is the best maintenance and investment strategy for the next year?";
        if (m_questionInput) m_questionInput->setText(question);
    }

    m_lastQuestion = question;
    refreshMetrics();
    m_scenarios = buildScenarios(question);
    rebuildScenarioCards();

    if (m_debateView) {
        m_debateView->setPlainText("AI Council is evaluating all futures...\n\nPlease wait.");
    }
    if (m_verdictLabel) {
        m_verdictLabel->setText("Computing final recommendation...");
    }

    setLoadingState(true);

    if (!m_groqClient) {
        onGroqDebateResponse("AI_ERROR: Groq client unavailable");
        return;
    }

    const QString sys =
        "You are NEXUS Parallel Futures Council: CFO strategist, Maintenance Chief, and Risk Auditor. "
        "You must challenge assumptions and produce one practical verdict for this workshop.";
    const QString usr = buildDebatePrompt();
    m_groqClient->sendPrompt(sys, usr, this, "onGroqDebateResponse");
}

void ParallelFuturesWidget::onGroqDebateResponse(const QString &text) {
    QString response = text.trimmed();
    const QString lower = response.toLower();
    const bool failed = response.isEmpty()
        || response.startsWith("AI_ERROR")
        || lower.contains("api key")
        || lower.contains("quota")
        || lower.contains("billing")
        || lower.contains("permission denied");

    if (failed) {
        response = buildLocalDebate();
    }

    if (m_debateView) {
        m_debateView->setPlainText(response);
    }

    QString verdict = "Review complete.";
    QRegularExpression rx("Final Verdict\\s*:\\s*([^\\n]+)");
    QRegularExpressionMatch match = rx.match(response);
    if (match.hasMatch()) {
        verdict = match.captured(1).trimmed();
    } else {
        QString firstLine = response.section('\n', 0, 0).trimmed();
        if (!firstLine.isEmpty()) verdict = firstLine;
    }

    if (m_verdictLabel) {
        m_verdictLabel->setText(verdict);
    }

    setLoadingState(false);
}

void ParallelFuturesWidget::onTick() {
    m_phase += 0.016f;
    if (m_phase > 100000.0f) m_phase = 0.0f;

    if (m_loading && m_statusLabel) {
        int dots = 1 + int((std::sin(m_phase * 2.8f) + 1.0f) * 1.5f);
        m_statusLabel->setText(QString("AI council deliberating%1").arg(QString(".").repeated(dots)));
        int alpha = 120 + int((std::sin(m_phase * 3.5f) + 1.0f) * 50.0f);
        m_statusLabel->setStyleSheet(QString("QLabel{color:rgba(193,127,62,%1);font-size:10px;font-weight:900;padding:2px 4px;}")
                                     .arg(alpha));
    }

    update();
}

void ParallelFuturesWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);

    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor("#0A0705"));
    bg.setColorAt(0.6, QColor("#050403"));
    bg.setColorAt(1.0, QColor("#030202"));
    p.fillRect(rect(), bg);

    p.setPen(QPen(QColor(193, 127, 62, 12), 1));
    const int step = 34;
    const int offset = int(std::fmod(m_phase * 12.0f, float(step)));
    for (int y = -step; y < height() + step; y += step) {
        p.drawLine(0, y + offset, width(), y + offset);
    }
    p.setPen(QPen(QColor(193, 127, 62, 8), 1));
    for (int x = 0; x < width(); x += step) {
        p.drawLine(x, 0, x, height());
    }
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

    QStringList tabsLabels = {"Graph", "Time", "Report", "Decisions", "Maintenance", "AI Forge", "Futures Lab"};
    QStringList tabsIcons = {":/assets/graph.png", ":/assets/time.png", ":/assets/report.png", ":/assets/predict.png", ":/assets/maintenance.png", ":/assets/aiforge.png", ":/assets/predict.png"};
    
    for (int i = 0; i < tabsLabels.size(); ++i) {
        QPushButton *btn = new QPushButton(tabsLabels[i], this);
        if (!tabsIcons[i].isEmpty()) {
            btn->setIcon(QIcon(tabsIcons[i]));
            btn->setIconSize(QSize(18, 18));
        }
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
    m_aiForgeWidget = new AiForgeWidget(m_stack);
    m_parallelFuturesWidget = new ParallelFuturesWidget(m_stack);

    m_stack->addWidget(m_graphWidget);       // 0
    m_stack->addWidget(m_timeMachine);       // 1
    m_stack->addWidget(m_reportWidget);      // 2
    m_stack->addWidget(m_decisionMapper);    // 3
    m_stack->addWidget(m_maintenanceWidget); // 4
    m_stack->addWidget(m_aiForgeWidget);     // 5
    m_stack->addWidget(m_parallelFuturesWidget); // 6

    mainLay->addWidget(m_stack, 1);

    m_groqApiKey = "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";

    // Engine
    m_engine = new InferenceEngine(this);
    m_networkManager = new QNetworkAccessManager(this);
    m_groqClient = new NexusGroqClient(this);
    m_groqClient->setApiKey(m_groqApiKey);
    m_aiForgeWidget->setGroqClient(m_groqClient);
    m_parallelFuturesWidget->setGroqClient(m_groqClient);
    m_parallelFuturesWidget->bindEngine(m_engine);

    connect(m_engine, &InferenceEngine::insightsReady, this, [this](){
        // Generate report and load decision mapper
        m_reportWidget->generateReport(m_engine);
        m_decisionMapper->loadData(m_engine);
        m_parallelFuturesWidget->bindEngine(m_engine);
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

    QNetworkRequest req;
    QJsonObject obj;
    const bool isGeminiKey = m_groqApiKey.trimmed().startsWith("AIza");

    if (isGeminiKey) {
        QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent");
        QUrlQuery query;
        query.addQueryItem("key", m_groqApiKey.trimmed());
        url.setQuery(query);

        req = QNetworkRequest(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonArray parts;
        parts.append(QJsonObject{{"text", prompt}});
        QJsonArray contents;
        contents.append(QJsonObject{{"role", "user"}, {"parts", parts}});
        obj["contents"] = contents;
        obj["generationConfig"] = QJsonObject{{"temperature", 0.7}, {"maxOutputTokens", 1000}};
    } else {
        req = QNetworkRequest(QUrl("https://api.groq.com/openai/v1/chat/completions"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_groqApiKey).toUtf8());

        obj["model"] = "llama-3.3-70b-versatile";
        QJsonArray messages;
        QJsonObject msg;
        msg["role"] = "user";
        msg["content"] = prompt;
        messages.append(msg);
        obj["messages"] = messages;
        obj["temperature"] = 0.7;
    }

    QNetworkReply *reply = m_networkManager->post(req, QJsonDocument(obj).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [reply, callback, isGeminiKey]() {
        const QByteArray raw = reply->readAll();
        QString content;

        if (reply->error() == QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            const QJsonObject root = doc.object();
            if (isGeminiKey) {
                const QJsonArray candidates = root["candidates"].toArray();
                if (!candidates.isEmpty()) {
                    const QJsonObject cand0 = candidates.at(0).toObject();
                    const QJsonArray parts = cand0["content"].toObject()["parts"].toArray();
                    if (!parts.isEmpty()) {
                        content = parts.at(0).toObject()["text"].toString();
                    }
                }
                if (content.trimmed().isEmpty()) {
                    const QString err = root["error"].toObject()["message"].toString();
                    content = err.isEmpty() ? "AI Error: Empty Gemini response" : QString("AI Error: %1").arg(err);
                }
            } else {
                const QJsonArray choices = root["choices"].toArray();
                if (!choices.isEmpty()) {
                    content = choices.at(0).toObject()["message"].toObject()["content"].toString();
                }
                if (content.trimmed().isEmpty()) {
                    const QString err = root["error"].toObject()["message"].toString();
                    content = err.isEmpty() ? "AI Error: Empty Groq response" : QString("AI Error: %1").arg(err);
                }
            }
        } else {
            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            const QString err = doc.object()["error"].toObject()["message"].toString();
            content = err.isEmpty()
                ? QString("AI Error: %1").arg(reply->errorString())
                : QString("AI Error: %1").arg(err);
        }

        callback(content);
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
        m_parallelFuturesWidget->bindEngine(m_engine);
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
        m_parallelFuturesWidget->bindEngine(m_engine);

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
    const bool isGeminiKey = m_apiKey.trimmed().startsWith("AIza");
    QNetworkRequest req;
    QJsonObject obj;

    if (isGeminiKey) {
        QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent");
        QUrlQuery query;
        query.addQueryItem("key", m_apiKey.trimmed());
        url.setQuery(query);

        req = QNetworkRequest(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        obj["system_instruction"] = QJsonObject{{"parts", QJsonArray{QJsonObject{{"text", sys}}}}};
        QJsonArray contents;
        contents.append(QJsonObject{{"role", "user"}, {"parts", QJsonArray{QJsonObject{{"text", usr}}}}});
        obj["contents"] = contents;
        obj["generationConfig"] = QJsonObject{{"temperature", 0.7}, {"maxOutputTokens", 1000}};
    } else {
        req = QNetworkRequest(QUrl("https://api.groq.com/openai/v1/chat/completions"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

        obj["model"] = "llama-3.3-70b-versatile";
        QJsonArray msgs;
        msgs.append(QJsonObject{{"role", "system"}, {"content", sys}});
        msgs.append(QJsonObject{{"role", "user"}, {"content", usr}});
        obj["messages"] = msgs;
        obj["max_tokens"] = 1000;
        obj["temperature"] = 0.7;
    }

    QString slotName = QString::fromLatin1(sl ? sl : "");
    if (!slotName.isEmpty() && (slotName.startsWith('1') || slotName.startsWith('2'))) {
        slotName.remove(0, 1);
    }

    QNetworkReply *rep = m_network->post(req, QJsonDocument(obj).toJson(QJsonDocument::Compact));
    connect(rep, &QNetworkReply::finished, this, [rep, rc, slotName, isGeminiKey]() {
        QString res;
        const QByteArray data = rep->readAll();

        if (rep->error() == QNetworkReply::NoError) {
            const QJsonDocument doc = QJsonDocument::fromJson(data);
            const QJsonObject root = doc.object();

            if (isGeminiKey) {
                const QJsonArray candidates = root["candidates"].toArray();
                if (!candidates.isEmpty()) {
                    const QJsonObject cand0 = candidates.at(0).toObject();
                    const QJsonArray parts = cand0["content"].toObject()["parts"].toArray();
                    if (!parts.isEmpty()) {
                        res = parts.at(0).toObject()["text"].toString();
                    }
                }
                if (res.trimmed().isEmpty()) {
                    const QString err = root["error"].toObject()["message"].toString();
                    res = err.isEmpty()
                        ? QString("AI_ERROR: Empty Gemini response")
                        : QString("AI_ERROR: %1").arg(err);
                }
            } else {
                const QJsonArray choices = root["choices"].toArray();
                if (!choices.isEmpty()) {
                    res = choices.at(0).toObject()["message"].toObject()["content"].toString();
                }
                if (res.trimmed().isEmpty()) {
                    const QString err = root["error"].toObject()["message"].toString();
                    res = err.isEmpty()
                        ? QString("AI_ERROR: Empty Groq response")
                        : QString("AI_ERROR: %1").arg(err);
                }
            }
        } else {
            const QJsonDocument doc = QJsonDocument::fromJson(data);
            const QString err = doc.object()["error"].toObject()["message"].toString();
            res = err.isEmpty()
                ? QString("AI_ERROR (%1): %2").arg(rep->error()).arg(rep->errorString())
                : QString("AI_ERROR (%1): %2").arg(rep->error()).arg(err);
        }

        bool delivered = false;
        if (rc) {
            const QByteArray slotUtf8 = slotName.toUtf8();
            if (!slotUtf8.isEmpty()) {
                delivered = QMetaObject::invokeMethod(rc, slotUtf8.constData(), Qt::QueuedConnection, Q_ARG(QString, res));
            }
            if (!delivered) {
                delivered = QMetaObject::invokeMethod(rc, "onGroqResponse", Qt::QueuedConnection, Q_ARG(QString, res));
            }
        }

        if (!delivered) {
            qWarning() << "Groq response delivery failed for" << rc << slotName;
        }
        rep->deleteLater();
    });
}

