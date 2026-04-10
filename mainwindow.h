#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QHorizontalBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChart>
#include <QStyledItemDelegate>
#include <QGraphicsDropShadowEffect>
#include <QToolTip>
#include <QMouseEvent>
#include <QTabWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QVideoFrame>
#include <QPropertyAnimation>
#include <QTimer>
#include <QTimeEdit>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QPageLayout>
#include <QDir>
#include <QDateTime>
#include <QImage>
#include <QFileInfo>
#include <QFile>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QShortcut>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaFormat>
#include <QAudioBuffer>
#include <QCompleter>
#include <QStringListModel>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QFrame>
#include <QRandomGenerator>
#include <QHash>
#include <QSet>
#include <QTableView>

// --- Voice Waveform Widget ---
class VoiceWaveformWidget : public QWidget {
    Q_OBJECT
public:
    VoiceWaveformWidget(QWidget *parent = nullptr) : QWidget(parent), m_pos(0) {
        setFixedSize(220, 40);
        // Generate random waveform
        for(int i=0; i<30; ++i) m_vals << QRandomGenerator::global()->bounded(5, 25);
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, [this](){ m_pos = (m_pos + 1) % 30; update(); });
    }
    void startAnim() { m_timer->start(80); }
    void stopAnim() { m_timer->stop(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        for(int i=0; i<m_vals.size(); ++i) {
            int h = m_vals[i];
            QRect bar(i*7 + 5, 20 - h/2, 4, h);
            if(i <= m_pos) {
                p.setBrush(QColor("#D4AF37"));
                p.setPen(Qt::NoPen);
            } else {
                p.setBrush(QColor(139, 111, 71, 100));
                p.setPen(Qt::NoPen);
            }
            p.drawRoundedRect(bar, 2, 2);
        }
    }
private:
    QList<int> m_vals;
    int m_pos;
    QTimer *m_timer;
};

// --- Equipment Hover Card ---
class EquipmentHoverCard : public QFrame {
    Q_OBJECT
public:
    EquipmentHoverCard(QWidget *parent = nullptr) : QFrame(parent) {
        setFixedSize(220, 100);
        setStyleSheet("background: #2C2418; border: 2px solid #D4AF37; border-radius: 12px; color: white;");
        QVBoxLayout *l = new QVBoxLayout(this);
        m_title = new QLabel(this); m_title->setStyleSheet("font-weight: bold; color: gold;");
        m_detail = new QLabel(this); m_detail->setStyleSheet("font-size: 11px;");
        l->addWidget(m_title); l->addWidget(m_detail);
        hide();
    }
    void showCard(const QString &title, const QString &detail, QPoint pos) {
        m_title->setText(title); m_detail->setText(detail);
        move(pos); show(); raise();
    }
    void showCard(int equipmentId, QPoint pos) {
        QSqlQuery q;
        q.prepare("SELECT EQUIPMENT_TYPE, STATUS, QUANTITY FROM EQUIPMENT WHERE EQUIPMENT_ID = :id");
        q.bindValue(":id", equipmentId);
        if (q.exec() && q.next()) {
            m_title->setText(QString("Equipment #%1").arg(equipmentId));
            m_detail->setText(QString("%1\nStatus: %2 | Qty: %3")
                .arg(q.value(0).toString())
                .arg(q.value(1).toString())
                .arg(q.value(2).toInt()));
            move(pos); show(); raise();
        } else {
            hide();
        }
    }
private:
    QLabel *m_title, *m_detail;
};


#include "qrcodegen.h"

#include "loginwindow.h"
#include "homewindow.h"
#include "weatherassistant.h"
#include "nexuswidget.h"
#include "costswidget.h"
#include "voicecommandengine.h"

QT_BEGIN_NAMESPACE
namespace Ui { 
    class MainWindow; 
    class ClientManagement;
    class EmployeeManagement;
    class EquipmentManagement;
    class OrderManagement;
    class SupplierManagement;
}
QT_END_NAMESPACE

class QListWidgetItem;
class QNetworkAccessManager;
class QNetworkReply;
class QTableWidget;
class QGraphicsBlurEffect;
class QNetworkReply;

// --- Custom Animated Widgets for Stats ---
class AnimatedDonutChart : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animationValue READ animationValue WRITE setAnimationValue)
public:
    struct DataPoint {
        QString label;
        double value;
        QColor color;
    };
    explicit AnimatedDonutChart(QWidget *parent = nullptr);
    void setData(const QList<DataPoint> &data);
    void startAnimation();
    qreal animationValue() const { return m_animationValue; }
    void setAnimationValue(qreal value) { m_animationValue = value; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;

private:
    QList<DataPoint> m_data;
    qreal m_animationValue = 0;
    int m_hoveredIndex = -1;
    QString m_tooltipText;
};

