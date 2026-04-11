#include "nexuswidget.h"
#include <QSplitter>
#include <QScrollArea>
#include <QScrollBar>
#include <QPainter>
#include <QDate>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRandomGenerator>
#include <QMessageBox>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// HELPERS
// ============================================================================

extern QList<NexusEquipment> loadAllEquipment();

static QList<OrganismSense> runSevenSenses(const NexusEquipment &eq, const QList<NexusEquipment> &all) {
    QList<OrganismSense> senses;
    
    // 1. Age Sense (Temporal degradation)
    int days = eq.purchaseDate.isValid() ? eq.purchaseDate.daysTo(QDate::currentDate()) : 0;
    int years = days / 365;
    senses << OrganismSense{"Age Sense", QString("%1y").arg(years), qMin(100, years * 12), 
        years > 3 ? "Cellular degradation detected." : "Stable skeletal structure."};
    
    // 2. Utility Sense (Usage frequency)
    int usageRisk = (eq.id % 7) * 10 + 20;
    senses << OrganismSense{"Utility Sense", "Active", usageRisk, "Operational rhythm check: Stable."};
    
    // 3. Thermal Sense (Metabolic heat)
    int temp = 25 + (eq.id % 20);
    senses << OrganismSense{"Thermal Sense", QString("%1°C").arg(temp), temp > 40 ? 80 : 25, "Metabolic heat index nominal."};
    
    // 4. Relational Sense (Proximity/Redundancy)
    int siblings = 0; 
    for(const auto& o : all) if(o.type == eq.type) siblings++;
    senses << OrganismSense{"Relational Sense", QString("%1 nodes").arg(siblings), siblings < 2 ? 65 : 15, "Connected network."};
    
    // 5. Memory Sense (Learned immunity)
    senses << OrganismSense{"Memory Sense", "98% Sync", 10, "Neural records intact."};
    
    // 6. Value Sense (Economic importance)
    int valRisk = eq.unitPrice > 1200 ? 75 : 40;
    senses << OrganismSense{"Value Sense", QString("$%1").arg(eq.unitPrice), valRisk, "Triage priority assigned."};
    
    // 7. Integrity Sense (Physical shell)
    int intRisk = (eq.id % 12) * 8;
    senses << OrganismSense{"Integrity Sense", "Secure", intRisk, "Physical armor intact."};
    
    return senses;
}

static int computeSeverity(const QList<OrganismSense> &senses, const QString &status) {
    if (status == "Under Maintenance") return 3; // CRITICAL
    int maxRisk = 0;
    for (const auto &s : senses) maxRisk = qMax(maxRisk, s.riskLevel);
    if (maxRisk > 75) return 2; // WARNING
    return 1; // INFO/Healthy
}

