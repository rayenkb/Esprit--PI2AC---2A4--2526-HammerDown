#ifndef NEXUSWIDGET_H
#define NEXUSWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QSlider>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QButtonGroup>
#include <QEvent>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDateEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QRandomGenerator>
#include <cmath>

class QLineEdit;
class NexusGroqClient;

// ============================================================================
// DATA STRUCTURES
// ============================================================================
struct NexusEquipment {
    int id;
    QString type;
    int quantity;
    double unitPrice;
    QString status;
    QString description;
    int employeeId;
    QDate purchaseDate;
    QString location;
    QString notes;
    QDate nextMaintenance;
    double coutAcquisition;
    QString responsable;
};

struct NexusEmployee {
    int id;
    QString firstName;
    QString lastName;
    QString jobTitle;
};

struct NexusInsight {
    enum Severity { CRITICAL = 0, WARNING = 1, INFO = 2, INSIGHT = 3 };
    Severity severity;
    QString title;
    QString explanation;
    int confidence;
    QList<int> relatedEquipment;
};

struct OrganismSense {
    QString name;
    QString reading;
    int riskLevel;
    QString finding;
};

struct MaintenanceHistoryRecord {
    int id;
    int equipmentId;
    QString actionTaken;
    QDate datePerformed;
    QString notes;
    QString status; // Resolving, Resolved, Failed
};

struct MaintenanceOrganism {
    int equipmentId;
    QString equipmentName;
    QDateTime bornDate;
    int severity; 
    QList<OrganismSense> senses;
    QList<int> tendrilTargets;
    bool isHealing;
    double healingPhase;
    qreal growthFactor;
    qreal pulseRate;
    QList<MaintenanceHistoryRecord> geneticMemory;
};

// ============================================================================
// INFERENCE ENGINE
// ============================================================================
class InferenceEngine : public QObject {
    Q_OBJECT
public:
    explicit InferenceEngine(QObject *parent = nullptr);
    void loadData();
    void runAllRules();
    QList<NexusInsight> insights() const { return m_insights; }
    int overallConfidence() const { return m_overallConfidence; }
    int dataPointsAnalyzed() const { return m_dataPoints; }
    double healthScore() const { return m_healthScore; }

signals:
    void insightsReady();

private:
    QList<NexusInsight> m_insights;
    QList<NexusEquipment> m_equipment;
    QList<NexusEmployee> m_employees;
    int m_overallConfidence;
    int m_dataPoints;
    double m_healthScore;

    void rule01_AgeRisk(); void rule02_CascadeRisk(); void rule03_EmployeeSpecialization();
    void rule04_OrphanedEquipment(); void rule05_ValueConcentration(); void rule06_OperationalDependency();
    void rule07_OverworkDetection(); void rule08_SilentEquipment(); void rule15_SinglePointOfFailure();
    void rule19_ValueVsUsage(); void rule09_GoldenAge(); void rule10_MaintenancePattern();
    void rule11_PriceAnomaly(); void rule12_RetirementWave(); void rule13_NewEquipmentCluster();
    void rule14_HealthTrend(); void rule16_EmployeeWorkload(); void rule17_LongMaintenance();
    void rule18_WorkshopComplexity(); void rule20_MomentumDetection();
};

// ============================================================================
// MAINTENANCE ORGANISM WIDGET (THE ORIGINAL EPIC VERSION)
// ============================================================================
class MaintenanceOrganismWidget : public QWidget {
    Q_OBJECT
public:
    explicit MaintenanceOrganismWidget(QWidget *parent = nullptr);
    void loadData(InferenceEngine *engine);
    void updateOrganisms();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct CalendarEvent {
        QDate date;
        QString equipment;
        QString kind;
        QColor color;
    };

    void drawColonyPainter(QPainter &p);
    void drawOrganism(QPainter &p, const MaintenanceOrganism &org, const QPointF &pos);
    void drawTendril(QPainter &p, const QPointF &start, const QPointF &end, int severity);
    void drawCalendarPainter(QPainter &p);
    void drawCalendarMonth(QPainter &p, const QRectF &rect, const QDate &month);
    void drawHistoryPainter(QPainter &p);
    void drawHistoryNode(QPainter &p, const MaintenanceHistoryRecord &rec, const QPointF &pos);
    QList<CalendarEvent> buildCalendarEvents(const QDate &month) const;
    void navigateCalendarMonth(int delta);
    void populateHealingScheduler();

    InferenceEngine *m_engine;
    QList<MaintenanceOrganism> m_organisms;
    QMap<int, QPointF> m_nodePositions;
    QTimer *m_pulseTimer;
    qreal m_globalPulsePhase;
    int m_hoveredOrganism;
    