class StatCard : public QFrame {
    Q_OBJECT
public:
    explicit StatCard(const QString &title, const QString &value, const QString &trendText, bool isUp, QWidget *parent = nullptr);
    void updateData(const QString &value, const QString &trendText, bool isUp);
private:
    QLabel *m_titleLbl;
    QLabel *m_valLbl;
    QLabel *m_trendLbl;
};

// --- Hover Highlighter for Tables ---
class RowHoverDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    RowHoverDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent), m_hoveredRow(-1) {}
    void setHoveredRow(int row) { m_hoveredRow = row; }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        bool isSelected = opt.state & QStyle::State_Selected;
        bool isHovered = (index.row() == m_hoveredRow);

        if (isSelected || isHovered) {
            QRect r = opt.rect.adjusted(2, 2, -2, -2);
            QLinearGradient grad(r.topLeft(), r.bottomRight());
            
            if (isSelected && isHovered) {
                grad.setColorAt(0, QColor(212, 175, 55, 120)); // Super Gold
                grad.setColorAt(1, QColor(139, 111, 71, 60));
                painter->setPen(QPen(QColor("#D4AF37"), 2));
            } else if (isSelected) {
                grad.setColorAt(0, QColor(139, 111, 71, 100)); // Deep Amber
                grad.setColorAt(1, QColor(44, 34, 21, 80));
                painter->setPen(QPen(QColor(212, 175, 55, 100), 1));
            } else { // Hovered only
                grad.setColorAt(0, QColor(212, 175, 55, 60));
                grad.setColorAt(1, QColor(212, 175, 55, 15));
                painter->setPen(QPen(QColor(212, 175, 55, 80), 0.5));
            }
            
            painter->setBrush(grad);
            painter->drawRoundedRect(r, 8, 8);
        }
        
        painter->restore();
        
        // Remove standard selection drawing
        opt.state &= ~QStyle::State_Selected;
        QStyledItemDelegate::paint(painter, opt, index);
    }
