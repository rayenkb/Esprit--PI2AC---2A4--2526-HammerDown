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
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDesktopServices>
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
#include "equipment.h"
#include "supplier.h"
#include "order.h"
#include "employee.h"
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QFrame>
#include <QRandomGenerator>
#include <QHash>
#include <QSet>
#include <QTableView>
#include <QStackedWidget>
#include <QDateEdit>
#include <QTextEdit>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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
    MAINWINDOW_SUPPLIER_PUBLIC_DECLS
    MAINWINDOW_ORDER_PUBLIC_DECLS
    MAINWINDOW_EMPLOYEE_PUBLIC_DECLS
    void setupClientCalendar();
    void showTutorialOverlay(const QString &text);
    MAINWINDOW_EQUIPMENT_PUBLIC_DECLS
    void logActivity(const QString &action, const QString &module = "General", const QJsonObject &extra = QJsonObject());
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

    void on_gs_order_clicked();

    // --- Sidebar Navigation ---
    void on_nav_employees_clicked();
    void on_nav_clients_clicked();
    void on_nav_suppliers_clicked();

    void on_nav_orders_clicked();

    // --- System Navigation ---
    void on_btn_logout_clicked();
    void on_btn_home_clicked();
    
    // --- Order Management ---
    MAINWINDOW_ORDER_SLOT_DECLS

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
    MAINWINDOW_EMPLOYEE_SLOT_DECLS
    void callAiModel(const QString &sysPrompt, const QString &userPrompt, std::function<void(QString)> callback);
    void onTestArduino();
    void onTestArduinoScenario1();
    void onArduinoReadyRead();
    
    // --- Supplier Management ---
    MAINWINDOW_SUPPLIER_SLOT_DECLS

    MAINWINDOW_EQUIPMENT_SLOT_DECLS

    
    // --- Employee Chat ---
    


    // --- Face Recognition & Avatar ---
    void onUploadAvatar();
    void onScanFace();
    void on_userProfileClicked();
    void updateUserProfileDisplay();
    // --- Language Management ---
    void onLanguageChanged(const QString &language);
    void onPageChanged(int index);

private:
    Ui::MainWindow *ui;

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

    LoginWindow *loginWindow;
    HomeWindow *homeWindow;
    WeatherAssistant *weatherAssistant;
    VoiceCommandEngine *m_voiceEngine = nullptr;
    QPushButton        *m_micBtn      = nullptr;
    
    QTranslator *translator;
    QString currentLanguage;
    
    // Current logged-in employee ID
    int currentEmployeeId;
    bool m_homeWelcomeShown = false;
    QString m_lastDroppedImagePath;
    
    // Employee Management Face Recognition
    MAINWINDOW_EMPLOYEE_PRIVATE_DECLS
    
    // Audio components
    QMediaPlayer *loginAudioPlayer;
    QAudioOutput *loginAudioOutput;
    QMediaPlayer *homeAudioPlayer;
    QAudioOutput *homeAudioOutput;
    QMediaPlayer *tutorialLoopAudioPlayer;
    QAudioOutput *tutorialLoopAudioOutput;
    qreal currentVolume;
    
    bool m_homeAudioPausedBySettings = false;
    qint64 m_homeAudioSettingsResumePos = 0;
    bool m_homeAudioPausedByTutorial = false;
    qint64 m_homeAudioTutorialResumePos = 0;
    bool m_audioSuspendedForOstp = false;
    bool m_resumeLoginAfterOstp = false;
    bool m_resumeHomeAfterOstp = false;
    bool m_resumeTutorialAfterOstp = false;
    qint64 m_loginResumePosAfterOstp = 0;
    qint64 m_homeResumePosAfterOstp = 0;
    qint64 m_tutorialResumePosAfterOstp = 0;
    
    // Fade animation helpers
    void fadeOutAndPlay(QMediaPlayer *fadeOutPlayer, QAudioOutput *fadeOutOutput,
                        QMediaPlayer *fadeInPlayer, QAudioOutput *fadeInOutput);
    void fadeOut(QAudioOutput *output, std::function<void()> onComplete);
    void fadeIn(QAudioOutput *output);
    void pauseHomeAudioForSettings();
    void resumeHomeAudioAfterSettings();
    void pauseHomeAudioForTutorial();
    void resumeHomeAudioAfterTutorial();
    void suspendAudioForOstp();
    void restoreAudioAfterOstp();
    
    MAINWINDOW_EQUIPMENT_PRIVATE_DECLS


    MAINWINDOW_SUPPLIER_PRIVATE_DECLS

    MAINWINDOW_ORDER_PRIVATE_DECLS

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
    
    QSerialPort *arduino = nullptr;
    QStringList m_maintenanceEquipmentIds;

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void setAudioVolume(qreal volume);

signals:
    void audioVolumeChanged(qreal volume);
};
#endif // MAINWINDOW_H