    // UI Panels and areas
    QScrollArea *m_infoArea;
    QWidget *m_infoContainer;
    QLabel *m_speechBubble;
    QLabel *m_statusLabel;
    QLabel *m_colonyStatsLabel;
    
    // The 3 canvas areas
    QWidget *m_ringArea;
    QWidget *m_calendarArea;
    QWidget *m_historyArea;
    QStackedWidget *m_viewStack;
    
    // Scheduler side
    QVBoxLayout *m_scheduleLayout;
    QComboBox *m_healEquipCombo;
    QTextEdit *m_healNotes;
    QPushButton *m_healBtn;
    QDateEdit *m_healDeadline;
    QList<QCheckBox*> m_healChecklist;
    
    // Tab switching
    QPushButton *m_btnColony;
    QPushButton *m_btnCalendar;
    QPushButton *m_btnHistory;
    
    // History/Animation state
    QPointF m_lastMousePos;
    float m_calendarOffset;
    QDate m_calendarMonth;
    QDate m_prevCalendarMonth;
    int m_calendarSlideDir;
    qreal m_calendarSlideProgress;
    QTimer *m_calendarAnimTimer;
};

// ============================================================================
// OTHER WIDGETS
// ============================================================================
class KnowledgeGraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit KnowledgeGraphWidget(QWidget *parent = nullptr);
    void loadData();
    void highlightEquipment(int equipId);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
private:
    struct GraphNode { int id; QString label; bool isEquipment; QString status; double price; qreal x, y, vx, vy; qreal radius; QColor color; bool isGhost; qreal opacity; qreal glowPhase; bool isDragging; QString specialtyAura; };
    struct GraphEdge { int fromId, toId; int weight; QColor color; bool isDashed; QString label; };
    QList<GraphNode> m_nodes; QList<GraphEdge> m_edges; QList<NexusEquipment> m_equipment; QList<NexusEmployee> m_employees;
    QTimer *m_physicsTimer; int m_hoveredNode; int m_draggedNode; QPointF m_lastMouse; int m_frameTime;
    void buildGraph(); void computeHiddenConnections(); void stepPhysics(); void drawNode(QPainter&, const GraphNode&, bool); void drawEdge(QPainter&, const GraphEdge&); void drawHoverCard(QPainter&, const GraphNode&); int nodeAt(QPointF); QColor statusColor(const QString&);
};

class TimeMachineWidget : public QWidget {
    Q_OBJECT
public:
    explicit TimeMachineWidget(QWidget *parent = nullptr);
    void loadData();
    void goToDate(const QDate &date);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QList<NexusEquipment> m_equipment; QDate m_minDate, m_maxDate, m_currentDate; bool m_isPlaying; QTimer *m_playTimer;
    QLabel *m_dateLabel; QLabel *m_statsLabel; QSlider *m_timeSlider; QPushButton *m_playBtn; QScrollArea *m_cardArea; QWidget *m_cardContainer;
    void reconstructState(const QDate &date); QString statusAtDate(int, const QDate&); QFrame* createEquipmentCard(const NexusEquipment&, const QString&, const QDate&);
};

class InferenceDisplayWidget : public QWidget {
    Q_OBJECT
public:
    explicit InferenceDisplayWidget(QWidget *parent = nullptr);
    void setInsights(const QList<NexusInsight> &insights);
private:
    QScrollArea *m_scrollArea; QWidget *m_container; QVBoxLayout *m_layout; QList<QFrame*> m_cards; QTimer *m_animTimer; int m_visibleCount; QFrame* createInsightCard(const NexusInsight&);
};

class IntelligenceReportWidget : public QWidget {
    Q_OBJECT
public:
    explicit IntelligenceReportWidget(QWidget *parent = nullptr);
    void generateReport(InferenceEngine *engine);
    void setFullReport(const QString &html);
private:
    QTextEdit *m_reportView; QPushButton *m_regenerateBtn; QPushButton *m_exportBtn; InferenceEngine *m_engine; QString m_fullReport; int m_charIndex; QTimer *m_typewriterTimer; QString buildReportText();
};

class PatternArchaeologyWidget : public QWidget {
    Q_OBJECT
public:
    explicit PatternArchaeologyWidget(QWidget *parent = nullptr);
    void loadData(InferenceEngine *engine);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    struct Layer { QString title, subtitle; QColor color; QStringList entries; bool expanded; QRectF rect; };
    QList<Layer> m_layers; QTimer *m_revealTimer; int m_revealedLayers; qreal m_revealProgress; int m_expandedLayer;
};