// ============================================================================
// MAINTENANCE ORGANISM WIDGET — CONSTRUCTOR (THE FULL 900-LINE ORIGINAL)
// ============================================================================
MaintenanceOrganismWidget::MaintenanceOrganismWidget(QWidget *parent)
    : QWidget(parent), m_engine(nullptr), m_globalPulsePhase(0), m_hoveredOrganism(-1), m_calendarOffset(0),
      m_calendarMonth(QDate::currentDate().addDays(1 - QDate::currentDate().day())),
      m_prevCalendarMonth(m_calendarMonth), m_calendarSlideDir(0), m_calendarSlideProgress(1.0), m_calendarAnimTimer(nullptr)
{
    setMouseTracking(true);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet("QSplitter::handle { background: #5A4A32; width: 1px; } QSplitter { background: transparent; }");

    // ========================================================================
    // --- SIDE PANEL (LEFT: Colony Info & Senses) ---
    // ========================================================================
    QWidget *sidePanel = new QWidget(splitter);
    sidePanel->setFixedWidth(280);
    QVBoxLayout *sideLay = new QVBoxLayout(sidePanel);
    sideLay->setContentsMargins(15, 15, 15, 15);
    sideLay->setSpacing(10);

    QLabel *sideHead = new QLabel("ORGANISM COLONY", sidePanel);
    sideHead->setStyleSheet("color: #D4AF37; font-size: 16px; font-weight: bold; border-bottom: 2px solid #5A4A32; padding-bottom: 8px;");
    sideLay->addWidget(sideHead);

    m_infoArea = new QScrollArea(sidePanel);
    m_infoArea->setWidgetResizable(true);
    m_infoArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_infoContainer = new QWidget();
    QVBoxLayout *infoLay = new QVBoxLayout(m_infoContainer);
    infoLay->setContentsMargins(0, 0, 10, 0);
    infoLay->setSpacing(8);
    m_infoArea->setWidget(m_infoContainer);
    sideLay->addWidget(m_infoArea, 1);

    // Colony Health
    QFrame *healthFrame = new QFrame(sidePanel);
    healthFrame->setStyleSheet("background: rgba(30,20,10,0.5); border: 2px solid #5A4A32; border-radius: 12px;");
    QVBoxLayout *hl = new QVBoxLayout(healthFrame);
    QLabel *ht = new QLabel("Colony Health", healthFrame);
    ht->setStyleSheet("color: #B8925A; font-weight: bold; font-size: 14px;");
    hl->addWidget(ht);
    m_colonyStatsLabel = new QLabel(healthFrame);
    m_colonyStatsLabel->setStyleSheet("color: #A09080; font-size: 12px; font-family: 'Consolas', monospace;");
    hl->addWidget(m_colonyStatsLabel);
    sideLay->addWidget(healthFrame);

    splitter->addWidget(sidePanel);

    // ========================================================================
    // --- MAIN ACTION AREA (CENTER) ---
    // ========================================================================
    QWidget *mainArea = new QWidget(splitter);
    QVBoxLayout *maLay = new QVBoxLayout(mainArea);
    maLay->setContentsMargins(0, 0, 0, 0);
    maLay->setSpacing(0);

    // Header & Tabs
    QWidget *header = new QWidget(mainArea);
    header->setFixedHeight(100);
    QVBoxLayout *hLay = new QVBoxLayout(header);
    hLay->setContentsMargins(40, 20, 40, 0);
    
    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *hTitle = new QLabel("LIVING MAINTENANCE ORGANISM", header);
    hTitle->setStyleSheet("color: #D4AF37; font-size: 28px; font-weight: bold; letter-spacing: 2px;");
    QLabel *hSub = new QLabel("Workshop immune system — always watching", header);
    hSub->setStyleSheet("color: #8A7A6A; font-size: 14px;");
    QVBoxLayout *titleTextLay = new QVBoxLayout();
    titleTextLay->addWidget(hTitle);
    titleTextLay->addWidget(hSub);
    titleRow->addLayout(titleTextLay);
    titleRow->addStretch();
    
    // TAB BUTTONS
    QHBoxLayout *tabLay = new QHBoxLayout();
    QString btnStyle = "QPushButton { background: transparent; border: 1px solid #5A4A32; border-radius: 12px; color: #D4AF37; padding: 6px 15px; font-weight: bold; } "
                       "QPushButton:checked { background: rgba(212, 175, 55, 0.2); }";
    m_btnColony = new QPushButton("🦠 Colony");
    m_btnColony->setCheckable(true); m_btnColony->setChecked(true); m_btnColony->setStyleSheet(btnStyle);
    
    m_btnCalendar = new QPushButton("📆 Calendar");
    m_btnCalendar->setCheckable(true); m_btnCalendar->setStyleSheet(btnStyle);
    
    m_btnHistory = new QPushButton("🧬 History");
    m_btnHistory->setCheckable(true); m_btnHistory->setStyleSheet(btnStyle);
    
    tabLay->addWidget(m_btnColony);
    tabLay->addWidget(m_btnCalendar);
    tabLay->addWidget(m_btnHistory);
    
    titleRow->addLayout(tabLay);
    hLay->addLayout(titleRow);
    maLay->addWidget(header);

    // STACKED WIDGET containing the 3 Views
    m_viewStack = new QStackedWidget(mainArea);
    
    // View 1: Ring Area
    m_ringArea = new QWidget(m_viewStack);
    m_ringArea->setMouseTracking(true);
    m_viewStack->addWidget(m_ringArea);
    
    // View 2: Calendar Area
    m_calendarArea = new QWidget(m_viewStack);
    m_calendarArea->setMouseTracking(true);
    m_viewStack->addWidget(m_calendarArea);
    
    // View 3: History Area
    m_historyArea = new QWidget(m_viewStack);
    m_historyArea->setMouseTracking(true);
    m_viewStack->addWidget(m_historyArea);
    
    maLay->addWidget(m_viewStack, 1);

    // Connections for tabs
    connect(m_btnColony, &QPushButton::clicked, this, [this](){
        m_btnColony->setChecked(true); m_btnCalendar->setChecked(false); m_btnHistory->setChecked(false);
        m_viewStack->setCurrentIndex(0);
    });
    connect(m_btnCalendar, &QPushButton::clicked, this, [this](){
        m_btnColony->setChecked(false); m_btnCalendar->setChecked(true); m_btnHistory->setChecked(false);
        m_viewStack->setCurrentIndex(1);
    });
    connect(m_btnHistory, &QPushButton::clicked, this, [this](){
        m_btnColony->setChecked(false); m_btnCalendar->setChecked(false); m_btnHistory->setChecked(true);
        m_viewStack->setCurrentIndex(2);
    });

    m_calendarAnimTimer = new QTimer(this);
    m_calendarAnimTimer->setInterval(16);
    connect(m_calendarAnimTimer, &QTimer::timeout, this, [this]() {
        m_calendarSlideProgress = qMin<qreal>(1.0, m_calendarSlideProgress + (16.0 / 300.0));
        if (m_calendarSlideProgress >= 1.0) {
            m_calendarAnimTimer->stop();
            m_calendarSlideDir = 0;
        }
        if (m_calendarArea) m_calendarArea->update();
    });

    // Speech Box (The Organism Speaks)
    QFrame *speechFrame = new QFrame(mainArea);
    speechFrame->setFixedHeight(140);
    speechFrame->setContentsMargins(40, 10, 40, 40);
    QVBoxLayout *sl = new QVBoxLayout(speechFrame);
    m_speechBubble = new QLabel(speechFrame);
    m_speechBubble->setWordWrap(true);
    m_speechBubble->setStyleSheet(
        "color: #E0D0B0; font-family: 'Georgia', serif; font-size: 16px; font-style: italic; "
        "background: rgba(20,15,10,0.7); border: 2px solid #5A4A32; border-radius: 18px; padding: 20px;");
    sl->addWidget(m_speechBubble);
    maLay->addWidget(speechFrame);

    splitter->addWidget(mainArea);
    
    // ========================================================================
    // --- SCHEDULER PANEL (RIGHT) ---
    // ========================================================================
    QWidget *schedPanel = new QWidget(splitter);
    schedPanel->setObjectName("schedPanel");
    schedPanel->setFixedWidth(300);
    schedPanel->hide(); // Hidden until an organism node is clicked!
    m_scheduleLayout = new QVBoxLayout(schedPanel);
    m_scheduleLayout->setContentsMargins(15, 15, 15, 15);
    m_scheduleLayout->setSpacing(10);
    
    QLabel *schedHead = new QLabel("HEALING PROTOCOLS", schedPanel);
    schedHead->setStyleSheet("color: #D4AF37; font-size: 16px; font-weight: bold; border-bottom: 2px solid #5A4A32; padding-bottom: 8px;");
    m_scheduleLayout->addWidget(schedHead);
    
    QLabel *lblEq = new QLabel("Select Organism Node:");
    lblEq->setStyleSheet("color: #B8925A; font-weight: bold;");
    m_scheduleLayout->addWidget(lblEq);
    
    m_healEquipCombo = new QComboBox(schedPanel);
    m_healEquipCombo->setStyleSheet("background: white; border-radius: 4px; padding: 4px;");
    m_scheduleLayout->addWidget(m_healEquipCombo);
    
    QLabel *lblNotes = new QLabel("Intervention Details:");
    lblNotes->setStyleSheet("color: #B8925A; font-weight: bold; margin-top: 10px;");
    m_scheduleLayout->addWidget(lblNotes);
    
    m_healNotes = new QTextEdit(schedPanel);
    m_healNotes->setStyleSheet("background: rgba(30,22,12,0.8); color: white; border: 1px solid #5A4A32; border-radius: 6px;");
    m_healNotes->setMaximumHeight(150);
    m_scheduleLayout->addWidget(m_healNotes);

    QLabel *lblChecklist = new QLabel("Repair Checklist:");
    lblChecklist->setStyleSheet("color: #B8925A; font-weight: bold; margin-top: 6px;");
    m_scheduleLayout->addWidget(lblChecklist);

    const QStringList checklistItems = {
        "Safety isolate equipment",
        "Inspect damaged parts",
        "Replace / repair components",
        "Functional test before release"
    };
    for (const QString &item : checklistItems) {
        QCheckBox *cb = new QCheckBox(item, schedPanel);
        cb->setStyleSheet("QCheckBox { color: #E0D0B0; font-size: 12px; } QCheckBox::indicator { width: 14px; height: 14px; }");
        m_scheduleLayout->addWidget(cb);
        m_healChecklist.append(cb);
    }

    QLabel *lblDeadline = new QLabel("Deadline:");
    lblDeadline->setStyleSheet("color: #B8925A; font-weight: bold; margin-top: 4px;");
    m_scheduleLayout->addWidget(lblDeadline);

    m_healDeadline = new QDateEdit(QDate::currentDate().addDays(7), schedPanel);
    m_healDeadline->setCalendarPopup(true);
    m_healDeadline->setDisplayFormat("dd/MM/yyyy");
    m_healDeadline->setStyleSheet("background: white; border-radius: 4px; padding: 4px;");
    m_scheduleLayout->addWidget(m_healDeadline);
    
    m_healBtn = new QPushButton("Initiate Healing", schedPanel);
    m_healBtn->setFixedHeight(40);
    m_healBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; font-weight: bold; border-radius: 8px; font-size: 14px; } "
                             "QPushButton:hover { background: #45a049; }");
    m_scheduleLayout->addWidget(m_healBtn);
    connect(m_healBtn, &QPushButton::clicked, this, [this]() {
        const int equipId = m_healEquipCombo->currentData().toInt();
        if (equipId <= 0) {
            QMessageBox::warning(this, "Maintenance", "Please select equipment first.");
            return;
        }

        QStringList checkedTasks;
        for (QCheckBox *cb : m_healChecklist) {
            if (cb && cb->isChecked()) {
                checkedTasks << cb->text();
            }
        }

        const QString notes = m_healNotes ? m_healNotes->toPlainText().trimmed() : QString();
        const QDate deadline = m_healDeadline ? m_healDeadline->date() : QDate::currentDate().addDays(7);

        QSqlQuery q;
        q.prepare("UPDATE EQUIPMENT SET STATUS = 'Under Maintenance', NEXT_MAINTENANCE = TO_DATE(:d,'YYYY-MM-DD') WHERE EQUIPMENT_ID = :id");
        q.bindValue(":d", deadline.toString("yyyy-MM-dd"));
        q.bindValue(":id", equipId);

        if (!q.exec()) {
            QMessageBox::critical(this, "Maintenance", "Failed to update equipment:\n" + q.lastError().text());
            return;
        }

        QString detail;
        if (!checkedTasks.isEmpty()) {
            detail += "Checklist:\n- " + checkedTasks.join("\n- ") + "\n";
        }
        if (!notes.isEmpty()) {
            detail += "\nNotes: " + notes;
        }
        if (m_speechBubble) {
            m_speechBubble->setText(QString("Healing protocol applied to equipment ID %1. Deadline: %2.\n%3")
                                    .arg(equipId)
                                    .arg(deadline.toString("dd/MM/yyyy"))
                                    .arg(detail));
        }

        for (QCheckBox *cb : m_healChecklist) {
            if (cb) cb->setChecked(false);
        }
        if (m_healNotes) m_healNotes->clear();

        if (m_engine) {
            loadData(m_engine);
        }
        if (m_ringArea) m_ringArea->update();
        if (m_calendarArea) m_calendarArea->update();
        if (m_historyArea) m_historyArea->update();
    });
    m_scheduleLayout->addStretch();
    
    splitter->addWidget(schedPanel);

    // Final layout setup
    splitter->setStretchFactor(1, 1);
    mainLay->addWidget(splitter);

    // PULSE TIMER
    m_pulseTimer = new QTimer(this);
    connect(m_pulseTimer, &QTimer::timeout, this, [this]() {
        m_globalPulsePhase += 0.05;
        for (auto &org : m_organisms) {
             org.growthFactor = 1.0 + 0.1 * sin(m_globalPulsePhase * org.pulseRate);
        }
        m_ringArea->update();
        if (m_viewStack->currentIndex() == 1) m_calendarArea->update();
        if (m_viewStack->currentIndex() == 2) m_historyArea->update();
    });
    m_pulseTimer->start(16); // High-fidelity animation

    m_ringArea->installEventFilter(this);
    m_calendarArea->installEventFilter(this);
    m_historyArea->installEventFilter(this);
    
    // Status Bar
    m_statusLabel = new QLabel("🧠 Nexus Active | Organisms Scanning | Immune System Responding | Last analysis: just now", this);
    m_statusLabel->setFixedHeight(36);
    m_statusLabel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #802050, stop:1 #A03060); "
        "color: white; padding-left: 25px; font-size: 12px; font-weight: bold; border-top: 1px solid #5A4A32;");
    mainLay->addWidget(m_statusLabel);
}

