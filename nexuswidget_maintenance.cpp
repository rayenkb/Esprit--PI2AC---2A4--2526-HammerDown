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
    : QWidget(parent), m_engine(nullptr), m_globalPulsePhase(0), m_hoveredOrganism(-1), m_calendarOffset(0)
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

    // Hide tabs to match the unified biological ring UI from the screenshot
    m_btnColony->hide();
    m_btnCalendar->hide();
    m_btnHistory->hide();

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
    
    m_healBtn = new QPushButton("Initiate Healing", schedPanel);
    m_healBtn->setFixedHeight(40);
    m_healBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; font-weight: bold; border-radius: 8px; font-size: 14px; } "
                             "QPushButton:hover { background: #45a049; }");
    m_scheduleLayout->addWidget(m_healBtn);
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
    double cw = m_calendarArea->width(), ch = m_calendarArea->height();
    
    p.save();
    p.fillRect(0, 0, cw, ch, QColor(20, 15, 10)); // Deep dark 
    
    // Draw the central timeline nerve
    p.setPen(QPen(QColor(184, 146, 90, 80), 4));
    p.drawLine(50, ch/2, cw-50, ch/2);
    
    int numPoints = 6;
    double spacing = (cw - 200) / numPoints;
    
    for (int i=0; i<numPoints; i++) {
        double x = 100 + i * spacing;
        double y = ch/2;
        
        QDate d = QDate::currentDate().addDays(i * 15);
        p.setPen(QColor(212, 175, 55));
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(x - 30, y + 30, d.toString("MMM dd"));
        
        // Draw calendar cell
        double pulse = 5 * sin(m_globalPulsePhase + i);
        p.setBrush(QColor(139, 111, 71, 150));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(x, y), 8 + pulse, 8 + pulse);
    }
    
    p.setPen(Qt::white);
    QFont f("Georgia", 16);
    f.setItalic(true);
    p.setFont(f);
    p.drawText(cw/2 - 150, 50, "The Organism Timeline (Predictive Pulse)");
    p.restore();
}

// ============================================================================
// HISTORY VIEW PAINTER
// ============================================================================
void MaintenanceOrganismWidget::drawHistoryPainter(QPainter &p) {
    double cw = m_historyArea->width(), ch = m_historyArea->height();
    
    p.save();
    p.fillRect(0, 0, cw, ch, QColor(15, 12, 8)); // Archival darkness
    
    p.setPen(QColor(212, 175, 55));
    QFont f("Georgia", 16);
    f.setItalic(true);
    p.setFont(f);
    p.drawText(50, 50, "Genetic Memory Archive (History)");

    int yOffset = 100;
    for (const auto &org : m_organisms) {
        for (const auto &rec : org.geneticMemory) {
            drawHistoryNode(p, rec, QPointF(50, yOffset));
            
            p.setPen(Qt::white);
            p.setFont(QFont("Segoe UI", 12, QFont::Bold));
            p.drawText(120, yOffset + 5, org.equipmentName + " — " + rec.actionTaken);
            
            p.setPen(QColor(184, 146, 90));
            p.setFont(QFont("Segoe UI", 10));
            p.drawText(120, yOffset + 25, "Event Date: " + rec.datePerformed.toString("yyyy-MM-dd") + " | Status: " + rec.status);
            
            yOffset += 70;
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
    // Organism clicked
}

void MaintenanceOrganismWidget::resizeEvent(QResizeEvent *) {}
void MaintenanceOrganismWidget::drawTendril(QPainter &, const QPointF &, const QPointF &, int) {}