private:
    int m_hoveredRow;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupClientStats();
    void setupClientManagement();
    void setupClientDataMatrix();
    void setupEquipmentStats();
    void setupSupplierStats();
    void setupEmployeeStats();
    void setupClientCalendar();
    void showTutorialOverlay(const QString &text);
    void setupEmployeeModes();
    void toggleEmployeeFields(bool active);
    void onEmployeeEnsureHistoryTable();
    void logActivity(const QString &action, const QString &module = "General");
    void setupSupplierModes();
    void setupEquipmentModes();
    void setupOrderModes();
    void setupGlobalStyles();
    QPixmap getCircularPixmap(const QPixmap &src);
    void setupTabNavigation(QWidget* parentWidget, QTabWidget* tabWidget, const QStringList& tabNames, int startX, int yPos, const QList<int>& targetIndices = {}, int spacing = 115, int afterFirstShift = 0);
    void switchLanguage(const QString &language);
        bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void on_login_clicked();
    
    // --- Home Screen Navigation ---
    void on_gs_employes_clicked();
    void on_gs_client_clicked();
    void on_gs_fournisseur_clicked();
    void on_gs_equipment_clicked();
    void on_gs_order_clicked();

    // --- Sidebar Navigation ---
    void on_nav_employees_clicked();
    void on_nav_clients_clicked();
    void on_nav_suppliers_clicked();
    void on_nav_equipments_clicked();
    void on_nav_orders_clicked();

    // --- System Navigation ---
    void on_btn_logout_clicked();
    void on_btn_home_clicked();
    
    // --- Order Management ---
    void onOrderClearFields();
    void onOrderAdd();
    void onOrderModify();
    void onOrderDelete();
    void onOrderDeleteAll();
    void onOrderLoad();
    void onOrderRefreshCatalog();
    void onOrderSearchCatalog();
    void onOrderExportCatalog();
    void onOrderImportCatalog();
    void onOrderPrintCatalog();
    
    // --- QR Code ---
    void onGenerateQR();
    void onSaveQR();
    void onPrintQR();
    
    // --- Client Management ---
    void onClientClearFields();
    void onClientModClearFields();
    void onClientAdd();
    void onClientModify();
    void onClientDelete();
    void onClientRefreshView();
    void onClientSearch();
    void onClientExportPDF();
    void onClientRowSelected(const QModelIndex &index);
    void onClientCyberTraceRefresh();
    void onClientSendMail();
    void onClientBrowseMail();
    // --- Voice Commands ---
    void onVoiceCommand(const QString &text);
    void onVoiceListeningChanged(bool active);

    // --- Employee Management ---
    void onEmployeeClearFields();
    void onEmployeeAdd();
    void onEmployeeModify();
    void onEmployeeDelete();
    void onEmployeeRefreshView();
    void onEmployeeRefreshHistory();
    void updateSalaryInsight();
    void onSuggestSalary();
    void onStatsAiClicked();
    void onAIPulseClicked();
    void onAiPerformanceClicked();
    void onEmployeeSearch();
    void onEmployeeRowSelected(const QModelIndex &index);
    void onEmployeeSendMail();
    void onEmployeeExportPDF();
    void onEmployeeExportHistoryPDF();
    void onEmployeeHistorySearch();
    void onEmployeeMailTemplateChanged(int index);
    void processEmpCameraFrame();
    void callAiModel(const QString &sysPrompt, const QString &userPrompt, std::function<void(QString)> callback);
    
    // --- Supplier Management ---
    void onSupplierClearFields();
    void onSupplierAdd();
    void onSupplierModify();
    void onSupplierDelete();
    void onSupplierDeleteAll();
    void onSupplierExportPDF();
    void onSupplierPrint();
    void onSupplierLoad(const QModelIndex &index);
    void onSupplierSearch();
    void onSupplierRefreshView();
    void onSupplierSendSMS();
    void onSupplierUploadImage();
    
    // Supplier Map
    void setupSupplierMapTab();
    void refreshSupplierMap();
    void onSupplierGeocodeFinished(QNetworkReply *reply);
    void checkSupplierVicinity(int supplierId = -1);
    void loadSupplierMapPins();
    void onSupplierBellClicked();
    void checkAndPostSupplierNotifications();
    
    // Delivery rating system
    void onSupplierEnsureReviewsTable();
    void onSupplierReviewLoad();
    void onSupplierReviewSubmit();
    void onSupplierReviewRatingChanged(int value);
    void onSupplierPopulateRatingCombos();
    
    // --- Equipment Management ---
    void onEquipmentClearFields();
    void onEquipmentShareToChat();
    void onEquipmentAdd();
    void onEquipmentModify();
    void onEquipmentDelete();
    void onEquipmentRefreshView();
    void onEquipmentSearch();
    void onEquipmentHistoryRefresh();
    void onEquipmentHistorySearch();
    void onEquipmentHistoryClear();
    void onEquipmentCustomContextMenu(const QPoint &pos);
    void onEquipmentHistoryCustomContextMenu(const QPoint &pos, int tableIdx);
    void onEquipmentExportPDF();
    void onEquipmentExportStatsPDF();
    void onEquipmentBulkUpdateStatus();
    void onEquipmentDeleteAll();
    
    // --- Employee Chat ---
    void onChatEnsureTable();
    void onChatSendMessage();
    void onChatRefresh();
    void onChatEmployeeListRefresh();
    void onChatEmployeeSelected(QListWidgetItem *item);
    void onChatAttachImage();
    void onChatDeleteMessage(int index);
    void onChatSettingsClicked();
    void onChatEmojiClicked();
    void onChatGifClicked();
    void onChatSearchToggle();
    void onWeatherAssistantClicked();
    void setupChatForgeVisuals();
    void enforceChatTabTopOffset();
    void onMapNetworkFinished(QNetworkReply *reply);
    
    // Voice Slotes
    void onChatStartRecord();
    void onChatStopRecord();
    void onChatVoiceToggled();

    // --- Face Recognition & Avatar ---
    void onUploadAvatar();
    void onScanFace();
    void updateUserProfileDisplay();
    // --- Language Management ---
    void onLanguageChanged(const QString &language);
    void onPageChanged(int index);