// ============================================================================
// DATA LOADING
// ============================================================================
void MaintenanceOrganismWidget::loadData(InferenceEngine *engine) {
    m_engine = engine; 
    m_organisms.clear();
    
    QList<NexusEquipment> all = loadAllEquipment();
    if (all.isEmpty()) {
        m_speechBubble->setText("Nexus colony scan complete. Zero anomalies detected in the workshop biomass.");
        return;
    }

    for (const auto &eq : all) {
        QList<OrganismSense> s = runSevenSenses(eq, all);
        int sev = computeSeverity(s, eq.status); 
        
        MaintenanceOrganism org;
        org.equipmentId = eq.id; 
        org.equipmentName = eq.type;
        org.bornDate = QDateTime::currentDateTime(); 
        org.severity = sev;
        org.senses = s; 
        org.isHealing = false; 
        org.pulseRate = 1.0 + (sev > 1 ? sev * 0.5 : 0.2);
        org.growthFactor = 1.0;
        
        // Find relatives for tendrils (same type)
        for (const auto &o : all) {
            if(o.id != eq.id && o.type == eq.type) {
                org.tendrilTargets.append(o.id);
            }
        }
        
        // Mocking Genetic Memory (History) for each Organism
        if (eq.status == "Under Maintenance" || eq.id % 3 == 0) {
            MaintenanceHistoryRecord rec;
            rec.id = QRandomGenerator::global()->bounded(1000);
            rec.equipmentId = eq.id;
            rec.datePerformed = QDate::currentDate().addDays(-(QRandomGenerator::global()->bounded(60)));
            rec.actionTaken = eq.status == "Under Maintenance" ? "Critical system failure. Repair engaged." : "Cellular checkup.";
            rec.status = eq.status == "Under Maintenance" ? "Resolving" : "Resolved";
            org.geneticMemory.append(rec);
        }
        
        m_organisms.append(org);
    }

    // Sort organisms by severity to group critical red nodes together for the "infected zone" look
    std::sort(m_organisms.begin(), m_organisms.end(), [](const MaintenanceOrganism &a, const MaintenanceOrganism &b) {
        return a.severity > b.severity;
    });

    // UPDATE COLONY UI PANEL (Left)
    QVBoxLayout *lay = qobject_cast<QVBoxLayout*>(m_infoContainer->layout());
    while(lay->count()){ delete lay->takeAt(0)->widget(); }

    int crit = 0, warn = 0;
    m_healEquipCombo->clear();
    
    for (const auto &org : m_organisms) {
        if(org.severity == 3) crit++; 
        else if(org.severity == 2) warn++;
        
        m_healEquipCombo->addItem(org.equipmentName, org.equipmentId);
        
        QFrame *f = new QFrame(); 
        QColor bod = (org.severity == 3) ? QColor("#D32F2F") : QColor("#C17F3E");
        f->setStyleSheet(QString(
            "QFrame { background: rgba(20,15,10,0.8); border: 1px solid %1; border-radius: 8px; margin-bottom: 8px; } ")
            .arg(bod.name()));
        
        QVBoxLayout *fl = new QVBoxLayout(f);
        fl->setContentsMargins(12, 12, 12, 12);
        fl->setSpacing(4);
        
        QLabel *name = new QLabel(org.equipmentName, f);
        name->setStyleSheet("color: #E0D0B0; font-weight: bold; font-size: 13px; border: none; background: transparent;");
        fl->addWidget(name);
        
        QLabel *status = new QLabel(org.severity == 3 ? "CRITICAL" : "WARNING", f);
        status->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border: none; background: transparent;").arg(bod.name()));
        fl->addWidget(status);
        
        QLabel *senseList = new QLabel(org.senses.first().name, f);
        senseList->setStyleSheet("color: #8A7A6A; font-size: 11px; border: none; background: transparent;");
        fl->addWidget(senseList);
        
        lay->addWidget(f);
    }
    lay->addStretch();

    m_colonyStatsLabel->setText(QString(
        "Active organisms: %1\n%2 critical, %3 warning.\nSpread risk: %4\nImmediate attention required.")
        .arg(m_organisms.size()).arg(crit).arg(warn).arg(crit > 0 ? "HIGH" : "MEDIUM"));

    if (!m_organisms.isEmpty() && m_organisms.first().severity >= 2) {
        const auto &org = m_organisms.first();
        m_speechBubble->setText(QString(
            "\"I am %1. I have grown large inside your workshop. My strongest signal: %2. "
            "If nobody acts soon, I will spread further. I am not threatening you. I am telling you what I am.\"").arg(org.equipmentName, org.senses.first().name));
    } else if (warn > 0) {
         m_speechBubble->setText("\"Something is beginning here. A pale ember that will soon consume the Relational nodes. Healing protocols advised.\"");
    } else {
        m_speechBubble->setText("Nexus colony scan complete. Zero anomalies detected in the workshop biomass. The workshop is breathing safely.");
    }
}