class DecisionMapperWidget : public QWidget {
    Q_OBJECT
public:
    explicit DecisionMapperWidget(QWidget *parent = nullptr);
    void loadData(InferenceEngine *engine);
    void displayAiPrediction(const QString &text);
private:
    InferenceEngine *m_engine; QComboBox *m_actionCombo, *m_equipCombo, *m_statusCombo; QPushButton *m_analyzeBtn; QScrollArea *m_resultArea; QWidget *m_resultContainer; QWidget *m_rippleWidget;
    struct ConsequenceResult { int equipCountBefore, equipCountAfter; double valueBefore, valueAfter; double healthBefore, healthAfter; bool singlePointRisk, valueConcentrationRisk, dependencyRisk; QString recommendation, explanation; };
    ConsequenceResult m_lastResult; qreal m_rippleProgress; bool m_hasResult;
    void analyzeConsequences(); void displayResults(const ConsequenceResult&);
};

class AiForgeWidget : public QWidget {
    Q_OBJECT
public:
    explicit AiForgeWidget(QWidget *parent = nullptr);
    void setGroqClient(NexusGroqClient *client) { m_groqClient = client; }
    void startEntrance();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTick();
    void onSend();
    void onGroqResponse(const QString &text);

private:
    enum Mode { Advisor = 0, DeepAnalysis = 1, DailyBrief = 2 };
    struct ActivityRow {
        QString equipment;
        QString action;
        QString at;
        QColor color;
    };

    struct Msg {
        bool user = false;
        bool thinking = false;
        QString text;
        int shownChars = 0;
        QDateTime ts;
        int dataPoints = 0;
        int cps = 55;
        QWidget *rowWidget = nullptr;
        QLabel *bodyLabel = nullptr;
        QLabel *thinkingDotsLabel = nullptr;
        QLabel *thinkingCaptionLabel = nullptr;
    };

    QWidget *m_leftPanel = nullptr;
    QWidget *m_centerPanel = nullptr;
    QWidget *m_rightPanel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QFrame *m_healthBox = nullptr;
    QFrame *m_equipmentBox = nullptr;
    QFrame *m_alertBox = nullptr;
    QFrame *m_totalValueBox = nullptr;
    QLabel *m_healthValue = nullptr;
    QLabel *m_equipmentValue = nullptr;
    QLabel *m_alertValue = nullptr;
    QLabel *m_totalValueValue = nullptr;
    QLabel *m_contextStatus = nullptr;
    QWidget *m_recentHost = nullptr;
    QVBoxLayout *m_recentLayout = nullptr;
    QWidget *m_messagesHost = nullptr;
    QVBoxLayout *m_messagesLayout = nullptr;
    QScrollArea *m_messagesScroll = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendBtn = nullptr;
    QPushButton *m_modeAdvisorBtn = nullptr;
    QPushButton *m_modeDeepBtn = nullptr;
    QPushButton *m_modeBriefBtn = nullptr;
    QPushButton *m_quickFixBtn = nullptr;
    QPushButton *m_quickHealthBtn = nullptr;
    QPushButton *m_quickRoiBtn = nullptr;
    QPushButton *m_quickActionBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;

    QTimer *m_tickTimer = nullptr;
    QTimer *m_typingTimer = nullptr;
    QTimer *m_thinkingTimer = nullptr;
    NexusGroqClient *m_groqClient = nullptr;
    QList<Msg> m_msgs;
    Mode m_mode = Advisor;
    float m_globalTime = 0.0f;
    float m_entrance = 0.0f;
    bool m_waitingForAi = false;

    int m_equipmentCount = 0;
    int m_historyCount = 0;
    int m_activeAlerts = 0;
    int m_availableCount = 0;
    int m_inUseCount = 0;
    int m_maintenanceCount = 0;
    double m_totalValue = 0.0;
    double m_totalValueDisplay = 0.0;
    int m_healthScore = 0;
    int m_dataPoints = 0;
    QStringList m_equipmentNames;
    QList<ActivityRow> m_recentActivity;
    QString m_cachedSmartContext;
    QString m_urgentMaintenanceEquipment;
    int m_urgentMaintenanceDays = 0;
    QString m_pendingQuestion;
    QString m_activeQuestion;
    bool m_dbRefreshInFlight = false;
    bool m_dbRefreshQueued = false;
    int m_thinkingPhase = -1;
    int m_typingMessageIndex = -1;
    int m_thinkingAnimPhase = 0;

