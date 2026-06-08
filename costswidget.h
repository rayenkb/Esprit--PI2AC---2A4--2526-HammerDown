#ifndef COSTSWIDGET_H
#define COSTSWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QButtonGroup>
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
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QPrinter>
#include <QThread>
#include <cmath>

// ============================================================================
// FINANCIAL DATA STRUCTURE
// ============================================================================
struct EquipmentFinancials {
    int id = 0;
    QString name;
    QString type;
    QString status;
    double purchasePrice = 0.0;
    double ageInYears = 0.0;
    double ageInDays = 0.0;
    int maintenanceEvents = 0;
    int historyEvents = 0;
    double estimatedMaintenanceCost = 0.0;
    double estimatedOperationalCost = 0.0;
    double totalTCO = 0.0;
    double costPerYear = 0.0;
    double costPerDay = 0.0;
    double bookValue = 0.0;
    double depreciationPct = 0.0;
    double roiScore = 0.0;
    double repairCost = 0.0;
    double replacementCost = 0.0;
    int remainingYears = 0;
    double monthlyBudgetNeeded = 0.0;
    QString recommendation;
    QString recommendationColor;
    QDate purchaseDate;
};

// ============================================================================
// COST CALCULATION ENGINE (runs in QThread)
// ============================================================================
class CostCalculationEngine : public QObject {
    Q_OBJECT
public:
    explicit CostCalculationEngine(QObject *parent = nullptr);
    void calculate();
    QList<EquipmentFinancials> results() const { return m_results; }
    double totalInventoryValue() const { return m_totalInventory; }
    double totalTCO() const { return m_totalTCO; }
    double forecast90Days() const { return m_forecast90; }
    QString topPerformer() const { return m_topPerformer; }
    int urgentActions() const { return m_urgentActions; }

signals:
    void calculationComplete();

private:
    QList<EquipmentFinancials> m_results;
    double m_totalInventory = 0.0;
    double m_totalTCO = 0.0;
    double m_forecast90 = 0.0;
    QString m_topPerformer;
    int m_urgentActions = 0;
};

// ============================================================================
// TCO SUB-TAB WIDGET
// ============================================================================
class TCOWidget : public QWidget {
    Q_OBJECT
public:
    explicit TCOWidget(QWidget *parent = nullptr);
    void setData(const QList<EquipmentFinancials> &data);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QList<EquipmentFinancials> m_data;
    int m_selectedIndex = -1;
    int m_hoveredIndex = -1;
    float m_animProgress = 0.0f;
    float m_detailAnimProgress = 0.0f;
    float m_globalTime = 0.0f;
    QTimer *m_animTimer;
    QScrollArea *m_listScroll = nullptr;
    int m_scrollOffset = 0;
    int m_listItemHeight = 58;

    void drawListPanel(QPainter &p);
    void drawDetailPanel(QPainter &p);
    void drawCostStack(QPainter &p, const QRectF &area, const EquipmentFinancials &eq);
    void drawMetricBox(QPainter &p, const QRectF &rect, const QString &label, const QString &value, const QColor &color, float progress);
    void drawDepreciationGauge(QPainter &p, const QPointF &center, double radius, const EquipmentFinancials &eq);
    void drawVerdictBox(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq);
    QColor getRecommendationColor(const QString &rec);
};