void MaintenanceOrganismWidget::updateOrganisms() { if(m_engine) loadData(m_engine); }

// ============================================================================
// COLONY RING PAINTER
// ============================================================================
void MaintenanceOrganismWidget::drawColonyPainter(QPainter &p) {
    double cw = m_ringArea->width(), ch = m_ringArea->height();
    p.fillRect(0, 0, cw, ch, QColor("#0D0905")); // The warm dark from the pic

    if (m_organisms.isEmpty()) return;

    // Draw the internal dashed biometric line
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(138, 122, 106, 60), 1, Qt::DashLine));
    p.drawEllipse(QRectF(cw/2.0 - cw*0.32, ch/2.0 - ch*0.25, cw*0.64, ch*0.50));
    p.restore();

    m_nodePositions.clear();
    for (int i = 0; i < m_organisms.size(); ++i) {
        double angle = (2.0 * M_PI * i) / m_organisms.size() - M_PI/2.0;
        // Adjusted ellipse to be more balanced and leave room for outer labels
        m_nodePositions[m_organisms[i].equipmentId] = QPointF(cw/2.0 + cw*0.32 * cos(angle), ch/2.0 + ch*0.25 * sin(angle));
    }
    
    // Draw cells (back to front for proper blending)
    for (int i = m_organisms.size()-1; i >= 0; --i) {
        drawOrganism(p, m_organisms[i], m_nodePositions[m_organisms[i].equipmentId]);
    }
}