private:
    Ui::MainWindow *ui;

    void setupOrderCatalogResolutionTabs();
    void configureOrderCatalogTable(QTableWidget *table);
    bool populateOrderCatalogTable(QTableWidget *table, const QString &searchText, bool resolvedOnly);
    void markOrderAsPaid(int orderId);
    void markOrderAsUnpaid(int orderId);

    void retranslateDynamicRadios(QWidget *container);
    void setTabTextTr(QTabWidget *tabWidget, QWidget *tabPage, const QString &key);
    void retranslateDynamicWidgets(QWidget *container);
    void retranslateDynamicTabs(QTabWidget *tabWidget);
    void retranslateDynamicCharts(QWidget *container);
    void showWelcomeNotification(QWidget *parent, const QString &managementName);
    
    // UI Pointers for Modules
    Ui::ClientManagement *ui_client;
    Ui::EmployeeManagement *ui_employee;
    Ui::EquipmentManagement *ui_equipment;
    Ui::OrderManagement *ui_order;
    Ui::SupplierManagement *ui_supplier;

    // Widget containers for Modules
    QWidget *clientPage;
    QWidget *employeePage;
    QWidget *equipmentPage;
    QWidget *orderPage;
    QWidget *supplierPage;
    QTabWidget *m_orderCatalogStatusTabs = nullptr;
    QTableWidget *m_orderCatalogUnresolvedTable = nullptr;
    QTableWidget *m_orderCatalogResolvedTable = nullptr;

    LoginWindow *loginWindow;
    HomeWindow *homeWindow;
    WeatherAssistant *weatherAssistant;
    NexusWidget *m_nexusWidget = nullptr;
    CostsWidget *m_costsWidget = nullptr;
    VoiceCommandEngine *m_voiceEngine = nullptr;
    QPushButton        *m_micBtn      = nullptr;
    
    QTranslator *translator;
    QString currentLanguage;
    
    // Current logged-in employee ID
    int currentEmployeeId;
    int currentChatPartnerId;
    bool m_homeWelcomeShown = false;
    QByteArray pendingChatImage;
    bool m_isChatModernTheme = false;
    QString m_lastDroppedImagePath;
    
    // Voice Recording
    QMediaCaptureSession *m_captureSession = nullptr;
    QMediaRecorder *m_recorder = nullptr;
    QAudioInput *m_audioInput = nullptr;
    bool m_isRecording = false;
    
    // Smart Chat
    QCompleter *m_chatCompleter = nullptr;
    QStringListModel *m_completerModel = nullptr;
    EquipmentHoverCard *m_hoverCard = nullptr;
    QWidget *m_chatAmbientLayer = nullptr;
    
    // Employee Management Face Recognition
    QCamera *m_empCamera = nullptr;
    QMediaCaptureSession *m_empCaptureSession = nullptr;
    QVideoSink *m_empVideoSink = nullptr;
    bool m_isEmpFaceScanActive = false;
    int m_faceScanStage = 0; // 0: Center, 1: Left, 2: Right
    QString m_faceScanStatus;
    
    void shakeWidget(QWidget *w);
    
    // GIF / Emoji
    QNetworkAccessManager *giphyNetworkManager = nullptr;
    
    // Chat Timer
    QTimer *chatRefreshTimer;
    
    // Audio components
    QMediaPlayer *loginAudioPlayer;
    QAudioOutput *loginAudioOutput;
    QMediaPlayer *homeAudioPlayer;
    QAudioOutput *homeAudioOutput;
    QMediaPlayer *tutorialLoopAudioPlayer;
    QAudioOutput *tutorialLoopAudioOutput;
    QMediaPlayer *chatAudioPlayer;
    QAudioOutput *chatAudioOutput;
    qreal currentVolume;
    
    // Fade animation helpers
    void fadeOutAndPlay(QMediaPlayer *fadeOutPlayer, QAudioOutput *fadeOutOutput,
                        QMediaPlayer *fadeInPlayer, QAudioOutput *fadeInOutput);
    void fadeOut(QAudioOutput *output, std::function<void()> onComplete);
    void fadeIn(QAudioOutput *output);
    void showUnreadMessagesSplash();
    
    // Equipment Form Progress & Animation
    QProgressBar *m_equipProgress = nullptr;
    QLabel *m_eqTypeInd = nullptr, *m_eqDateInd = nullptr, *m_eqPriceInd = nullptr, *m_eqDescInd = nullptr;
    void updateEquipProgress();
    
    QProgressBar *m_supplierProgress = nullptr;
    QLabel *m_suppNameInd = nullptr, *m_suppEmailInd = nullptr, *m_suppTelInd = nullptr, *m_suppTypeInd = nullptr;
    void updateSupplierProgress();

    void playEquipSuccessAnimation(const QString &equipName);
    void playSupplierSuccessAnimation(const QString &supplierName);
    void playSupplierModifyAnimation(const QString &supplierName);
    void playSupplierDeleteAnimation(const QString &supplierName);

    void setupOrderMapTab();
    void requestMapForBuyerId();
    void populateMapClients();
    void requestMapTiles(double lat, double lon);
    void renderOrderMap();

    QNetworkAccessManager *m_mapNet = nullptr;
    QLabel *m_mapImageLabel = nullptr;
    QLabel *m_mapStatusLabel = nullptr;
    QLabel *m_mapAddressLabel = nullptr;
    QPushButton *m_mapRefreshBtn = nullptr;
    QTableWidget *m_mapClientTable = nullptr;
        QPushButton *m_mapZoomInBtn = nullptr;
        QPushButton *m_mapZoomOutBtn = nullptr;
    QPushButton *m_mapFullscreenBtn = nullptr;
    QDialog *m_mapFullscreenDialog = nullptr;
    QLabel *m_mapFullscreenLabel = nullptr;
    QGraphicsBlurEffect *m_mapBlurEffect = nullptr;
    QHash<QString, QPixmap> m_mapTileCache;
    QSet<QString> m_mapPendingTiles;
    int m_mapZoom = 14;
    QSize m_mapImageSize = QSize(640, 360);
    double m_mapTopLeftX = 0.0;
    double m_mapTopLeftY = 0.0;
    int m_mapTileX0 = 0;
    int m_mapTileY0 = 0;
    int m_mapTileX1 = 0;
    int m_mapTileY1 = 0;
    int m_mapTileErrors = 0;
    int m_mapExpectedTiles = 0;
    int m_mapLoadedTiles = 0;
        double m_mapCenterLat = 36.8065;
        double m_mapCenterLon = 10.1815;
        bool m_mapHasClientPin = false;
        double m_mapClientPinLat = 0.0;
        double m_mapClientPinLon = 0.0;
        bool m_mapDragging = false;
        QPoint m_mapDragStart;
        double m_mapDragCenterX = 0.0;
        double m_mapDragCenterY = 0.0;
        QPoint m_mapDragOffset;
        QPixmap m_mapCurrentPixmap;
        bool m_mapHasPixmap = false;
        
    // --- Supplier Map ---
    struct SupplierPin {
        int id;
        QString name;
        QString type;
        QString status;
        QString openTime;
        QString closeTime;
        double lat;
        double lon;
        QRect rect;
    };
    QList<SupplierPin> m_supplierPins;
    int m_supplierGeocodePendingCount = 0;

    QNetworkAccessManager *m_supplierMapNet = nullptr;
    QLabel *m_supplierMapImageLabel = nullptr;
    QLabel *m_supplierMapStatusLabel = nullptr;
    QPushButton *m_supplierMapRefreshBtn = nullptr;
    QPushButton *m_supplierMapZoomInBtn = nullptr;
    QPushButton *m_supplierMapZoomOutBtn = nullptr;
    QTimeEdit *m_teOpeningHour = nullptr;
    QTimeEdit *m_teClosingHour = nullptr;
    QPushButton *m_supplierBellBtn = nullptr;
    
    QHash<QString, QPixmap> m_supplierMapTileCache;
    QSet<QString> m_supplierMapPendingTiles;
    int m_supplierMapZoom = 13;
    QSize m_supplierMapImageSize = QSize(800, 500);
    double m_supplierMapTopLeftX = 0.0;
    double m_supplierMapTopLeftY = 0.0;
    int m_supplierMapTileX0 = 0;
    int m_supplierMapTileY0 = 0;
    int m_supplierMapTileX1 = 0;
    int m_supplierMapTileY1 = 0;
    int m_supplierMapTileErrors = 0;
    
    double m_supplierCenterLat = 36.8065; // Tunis default
    double m_supplierCenterLon = 10.1815;
    bool m_supplierMapDragging = false;
    QPoint m_supplierMapDragStart;
    double m_supplierMapDragCenterX = 0.0;
    double m_supplierMapDragCenterY = 0.0;
    QPoint m_supplierMapDragOffset;
    QPixmap m_supplierMapCurrentPixmap;
    bool m_supplierMapHasPixmap = false;
    
    // AI Summarization network manager
    QNetworkAccessManager *chatSummaryNetManager = nullptr;
    QNetworkAccessManager *aiNetworkManager = nullptr;
    QString aiApiKey;

    // Client Management Dynamic UIs
    QTableView *m_clientCyberTable = nullptr;
    QFrame *m_clientMatrixFrame = nullptr;
    
    // Presentation Mode
    bool m_isPresentationMode = false;
    QTimer *m_presentationTimer = nullptr;
    int m_presentationStep = 0;
    QWidget *m_presentationOverlay = nullptr;
    void togglePresentationMode();
    void advancePresentation();
    void startKenBurnsEffect();
    
protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void setAudioVolume(qreal volume);

signals:
    void audioVolumeChanged(qreal volume);
};
#endif // MAINWINDOW_H