// ============================================================================
// REPAIR VS REPLACE SUB-TAB
// ============================================================================
class RepairReplaceWidget : public QWidget {
    Q_OBJECT
public:
    explicit RepairReplaceWidget(QWidget *parent = nullptr);
    void setData(const QList<EquipmentFinancials> &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<EquipmentFinancials> m_data;
    int m_selectedIndex = 0;
    float m_globalTime = 0.0f;
    float m_animProgress = 0.0f;
    QTimer *m_animTimer;
    QComboBox *m_equipCombo = nullptr;

    void drawRepairPanel(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq);
    void drawReplacePanel(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq);
    void drawVSSection(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq);
    void drawRecommendationBanner(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq);
    bool repairWins(const EquipmentFinancials &eq);
};

// ============================================================================
// BUDGET FORECAST SUB-TAB
// ============================================================================
class ForecastWidget : public QWidget {
    Q_OBJECT
public:
    explicit ForecastWidget(QWidget *parent = nullptr);
    void setData(const QList<EquipmentFinancials> &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct MonthForecast {
        QString monthName;
        double total = 0.0;
        QList<QPair<QString, double>> items;
    };

    QList<EquipmentFinancials> m_data;
    QList<MonthForecast> m_months;
    int m_periodMonths = 3;
    float m_globalTime = 0.0f;
    float m_animProgress = 0.0f;
    QTimer *m_animTimer;
    int m_expandedMonth = -1;
    int m_hoveredBar = -1;

    void recalculate();
    void drawBarChart(QPainter &p, const QRectF &area);
    void drawSummary(QPainter &p, const QRectF &area);
    void drawPeriodSelector(QPainter &p, const QRectF &area);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

// ============================================================================
// ROI DASHBOARD SUB-TAB
// ============================================================================
class ROIWidget : public QWidget {
    Q_OBJECT
public:
    explicit ROIWidget(QWidget *parent = nullptr);
    void setData(const QList<EquipmentFinancials> &data);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QList<EquipmentFinancials> m_data;
    float m_globalTime = 0.0f;
    float m_animProgress = 0.0f;
    QTimer *m_animTimer;
    int m_expandedCard = -1;
    int m_hoveredCard = -1;
    int m_scrollOffset = 0;

    void drawWorkshopGauge(QPainter &p, const QRectF &area, double avgScore);
    void drawROICard(QPainter &p, const QRectF &rect, const EquipmentFinancials &eq, int rank, bool expanded, bool hovered);
};

// ============================================================================
// REPLACEMENT TIMELINE SUB-TAB
// ============================================================================
class TimelineWidget : public QWidget {
    Q_OBJECT
public:
    explicit TimelineWidget(QWidget *parent = nullptr);
    void setData(const QList<EquipmentFinancials> &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<EquipmentFinancials> m_data;
    float m_globalTime = 0.0f;
    float m_animProgress = 0.0f;
    QTimer *m_animTimer;

    void drawSummaryBanner(QPainter &p, const QRectF &area);
    void drawTimeline(QPainter &p, const QRectF &area);
};

// ============================================================================
// REPORT GENERATOR SUB-TAB
// ============================================================================
class ReportWidget : public QWidget {
    Q_OBJECT
public:
    explicit ReportWidget(QWidget *parent = nullptr);
    void setData(const QList<EquipmentFinancials> &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<EquipmentFinancials> m_data;
    float m_globalTime = 0.0f;
    QTimer *m_animTimer;
    QLineEdit *m_titleInput = nullptr;
    QList<QCheckBox*> m_sectionChecks;
    QPushButton *m_generateBtn = nullptr;
    QPushButton *m_exportBtn = nullptr;
    float m_generateProgress = 0.0f;
    bool m_isGenerating = false;
    bool m_isGenerated = false;

    void generateReport();
    void exportPDF();
    void drawPreview(QPainter &p, const QRectF &area);
};

// ============================================================================
// GROQ FINANCIAL INSIGHT CLIENT
// ============================================================================
class CostGroqClient : public QObject {
    Q_OBJECT
public:
    explicit CostGroqClient(QObject *parent = nullptr);
    void setApiKey(const QString &apiKey) { m_apiKey = apiKey.trimmed(); }
    void requestInsight(const QString &sysPrompt, const QString &userPrompt);
    QString lastInsight() const { return m_lastInsight; }
    bool isLoading() const { return m_isLoading; }

signals:
    void insightReady(const QString &insight);

private:
    QNetworkAccessManager *m_network;
    QString m_apiKey;
    QString m_lastInsight;
    bool m_isLoading = false;
};

// ============================================================================
// MAIN COSTS WIDGET
// ============================================================================
class CostsWidget : public QWidget {
    Q_OBJECT
public:
    explicit CostsWidget(QWidget *parent = nullptr);
    void initialize();

signals:
    void costsReady();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Core
    QStackedWidget *m_stack = nullptr;
    QList<QPushButton*> m_tabButtons;
    QTimer *m_animTimer = nullptr;
    float m_globalTime = 0.0f;
    bool m_isLoaded = false;
    float m_loadingProgress = 0.0f;
    int m_loadingPhase = 0;
    QTimer *m_loadingTimer = nullptr;

    // Calculation engine
    CostCalculationEngine *m_engine = nullptr;
    QThread *m_calcThread = nullptr;

    // Sub-tab widgets
    TCOWidget *m_tcoWidget = nullptr;
    RepairReplaceWidget *m_repairWidget = nullptr;
    ForecastWidget *m_forecastWidget = nullptr;
    ROIWidget *m_roiWidget = nullptr;
    TimelineWidget *m_timelineWidget = nullptr;
    ReportWidget *m_reportWidget = nullptr;

    // Groq
    CostGroqClient *m_groqClient = nullptr;
    QString m_currentInsight;
    float m_insightCharIndex = 0.0f;
    QString m_displayedInsight;

    // KPI animation
    float m_kpiAnimProgress = 0.0f;
    QTimer *m_kpiAnimTimer = nullptr;

    // Header KPI values (cached)
    double m_totalInventory = 0.0;
    double m_totalTCO = 0.0;
    double m_forecast90 = 0.0;
    QString m_topPerformer;
    int m_urgentActions = 0;

    int m_activeSubTab = 0;

    void buildUI();
    void switchSubTab(int index);
    void drawHeader(QPainter &p);
    void drawKPI(QPainter &p, const QRectF &rect, const QString &value, const QString &label,
                 const QColor &color, int kpiIndex);
    void drawSubTabBar(QPainter &p);
    void drawBackground(QPainter &p);
    void drawInsightBar(QPainter &p);
    void runLoadingAnimation();
    void requestGroqInsight();
    void onDataReady();
    QString formatNumber(double val);
};

#endif // COSTSWIDGET_H