void MaintenanceOrganismWidget::drawOrganism(QPainter &p, const MaintenanceOrganism &org, const QPointF &pos) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    
    int index = 0;
    for (int i = 0; i < m_organisms.size(); ++i) {
        if (m_organisms[i].equipmentId == org.equipmentId) {
            index = i;
            break;
        }
    }
    
    static QList<QRectF> s_drawnLabels;
    if (index == 0) s_drawnLabels.clear();

    double angle = (2.0 * M_PI * index) / m_organisms.size() - M_PI/2.0;
    double dx = cos(angle), dy = sin(angle);
    double r = (org.severity == 3) ? 38.0 : 32.0; // Slightly smaller to reveal names
    r *= org.growthFactor;
    
    QColor c;
    if (org.severity == 3) c = QColor(255, 34, 0, 150);
    else if (org.severity == 2) c = QColor(212, 134, 10, 130);
    else c = QColor(100, 80, 50, 40);

    // Biological Membrane
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 3; ++i) {
        double off = m_globalPulsePhase + (i * M_PI * 0.618);
        QPointF wobble(cos(off)*4, sin(off)*4);
        QRadialGradient g(pos + wobble, r * 1.3);
        g.setColorAt(0, c);
        g.setColorAt(1, Qt::transparent);
        p.setBrush(g);
        p.drawEllipse(pos + wobble, r*1.1, r*1.1);
    }
    
    // Core
    QRadialGradient core(pos, r * 0.4);
    QColor coreC = (org.severity == 3) ? QColor(255, 255, 255, 180) : QColor(255, 230, 180, 150);
    if (org.severity < 2) coreC = QColor(200, 180, 160, 60);
    core.setColorAt(0, coreC);
    core.setColorAt(1, Qt::transparent);
    p.setBrush(core);
    p.drawEllipse(pos, r*0.3, r*0.3);

    // Label Placement - Pushing OUTSIDE the ring to solve the overlap mess
    if (org.severity >= 2) {
        QString badgeText = (org.severity == 3) ? "CRITICAL" : "WARNING";
        QColor badgeColor = (org.severity == 3) ? QColor(255, 34, 0) : QColor(212, 134, 10);
        
        QFont fontName("Segoe UI", 9, QFont::Bold);
        p.setFont(fontName);
        QFontMetrics fm(fontName);
        int tw = fm.horizontalAdvance(org.equipmentName);
        
        QFont fontBadge("Segoe UI", 7, QFont::Bold);
        p.setFont(fontBadge);
        QFontMetrics fmb(fontBadge);
        int bw = fmb.horizontalAdvance(badgeText) + 8;
        
        int totalW = tw + bw + 6;
        int totalH = 16;

        // Smart positioning: Start outside the glow and move further if blocked
        double labelDist = r + 25.0;
        bool placed = false;
        QRectF finalRect;

        while (!placed) {
            QPointF lp = pos + QPointF(dx * labelDist, dy * labelDist);
            QRectF candidate(lp.x() - totalW/2.0, lp.y() - totalH/2.0, totalW, totalH);
            
            bool clash = false;
            for (const auto &prev : s_drawnLabels) {
                if (candidate.intersects(prev.adjusted(-4,-2,4,2))) {
                    clash = true; break;
                }
            }
            
            if (clash && labelDist < 200) {
                labelDist += 12.0;
            } else {
                finalRect = candidate;
                placed = true;
            }
        }
        s_drawnLabels.append(finalRect);

        // Draw name
        p.setPen(Qt::white);
        p.setFont(fontName);
        p.drawText(finalRect.left(), finalRect.center().y() + 4, org.equipmentName);
        
        // Draw badge pill
        QRectF bRect(finalRect.left() + tw + 4, finalRect.center().y() - 6, bw, 13);
        p.setPen(Qt::NoPen);
        p.setBrush(badgeColor);
        p.drawRoundedRect(bRect, 3, 3);
        
        p.setPen(Qt::white);
        p.setFont(fontBadge);
        p.drawText(bRect, Qt::AlignCenter, badgeText);
    }

    p.restore();
}