    void buildUi();
    void refreshStatsFromDb();
    void rebuildRecentActivity();
    QString buildSmartContext() const;
    QString buildSystemPrompt() const;
    QString buildUserPrompt(const QString &question) const;
    QString buildFallbackResponse(const QString &reason, const QString &question = QString()) const;
    QString formatAiRichText(const QString &text) const;
    void addUserMessage(const QString &text);
    void addThinkingMessage();
    void resolveThinkingMessage(const QString &text);
    void removeThinkingMessage();
    void appendMessageWidget(int index);
    void clearMessageWidgets();
    void rebuildMessageWidgets();
    void startTypewriterForLastMessage();
    void updateTypingFrame();
    void updateThinkingFrame();
    void scrollMessagesToBottom();
    void sendQuestion(const QString &question);
    void setMode(Mode mode);
    void updateModeButtons();
    void updateInteractiveState();
    void updateStatsAnimation();
};

class ParallelFuturesWidget : public QWidget {
    Q_OBJECT
public:
    explicit ParallelFuturesWidget(QWidget *parent = nullptr);
    void setGroqClient(NexusGroqClient *client) { m_groqClient = client; }
    void bindEngine(InferenceEngine *engine);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTick();
    void onGenerate();
    void onGroqDebateResponse(const QString &text);

private:
    struct FutureScenario {
        QString name;
        QString posture;
        int cost = 0;
        int downtime = 0;
        int risk = 0;
        int health = 0;
        QString narrative;
    };

    InferenceEngine *m_engine = nullptr;
    NexusGroqClient *m_groqClient = nullptr;
    QTimer *m_tickTimer = nullptr;

    QWidget *m_leftPanel = nullptr;
    QWidget *m_centerPanel = nullptr;
    QWidget *m_rightPanel = nullptr;
    QLineEdit *m_questionInput = nullptr;
    QComboBox *m_horizonCombo = nullptr;
    QPushButton *m_generateBtn = nullptr;
    QPushButton *m_randomizeBtn = nullptr;
    QLabel *m_healthChip = nullptr;
    QLabel *m_assetsChip = nullptr;
    QLabel *m_maintenanceChip = nullptr;
    QLabel *m_valueChip = nullptr;
    QScrollArea *m_cardsScroll = nullptr;
    QWidget *m_cardsHost = nullptr;
    QVBoxLayout *m_cardsLayout = nullptr;
    QTextEdit *m_debateView = nullptr;
    QLabel *m_verdictLabel = nullptr;
    QLabel *m_statusLabel = nullptr;

    QList<FutureScenario> m_scenarios;
    QString m_lastQuestion;
    int m_equipmentCount = 0;
    int m_maintenanceCount = 0;
    int m_activeAlerts = 0;
    int m_healthScore = 0;
    double m_totalValue = 0.0;
    float m_phase = 0.0f;
    bool m_loading = false;

    void buildUi();
    void refreshMetrics();
    void rebuildScenarioCards();
    QList<FutureScenario> buildScenarios(const QString &question) const;
    QString buildDebatePrompt() const;
    QString buildLocalDebate() const;
    void setLoadingState(bool loading);
};

class NexusGroqClient : public QObject {
    Q_OBJECT
public:
    explicit NexusGroqClient(QObject *parent = nullptr);
    void setApiKey(const QString &key) { m_apiKey = key.trimmed(); }
    void sendPrompt(const QString &sys, const QString &usr, QObject* rc, const char* sl);
private:
    QString m_apiKey; QNetworkAccessManager *m_network;
};

class NexusWidget : public QWidget {
    Q_OBJECT
public:
    explicit NexusWidget(QWidget *parent = nullptr);
    void initialize();
    void runAiReport();
    void runAiInference();
    void runAiDecision(const QString &action, int equipId);
    void highlightEquipmentInGraph(int equipId);
    void goToTimeMachineDate(const QDate &date);
signals:
    void nexusReady();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QStackedWidget *m_stack; QList<QPushButton*> m_tabButtons; QLabel *m_statusBar; QTimer *m_loadingTimer; int m_loadingPhase; qreal m_loadingProgress; bool m_isLoaded;
    KnowledgeGraphWidget *m_graphWidget; TimeMachineWidget *m_timeMachine; InferenceDisplayWidget *m_inferenceDisplay; IntelligenceReportWidget *m_reportWidget; PatternArchaeologyWidget *m_archaeologyWidget; DecisionMapperWidget *m_decisionMapper; MaintenanceOrganismWidget *m_maintenanceWidget; AiForgeWidget *m_aiForgeWidget; ParallelFuturesWidget *m_parallelFuturesWidget;
    InferenceEngine *m_engine; QString m_groqApiKey; QNetworkAccessManager *m_networkManager; NexusGroqClient *m_groqClient;
    void switchTab(int index); void runLoadingAnimation(); void updateStatusBar(); void callGroq(const QString &prompt, std::function<void(QString)> callback);
};

#endif // NEXUSWIDGET_H