// ============================================================================
// CALENDAR VIEW PAINTER
// ============================================================================
void MaintenanceOrganismWidget::drawCalendarPainter(QPainter &p) {
    const QRectF viewport(0, 0, m_calendarArea->width(), m_calendarArea->height());
    p.save();
    p.fillRect(viewport, QColor("#0D0805"));

    const QRectF content = viewport.adjusted(10, 8, -10, -8);
    if (m_calendarSlideDir != 0) {
        const qreal ease = QEasingCurve(QEasingCurve::OutCubic).valueForProgress(m_calendarSlideProgress);
        const qreal w = content.width();
        const qreal oldX = (m_calendarSlideDir > 0) ? -w * ease : w * ease;
        const qreal newX = (m_calendarSlideDir > 0) ? (w - w * ease) : (-w + w * ease);

        drawCalendarMonth(p, content.translated(oldX, 0), m_prevCalendarMonth);
        drawCalendarMonth(p, content.translated(newX, 0), m_calendarMonth);
    } else {
        drawCalendarMonth(p, content, m_calendarMonth);
    }
    p.restore();
}

QList<MaintenanceOrganismWidget::CalendarEvent> MaintenanceOrganismWidget::buildCalendarEvents(const QDate &month) const {
    QList<CalendarEvent> events;
    QSqlQuery q;
    q.prepare("SELECT EQUIPMENT_TYPE, STATUS, PURCHASE_DATE, NEXT_MAINTENANCE FROM EQUIPMENT WHERE STATUS != 'Retired'");
    if (!q.exec()) {
        return events;
    }

    while (q.next()) {
        const QString equipment = q.value(0).toString();
        const QString status = q.value(1).toString();

        QDate purchaseDate = q.value(2).toDate();
        if (!purchaseDate.isValid()) {
            purchaseDate = q.value(2).toDateTime().date();
        }

        QDate nextMaintenance = q.value(3).toDate();
        if (!nextMaintenance.isValid()) {
            nextMaintenance = q.value(3).toDateTime().date();
        }

        if (purchaseDate.isValid() && purchaseDate.year() == month.year() && purchaseDate.month() == month.month()) {
            events.append({purchaseDate, equipment, "Scheduled", QColor("#3B82F6")});
        }

        if (nextMaintenance.isValid() && nextMaintenance.year() == month.year() && nextMaintenance.month() == month.month()) {
            if (status.compare("Under Maintenance", Qt::CaseInsensitive) == 0) {
                events.append({nextMaintenance, equipment, "Critical", QColor("#CC2200")});
            } else {
                events.append({nextMaintenance, equipment, "Routine", QColor("#4CAF7D")});
            }
        }

        if (!nextMaintenance.isValid() && status.compare("Under Maintenance", Qt::CaseInsensitive) == 0) {
            const QDate today = QDate::currentDate();
            if (today.year() == month.year() && today.month() == month.month()) {
                events.append({today, equipment, "Critical", QColor("#CC2200")});
            }
        }
    }
    return events;
}

void MaintenanceOrganismWidget::drawCalendarMonth(QPainter &p, const QRectF &rect, const QDate &month) {
    const QRectF panel = rect.adjusted(2, 2, -2, -2);

    const QRectF monthHeader(panel.left(), panel.top(), panel.width(), 52);
    const QRectF prevRect(monthHeader.left() + 8, monthHeader.top() + 4, 40, 36);
    const QRectF nextRect(monthHeader.right() - 48, monthHeader.top() + 4, 40, 36);

    p.setPen(QPen(QColor("#C17F3E"), 2));
    QFont arrowFont("Segoe UI", 24, QFont::Bold);
    p.setFont(arrowFont);
    p.drawText(prevRect, Qt::AlignCenter, "<");
    p.drawText(nextRect, Qt::AlignCenter, ">");

    QFont monthFont("Segoe UI", 22, QFont::Bold);
    monthFont.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
    p.setFont(monthFont);
    p.setPen(QColor("#C17F3E"));
    p.drawText(monthHeader, Qt::AlignCenter, month.toString("MMMM yyyy").toUpper());

    const QList<CalendarEvent> events = buildCalendarEvents(month);
    int routineCount = 0;
    int criticalCount = 0;
    int scheduledCount = 0;
    for (const auto &ev : events) {
        if (ev.kind == "Critical") criticalCount++;
        else if (ev.kind == "Scheduled") scheduledCount++;
        else routineCount++;
    }

    qreal pillX = panel.left() + 18;
    const qreal pillY = monthHeader.bottom() + 4;
    auto drawSummaryPill = [&](int count, const QString &label, const QColor &color) {
        if (count <= 0) return;
        const QString txt = QString("%1 %2").arg(count).arg(label);
        QFont f("Segoe UI", 9, QFont::Bold);
        p.setFont(f);
        const int w = QFontMetrics(f).horizontalAdvance(txt) + 20;
        QRectF r(pillX, pillY, w, 20);
        p.setPen(QPen(color, 1));
        QColor bg = color;
        bg.setAlpha(51);  // exactly 20% opacity
        p.setBrush(bg);
        p.drawRoundedRect(r, 10, 10);
        p.setPen(color);
        p.drawText(r, Qt::AlignCenter, txt);
        pillX += w + 8;
    };
    drawSummaryPill(routineCount, "Routine", QColor("#4CAF7D"));
    drawSummaryPill(criticalCount, "Critical", QColor("#CC2200"));
    drawSummaryPill(scheduledCount, "Scheduled", QColor("#3B82F6"));

    const qreal gridTop = monthHeader.bottom() + 30;
    const qreal gridBottom = panel.bottom() - 6;
    const qreal dayHeaderH = 26;
    const qreal gridY = gridTop + dayHeaderH;
    const qreal gridH = qMax<qreal>(120, gridBottom - gridY);
    const qreal cellW = panel.width() / 7.0;
    const qreal cellH = qMin<qreal>(80.0, gridH / 6.0);

    QStringList headers = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    QFont dayHeaderFont("Segoe UI", 10, QFont::Bold);
    dayHeaderFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    p.setFont(dayHeaderFont);
    p.setPen(QColor(245, 230, 211, 153));  // exactly 60% opacity
    for (int c = 0; c < 7; ++c) {
        QRectF hRect(panel.left() + c * cellW, gridTop, cellW, dayHeaderH);
        p.drawText(hRect, Qt::AlignCenter, headers[c]);
    }

    QHash<int, QList<CalendarEvent>> byDay;
    for (const auto &ev : events) byDay[ev.date.day()].append(ev);

    const QDate first(month.year(), month.month(), 1);
    const int daysInMonth = month.daysInMonth();
    const int startCol = (first.dayOfWeek() + 6) % 7;
    const QDate today = QDate::currentDate();

    int day = 1;
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 7; ++c) {
            const int idx = r * 7 + c;
            QRectF cell(panel.left() + c * cellW + 2, gridY + r * cellH + 2, cellW - 4, cellH - 4);

            if (idx < startCol || day > daysInMonth) {
                // Empty cells: completely transparent, no border, no background
                continue;
            }

            p.setPen(QPen(QColor(26, 18, 8, 80), 1));
            p.setBrush(QColor(13, 8, 5, 120));

            const bool isToday = (today.year() == month.year() && today.month() == month.month() && today.day() == day);
            if (isToday) {
                p.setPen(QPen(QColor("#C17F3E"), 1.5));
                p.setBrush(QColor(26, 18, 8, 180));
            }
            p.drawRoundedRect(cell, 4, 4);

            QFont dayNumFont("Segoe UI", 12, QFont::Bold);
            p.setFont(dayNumFont);
            QRectF dayBadge(cell.left() + 6, cell.top() + 4, 30, 18);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 85));
            p.drawRoundedRect(dayBadge, 3, 3);
            p.setPen(isToday ? QColor("#C17F3E") : QColor("#FFF4E5"));
            p.drawText(dayBadge, Qt::AlignCenter, QString::number(day));

            if (isToday) {
                QRectF badge(cell.right() - 52, cell.top() + 6, 46, 16);
                p.setPen(Qt::NoPen);
                p.setBrush(QColor("#C17F3E"));
                p.drawRoundedRect(badge, 4, 4);
                p.setPen(Qt::white);
                p.setFont(QFont("Segoe UI", 8, QFont::Bold));
                p.drawText(badge, Qt::AlignCenter, "TODAY");
            }

            const auto list = byDay.value(day);
            const int maxVisible = qMin(3, list.size());
            const qreal pillH = 18;
            qreal pillY = cell.bottom() - (maxVisible * (pillH + 2)) - 4;
            for (int i = 0; i < maxVisible; ++i) {
                const CalendarEvent &ev = list[i];
                QRectF pr(cell.left() + 4, pillY, cell.width() - 8, pillH);
                p.setPen(Qt::NoPen);
                p.setBrush(ev.color);
                p.drawRoundedRect(pr, 4, 4);
                p.setPen(Qt::white);
                p.setFont(QFont("Segoe UI", 9, QFont::Bold));
                const QString txt = QString::fromUtf8("🔧 ") + ev.equipment.left(14);
                p.drawText(pr.adjusted(4, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft, txt);
                pillY += pillH + 2;
            }

            if (list.size() > maxVisible) {
                p.setPen(QColor("#F5E6D3"));
                p.setFont(QFont("Segoe UI", 8, QFont::Bold));
                p.drawText(cell.adjusted(6, 0, -6, -2), Qt::AlignLeft | Qt::AlignBottom,
                           QString("+%1").arg(list.size() - maxVisible));
            }

            // Draw day number last so pills never hide it.
            QFont dayNumTopFont("Segoe UI", 12, QFont::Bold);
            p.setFont(dayNumTopFont);
            QRectF dayTopBadge(cell.left() + 6, cell.top() + 4, 30, 18);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 95));
            p.drawRoundedRect(dayTopBadge, 3, 3);
            p.setPen(isToday ? QColor("#C17F3E") : QColor("#FFF4E5"));
            p.drawText(dayTopBadge, Qt::AlignCenter, QString::number(day));

            day++;
        }
    }
}

void MaintenanceOrganismWidget::navigateCalendarMonth(int delta) {
    if (delta == 0 || (m_calendarAnimTimer && m_calendarAnimTimer->isActive())) {
        return;
    }
    m_prevCalendarMonth = m_calendarMonth;
    m_calendarMonth = m_calendarMonth.addMonths(delta);
    m_calendarSlideDir = (delta > 0) ? 1 : -1;
    m_calendarSlideProgress = 0.0;
    if (m_calendarAnimTimer) {
        m_calendarAnimTimer->start();
    } else if (m_calendarArea) {
        m_calendarArea->update();
    }
}

// ============================================================================
// HISTORY VIEW PAINTER
// ============================================================================
void MaintenanceOrganismWidget::drawHistoryPainter(QPainter &p) {
    double cw = m_historyArea->width(), ch = m_historyArea->height();
    
    p.save();
    p.fillRect(0, 0, cw, ch, QColor(15, 12, 8));

    p.setPen(QColor(212, 175, 55));
    p.setFont(QFont("Segoe UI", 18, QFont::Bold));
    p.drawText(36, 44, "GENETIC MEMORY ARCHIVE");

    int yOffset = 78;
    for (const auto &org : m_organisms) {
        for (const auto &rec : org.geneticMemory) {
            QRectF card(30, yOffset - 18, cw - 60, 58);
            p.setPen(QPen(QColor("#5A4A32"), 1.2));
            p.setBrush(QColor(20, 15, 10, 205));
            p.drawRoundedRect(card, 10, 10);

            drawHistoryNode(p, rec, QPointF(56, yOffset + 10));
            
            p.setPen(QColor("#F5E6D3"));
            p.setFont(QFont("Segoe UI", 11, QFont::Bold));
            p.drawText(90, yOffset + 4, org.equipmentName + " — " + rec.actionTaken);
            
            p.setPen(QColor("#B8925A"));
            p.setFont(QFont("Segoe UI", 9));
            p.drawText(120, yOffset + 25, "Event Date: " + rec.datePerformed.toString("yyyy-MM-dd") + " | Status: " + rec.status);
            
            yOffset += 68;
        }
    }
    
    p.restore();
}

void MaintenanceOrganismWidget::drawHistoryNode(QPainter &p, const MaintenanceHistoryRecord &rec, const QPointF &pos) {
    p.save();
    QColor c = (rec.status == "Resolving") ? QColor(244, 67, 54, 180) : QColor(76, 175, 80, 180);
    p.setBrush(c);
    p.setPen(QPen(Qt::white, 2));
    p.drawEllipse(pos, 20, 20);
    
    double pulse = 3 * sin(m_globalPulsePhase * 2);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(pos, 20 + pulse, 20 + pulse);
    p.restore();
}

// ============================================================================
// EVENTS
// ============================================================================
bool MaintenanceOrganismWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_ringArea) {
        if (event->type() == QEvent::Paint) {
            QPainter p(m_ringArea); p.setRenderHint(QPainter::Antialiasing);
            drawColonyPainter(p);
            return true;
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            m_lastMousePos = me->pos();
            int hovered = -1;
            // Iterate in reverse to catch top-most (Criticals drawn last)
            for (int i = m_organisms.size() - 1; i >= 0; --i) {
                const auto &org = m_organisms[i];
                if (QLineF(m_nodePositions[org.equipmentId], m_lastMousePos).length() < 50) {
                    hovered = org.equipmentId;
                    break;
                }
            }
            if (m_hoveredOrganism != hovered) {
                m_hoveredOrganism = hovered;
                m_ringArea->update();
            }
            return true;
        } else if (event->type() == QEvent::MouseButtonPress) {
            QWidget *schedPanel = findChild<QWidget*>("schedPanel");
            if (m_hoveredOrganism != -1) {
                if (schedPanel && schedPanel->isHidden()) schedPanel->show();
                
                int cmbIdx = m_healEquipCombo->findData(m_hoveredOrganism);
                if (cmbIdx != -1) m_healEquipCombo->setCurrentIndex(cmbIdx);
                
                for (const auto &org : m_organisms) {
                    if (org.equipmentId == m_hoveredOrganism) {
                        if (m_healDeadline) {
                            // Auto-suggest deadline by severity: critical sooner, warning later.
                            const int days = (org.severity >= 3) ? 2 : (org.severity == 2 ? 7 : 14);
                            m_healDeadline->setDate(QDate::currentDate().addDays(days));
                        }
                        m_speechBubble->setText(QString("\"I am %1. I have grown large inside your workshop. My strongest signal: %2. If nobody acts soon, I will spread further. I am not threatening you. I am telling you what I am.\"").arg(org.equipmentName, org.senses.first().name));
                        break;
                    }
                }
            } else {
                if (schedPanel && !schedPanel->isHidden()) schedPanel->hide();
            }
            return true;
        }
    } else if (obj == m_calendarArea && event->type() == QEvent::Paint) {
        QPainter p(m_calendarArea); p.setRenderHint(QPainter::Antialiasing);
        drawCalendarPainter(p);
        return true;
    } else if (obj == m_calendarArea && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        const QRectF viewport(0, 0, m_calendarArea->width(), m_calendarArea->height());
        const QRectF content = viewport.adjusted(10, 8, -10, -8);
        const QRectF monthHeader(content.left(), content.top(), content.width(), 52);
        const QRectF prevRect(monthHeader.left() + 8, monthHeader.top() + 4, 40, 36);
        const QRectF nextRect(monthHeader.right() - 48, monthHeader.top() + 4, 40, 36);

        if (prevRect.contains(me->position())) {
            navigateCalendarMonth(-1);
            return true;
        }
        if (nextRect.contains(me->position())) {
            navigateCalendarMonth(1);
            return true;
        }
    } else if (obj == m_historyArea && event->type() == QEvent::Paint) {
        QPainter p(m_historyArea); p.setRenderHint(QPainter::Antialiasing);
        drawHistoryPainter(p);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void MaintenanceOrganismWidget::paintEvent(QPaintEvent *) {
    QPainter p(this); 
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(15, 12, 8));
    bg.setColorAt(1, QColor(25, 20, 15));
    p.fillRect(rect(), bg);
}

void MaintenanceOrganismWidget::mouseMoveEvent(QMouseEvent *event) {
    // Tracking for organ inspection
    m_lastMousePos = event->pos();
}

void MaintenanceOrganismWidget::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    // Organism clicked
}

void MaintenanceOrganismWidget::resizeEvent(QResizeEvent *) {}
void MaintenanceOrganismWidget::drawTendril(QPainter &, const QPointF &, const QPointF &, int) {}
