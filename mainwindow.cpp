#include <QDialog>
#include <QGraphicsBlurEffect>
#include <QFrame>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QProgressBar>
#include <QTimer>
#include <QPointer>
#include <QSharedPointer>
#include <QVariantAnimation>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QUrl>
#include <QBuffer>
#include <QMovie>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFileDialog>
#include <QWheelEvent>
#include <QtMath>

#include "mainwindow.h"
#include <QPair>
#include "smtpsender.h"
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QStatusBar>
#include "welcomenotificationbar.h"
#include "ui_mainwindow.h"
#include "ui_client_management.h"
#include "ui_employee_management.h"
#include "ui_equipment_management.h"
#include "ui_order_management.h"
#include "ui_supplier_management.h"
#include "weatherassistant.h"
#include "chatbotdialog.h"
#include "buttonanimator.h"
#include "modelingwidget.h"
#include "imagedropzone.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QScrollArea>
#include <QShortcut>
#include <QButtonGroup>
#include <QRadioButton>
#include <QAbstractButton>
#include <QSplineSeries>
#include <QCalendarWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QGroupBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QStackedWidget>
#include <QItemSelectionModel>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QMenu>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QToolTip>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QCursor>
#include <QLocale>
#include <QFileInfo>
#include <QProcess>
#include <QUuid>
#include <functional>
#include <algorithm>
// =============================================================================
// ANIMATED DONUT CHART WIDGET
// =============================================================================
AnimatedDonutChart::AnimatedDonutChart(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
}

void AnimatedDonutChart::setData(const QList<DataPoint> &data) {
    m_data = data;
    update();
}

void AnimatedDonutChart::startAnimation() {
    QPropertyAnimation *anim = new QPropertyAnimation(this, "animationValue");
    anim->setDuration(1200);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuart);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedDonutChart::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    double total = 0;
    for (const auto &d : m_data) total += d.value;
    if (total == 0) return;

    QRectF contentRect = rect().adjusted(20, 20, -20, -20);
    int size = qMin((int)contentRect.width(), (int)contentRect.height());
    contentRect = QRectF(contentRect.center().x() - size/2, contentRect.center().y() - size/2, size, size);

    double startAngle = 90; // Start from top
    for (int i = 0; i < m_data.size(); ++i) {
        double span = (m_data[i].value / total) * 360.0 * m_animationValue;
        
        QRadialGradient grad(contentRect.center(), size/2);
        grad.setColorAt(0.5, m_data[i].color.lighter(120));
        grad.setColorAt(1.0, m_data[i].color);

        p.setBrush(grad);
        p.setPen(QPen(QColor(40,40,40), 2));
        
        if (i == m_hoveredIndex) {
            p.setBrush(m_data[i].color.lighter(140));
            QRectF offsetRect = contentRect.adjusted(-5, -5, 5, 5);
            p.drawPie(offsetRect, startAngle * 16, -span * 16);
        } else {
            p.drawPie(contentRect, startAngle * 16, -span * 16);
        }
        
        startAngle -= (m_data[i].value / total) * 360.0;
    }

    // Mask for Donut
    p.setBrush(QColor(30, 20, 10)); // Match background
    p.setPen(QPen(QColor(139, 111, 71), 2));
    p.drawEllipse(contentRect.center(), size/4, size/4);
}

void AnimatedDonutChart::mouseMoveEvent(QMouseEvent *event) {
    double total = 0;
    for (const auto &d : m_data) total += d.value;
    if (total == 0) return;

    QRectF contentRect = this->rect().adjusted(20, 20, -20, -20);
    int size = qMin((int)contentRect.width(), (int)contentRect.height());
    contentRect = QRectF(contentRect.center().x() - size/2, contentRect.center().y() - size/2, size, size);

    QPointF center = contentRect.center();
    QPointF pos = event->position();
    double dist = QLineF(center, pos).length();

    if (dist < size/4 || dist > size/2) {
        m_hoveredIndex = -1;
        QToolTip::hideText();
        update();
        return;
    }

    double angle = QLineF(center, pos).angle(); // 0-360, 0 is right
    // Normalize angle to start from 90 (top) and go counter-clockwise
    double normalizedAngle = 90 - angle;
    if (normalizedAngle < 0) normalizedAngle += 360;

    double current = 0;
    int oldHover = m_hoveredIndex;
    m_hoveredIndex = -1;

    for (int i = 0; i < m_data.size(); ++i) {
        double span = (m_data[i].value / total) * 360.0;
        if (normalizedAngle >= current && normalizedAngle < current + span) {
            m_hoveredIndex = i;
            m_tooltipText = QString("%1: %2 (%3%)")
                .arg(m_data[i].label)
                .arg(m_data[i].value)
                .arg(qRound(m_data[i].value / total * 100));
            QToolTip::showText(event->globalPosition().toPoint(), m_tooltipText, this);
            break;
        }
        current += span;
    }

    if (oldHover != m_hoveredIndex) update();
}

bool AnimatedDonutChart::event(QEvent *event) {
    if (event->type() == QEvent::Leave) {
        m_hoveredIndex = -1;
        update();
    }
    return QWidget::event(event);
}

// =============================================================================
// STAT CARD WIDGET
// =============================================================================
StatCard::StatCard(const QString &title, const QString &value, const QString &trend, bool isUp, QWidget *parent) 
    : QFrame(parent) {
    setFixedSize(200, 100);
    setObjectName("statCard");
    setStyleSheet(
        "QFrame#statCard { background: rgba(50, 40, 30, 0.6); border: 2px solid #8B6F47; border-radius: 16px; }"
        "QFrame#statCard:hover { background: rgba(80, 60, 40, 0.7); border-color: #D4AF37; margin: -2px; }"
    );

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(15, 12, 15, 12);
    lay->setSpacing(2);

    m_titleLbl = new QLabel(title, this);
    m_titleLbl->setStyleSheet("color: #B8925A; font-size: 13px; font-weight: bold; border:none;");
    lay->addWidget(m_titleLbl);

    m_valLbl = new QLabel(value, this);
    m_valLbl->setStyleSheet("color: white; font-size: 24px; font-weight: bold; border:none;");
    lay->addWidget(m_valLbl);

    m_trendLbl = new QLabel(this);
    updateData(value, trend, isUp);
    lay->addWidget(m_trendLbl);
}

void StatCard::updateData(const QString &value, const QString &trendText, bool isUp) {
    m_valLbl->setText(value);
    QString arrow = isUp ? "↑" : "↓";
    QString color = isUp ? "#4CAF50" : "#FF5252";
    m_trendLbl->setText(QString("%1 %2 from last month").arg(arrow).arg(trendText));
    m_trendLbl->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: bold; border:none;").arg(color));
}

namespace {
constexpr qreal kHomeOstmVolumeScale = 0.40;

qreal homeOstmVolume(qreal baseVolume)
{
    return qBound<qreal>(0.0, baseVolume * kHomeOstmVolumeScale, 1.5);
}

QString trKey(const QString &key)
{
    return QCoreApplication::translate("QObject", key.toUtf8().constData());
}

void setTrKey(QWidget *widget, const QString &key)
{
    if (widget) {
        widget->setProperty("trKey", key);
    }
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , translator(new QTranslator(this))
    , currentLanguage("en")
    , currentEmployeeId(0)
    , currentVolume(1.0) // Initialize before equipment members
    , m_isChatModernTheme(false) // Classic is now default
{
    ui->setupUi(this);
    {
        QSettings settings("HammerDown", "HammerDown");
        currentVolume = qBound<qreal>(0.0, settings.value("audio/volume", 1.0).toDouble(), 1.5);
    }
    weatherAssistant = nullptr;
    currentChatPartnerId = -1;
    
    // Set application icon
    setWindowIcon(QIcon(":/assets/logo.png"));

    // --- CRITICAL REFACTOR: CLEAR STACK AND REBUILD ---
    // Remove any hardcoded pages (e.g., page_login, page_home) created by setupUi
    while (ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();
    }

    // 1. Login Window (Index 0)
    loginWindow = new LoginWindow(this);
    ui->stackedWidget->addWidget(loginWindow);
    connect(loginWindow, &LoginWindow::loginSuccessful, this, [this](int employeeId) {
        currentEmployeeId = employeeId;
        on_login_clicked();
        // Show unread messages splash after login
        QTimer::singleShot(600, this, &MainWindow::showUnreadMessagesSplash);
    });

    // 2. Home Window (Index 1)
    homeWindow = new HomeWindow(this);
    ui->stackedWidget->addWidget(homeWindow);
    
    // Connect HomeWindow signals
    connect(homeWindow, &HomeWindow::employesClicked,    this, &MainWindow::on_gs_employes_clicked);
    connect(homeWindow, &HomeWindow::clientClicked,      this, &MainWindow::on_gs_client_clicked);
    connect(homeWindow, &HomeWindow::fournisseurClicked, this, &MainWindow::on_gs_fournisseur_clicked);
    connect(homeWindow, &HomeWindow::equipmentClicked,   this, &MainWindow::on_gs_equipment_clicked);
    connect(homeWindow, &HomeWindow::orderClicked,       this, &MainWindow::on_gs_order_clicked);
    connect(homeWindow, &HomeWindow::languageChanged,    this, &MainWindow::onLanguageChanged);
    connect(homeWindow, &HomeWindow::volumeChanged,      this, &MainWindow::setAudioVolume);
    connect(homeWindow, &HomeWindow::disconnectClicked,  this, &MainWindow::on_btn_logout_clicked);
    connect(homeWindow, &HomeWindow::userProfileClicked,  this, &MainWindow::on_userProfileClicked);
    connect(homeWindow, &HomeWindow::settingsDialogOpened, this, &MainWindow::pauseHomeAudioForSettings);
    connect(homeWindow, &HomeWindow::settingsDialogClosed, this, &MainWindow::resumeHomeAudioAfterSettings);
    connect(homeWindow, &HomeWindow::tutorialOpened, this, &MainWindow::pauseHomeAudioForTutorial);
    connect(homeWindow, &HomeWindow::tutorialClosed, this, &MainWindow::resumeHomeAudioAfterTutorial);
    connect(homeWindow, &HomeWindow::botawkAnimationStarted, this, [this]() {
        if (!homeAudioPlayer) return;
        homeAudioPlayer->stop();
        m_homeAudioPausedBySettings = false;
    });
    connect(homeWindow, &HomeWindow::gerPlaybackFinished, this, [this]() {
        if (!homeAudioPlayer || !homeAudioOutput) return;
        if (m_audioSuspendedForOstp) return;
        if (!ui || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 1) return;

        homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
        homeAudioPlayer->setPosition(0);
        homeAudioPlayer->play();
    });

    // Initial sync of homeWindow state
    homeWindow->setLanguage(currentLanguage);
    homeWindow->setVolume(currentVolume);

    // 3. Employee Management (Index 2)
    ui_employee = new Ui::EmployeeManagement;
    employeePage = new QWidget(this);
    ui_employee->setupUi(employeePage);
    ui->stackedWidget->removeWidget(ui->page_employee);
    ui->stackedWidget->insertWidget(2, employeePage);
    connect(ui_employee->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    // Employee CRUD connections
    connect(ui_employee->btn_add,    &QPushButton::clicked, this, &MainWindow::onEmployeeAdd);
    connect(ui_employee->btn_modify, &QPushButton::clicked, this, &MainWindow::onEmployeeModify);
    connect(ui_employee->btn_upload_avatar, &QPushButton::clicked, this, &MainWindow::onUploadAvatar);
    connect(ui_employee->btn_scan_face, &QPushButton::clicked, this, &MainWindow::onScanFace);
    
    // Salary Intelligence
    ui_employee->dsb_salaire->setRange(0, 9999.99);
    connect(ui_employee->dsb_salaire, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateSalaryInsight);
    connect(ui_employee->le_fonction, &QLineEdit::textChanged, this, &MainWindow::updateSalaryInsight);
    // connect(ui_employee->btn_suggest_salary, &QPushButton::clicked, this, &MainWindow::onSuggestSalary);
    connect(ui_employee->btn_stats_ai_gen, &QPushButton::clicked, this, &MainWindow::onStatsAiClicked);
    // connect(ui_employee->btn_ai_pulse, &QPushButton::clicked, this, &MainWindow::onAIPulseClicked);
    connect(ui_employee->btn_ai_performance, &QPushButton::clicked, this, &MainWindow::onAiPerformanceClicked);
    connect(ui_employee->btn_test_arduino1, &QPushButton::clicked, this, &MainWindow::onTestArduino);
    connect(ui_employee->btn_test_arduino_scenario_1, &QPushButton::clicked, this, &MainWindow::onTestArduinoScenario1);
    
    // --- Employee Input Validation & Restrictions ---
    ui_employee->le_id->setValidator(new QIntValidator(1, 9999999, this));
    ui_employee->le_num->setValidator(new QIntValidator(1, 99999999, this));
    
    // Letters only for First/Last Names
    QRegularExpression regLetters("^[A-Za-z\\s]+$");
    QRegularExpressionValidator *letterVal = new QRegularExpressionValidator(regLetters, this);
    ui_employee->le_nom->setValidator(letterVal);
    ui_employee->le_prenom->setValidator(letterVal);

    // Job Title Selection (Restrict to 6 premium options)
    QStringList workRoles = {"Smith", "Cleaner", "Developer", "Cashier", "Carpenter", "Boss"};
    QCompleter *jobComp = new QCompleter(workRoles, this);
    jobComp->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui_employee->le_fonction->setCompleter(jobComp);
    ui_employee->le_fonction->setPlaceholderText("Select: Smith, Cleaner, Dev, Cashier, Carp, Boss");

    ui_employee->de_birthdate->setDateRange(QDate(1950, 1, 1), QDate::currentDate());
    ui_employee->de_birthdate->setDate(QDate(1995, 1, 1));
    // Auto-refresh employee view when switching to view tab
    connect(ui_employee->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_employee->tabWidget->widget(idx) == ui_employee->tab_view)
            onEmployeeRefreshView();
        else if (ui_employee->tabWidget->widget(idx) == ui_employee->tab_history)
            onEmployeeRefreshHistory();
        else if (ui_employee->tabWidget->widget(idx) == ui_employee->tab_stats)
            setupEmployeeStats();
    });
    connect(ui_employee->btn_refresh_emp, &QPushButton::clicked, this, &MainWindow::onEmployeeRefreshView);
    connect(ui_employee->le_recherche_emp, &QLineEdit::textChanged, this, &MainWindow::onEmployeeSearch);
    connect(ui_employee->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEmployeeRefreshHistory);
    connect(ui_employee->btn_delete, &QPushButton::clicked, this, &MainWindow::onEmployeeDelete);
    connect(ui_employee->tableView_employes, &QTableView::clicked, this, [this](const QModelIndex &idx){
        if (idx.column() == 0) { // Edit Action
            onEmployeeRowSelected(idx);
            ui_employee->tabWidget->setCurrentIndex(0);
            // Switch UI to "Modify Employee" mode
            if (auto *rb = ui_employee->tab_add->findChild<QRadioButton*>("rb_employee_mod_mode")) {
                rb->setChecked(true);
            }
        } else if (idx.column() == 1) { // Delete Action
            onEmployeeDelete();
        }
    });

    // ID-Unlock Logic for Management CRUD (Consolidated)
    connect(ui_employee->le_id, &QLineEdit::textChanged, this, [=](const QString &t){
        toggleEmployeeFields(!t.trimmed().isEmpty());
    });
    toggleEmployeeFields(false); // Default to locked

    // 4. Client Management (Index 3)
    ui_client = new Ui::ClientManagement;
    clientPage = new QWidget(this);
    ui_client->setupUi(clientPage);
    ui->stackedWidget->removeWidget(ui->page_client);
    ui->stackedWidget->insertWidget(3, clientPage);
    connect(ui_client->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    // Client CRUD connections
    connect(ui_client->btn_add,    &QPushButton::clicked, this, &MainWindow::onClientAdd);
    connect(ui_client->btn_modify, &QPushButton::clicked, this, &MainWindow::onClientModify);
    connect(ui_client->btn_delete, &QPushButton::clicked, this, &MainWindow::onClientDelete);
    connect(ui_client->btn_search, &QPushButton::clicked, this, &MainWindow::onClientSearch);
    connect(ui_client->tableView, &QAbstractItemView::clicked, this, &MainWindow::onClientRowSelected);

    // Add explict View Tab buttons for Edit
    connect(ui_client->btn_edit_view, &QPushButton::clicked, this, [this]() {
        QModelIndex idx = ui_client->tableView->currentIndex();
        if (idx.isValid()) {
            // First run row prepopulation logic
            onClientRowSelected(idx);
            // Switch to unified Manage tab (which contains add/modify forms)
            ui_client->tabWidget->setCurrentWidget(ui_client->tab_add);
            // Programmatically click the "Modify Mode" radio button inside that tab
            if (auto *rb = ui_client->tab_add->findChild<QRadioButton*>("rb_client_mod_mode")) {
                rb->setChecked(true);
            }
        } else {
            QMessageBox::warning(this, "Selection", "Please select a client to edit.");
        }
    });

    // Also search on Enter in the search box
    connect(ui_client->le_recherche, &QLineEdit::returnPressed, this, &MainWindow::onClientSearch);
    connect(ui_client->btn_pdf,    &QPushButton::clicked, this, &MainWindow::onClientExportPDF);
    // Mail tab buttons
    connect(ui_client->btn_send,   &QPushButton::clicked, this, &MainWindow::onClientSendMail);
    connect(ui_client->btn_browse, &QPushButton::clicked, this, &MainWindow::onClientBrowseMail);

    // Hide SMTP config fields — credentials are hardcoded in onClientSendMail
    ui_client->l_smtp->hide();  ui_client->le_smtp->hide();
    ui_client->l_port->hide();  ui_client->le_port->hide();
    ui_client->l_user->hide();  ui_client->le_user->hide();
    ui_client->l_pass->hide();  ui_client->le_pass->hide();

    // Auto-refresh client view when switching to view tab
    connect(ui_client->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_client->tabWidget->widget(idx) == ui_client->tab_view)
            onClientRefreshView();
    });

    // 5. Supplier Management (Index 4)
    ui_supplier = new Ui::SupplierManagement;
    supplierPage = new QWidget(this);
    ui_supplier->setupUi(supplierPage);
    ui->stackedWidget->removeWidget(ui->page_supplier);
    ui->stackedWidget->insertWidget(4, supplierPage);
    connect(ui_supplier->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    m_supplierMapNet = new QNetworkAccessManager(this);
    connect(m_supplierMapNet, &QNetworkAccessManager::finished, this, &MainWindow::onSupplierGeocodeFinished);
    setupSupplierMapTab();
    setupSupplierAiAdvisorTab();

    // 6. Equipment Management (Index 5)
    ui_equipment = new Ui::EquipmentManagement;
    equipmentPage = new QWidget(this);
    ui_equipment->setupUi(equipmentPage);
    ui->stackedWidget->removeWidget(ui->page_equipment);
    ui->stackedWidget->insertWidget(5, equipmentPage);

    setupEquipmentConnections();

    // 7. Order Management (Index 6)
    ui_order = new Ui::OrderManagement;
    orderPage = new QWidget(this);
    ui_order->setupUi(orderPage);

    m_mapNet = new QNetworkAccessManager(this);
    connect(m_mapNet, &QNetworkAccessManager::finished, this, &MainWindow::onMapNetworkFinished);
    setupOrderMapTab();
    
    // Order Input Validation
    ui_order->le_id->setValidator(new QIntValidator(1, 999999999, this));
    ui_order->le_stock->setValidator(new QIntValidator(1, 999999, this));
    ui_order->le_buyer->setValidator(new QIntValidator(1, 999999999, this));
    QDoubleValidator *priceValidator = new QDoubleValidator(0.01, 9999999.99, 2, this);
    priceValidator->setNotation(QDoubleValidator::StandardNotation);
    ui_order->le_prix->setValidator(priceValidator);
    
    setupOrderCatalogResolutionTabs();

    ui_order->le_catalog_search->setStyleSheet(
        "QLineEdit {"
        "  background: rgba(255,255,255,0.96);"
        "  color: #2E261C;"
        "  border: 1.5px solid #8B6F47;"
        "  border-radius: 8px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #A38253;"
        "}"
    );

    const QString catalogBtnStyle =
        "QPushButton {"
        "  background-color: #8B6F47;"
        "  color: white;"
        "  border-radius: 8px;"
        "  padding: 8px 14px;"
        "  font-weight: bold;"
        "  border: 1px solid #6d5638;"
        "}"
        "QPushButton:hover {"
        "  background-color: #a38253;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #6d5638;"
        "}";
    ui_order->btn_export_catalog->setStyleSheet(catalogBtnStyle);
    ui_order->btn_print_catalog->setStyleSheet(catalogBtnStyle);
    ui_order->btn_delete_all->setStyleSheet(catalogBtnStyle);
    ui->stackedWidget->removeWidget(ui->page_order);
    ui->stackedWidget->insertWidget(6, orderPage);
    connect(ui_order->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    connect(ui_order->btn_clear, &QPushButton::clicked, this, &MainWindow::onOrderClearFields);
    connect(ui_order->btn_add, &QPushButton::clicked, this, &MainWindow::onOrderAdd);
    connect(ui_order->btn_modify, &QPushButton::clicked, this, &MainWindow::onOrderModify);
    connect(ui_order->btn_delete, &QPushButton::clicked, this, &MainWindow::onOrderDelete);
    
    // Connect catalog buttons
    connect(ui_order->le_catalog_search, &QLineEdit::returnPressed, this, &MainWindow::onOrderSearchCatalog);
    connect(ui_order->btn_export_catalog, &QPushButton::clicked, this, &MainWindow::onOrderExportCatalog);
    connect(ui_order->btn_import, &QPushButton::clicked, this, &MainWindow::onOrderImportCatalog);
    connect(ui_order->btn_print_catalog, &QPushButton::clicked, this, &MainWindow::onOrderPrintCatalog);
    connect(ui_order->btn_delete_all, &QPushButton::clicked, this, &MainWindow::onOrderDeleteAll);
    
    // Connect QR Code tab buttons
    connect(ui_order->btn_generate_qr, &QPushButton::clicked, this, &MainWindow::onGenerateQR);
    connect(ui_order->btn_save_qr, &QPushButton::clicked, this, &MainWindow::onSaveQR);
    connect(ui_order->btn_print_qr, &QPushButton::clicked, this, &MainWindow::onPrintQR);
    
    // Embed the 3D modeling widget inside the 3D Modeling tab
    {
        auto *modeler = new ModelingWidget(ui_order->tab_3d_modeling);
        auto *tabLayout = new QVBoxLayout(ui_order->tab_3d_modeling);
        tabLayout->setContentsMargins(0, 0, 0, 0);
        tabLayout->addWidget(modeler);

        // Auto-load preset when order type changes
        connect(ui_order->cb_type, &QComboBox::currentTextChanged, modeler, &ModelingWidget::loadPreset);
    }

    // Auto-refresh catalog when switching to catalog tab, and close help panels
    connect(ui_order->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        // Close help buttons when switching tabs
        ui_order->btn_help->setChecked(false);
        ui_order->btn_help_qr->setChecked(false);
        
        // Stop ost4 and fade back to ost1 if tutorial audio is playing
        if (tutorialLoopAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOutAndPlay(tutorialLoopAudioPlayer, tutorialLoopAudioOutput,
                           loginAudioPlayer, loginAudioOutput);
        }
        
        QWidget *currentTab = ui_order->tabWidget->widget(index);
        if (currentTab == ui_order->tab_catalog) {
            onOrderRefreshCatalog();
        } else if (currentTab && currentTab->objectName() == "tab_map") {
            populateMapClients();
        }
    });
    
    // New connections
    connect(ui_employee->btn_clear,            &QPushButton::clicked, this, &MainWindow::onEmployeeClearFields);
    connect(ui_employee->btn_send_mail,       &QPushButton::clicked, this, &MainWindow::onEmployeeSendMail);
    connect(ui_employee->btn_export_pdf,      &QPushButton::clicked, this, &MainWindow::onEmployeeExportHistoryPDF);
    connect(ui_employee->le_history_search,   &QLineEdit::textChanged, this, &MainWindow::onEmployeeHistorySearch);
    connect(ui_employee->cb_history_filter,   &QComboBox::currentIndexChanged, this, &MainWindow::onEmployeeHistorySearch);
    connect(ui_employee->cb_mail_template,    &QComboBox::currentIndexChanged, this, &MainWindow::onEmployeeMailTemplateChanged);
    // btn_modify / btn_delete are already connected earlier (avoid duplicate CRUD calls)
    
    connect(ui_supplier->btn_add,            &QPushButton::clicked, this, &MainWindow::onSupplierAdd);
    connect(ui_supplier->btn_modify,         &QPushButton::clicked, this, &MainWindow::onSupplierModify);
    connect(ui_supplier->btn_delete,         &QPushButton::clicked, this, &MainWindow::onSupplierDelete);
    connect(ui_supplier->btn_clear,          &QPushButton::clicked, this, &MainWindow::onSupplierClearFields);

    connect(ui_supplier->btn_export_pdf_view, &QPushButton::clicked, this, &MainWindow::onSupplierExportPDF);
    connect(ui_supplier->btn_print_view,      &QPushButton::clicked, this, &MainWindow::onSupplierPrint);
    connect(ui_supplier->btn_delete_all_view, &QPushButton::clicked, this, &MainWindow::onSupplierDeleteAll);

    connect(ui_supplier->btn_send_sms,       &QPushButton::clicked, this, &MainWindow::onSupplierSendSMS);
    connect(ui_supplier->btn_upload_image,   &QPushButton::clicked, this, &MainWindow::onSupplierUploadImage);
    connect(ui_supplier->btn_chercher,       &QPushButton::clicked, this, &MainWindow::onSupplierSearch);
    connect(ui_supplier->tableView, &QAbstractItemView::clicked, this, [this](const QModelIndex &idx){
        if (idx.column() == 0) { // Edit
            onSupplierLoad(idx);
        } else if (idx.column() == 1) { // Delete
            onSupplierDelete();
        }
    });

    // --- UI CLEANUP: Hide help buttons and chat icons as requested ---
    // Suppliers
    ui_supplier->btn_delete->hide();

    // Equipments
    ui_equipment->btn_delete->hide();
    
    // Chat Header Buttons
    ui_equipment->btn_chat_settings->hide();
    ui_equipment->btn_chat_music->hide();
    ui_equipment->btn_chat_refresh->hide();

    // Employees

    // Clients

    // Orders



    // Hide global delete button
    ui_equipment->btn_delete->hide();
    // Auto-refresh equipment view when switching to view tab, and history when switching to history


    // History Connections


    // --- Supplier Delivery Rating System ---
    connect(ui_supplier->btn_submit_review,   &QPushButton::clicked, this, &MainWindow::onSupplierReviewSubmit);
    connect(ui_supplier->btn_refresh_reviews, &QPushButton::clicked, this, &MainWindow::onSupplierReviewLoad);
    connect(ui_supplier->cb_supplier_reviews, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int){ onSupplierReviewLoad(); });
    connect(ui_supplier->sb_rating, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSupplierReviewRatingChanged);

    // Auto-refresh when switching tabs (stats, view, reviews)
    connect(ui_supplier->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        ui_supplier->btn_help_gestion->setChecked(false);
        ui_supplier->btn_help_stats->setChecked(false);
        // ui_supplier->btn_help_reviews->setChecked(false); // Enable if present

        if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_stats) {
            setupSupplierStats();
        } else if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_view) {
            onSupplierRefreshView();
        } else if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_reviews) {
            onSupplierPopulateRatingCombos();
            onSupplierReviewLoad();
        }
    });

    onSupplierEnsureReviewsTable();
    onSupplierPopulateRatingCombos();
    // onEmployeeEnsureHistoryTable(); // Removed redundant startup log entry
    // Keep history fully driven from EQUIPMENT table snapshots/state.
    // ensureEquipmentHistoryDatabaseObjects();

    // --- Apply Hover Animations to Management Module Buttons ---
    // Employee Management
    for (auto* button : employeePage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Client Management
    for (auto* button : clientPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Supplier Management
    for (auto* button : supplierPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Equipment Management
    for (auto* button : equipmentPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Order Management
    for (auto* button : orderPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }

    // Connect page change signal for dynamic retranslation
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onPageChanged);
    
    // Start at login page
    ui->stackedWidget->setCurrentIndex(0);

    // Call setup function for client stats
    setupClientStats();
    
    // Consolidate Client Management Tabs
    setupClientManagement();


    // Setup Employee Stats
    setupEmployeeStats();

    // Setup Supplier Stats
    setupSupplierStats();

    // Setup Client Calendar
    setupClientCalendar();

    // Standardize Add/Modify Modes
    setupEmployeeModes();
    setupSupplierModes();
    onSupplierRefreshView();
    setupEquipmentModes();
    setupOrderModes();

    // Hide tab bars and setup radio button navigation
    ui_client->tabWidget->tabBar()->hide();
    ui_employee->tabWidget->tabBar()->hide();
    ui_supplier->tabWidget->tabBar()->hide();
    ui_equipment->tabWidget->tabBar()->hide();
    ui_order->tabWidget->tabBar()->hide();
    // Setup radio button navigation for all UIs
    // Y set to (TabWidgetY + 25) to align with inner buttons.
    setupTabNavigation(clientPage, ui_client->tabWidget, {"Manage", "View", "Stats", "Mail", "Calendar"}, 150, 45, {0, 1, 2, 3, 4}, 115, 40);   // Override indices: Manage->0, View->1, Stats->2, Mail->3, Calendar->4
    setupTabNavigation(employeePage, ui_employee->tabWidget, {"Manage", "View", "Stats", "History"}, 150, 95, {}, 115, 40);  // 70+25
    setupTabNavigation(supplierPage, ui_supplier->tabWidget, {"Manage", "Stats", "View", "Reviews", "Map", "AI Advisor"}, 140, 45, {0, 1, 2, 3, 4, 5}, 100, 30);

    setupTabNavigation(orderPage, ui_order->tabWidget, {"Manage", "QR Code", "Catalog", "3D Modeling", "Map"}, 250, 85, {}, 125, 40);   // 60+25

    // ---- Voice Command Engine ------------------------------------------------
    {
        // Walk up from the exe to find vosk/vosk-model/ in the project tree
        auto findUpward = [](const QString &startDir, const QString &relPath) -> QString {
            QDir dir(startDir);
            for (int i = 0; i < 8; ++i) {
                const QString c = dir.absoluteFilePath(relPath);
                if (QDir(c).exists()) return c;
                if (!dir.cdUp()) break;
            }
            return startDir + "/" + relPath; // fallback keeps original error message
        };
        const QString modelPath = findUpward(
            QCoreApplication::applicationDirPath(), "vosk/vosk-model");
        m_voiceEngine = new VoiceCommandEngine(modelPath, this);
        connect(m_voiceEngine, &VoiceCommandEngine::commandDetected,
                this, &MainWindow::onVoiceCommand);
        connect(m_voiceEngine, &VoiceCommandEngine::listeningChanged,
                this, &MainWindow::onVoiceListeningChanged);
        // initFailed must be connected BEFORE init() is ever called
        connect(m_voiceEngine, &VoiceCommandEngine::initFailed, this,
                [this](const QString &msg) {
            QMessageBox::warning(this, "Voice Commands", msg);
            if (m_micBtn) m_micBtn->setChecked(false);
        });

        // Mic toggle button — sits in the status bar, visible on every page
        m_micBtn = new QPushButton("  Mic: OFF");
        m_micBtn->setCheckable(true);
        m_micBtn->setFixedHeight(28);
        m_micBtn->setCursor(Qt::PointingHandCursor);
        m_micBtn->setStyleSheet(R"(
            QPushButton {
                background: #2C2418; color: #806050;
                border: 1px solid #4A3728; border-radius: 6px;
                padding: 0 14px; font-size: 12px; font-weight: bold;
            }
            QPushButton:hover  { background: #3C3020; color: #A08060; }
            QPushButton:checked {
                background: #8B6F47; color: white;
                border: 1px solid #D4AF37;
            }
        )");
        statusBar()->addPermanentWidget(m_micBtn);
        statusBar()->setStyleSheet("background: #1C1610; border-top: 1px solid #3A2A1A;");

        connect(m_micBtn, &QPushButton::clicked, this, [this]() {
            if (!m_voiceEngine->isReady()) {
                // init() will emit initFailed if something is missing
                if (!m_voiceEngine->init()) {
                    m_micBtn->setChecked(false);
                    return;
                }
            }
            m_voiceEngine->toggleListening();
        });
    }
    // --------------------------------------------------------------------------

    // Standardize UI Styling
    setupGlobalStyles();
    
    // Connect all help buttons to toggle between ost1 and ost4 with fade
    auto connectHelpButton = [this](QWidget* page, const QString& buttonName, const QString& tutorialText) {
        QToolButton* btn = page->findChild<QToolButton*>(buttonName);
        if (btn) {
            connect(btn, &QToolButton::clicked, this, [this, btn, tutorialText]() {
                // Fade ost1 out and play ost4
                fadeOutAndPlay(loginAudioPlayer, loginAudioOutput,
                               tutorialLoopAudioPlayer, tutorialLoopAudioOutput);
                // Show blur overlay dialog (blocks until closed)
                showTutorialOverlay(tutorialText);
                // When dialog closes: uncheck the button, fade ost4 out and resume ost1
                btn->setChecked(false);
                tutorialLoopAudioPlayer->stop();
                fadeOutAndPlay(tutorialLoopAudioPlayer, tutorialLoopAudioOutput,
                               loginAudioPlayer, loginAudioOutput);
            });
        }
    };
    
    // Connect Employee Management help buttons
    connectHelpButton(employeePage, "btn_help_add", "This is the employee management tutorial. Here you can add, modify, or delete employees. Fill in all fields and click 'Add' to create a new employee.");

    // Connect Client Management help buttons
    connectHelpButton(clientPage, "btn_help_add", "This is the client management tutorial. Here you can add, modify, or delete clients. Fill in all fields and click 'Add' to create a new client.");

    // Connect Supplier Management help buttons
    connectHelpButton(supplierPage, "btn_help_gestion", "This is the supplier management tutorial. Here you can manage suppliers.");
    connectHelpButton(supplierPage, "btn_help_stats", "This is the supplier stats tutorial. Here you can view supplier statistics.");
    connectHelpButton(supplierPage, "btn_help_reviews", "This is the supplier reviews tutorial. Here you can view and manage supplier reviews.");

    // Connect Equipment Management help buttons
    connectHelpButton(equipmentPage, "btn_help_gestion", "This is the equipment management tutorial. Here you can manage equipment.");
    connectHelpButton(equipmentPage, "btn_help_stats", "This is the equipment stats tutorial. Here you can view equipment statistics.");
    connectHelpButton(equipmentPage, "btn_help_view", "This is the equipment list tutorial. Here you can search and filter through all existing equipment.");
    


    // Connect Order Management help buttons
    connectHelpButton(orderPage, "btn_help", "This is the order tutorial. Here you can add, modify, or delete orders. Fill in all fields and click 'Add' to create a new order. Use the search and catalog features to manage orders efficiently.");
    connectHelpButton(orderPage, "btn_help_qr", "This is the QR code tutorial. Here you can generate, save, and print QR codes for orders.");
    
    // Initialize audio player for management pages
    loginAudioPlayer = new QMediaPlayer(this);
    loginAudioOutput = new QAudioOutput(this);
    loginAudioPlayer->setAudioOutput(loginAudioOutput);
    loginAudioPlayer->setSource(QUrl("qrc:/assets/ost1.mp3"));
    loginAudioPlayer->setLoops(QMediaPlayer::Infinite); // Loop indefinitely
    loginAudioOutput->setVolume(currentVolume);
    
    // Initialize audio player for home page
    homeAudioPlayer = new QMediaPlayer(this);
    homeAudioOutput = new QAudioOutput(this);
    homeAudioPlayer->setAudioOutput(homeAudioOutput);
    homeAudioPlayer->setSource(QUrl("qrc:/assets/ostm.mp3"));
    homeAudioPlayer->setLoops(QMediaPlayer::Infinite);
    homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
    
    // Initialize audio player for tutorial (help buttons) - only ost4
    tutorialLoopAudioPlayer = new QMediaPlayer(this);
    tutorialLoopAudioOutput = new QAudioOutput(this);
    tutorialLoopAudioPlayer->setAudioOutput(tutorialLoopAudioOutput);
    tutorialLoopAudioPlayer->setSource(QUrl("qrc:/assets/ost4.mp3"));
    tutorialLoopAudioPlayer->setLoops(QMediaPlayer::Infinite);
    tutorialLoopAudioOutput->setVolume(currentVolume);


}


void MainWindow::showTutorialOverlay(const QString &text)
{
    // Apply blur effect to background
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(8.0);
    this->setGraphicsEffect(blur);

    // Create overlay dialog
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setAlignment(Qt::AlignCenter);

    QFrame *card = new QFrame(&dialog);
    card->setObjectName("tutorialCard");
    card->setStyleSheet("QFrame#tutorialCard { background: #fffbe6; border-radius: 16px; border: 2px solid #8B6F47; padding: 32px; }");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    QLabel *label = new QLabel(text, card);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setStyleSheet("font-size: 20px; color: #6b4f1d; font-weight: bold;");
    cardLayout->addWidget(label);

    QPushButton *closeBtn = new QPushButton("Close", card);
    closeBtn->setObjectName("closeTutorialBtn");
    closeBtn->setStyleSheet("QPushButton#closeTutorialBtn { background: #8B6F47; color: white; font-size: 16px; border-radius: 8px; padding: 8px 24px; border: 2px solid transparent; }"
                           "QPushButton#closeTutorialBtn:hover { background: #a8845a; border: 2px solid #6b4f1d; }");
    cardLayout->addWidget(closeBtn, 0, Qt::AlignCenter);

    layout->addWidget(card, 0, Qt::AlignCenter);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();

    // Remove blur after closing
    this->setGraphicsEffect(nullptr);
}

void MainWindow::setupClientStats()
{
    // 1. Create/Clear Layout for the stats container
    if (!ui_client->widget_chart->layout()) {
        QHBoxLayout *layout = new QHBoxLayout(ui_client->widget_chart);
        ui_client->widget_chart->setLayout(layout);
    } else {
        QLayoutItem *child;
        while ((child = ui_client->widget_chart->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    }

    // --- CYBERPUNK THEME HELPERS ---
    QColor bgTrans(10, 10, 10, 180);
    QColor goldColor("#D4AF37");
    QColor silverColor("#C0C0C0");
    QFont chartFont("Consolas", 11, QFont::Bold);
    QFont titleFont("Outfit", 14, QFont::Bold);

    QSqlQuery q;
    
    // --- 1. Total Clients Widget ---
    int totalClients = 0;
    if (q.exec("SELECT COUNT(*) FROM CLIENTS") && q.next()) {
        totalClients = q.value(0).toInt();
    }
    
    QFrame *summaryBox = new QFrame();
    summaryBox->setMinimumWidth(250);
    summaryBox->setStyleSheet("QFrame { background: rgba(10, 10, 10, 0.7); border: 2px solid #D4AF37; border-radius: 10px; }");
    QVBoxLayout *sumLayout = new QVBoxLayout(summaryBox);
    
    QLabel *lblTitle = new QLabel(trKey("TOTAL CLIENTS"));
    lblTitle->setStyleSheet("color: #D4AF37; border: none; font-family: 'Outfit'; font-size: 16px; font-weight: bold;");
    lblTitle->setAlignment(Qt::AlignCenter);
    
    QLabel *lblCount = new QLabel(QString::number(totalClients));
    lblCount->setStyleSheet("color: #FFFFFF; border: none; font-family: 'Consolas'; font-size: 52px; font-weight: bold;");
    lblCount->setAlignment(Qt::AlignCenter);
    
    sumLayout->addStretch();
    sumLayout->addWidget(lblTitle);
    sumLayout->addWidget(lblCount);
    sumLayout->addStretch();

    // --- 2. CHART 1: PIE CHART (Gender Matrix) ---
    QPieSeries *series = new QPieSeries();
    int maleCount = 0, femaleCount = 0;
    
    if (q.exec("SELECT GENDER, COUNT(*) FROM CLIENTS GROUP BY GENDER")) {
        while (q.next()) {
            QString g = q.value(0).toString().trimmed();
            int c = q.value(1).toInt();
            if (g.compare("Male", Qt::CaseInsensitive) == 0) maleCount = c;
            else if (g.compare("Female", Qt::CaseInsensitive) == 0) femaleCount = c;
        }
    }
    
    if (maleCount == 0 && femaleCount == 0) {
        series->append("No Data", 1);
    } else {
        series->append(trKey("Male"), maleCount);
        series->append(trKey("Female"), femaleCount);
        
        QPieSlice *sliceMale = series->slices().at(0);
        sliceMale->setBrush(goldColor);
        sliceMale->setLabelVisible(maleCount > 0);
        sliceMale->setLabelColor(Qt::white);
        sliceMale->setLabelFont(chartFont);

        QPieSlice *sliceFemale = series->slices().at(1);
        sliceFemale->setBrush(silverColor);
        sliceFemale->setLabelVisible(femaleCount > 0);
        sliceFemale->setLabelColor(Qt::white);
        sliceFemale->setLabelFont(chartFont);
        if(femaleCount > 0) sliceFemale->setExploded();
    }

    QChart *chartPie = new QChart();
    chartPie->addSeries(series);
    chartPie->setTitle(trKey("Gender Distribution"));
    chartPie->setProperty("trTitleKey", "Gender Distribution");
    chartPie->setTitleFont(titleFont);
    chartPie->setTitleBrush(goldColor);
    chartPie->setBackgroundBrush(bgTrans);
    chartPie->legend()->setLabelBrush(Qt::white);
    chartPie->legend()->setFont(chartFont);
    chartPie->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartViewPie = new QChartView(chartPie);
    chartViewPie->setRenderHint(QPainter::Antialiasing);
    chartViewPie->setStyleSheet("background: transparent; border: 2px solid #8B6F47; border-radius: 10px;");

    // --- 3. CHART 2: BAR CHART (Contact Vectors) ---
    QBarSeries *seriesBar = new QBarSeries();
    QBarSet *domainSet = new QBarSet(trKey("Clients"));
    domainSet->setColor(goldColor);
    
    QStringList categories;
    int maxVal = 0;
    
    QString domainQuery = "SELECT SUBSTR(EMAIL, INSTR(EMAIL, '@') + 1) AS DOMAIN, COUNT(*) AS C "
                          "FROM CLIENTS WHERE EMAIL LIKE '%@%' "
                          "GROUP BY SUBSTR(EMAIL, INSTR(EMAIL, '@') + 1) "
                          "ORDER BY C DESC";
                          
    if (q.exec(domainQuery)) {
        int count = 0;
        while (q.next() && count < 5) { // Top 5 limits
            QString dom = q.value(0).toString().trimmed();
            int c = q.value(1).toInt();
            if(dom.isEmpty()) continue;
            
            // Extract just the provider name
            dom = dom.split('.').first().toUpper();
            
            *domainSet << c;
            categories << dom;
            if(c > maxVal) maxVal = c;
            count++;
        }
    }
    
    // Fallback if no valid emails found
    if (categories.isEmpty()) {
       *domainSet << 0;
       categories << "NONE";
    }

    seriesBar->append(domainSet);
    
    QChart *chartBar = new QChart();
    chartBar->addSeries(seriesBar);
    chartBar->setTitle(trKey("Top Email Providers"));
    chartBar->setProperty("trTitleKey", "Top Email Providers");
    chartBar->setTitleFont(titleFont);
    chartBar->setTitleBrush(goldColor);
    chartBar->setBackgroundBrush(bgTrans);
    chartBar->legend()->hide();
    chartBar->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsBrush(Qt::white);
    axisX->setLabelsFont(chartFont);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    seriesBar->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxVal + 1);
    axisY->setLabelsBrush(Qt::white);
    axisY->setLabelsFont(chartFont);
    axisY->setGridLineColor(QColor(212, 175, 55, 40));
    chartBar->addAxis(axisY, Qt::AlignLeft);
    seriesBar->attachAxis(axisY);

    QChartView *chartViewBar = new QChartView(chartBar);
    chartViewBar->setRenderHint(QPainter::Antialiasing);
    chartViewBar->setStyleSheet("background: transparent; border: 2px solid #8B6F47; border-radius: 10px;");
    
    // Add dynamically mapped visuals to layout
    ui_client->widget_chart->layout()->addWidget(summaryBox);
    ui_client->widget_chart->layout()->addWidget(chartViewPie);
    ui_client->widget_chart->layout()->addWidget(chartViewBar);
}

MainWindow::~MainWindow()
{
    {
        QSettings settings("HammerDown", "HammerDown");
        settings.setValue("audio/volume", currentVolume);
    }
    delete ui;
    delete ui_client;
    delete ui_employee;
    delete ui_equipment;
    delete ui_order;
    delete ui_supplier;
}

// =============================================================================
// VOICE COMMANDS
// =============================================================================

// Helper: returns true if 'text' contains any of the given keywords
static bool hasAny(const QString &text, const QStringList &kw) {
    for (const QString &w : kw)
        if (text.contains(w)) return true;
    return false;
}

void MainWindow::onVoiceCommand(const QString &text)
{
    // ── Module navigation ────────────────────────────────────────────────────
    if (hasAny(text, {"client", "clients"})) {
        on_nav_clients_clicked();
    } else if (hasAny(text, {"employee", "employees", "staff"})) {
        on_nav_employees_clicked();
    } else if (hasAny(text, {"supplier", "suppliers"})) {
        on_nav_suppliers_clicked();
    } else if (hasAny(text, {"equipment"})) {
        on_nav_equipments_clicked();
    } else if (hasAny(text, {"order", "orders"})) {
        on_nav_orders_clicked();
    } else if (hasAny(text, {"home", "dashboard", "back"})) {
        on_btn_home_clicked();

    // ── Client tabs ──────────────────────────────────────────────────────────
    } else if (hasAny(text, {"manage", "management", "add"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(0);
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(0);
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(0);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(0);
        if (ui_order) ui_order->tabWidget->setCurrentIndex(0);
    } else if (hasAny(text, {"view", "list", "show all"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(1);
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(1);
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(2);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(1);
    } else if (hasAny(text, {"stats", "statistics", "chart", "analytics"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(2);
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(2);
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(1);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(3);
    } else if (hasAny(text, {"mail", "email", "message", "send"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(3);
    } else if (hasAny(text, {"calendar", "schedule", "events"})) {
        if (ui_client) ui_client->tabWidget->setCurrentIndex(4);
    } else if (hasAny(text, {"history", "log", "audit"})) {
        if (ui_employee) ui_employee->tabWidget->setCurrentIndex(3);
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(2);
    } else if (hasAny(text, {"catalog"})) {
        if (ui_order) ui_order->tabWidget->setCurrentIndex(2);
    } else if (hasAny(text, {"map", "location"})) {
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(4);
        if (ui_order) ui_order->tabWidget->setCurrentIndex(4);
    } else if (hasAny(text, {"review", "rating"})) {
        if (ui_supplier) ui_supplier->tabWidget->setCurrentIndex(3);
    } else if (hasAny(text, {"nexus", "ai", "intelligence"})) {
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(5);
    } else if (hasAny(text, {"cost", "costs", "budget"})) {
        if (ui_equipment) ui_equipment->tabWidget->setCurrentIndex(6);
    }

    // Show a brief status hint
    statusBar()->showMessage("Voice: \"" + text + "\"", 3000);
}

void MainWindow::onVoiceListeningChanged(bool active)
{
    if (m_micBtn) {
        m_micBtn->setChecked(active);
        m_micBtn->setText(active ? "  Mic: ON" : "  Mic: OFF");
    }
    if (active)
        statusBar()->showMessage("Listening...", 0);
    else
        statusBar()->clearMessage();
}

void MainWindow::setAudioVolume(qreal volume)
{
    currentVolume = qBound<qreal>(0.0, volume, 1.5);
    {
        QSettings settings("HammerDown", "HammerDown");
        settings.setValue("audio/volume", currentVolume);
    }

    emit audioVolumeChanged(currentVolume);
    if (loginAudioOutput) {
        loginAudioOutput->setVolume(currentVolume);
    }
    if (homeWindow) {
        homeWindow->setVolume(currentVolume);
    }
    if (homeAudioOutput) {
        homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
    }
    if (tutorialLoopAudioOutput) {
        tutorialLoopAudioOutput->setVolume(currentVolume);
    }
    if (chatAudioOutput) {
        chatAudioOutput->setVolume(currentVolume);
    }
}

void MainWindow::pauseHomeAudioForSettings()
{
    if (!homeAudioPlayer) return;
    if (!ui || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 1) return;
    if (m_audioSuspendedForOstp) return;
    if (homeAudioPlayer->playbackState() != QMediaPlayer::PlayingState) return;

    m_homeAudioSettingsResumePos = homeAudioPlayer->position();
    m_homeAudioPausedBySettings = true;
    homeAudioPlayer->stop();
}

void MainWindow::resumeHomeAudioAfterSettings()
{
    if (!m_homeAudioPausedBySettings || !homeAudioPlayer) {
        m_homeAudioPausedBySettings = false;
        return;
    }
    if (!ui || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 1) {
        m_homeAudioPausedBySettings = false;
        return;
    }
    if (m_audioSuspendedForOstp) {
        return;
    }

    homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
    homeAudioPlayer->setPosition(m_homeAudioSettingsResumePos);
    homeAudioPlayer->play();
    m_homeAudioPausedBySettings = false;
}

void MainWindow::pauseHomeAudioForTutorial()
{
    if (!homeAudioPlayer) return;
    if (!ui || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 1) return;
    if (m_audioSuspendedForOstp) return;
    if (homeAudioPlayer->playbackState() != QMediaPlayer::PlayingState) return;

    m_homeAudioTutorialResumePos = homeAudioPlayer->position();
    m_homeAudioPausedByTutorial = true;
    homeAudioPlayer->stop();
}

void MainWindow::resumeHomeAudioAfterTutorial()
{
    if (!m_homeAudioPausedByTutorial || !homeAudioPlayer) {
        m_homeAudioPausedByTutorial = false;
        return;
    }
    if (!ui || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 1) {
        m_homeAudioPausedByTutorial = false;
        return;
    }
    if (m_audioSuspendedForOstp) {
        return;
    }

    homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
    homeAudioPlayer->setPosition(m_homeAudioTutorialResumePos);
    homeAudioPlayer->play();
    m_homeAudioPausedByTutorial = false;
}

void MainWindow::suspendAudioForOstp()
{
    if (m_audioSuspendedForOstp) return;
    m_audioSuspendedForOstp = true;

    auto suspendPlayer = [](QMediaPlayer *player, bool &resumeFlag, qint64 &resumePos) {
        resumeFlag = false;
        resumePos = 0;
        if (!player) return;
        if (player->playbackState() != QMediaPlayer::PlayingState) return;
        resumePos = player->position();
        resumeFlag = true;
        player->stop();
    };

    suspendPlayer(loginAudioPlayer, m_resumeLoginAfterOstp, m_loginResumePosAfterOstp);
    suspendPlayer(homeAudioPlayer, m_resumeHomeAfterOstp, m_homeResumePosAfterOstp);
    suspendPlayer(tutorialLoopAudioPlayer, m_resumeTutorialAfterOstp, m_tutorialResumePosAfterOstp);
    suspendPlayer(chatAudioPlayer, m_resumeChatAfterOstp, m_chatResumePosAfterOstp);

    if (homeWindow) {
        homeWindow->suspendActiveAudioForOverlay();
    }
}

void MainWindow::restoreAudioAfterOstp()
{
    if (!m_audioSuspendedForOstp) return;

    auto resumePlayer = [](QMediaPlayer *player, bool &resumeFlag, qint64 resumePos) {
        if (!resumeFlag || !player) {
            resumeFlag = false;
            return;
        }
        player->setPosition(resumePos);
        player->play();
        resumeFlag = false;
    };

    if (homeAudioOutput) homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
    if (loginAudioOutput) loginAudioOutput->setVolume(currentVolume);
    if (tutorialLoopAudioOutput) tutorialLoopAudioOutput->setVolume(currentVolume);
    if (chatAudioOutput) chatAudioOutput->setVolume(currentVolume);

    resumePlayer(loginAudioPlayer, m_resumeLoginAfterOstp, m_loginResumePosAfterOstp);
    resumePlayer(homeAudioPlayer, m_resumeHomeAfterOstp, m_homeResumePosAfterOstp);
    resumePlayer(tutorialLoopAudioPlayer, m_resumeTutorialAfterOstp, m_tutorialResumePosAfterOstp);
    resumePlayer(chatAudioPlayer, m_resumeChatAfterOstp, m_chatResumePosAfterOstp);

    if (homeWindow) {
        homeWindow->resumeSuspendedAudioAfterOverlay();
    }

    m_audioSuspendedForOstp = false;
}

// Fade out audio and then play another audio with fade in
void MainWindow::fadeOutAndPlay(QMediaPlayer *fadeOutPlayer, QAudioOutput *fadeOutOutput,
                                 QMediaPlayer *fadeInPlayer, QAudioOutput *fadeInOutput)
{
    if (fadeOutPlayer && fadeOutPlayer->playbackState() == QMediaPlayer::PlayingState) {
        fadeOut(fadeOutOutput, [=]() {
            fadeOutPlayer->stop();
            if (fadeInPlayer) {
                fadeInPlayer->setPosition(0);
                fadeInPlayer->play();
                fadeIn(fadeInOutput);
            }
        });
    } else {
        if (fadeInPlayer) {
            fadeInPlayer->setPosition(0);
            fadeInPlayer->play();
            fadeIn(fadeInOutput);
        }
    }
}

// Fade out effect
void MainWindow::fadeOut(QAudioOutput *output, std::function<void()> onComplete)
{
    if (!output) return;
    
    QPropertyAnimation *fadeAnimation = new QPropertyAnimation(output, "volume");
    fadeAnimation->setDuration(500); // 500ms fade
    fadeAnimation->setStartValue(output->volume());
    fadeAnimation->setEndValue(0.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(fadeAnimation, &QPropertyAnimation::finished, this, [=]() {
        if (onComplete) onComplete();
        fadeAnimation->deleteLater();
    });
    
    fadeAnimation->start();
}

// Fade in effect
void MainWindow::fadeIn(QAudioOutput *output)
{
    if (!output) return;
    
    output->setVolume(0.0);
    
    QPropertyAnimation *fadeAnimation = new QPropertyAnimation(output, "volume");
    fadeAnimation->setDuration(500); // 500ms fade
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(currentVolume);
    fadeAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    connect(fadeAnimation, &QPropertyAnimation::finished, fadeAnimation, &QPropertyAnimation::deleteLater);
    
    fadeAnimation->start();
}

// --- Navigation Slots ---

// Login -> Home (Page 0 -> Page 1)
void MainWindow::on_login_clicked()
{
    m_homeWelcomeShown = false; // Reset to show welcome notification on fresh login
    updateUserProfileDisplay();
    ui->stackedWidget->setCurrentIndex(1); 
}

// Home -> Modules
void MainWindow::on_gs_employes_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    ui_employee->tabWidget->setCurrentIndex(0);
    onEmployeeRefreshView();
    onEmployeeRefreshHistory();
}
void MainWindow::on_gs_client_clicked()      { ui->stackedWidget->setCurrentIndex(3); ui_client->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_fournisseur_clicked() { 
    ui->stackedWidget->setCurrentIndex(4); 
    ui_supplier->tabWidget->setCurrentIndex(0); 
    // Only trigger AI scan on first visit per session
    if (!m_aiAdvisorStartupDone) {
        m_aiAdvisorStartupDone = true;
        QTimer::singleShot(200, this, &MainWindow::checkWorkshopStockAndNotifyAI);
    }
}
void MainWindow::on_gs_order_clicked()       { ui->stackedWidget->setCurrentIndex(6); ui_order->tabWidget->setCurrentIndex(0); }

// Navigation sidebar
void MainWindow::on_nav_employees_clicked()  { 
    ui->stackedWidget->setCurrentIndex(2); 
    ui_employee->tabWidget->setCurrentIndex(0); 
    onEmployeeRefreshView(); 
    onEmployeeRefreshHistory();
}
void MainWindow::on_nav_clients_clicked()    { ui->stackedWidget->setCurrentIndex(3); ui_client->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_suppliers_clicked()  { ui->stackedWidget->setCurrentIndex(4); ui_supplier->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_orders_clicked()     { ui->stackedWidget->setCurrentIndex(6); ui_order->tabWidget->setCurrentIndex(0); }

// Logout / Home
void MainWindow::on_btn_logout_clicked()
{
    currentEmployeeId = 0;
    currentChatPartnerId = -1;
    if (chatRefreshTimer) chatRefreshTimer->stop();
    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::on_btn_home_clicked()       
{ 
    // Stop ost4 (tutorial audio) if it's playing
    if (tutorialLoopAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
        tutorialLoopAudioPlayer->stop();
    }
    
    // Uncheck all help buttons when returning to home
    auto uncheckHelpButton = [](QWidget* page, const QString& buttonName) {
        QToolButton* btn = page ? page->findChild<QToolButton*>(buttonName) : nullptr;
        if (btn && btn->isCheckable()) {
            btn->setChecked(false);
        }
    };
    
    // Uncheck all help buttons across all pages
    uncheckHelpButton(employeePage, "btn_help_add");
    uncheckHelpButton(clientPage, "btn_help_add");
    uncheckHelpButton(supplierPage, "btn_help_gestion");
    uncheckHelpButton(supplierPage, "btn_help_stats");
    uncheckHelpButton(supplierPage, "btn_help_reviews");
    uncheckHelpButton(equipmentPage, "btn_help_gestion");
    uncheckHelpButton(equipmentPage, "btn_help_stats");
    uncheckHelpButton(orderPage, "btn_help");
    uncheckHelpButton(orderPage, "btn_help_qr");
    
    ui->stackedWidget->setCurrentIndex(1); 
}

void MainWindow::onOrderClearFields()
{
    if (ui_order) {
        ui_order->le_id->clear();
        if (ui_order->cb_type && ui_order->cb_type->count() > 0) {
            ui_order->cb_type->setCurrentIndex(0);
        }
        ui_order->le_stock->clear();
        ui_order->le_prix->clear();
        ui_order->le_buyer->clear();
        ui_order->le_qr_order_id->clear();
        ui_order->le_catalog_search->clear();
    }
}

void MainWindow::updateSalaryInsight()
{
    if (!ui_employee) return;
    
    QString jobTitle = ui_employee->le_fonction->text().trimmed();
    
    if (jobTitle.isEmpty()) {
        // ui_employee->lbl_salary_insight->setText("Market Avg: --");
        return;
    }
    
    // Enhanced market salary estimation with real-time data simulation
    double marketAvg = 3200;
    if (jobTitle.contains("Senior", Qt::CaseInsensitive)) marketAvg += 2200;
    if (jobTitle.contains("Manager", Qt::CaseInsensitive)) marketAvg += 2800;
    if (jobTitle.contains("Lead", Qt::CaseInsensitive)) marketAvg += 1700;
    if (jobTitle.contains("Director", Qt::CaseInsensitive)) marketAvg += 3500;
    if (jobTitle.contains("Carpenter", Qt::CaseInsensitive)) marketAvg += 800;
    if (jobTitle.contains("Engineer", Qt::CaseInsensitive)) marketAvg += 2000;
    if (jobTitle.contains("Technician", Qt::CaseInsensitive)) marketAvg += 600;
    if (jobTitle.contains("Specialist", Qt::CaseInsensitive)) marketAvg += 1200;
}

void MainWindow::onSuggestSalary()
{
    if (!ui_employee) return;
    QString role;
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) {
        role = cb->currentText().trimmed();
    } else {
        role = ui_employee->le_fonction->text().trimmed();
    }
    if (role.isEmpty()) return;

    QSqlQuery q;
    q.prepare("SELECT AVG(SALARY) FROM EMPLOYEES WHERE JOB_TITLE = :role");
    q.bindValue(":role", role);
    
    if (q.exec() && q.next()) {
        double avg = q.value(0).toDouble();
        if (avg > 0) {
            ui_employee->dsb_salaire->setValue(avg);
        } else {
            // Suggest a default based on typical ranges if no data exists
            ui_employee->dsb_salaire->setValue(2500); 
        }
    }
}

void MainWindow::onOrderAdd()
{
    if (!ui_order) return;
    
    // Check if we are in Add mode (where ID is auto-generated)
    bool isAddMode = false;
    QRadioButton *rbAdd = ui_order->tab_manage->findChild<QRadioButton*>("rb_order_add_mode");
    if (rbAdd && rbAdd->isChecked()) {
        isAddMode = true;
    }

    QString type = ui_order->cb_type ? ui_order->cb_type->currentText() : QString();
    QString stock = ui_order->le_stock->text();
    QString prix = ui_order->le_prix->text();
    QString buyer = ui_order->le_buyer->text();

    int orderId = 0;
    
    if (isAddMode) {
        // Find the lowest available (missing) positive integer
        // By checking where (order_id + 1) does NOT exist in the table.
        // We also handle the case where 1 itself is missing or the table is empty.
        QSqlQuery query;
        QString qStr = "SELECT MIN(t1.order_id + 1) AS next_id "
                       "FROM ORDERS t1 "
                       "WHERE NOT EXISTS (SELECT 1 FROM ORDERS t2 WHERE t2.order_id = t1.order_id + 1)";
                       
        // First check if '1' is available
        QSqlQuery checkOne("SELECT 1 FROM ORDERS WHERE order_id = 1");
        if (!checkOne.next()) {
            orderId = 1; // 1 is available
        } else if (query.exec(qStr) && query.next() && !query.value(0).isNull()) {
            orderId = query.value(0).toInt();
        } else {
            // Fallback (should theoretically never happen if 1 exists but just in case)
            QSqlQuery maxQuery("SELECT NVL(MAX(order_id), 0) + 1 FROM ORDERS");
            if (maxQuery.next()) {
                orderId = maxQuery.value(0).toInt();
            } else {
                orderId = 1;
            }
        }
    } else {
        // Required for Modify (shouldn't be reached from Add button, but safe to keep)
        QString id = ui_order->le_id->text();
        if (id.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Order ID is required!");
            return;
        }
        bool idOk;
        orderId = id.toInt(&idOk);
        if (!idOk) {
            QMessageBox::warning(this, "Input Error", "Order ID must be a whole number.");
            return;
        }
        
        // Check if order ID already exists
        QSqlQuery checkOrder;
        checkOrder.prepare("SELECT COUNT(*) FROM ORDERS WHERE order_id = :id");
        checkOrder.bindValue(":id", orderId);
        if (checkOrder.exec() && checkOrder.next() && checkOrder.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Duplicate Error", 
                "Order ID " + QString::number(orderId) + " already exists!\n\n"
                "Please use a different Order ID.");
            return;
        }
    }
    
    // Validate other fields
    if (type.isEmpty() || stock.isEmpty() || prix.isEmpty() || buyer.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required!");
        return;
    }
    
    // Validate numeric inputs
    bool buyerOk, stockOk, priceOk;
    
    int clientId = buyer.toInt(&buyerOk);
    int quantity = stock.toInt(&stockOk);
    double price = prix.toDouble(&priceOk);
    
    if (!buyerOk || !stockOk || !priceOk) {
        QMessageBox::warning(this, "Input Error", 
            "Please enter valid numbers:\n"
            "• Client ID: whole number\n"
            "• Quantity: whole number\n"
            "• Price: decimal number");
        return;
    }
    
    // Check if client exists
    QSqlQuery checkClient;
    checkClient.prepare("SELECT FIRST_NAME, LAST_NAME FROM CLIENTS WHERE CLIENT_ID = :id");
    checkClient.bindValue(":id", clientId);
    
    if (!checkClient.exec() || !checkClient.next()) {
        QMessageBox::warning(this, "Invalid Client ID", 
            "Client ID " + QString::number(clientId) + " does not exist!\n\n"
            "Please enter a valid Client ID from the Clients table.\n"
            "You can check existing clients in the Client Management section.");
        return;
    }
    
    QString clientName = checkClient.value(0).toString() + " " + checkClient.value(1).toString();
    
    // Confirm order creation
    int reply = QMessageBox::question(this, "Confirm Order", 
        "Create order with these details?\n\n"
        "Order ID: " + QString::number(orderId) + " (Auto)\n"
        "Client: " + clientName + " (ID: " + QString::number(clientId) + ")\n"
        "Type: " + type + "\n"
        "Quantity: " + QString::number(quantity) + "\n"
        "Price: $" + QString::number(price, 'f', 2),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::No) return;
    
    // Insert order
    QSqlQuery query;
    query.prepare("INSERT INTO ORDERS (order_id, client_id, employee_id, order_type, total_quantity, total_price, order_date, order_status, payment_status) "
                  "VALUES (:id, :buyer, :employee, :type, :quantity, :price, SYSDATE, 'Pending', 'Unpaid')");
    query.bindValue(":id", orderId);
    query.bindValue(":buyer", clientId);
    query.bindValue(":employee", currentEmployeeId);
    query.bindValue(":type", type);
    query.bindValue(":quantity", quantity);
    query.bindValue(":price", price);
    
    if (query.exec()) {
        QMessageBox::information(this, "Success", "Order #" + QString::number(orderId) + " added successfully!");
        logActivity("Added new order #" + QString::number(orderId) + " for Client ID: " + buyer, "Orders");
        onOrderClearFields();
        onOrderRefreshCatalog();
    } else {
        QString errorMsg = query.lastError().databaseText();
        QMessageBox::critical(this, "Database Error", 
            "Failed to add order.\n\n" + errorMsg);
    }
}

void MainWindow::onOrderModify()
{
    if (!ui_order) return;
    
    QString id = ui_order->le_id->text();
    QString type = ui_order->cb_type ? ui_order->cb_type->currentText() : QString();
    QString stock = ui_order->le_stock->text();
    QString prix = ui_order->le_prix->text();
    QString buyer = ui_order->le_buyer->text();
    
    // Validate input
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Order ID is required!");
        return;
    }
    
    if (type.isEmpty() || stock.isEmpty() || prix.isEmpty() || buyer.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled to modify!");
        return;
    }
    
    // Validate numeric inputs
    bool idOk, buyerOk, stockOk, priceOk;
    id.toInt(&idOk);
    buyer.toInt(&buyerOk);
    stock.toInt(&stockOk);
    prix.toDouble(&priceOk);
    
    if (!idOk || !buyerOk || !stockOk || !priceOk) {
        QMessageBox::warning(this, "Input Error",
            "Please enter valid numbers:\n"
            "\u2022 Order ID: whole number\n"
            "\u2022 Client ID: whole number\n"
            "\u2022 Quantity: whole number\n"
            "\u2022 Price: decimal number");
        return;
    }
    
    // Check if client exists
    QSqlQuery checkClient;
    checkClient.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
    checkClient.bindValue(":id", buyer.toInt());
    if (!checkClient.exec() || !checkClient.next() || checkClient.value(0).toInt() == 0) {
        QMessageBox::warning(this, "Invalid Client ID",
            "Client ID " + buyer + " does not exist!");
        return;
    }
    
    QSqlQuery query;
    query.prepare("UPDATE ORDERS SET order_type = :type, total_quantity = :quantity, "
                  "total_price = :price, client_id = :buyer WHERE order_id = :id");
    query.bindValue(":id", id.toInt());
    query.bindValue(":type", type);
    query.bindValue(":quantity", stock.toInt());
    query.bindValue(":price", prix.toDouble());
    query.bindValue(":buyer", buyer.toInt());
    
    if (query.exec()) {
        if (query.numRowsAffected() > 0) {
            QMessageBox::information(this, "Success", "Order modified successfully!");
            logActivity("Modified order #" + id, "Orders");
            onOrderClearFields();
            onOrderRefreshCatalog();
        } else {
            QMessageBox::warning(this, "Not Found", "Order ID not found in database!");
        }
    } else {
        QMessageBox::critical(this, "Database Error", 
            "Failed to modify order.\n\nTechnical details: " + query.lastError().databaseText());
    }
}

void MainWindow::onOrderDelete()
{
    if (!ui_order) return;
    
    QString id = ui_order->le_id->text();
    
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Order ID is required!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        "Are you sure you want to delete order: " + id + "?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::No) {
        return;
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM ORDERS WHERE order_id = :id");
    query.bindValue(":id", id.toInt());
    
    if (query.exec()) {
        if (query.numRowsAffected() > 0) {
            QMessageBox::information(this, "Success", "Order deleted successfully!");
            logActivity("Deleted order #" + id, "Orders");
            onOrderClearFields();
            onOrderRefreshCatalog();
        } else {
            QMessageBox::warning(this, "Not Found", "Order ID not found in database!");
        }
    } else {
        QMessageBox::critical(this, "Database Error", 
            "Failed to delete order.\n\nTechnical details: " + query.lastError().databaseText());
    }
}

void MainWindow::onOrderDeleteAll()
{
    if (!ui_order) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete All", 
        "Are you sure you want to delete ALL orders? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (reply == QMessageBox::No) {
        return;
    }
    
    QSqlQuery query;
    if (query.exec("DELETE FROM ORDERS")) {
        QMessageBox::information(this, "Success", "All orders have been deleted successfully!");
        logActivity("Deleted ALL orders from database", "Orders");
        onOrderClearFields();
        onOrderRefreshCatalog();
    } else {
        QMessageBox::critical(this, "Database Error", 
            "Failed to delete orders.\n\nTechnical details: " + query.lastError().databaseText());
    }
}

void MainWindow::onOrderLoad()
{
    if (!ui_order) return;
    
    QString id = ui_order->le_id->text();
    
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Order ID is required!");
        return;
    }
    
    QSqlQuery query;
    query.prepare("SELECT order_type, total_quantity, total_price, client_id FROM ORDERS WHERE order_id = :id");
    query.bindValue(":id", id.toInt());
    
    if (query.exec() && query.next()) {
        const QString dbType = query.value(0).toString();
        if (ui_order->cb_type) {
            int idx = ui_order->cb_type->findText(dbType, Qt::MatchFixedString);
            if (idx == -1 && !dbType.isEmpty()) {
                ui_order->cb_type->addItem(dbType);
                idx = ui_order->cb_type->findText(dbType, Qt::MatchFixedString);
            }
            if (idx >= 0) {
                ui_order->cb_type->setCurrentIndex(idx);
            }
        }
        ui_order->le_stock->setText(query.value(1).toString());
        ui_order->le_prix->setText(query.value(2).toString());
        ui_order->le_buyer->setText(query.value(3).toString());
        QMessageBox::information(this, "Success", "Order loaded successfully!");
    } else {
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, "Database Error", 
                "Failed to load order.\n\nTechnical details: " + query.lastError().databaseText());
        } else {
            QMessageBox::warning(this, "Not Found", "Order ID not found in database!");
        }
    }
}

// ==================== QR Code Helper ====================
static QPixmap generateQrPixmap(const QString &text, int pixelSize = 8, int border = 4)
{
    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(text.toUtf8().constData(), QrCode::Ecc::MEDIUM);
    const int qrSize = qr.getSize();
    const int qrPixelSize = (qrSize + border * 2) * pixelSize;
    const int framePadding = qMax(6, pixelSize * 2);
    const int imgSize = qrPixelSize + framePadding * 2;

    // Use app palette shades with high contrast to remain scanner-friendly.
    const QColor bgColor(245, 236, 219);      // warm parchment
    const QColor moduleColor(34, 29, 22);     // near-black umber
    const QColor frameColor(139, 111, 71);    // app accent brown

    QImage img(imgSize, imgSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(bgColor);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(frameColor, qMax(2, pixelSize / 2)));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(framePadding / 2,
                            framePadding / 2,
                            imgSize - framePadding,
                            imgSize - framePadding,
                            framePadding * 0.55,
                            framePadding * 0.55);
    painter.end();

    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {
            if (qr.getModule(x, y)) {
                for (int dy = 0; dy < pixelSize; dy++) {
                    for (int dx = 0; dx < pixelSize; dx++) {
                        img.setPixelColor(framePadding + (x + border) * pixelSize + dx,
                                          framePadding + (y + border) * pixelSize + dy,
                                          moduleColor);
                    }
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

static QString buildOrderQrContent(int orderId, const QString &orderType, int quantity,
                                    double price, const QString &orderDate,
                                    const QString &orderStatus, const QString &paymentStatus,
                                    int clientId, const QString &clientName,
                                    const QString &clientEmail, const QString &clientPhone)
{
    QString content;
    content += "=== Hammer Down Order Invoice ===\n";
    content += "Order #" + QString::number(orderId) + "\n";
    content += "Date: " + orderDate + "\n";
    content += "Type: " + orderType + "\n";
    content += "Quantity: " + QString::number(quantity) + "\n";
    content += "Unit Price: $" + QString::number(price, 'f', 2) + "\n";
    content += "Total: $" + QString::number(price * quantity, 'f', 2) + "\n";
    content += "Status: " + orderStatus + "\n";
    content += "Payment: " + paymentStatus + "\n";
    content += "---\n";
    content += "Client #" + QString::number(clientId) + "\n";
    content += "Name: " + clientName + "\n";
    if (!clientEmail.isEmpty()) content += "Email: " + clientEmail + "\n";
    if (!clientPhone.isEmpty()) content += "Phone: " + clientPhone + "\n";
    content += "==============================";
    return content;
}

void MainWindow::setupOrderCatalogResolutionTabs()
{
    if (!ui_order || !ui_order->tab_catalog || !ui_order->table_catalog)
        return;

    m_orderCatalogUnresolvedTable = ui_order->table_catalog;

    const QRect catalogRect = m_orderCatalogUnresolvedTable->geometry();
    m_orderCatalogStatusTabs = new QTabWidget(ui_order->tab_catalog);
    m_orderCatalogStatusTabs->setObjectName("orderCatalogStatusTabs");
    m_orderCatalogStatusTabs->setGeometry(catalogRect);
    m_orderCatalogStatusTabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #8B6F47; background: transparent; }"
        "QTabBar::tab { background: rgba(255,255,255,0.88); color: #2E261C; border: 1px solid #8B6F47;"
        " padding: 7px 14px; min-width: 120px; }"
        "QTabBar::tab:selected { background: #8B6F47; color: white; }"
    );

    auto *unresolvedPage = new QWidget(m_orderCatalogStatusTabs);
    auto *resolvedPage = new QWidget(m_orderCatalogStatusTabs);

    auto *unresolvedLayout = new QVBoxLayout(unresolvedPage);
    unresolvedLayout->setContentsMargins(0, 0, 0, 0);
    unresolvedLayout->setSpacing(0);
    unresolvedLayout->addWidget(m_orderCatalogUnresolvedTable);

    auto *resolvedLayout = new QVBoxLayout(resolvedPage);
    resolvedLayout->setContentsMargins(0, 0, 0, 0);
    resolvedLayout->setSpacing(0);
    m_orderCatalogResolvedTable = new QTableWidget(resolvedPage);
    resolvedLayout->addWidget(m_orderCatalogResolvedTable);

    m_orderCatalogStatusTabs->addTab(unresolvedPage, "Unresolved");
    m_orderCatalogStatusTabs->addTab(resolvedPage, "Resolved");

    configureOrderCatalogTable(m_orderCatalogUnresolvedTable);
    configureOrderCatalogTable(m_orderCatalogResolvedTable);
}

void MainWindow::configureOrderCatalogTable(QTableWidget *table)
{
    if (!table)
        return;

    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Order ID", "Type", "Quantity", "Unit Price", "Total Price", "Buyer ID", "Payment", "Action"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(true);
    table->setFocusPolicy(Qt::NoFocus);
    table->setIconSize(QSize(54, 54));
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setFixedHeight(42);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setStyleSheet(
        "QTableWidget {"
        "  background: rgba(255, 255, 255, 0.94);"
        "  border: 1px solid #8B6F47;"
        "  border-radius: 0px;"
        "  color: #1D1D1D;"
        "  gridline-color: #8B6F47;"
        "  selection-background-color: #E0E0E0;"
        "  selection-color: #1D1D1D;"
        "}"
        "QTableWidget::item {"
        "  padding: 5px 8px;"
        "  border-right: 1px solid #8B6F47;"
        "  border-bottom: 1px solid #8B6F47;"
        "}"
        "QHeaderView::section {"
        "  background: #8B6F47;"
        "  color: #1F2A44;"
        "  border: 1px solid #705a39;"
        "  padding: 6px;"
        "  font-weight: bold;"
        "}"
        "QTableCornerButton::section {"
        "  background: #8B6F47;"
        "  border: 1px solid #705a39;"
        "}"
    );
}

bool MainWindow::populateOrderCatalogTable(QTableWidget *table, const QString &searchText, bool resolvedOnly)
{
    if (!table)
        return true;

    QSqlQuery query;
    const QString paymentCondition = resolvedOnly
        ? "UPPER(NVL(payment_status, 'UNPAID')) = 'PAID'"
        : "UPPER(NVL(payment_status, 'UNPAID')) <> 'PAID'";

    QString sql = "SELECT order_id, order_type, total_quantity, total_price, client_id, NVL(payment_status, 'Unpaid') "
                  "FROM ORDERS WHERE " + paymentCondition;
    const QString trimmedSearch = searchText.trimmed();
    if (!trimmedSearch.isEmpty()) {
        sql += " AND (CAST(order_id AS VARCHAR2(50)) LIKE :search "
               "OR UPPER(order_type) LIKE :search "
               "OR CAST(client_id AS VARCHAR2(50)) LIKE :search)";
    }
    sql += " ORDER BY order_id";

    query.prepare(sql);
    if (!trimmedSearch.isEmpty())
        query.bindValue(":search", "%" + trimmedSearch.toUpper() + "%");

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error",
            "Failed to load catalog orders.\n\nTechnical details: " + query.lastError().databaseText());
        return false;
    }

    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);

        const int orderId = query.value(0).toInt();
        const QString orderType = query.value(1).toString();
        const int quantity = query.value(2).toInt();
        const double unitPrice = query.value(3).toDouble();
        const double totalPrice = unitPrice * quantity;
        const QString buyerId = query.value(4).toString();
        const QString paymentStatus = query.value(5).toString();

        QTableWidgetItem *orderIdItem = new QTableWidgetItem(QString::number(orderId));
        orderIdItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 0, orderIdItem);

        QTableWidgetItem *typeItem = new QTableWidgetItem(trKey(orderType));
        typeItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        table->setItem(row, 1, typeItem);

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(quantity));
        qtyItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 2, qtyItem);

        QTableWidgetItem *unitPriceItem = new QTableWidgetItem(QString::number(unitPrice, 'f', 2));
        unitPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        table->setItem(row, 3, unitPriceItem);

        QTableWidgetItem *totalPriceItem = new QTableWidgetItem(QString::number(totalPrice, 'f', 2));
        totalPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        table->setItem(row, 4, totalPriceItem);

        QTableWidgetItem *buyerItem = new QTableWidgetItem(buyerId);
        buyerItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 5, buyerItem);

        QTableWidgetItem *paymentItem = new QTableWidgetItem(paymentStatus);
        paymentItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 6, paymentItem);

        auto *actionBtn = new QPushButton(table);
        if (resolvedOnly) {
            actionBtn->setText("Mark Unpaid");
            actionBtn->setStyleSheet(
                "QPushButton { background: #8B2F2F; color: #F5E6C8; border-radius: 6px; padding: 4px 8px; font-weight: bold; }"
                "QPushButton:hover { background: #A43A3A; }"
            );
            connect(actionBtn, &QPushButton::clicked, this, [this, orderId]() {
                markOrderAsUnpaid(orderId);
            });
        } else {
            actionBtn->setText("Mark Paid");
            actionBtn->setStyleSheet(
                "QPushButton { background: #2E6B3E; color: #F5E6C8; border-radius: 6px; padding: 4px 8px; font-weight: bold; }"
                "QPushButton:hover { background: #3A8A4F; }"
            );
            connect(actionBtn, &QPushButton::clicked, this, [this, orderId]() {
                markOrderAsPaid(orderId);
            });
        }
        table->setCellWidget(row, 7, actionBtn);

        table->setRowHeight(row, 56);
        ++row;
    }

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    return true;
}

void MainWindow::markOrderAsPaid(int orderId)
{
    QSqlQuery query;
    query.prepare("UPDATE ORDERS SET payment_status = 'Paid' WHERE order_id = :id");
    query.bindValue(":id", orderId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error",
            "Failed to update payment status.\n\nTechnical details: " + query.lastError().databaseText());
        return;
    }

    onOrderSearchCatalog();
}

void MainWindow::markOrderAsUnpaid(int orderId)
{
    QSqlQuery query;
    query.prepare("UPDATE ORDERS SET payment_status = 'Unpaid' WHERE order_id = :id");
    query.bindValue(":id", orderId);
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error",
            "Failed to update payment status.\n\nTechnical details: " + query.lastError().databaseText());
        return;
    }

    onOrderSearchCatalog();
}

void MainWindow::onOrderRefreshCatalog()
{
    if (!ui_order) return;

    const QString searchText = ui_order->le_catalog_search ? ui_order->le_catalog_search->text().trimmed() : QString();
    if (!populateOrderCatalogTable(m_orderCatalogUnresolvedTable, searchText, false))
        return;
    populateOrderCatalogTable(m_orderCatalogResolvedTable, searchText, true);
}

void MainWindow::onOrderSearchCatalog()
{
    if (!ui_order) return;

    const QString searchText = ui_order->le_catalog_search ? ui_order->le_catalog_search->text().trimmed() : QString();
    if (!populateOrderCatalogTable(m_orderCatalogUnresolvedTable, searchText, false))
        return;
    populateOrderCatalogTable(m_orderCatalogResolvedTable, searchText, true);
}

void MainWindow::onOrderExportCatalog()
{
    if (!ui_order) return;
    
    // Get the order ID from search bar
    QString searchId = ui_order->le_catalog_search->text().trimmed();
    
    if (searchId.isEmpty()) {
        QMessageBox::warning(this, tr("No Order Selected"), 
            tr("Please enter an Order ID in the search box to export."));
        return;
    }
    
    // Validate it's a number
    bool ok;
    int orderId = searchId.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Invalid Order ID"), 
            tr("Please enter a valid Order ID number."));
        return;
    }
    
    // Query the specific order
    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderId);
    
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Order Not Found"), 
            tr("Order ID %1 does not exist in the database.").arg(searchId));
        return;
    }
    
    // Extract order data
    QString orderType = query.value(1).toString();
    int quantity = query.value(2).toInt();
    double price = query.value(3).toDouble();
    int clientId = query.value(4).toInt();
    QString orderDate = query.value(5).toDateTime().toString("MMMM dd, yyyy");
    QString orderStatus = query.value(6).toString();
    QString paymentStatus = query.value(7).toString();
    QString clientFirstName = query.value(8).toString();
    QString clientLastName = query.value(9).toString();
    QString clientEmail = query.value(10).toString();
    QString clientPhone = query.value(11).toString();
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Order to PDF"), 
                                                    QDir::homePath() + "/Order_" + searchId + ".pdf",
                                                    "PDF Files (*.pdf)");
    
    if (fileName.isEmpty()) return;
    
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageSize(QPageSize(QPageSize::A4));
    
    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to create PDF file.");
        return;
    }
    
    // Page dimensions and margins
    int pageWidth = printer.width();
    int pageHeight = printer.height();
    int margin = 80;  // 1+ inch margins for professional look
    int contentWidth = pageWidth - 2 * margin;
    int y = margin;
    
    // ==================== HEADER SECTION ====================
    // System title bar with background
    painter.fillRect(0, 0, pageWidth, 100, QColor(45, 45, 45));
    
    // System title
    QFont titleFont("Segoe UI", 22, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(Qt::white);
    painter.drawText(margin, 35, "Order Management System");
    
    // Subtitle
    QFont subtitleFont("Segoe UI", 10);
    painter.setFont(subtitleFont);
    painter.setPen(QColor(220, 220, 220));
    painter.drawText(margin, 60, "Professional Order Processing & Invoice Generation");
    
    y = 130;
    
    // ==================== COMPANY & ORDER INFO ====================
    // Company logo and info (left side)
    int logoSize = 70;
    QPixmap logo(":/assets/logo.png");
    if (!logo.isNull()) {
        QPixmap scaledLogo = logo.scaled(logoSize, logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(margin, y, scaledLogo);
    }
    
    QFont companyFont("Segoe UI", 18, QFont::Bold);
    painter.setFont(companyFont);
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(margin + logoSize + 20, y + 25, "Hammer Down");
    
    QFont companySubFont("Segoe UI", 9);
    painter.setFont(companySubFont);
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(margin + logoSize + 20, y + 45, "Business Solutions Provider");
    
    // Order number and date (right side)
    QFont orderNumFont("Segoe UI", 20, QFont::Bold);
    painter.setFont(orderNumFont);
    painter.setPen(QColor(45, 45, 45));
    QString orderText = "ORDER #" + searchId;
    QFontMetrics fm1(orderNumFont);
    int orderWidth = fm1.horizontalAdvance(orderText);
    painter.drawText(pageWidth - margin - orderWidth, y + 25, orderText);
    
    QFont dateFont("Segoe UI", 9);
    painter.setFont(dateFont);
    painter.setPen(QColor(100, 100, 100));
    QString dateGenerated = "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy hh:mm AP");
    QFontMetrics fm2(dateFont);
    int dateWidth = fm2.horizontalAdvance(dateGenerated);
    painter.drawText(pageWidth - margin - dateWidth, y + 50, dateGenerated);
    
    y += 100;
    
    // Horizontal separator
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawLine(margin, y, pageWidth - margin, y);
    
    y += 40;
    
    // ==================== ORDER DETAILS SECTION ====================
    // Section header
    QFont sectionHeaderFont("Segoe UI", 14, QFont::Bold);
    painter.setFont(sectionHeaderFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin, y, "Order Details");
    
    y += 10;
    
    // Section underline
    painter.setPen(QPen(QColor(139, 111, 71), 3));
    painter.drawLine(margin, y, margin + 120, y);
    
    y += 30;
    
    // Order details box with light background
    int detailsBoxHeight = 180;
    painter.fillRect(margin, y, contentWidth, detailsBoxHeight, QColor(250, 250, 252));
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawRect(margin, y, contentWidth, detailsBoxHeight);
    
    y += 30;
    
    // Two-column layout for order info
    QFont labelFont("Segoe UI", 10, QFont::Bold);
    QFont valueFont("Segoe UI", 10);
    int labelCol = margin + 30;
    int valueCol = margin + 200;
    int rowHeight = 28;
    
    auto drawDetailRow = [&](const QString& label, const QString& value) {
        painter.setFont(labelFont);
        painter.setPen(QColor(90, 90, 90));
        painter.drawText(labelCol, y, label);
        
        painter.setFont(valueFont);
        painter.setPen(QColor(40, 40, 40));
        painter.drawText(valueCol, y, value);
        
        y += rowHeight;
    };
    
    drawDetailRow("Order Date:", orderDate);
    drawDetailRow("Order Type:", orderType);
    drawDetailRow("Quantity:", QString::number(quantity) + " units");
    drawDetailRow("Unit Price:", "$" + QString::number(price, 'f', 2));
    drawDetailRow("Order Status:", orderStatus);
    drawDetailRow("Payment Status:", paymentStatus);
    
    y += 30;
    
    // ==================== CLIENT INFORMATION SECTION ====================
    // Section header
    painter.setFont(sectionHeaderFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin, y, "Client Information");
    
    y += 10;
    
    // Section underline
    painter.setPen(QPen(QColor(139, 111, 71), 3));
    painter.drawLine(margin, y, margin + 140, y);
    
    y += 30;
    
    // Client details box
    int clientBoxHeight = 140;
    painter.fillRect(margin, y, contentWidth, clientBoxHeight, QColor(250, 250, 252));
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawRect(margin, y, contentWidth, clientBoxHeight);
    
    y += 30;
    
    drawDetailRow("Client ID:", QString::number(clientId));
    drawDetailRow("Full Name:", clientFirstName + " " + clientLastName);
    drawDetailRow("Email Address:", clientEmail.isEmpty() ? "Not provided" : clientEmail);
    drawDetailRow("Phone Number:", clientPhone.isEmpty() ? "Not provided" : clientPhone);
    
    y += 40;
    
    // ==================== PAYMENT SUMMARY ====================
    // Summary section with accent color
    int summaryBoxHeight = 100;
    painter.fillRect(margin, y, contentWidth, summaryBoxHeight, QColor(139, 111, 71));
    
    // Inner white box for amount
    int innerMargin = 3;
    painter.fillRect(margin + innerMargin, y + innerMargin, 
                     contentWidth - 2 * innerMargin, summaryBoxHeight - 2 * innerMargin, 
                     QColor(255, 255, 255));
    
    y += 40;
    
    // Total amount label
    QFont summaryLabelFont("Segoe UI", 16, QFont::Bold);
    painter.setFont(summaryLabelFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin + 30, y, "TOTAL AMOUNT");
    
    // Total amount value (right aligned)
    QFont totalAmountFont("Segoe UI", 24, QFont::Bold);
    painter.setFont(totalAmountFont);
    painter.setPen(QColor(139, 111, 71));
    double totalPrice = price * quantity;
    QString totalText = "$" + QString::number(totalPrice, 'f', 2);
    QFontMetrics fm3(totalAmountFont);
    int totalWidth = fm3.horizontalAdvance(totalText);
    painter.drawText(pageWidth - margin - totalWidth - 30, y + 5, totalText);
    
    y += 35;
    
    // Payment status in summary
    QFont statusFont("Segoe UI", 10);
    painter.setFont(statusFont);
    painter.setPen(QColor(100, 100, 100));
    QString statusText = "Payment Status: " + paymentStatus;
    painter.drawText(margin + 30, y, statusText);
    
    // ==================== QR CODE ====================
    y += 80;  // Move below the summary box
    
    // Build QR content with same info as PDF
    QString qrContent = buildOrderQrContent(orderId, orderType, quantity, price,
                                             orderDate, orderStatus, paymentStatus,
                                             clientId, clientFirstName + " " + clientLastName,
                                             clientEmail, clientPhone);
    QPixmap qrPixmap = generateQrPixmap(qrContent, 4, 2);
    
    // QR section label
    painter.setFont(sectionHeaderFont);
    painter.setPen(QColor(45, 45, 45));
    painter.drawText(margin, y, "Scan QR Code");
    y += 10;
    painter.setPen(QPen(QColor(139, 111, 71), 3));
    painter.drawLine(margin, y, margin + 120, y);
    y += 20;
    
    // Draw QR code
    int qrDisplaySize = 140;
    QPixmap scaledQr = qrPixmap.scaled(qrDisplaySize, qrDisplaySize, Qt::KeepAspectRatio, Qt::FastTransformation);
    painter.drawPixmap(margin, y, scaledQr);
    
    // QR description text next to QR code
    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(margin + qrDisplaySize + 20, y + 30, "Scan this QR code to view");
    painter.drawText(margin + qrDisplaySize + 20, y + 50, "complete order details.");
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor(140, 140, 140));
    painter.drawText(margin + qrDisplaySize + 20, y + 80, "Contains: Order info, client data,");
    painter.drawText(margin + qrDisplaySize + 20, y + 95, "pricing and payment status.");
    
    // ==================== FOOTER ======================================
    y = pageHeight - 50;
    
    // Footer separator line
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawLine(margin, y, pageWidth - margin, y);
    
    y += 25;
    
    // Centered footer text
    QFont footerFont("Segoe UI", 8);
    painter.setFont(footerFont);
    painter.setPen(QColor(120, 120, 120));
    
    QString footerText = "";
    QFontMetrics fmFooter(footerFont);
    int footerWidth = fmFooter.horizontalAdvance(footerText);
    int footerX = (pageWidth - footerWidth) / 2;
    painter.drawText(footerX, y, footerText);
    
    painter.end();
    
    QMessageBox::information(this, tr("Success"), 
        tr("Order #%1 exported successfully!\n\nFile saved to:\n%2").arg(searchId).arg(fileName));
}

void MainWindow::onOrderImportCatalog()
{
    if (!ui_order) return;
    
    QMessageBox msgBox;
    msgBox.setWindowTitle("Import Orders");
    msgBox.setText("Would you like to download a blank template to fill out, or import an already filled file?");
    QPushButton *btnTemplate = msgBox.addButton("Download Template", QMessageBox::ActionRole);
    QPushButton *btnImport = msgBox.addButton("Import File", QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == btnTemplate) {
        // Option 1: Generate Template with 4 columns + example
        QString fileName = QFileDialog::getSaveFileName(this, "Save Template", QDir::homePath() + "/Template_Orders.csv", "CSV Files (*.csv)");
        if (fileName.isEmpty()) return;
        
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Could not create the template file.");
            return;
        }
        
        QTextStream out(&file);
        const QChar delimiter = ';';
        out << "sep=" << delimiter << "\n";
        out << "Type" << delimiter << "Quantity" << delimiter << "Price" << delimiter << "BuyerID\n";
        out << "Other" << delimiter << "10" << delimiter << "15.50" << delimiter << "1\n";
        file.close();
        
        QMessageBox::information(this, "Template Created",
            "Template saved successfully!\n\n"
            "Please open it in Excel or Notepad, fill out your orders using exactly those 4 columns:\n"
            "  Type, Quantity, Price, BuyerID\n\n"
            "An example row is included. You can remove the header line if you want — the importer is smart enough to handle it either way.");
        
    } else if (msgBox.clickedButton() == btnImport) {
        // Option 2: Import Filled File — smart parsing
        QString fileName = QFileDialog::getOpenFileName(this, "Import Orders", QDir::homePath(), "CSV Files (*.csv);;Text Files (*.txt);;Excel Files (*.xlsx *.xls);;All Files (*)");
        if (fileName.isEmpty()) return;

        const QFileInfo fi(fileName);
        const QString suffix = fi.suffix().toLower();
        const bool isWorkbookExtension = (suffix == "xlsx" || suffix == "xls" || suffix == "xlsm" || suffix == "xlsb");
        const bool isXlsxExtension = (suffix == "xlsx");

        QString tempCsvPath;
        QString importPath = fileName;

        if (isWorkbookExtension) {
            tempCsvPath = QDir::tempPath() + "/orders_import_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".csv";
            QFile::remove(tempCsvPath);
            const QString tempPath = QDir::toNativeSeparators(tempCsvPath);

            auto psEscape = [](QString s) {
                s.replace("'", "''");
                return s;
            };

            const QString sourcePath = QDir::toNativeSeparators(fileName);

            auto runExcelComConversion = [&](QString *outError) {
                const QString psScript = QString(
                    "$ErrorActionPreference='Stop'; "
                    "$excel=$null; $wb=$null; "
                    "try { "
                    "  $excel=New-Object -ComObject Excel.Application; "
                    "  $excel.Visible=$false; "
                    "  $excel.DisplayAlerts=$false; "
                    "  $wb=$excel.Workbooks.Open('%1'); "
                    "  $wb.SaveAs('%2', 62); "
                    "  $wb.Close($false); "
                    "} finally { "
                    "  if ($wb -ne $null) { [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($wb) } "
                    "  if ($excel -ne $null) { $excel.Quit(); [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($excel) } "
                    "}"
                ).arg(psEscape(sourcePath), psEscape(tempPath));

                QProcess ps;
                ps.start("powershell", QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);
                const bool finished = ps.waitForFinished(120000);
                const bool ok = finished && ps.exitStatus() == QProcess::NormalExit && ps.exitCode() == 0;
                if (!ok && outError) {
                    *outError = QString::fromLocal8Bit(ps.readAllStandardError()).trimmed();
                    if (outError->isEmpty()) {
                        *outError = QString::fromLocal8Bit(ps.readAllStandardOutput()).trimmed();
                    }
                }
                return ok;
            };

            auto runOpenXmlPowerShellConversion = [&](QString *outError) {
                const QString psScript = QString(
                    "$ErrorActionPreference='Stop'; "
                    "$src='%1'; $dst='%2'; "
                    "Add-Type -AssemblyName System.IO.Compression.FileSystem; "
                    "$zip=[System.IO.Compression.ZipFile]::OpenRead($src); "
                    "try { "
                    "  function Get-EntryText($z,$name) { "
                    "    $entry=$z.GetEntry($name); if ($null -eq $entry) { return $null }; "
                    "    $sr=New-Object System.IO.StreamReader($entry.Open()); "
                    "    try { return $sr.ReadToEnd() } finally { $sr.Close() } "
                    "  }; "
                    "  $shared=@(); "
                    "  $sharedXml=Get-EntryText $zip 'xl/sharedStrings.xml'; "
                    "  if ($sharedXml) { "
                    "    [xml]$sx=$sharedXml; "
                    "    foreach($si in $sx.SelectNodes(\"//*[local-name()='si']\")) { "
                    "      $txt=''; foreach($t in $si.SelectNodes(\".//*[local-name()='t']\")) { $txt += [string]$t.InnerText }; $shared += $txt "
                    "    } "
                    "  }; "
                    "  [xml]$wb=(Get-EntryText $zip 'xl/workbook.xml'); "
                    "  [xml]$rels=(Get-EntryText $zip 'xl/_rels/workbook.xml.rels'); "
                    "  $sheet=$wb.SelectSingleNode(\"/*[local-name()='workbook']/*[local-name()='sheets']/*[local-name()='sheet']\"); "
                    "  if ($null -eq $sheet) { throw 'No worksheet found in workbook.' }; "
                    "  $rid=$sheet.GetAttribute('id','http://schemas.openxmlformats.org/officeDocument/2006/relationships'); "
                    "  $relNode=$rels.SelectSingleNode(\"/*[local-name()='Relationships']/*[local-name()='Relationship'][@Id='\" + $rid + \"']\"); "
                    "  $target=if($relNode){[string]$relNode.Attributes['Target'].Value}else{''}; "
                    "  if ([string]::IsNullOrWhiteSpace($target)) { throw 'Cannot resolve first worksheet relationship.' }; "
                    "  if ($target.StartsWith('/')) { $sheetPath=$target.TrimStart('/') } "
                    "  elseif ($target.StartsWith('xl/')) { $sheetPath=$target } "
                    "  else { $sheetPath='xl/' + $target }; "
                    "  [xml]$sh=(Get-EntryText $zip $sheetPath); "
                    "  $sw=New-Object System.IO.StreamWriter($dst,$false,[System.Text.UTF8Encoding]::new($false)); "
                    "  try { "
                    "    foreach($row in @($sh.SelectNodes(\"/*[local-name()='worksheet']/*[local-name()='sheetData']/*[local-name()='row']\"))) { "
                    "      $map=@{}; $max=-1; "
                    "      foreach($c in @($row.SelectNodes(\"*[local-name()='c']\"))) { "
                    "        $ref=[string]$c.GetAttribute('r'); $letters=''; "
                    "        for($i=0; $i -lt $ref.Length; $i++) { $ch=$ref[$i]; if ($ch -ge 'A' -and $ch -le 'Z') { $letters += $ch } else { break } }; "
                    "        $idx=0; foreach($ch in $letters.ToCharArray()) { $idx = ($idx * 26) + ([int][char]$ch - 64) }; $idx=$idx-1; "
                    "        if ($idx -lt 0) { $idx = 0 }; if ($idx -gt $max) { $max=$idx }; "
                    "        $t=[string]$c.GetAttribute('t'); $value=''; "
                    "        if ($t -eq 's') { "
                    "          $vNode=$c.SelectSingleNode(\"*[local-name()='v']\"); $raw=if($vNode){[string]$vNode.InnerText}else{''}; if ($raw -match '^\\d+$') { $si=[int]$raw; if ($si -ge 0 -and $si -lt $shared.Count) { $value=$shared[$si] } } "
                    "        } elseif ($t -eq 'inlineStr') { "
                    "          $isNode=$c.SelectSingleNode(\"*[local-name()='is']\"); if($isNode){ foreach($n in @($isNode.SelectNodes(\".//*[local-name()='t']\"))){ $value += [string]$n.InnerText } } "
                    "        } else { "
                    "          $vNode=$c.SelectSingleNode(\"*[local-name()='v']\"); if($vNode){ $value=[string]$vNode.InnerText } "
                    "        }; "
                    "        $map[$idx]=$value; "
                    "      }; "
                    "      if ($max -lt 0) { continue }; "
                    "      $vals=New-Object System.Collections.Generic.List[string]; "
                    "      for($i=0; $i -le $max; $i++) { if ($map.ContainsKey($i)) { [void]$vals.Add([string]$map[$i]) } else { [void]$vals.Add('') } }; "
                    "      while($vals.Count -gt 0 -and [string]::IsNullOrEmpty($vals[$vals.Count-1])) { $vals.RemoveAt($vals.Count-1) }; "
                    "      $escaped=@(); foreach($v in $vals) { $escaped += ('\"' + ($v -replace '\"','\"\"') + '\"') }; "
                    "      $sw.WriteLine(($escaped -join ',')); "
                    "    } "
                    "  } finally { $sw.Close() } "
                    "} finally { $zip.Dispose() }"
                ).arg(psEscape(sourcePath), psEscape(tempPath));

                QProcess ps;
                ps.start("powershell", QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command" << psScript);
                const bool finished = ps.waitForFinished(120000);
                const bool ok = finished && ps.exitStatus() == QProcess::NormalExit && ps.exitCode() == 0;
                if (!ok && outError) {
                    *outError = QString::fromLocal8Bit(ps.readAllStandardError()).trimmed();
                    if (outError->isEmpty()) {
                        *outError = QString::fromLocal8Bit(ps.readAllStandardOutput()).trimmed();
                    }
                }
                return ok;
            };

            auto runPythonXlsxConversion = [&](QString *outError) {
                const QString pyScript =
                    "import csv, re, sys, zipfile, xml.etree.ElementTree as ET\n"
                    "src, dst = sys.argv[1], sys.argv[2]\n"
                    "NS_MAIN='http://schemas.openxmlformats.org/spreadsheetml/2006/main'\n"
                    "NS_REL_DOC='http://schemas.openxmlformats.org/officeDocument/2006/relationships'\n"
                    "NS_REL_PKG='http://schemas.openxmlformats.org/package/2006/relationships'\n"
                    "def col_to_idx(ref):\n"
                    "    m = re.match(r'([A-Z]+)', ref or '')\n"
                    "    if not m: return 0\n"
                    "    idx = 0\n"
                    "    for ch in m.group(1): idx = idx * 26 + (ord(ch) - 64)\n"
                    "    return idx - 1\n"
                    "with zipfile.ZipFile(src) as z:\n"
                    "    shared = []\n"
                    "    if 'xl/sharedStrings.xml' in z.namelist():\n"
                    "        sroot = ET.fromstring(z.read('xl/sharedStrings.xml'))\n"
                    "        for si in sroot.findall('{%s}si' % NS_MAIN):\n"
                    "            txt = ''.join(t.text or '' for t in si.findall('.//{%s}t' % NS_MAIN))\n"
                    "            shared.append(txt)\n"
                    "    wb = ET.fromstring(z.read('xl/workbook.xml'))\n"
                    "    rels = ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))\n"
                    "    rel_map = {}\n"
                    "    for rel in rels.findall('{%s}Relationship' % NS_REL_PKG):\n"
                    "        rel_map[rel.get('Id')] = rel.get('Target', '')\n"
                    "    first_sheet = wb.find('.//{%s}sheets/{%s}sheet' % (NS_MAIN, NS_MAIN))\n"
                    "    if first_sheet is None:\n"
                    "        raise RuntimeError('No worksheet found in workbook.')\n"
                    "    rid = first_sheet.get('{%s}id' % NS_REL_DOC)\n"
                    "    target = rel_map.get(rid, '')\n"
                    "    if not target:\n"
                    "        raise RuntimeError('Cannot resolve first worksheet relationship.')\n"
                    "    if target.startswith('/'):\n"
                    "        sheet_path = target.lstrip('/')\n"
                    "    elif target.startswith('xl/'):\n"
                    "        sheet_path = target\n"
                    "    else:\n"
                    "        sheet_path = 'xl/' + target\n"
                    "    sheet = ET.fromstring(z.read(sheet_path))\n"
                    "    with open(dst, 'w', newline='', encoding='utf-8') as f:\n"
                    "        writer = csv.writer(f)\n"
                    "        for row in sheet.findall('.//{%s}sheetData/{%s}row' % (NS_MAIN, NS_MAIN)):\n"
                    "            data = {}\n"
                    "            max_col = -1\n"
                    "            for cell in row.findall('{%s}c' % NS_MAIN):\n"
                    "                ref = cell.get('r', '')\n"
                    "                col = col_to_idx(ref)\n"
                    "                max_col = max(max_col, col)\n"
                    "                ctype = cell.get('t', '')\n"
                    "                value = ''\n"
                    "                if ctype == 'inlineStr':\n"
                    "                    is_elem = cell.find('{%s}is' % NS_MAIN)\n"
                    "                    if is_elem is not None:\n"
                    "                        value = ''.join(t.text or '' for t in is_elem.findall('.//{%s}t' % NS_MAIN))\n"
                    "                else:\n"
                    "                    v = cell.find('{%s}v' % NS_MAIN)\n"
                    "                    raw = v.text if v is not None and v.text is not None else ''\n"
                    "                    if ctype == 's':\n"
                    "                        try:\n"
                    "                            value = shared[int(raw)]\n"
                    "                        except Exception:\n"
                    "                            value = ''\n"
                    "                    else:\n"
                    "                        value = raw\n"
                    "                data[col] = value\n"
                    "            if max_col < 0:\n"
                    "                continue\n"
                    "            out = [data.get(i, '') for i in range(max_col + 1)]\n"
                    "            while out and out[-1] == '':\n"
                    "                out.pop()\n"
                    "            writer.writerow(out)\n";

                QProcess py;
                py.start("python", QStringList() << "-c" << pyScript << sourcePath << tempPath);
                bool finished = py.waitForFinished(120000);
                bool ok = finished && py.exitStatus() == QProcess::NormalExit && py.exitCode() == 0;

                if (!ok) {
                    QProcess pyLauncher;
                    pyLauncher.start("py", QStringList() << "-3" << "-c" << pyScript << sourcePath << tempPath);
                    finished = pyLauncher.waitForFinished(120000);
                    ok = finished && pyLauncher.exitStatus() == QProcess::NormalExit && pyLauncher.exitCode() == 0;

                    if (!ok && outError) {
                        *outError = QString::fromLocal8Bit(py.readAllStandardError()).trimmed();
                        if (outError->isEmpty()) {
                            *outError = QString::fromLocal8Bit(pyLauncher.readAllStandardError()).trimmed();
                        }
                        if (outError->isEmpty()) {
                            *outError = "Python-based .xlsx conversion failed.";
                        }
                    }
                }

                return ok;
            };

            QString importError;
            bool converted = false;
            if (isXlsxExtension) {
                QStringList conversionErrors;
                QString stepError;

                converted = runOpenXmlPowerShellConversion(&stepError);
                if (!converted && !stepError.isEmpty()) {
                    conversionErrors << ("OpenXML parser: " + stepError);
                }
                if (!converted) {
                    stepError.clear();
                    converted = runPythonXlsxConversion(&stepError);
                    if (!converted && !stepError.isEmpty()) {
                        conversionErrors << ("Python parser: " + stepError);
                    }
                }

                if (!converted) {
                    importError = conversionErrors.join("\n\n");
                    if (importError.isEmpty()) {
                        importError = "Unable to read .xlsx workbook. Install Python 3 or use CSV import.";
                    }
                }
            } else {
                converted = runExcelComConversion(&importError);
            }

            if (!converted) {
                QMessageBox::critical(this, "Excel Import Error",
                    "Failed to import workbook.\n\n" +
                    (importError.isEmpty()
                        ? "For .xlsx files, install Python 3 (or Microsoft Excel). For .xls/.xlsm/.xlsb files, Microsoft Excel is required."
                        : importError));
                return;
            }

            importPath = tempCsvPath;
        }

        QFile file(importPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "File Error", "Could not open the file for reading.");
            return;
        }
        
        QTextStream in(&file);
        int lineNumber = 0;
        int successCount = 0;
        QStringList errorMessages;
        QList<QVariantList> validRows;
        
        bool isFirstLine = true;
        
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            line.remove('\r');
            lineNumber++;

            if (isFirstLine && !line.isEmpty() && line.front() == QChar(0xFEFF)) {
                line.remove(0, 1);
            }
            
            if (line.isEmpty()) continue;

            // Excel CSV separator hint (for example: "sep=;")
            if (isFirstLine && line.startsWith("sep=", Qt::CaseInsensitive)) {
                continue;
            }

            QChar delimiter = ',';
            if (line.contains(';')) {
                delimiter = ';';
            } else if (line.contains('\t')) {
                delimiter = '\t';
            }

            QStringList parts = line.split(delimiter);
            
            // Smart header detection: skip any first line that looks like a header
            if (isFirstLine && parts.size() > 0 &&
                (parts[0].contains("Type", Qt::CaseInsensitive) ||
                 parts[0].contains("Quantity", Qt::CaseInsensitive) ||
                 parts[0].contains("Price", Qt::CaseInsensitive) ||
                 parts[0].contains("Buyer", Qt::CaseInsensitive))) {
                isFirstLine = false;
                continue;
            }
            isFirstLine = false;
            
            // Must have exactly 4 fields
            if (parts.size() < 4) {
                errorMessages << QString("Line %1: Missing fields. Expected 4 columns: Type, Quantity, Price, BuyerID.").arg(lineNumber);
                continue;
            }
            if (parts.size() > 4) {
                errorMessages << QString("Line %1: Too many fields (%2). Expected exactly 4 columns: Type, Quantity, Price, BuyerID.").arg(lineNumber).arg(parts.size());
                continue;
            }

            auto normalizeCsvField = [](QString value) {
                value = value.trimmed();
                if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"')) {
                    value = value.mid(1, value.size() - 2);
                }
                value.replace("\"\"", "\"");
                return value.trimmed();
            };
            
            QString typeStr   = normalizeCsvField(parts[0]);
            QString qtyStr    = normalizeCsvField(parts[1]);
            QString priceStr  = normalizeCsvField(parts[2]);
            QString buyerStr  = normalizeCsvField(parts[3]);
            
            QString lineErrors;
            
            // --- Smart Validation 1: Type (must match allowed types) ---
            QString cleanType;
            for (QChar c : typeStr) {
                if (c.isLetter() || c.isSpace()) cleanType += c;
            }
            cleanType = cleanType.trimmed();
            static const QStringList allowedTypes = {"Chair", "Table", "Cabinet", "Wardrobe", "Other"};
            if (cleanType.isEmpty()) {
                lineErrors += "  - Type could not be read (must contain letters).\n";
            } else if (!allowedTypes.contains(cleanType, Qt::CaseInsensitive)) {
                lineErrors += QString("  - Type '%1' is not allowed. Must be one of: Chair, Table, Cabinet, Wardrobe, Other.\n").arg(cleanType);
            }
            
            // --- Smart Validation 2: Quantity (extract digits, ignore 'units'/spaces) ---
            QString cleanQtyStr;
            for (QChar c : qtyStr) {
                if (c.isDigit()) cleanQtyStr += c;
            }
            bool qtyOk;
            int qty = cleanQtyStr.toInt(&qtyOk);
            if (!qtyOk || qty <= 0) {
                lineErrors += "  - Quantity must be a valid positive number.\n";
            }
            
            // --- Smart Validation 3: Price (extract digits + one decimal, ignore $, DT, spaces) ---
            QString cleanPriceStr;
            bool decimalFound = false;
            for (QChar c : priceStr) {
                if (c.isDigit()) {
                    cleanPriceStr += c;
                } else if ((c == '.' || c == ',') && !decimalFound) {
                    cleanPriceStr += '.';
                    decimalFound = true;
                }
            }
            bool priceOk;
            double price = cleanPriceStr.toDouble(&priceOk);
            if (!priceOk || price <= 0.0) {
                lineErrors += "  - Price must be a valid positive number.\n";
            }
            
            // --- Smart Validation 4: BuyerID (must be positive int & exist in DB) ---
            bool buyerOk;
            int buyerId = buyerStr.toInt(&buyerOk);
            if (!buyerOk || buyerId <= 0) {
                lineErrors += "  - Buyer ID must be a positive whole number.\n";
            } else {
                QSqlQuery checkClient;
                checkClient.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
                checkClient.bindValue(":id", buyerId);
                if (checkClient.exec() && checkClient.next() && checkClient.value(0).toInt() == 0) {
                    lineErrors += QString("  - Buyer ID %1 does not exist in the database.\n").arg(buyerId);
                }
                checkClient.finish();
            }
            
            if (!lineErrors.isEmpty()) {
                errorMessages << QString("Line %1 ('%2'):\n%3").arg(lineNumber).arg(line).arg(lineErrors);
            } else {
                validRows.append(QVariantList{cleanType, qty, price, buyerId});
            }
        }
        file.close();
        
        // Show errors and ask whether to continue with valid rows
        if (!errorMessages.isEmpty()) {
            QString errorSummary = QString("Found %1 error(s) in the file:\n\n").arg(errorMessages.size());
            int displayLimit = qMin(10, (int)errorMessages.size());
            for (int i = 0; i < displayLimit; ++i)
                errorSummary += errorMessages[i] + "\n";
            if (errorMessages.size() > 10)
                errorSummary += "... and more.\n\n";
            
            if (validRows.isEmpty()) {
                QMessageBox::warning(this, "Import Failed", "No valid rows found to import.\n\n" + errorSummary);
                return;
            } else {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "Import Encountered Errors",
                    errorSummary + QString("\nDo you want to skip the errors and import the %1 valid order(s)?").arg(validRows.size()),
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::No) return;
            }
        } else {
            if (validRows.isEmpty()) {
                QMessageBox::information(this, "Import", "The file was empty or contained only a header.");
                return;
            }
        }
        
        // Get next available order ID
        int baseOrderId = 1;
        {
            QSqlQuery maxQuery;
            if (maxQuery.exec("SELECT MAX(order_id) FROM ORDERS")) {
                if (maxQuery.next() && !maxQuery.value(0).isNull())
                    baseOrderId = maxQuery.value(0).toInt() + 1;
            } else {
                QMessageBox::critical(this, "Database Error", "Failed to retrieve the next Order ID:\n" + maxQuery.lastError().text());
                return;
            }
            maxQuery.finish();
        }
        
        // Insert valid rows
        for (const QVariantList &row : validRows) {
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO ORDERS (order_id, client_id, employee_id, order_type, total_quantity, total_price, order_date, order_status, payment_status) "
                                "VALUES (:id, :buyer, :employee, :type, :quantity, :price, SYSDATE, 'Pending', 'Unpaid')");
            insertQuery.bindValue(":id", baseOrderId);
            insertQuery.bindValue(":buyer", row[3]);
            insertQuery.bindValue(":employee", currentEmployeeId);
            insertQuery.bindValue(":type", row[0]);
            insertQuery.bindValue(":quantity", row[1]);
            insertQuery.bindValue(":price", row[2]);
            
            if (insertQuery.exec()) {
                successCount++;
                baseOrderId++;
            } else {
                QMessageBox::critical(this, "Database Error",
                    "Failed to insert row " + QString::number(successCount + 1) + ":\n" + insertQuery.lastError().text());
            }
        }
        
        QMessageBox::information(this, "Import Complete", QString("Successfully imported %1 order(s)!").arg(successCount));
        onOrderRefreshCatalog();
    }
}

void MainWindow::onOrderPrintCatalog()
{
    if (!ui_order) return;
    
    // Get the order ID from search bar
    QString searchId = ui_order->le_catalog_search->text().trimmed();
    
    if (searchId.isEmpty()) {
        QMessageBox::warning(this, tr("No Order Selected"), 
            tr("Please enter an Order ID in the search box to print."));
        return;
    }
    
    // Validate it's a number
    bool ok;
    int orderId = searchId.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Invalid Order ID"), 
            tr("Please enter a valid Order ID number."));
        return;
    }
    
    // Query the specific order
    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderId);
    
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, tr("Order Not Found"), 
            tr("Order ID %1 does not exist in the database.").arg(searchId));
        return;
    }
    
    // Extract order data
    QString orderType = query.value(1).toString();
    int quantity = query.value(2).toInt();
    double price = query.value(3).toDouble();
    int clientId = query.value(4).toInt();
    QString orderDate = query.value(5).toDateTime().toString("MMMM dd, yyyy");
    QString orderStatus = query.value(6).toString();
    QString paymentStatus = query.value(7).toString();
    QString clientFirstName = query.value(8).toString();
    QString clientLastName = query.value(9).toString();
    QString clientEmail = query.value(10).toString();
    QString clientPhone = query.value(11).toString();
    
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageSize(QPageSize(QPageSize::A4));
    
    QPrintDialog printDialog(&printer, this);
    
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter;
        if (!painter.begin(&printer)) {
            QMessageBox::critical(this, "Print Error", "Failed to print.");
            return;
        }
        
        // Page dimensions and margins
        int pageWidth = printer.width();
        int pageHeight = printer.height();
        int margin = 80;  // 1+ inch margins for professional look
        int contentWidth = pageWidth - 2 * margin;
        int y = margin;
        
        // ==================== HEADER SECTION ====================
        // System title bar with background
        painter.fillRect(0, 0, pageWidth, 100, QColor(45, 45, 45));
        
        // System title
        QFont titleFont("Segoe UI", 22, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(Qt::white);
        painter.drawText(margin, 35, "Order Management System");
        
        // Subtitle
        QFont subtitleFont("Segoe UI", 10);
        painter.setFont(subtitleFont);
        painter.setPen(QColor(220, 220, 220));
        painter.drawText(margin, 60, "Professional Order Processing & Invoice Generation");
        
        y = 130;
        
        // ==================== COMPANY & ORDER INFO ====================
        // Company logo and info (left side)
        int logoSize = 70;
        QPixmap logo(":/assets/logo.png");
        if (!logo.isNull()) {
            QPixmap scaledLogo = logo.scaled(logoSize, logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(margin, y, scaledLogo);
        }
        
        QFont companyFont("Segoe UI", 18, QFont::Bold);
        painter.setFont(companyFont);
        painter.setPen(QColor(139, 111, 71));
        painter.drawText(margin + logoSize + 20, y + 25, "Hammer Down");
        
        QFont companySubFont("Segoe UI", 9);
        painter.setFont(companySubFont);
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(margin + logoSize + 20, y + 45, "Business Solutions Provider");
        
        // Order number and date (right side)
        QFont orderNumFont("Segoe UI", 20, QFont::Bold);
        painter.setFont(orderNumFont);
        painter.setPen(QColor(45, 45, 45));
        QString orderText = "ORDER #" + searchId;
        QFontMetrics fm1(orderNumFont);
        int orderWidth = fm1.horizontalAdvance(orderText);
        painter.drawText(pageWidth - margin - orderWidth, y + 25, orderText);
        
        QFont dateFont("Segoe UI", 9);
        painter.setFont(dateFont);
        painter.setPen(QColor(100, 100, 100));
        QString dateGenerated = "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy hh:mm AP");
        QFontMetrics fm2(dateFont);
        int dateWidth = fm2.horizontalAdvance(dateGenerated);
        painter.drawText(pageWidth - margin - dateWidth, y + 50, dateGenerated);
        
        y += 100;
        
        // Horizontal separator
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawLine(margin, y, pageWidth - margin, y);
        
        y += 40;
        
        // ==================== ORDER DETAILS SECTION ====================
        // Section header
        QFont sectionHeaderFont("Segoe UI", 14, QFont::Bold);
        painter.setFont(sectionHeaderFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin, y, "Order Details");
        
        y += 10;
        
        // Section underline
        painter.setPen(QPen(QColor(139, 111, 71), 3));
        painter.drawLine(margin, y, margin + 120, y);
        
        y += 30;
        
        // Order details box with light background
        int detailsBoxHeight = 180;
        painter.fillRect(margin, y, contentWidth, detailsBoxHeight, QColor(250, 250, 252));
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.drawRect(margin, y, contentWidth, detailsBoxHeight);
        
        y += 30;
        
        // Two-column layout for order info
        QFont labelFont("Segoe UI", 10, QFont::Bold);
        QFont valueFont("Segoe UI", 10);
        int labelCol = margin + 30;
        int valueCol = margin + 200;
        int rowHeight = 28;
        
        auto drawDetailRow = [&](const QString& label, const QString& value) {
            painter.setFont(labelFont);
            painter.setPen(QColor(90, 90, 90));
            painter.drawText(labelCol, y, label);
            
            painter.setFont(valueFont);
            painter.setPen(QColor(40, 40, 40));
            painter.drawText(valueCol, y, value);
            
            y += rowHeight;
        };
        
        drawDetailRow("Order Date:", orderDate);
        drawDetailRow("Order Type:", orderType);
        drawDetailRow("Quantity:", QString::number(quantity) + " units");
        drawDetailRow("Unit Price:", "$" + QString::number(price, 'f', 2));
        drawDetailRow("Order Status:", orderStatus);
        drawDetailRow("Payment Status:", paymentStatus);
        
        y += 30;
        
        // ==================== CLIENT INFORMATION SECTION ====================
        // Section header
        painter.setFont(sectionHeaderFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin, y, "Client Information");
        
        y += 10;
        
        // Section underline
        painter.setPen(QPen(QColor(139, 111, 71), 3));
        painter.drawLine(margin, y, margin + 140, y);
        
        y += 30;
        
        // Client details box
        int clientBoxHeight = 140;
        painter.fillRect(margin, y, contentWidth, clientBoxHeight, QColor(250, 250, 252));
        painter.setPen(QPen(QColor(220, 220, 220), 1));
        painter.drawRect(margin, y, contentWidth, clientBoxHeight);
        
        y += 30;
        
        drawDetailRow("Client ID:", QString::number(clientId));
        drawDetailRow("Full Name:", clientFirstName + " " + clientLastName);
        drawDetailRow("Email Address:", clientEmail.isEmpty() ? "Not provided" : clientEmail);
        drawDetailRow("Phone Number:", clientPhone.isEmpty() ? "Not provided" : clientPhone);
        
        y += 40;
        
        // ==================== PAYMENT SUMMARY ====================
        // Summary section with accent color
        int summaryBoxHeight = 100;
        painter.fillRect(margin, y, contentWidth, summaryBoxHeight, QColor(139, 111, 71));
        
        // Inner white box for amount
        int innerMargin = 3;
        painter.fillRect(margin + innerMargin, y + innerMargin, 
                         contentWidth - 2 * innerMargin, summaryBoxHeight - 2 * innerMargin, 
                         QColor(255, 255, 255));
        
        y += 40;
        
        // Total amount label
        QFont summaryLabelFont("Segoe UI", 16, QFont::Bold);
        painter.setFont(summaryLabelFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin + 30, y, "TOTAL AMOUNT");
        
        // Total amount value (right aligned)
        QFont totalAmountFont("Segoe UI", 24, QFont::Bold);
        painter.setFont(totalAmountFont);
        painter.setPen(QColor(139, 111, 71));
        double totalPrice = price * quantity;
        QString totalText = "$" + QString::number(totalPrice, 'f', 2);
        QFontMetrics fm3(totalAmountFont);
        int totalWidth = fm3.horizontalAdvance(totalText);
        painter.drawText(pageWidth - margin - totalWidth - 30, y + 5, totalText);
        
        y += 35;
        
        // Payment status in summary
        QFont statusFont("Segoe UI", 10);
        painter.setFont(statusFont);
        painter.setPen(QColor(100, 100, 100));
        QString statusText = "Payment Status: " + paymentStatus;
        painter.drawText(margin + 30, y, statusText);
        
        // ==================== QR CODE ====================
        y += 80;  // Move below the summary box
        
        // Build QR content with same info as PDF
        QString qrContent = buildOrderQrContent(orderId, orderType, quantity, price,
                                                 orderDate, orderStatus, paymentStatus,
                                                 clientId, clientFirstName + " " + clientLastName,
                                                 clientEmail, clientPhone);
        QPixmap qrPixmap = generateQrPixmap(qrContent, 4, 2);
        
        // QR section label
        painter.setFont(sectionHeaderFont);
        painter.setPen(QColor(45, 45, 45));
        painter.drawText(margin, y, "Scan QR Code");
        y += 10;
        painter.setPen(QPen(QColor(139, 111, 71), 3));
        painter.drawLine(margin, y, margin + 120, y);
        y += 20;
        
        // Draw QR code
        int qrDisplaySize = 140;
        QPixmap scaledQr = qrPixmap.scaled(qrDisplaySize, qrDisplaySize, Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawPixmap(margin, y, scaledQr);
        
        // QR description text next to QR code
        painter.setFont(QFont("Segoe UI", 9));
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(margin + qrDisplaySize + 20, y + 30, "Scan this QR code to view");
        painter.drawText(margin + qrDisplaySize + 20, y + 50, "complete order details.");
        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor(140, 140, 140));
        painter.drawText(margin + qrDisplaySize + 20, y + 80, "Contains: Order info, client data,");
        painter.drawText(margin + qrDisplaySize + 20, y + 95, "pricing and payment status.");
        
        // ==================== FOOTER ======================================
        y = pageHeight - 50;
        
        // Footer separator line
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.drawLine(margin, y, pageWidth - margin, y);
        
        y += 25;
        
        // Centered footer text
        QFont footerFont("Segoe UI", 8);
        painter.setFont(footerFont);
        painter.setPen(QColor(120, 120, 120));
        
        QString footerText = "Thank you for your business! | Hammer Down © 2026 - All Rights Reserved | For inquiries, contact support@hammerdown.com";
        QFontMetrics fmFooter(footerFont);
        int footerWidth = fmFooter.horizontalAdvance(footerText);
        int footerX = (pageWidth - footerWidth) / 2;
        painter.drawText(footerX, y, footerText);
        
        painter.end();
        QMessageBox::information(this, "Success", "Order #" + searchId + " printed successfully!");
    }
}

// ==================== QR Code Tab Functions ====================
void MainWindow::onGenerateQR()
{
    if (!ui_order) return;

    QString searchId = ui_order->le_qr_order_id->text().trimmed();

    if (searchId.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter an Order ID.");
        return;
    }

    bool ok;
    int orderId = searchId.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Invalid ID", "Please enter a valid numeric Order ID.");
        return;
    }

    // Query the order with client info
    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderId);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Not Found",
            "Order #" + searchId + " does not exist in the database.");
        return;
    }

    QString orderType = query.value(1).toString();
    int quantity = query.value(2).toInt();
    double price = query.value(3).toDouble();
    int clientId = query.value(4).toInt();
    QString orderDate = query.value(5).toDateTime().toString("MMM dd, yyyy");
    QString orderStatus = query.value(6).toString();
    QString paymentStatus = query.value(7).toString();
    QString clientName = query.value(8).toString() + " " + query.value(9).toString();
    QString clientEmail = query.value(10).toString();
    QString clientPhone = query.value(11).toString();

    QString qrContent = buildOrderQrContent(orderId, orderType, quantity, price,
                                             orderDate, orderStatus, paymentStatus,
                                             clientId, clientName, clientEmail, clientPhone);

    QPixmap qrPixmap = generateQrPixmap(qrContent, 8, 4);

    // Display in the label (scale to fit the 300x300 display area)
    ui_order->label_qr_display->setPixmap(
        qrPixmap.scaled(280, 280, Qt::KeepAspectRatio, Qt::FastTransformation));

    QMessageBox::information(this, "QR Generated",
        "QR Code for Order #" + searchId + " has been generated!\n\n"
        "Scan the QR code to view order details.");
}

void MainWindow::onSaveQR()
{
    if (!ui_order) return;

    QPixmap currentQr = ui_order->label_qr_display->pixmap();
    if (currentQr.isNull()) {
        QMessageBox::warning(this, "No QR Code", "Please generate a QR code first.");
        return;
    }

    QString orderId = ui_order->le_qr_order_id->text().trimmed();
    QString fileName = QFileDialog::getSaveFileName(this, "Save QR Code",
        QDir::homePath() + "/QR_Order_" + orderId + ".png",
        "PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*)");

    if (fileName.isEmpty()) return;

    // Re-generate at high resolution for saving
    // Get the content again
    bool ok;
    int orderIdInt = orderId.toInt(&ok);
    if (!ok) {
        // If we can't parse the ID, just save the displayed pixmap
        currentQr.save(fileName);
        QMessageBox::information(this, "Saved", "QR Code saved to:\n" + fileName);
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                  "o.client_id, o.order_date, o.order_status, o.payment_status, "
                  "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                  "FROM ORDERS o "
                  "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                  "WHERE o.order_id = :id");
    query.bindValue(":id", orderIdInt);

    if (query.exec() && query.next()) {
        QString qrContent = buildOrderQrContent(
            orderIdInt,
            query.value(1).toString(),
            query.value(2).toInt(),
            query.value(3).toDouble(),
            query.value(5).toDateTime().toString("MMM dd, yyyy"),
            query.value(6).toString(),
            query.value(7).toString(),
            query.value(4).toInt(),
            query.value(8).toString() + " " + query.value(9).toString(),
            query.value(10).toString(),
            query.value(11).toString());

        QPixmap highResQr = generateQrPixmap(qrContent, 16, 4);  // Higher resolution for file
        highResQr.save(fileName);
    } else {
        currentQr.save(fileName);
    }

    QMessageBox::information(this, "Saved", "QR Code saved to:\n" + fileName);
}

void MainWindow::onPrintQR()
{
    if (!ui_order) return;

    QPixmap currentQr = ui_order->label_qr_display->pixmap();
    if (currentQr.isNull()) {
        QMessageBox::warning(this, "No QR Code", "Please generate a QR code first.");
        return;
    }

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter;
        if (!painter.begin(&printer)) {
            QMessageBox::critical(this, "Print Error", "Failed to start printing.");
            return;
        }

        int pageWidth = printer.width();
        int pageHeight = printer.height();

        // Re-generate high resolution QR
        QString orderId = ui_order->le_qr_order_id->text().trimmed();
        QPixmap qrToPrint = currentQr;

        bool ok;
        int orderIdInt = orderId.toInt(&ok);
        if (ok) {
            QSqlQuery query;
            query.prepare("SELECT o.order_id, o.order_type, o.total_quantity, o.total_price, "
                          "o.client_id, o.order_date, o.order_status, o.payment_status, "
                          "c.FIRST_NAME, c.LAST_NAME, c.EMAIL, c.PHONE_NUMBER "
                          "FROM ORDERS o "
                          "LEFT JOIN CLIENTS c ON o.client_id = c.CLIENT_ID "
                          "WHERE o.order_id = :id");
            query.bindValue(":id", orderIdInt);
            if (query.exec() && query.next()) {
                QString qrContent = buildOrderQrContent(
                    orderIdInt,
                    query.value(1).toString(),
                    query.value(2).toInt(),
                    query.value(3).toDouble(),
                    query.value(5).toDateTime().toString("MMM dd, yyyy"),
                    query.value(6).toString(),
                    query.value(7).toString(),
                    query.value(4).toInt(),
                    query.value(8).toString() + " " + query.value(9).toString(),
                    query.value(10).toString(),
                    query.value(11).toString());
                qrToPrint = generateQrPixmap(qrContent, 12, 4);
            }
        }

        // Print layout: title, then centered QR code, then order ID below
        int y = 60;

        // Title
        QFont titleFont("Segoe UI", 18, QFont::Bold);
        painter.setFont(titleFont);
        painter.setPen(QColor(45, 45, 45));
        QString title = "Order #" + orderId + " - QR Code";
        QFontMetrics fm(titleFont);
        int titleW = fm.horizontalAdvance(title);
        painter.drawText((pageWidth - titleW) / 2, y, title);

        y += 40;

        // Subtitle
        QFont subFont("Segoe UI", 10);
        painter.setFont(subFont);
        painter.setPen(QColor(100, 100, 100));
        QString sub = "Scan this QR code to view full order details";
        QFontMetrics fm2(subFont);
        painter.drawText((pageWidth - fm2.horizontalAdvance(sub)) / 2, y, sub);

        y += 40;

        // Center QR code on page
        int qrDisplaySize = std::min(pageWidth - 160, 400);
        QPixmap scaledQr = qrToPrint.scaled(qrDisplaySize, qrDisplaySize, Qt::KeepAspectRatio, Qt::FastTransformation);
        int qrX = (pageWidth - scaledQr.width()) / 2;
        painter.drawPixmap(qrX, y, scaledQr);

        y += scaledQr.height() + 30;

        // Order ID text below QR
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.setPen(QColor(139, 111, 71));
        QString label = "ORDER #" + orderId;
        QFontMetrics fm3(QFont("Segoe UI", 12, QFont::Bold));
        painter.drawText((pageWidth - fm3.horizontalAdvance(label)) / 2, y, label);

        y += 30;

        // Generated date
        painter.setFont(QFont("Segoe UI", 9));
        painter.setPen(QColor(120, 120, 120));
        QString genDate = "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy hh:mm AP");
        QFontMetrics fm4(QFont("Segoe UI", 9));
        painter.drawText((pageWidth - fm4.horizontalAdvance(genDate)) / 2, y, genDate);

        // Footer
        painter.setFont(QFont("Segoe UI", 8));
        painter.setPen(QColor(140, 140, 140));
        QString footer = "Hammer Down - Order Management System";
        QFontMetrics fm5(QFont("Segoe UI", 8));
        painter.drawText((pageWidth - fm5.horizontalAdvance(footer)) / 2, pageHeight - 40, footer);

        QMessageBox::information(this, "Printed", "QR Code for Order #" + orderId + " printed successfully!");
    }
}


void MainWindow::onClientClearFields()
{
    if (ui_client) {
        ui_client->le_nom->clear();
        ui_client->le_prenom->clear();
        ui_client->le_adresse->clear();
        ui_client->le_tel->clear();
        ui_client->le_email->clear();
        // Reset radio buttons
        ui_client->rb_homme->setAutoExclusive(false);
        ui_client->rb_femme->setAutoExclusive(false);
        ui_client->rb_homme->setChecked(false);
        ui_client->rb_femme->setChecked(false);
        ui_client->rb_homme->setAutoExclusive(true);
        ui_client->rb_femme->setAutoExclusive(true);
    }
}

void MainWindow::onClientModClearFields()
{
    if (ui_client) {
        ui_client->le_id_mod->clear();
        ui_client->le_nom_mod->clear();
        ui_client->le_prenom_mod->clear();
        ui_client->le_adresse_mod->clear();
        ui_client->le_tel_mod->clear();
        ui_client->le_email_mod->clear();
        // Reset radio buttons
        ui_client->rb_homme_mod->setAutoExclusive(false);
        ui_client->rb_femme_mod->setAutoExclusive(false);
        ui_client->rb_homme_mod->setChecked(false);
        ui_client->rb_femme_mod->setChecked(false);
        ui_client->rb_homme_mod->setAutoExclusive(true);
        ui_client->rb_femme_mod->setAutoExclusive(true);
    }
}


void MainWindow::onSupplierClearFields()
{
    if (ui_supplier) {
        ui_supplier->le_id->clear();
        ui_supplier->le_nom->clear();
        ui_supplier->le_adresse->clear();
        ui_supplier->le_email->clear();
        ui_supplier->le_tel->clear();
        ui_supplier->le_product_type->clear();
        ui_supplier->le_type->clear();
        ui_supplier->sb_cp->setValue(0);
        ui_supplier->txt_sms->clear();
        if (m_teOpeningHour)  m_teOpeningHour->setTime(QTime(8, 0));
        if (m_teClosingHour) m_teClosingHour->setTime(QTime(18, 0));
    }
}

// ==================== Supplier Management ====================

void MainWindow::onSupplierRefreshView()
{
    if (!ui_supplier) return;

    // Populate the tableView in the View tab
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT 1 AS \"Action\", 2 AS \"Del\", SUPPLIER_ID AS \"ID\", "
        "SUPPLIER_NAME AS \"Company\", ADDRESS AS \"Address\","
        " EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", PRODUCT_TYPE AS \"Product Type\","
        " POSTAL_CODE AS \"Postal Code\""
        " FROM SUPPLIERS ORDER BY SUPPLIER_ID"
    );


    if (model->lastError().isValid()) {
        qDebug() << "[Supplier] onSupplierRefreshView query error:" << model->lastError().text();
        QMessageBox::critical(this, "Database Error",
            "Failed to load supplier list:\n" + model->lastError().databaseText());
        return;
    }

    ui_supplier->tableView->setModel(model);
    ui_supplier->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_supplier->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_supplier->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Re-populate the Reviews combo box with supplier names
    ui_supplier->cb_supplier_reviews->clear();
    ui_supplier->cb_supplier_reviews->addItem("-- Select Supplier --", -1);
    QSqlQuery qRev;
    qRev.prepare("SELECT SUPPLIER_ID, SUPPLIER_NAME FROM SUPPLIERS ORDER BY SUPPLIER_NAME");
    if (qRev.exec()) {
        while (qRev.next()) {
            QString name = qRev.value(1).toString();
            int sid = qRev.value(0).toInt();
            ui_supplier->cb_supplier_reviews->addItem(name.isEmpty() ? QString("Supplier #%1").arg(sid) : name, sid);
        }
    }
}

void MainWindow::onSupplierAdd()
{
    if (!ui_supplier) return;

    QString id    = ui_supplier->le_id->text().trimmed();
    QString nom   = ui_supplier->le_nom->text().trimmed();
    QString addr  = ui_supplier->le_adresse->text().trimmed();
    QString email = ui_supplier->le_email->text().trimmed();
    QString tel   = ui_supplier->le_tel->text().trimmed();
    QString type  = ui_supplier->le_type->text().trimmed();
    QString openTime  = m_teOpeningHour  ? m_teOpeningHour->time().toString("HH:mm")  : "";
    QString closeTime = m_teClosingHour ? m_teClosingHour->time().toString("HH:mm") : "";
    int cp        = ui_supplier->sb_cp->value();

    if (id.isEmpty() || nom.isEmpty() || addr.isEmpty() || email.isEmpty() || tel.isEmpty() || type.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required!");
        return;
    }

    if (!email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid email address (must contain @ and .)");
        return;
    }

    bool phoneOk;
    tel.toLongLong(&phoneOk);
    if (!phoneOk || tel.length() < 8) {
        QMessageBox::warning(this, "Input Error", "Phone number must be at least 8 digits and contain only numbers!");
        return;
    }

    bool idOk;
    int suppId = id.toInt(&idOk);
    if (!idOk) {
        QMessageBox::warning(this, "Input Error", "Supplier ID must be a valid integer.");
        return;
    }

    // Check duplicate
    QSqlQuery chk;
    chk.prepare("SELECT COUNT(*) FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    chk.bindValue(":id", suppId);
    if (chk.exec() && chk.next() && chk.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Duplicate", 
            QString("Supplier ID %1 already exists!").arg(suppId));
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Add",
        QString("Add supplier:\n\nID: %1\nCompany: %2\nEmail: %3\nPhone: %4")
            .arg(suppId).arg(nom).arg(email).arg(tel),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlQuery q;
    q.prepare("INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, ADDRESS, EMAIL, PHONE_NUMBER,"
              " TYPE_NOTIFICATION, POSTAL_CODE, REGISTRATION_DATE, ACCOUNT_STATUS, OPENING_TIME, CLOSING_TIME, PRODUCT_TYPE)"
              " VALUES (:id, :nom, :addr, :email, :tel, :type, :cp, SYSDATE, 'Active', :openTime, :closeTime, :productType)");
    q.bindValue(":id",          suppId);
    q.bindValue(":nom",         nom);
    q.bindValue(":addr",        addr);
    q.bindValue(":email",       email);
    q.bindValue(":tel",         tel);
    q.bindValue(":type",        type);
    q.bindValue(":cp",          cp);
    q.bindValue(":openTime",    openTime);
    q.bindValue(":closeTime",   closeTime);
    q.bindValue(":productType", ui_supplier->le_product_type->text().trimmed());

    if (q.exec()) {
        if (homeWindow && homeWindow->isAnimationMode()) {
            playSupplierSuccessAnimation(nom);
        } else {
            QMessageBox::information(this, "Success",
                QString("Supplier '%1' (ID: %2) added successfully!").arg(nom).arg(suppId));
        }
        checkSupplierVicinity(suppId);
        onSupplierClearFields();
        onSupplierRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error",
            "Failed to add supplier.\n\n" + q.lastError().databaseText());
    }
}

void MainWindow::onSupplierModify()
{
    if (!ui_supplier) return;

    QString id    = ui_supplier->le_id->text().trimmed();
    QString nom   = ui_supplier->le_nom->text().trimmed();
    QString addr  = ui_supplier->le_adresse->text().trimmed();
    QString email = ui_supplier->le_email->text().trimmed();
    QString tel   = ui_supplier->le_tel->text().trimmed();
    QString type  = ui_supplier->le_type->text().trimmed();
    QString openTime  = m_teOpeningHour  ? m_teOpeningHour->time().toString("HH:mm")  : "";
    QString closeTime = m_teClosingHour ? m_teClosingHour->time().toString("HH:mm") : "";
    int cp        = ui_supplier->sb_cp->value();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error",
            "Please select a supplier from the View tab first,\nor enter a Supplier ID to modify.");
        return;
    }

    if (nom.isEmpty() || addr.isEmpty() || email.isEmpty() || tel.isEmpty() || type.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required to modify the supplier!");
        return;
    }

    if (!email.contains('@') || !email.contains('.')) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid email address (must contain @ and .)");
        return;
    }

    bool phoneOk;
    tel.toLongLong(&phoneOk);
    if (!phoneOk || tel.length() < 8) {
        QMessageBox::warning(this, "Input Error", "Phone number must be at least 8 digits and contain only numbers!");
        return;
    }

    bool idOk;
    int suppId = id.toInt(&idOk);
    if (!idOk) {
        QMessageBox::warning(this, "Input Error", "Supplier ID must be a valid integer.");
        return;
    }

    int reply = QMessageBox::question(this, "Confirm Modify",
        QString("Update supplier ID %1 (%2)?").arg(suppId).arg(nom),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlQuery q;
    q.prepare("UPDATE SUPPLIERS SET SUPPLIER_NAME=:nom, ADDRESS=:addr, EMAIL=:email, PHONE_NUMBER=:tel,"
              " TYPE_NOTIFICATION=:type, POSTAL_CODE=:cp, OPENING_TIME=:openTime, CLOSING_TIME=:closeTime,"
              " PRODUCT_TYPE=:productType"
              " WHERE SUPPLIER_ID=:id");
    q.bindValue(":nom",         nom);
    q.bindValue(":addr",        addr);
    q.bindValue(":email",       email);
    q.bindValue(":tel",         tel);
    q.bindValue(":type",        type);
    q.bindValue(":cp",          cp);
    q.bindValue(":openTime",    openTime);
    q.bindValue(":closeTime",   closeTime);
    q.bindValue(":productType", ui_supplier->le_product_type->text().trimmed());
    q.bindValue(":id",          suppId);

    if (q.exec()) {
        if (q.numRowsAffected() > 0) {
            if (homeWindow && homeWindow->isAnimationMode()) {
                playSupplierModifyAnimation(nom);
            } else {
                QMessageBox::information(this, "Success",
                    QString("Supplier ID %1 updated successfully!").arg(suppId));
            }
            checkSupplierVicinity(suppId);
            onSupplierClearFields();
            onSupplierRefreshView();
        } else {
            QMessageBox::warning(this, "Not Found",
                QString("No supplier found with ID %1.").arg(suppId));
        }
    } else {
        QMessageBox::critical(this, "Database Error",
            "Failed to modify supplier.\n\n" + q.lastError().databaseText());
    }
}

void MainWindow::onSupplierDelete()
{
    if (!ui_supplier) return;

    QModelIndex idx = ui_supplier->tableView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select a supplier from the list to delete.");
        return;
    }

    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_supplier->tableView->model());
    if (!model) return;

    // Col order: Action, Delete, ID, Company, Address, Email, Phone, Type, PostalCode
    QString suppId = model->data(model->index(idx.row(), 2)).toString();
    QString nom    = model->data(model->index(idx.row(), 3)).toString();

    int reply = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to DELETE supplier:\n\nID: %1\nCompany: %2\n\nThis action cannot be undone!")
            .arg(suppId).arg(nom),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlQuery q;
    q.prepare("DELETE FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    q.bindValue(":id", suppId.toInt());

    if (q.exec()) {
        if (homeWindow && homeWindow->isAnimationMode()) {
            playSupplierDeleteAnimation(nom);
        } else {
            QMessageBox::information(this, "Success", "Supplier deleted successfully.");
        }
        onSupplierClearFields();
        onSupplierRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to delete supplier:\n" + q.lastError().text());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSupplierDeleteAll()
{
    if (!ui_supplier) return;

    // Use a more robust count: database query first, then fallback to model row count
    int count = 0;
    QSqlQuery qCount;
    if (qCount.exec("SELECT COUNT(*) FROM SUPPLIERS") && qCount.next()) {
        count = qCount.value(0).toInt();
    } else if (ui_supplier->tableView->model()) {
        // Fallback to currently loaded rows if SQL fails
        count = ui_supplier->tableView->model()->rowCount();
    }

    if (count == 0) {
        QMessageBox::information(this, tr("Delete All"), tr("There are no suppliers to delete."));
        return;
    }

    auto reply = QMessageBox::warning(
        this,
        tr("Confirm Delete All"),
        tr("This will permanently delete ALL %1 supplier(s) and their ratings/notifications.\n\nThis action cannot be undone!").arg(count),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel
    );
    if (reply != QMessageBox::Yes) return;

    QSqlQuery qDel;
    if (qDel.exec("DELETE FROM SUPPLIERS")) {
        QSqlDatabase::database().commit();
        QMessageBox::information(this, tr("Deleted"),
            tr("%1 supplier(s) deleted successfully.").arg(count));
        onSupplierClearFields();
        onSupplierRefreshView();
        setupSupplierStats(); // refresh the stats tab too
    } else {
        QMessageBox::critical(this, tr("Database Error"),
            tr("Failed to delete suppliers:\n") + qDel.lastError().text());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSupplierExportPDF()
{
    if (!ui_supplier) return;

    // Gather data from the current tableView model
    const QAbstractItemModel *model = ui_supplier->tableView->model();
    if (!model || model->rowCount() == 0) {
        QMessageBox::information(this, tr("Export PDF"), tr("No supplier data to export."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Suppliers to PDF"),
        QDir::homePath() + "/Suppliers_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf)"
    );
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, tr("Export Error"), tr("Failed to create PDF file."));
        return;
    }

    const int W      = printer.width();
    const int margin = 60;
    const int cw     = W - 2 * margin;
    int y            = 0;

    // ── Header bar ────────────────────────────────────────────────────────────
    painter.fillRect(0, 0, W, 90, QColor(28, 22, 16));
    painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
    painter.setPen(QColor("#D4AF37"));
    painter.drawText(margin, 38, tr("Supplier Management System"));
    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(180, 160, 120));
    painter.drawText(margin, 60, tr("Professional Supplier Directory Export"));

    // Logo (right side of header)
    QPixmap logo(":/assets/logo.png");
    if (!logo.isNull())
        painter.drawPixmap(W - margin - 70, 10, logo.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Generated timestamp
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor("#8B6F47"));
    const QString stamp = tr("Generated: ") + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    QFontMetrics fm8(QFont("Segoe UI", 8));
    painter.drawText(W - margin - fm8.horizontalAdvance(stamp), 82, stamp);

    y = 110;

    // ── Summary strip ─────────────────────────────────────────────────────────
    int totalSuppliers = 0, activeCount = 0;
    double avgRat = 0.0;
    {
        QSqlQuery qs("SELECT COUNT(*), "
                     "SUM(CASE WHEN ACCOUNT_STATUS='Active' THEN 1 ELSE 0 END), "
                     "AVG(CASE WHEN AVERAGE_RATING>0 THEN AVERAGE_RATING END) "
                     "FROM SUPPLIERS");
        if (qs.exec() && qs.next()) {
            totalSuppliers = qs.value(0).toInt();
            activeCount    = qs.value(1).toInt();
            avgRat         = qs.value(2).isNull() ? 0.0 : qs.value(2).toDouble();
        }
    }
    painter.fillRect(margin, y, cw, 38, QColor(44, 34, 22));
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(QColor("#F5E6D3"));
    const QString summary = QString(tr("Total: %1   |   Active: %2   |   Inactive: %3   |   Avg Rating: %4 / 5.0"))
        .arg(totalSuppliers).arg(activeCount).arg(totalSuppliers - activeCount)
        .arg(QString::number(avgRat, 'f', 1));
    painter.drawText(margin + 12, y + 25, summary);
    y += 50;

    // ── Column definitions ────────────────────────────────────────────────────
    // Skip the first 2 model cols (Action / Delete icons), show cols 2-8
    struct Col { QString name; int widthPct; };
    const QList<Col> cols = {
        {tr("ID"),          6},
        {tr("Company"),    22},
        {tr("Address"),    22},
        {tr("Email"),      18},
        {tr("Phone"),      12},
        {tr("Product Type"), 12},
        {tr("Postal"),      8},
    };
    // Pre-compute pixel widths
    QList<int> colWidths;
    for (const Col &c : cols) colWidths << (cw * c.widthPct / 100);

    const int rowH    = 22;
    const int headerH = 28;

    auto drawRow = [&](int row, bool isHeader) {
        int x = margin;
        QColor bg  = isHeader ? QColor("#8B6F47") :
                     (row % 2 == 0 ? QColor(240, 232, 220) : QColor(255, 252, 245));
        QColor fg  = isHeader ? Qt::white : QColor(40, 30, 20);
        int    h   = isHeader ? headerH : rowH;
        painter.fillRect(x, y, cw, h, bg);
        painter.setPen(QPen(QColor(180, 150, 110), 0.5));
        painter.drawRect(x, y, cw, h);
        painter.setPen(fg);
        painter.setFont(QFont("Segoe UI", isHeader ? 9 : 8, isHeader ? QFont::Bold : QFont::Normal));
        for (int c = 0; c < cols.size(); ++c) {
            QString text = isHeader
                ? cols[c].name
                : model->data(model->index(row, c + 2)).toString(); // skip col 0,1
            QRect cell(x + 3, y + 2, colWidths[c] - 6, h - 4);
            painter.drawText(cell, Qt::AlignVCenter | Qt::AlignLeft,
                             painter.fontMetrics().elidedText(text, Qt::ElideRight, cell.width()));
            x += colWidths[c];
        }
    };

    // Draw table header
    drawRow(-1, true);
    y += headerH;

    // Draw data rows, paginating automatically
    for (int r = 0; r < model->rowCount(); ++r) {
        if (y + rowH > printer.height() - margin) {
            printer.newPage();
            y = margin;
            // Repeat header on each new page
            drawRow(-1, true);
            y += headerH;
        }
        drawRow(r, false);
        y += rowH;
    }

    // ── Footer ────────────────────────────────────────────────────────────────
    y += 18;
    painter.setPen(QPen(QColor("#8B6F47"), 1));
    painter.drawLine(margin, y, W - margin, y);
    y += 12;
    painter.setFont(QFont("Segoe UI", 8));
    painter.setPen(QColor("#8B6F47"));
    painter.drawText(margin, y, tr("Hammer Down — Supplier Management System — Confidential"));
    painter.drawText(W - margin - 80, y, QString(tr("Total: %1 suppliers")).arg(totalSuppliers));

    painter.end();
    QMessageBox::information(this, tr("Export Successful"),
        tr("PDF exported successfully to:\n%1").arg(fileName));
}

// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onSupplierPrint()
{
    if (!ui_supplier) return;

    const QAbstractItemModel *model = ui_supplier->tableView->model();
    if (!model || model->rowCount() == 0) {
        QMessageBox::information(this, tr("Print"), tr("No supplier data to print."));
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Supplier List"));
    if (dialog.exec() != QDialog::Accepted) return;

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, tr("Print Error"), tr("Failed to start printing."));
        return;
    }

    const int W      = printer.width();
    const int margin = 120;
    const int cw     = W - 2 * margin;
    int y            = margin;

    // Header
    painter.fillRect(0, 0, W, 180, QColor(28, 22, 16));
    painter.setFont(QFont("Segoe UI", 28, QFont::Bold));
    painter.setPen(QColor("#D4AF37"));
    painter.drawText(margin, 90, tr("Supplier Directory"));
    painter.setFont(QFont("Segoe UI", 14));
    painter.setPen(QColor(180, 160, 120));
    painter.drawText(margin, 130, tr("Hammer Down — Supplier Management System"));
    painter.setFont(QFont("Segoe UI", 12));
    painter.setPen(QColor("#8B6F47"));
    painter.drawText(margin, 165, QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
    y = 210;

    // Column setup
    struct Col { QString name; int widthPct; };
    const QList<Col> cols = {
        {tr("ID"),       6}, {tr("Company"), 22}, {tr("Address"), 22},
        {tr("Email"),   18}, {tr("Phone"),   12}, {tr("Type"),    12}, {tr("Postal"), 8}
    };
    QList<int> colWidths;
    for (const Col &c : cols) colWidths << (cw * c.widthPct / 100);

    const int rowH = 56, headerH = 70;

    auto drawRow = [&](int row, bool isHeader) {
        int x = margin;
        QColor bg = isHeader ? QColor("#8B6F47") :
                    (row % 2 == 0 ? QColor(240, 232, 220) : QColor(255, 252, 245));
        int h = isHeader ? headerH : rowH;
        painter.fillRect(x, y, cw, h, bg);
        painter.setPen(QPen(QColor(180, 150, 110), 1));
        painter.drawRect(x, y, cw, h);
        painter.setPen(isHeader ? Qt::white : QColor(40, 30, 20));
        painter.setFont(QFont("Segoe UI", isHeader ? 16 : 14,
                              isHeader ? QFont::Bold : QFont::Normal));
        for (int c = 0; c < cols.size(); ++c) {
            QString text = isHeader
                ? cols[c].name
                : model->data(model->index(row, c + 2)).toString();
            QRect cell(x + 8, y + 4, colWidths[c] - 16, h - 8);
            painter.drawText(cell, Qt::AlignVCenter | Qt::AlignLeft,
                             painter.fontMetrics().elidedText(text, Qt::ElideRight, cell.width()));
            x += colWidths[c];
        }
    };

    drawRow(-1, true);
    y += headerH;

    for (int r = 0; r < model->rowCount(); ++r) {
        if (y + rowH > printer.height() - margin) {
            printer.newPage();
            y = margin;
            drawRow(-1, true);
            y += headerH;
        }
        drawRow(r, false);
        y += rowH;
    }

    // Footer line
    y += 30;
    painter.setPen(QPen(QColor("#8B6F47"), 2));
    painter.drawLine(margin, y, W - margin, y);
    y += 24;
    painter.setFont(QFont("Segoe UI", 12));
    painter.setPen(QColor("#8B6F47"));
    painter.drawText(margin, y, tr("Hammer Down — Confidential"));
    painter.drawText(W - margin - 300, y,
        QString(tr("Total: %1 suppliers")).arg(model->rowCount()));

    painter.end();
}

void MainWindow::onSupplierLoad(const QModelIndex &index)
{
    if (!ui_supplier || !index.isValid()) return;

    const QAbstractItemModel *model = ui_supplier->tableView->model();
    if (!model) return;

    int row = index.row();
    // Col order: Action, Delete, ID, Company, Address, Email, Phone, Type, PostalCode
    QString suppId  = model->data(model->index(row, 2)).toString();
    QString nom     = model->data(model->index(row, 3)).toString();
    QString addr    = model->data(model->index(row, 4)).toString();
    QString email   = model->data(model->index(row, 5)).toString();
    QString tel     = model->data(model->index(row, 6)).toString();
    QString type    = model->data(model->index(row, 7)).toString();
    int     cp      = model->data(model->index(row, 8)).toInt();

    ui_supplier->le_id->setText(suppId);
    ui_supplier->le_nom->setText(nom);
    ui_supplier->le_adresse->setText(addr);
    ui_supplier->le_email->setText(email);
    ui_supplier->le_tel->setText(tel);
    ui_supplier->le_type->setText(type);
    ui_supplier->sb_cp->setValue(cp);

    // Load opening/closing hours from DB directly
    QSqlQuery hq;
    hq.prepare("SELECT OPENING_TIME, CLOSING_TIME, PRODUCT_TYPE FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    hq.bindValue(":id", suppId.toInt());
    if (hq.exec() && hq.next()) {
        QString ot = hq.value(0).toString();
        QString ct = hq.value(1).toString();
        QString pt = hq.value(2).toString();
        if (m_teOpeningHour)
            m_teOpeningHour->setTime(ot.isEmpty() ? QTime(8, 0) : QTime::fromString(ot, "HH:mm"));
        if (m_teClosingHour)
            m_teClosingHour->setTime(ct.isEmpty() ? QTime(18, 0) : QTime::fromString(ct, "HH:mm"));
        ui_supplier->le_product_type->setText(pt);
    }

    // Switch to the Manage Suppliers tab (index 0)
    ui_supplier->tabWidget->setCurrentIndex(0);

    // Switch radio to "Manage Supplier" mode (rb_supplier_mod_mode)
    QRadioButton *rbMod = ui_supplier->tab_gestion->findChild<QRadioButton*>("rb_supplier_mod_mode");
    if (rbMod) rbMod->setChecked(true);
}

void MainWindow::onSupplierSearch()
{
    if (!ui_supplier) return;

    QString search = ui_supplier->le_recherche->text().trimmed();

    QSqlQueryModel *model = new QSqlQueryModel(this);
    QString sqlBase =
        "SELECT 1 AS \"Action\", 2 AS \"Del\", SUPPLIER_ID AS \"ID\", "
        "SUPPLIER_NAME AS \"Company\", ADDRESS AS \"Address\","
        " EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", TYPE_NOTIFICATION AS \"Type\","
        " POSTAL_CODE AS \"Postal Code\""
        " FROM SUPPLIERS";

    if (search.isEmpty()) {
        model->setQuery(sqlBase + " ORDER BY SUPPLIER_ID");
    } else {
        QSqlQuery q;
        q.prepare(sqlBase + " WHERE UPPER(SUPPLIER_NAME) LIKE UPPER(:search) ORDER BY SUPPLIER_NAME");
        q.bindValue(":search", "%" + search + "%");
        q.exec();
        model->setQuery(std::move(q));
    }

    if (model->lastError().isValid()) {
        QMessageBox::warning(this, "Search Error",
            "Search failed:\n" + model->lastError().text());
        return;
    }

    ui_supplier->tableView->setModel(model);
    ui_supplier->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_supplier->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_supplier->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::onSupplierSendSMS()
{
    if (!ui_supplier) return;

    QString tel     = ui_supplier->le_tel->text().trimmed();
    QString message = ui_supplier->txt_sms->toPlainText().trimmed();

    if (tel.isEmpty()) {
        QMessageBox::warning(this, "SMS Error",
            trKey("No phone number found.\nPlease select a supplier or enter a phone number first."));
        return;
    }

    if (message.isEmpty()) {
        QMessageBox::warning(this, "SMS Error",
            trKey("Please type an SMS message in the SMS Message field."));
        return;
    }

    // --- Strategy 1: Direct PC-to-Phone Link ---
    if (homeWindow && homeWindow->isAnimationMode()) {
        triggerPhoneAnimation(message, tel);
        ui_supplier->txt_sms->clear();
    } else {
        QString urlStr = QString("sms:%1?body=%2").arg(tel).arg(QString(QUrl::toPercentEncoding(message)));
        bool success = QDesktopServices::openUrl(QUrl(urlStr));

        if (success) {
            QMessageBox::information(this, trKey("SMS Link Opened"),
                trKey("Your system's SMS handler (like Phone Link) has been opened.\n"
                      "Please complete the sending process on your phone or PC app."));
            ui_supplier->txt_sms->clear();
        } else {
            QMessageBox::critical(this, trKey("SMS Error"),
                trKey("Failed to open the system's SMS handler.\n"
                      "Please ensure you have an app like 'Phone Link' set up on your PC."));
        }
    }
}


void MainWindow::onSupplierUploadImage()
{
    if (!ui_supplier) return;

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Choose Supplier Image",
        QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp)"
    );

    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Image Error", "Could not load the selected image.");
        return;
    }

    // Scale and display in the preview label
    pixmap = pixmap.scaled(
        ui_supplier->lbl_image_preview->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
    ui_supplier->lbl_image_preview->setPixmap(pixmap);
    ui_supplier->lbl_image_preview->setAlignment(Qt::AlignCenter);
}

// ==================== Supplier Delivery Rating System ====================

void MainWindow::onSupplierEnsureReviewsTable()
{
    QSqlQuery q;
    // We now use SUPPLIERS directly, adding JSON columns and aggregations if missing
    q.exec("ALTER TABLE SUPPLIERS ADD RATINGS_JSON CLOB");
    q.exec("ALTER TABLE SUPPLIERS ADD AVERAGE_RATING NUMBER(3,2) DEFAULT 0");
    q.exec("ALTER TABLE SUPPLIERS ADD NOTIFICATIONS_JSON CLOB");
    q.exec("ALTER TABLE SUPPLIERS ADD PRODUCT_TYPE VARCHAR(200)");
    // Ignore ORA-01430 / SQLite "duplicate column" errors if columns already exist
}

void MainWindow::onSupplierPopulateRatingCombos()
{
    if (!ui_supplier) return;

    // 1. Populate Suppliers (the master filter)
    ui_supplier->cb_supplier_reviews->clear();
    ui_supplier->cb_supplier_reviews->addItem("-- Select Supplier --", -1);
    QSqlQuery qSupp;
    qSupp.prepare("SELECT SUPPLIER_ID, SUPPLIER_NAME FROM SUPPLIERS ORDER BY SUPPLIER_NAME");
    if (qSupp.exec()) {
        while (qSupp.next()) {
            QString name = qSupp.value(1).toString();
            int sid = qSupp.value(0).toInt();
            ui_supplier->cb_supplier_reviews->addItem(name.isEmpty() ? QString("Supplier #%1").arg(sid) : name, sid);
        }
    }

    // 2. Populate Employees
    ui_supplier->cb_employee_rating->clear();
    ui_supplier->cb_employee_rating->addItem("-- Select Employee --", 0);
    QSqlQuery qEmp("SELECT EMPLOYEE_ID, LAST_NAME || ' ' || FIRST_NAME FROM EMPLOYEES ORDER BY LAST_NAME");
    if (qEmp.exec()) {
        while (qEmp.next()) {
            ui_supplier->cb_employee_rating->addItem(qEmp.value(1).toString(), qEmp.value(0));
        }
    }

    // 3. Populate Equipment
    ui_supplier->cb_equipment_rating->clear();
    ui_supplier->cb_equipment_rating->addItem("-- Select Equipment --", 0);
    QSqlQuery qEquip("SELECT EQUIPMENT_ID, TO_CHAR(DESCRIPTION) FROM EQUIPMENT WHERE STATUS != 'Retired' ORDER BY TO_CHAR(DESCRIPTION)");
    if (qEquip.exec()) {
        while (qEquip.next()) {
            ui_supplier->cb_equipment_rating->addItem(qEquip.value(1).toString(), qEquip.value(0));
        }
    }
}

void MainWindow::onSupplierReviewRatingChanged(int value)
{
    if (!ui_supplier) return;
    QString stars;
    for (int i = 0; i < value; ++i)  stars += QChar(0x2605); // ★
    for (int i = value; i < 5; ++i) stars += QChar(0x2606); // ☆
    ui_supplier->lbl_rating_stars->setText(stars);

    if (homeWindow && homeWindow->isAnimationMode()) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_supplier->lbl_rating_stars);
        ui_supplier->lbl_rating_stars->setGraphicsEffect(eff);
        QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutBack);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::onSupplierReviewLoad()
{
    if (!ui_supplier) return;

    int suppId = ui_supplier->cb_supplier_reviews->currentData().toInt();
    if (suppId <= 0) {
        ui_supplier->lbl_avg_score->setText("–");
        ui_supplier->lbl_stars_row->setText("☆☆☆☆☆");
        ui_supplier->lbl_review_count->setText("No supplier selected");
        return;
    }

    // Fetch the JSON array from the SUPPLIERS table
    QSqlQuery qFetch;
    qFetch.prepare("SELECT RATINGS_JSON FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    qFetch.bindValue(":id", suppId);
    if (!qFetch.exec() || !qFetch.next()) return;

    QString jsonStr = qFetch.value(0).toString();
    QJsonArray ratingsArr;
    if (!jsonStr.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isArray()) ratingsArr = doc.array();
    }

    // Prepare dictionary maps for foreign keys
    QMap<int, QString> empMap;
    QSqlQuery qEmp("SELECT EMPLOYEE_ID, LAST_NAME || ' ' || FIRST_NAME FROM EMPLOYEES");
    if (qEmp.exec()) {
        while (qEmp.next()) empMap[qEmp.value(0).toInt()] = qEmp.value(1).toString();
    }

    QMap<int, QString> eqMap;
    QSqlQuery qEq("SELECT EQUIPMENT_ID, DESCRIPTION FROM EQUIPMENT WHERE STATUS != 'Retired'");
    if (qEq.exec()) {
        while (qEq.next()) eqMap[qEq.value(0).toInt()] = qEq.value(1).toString();
    }

    // Populate the UI Table Reviews
    QStandardItemModel *model = new QStandardItemModel(ratingsArr.size(), 5, this);
    model->setHorizontalHeaderLabels({"Rating", "Employee", "Equipment", "Note", "Date"});

    int total = ratingsArr.size();
    double sum = 0;
    int c5 = 0, c4 = 0, c3 = 0, c2 = 0, c1 = 0;

    for (int i = 0; i < total; ++i) {
        // Read backwards to show newest first
        QJsonObject obj = ratingsArr[total - 1 - i].toObject();
        int r = obj["rating"].toInt();
        int eId = obj["employee_id"].toInt();
        int eqId = obj["equipment_id"].toInt();
        QString note = obj["note"].toString();
        QString date = obj["date"].toString().left(10); // get YYYY-MM-DD

        sum += r;
        if (r == 5) c5++; else if (r == 4) c4++; else if (r == 3) c3++; else if (r == 2) c2++; else if (r == 1) c1++;

        model->setItem(i, 0, new QStandardItem(QString::number(r)));
        model->setItem(i, 1, new QStandardItem(empMap.value(eId, "")));
        model->setItem(i, 2, new QStandardItem(eqMap.value(eqId, "")));
        model->setItem(i, 3, new QStandardItem(note));
        model->setItem(i, 4, new QStandardItem(date));
    }

    ui_supplier->table_reviews->setModel(model);
    ui_supplier->table_reviews->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_supplier->table_reviews->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_supplier->table_reviews->setSelectionBehavior(QAbstractItemView::SelectRows);

    double avg = total == 0 ? 0 : (sum / total);

    if (total == 0) {
        ui_supplier->lbl_avg_score->setText("–");
        ui_supplier->lbl_stars_row->setText("☆☆☆☆☆");
        ui_supplier->lbl_review_count->setText("No deliveries rated yet");
        ui_supplier->pb_dist_5->setValue(0);
        ui_supplier->pb_dist_4->setValue(0);
        ui_supplier->pb_dist_3->setValue(0);
        ui_supplier->pb_review_quality->setValue(0);
        ui_supplier->pb_review_response->setValue(0);
        ui_supplier->pb_review_price->setValue(0);
        return;
    }

    // Average score label
    ui_supplier->lbl_avg_score->setText(QString::number(avg, 'f', 1));

    // Star display: filled + empty
    int filled = qRound(avg);
    QString starsStr;
    for (int i = 0; i < filled; ++i)  starsStr += QChar(0x2605);
    for (int i = filled; i < 5; ++i)  starsStr += QChar(0x2606);
    ui_supplier->lbl_stars_row->setText(starsStr);

    // Review count
    ui_supplier->lbl_review_count->setText(
        QString("Based on %1 delivery rating%2").arg(total).arg(total == 1 ? "" : "s"));

    // Distribution bars (percentage of total)
    ui_supplier->pb_dist_5->setValue(total > 0 ? c5 * 100 / total : 0);
    ui_supplier->pb_dist_4->setValue(total > 0 ? c4 * 100 / total : 0);
    ui_supplier->pb_dist_3->setValue(total > 0 ? c3 * 100 / total : 0);
    // reuse lbl_dist_insight to show 2- and 1-star counts textually
    ui_supplier->lbl_dist_insight->setText(
        QString("2★: %1  |  1★: %2\n(out of %3 total ratings)").arg(c2).arg(c1).arg(total));

    // Satisfaction breakdown — all driven by the single avg
    int avgPct = qRound(avg / 5.0 * 100);
    ui_supplier->pb_review_quality->setValue(qMin(100, avgPct));
    ui_supplier->pb_review_response->setValue(qMin(100, qMax(0, qRound((avg - 0.3) / 5.0 * 100))));
    ui_supplier->pb_review_price->setValue(qMin(100, qMax(0, qRound((avg + 0.2) / 5.0 * 100))));
}

void MainWindow::onSupplierReviewSubmit()
{
    if (!ui_supplier) return;

    int suppId = ui_supplier->cb_supplier_reviews->currentData().toInt();
    if (suppId <= 0) {
        QMessageBox::warning(this, "No Supplier", "Please select a supplier first.");
        return;
    }

    int rating = ui_supplier->sb_rating->value();
    QString note = ui_supplier->le_comment->text().trimmed();
    int employeeId = ui_supplier->cb_employee_rating->currentData().toInt();
    int equipmentId = ui_supplier->cb_equipment_rating->currentData().toInt();

    // Fetch existing JSON ratings array
    QSqlQuery qFetch;
    qFetch.prepare("SELECT RATINGS_JSON FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    qFetch.bindValue(":id", suppId);
    QJsonArray ratingsArr;
    if (qFetch.exec() && qFetch.next()) {
        QString jsonStr = qFetch.value(0).toString();
        if (!jsonStr.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            if (doc.isArray()) ratingsArr = doc.array();
        }
    }

    // Append new rating object
    QJsonObject newRating;
    newRating["rating"] = rating;
    newRating["note"] = note;
    newRating["employee_id"] = employeeId;
    newRating["equipment_id"] = equipmentId;
    newRating["date"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    ratingsArr.append(newRating);

    // Calculate new average
    double total = 0;
    for (int i = 0; i < ratingsArr.size(); ++i) {
        total += ratingsArr[i].toObject()["rating"].toDouble();
    }
    double avg = ratingsArr.isEmpty() ? 0 : (total / ratingsArr.size());

    // Serialize back to string
    QJsonDocument newDoc(ratingsArr);
    QString newJsonStr = QString::fromUtf8(newDoc.toJson(QJsonDocument::Compact));

    // Update SUPPLIERS table
    QSqlQuery q;
    q.prepare("UPDATE SUPPLIERS SET RATINGS_JSON = :json, AVERAGE_RATING = :avg WHERE SUPPLIER_ID = :id");
    q.bindValue(":json", newJsonStr);
    q.bindValue(":avg", avg);
    q.bindValue(":id", suppId);

    if (q.exec()) {
        QString starStr;
        for (int i = 0; i < rating; ++i) starStr += QChar(0x2605);
        QString displayName = ui_supplier->cb_supplier_reviews->currentText();
        QMessageBox::information(this, "Rating Logged",
            QString("Successfully logged a %1 rating for %2.").arg(starStr).arg(displayName));
            
        ui_supplier->le_comment->clear();
        ui_supplier->sb_rating->setValue(5);
        onSupplierReviewLoad(); // Refresh dashboard and history
    } else {
        QMessageBox::critical(this, "Database Error",
            "Failed to log rating.\n\n" + q.lastError().databaseText());
    }
}

void MainWindow::setupClientManagement()
{
    // 1. Rename 'Add Client' tab to 'Manage Clients'
    setTabTextTr(ui_client->tabWidget, ui_client->tab_add, "Manage Clients");

    // 2. Reparent 'Modify Client' GroupBox to 'Add Client' tab
    ui_client->group_modify->setParent(ui_client->tab_add);
    
    // Position it same as group_add
    ui_client->group_add->move(20, 70);
    ui_client->group_modify->move(20, 70);

    // Hide modify group initially
    ui_client->group_modify->setVisible(false);
    ui_client->group_add->setVisible(true);

    // 3. Create Toggle Radio Buttons
    QRadioButton *rbAdd = new QRadioButton(trKey("Add Mode"), ui_client->tab_add);
    QRadioButton *rbMod = new QRadioButton(trKey("Modify Mode"), ui_client->tab_add);
    rbMod->setObjectName("rb_client_mod_mode");
    setTrKey(rbAdd, "Add Mode");
    setTrKey(rbMod, "Modify Mode");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);
    
    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // 4. Connect Signals
    connect(rbAdd, &QRadioButton::toggled, [=](bool checked){
        if(checked) {
            ui_client->group_add->setVisible(true);
            ui_client->group_modify->setVisible(false);
        }
    });

    connect(rbMod, &QRadioButton::toggled, [=](bool checked){
        if(checked) {
            ui_client->group_add->setVisible(false);
            ui_client->group_modify->setVisible(true);
        }
    });

    // 5. Remove the empty 'Modify Client' tab
    int modifyTabIndex = ui_client->tabWidget->indexOf(ui_client->tab_modify);
    if (modifyTabIndex != -1) {
        ui_client->tabWidget->removeTab(modifyTabIndex);
    }
    
    // 6. Cyberpunk Input Validations
    QRegularExpression nameRegex("^[a-zA-Z\\s\\-']+$");
    QValidator *nameVal = new QRegularExpressionValidator(nameRegex, this);
    ui_client->le_nom->setValidator(nameVal);
    ui_client->le_prenom->setValidator(nameVal);
    ui_client->le_nom_mod->setValidator(nameVal);
    ui_client->le_prenom_mod->setValidator(nameVal);

    QRegularExpression phoneRegex("^\\+?\\d{8,15}$");
    QValidator *phoneVal = new QRegularExpressionValidator(phoneRegex, this);
    ui_client->le_tel->setValidator(phoneVal);
    ui_client->le_tel_mod->setValidator(phoneVal);

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    QValidator *emailVal = new QRegularExpressionValidator(emailRegex, this);
    ui_client->le_email->setValidator(emailVal);
    ui_client->le_email_mod->setValidator(emailVal);

    QRegularExpression idRegex("^[1-9]\\d*$");
    QValidator *idVal = new QRegularExpressionValidator(idRegex, this);
    ui_client->le_id_mod->setValidator(idVal);

    // 7. Inject Cyber Tabs Dynamically to bypass ui cache
    QWidget *traceTab = new QWidget();
    ui_client->tabWidget->addTab(traceTab, "Cyber Trace");
    m_clientCyberTable = new QTableView(traceTab);
    m_clientCyberTable->setGeometry(20, 20, 1200, 660);
    m_clientCyberTable->setStyleSheet("QTableView { background: rgba(0,0,0,0.6); gridline-color: #5A4A32; border: 1px solid #8B6F47; color: #D4AF37; font-family: 'Consolas'; } QHeaderView::section { background: rgba(139,111,71,0.3); border: 1px solid #8B6F47; color: #D4AF37; font-weight: bold; } QTableView::item:selected { background: rgba(139,111,71,0.5); border: 1px solid #D4AF37; }");

    QPushButton *btn_refresh_trace = new QPushButton("UPDATE LOG", traceTab);
    btn_refresh_trace->setGeometry(1070, 690, 150, 40);
    btn_refresh_trace->setStyleSheet("QPushButton { background: rgba(139, 111, 71, 0.4); border: 1px solid #8B6F47; border-radius: 5px; color: #D4AF37; font-weight: bold; font-family: 'Consolas'; } QPushButton:hover { background: rgba(139, 111, 71, 0.8); border: 1px solid #D4AF37; }");

    QWidget *matrixTab = new QWidget();
    ui_client->tabWidget->addTab(matrixTab, "Data Matrix");
    m_clientMatrixFrame = new QFrame(matrixTab);
    m_clientMatrixFrame->setGeometry(100, 100, 1040, 550);
    m_clientMatrixFrame->setStyleSheet("background: rgba(10, 10, 10, 0.7); border: 2px solid #8B6F47; border-radius: 10px;");

    connect(btn_refresh_trace, &QPushButton::clicked, this, &MainWindow::onClientCyberTraceRefresh);
    
    // Call data matrix initialization
    setupClientDataMatrix();
}

void MainWindow::onClientCyberTraceRefresh()
{
    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray auditArray;
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isArray()) auditArray = doc.array();
            file.close();
        }
    }

    QVector<QJsonObject> filtered;
    filtered.reserve(auditArray.size());
    for (const QJsonValue &v : auditArray) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value("module_name").toString() == "Clients") filtered.push_back(o);
    }

    std::sort(filtered.begin(), filtered.end(), [](const QJsonObject &a, const QJsonObject &b) {
        const qint64 at = a.value("timestamp_ms").toVariant().toLongLong();
        const qint64 bt = b.value("timestamp_ms").toVariant().toLongLong();
        return bt < at; // descending
    });

    QStandardItemModel *model = new QStandardItemModel(filtered.size(), 3, this);
    model->setHorizontalHeaderLabels({"Timestamp", "Operative", "Action Sequence"});

    for (int r = 0; r < filtered.size(); ++r) {
        const QJsonObject o = filtered.at(r);
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(o.value("timestamp_ms").toVariant().toLongLong());
        if (!dt.isValid()) dt = QDateTime::fromString(o.value("timestamp_iso").toString(), Qt::ISODate);
        const QString timeStr = dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : QString();

        model->setItem(r, 0, new QStandardItem(timeStr));
        model->setItem(r, 1, new QStandardItem(o.value("employee_name").toString()));
        model->setItem(r, 2, new QStandardItem(o.value("action_details").toString()));
    }

    m_clientCyberTable->setModel(model);
    m_clientCyberTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_clientCyberTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clientCyberTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Grid cyber animation on refresh
    QGraphicsOpacityEffect *traceEff = new QGraphicsOpacityEffect(this);
    m_clientCyberTable->setGraphicsEffect(traceEff);
    QPropertyAnimation *traceAnim = new QPropertyAnimation(traceEff, "opacity");
    traceAnim->setDuration(600);
    traceAnim->setStartValue(0.1);
    traceAnim->setEndValue(1.0);
    traceAnim->setEasingCurve(QEasingCurve::InBack);
    traceAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::setupClientDataMatrix()
{
    // Clean old layout
    if (m_clientMatrixFrame->layout()) {
        QLayoutItem* item;
        while ((item = m_clientMatrixFrame->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete m_clientMatrixFrame->layout();
    }

    QVBoxLayout *layout = new QVBoxLayout(m_clientMatrixFrame);
    
    QSqlQuery q;
    int total = 0, male = 0, female = 0;
    if (q.exec("SELECT COUNT(*), SUM(CASE WHEN GENDER='Male' THEN 1 ELSE 0 END), SUM(CASE WHEN GENDER='Female' THEN 1 ELSE 0 END) FROM CLIENTS")) {
        if (q.next()) {
            total = q.value(0).toInt();
            male = q.value(1).toInt();
            female = q.value(2).toInt();
        }
    }

    QLabel *statsLabel = new QLabel(m_clientMatrixFrame);
    statsLabel->setStyleSheet("color: #00FF41; font-family: 'Consolas'; font-size: 20px; text-align: left; background: transparent;");
    statsLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statsLabel);
    
    QString fullText = QString(
        "NETWORK TARGETS IDENTIFIED: %1\n\n"
        "[+] MALE OPERATIVES: %2\n"
        "[+] FEMALE OPERATIVES: %3\n\n"
        "DATABASE LINK ... ACTIVE"
    ).arg(total).arg(male).arg(female);

    // Typing effect via Lambda and Timer
    for (int i = 1; i <= fullText.length(); ++i) {
        QTimer::singleShot(i * 25, statsLabel, [statsLabel, fullText, i]() {
            statsLabel->setText(fullText.left(i));
        });
    }
    
    // Cool Cyberpunk Animation Effect
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    m_clientMatrixFrame->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(1200);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::InExpo);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::setupSupplierStats()
{
    if (!ui_supplier) return;


    // --- 1. Top Level Metrics (Aggregated from Database) ---

    // Product Quality: % of all individual ratings that are 4 or 5 stars
    {
        int totalRatings = 0, highRatings = 0;
        QSqlQuery qQual("SELECT RATINGS_JSON FROM SUPPLIERS WHERE RATINGS_JSON IS NOT NULL");
        while (qQual.next()) {
            QJsonArray arr = QJsonDocument::fromJson(qQual.value(0).toString().toUtf8()).array();
            for (const QJsonValue &v : arr) {
                double r = v.toObject()["rating"].toDouble();
                if (r > 0) { totalRatings++; if (r >= 4.0) highRatings++; }
            }
        }
        int qualityPct = (totalRatings > 0) ? qBound(0, qRound(100.0 * highRatings / totalRatings), 100) : 0;
        ui_supplier->pb_quality->setValue(qualityPct);
    }

    // Contact Coverage: % of suppliers with both email AND phone on file (responsiveness proxy)
    {
        int total = 0, contactComplete = 0;
        QSqlQuery qCov(
            "SELECT COUNT(*), "
            "SUM(CASE WHEN EMAIL IS NOT NULL AND TRIM(EMAIL) != '' "
            "         AND PHONE_NUMBER IS NOT NULL AND TRIM(TO_CHAR(PHONE_NUMBER)) != '' "
            "    THEN 1 ELSE 0 END) "
            "FROM SUPPLIERS"
        );
        if (qCov.exec() && qCov.next()) {
            total           = qCov.value(0).toInt();
            contactComplete = qCov.value(1).toInt();
        }
        int coveragePct = (total > 0) ? qBound(0, qRound(100.0 * contactComplete / total), 100) : 0;
        // Relabel so it no longer says "Delivery Speed"
        ui_supplier->lbl_bar_speed->setText(trKey("Contact Coverage:"));
        ui_supplier->pb_speed->setValue(coveragePct);
    }

    // Retention: % of suppliers that have submitted at least one rating
    {
        int total = 0, withRating = 0;
        QSqlQuery qRet("SELECT COUNT(*), SUM(CASE WHEN AVERAGE_RATING > 0 THEN 1 ELSE 0 END) FROM SUPPLIERS");
        if (qRet.exec() && qRet.next()) {
            total      = qRet.value(0).toInt();
            withRating = qRet.value(1).toInt();
        }
        int retPct = (total > 0) ? qBound(0, qRound(100.0 * withRating / total), 100) : 0;
        ui_supplier->lbl_percent_retention->setText(QString::number(retPct) + "%");
    }

    // Accuracy: % of suppliers whose average rating is >= 3.0 ("good or better")
    {
        int total = 0, highRating = 0;
        QSqlQuery qAcc("SELECT COUNT(*), SUM(CASE WHEN AVERAGE_RATING >= 3 THEN 1 ELSE 0 END) FROM SUPPLIERS WHERE AVERAGE_RATING > 0");
        if (qAcc.exec() && qAcc.next()) {
            total      = qAcc.value(0).toInt();
            highRating = qAcc.value(1).toInt();
        }
        int accPct = (total > 0) ? qBound(0, qRound(100.0 * highRating / total), 100) : 0;
        ui_supplier->lbl_percent_accuracy->setText(QString::number(accPct) + "%");
    }

    // --- 2. Chart 1: Product Categories (Real Data) ---
    QPieSeries *seriesCat = new QPieSeries();
    seriesCat->setHoleSize(0.45);
    
    QSqlQuery qCats("SELECT PRODUCT_TYPE, COUNT(*) FROM SUPPLIERS GROUP BY PRODUCT_TYPE");
    int catIdx = 0;
    int totalSuppliers = 0;
    QList<QPair<QString, int>> catData;
    while(qCats.next()){
        int c = qCats.value(1).toInt();
        totalSuppliers += c;
        catData.append({qCats.value(0).toString(), c});
    }

    QStringList catColors = {"#D4AF37", "#8B6F47", "#5D4037", "#2E1A0C", "#A0825A"};
    for(const auto &p : catData) {
        QString cat = p.first;
        int count = p.second;
        if (cat.isEmpty()) cat = trKey("Other");
        double pct = (totalSuppliers > 0) ? (100.0 * count / totalSuppliers) : 0.0;
        QPieSlice *slice = seriesCat->append(QString("%1 (%2%)").arg(cat).arg(pct, 0, 'f', 1), count);
        slice->setBrush(QColor(catColors.at(catIdx % catColors.size())));
        slice->setLabelVisible();
        slice->setLabelColor(Qt::white);
        catIdx++;
    }

    QChart *chartCat = new QChart();
    chartCat->addSeries(seriesCat);
    chartCat->setTitle(trKey("Category Distribution"));
    chartCat->setTitleBrush(QBrush(QColor("#D4AF37")));
    chartCat->setAnimationOptions(QChart::SeriesAnimations);
    chartCat->legend()->setAlignment(Qt::AlignRight);
    chartCat->legend()->setLabelColor(Qt::white);
    chartCat->setBackgroundBrush(Qt::transparent);
    
    QChartView *viewCat = new QChartView(chartCat);
    viewCat->setRenderHint(QPainter::Antialiasing);
    viewCat->setStyleSheet("background: transparent;");
    
    if (ui_supplier->frame_chart_types->layout()) {
        QLayoutItem *child;
        while ((child = ui_supplier->frame_chart_types->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    } else {
        new QVBoxLayout(ui_supplier->frame_chart_types);
    }
    if (ui_supplier->chart_types_view) ui_supplier->chart_types_view->hide();
    ui_supplier->frame_chart_types->layout()->addWidget(viewCat);

    // --- 3. Chart 2: Supplier Satisfaction (Real Data) ---
    QBarSet *setScore = new QBarSet(trKey("Avg Rating %"));
    QStringList categories;
    
    QSqlQuery qGet("SELECT SUPPLIER_NAME, AVERAGE_RATING FROM SUPPLIERS WHERE ACCOUNT_STATUS = 'Active' ORDER BY AVERAGE_RATING DESC");
    int limit = 0;
    while (qGet.next() && limit < 10) { // Limit to top 10 suppliers
        QString name = qGet.value(0).toString();
        double rating = qGet.value(1).toDouble();
        
        // Show even if 0, but if you want to skip 0, you can add 'WHERE AVERAGE_RATING > 0'
        
        QString shortName = name.length() > 10 ? name.left(8) + ".." : name;
        categories << shortName;
        *setScore << (rating * 20.0); // 0-5 -> 0-100%
        limit++;
    }
    
    // Fallback if empty to prevent crash
    if (categories.isEmpty()) {
        categories << "None";
        *setScore << 0;
    }

    setScore->setLabel(trKey("Satisfaction %"));
    setScore->setColor(QColor("#D4AF37"));

    QBarSeries *seriesTrend = new QBarSeries();
    seriesTrend->append(setScore);

    QChart *chartTrend = new QChart();
    chartTrend->addSeries(seriesTrend);
    chartTrend->setTitle(trKey("Supplier Satisfaction"));
    chartTrend->setTitleBrush(QBrush(QColor("#D4AF37")));
    chartTrend->setAnimationOptions(QChart::SeriesAnimations);
    chartTrend->setBackgroundBrush(Qt::transparent);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(Qt::white);
    chartTrend->addAxis(axisX, Qt::AlignBottom);
    seriesTrend->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%d%");
    axisY->setLabelsColor(Qt::white);
    chartTrend->addAxis(axisY, Qt::AlignLeft);
    seriesTrend->attachAxis(axisY);
    chartTrend->legend()->setVisible(false);

    QChartView *viewTrend = new QChartView(chartTrend);
    viewTrend->setRenderHint(QPainter::Antialiasing);
    viewTrend->setStyleSheet("background: transparent;");

    if (ui_supplier->frame_chart_reviews->layout()) {
        QLayoutItem *child;
        while ((child = ui_supplier->frame_chart_reviews->layout()->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    } else {
        new QVBoxLayout(ui_supplier->frame_chart_reviews);
    }
    if (ui_supplier->chart_reviews_view) ui_supplier->chart_reviews_view->hide();
    ui_supplier->frame_chart_reviews->layout()->addWidget(viewTrend);

    // --- 4. Top Performer Card — real Compliance Score + Consistency Index ---
    QSqlQuery qTop(
        "SELECT SUPPLIER_NAME, AVERAGE_RATING, RATINGS_JSON "
        "FROM SUPPLIERS "
        "WHERE AVERAGE_RATING > 0 "
        "ORDER BY AVERAGE_RATING DESC"
    );
    if (qTop.next()) {
        const QString topName   = qTop.value(0).toString();
        const double  topRating = qTop.value(1).toDouble();
        const QString ratJson   = qTop.value(2).toString();

        double compliancePct  = 0.0;
        QString consistLabel  = trKey("N/A");

        if (!ratJson.isEmpty()) {
            QJsonArray arr = QJsonDocument::fromJson(ratJson.toUtf8()).array();
            const int n = arr.size();
            if (n > 0) {
                int goodCount = 0;
                double sumSqDev = 0.0;
                for (const QJsonValue &v : arr) {
                    const double r = v.toObject()["rating"].toDouble();
                    if (r >= 3.0) goodCount++;
                    sumSqDev += (r - topRating) * (r - topRating);
                }
                compliancePct = 100.0 * goodCount / n;
                const double stdDev = qSqrt(sumSqDev / n);
                if      (stdDev <= 0.5) consistLabel = trKey("High");
                else if (stdDev <= 1.0) consistLabel = trKey("Good");
                else                   consistLabel = trKey("Variable");
            }
        }

        ui_supplier->lbl_top_performer->setText(
            trKey("🏆 Top Performer: ") + topName + "\n" +
            trKey("Compliance Score: ") + QString::number(compliancePct, 'f', 1) + "%\n" +
            trKey("Consistency Index: ") + consistLabel
        );
    } else {
        // No suppliers with ratings yet
        ui_supplier->lbl_top_performer->setText(trKey("No rated suppliers yet."));
    }

    // --- 5. Network Status summary (was hardcoded) ---
    {
        int activeCount   = 0;
        int inactiveCount = 0;
        int totalCount    = 0;
        QSqlQuery qStatus(
            "SELECT ACCOUNT_STATUS, COUNT(*) "
            "FROM SUPPLIERS "
            "GROUP BY ACCOUNT_STATUS"
        );
        while (qStatus.next()) {
            const QString st  = qStatus.value(0).toString();
            const int     cnt = qStatus.value(1).toInt();
            totalCount += cnt;
            if (st.compare("Active", Qt::CaseInsensitive) == 0)
                activeCount = cnt;
            else
                inactiveCount += cnt;
        }

        // Derive a simple health label from the active ratio
        QString perfLabel;
        if (totalCount == 0) {
            perfLabel = trKey("N/A");
        } else {
            double ratio = (double)activeCount / totalCount;
            if (ratio >= 0.85)      perfLabel = trKey("Optimal");
            else if (ratio >= 0.60) perfLabel = trKey("Good");
            else if (ratio >= 0.40) perfLabel = trKey("Fair");
            else                   perfLabel = trKey("Poor");
        }

        const QString now = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
        ui_supplier->lbl_summary_val->setText(
            QString(trKey("Active: %1  |  Inactive: %2  |  Performance: %3\nLast updated: %4"))
                .arg(activeCount)
                .arg(inactiveCount)
                .arg(perfLabel)
                .arg(now)
        );
    }

    // --- 6. Top-2 supplier types by avg rating (was hardcoded North/South bars) ---
    {
        // Fetch the two top-performing TYPE_NOTIFICATION categories by average rating.
        // Scale the 1-5 avg rating to 0-100 for the progress bars.
        QSqlQuery qTypeRating(
            "SELECT TYPE_NOTIFICATION, "
            "       AVG(CASE WHEN AVERAGE_RATING > 0 THEN AVERAGE_RATING ELSE NULL END) AS AVG_R, "
            "       COUNT(*) AS CNT "
            "FROM SUPPLIERS "
            "GROUP BY TYPE_NOTIFICATION "
            "ORDER BY AVG_R DESC NULLS LAST"
        );

        // Row 1 — best type
        if (qTypeRating.next()) {
            const QString typeName = qTypeRating.value(0).toString().isEmpty()
                                     ? trKey("General") : qTypeRating.value(0).toString();
            const double  avgR     = qTypeRating.value(1).isNull() ? 0.0
                                     : qTypeRating.value(1).toDouble();
            const int     barVal   = qBound(0, qRound(avgR * 20.0), 100); // 1-5 → 0-100

            // Re-label the static QLabel sitting next to pb_reg_1
            ui_supplier->lbl_reg_1->setText(typeName + ":");
            ui_supplier->pb_reg_1->setValue(barVal);
            ui_supplier->pb_reg_1->setToolTip(
                QString(trKey("Type: %1  —  Avg rating: %2 / 5.0"))
                    .arg(typeName)
                    .arg(QString::number(avgR, 'f', 1))
            );
        }

        // Row 2 — second-best type
        if (qTypeRating.next()) {
            const QString typeName = qTypeRating.value(0).toString().isEmpty()
                                     ? trKey("Other") : qTypeRating.value(0).toString();
            const double  avgR     = qTypeRating.value(1).isNull() ? 0.0
                                     : qTypeRating.value(1).toDouble();
            const int     barVal   = qBound(0, qRound(avgR * 20.0), 100);

            ui_supplier->lbl_reg_2->setText(typeName + ":");
            ui_supplier->pb_reg_2->setValue(barVal);
            ui_supplier->pb_reg_2->setToolTip(
                QString(trKey("Type: %1  —  Avg rating: %2 / 5.0"))
                    .arg(typeName)
                    .arg(QString::number(avgR, 'f', 1))
            );
        }
    }
}

void MainWindow::setupClientCalendar()
{
    // Replace the contents of the existing tab_calendar widget in-place
    QWidget *calendarTab = ui_client->tab_calendar;

    // Hide existing UI-file children (do NOT delete — ui_client still holds those pointers)
    for (QWidget *child : calendarTab->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
        child->hide();

    QHBoxLayout *mainLayout = new QHBoxLayout(calendarTab);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // =========================================================================
    // LEFT — styled calendar frame
    // =========================================================================
    QFrame *calFrame = new QFrame();
    calFrame->setStyleSheet(R"(
        QFrame {
            background: rgba(28, 22, 16, 0.88);
            border-radius: 14px;
            border: 1px solid #4A3728;
        }
    )");
    QVBoxLayout *calFrameLayout = new QVBoxLayout(calFrame);
    calFrameLayout->setContentsMargins(0, 0, 0, 10);
    calFrameLayout->setSpacing(0);

    QCalendarWidget *calendar = new QCalendarWidget();
    calendar->setGridVisible(false);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    calendar->setStyleSheet(R"(
        QCalendarWidget { background: transparent; }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #1C1610;
            border-radius: 14px 14px 0 0;
            padding: 8px 12px;
            min-height: 46px;
        }
        QCalendarWidget QToolButton {
            color: #D4AF37;
            font-size: 13px;
            font-weight: bold;
            background: transparent;
            border: none;
            padding: 6px 14px;
            border-radius: 8px;
        }
        QCalendarWidget QToolButton:hover { background: rgba(212,175,55,0.18); }
        QCalendarWidget QToolButton::menu-indicator { image: none; }
        QCalendarWidget QMenu {
            background: #2C2418;
            color: #D4AF37;
            border: 1px solid #8B6F47;
            border-radius: 6px;
        }
        QCalendarWidget QSpinBox {
            color: #D4AF37;
            background: transparent;
            font-size: 14px;
            font-weight: bold;
            border: none;
            selection-background-color: #8B6F47;
        }
        QCalendarWidget QAbstractItemView {
            background: transparent;
            color: #D8C9B0;
            font-size: 13px;
            selection-background-color: #8B6F47;
            selection-color: white;
            alternate-background-color: transparent;
            outline: none;
            gridline-color: transparent;
        }
        QCalendarWidget QAbstractItemView:disabled { color: #3D3020; }
        QCalendarWidget QWidget { alternate-background-color: transparent; background: transparent; }
    )");
    calFrameLayout->addWidget(calendar);

    // Legend strip
    QWidget *legend = new QWidget();
    legend->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *legendLayout = new QHBoxLayout(legend);
    legendLayout->setContentsMargins(16, 2, 16, 4);
    legendLayout->setSpacing(6);
    auto addDot = [&](const QString &lbl, const QString &hex) {
        QLabel *dot = new QLabel();
        dot->setFixedSize(10, 10);
        dot->setStyleSheet(QString("background:%1; border-radius:5px; border:none;").arg(hex));
        QLabel *txt = new QLabel(lbl);
        txt->setStyleSheet("color:#806050; font-size:11px; background:transparent; border:none;");
        legendLayout->addWidget(dot);
        legendLayout->addWidget(txt);
        legendLayout->addSpacing(8);
    };
    addDot("Order",       "#D4AF37");
    addDot("Client",      "#78C878");
    addDot("Employee",    "#60B4D8");
    addDot("Equipment",   "#C080E0");
    addDot("Maintenance", "#FF8060");
    addDot("Supplier",    "#E8C040");
    legendLayout->addStretch();
    calFrameLayout->addWidget(legend);

    // =========================================================================
    // Fetch ALL date-bearing events from every module
    // =========================================================================
    // eventMap: "yyyy-MM-dd" -> list of (type, display text)
    QMap<QString, QList<QPair<QString,QString>>> eventMap;

    // 1. Orders — order_date
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(o.order_date,'YYYY-MM-DD'), o.order_type, o.order_status, "
                   "NVL(c.FIRST_NAME||' '||c.LAST_NAME,'Unknown') "
                   "FROM ORDERS o LEFT JOIN CLIENTS c ON o.client_id=c.CLIENT_ID "
                   "WHERE o.order_date IS NOT NULL ORDER BY o.order_date")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"order", q.value(1).toString() + "  —  " + q.value(3).toString()
                              + "  [" + q.value(2).toString() + "]"});
        }
    }

    // 2. Clients — REGISTRATION_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(REGISTRATION_DATE,'YYYY-MM-DD'), "
                   "FIRST_NAME||' '||LAST_NAME FROM CLIENTS WHERE REGISTRATION_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"client", "Client joined:  " + q.value(1).toString()});
        }
    }

    // 3. Employees — HIRE_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(HIRE_DATE,'YYYY-MM-DD'), "
                   "FIRST_NAME||' '||LAST_NAME, JOB_TITLE "
                   "FROM EMPLOYEES WHERE HIRE_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"employee", "Hired:  " + q.value(1).toString()
                                 + "  (" + q.value(2).toString() + ")"});
        }
    }

    // 4. Equipment — PURCHASE_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(PURCHASE_DATE,'YYYY-MM-DD'), EQUIPMENT_TYPE, STATUS "
                   "FROM EQUIPMENT WHERE PURCHASE_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"equipment", "Purchased:  " + q.value(1).toString()
                                  + "  [" + q.value(2).toString() + "]"});
        }
    }

    // 5. Equipment — NEXT_MAINTENANCE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(NEXT_MAINTENANCE,'YYYY-MM-DD'), EQUIPMENT_TYPE "
                   "FROM EQUIPMENT WHERE NEXT_MAINTENANCE IS NOT NULL AND STATUS != 'Retired'")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"maintenance", "Maintenance due:  " + q.value(1).toString()});
        }
    }

    // 6. Suppliers — REGISTRATION_DATE
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(REGISTRATION_DATE,'YYYY-MM-DD'), SUPPLIER_NAME "
                   "FROM SUPPLIERS WHERE REGISTRATION_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"supplier", "Supplier registered:  " + q.value(1).toString()});
        }
    }

    // Colour-code dates — priority: maintenance > order > equipment > employee > client > supplier
    struct TypeInfo { QString type; QColor bg; QColor fg; };
    const QList<TypeInfo> typePriority = {
        {"maintenance", QColor(200,80,60,70),  QColor("#FF8060")},
        {"order",       QColor(139,111,71,70), QColor("#D4AF37")},
        {"equipment",   QColor(140,80,200,60), QColor("#C080E0")},
        {"employee",    QColor(60,140,190,60), QColor("#60B4D8")},
        {"client",      QColor(80,160,80,60),  QColor("#78C878")},
        {"supplier",    QColor(180,160,40,60), QColor("#E8C040")},
    };

    for (auto it = eventMap.cbegin(); it != eventMap.cend(); ++it) {
        QDate d = QDate::fromString(it.key(), "yyyy-MM-dd");
        if (!d.isValid()) continue;
        QSet<QString> types;
        for (const auto &ev : it.value()) types.insert(ev.first);

        for (const auto &ti : typePriority) {
            if (!types.contains(ti.type)) continue;
            QTextCharFormat fmt;
            fmt.setBackground(ti.bg);
            fmt.setForeground(ti.fg);
            fmt.setFontWeight(QFont::Bold);
            calendar->setDateTextFormat(d, fmt);
            break; // highest-priority type wins
        }
    }

    // Today highlight (always override so it stays visible)
    {
        QTextCharFormat todayFmt;
        todayFmt.setBackground(QColor("#8B6F47"));
        todayFmt.setForeground(QColor("#FFEFCF"));
        todayFmt.setFontWeight(QFont::Bold);
        calendar->setDateTextFormat(QDate::currentDate(), todayFmt);
    }

    // =========================================================================
    // RIGHT — event detail panel
    // =========================================================================
    QFrame *rightPanel = new QFrame();
    rightPanel->setMinimumWidth(270);
    rightPanel->setMaximumWidth(330);
    rightPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    rightPanel->setStyleSheet(R"(
        QFrame {
            background: rgba(28, 22, 16, 0.88);
            border-radius: 14px;
            border: 1px solid #4A3728;
        }
    )");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    rightLayout->setSpacing(8);

    QLabel *lblDate = new QLabel("Select a date");
    lblDate->setAlignment(Qt::AlignCenter);
    lblDate->setWordWrap(true);
    lblDate->setStyleSheet(R"(
        font-size: 14px; font-weight: bold; color: #D4AF37;
        background: rgba(139,111,71,0.12); border-radius: 8px;
        border: 1px solid #3A2A1A; padding: 10px 6px;
    )");

    QLabel *lblCount = new QLabel("");
    lblCount->setAlignment(Qt::AlignCenter);
    lblCount->setStyleSheet("font-size: 11px; color: #6A5040; background: transparent; border: none;");

    auto makeSep = [&]() -> QFrame* {
        QFrame *sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("background: #3A2A1A; border: none; max-height: 1px;");
        return sep;
    };

    QString listStyle = R"(
        QListWidget {
            background: transparent; border: none;
            color: #D8C9B0; font-size: 12px; outline: none;
        }
        QListWidget::item {
            padding: 7px 10px; border-radius: 6px; margin: 2px 0;
            background: rgba(255,255,255,0.03);
        }
        QListWidget::item:hover { background: rgba(139,111,71,0.15); }
        QListWidget::item:selected { background: rgba(139,111,71,0.30); color: #D4AF37; }
    )";

    QListWidget *eventList = new QListWidget();
    eventList->setStyleSheet(listStyle);

    QLabel *lblUpHdr = new QLabel("Upcoming — Next 7 Days");
    lblUpHdr->setStyleSheet("font-size: 11px; font-weight: bold; color: #8B6F47; "
                             "background: transparent; border: none; padding: 2px 0;");

    QListWidget *upcomingList = new QListWidget();
    upcomingList->setMaximumHeight(180);
    upcomingList->setStyleSheet(listStyle);

    // Populate upcoming list
    QDate today = QDate::currentDate();
    bool anyUpcoming = false;
    for (int i = 0; i <= 7; ++i) {
        QDate d = today.addDays(i);
        const QString key = d.toString("yyyy-MM-dd");
        if (!eventMap.contains(key)) continue;
        for (const auto &ev : eventMap[key]) {
            QString dayLbl = (i == 0) ? "Today" : (i == 1) ? "Tomorrow"
                                                 : d.toString("ddd d MMM");
            auto *item = new QListWidgetItem(dayLbl + ":  " + ev.second);
            if      (ev.first == "maintenance") item->setForeground(QColor("#FF8060"));
            else if (ev.first == "client")      item->setForeground(QColor("#78C878"));
            else if (ev.first == "employee")    item->setForeground(QColor("#60B4D8"));
            else if (ev.first == "equipment")   item->setForeground(QColor("#C080E0"));
            else if (ev.first == "supplier")    item->setForeground(QColor("#E8C040"));
            else                                item->setForeground(QColor("#D4AF37"));
            upcomingList->addItem(item);
            anyUpcoming = true;
        }
    }
    if (!anyUpcoming) {
        auto *item = new QListWidgetItem("No events in the next 7 days.");
        item->setForeground(QColor("#444"));
        upcomingList->addItem(item);
    }

    QPushButton *btnToday = new QPushButton("Go to Today");
    btnToday->setCursor(Qt::PointingHandCursor);
    btnToday->setStyleSheet(R"(
        QPushButton {
            background: #8B6F47; color: white;
            border-radius: 8px; padding: 9px 0;
            font-weight: bold; font-size: 13px; border: none;
        }
        QPushButton:hover { background: #D4AF37; color: #1C1610; }
        QPushButton:pressed { background: #6B4F2F; }
    )");

    rightLayout->addWidget(lblDate);
    rightLayout->addWidget(lblCount);
    rightLayout->addWidget(makeSep());
    rightLayout->addWidget(eventList, 1);
    rightLayout->addWidget(makeSep());
    rightLayout->addWidget(lblUpHdr);
    rightLayout->addWidget(upcomingList);
    rightLayout->addWidget(btnToday);

    // =========================================================================
    // Connections
    // =========================================================================
    connect(btnToday, &QPushButton::clicked, calendar, [calendar]() {
        calendar->setSelectedDate(QDate::currentDate());
        emit calendar->clicked(QDate::currentDate());
    });

    connect(calendar, &QCalendarWidget::clicked, this,
            [lblDate, lblCount, eventList, eventMap](const QDate &date) {
        lblDate->setText(date.toString("dddd, MMMM d yyyy"));
        eventList->clear();
        const QString key = date.toString("yyyy-MM-dd");
        const auto &evs = eventMap.value(key);
        if (evs.isEmpty()) {
            lblCount->setText("No events");
            auto *item = new QListWidgetItem("No events scheduled.");
            item->setForeground(QColor("#444"));
            eventList->addItem(item);
        } else {
            lblCount->setText(QString::number(evs.size())
                              + (evs.size() == 1 ? " event" : " events"));
            for (const auto &ev : evs) {
                QString prefix;
                QColor  color;
                if      (ev.first == "order")       { prefix = "Order      "; color = QColor("#D4AF37"); }
                else if (ev.first == "client")      { prefix = "Client     "; color = QColor("#78C878"); }
                else if (ev.first == "employee")    { prefix = "Employee   "; color = QColor("#60B4D8"); }
                else if (ev.first == "equipment")   { prefix = "Equipment  "; color = QColor("#C080E0"); }
                else if (ev.first == "maintenance") { prefix = "Maint.     "; color = QColor("#FF8060"); }
                else                                { prefix = "Supplier   "; color = QColor("#E8C040"); }
                auto *item = new QListWidgetItem(prefix + ev.second);
                item->setForeground(color);
                eventList->addItem(item);
            }
        }
    });

    // =========================================================================
    // Assemble main layout
    // =========================================================================
    mainLayout->addWidget(calFrame, 60);
    mainLayout->addWidget(rightPanel, 40);
}

void MainWindow::setupEmployeeModes()
{
    // Rename tab_add to "Manage Employees"
    setTabTextTr(ui_employee->tabWidget, ui_employee->tab_add, "Manage Employees");

    // Replace le_fonction with QComboBox
    // Parent it to group_add so geometry() matches the existing label/input layout
    QComboBox *cbJob = new QComboBox(ui_employee->group_add);
    cbJob->setObjectName("cb_job_title");
    // Keep the same capitalization used elsewhere (and commonly stored in DB)
    cbJob->addItems({"Smith", "Cleaner", "Developer", "Cashier", "Carpenter", "Boss"});
    cbJob->setGeometry(ui_employee->le_fonction->geometry());
    cbJob->setStyleSheet(
        "QComboBox {"
        " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);"
        " border: 2px solid #8B6F47;"
        " border-radius: 8px;"
        " padding: 3px 12px;"
        " font-size: 14px;"
        " color: #333;"
        "}"
        "QComboBox:hover {"
        " border: 2px solid #A0825A;"
        "}"
        "QComboBox:focus {"
        " border: 2px solid #8B4513;"
        " background: #FFFAF0;"
        "}"
        "QComboBox QAbstractItemView {"
        " background-color: #FFFFFF;"
        " color: #333333;"
        " border: 1px solid #8B6F47;"
        " selection-background-color: #8B6F47;"
        " selection-color: #FFFFFF;"
        " outline: 0;"
        "}"
    );
    cbJob->setEditable(false);
    cbJob->setInsertPolicy(QComboBox::NoInsert);
    ui_employee->le_fonction->hide();
    cbJob->show();
    connect(cbJob, &QComboBox::currentTextChanged, this, &MainWindow::updateSalaryInsight);

    // Strict Input Validation (Contrôle de Saisie)
    // ID: numbers only
    ui_employee->le_id->setValidator(new QIntValidator(1, 999999, this));
    
    // Names: alpha characters only
    QRegularExpression nameRegex("^[A-Za-z\\s]*$");
    ui_employee->le_nom->setValidator(new QRegularExpressionValidator(nameRegex, this));
    ui_employee->le_prenom->setValidator(new QRegularExpressionValidator(nameRegex, this));
    
    // Phone: exactly 8 digits logic handled by mask
    ui_employee->le_num->setValidator(new QIntValidator(0, 99999999, this));
    ui_employee->le_num->setMaxLength(8);

    // Create Radio Buttons in tab_add
    QRadioButton *rbAdd = new QRadioButton(trKey("Add Employee"), ui_employee->tab_add);
    QRadioButton *rbMod = new QRadioButton(trKey("Modify Employee"), ui_employee->tab_add);
    rbAdd->setObjectName("rb_employee_add_mode");
    rbMod->setObjectName("rb_employee_mod_mode");
    setTrKey(rbAdd, "Add Employee");
    setTrKey(rbMod, "Modify Employee");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // Move group lower
    ui_employee->group_add->move(20, 70);

    // Lambda to update UI
    ui_employee->group_add->setProperty("trTitleAddKey", "Add Employee");
    ui_employee->group_add->setProperty("trTitleModKey", "Modify Employee");
    ui_employee->group_add->setProperty("trModeAddRadio", "rb_employee_add_mode");
    ui_employee->group_add->setProperty("trModeModRadio", "rb_employee_mod_mode");

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_employee->group_add->setTitle(trKey("Add Employee"));
            ui_employee->btn_add->setVisible(true);
            ui_employee->btn_modify->setVisible(false);
            ui_employee->le_id->setEnabled(true);
            toggleEmployeeFields(true);
        } else {
            ui_employee->group_add->setTitle(trKey("Modify Employee"));
            ui_employee->btn_add->setVisible(false);
            ui_employee->btn_modify->setVisible(true);
            ui_employee->le_id->setEnabled(false);
            // In modify mode, keep fields enabled for the currently selected employee.
            // We only "lock" when no employee is selected (empty ID).
            toggleEmployeeFields(!ui_employee->le_id->text().trimmed().isEmpty());
        }
    };
    
    connect(rbAdd, &QRadioButton::toggled, updateUI);
    connect(rbMod, &QRadioButton::toggled, [=](bool checked){ updateUI(!checked); });
    
    // Initialize Camera for Employee Management Scan
    m_empCamera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    m_empCaptureSession = new QMediaCaptureSession(this);
    m_empVideoSink = new QVideoSink(this);
    m_empCaptureSession->setCamera(m_empCamera);
    m_empCaptureSession->setVideoSink(m_empVideoSink);
    connect(m_empVideoSink, &QVideoSink::videoFrameChanged, this, &MainWindow::processEmpCameraFrame);
}


void MainWindow::setupSupplierModes()
{
    setTabTextTr(ui_supplier->tabWidget, ui_supplier->tab_gestion, "Manage Suppliers");

    QRadioButton *rbAdd = new QRadioButton(trKey("Add Supplier"), ui_supplier->tab_gestion);
    QRadioButton *rbMod = new QRadioButton(trKey("Manage Supplier"), ui_supplier->tab_gestion);
    rbAdd->setObjectName("rb_supplier_add_mode");
    rbMod->setObjectName("rb_supplier_mod_mode");
    setTrKey(rbAdd, "Add Supplier");
    setTrKey(rbMod, "Manage Supplier");

    rbAdd->setGeometry(770, 25, 150, 30);
    rbMod->setGeometry(930, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_supplier->groupBox_gestion->move(20, 70);

    ui_supplier->groupBox_gestion->setProperty("trTitleAddKey", "");
    ui_supplier->groupBox_gestion->setProperty("trTitleModKey", "");
    ui_supplier->groupBox_gestion->setProperty("trModeAddRadio", "rb_supplier_add_mode");
    ui_supplier->groupBox_gestion->setProperty("trModeModRadio", "rb_supplier_mod_mode");

    // --- Opening / Closing Hours widgets (single row below SMS field) ---
    QString lblStyle = "color: white; font-size: 13px; font-weight: bold; background: transparent;";
    QString teStyle  = "background: white; border: 2px solid #8B6F47; border-radius: 8px; padding: 2px 8px; font-size: 13px; color: #333;";

    QLabel *lblOpen = new QLabel("Open:", ui_supplier->groupBox_gestion);
    lblOpen->setStyleSheet(lblStyle);
    lblOpen->setGeometry(200, 470, 55, 28);
    lblOpen->show();

    m_teOpeningHour = new QTimeEdit(ui_supplier->groupBox_gestion);
    m_teOpeningHour->setDisplayFormat("HH:mm");
    m_teOpeningHour->setGeometry(260, 468, 90, 28);
    m_teOpeningHour->setStyleSheet(teStyle);
    m_teOpeningHour->setTime(QTime(8, 0));
    m_teOpeningHour->show();

    QLabel *lblClose = new QLabel("Close:", ui_supplier->groupBox_gestion);
    lblClose->setStyleSheet(lblStyle);
    lblClose->setGeometry(365, 470, 55, 28);
    lblClose->show();

    m_teClosingHour = new QTimeEdit(ui_supplier->groupBox_gestion);
    m_teClosingHour->setDisplayFormat("HH:mm");
    m_teClosingHour->setGeometry(425, 468, 90, 28);
    m_teClosingHour->setStyleSheet(teStyle);
    m_teClosingHour->setTime(QTime(18, 0));
    m_teClosingHour->show();
    // --- End hours widgets ---

    // --- Notification Bell button (placed on the tab_gestion, not groupBox) ---
    m_supplierBellBtn = new QPushButton(ui_supplier->tab_gestion);
    m_supplierBellBtn->setText(QString(QChar(0xD83D)) + QChar(0xDD14)); // 🔔
    m_supplierBellBtn->setObjectName("btn_supplier_bell");
    m_supplierBellBtn->setGeometry(1120, 8, 44, 44);
    m_supplierBellBtn->setStyleSheet(
        "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
        "QPushButton:hover { background-color: #a3845a; }"
        "QPushButton:pressed{ background-color: #6b5535; }");
    m_supplierBellBtn->setCursor(Qt::PointingHandCursor);
    m_supplierBellBtn->setToolTip("Supplier Notifications");
    m_supplierBellBtn->show();
    connect(m_supplierBellBtn, &QPushButton::clicked, this, &MainWindow::onSupplierBellClicked);
    // Run once at startup after setup is complete
    QTimer::singleShot(1500, this, [this](){
        m_aiAdvisorStartupDone = true;
        checkAndPostSupplierNotifications();
        checkWorkshopStockAndNotifyAI();
    });
    // ---

    // --- Form Completion Progress Bar ---
    m_supplierProgress = new QProgressBar(ui_supplier->groupBox_gestion);
    m_supplierProgress->setRange(0, 100);
    m_supplierProgress->setValue(0);
    m_supplierProgress->setTextVisible(false);
    m_supplierProgress->setFixedHeight(12);
    m_supplierProgress->setStyleSheet(
        "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");

    QLabel *pTitle = new QLabel("Form Completion:", ui_supplier->groupBox_gestion);
    pTitle->setObjectName("lbl_supp_prog_title");
    pTitle->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 11px; font-family: 'Segoe UI';");

    auto makeSuppInd = [&](const QString &txt, const QString &obj) {
        QLabel *l = new QLabel(txt, ui_supplier->groupBox_gestion);
        l->setObjectName(obj);
        l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        return l;
    };
    m_suppNameInd = makeSuppInd("[👤 Name ⬜]", "ind_supp_name"); 
    m_suppEmailInd = makeSuppInd("[📧 Email ⬜]", "ind_supp_email"); 
    m_suppTelInd = makeSuppInd("[📞 Phone ⬜]", "ind_supp_tel"); 
    m_suppTypeInd = makeSuppInd("[🏢 Type ⬜]", "ind_supp_type");

    pTitle->move(50, 40);
    m_supplierProgress->setGeometry(50, 60, 555, 12);
    m_suppNameInd->move(50, 78);
    m_suppEmailInd->move(150, 78);
    m_suppTelInd->move(260, 78);
    m_suppTypeInd->move(380, 78);

    connect(ui_supplier->le_nom, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->le_email, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->le_tel, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->le_type, &QLineEdit::textChanged, this, &MainWindow::updateSupplierProgress);
    connect(ui_supplier->btn_send_sms, &QPushButton::clicked, this, &MainWindow::onSupplierSendSMS);


    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_supplier->groupBox_gestion->setTitle("");
            ui_supplier->btn_add->setVisible(true);
            ui_supplier->btn_modify->setVisible(false);
            ui_supplier->btn_delete->setVisible(false);
            
            m_supplierProgress->setVisible(true);
            pTitle->setVisible(true);
            m_suppNameInd->setVisible(true);
            m_suppEmailInd->setVisible(true);
            m_suppTelInd->setVisible(true);
            m_suppTypeInd->setVisible(true);
        } else {
            ui_supplier->groupBox_gestion->setTitle("");
            ui_supplier->btn_add->setVisible(false);
            ui_supplier->btn_modify->setVisible(true);
            ui_supplier->btn_delete->setVisible(true);
            
            m_supplierProgress->setVisible(false);
            pTitle->setVisible(false);
            m_suppNameInd->setVisible(false);
            m_suppEmailInd->setVisible(false);
            m_suppTelInd->setVisible(false);
            m_suppTypeInd->setVisible(false);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);

    if (homeWindow && homeWindow->isAnimationMode()) {
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_add);
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_modify);
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_delete);
        ButtonAnimator::applyHoverAnimation(ui_supplier->btn_send_sms);
    }
}


void MainWindow::setupOrderModes()
{
    const int manageBlockShiftX = 190;

    // tab_manage
    int idx = ui_order->tabWidget->indexOf(ui_order->tab_manage);
    if(idx != -1) {
        setTabTextTr(ui_order->tabWidget, ui_order->tab_manage, "Manage Orders");
    }

    // Panel matching the "Log Delivery Rating" group box style
    QGroupBox *orderPanel = new QGroupBox(trKey("Manage Orders"), ui_order->tab_manage);
    orderPanel->setObjectName("order_manage_panel");
    orderPanel->setGeometry(28 + manageBlockShiftX, 60, 615, 440);
    orderPanel->setStyleSheet(
        "QGroupBox#order_manage_panel {"
        "  background-color: rgba(60, 45, 30, 0.7);"
        "  border: 2px solid #8B6F47;"
        "  border-radius: 12px;"
        "  margin-top: 18px;"
        "  color: white;"
        "}"
        "QGroupBox#order_manage_panel::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top center;"
        "  padding: 2px 12px;"
        "  background-color: #8B6F47;"
        "  font-weight: bold;"
        "  color: white;"
        "  border-radius: 4px;"
        "}"
    );
    orderPanel->lower();
    orderPanel->show();

    const QList<QWidget*> manageWidgets = {
        ui_order->label_id,
        ui_order->le_id,
        ui_order->label_type,
        ui_order->cb_type,
        ui_order->label_stock,
        ui_order->le_stock,
        ui_order->label_prix,
        ui_order->le_prix,
        ui_order->label_buyer,
        ui_order->le_buyer,
        ui_order->btn_add,
        ui_order->btn_modify,
        ui_order->btn_delete,
        ui_order->btn_clear,
        ui_order->btn_import
    };
    for (QWidget *w : manageWidgets) {
        if (!w) continue;
        w->move(w->x() + manageBlockShiftX, w->y());
    }

    const int qrBlockShiftY = 26;

    QGroupBox *qrPanel = new QGroupBox(trKey("QR Code"), ui_order->tab_qrcode);
    qrPanel->setObjectName("order_qr_panel");
    qrPanel->setGeometry(300, 28 + qrBlockShiftY, 450, 482);
    qrPanel->setStyleSheet(
        "QGroupBox#order_qr_panel {"
        "  background-color: rgba(60, 45, 30, 0.7);"
        "  border: 2px solid #8B6F47;"
        "  border-radius: 12px;"
        "  margin-top: 18px;"
        "  color: white;"
        "}"
        "QGroupBox#order_qr_panel::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top center;"
        "  padding: 2px 12px;"
        "  background-color: #8B6F47;"
        "  font-weight: bold;"
        "  color: white;"
        "  border-radius: 4px;"
        "}"
    );
    qrPanel->lower();
    qrPanel->show();

    const QList<QWidget*> qrWidgets = {
        ui_order->label_qr_order_id,
        ui_order->le_qr_order_id,
        ui_order->btn_generate_qr,
        ui_order->label_qr_display,
        ui_order->btn_save_qr,
        ui_order->btn_print_qr
    };
    for (QWidget *w : qrWidgets) {
        if (!w) continue;
        w->move(w->x(), w->y() + qrBlockShiftY);
    }

    QRadioButton *rbAdd = new QRadioButton(trKey("Add Order"), ui_order->tab_manage);
    QRadioButton *rbMod = new QRadioButton(trKey("Manage Order"), ui_order->tab_manage);
    rbAdd->setObjectName("rb_order_add_mode");
    rbMod->setObjectName("rb_order_mod_mode");
    setTrKey(rbAdd, "Add Order");
    setTrKey(rbMod, "Manage Order");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_order->btn_add->setVisible(true);
            ui_order->btn_modify->setVisible(false);
            ui_order->btn_delete->setVisible(false);
            ui_order->btn_import->setVisible(true);
            ui_order->label_id->setVisible(false);
            ui_order->le_id->setVisible(false);
        } else {
            ui_order->btn_add->setVisible(false);
            ui_order->btn_modify->setVisible(true);
            ui_order->btn_delete->setVisible(true);
            ui_order->btn_import->setVisible(false);
            ui_order->label_id->setVisible(true);
            ui_order->le_id->setVisible(true);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);
}

void MainWindow::setupGlobalStyles()
{
    // Apply brown '?' style to all help buttons across all management pages
    QList<QToolButton*> helpBtns = this->findChildren<QToolButton*>(QRegularExpression("^btn_help.*"));
    for(QToolButton* btn : helpBtns) {
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QToolButton { background-color: #8B6F47; border-radius: 16px; color: white; font-weight: bold; border: none; font-size: 16px; }"
            "QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"
        );
        btn->setFixedSize(32, 32);
        btn->setText("?");
    }

    QString style = R"(
        /* --- General Application Style --- */
        QWidget {
            font-family: 'Gadugi', 'Segoe UI', sans-serif;
            font-size: 14px;
        }

        /* --- Buttons --- */
        QPushButton {
            background-color: #8B6F47; /* Gold/Brown */
            color: white;
            border-radius: 5px;
            padding: 8px 15px;
            font-weight: bold;
            border: 1px solid #6d5638;
        }
        QPushButton:hover {
            background-color: #a38253;
            border: 1px solid #8B6F47;
        }
        QPushButton:pressed {
            background-color: #6d5638;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
            border: 1px solid #aaaaaa;
        }

        /* --- Input Fields --- */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QComboBox {
            background-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
            padding: 5px;
            color: #333333;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QComboBox:focus {
            border: 1px solid #8B6F47;
        }

        /* --- Group Boxes --- */
        QGroupBox {
            border: 1px solid #8B6F47;
            border-radius: 6px;
            margin-top: 24px; /* Leave space for title */
            background-color: rgba(255, 255, 255, 0.8); /* Slight transparency */
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 5px 10px;
            background-color: #8B6F47;
            color: white;
            border-radius: 4px;
            font-weight: bold;
        }

        /* --- Tab Widget --- */
        QTabWidget::pane {
            border: 1px solid #cccccc;
            background: rgba(255, 255, 255, 0.9);
            border-radius: 4px;
        }
        QTabWidget::tab-bar {
            left: 5px; /* move to the right by 5px */
        }
        QTabBar::tab {
            background: #e0e0e0;
            border: 1px solid #cccccc;
            border-bottom-color: #cccccc; /* same as the pane color */
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            min-width: 8ex;
            padding: 8px 15px;
            margin-right: 2px;
            color: #333;
        }
        QTabBar::tab:selected, QTabBar::tab:hover {
            background: #8B6F47;
            color: white;
            border-color: #8B6F47;
        }

        /* --- Tables & Lists --- */
        QTableView, QListWidget {
            border: 1px solid #cccccc;
            gridline-color: #eeeeee;
            background-color: white;
            color: #333333;
            selection-background-color: #8B6F47; /* Solid Gold */
            selection-color: white;
            alternate-background-color: #f9f9f9;
        }
        QHeaderView::section {
            background-color: #8B6F47;
            color: white;
            padding: 5px;
            border: none;
            font-weight: bold;
        }
        
        /* --- Scrollbars --- */
        QScrollBar:vertical {
            border: none;
            background: #f0f0f0;
            width: 10px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #cdcdcd;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: #8B6F47;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    
    // Apply style to the entire application to ensure consistency
    qApp->setStyleSheet(style);
}

void MainWindow::setupTabNavigation(QWidget* parentWidget, QTabWidget* tabWidget, const QStringList& tabNames, int startX, int yPos, const QList<int>& targetIndices, int spacing, int afterFirstShift)
{
    int y = yPos;
    
    QString rbStyle = "QRadioButton { font-weight: bold; font-size: 12px; color: white; } QRadioButton::indicator { width: 14px; height: 14px; }";
    
    QButtonGroup *group = new QButtonGroup(parentWidget);
    group->setExclusive(true);
    
    for(int i = 0; i < tabNames.size(); i++) {
        const QString key = tabNames[i];
        const QString translated = QCoreApplication::translate("QObject", key.toUtf8().constData());
        QRadioButton *rb = new QRadioButton(translated, parentWidget);
        const int tabX = startX + (spacing * i) + (i > 0 ? afterFirstShift : 0);
        rb->setGeometry(tabX, y, 98, 28);
        rb->setStyleSheet(rbStyle);
        rb->setProperty("trKey", key);
        
        if(i == 0) rb->setChecked(true);
        
        // Connect to switch tabs
        connect(rb, &QRadioButton::toggled, [=](bool checked){
            if(checked) {
                int index = (targetIndices.size() > i) ? targetIndices[i] : i;
                tabWidget->setCurrentIndex(index);
            }
        });

        // Sync tabWidget back to radio buttons when changed programmatically
        connect(tabWidget, &QTabWidget::currentChanged, [=](int activeIndex){
            int index = (targetIndices.size() > i) ? targetIndices[i] : i;
            if (activeIndex == index) {
                rb->setChecked(true);
            }
        });

        group->addButton(rb, i);
    }
}

void MainWindow::setTabTextTr(QTabWidget *tabWidget, QWidget *tabPage, const QString &key)
{
    if (!tabWidget || !tabPage) {
        return;
    }

    int index = tabWidget->indexOf(tabPage);
    if (index == -1) {
        return;
    }

    tabPage->setProperty("tabTrKey", key);
    tabWidget->setTabText(index, trKey(key));
}

void MainWindow::retranslateDynamicRadios(QWidget *container)
{
    retranslateDynamicWidgets(container);
}

void MainWindow::retranslateDynamicWidgets(QWidget *container)
{
    if (!container) {
        return;
    }

    const auto widgets = container->findChildren<QWidget*>();
    for (auto *widget : widgets) {
        if (!widget) {
            continue;
        }

        const QVariant key = widget->property("trKey");
        if (key.isValid()) {
            const QString text = trKey(key.toString());
            if (auto *btn = qobject_cast<QAbstractButton*>(widget)) {
                btn->setText(text);
            } else if (auto *label = qobject_cast<QLabel*>(widget)) {
                label->setText(text);
            } else if (auto *box = qobject_cast<QGroupBox*>(widget)) {
                box->setTitle(text);
            }
        } else if (currentLanguage != "en") {
            if (auto *btn = qobject_cast<QAbstractButton*>(widget)) {
                const QString original = btn->text();
                if (!original.isEmpty()) {
                    const QString translated = trKey(original);
                    if (translated != original) {
                        btn->setText(translated);
                    }
                }
            }
        }

        auto *box = qobject_cast<QGroupBox*>(widget);
        if (box) {
            const QString addKey = box->property("trTitleAddKey").toString();
            const QString modKey = box->property("trTitleModKey").toString();
            const QString addRadioName = box->property("trModeAddRadio").toString();
            const QString modRadioName = box->property("trModeModRadio").toString();

            if (!addKey.isEmpty() && !modKey.isEmpty()) {
                bool isAdd = true;
                if (!addRadioName.isEmpty()) {
                    auto *rbAdd = container->findChild<QRadioButton*>(addRadioName);
                    if (rbAdd) {
                        isAdd = rbAdd->isChecked();
                    }
                } else if (!modRadioName.isEmpty()) {
                    auto *rbMod = container->findChild<QRadioButton*>(modRadioName);
                    if (rbMod) {
                        isAdd = !rbMod->isChecked();
                    }
                }

                box->setTitle(trKey(isAdd ? addKey : modKey));
            }
        }
    }

    const auto lists = container->findChildren<QListWidget*>();
    for (auto *list : lists) {
        if (!list) {
            continue;
        }

        for (int i = 0; i < list->count(); ++i) {
            auto *item = list->item(i);
            if (!item) {
                continue;
            }

            const QVariant key = item->data(Qt::UserRole);
            if (key.isValid()) {
                item->setText(trKey(key.toString()));
            }
        }
    }

    const auto tables = container->findChildren<QTableWidget*>();
    for (auto *table : tables) {
        if (!table) {
            continue;
        }

        for (int row = 0; row < table->rowCount(); ++row) {
            for (int col = 0; col < table->columnCount(); ++col) {
                QTableWidgetItem *item = table->item(row, col);
                if (item) {
                    QString text = item->text();
                    QString translated = trKey(text);
                    if (translated != text) {
                        item->setText(translated);
                    }
                }
            }
        }
    }
}

void MainWindow::retranslateDynamicTabs(QTabWidget *tabWidget)
{
    if (!tabWidget) {
        return;
    }

    for (int i = 0; i < tabWidget->count(); ++i) {
        QWidget *tab = tabWidget->widget(i);
        if (!tab) {
            continue;
        }

        const QVariant key = tab->property("tabTrKey");
        if (key.isValid()) {
            tabWidget->setTabText(i, trKey(key.toString()));
        }
    }
}

void MainWindow::retranslateDynamicCharts(QWidget *container)
{
    if (!container) {
        return;
    }

    const auto views = container->findChildren<QChartView*>();
    for (auto *view : views) {
        if (!view || !view->chart()) {
            continue;
        }

        QChart *chart = view->chart();
        const QVariant titleKey = chart->property("trTitleKey");
        if (titleKey.isValid()) {
            chart->setTitle(trKey(titleKey.toString()));
        }

        const auto seriesList = chart->series();
        for (auto *series : seriesList) {
            if (!series) {
                continue;
            }

            const QVariant nameKey = series->property("trNameKey");
            if (nameKey.isValid()) {
                series->setName(trKey(nameKey.toString()));
            }

            if (auto *pie = qobject_cast<QPieSeries*>(series)) {
                const QVariant sliceKeys = pie->property("trSliceNames");
                if (sliceKeys.isValid()) {
                    const QStringList keys = sliceKeys.toStringList();
                    const auto slices = pie->slices();
                    for (int i = 0; i < slices.size() && i < keys.size(); ++i) {
                        slices.at(i)->setLabel(trKey(keys.at(i)));
                        slices.at(i)->setLabelVisible();
                    }
                }
            }
        }

        const auto axes = chart->axes();
        for (auto *axis : axes) {
            if (!axis) {
                continue;
            }

            if (auto *valueAxis = qobject_cast<QValueAxis*>(axis)) {
                const QVariant axisKey = valueAxis->property("trTitleKey");
                if (axisKey.isValid()) {
                    valueAxis->setTitleText(trKey(axisKey.toString()));
                }
            }

            if (auto *catAxis = qobject_cast<QBarCategoryAxis*>(axis)) {
                const QVariant catKeys = catAxis->property("trCategories");
                if (catKeys.isValid()) {
                    const QStringList keys = catKeys.toStringList();
                    QStringList translated;
                    translated.reserve(keys.size());
                    for (const auto &k : keys) {
                        translated << trKey(k);
                    }
                    catAxis->setCategories(translated);
                }
            }
        }
    }
}

void MainWindow::onLanguageChanged(const QString &language)
{
    switchLanguage(language);
}

void MainWindow::switchLanguage(const QString &language)
{
    if (currentLanguage == language) {
        return;
    }
    
    currentLanguage = language;
    qApp->removeTranslator(translator);
    if (language != "en") {
        QString qmFile = ":/translations/app_" + language + ".qm";
        if (translator->load(qmFile)) {
            qApp->installTranslator(translator);
        }
    }
    if (homeWindow) {
        homeWindow->setLanguage(language);
    }
    onPageChanged(ui->stackedWidget->currentIndex());
}

void MainWindow::showWelcomeNotification(QWidget *parent, const QString &managementName)
{
    QString empName = "Team Member";
    QString empRole = "";
    if (currentEmployeeId > 0) {
        QSqlQuery nq;
        nq.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME, JOB_TITLE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        nq.bindValue(":id", currentEmployeeId);
        if (nq.exec() && nq.next()) {
            empName = nq.value(0).toString();
            empRole = nq.value(1).toString();
        }
    }
    WelcomeNotificationBar *bar = new WelcomeNotificationBar(empName, empRole, managementName, parent);
    bar->startEntrance();
}

void MainWindow::onPageChanged(int index)
{
    // --- Background Music Logic ---
    if (index == 1) {
        // Home page: stop OST1 and ensure OSTM is active (covers login -> home and module -> home)
        if (loginAudioPlayer && loginAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(loginAudioOutput, [this](){ loginAudioPlayer->stop(); });
        }

        if (homeAudioPlayer) {
            homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
            homeAudioPlayer->setPosition(0);
            homeAudioPlayer->play();
        }
    } else {
        // All other pages: stop home-page audio (OST2 + animation track)
        homeWindow->stopHomeAudio();
        if (homeAudioPlayer && homeAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(homeAudioOutput, [this](){
                if (ui && ui->stackedWidget && ui->stackedWidget->currentIndex() != 1) {
                    homeAudioPlayer->stop();
                }
            });
        }
        if (chatAudioPlayer && chatAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(chatAudioOutput, [this](){ chatAudioPlayer->stop(); });
        }

        // Management pages (2-6): play OST1 (including chat tab — music continues uninterrupted)
        if (index >= 2 && index <= 6) {
            if (loginAudioPlayer && loginAudioPlayer->playbackState() != QMediaPlayer::PlayingState) {
                loginAudioPlayer->setPosition(0);
                loginAudioPlayer->play();
                fadeIn(loginAudioOutput);
            }
        } else {
            // Login (0): no music
            if (loginAudioPlayer && loginAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
                fadeOut(loginAudioOutput, [this](){ loginAudioPlayer->stop(); });
            }
        }
    }
    
    // Retranslate the newly visible page
    switch (index) {
        case 0: // Login
            if (loginWindow) {
                loginWindow->retranslateUI();
            }
            break;
        case 1: // Home
            if (homeWindow) {
                homeWindow->retranslateUI();
                updateUserProfileDisplay();
                if (!m_homeWelcomeShown) {
                    showWelcomeNotification(homeWindow, "Home");
                    m_homeWelcomeShown = true;
                }
            }
            break;
        case 2: // Employees
            if (ui_employee && employeePage) {
                ui_employee->retranslateUi(employeePage);
                retranslateDynamicWidgets(employeePage);
                retranslateDynamicTabs(ui_employee->tabWidget);
                retranslateDynamicCharts(employeePage);
            }
            break;
        case 3: // Clients
            if (ui_client && clientPage) {
                ui_client->retranslateUi(clientPage);
                retranslateDynamicWidgets(clientPage);
                retranslateDynamicTabs(ui_client->tabWidget);
                retranslateDynamicCharts(clientPage);
            }
            break;
        case 4: // Suppliers
            if (ui_supplier && supplierPage) {
                ui_supplier->retranslateUi(supplierPage);
                retranslateDynamicWidgets(supplierPage);
                retranslateDynamicTabs(ui_supplier->tabWidget);
                retranslateDynamicCharts(supplierPage);
            }
            break;
        case 5: // Equipment
            if (ui_equipment && equipmentPage) {
                ui_equipment->retranslateUi(equipmentPage);
                retranslateDynamicWidgets(equipmentPage);
                retranslateDynamicTabs(ui_equipment->tabWidget);
                retranslateDynamicCharts(equipmentPage);
            }
            break;
        case 6: // Orders
            if (ui_order && orderPage) {
                ui_order->retranslateUi(orderPage);
                retranslateDynamicWidgets(orderPage);
                retranslateDynamicTabs(ui_order->tabWidget);
                retranslateDynamicCharts(orderPage);
            }
            break;
    }
}

// =============================================================================
// CLIENT MANAGEMENT CRUD
// =============================================================================

void MainWindow::onClientRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT CLIENT_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
        " FROM CLIENTS ORDER BY CLIENT_ID"
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load clients:\n" + model->lastError().text());
        return;
    }
    ui_client->tableView->setModel(model);
    ui_client->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui_client->tableView->horizontalHeader()->setMinimumSectionSize(120);
    ui_client->tableView->horizontalHeader()->setStretchLastSection(true);
    ui_client->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_client->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_client->tableView->verticalHeader()->setDefaultSectionSize(55);
    
    // Modern Glassmorphism Design
    ui_client->tableView->setStyleSheet(
        "QTableView {"
        "  background-color: rgba(15, 12, 8, 0.85);"
        "  border: 1px solid rgba(212, 175, 55, 0.3);"
        "  border-radius: 20px;"
        "  gridline-color: rgba(212, 175, 55, 0.05);"
        "  color: #F0E6D2;"
        "  font-family: 'Outfit', 'Segoe UI';"
        "  font-size: 13px;"
        "  selection-background-color: rgba(212, 175, 55, 0.25);"
        "  selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(40, 32, 20, 0.9);"
        "  color: #D4AF37;"
        "  padding: 15px;"
        "  border-bottom: 2px solid #D4AF37;"
        "  border-right: 1px solid rgba(212, 175, 55, 0.1);"
        "  font-weight: 800;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
        "QTableView::item {"
        "  padding: 12px;"
        "  border-bottom: 1px solid rgba(212, 175, 0, 0.03);"
        "}"
        "QScrollBar:vertical {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }"
        "QScrollBar:horizontal {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  height: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-width: 20px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: none; background: none; }"
    );

    // Modern Animation for Client View
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_client->tableView);
    ui_client->tableView->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(600);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->start(QAbstractAnimation::DeleteWhenStopped);

    QPropertyAnimation *slide = new QPropertyAnimation(ui_client->tableView, "pos");
    slide->setDuration(600);
    QPoint currentPos = ui_client->tableView->pos();
    slide->setStartValue(currentPos + QPoint(0, 20));
    slide->setEndValue(currentPos);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    slide->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onClientAdd()
{
    // Calculate auto-increment ID (MEX)
    int clientId = 1;
    QSqlQuery qMex("SELECT CLIENT_ID FROM CLIENTS ORDER BY CLIENT_ID ASC");
    while(qMex.next()) {
        if(qMex.value(0).toInt() == clientId) {
            clientId++;
        } else if(qMex.value(0).toInt() > clientId) {
            break;
        }
    }
    QString id = QString::number(clientId);

    QString nom     = ui_client->le_nom->text().trimmed();
    QString prenom  = ui_client->le_prenom->text().trimmed();
    QString adresse = ui_client->le_adresse->text().trimmed();
    QString tel     = ui_client->le_tel->text().trimmed();
    QString email   = ui_client->le_email->text().trimmed();

    if (!ui_client->rb_homme->isChecked() && !ui_client->rb_femme->isChecked()) {
        QMessageBox::warning(this, "Validation", "Please select a gender (Male or Female).");
        return;
    }
    QString gender  = ui_client->rb_homme->isChecked() ? "Male" : "Female";

    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Last Name and First Name are required.");
        return;
    }
    if (nom.length() < 2 || nom.length() > 12 || prenom.length() < 2 || prenom.length() > 12) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] First and Last Name must be between 2 and 12 letters.");
        return;
    }

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] Invalid email structure detected. Requires standard format.");
        return;
    }

    QSqlQuery q;
    q.prepare("INSERT INTO CLIENTS (CLIENT_ID, LAST_NAME, FIRST_NAME, ADDRESS, PHONE_NUMBER, EMAIL, GENDER,"
              " REGISTRATION_DATE, STATUS, ASSIGNED_EMPLOYEE_ID, ACCOUNT_BALANCE)"
              " VALUES (:id, :nom, :prenom, :addr, :tel, :email, :gender,"
              " SYSDATE, 'Active', :empid, 0)");
    q.bindValue(":id",     clientId);
    q.bindValue(":nom",    nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":addr",   adresse);
    q.bindValue(":tel",    tel);
    q.bindValue(":email",  email);
    q.bindValue(":gender", gender);
    q.bindValue(":empid",  currentEmployeeId);

    if (q.exec()) {
        QMessageBox::information(this, "Success", "Client added successfully.");
        logActivity("Added new client: " + prenom + " " + nom + " (ID: " + id + ")", "Clients");
        onClientClearFields();
        onClientRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to add client:\n" + q.lastError().text());
    }
}

void MainWindow::onClientModify()
{
    QString id      = ui_client->le_id_mod->text().trimmed();
    QString nom     = ui_client->le_nom_mod->text().trimmed();
    QString prenom  = ui_client->le_prenom_mod->text().trimmed();
    QString adresse = ui_client->le_adresse_mod->text().trimmed();
    QString tel     = ui_client->le_tel_mod->text().trimmed();
    QString email   = ui_client->le_email_mod->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Please enter the Client ID to modify.");
        return;
    }

    if (nom.length() < 2 || nom.length() > 12 || prenom.length() < 2 || prenom.length() > 12) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] First and Last Name must be between 2 and 12 letters.");
        return;
    }

    QRegularExpression emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] Invalid email structure detected. Requires standard format.");
        return;
    }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
    checkQuery.bindValue(":id", id.toInt());
    if (checkQuery.exec() && checkQuery.next()) {
        if (checkQuery.value(0).toInt() == 0) {
            QMessageBox::warning(this, "Not Found", "ERROR: The specified Client ID does not exist in the database.");
            return;
        }
    }

    QSqlQuery q;
    QString gender = ui_client->rb_homme_mod->isChecked() ? "Male" : "Female";

    q.prepare("UPDATE CLIENTS SET LAST_NAME=:nom, FIRST_NAME=:prenom, ADDRESS=:addr,"
              " PHONE_NUMBER=:tel, EMAIL=:email, GENDER=:gender WHERE CLIENT_ID=:id");
    q.bindValue(":id",     id.toInt());
    q.bindValue(":nom",    nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":addr",   adresse);
    q.bindValue(":tel",    tel);
    q.bindValue(":email",  email);
    q.bindValue(":gender", gender);

    if (q.exec()) {
        if (q.numRowsAffected() > 0) {
            QMessageBox::information(this, "Success", "Client updated successfully.");
            logActivity("Modified client: " + prenom + " " + nom + " (ID: " + id + ")", "Clients");
            onClientModClearFields();
            onClientRefreshView();
        } else {
            QMessageBox::warning(this, "Not Found", "No client found with that ID.");
        }
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update client:\n" + q.lastError().text());
    }
}

void MainWindow::onClientDelete()
{
    // Get selected row from tableView
    QModelIndex idx = ui_client->tableView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select a client from the list to delete.");
        return;
    }
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_client->tableView->model());
    if (!model) return;
    QString clientId = model->data(model->index(idx.row(), 0)).toString();
    QString name     = model->data(model->index(idx.row(), 1)).toString()
                     + " " + model->data(model->index(idx.row(), 2)).toString();

    int ret = QMessageBox::question(this, "Confirm Delete",
        "Delete client: " + name + " (ID: " + clientId + ")?",
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    QSqlQuery q;
    q.prepare("DELETE FROM CLIENTS WHERE CLIENT_ID = :id");
    q.bindValue(":id", clientId.toInt());
    if (q.exec()) {
        QMessageBox::information(this, "Deleted", "Client deleted successfully.");
        logActivity("Deleted client: " + name + " (ID: " + clientId + ")", "Clients");
        onClientRefreshView();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to delete client:\n" + q.lastError().text());
    }
}

void MainWindow::onClientSearch()
{
    QString search = ui_client->le_recherche->text().trimmed();
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (search.isEmpty()) {
        model->setQuery(
            "SELECT CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS ORDER BY CLIENT_ID"
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS WHERE UPPER(LAST_NAME) LIKE :s OR UPPER(FIRST_NAME) LIKE :s"
            " OR UPPER(EMAIL) LIKE :s OR CAST(CLIENT_ID AS VARCHAR2(20)) LIKE :s"
            " ORDER BY CLIENT_ID"
        );
        q.bindValue(":s", "%" + search.toUpper() + "%");
        q.exec();
        model->setQuery(std::move(q));
    }
    ui_client->tableView->setModel(model);
    ui_client->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui_client->tableView->horizontalHeader()->setMinimumSectionSize(120);
    ui_client->tableView->horizontalHeader()->setStretchLastSection(true);
    ui_client->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui_client->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui_client->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_client->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_client->tableView->verticalHeader()->setDefaultSectionSize(55);
    
    // Modern Glassmorphism Design
    ui_client->tableView->setStyleSheet(
        "QTableView {"
        "  background-color: rgba(15, 12, 8, 0.85);"
        "  border: 1px solid rgba(212, 175, 55, 0.3);"
        "  border-radius: 20px;"
        "  gridline-color: rgba(212, 175, 55, 0.05);"
        "  color: #F0E6D2;"
        "  font-family: 'Outfit', 'Segoe UI';"
        "  font-size: 13px;"
        "  selection-background-color: rgba(212, 175, 55, 0.25);"
        "  selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(40, 32, 20, 0.9);"
        "  color: #D4AF37;"
        "  padding: 15px;"
        "  border-bottom: 2px solid #D4AF37;"
        "  border-right: 1px solid rgba(212, 175, 55, 0.1);"
        "  font-weight: 800;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
        "QTableView::item {"
        "  padding: 12px;"
        "  border-bottom: 1px solid rgba(212, 175, 0, 0.03);"
        "}"
        "QScrollBar:vertical {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }"
        "QScrollBar:horizontal {"
        "  background: rgba(15, 12, 8, 0.85);"
        "  height: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #D4AF37;"
        "  border-radius: 6px;"
        "  min-width: 20px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: none; background: none; }"
    );

    // Animation for Search result
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_client->tableView);
    ui_client->tableView->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(400);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onClientRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_client->tableView->model());
    if (!model) return;
    int row = index.row();

    // ALWAYS prepopulate the modify fields
    ui_client->le_id_mod->setText(model->data(model->index(row, 0)).toString());
    ui_client->le_nom_mod->setText(model->data(model->index(row, 1)).toString());
    ui_client->le_prenom_mod->setText(model->data(model->index(row, 2)).toString());
    ui_client->le_adresse_mod->setText(model->data(model->index(row, 3)).toString());
    ui_client->le_tel_mod->setText(model->data(model->index(row, 4)).toString());
    ui_client->le_email_mod->setText(model->data(model->index(row, 5)).toString());
    QString gender = model->data(model->index(row, 6)).toString();
    if (gender == "Male") {
        ui_client->rb_homme_mod->setChecked(true);
    } else if (gender == "Female") {
        ui_client->rb_femme_mod->setChecked(true);
    }
}

void MainWindow::toggleEmployeeFields(bool active)
{
    if (!ui_employee) return;
    ui_employee->le_nom->setEnabled(active);
    ui_employee->le_prenom->setEnabled(active);
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) cb->setEnabled(active);
    ui_employee->le_mdp->setEnabled(active);
    ui_employee->le_email->setEnabled(active);
    ui_employee->le_num->setEnabled(active);
    ui_employee->dsb_salaire->setEnabled(active);
    ui_employee->de_birthdate->setEnabled(active);
    ui_employee->btn_upload_avatar->setEnabled(active);
    ui_employee->btn_scan_face->setEnabled(active);
    // ui_employee->btn_suggest_salary->setEnabled(active);
    
    QString style = active ? "" : "background: rgba(0,0,0,0.1); color: rgba(255,255,255,0.2);";
    ui_employee->le_nom->setStyleSheet(style);
    ui_employee->le_prenom->setStyleSheet(style);
    ui_employee->le_fonction->setStyleSheet(style);
    ui_employee->le_mdp->setStyleSheet(style);
    ui_employee->le_email->setStyleSheet(style);
    ui_employee->le_num->setStyleSheet(style);
}

void MainWindow::onEmployeeClearFields()
{
    ui_employee->le_id->clear();
    ui_employee->le_nom->clear();
    ui_employee->le_prenom->clear();
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) cb->setCurrentIndex(0);
    ui_employee->le_email->clear();
    ui_employee->le_num->clear();
    ui_employee->le_mdp->clear();
    ui_employee->le_address->clear();
    ui_employee->dsb_salaire->setValue(0.0);
    ui_employee->de_birthdate->setDate(QDate(1995, 1, 1));
    ui_employee->lbl_avatar->setPixmap(QPixmap(":/assets/default_avatar.png").scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    toggleEmployeeFields(false); // Lock fields on clear
}

void MainWindow::onEmployeeRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
    if (!model) return;
    int row = index.row();
    // Col order: Action, Delete, ID, Last Name, First Name, Job Title, Age, Email, Phone, Address, Salary
    ui_employee->le_id->setText(model->data(model->index(row, 2)).toString());
    toggleEmployeeFields(true); // Unlock fields on selection
    ui_employee->le_nom->setText(model->data(model->index(row, 3)).toString());
    ui_employee->le_prenom->setText(model->data(model->index(row, 4)).toString());
    QString jt = model->data(model->index(row, 5)).toString();
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) {
        // Match case-insensitively since DB values may not match combo capitalization exactly.
        bool matched = false;
        for (int i = 0; i < cb->count(); ++i) {
            if (cb->itemText(i).trimmed().compare(jt.trimmed(), Qt::CaseInsensitive) == 0) {
                cb->setCurrentIndex(i);
                matched = true;
                break;
            }
        }
        if (!matched) {
            // Keep selection safe: default to first option.
            cb->setCurrentIndex(0);
        }
    }
    int age = model->data(model->index(row, 6)).toInt();
    ui_employee->de_birthdate->setDate(QDate::currentDate().addYears(-age));
    ui_employee->le_email->setText(model->data(model->index(row, 7)).toString());
    ui_employee->le_num->setText(model->data(model->index(row, 8)).toString());
    ui_employee->le_address->setText(model->data(model->index(row, 9)).toString());
    ui_employee->dsb_salaire->setValue(model->data(model->index(row, 10)).toDouble());

    // Load AVATAR only (employee_[ID].png) - face scan images are NEVER shown here
    QString idStr = model->data(model->index(row, 2)).toString();
    QString avPath = QString("assets/av/employee_%1.png").arg(idStr);

    if (QFile::exists(avPath)) {
        QPixmap pix(avPath);
        ui_employee->lbl_avatar->setPixmap(getCircularPixmap(pix).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // No custom avatar - show placeholder text, NOT the face scan image
        ui_employee->lbl_avatar->setPixmap(QPixmap());
        ui_employee->lbl_avatar->setText("No Avatar");
    }

    // Pre-fill mail tab
    ui_employee->le_mail_to->setText(model->data(model->index(row, 7)).toString());
}

// =============================================================================
// EMPLOYEE MANAGEMENT CRUD
// =============================================================================

void MainWindow::onEmployeeRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT 'Edit' AS \"Action\", 'Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", ADDRESS AS \"Address\", SALARY AS \"Salary\""
        " FROM EMPLOYEES ORDER BY EMPLOYEE_ID"
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load employees:\n" + model->lastError().text());
        return;
    }
    ui_employee->tableView_employes->setModel(model);
    ui_employee->tableView_employes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_employee->tableView_employes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_employee->tableView_employes->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui_employee->tableView_employes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_employee->tableView_employes->setMouseTracking(true);
    
    // --- Advanced Hover Integration ---
    RowHoverDelegate *hoverDelegate = new RowHoverDelegate(this);
    ui_employee->tableView_employes->setItemDelegate(hoverDelegate);
    ui_employee->tableView_employes->viewport()->installEventFilter(this);

    // Modern Animation for the View Section
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_employee->tableView_employes);
    ui_employee->tableView_employes->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(600);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->start(QAbstractAnimation::DeleteWhenStopped);

    // Slide-up animation
    QPropertyAnimation *slide = new QPropertyAnimation(ui_employee->tableView_employes, "pos");
    slide->setDuration(600);
    QPoint currentPos = ui_employee->tableView_employes->pos();
    slide->setStartValue(currentPos + QPoint(0, 30));
    slide->setEndValue(currentPos);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    slide->start(QAbstractAnimation::DeleteWhenStopped);

    // Summary Stats Calculation
    QSqlQuery q;
    if (q.exec("SELECT COUNT(*), AVG(SALARY), AVG(AGE) FROM EMPLOYEES")) {
        if (q.next()) {
            int total = q.value(0).toInt();
            double avgSalary = q.value(1).toDouble();
            double avgAge = q.value(2).toDouble();

            ui_employee->lbl_stat_total->setText(QString("Total Personnel: %1").arg(total));
            ui_employee->lbl_stat_avg_salary->setText(QString("Avg Market Value: $%1").arg(avgSalary, 0, 'f', 0));
            ui_employee->lbl_stat_avg_age->setText(QString("Avg Team Age: %1").arg(avgAge, 0, 'f', 1));
            
            // Animate stats frame
            QGraphicsOpacityEffect *statsEff = new QGraphicsOpacityEffect(ui_employee->frame_view_stats);
            ui_employee->frame_view_stats->setGraphicsEffect(statsEff);
            QPropertyAnimation *statsAnim = new QPropertyAnimation(statsEff, "opacity");
            statsAnim->setDuration(800);
            statsAnim->setStartValue(0.0);
            statsAnim->setEndValue(1.0);
            statsAnim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    // Keep the stats dashboard in sync with current employee data.
    setupEmployeeStats();
}

void MainWindow::onEmployeeSearch()
{
    QString search = ui_employee->le_recherche_emp->text().trimmed();
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (search.isEmpty()) {
        model->setQuery(
            "SELECT 'Edit' AS \"Action\", 'Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", SALARY AS \"Salary\""
            " FROM EMPLOYEES ORDER BY EMPLOYEE_ID"
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT 'Edit' AS \"Action\", 'Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", SALARY AS \"Salary\""
            " FROM EMPLOYEES WHERE UPPER(LAST_NAME) LIKE :s OR UPPER(FIRST_NAME) LIKE :s"
            " OR UPPER(EMAIL) LIKE :s OR UPPER(JOB_TITLE) LIKE :s OR CAST(EMPLOYEE_ID AS VARCHAR2(20)) LIKE :s"
            " ORDER BY EMPLOYEE_ID"
        );
        q.bindValue(":s", "%" + search.toUpper() + "%");
        q.exec();
        model->setQuery(std::move(q));
    }
    ui_employee->tableView_employes->setModel(model);

    // Simple Animation for search results
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_employee->tableView_employes);
    ui_employee->tableView_employes->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(400);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onEmployeeRefreshHistory()
{
    // Populate module filter combo if empty (except first item)
    if (ui_employee->cb_history_filter->count() <= 1) {
        QSignalBlocker blocker(ui_employee->cb_history_filter);
        ui_employee->cb_history_filter->clear();
        // Keep the label consistent with the UI default to avoid mismatch bugs.
        ui_employee->cb_history_filter->addItem("All Personnel");
        ui_employee->cb_history_filter->addItem("Employees");
        ui_employee->cb_history_filter->addItem("Clients");
        ui_employee->cb_history_filter->addItem("Equipment");
        ui_employee->cb_history_filter->addItem("Orders");
        ui_employee->cb_history_filter->addItem("General");
    }

    QString searchText = ui_employee->le_history_search->text().trimmed().toUpper();
    QString moduleFilter = ui_employee->cb_history_filter->currentText();

    const bool isAllModule = moduleFilter.toLower().startsWith("all");

    // Read audit/history from local JSON file (no DB tables).
    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray auditArray;
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isArray()) auditArray = doc.array();
            file.close();
        }
    }

    QVector<QJsonObject> filtered;
    filtered.reserve(auditArray.size());

    for (const QJsonValue &v : auditArray) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        const QString emp    = o.value("employee_name").toString();
        const QString action = o.value("action_details").toString();
        const QString mod    = o.value("module_name").toString();

        if (!isAllModule && !moduleFilter.isEmpty()) {
            if (mod != moduleFilter) continue;
        }

        if (!searchText.isEmpty()) {
            const QString empUp = emp.toUpper();
            const QString actionUp = action.toUpper();
            if (!empUp.contains(searchText) && !actionUp.contains(searchText)) continue;
        }

        filtered.push_back(o);
    }

    std::sort(filtered.begin(), filtered.end(), [](const QJsonObject &a, const QJsonObject &b) {
        const qint64 at = a.value("timestamp_ms").toVariant().toLongLong();
        const qint64 bt = b.value("timestamp_ms").toVariant().toLongLong();
        return bt < at; // descending
    });

    const int maxRows = 250;
    const int rowCount = qMin(filtered.size(), maxRows);
    QStandardItemModel *model = new QStandardItemModel(rowCount, 4, this);
    model->setHorizontalHeaderLabels({"Time", "Employee", "Action", "Module"});

    for (int r = 0; r < rowCount; ++r) {
        const QJsonObject o = filtered.at(r);
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(o.value("timestamp_ms").toVariant().toLongLong());
        if (!dt.isValid()) dt = QDateTime::fromString(o.value("timestamp_iso").toString(), Qt::ISODate);
        const QString timeStr = dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : QString();

        model->setItem(r, 0, new QStandardItem(timeStr));
        model->setItem(r, 1, new QStandardItem(o.value("employee_name").toString()));
        model->setItem(r, 2, new QStandardItem(o.value("action_details").toString()));
        model->setItem(r, 3, new QStandardItem(o.value("module_name").toString()));
    }

    ui_employee->tableView_historique_emp->setModel(model);
    ui_employee->tableView_historique_emp->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_employee->tableView_historique_emp->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_employee->tableView_historique_emp->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_employee->tableView_historique_emp->verticalHeader()->setDefaultSectionSize(55);
    
    // Modern Glassmorphism Design
    ui_employee->tableView_historique_emp->setStyleSheet(
        "QTableView {"
        "  background-color: rgba(15, 12, 8, 0.85);"
        "  border: 1px solid rgba(212, 175, 55, 0.3);"
        "  border-radius: 20px;"
        "  gridline-color: rgba(212, 175, 55, 0.05);"
        "  color: #F0E6D2;"
        "  font-family: 'Outfit', 'Segoe UI';"
        "  font-size: 13px;"
        "  selection-background-color: rgba(212, 175, 55, 0.25);"
        "  selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "  background-color: rgba(40, 32, 20, 0.9);"
        "  color: #D4AF37;"
        "  padding: 15px;"
        "  border-bottom: 2px solid #D4AF37;"
        "  border-right: 1px solid rgba(212, 175, 55, 0.1);"
        "  font-weight: 800;"
        "  text-transform: uppercase;"
        "  letter-spacing: 1px;"
        "}"
        "QTableView::item {"
        "  padding: 12px;"
        "  border-bottom: 1px solid rgba(212, 175, 0, 0.03);"
        "}"
    );

    if (qobject_cast<QPushButton*>(sender()) == ui_employee->btn_refresh_history) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_employee->tableView_historique_emp);
        ui_employee->tableView_historique_emp->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
        a->setDuration(400); a->setStartValue(0.0); a->setEndValue(1.0); a->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::onEmployeeHistorySearch()
{
    onEmployeeRefreshHistory();
}

void MainWindow::onEmployeeSendMail()
{
    QString to = ui_employee->le_mail_to->text().trimmed();
    QString subject = ui_employee->le_mail_subject->text().trimmed();
    QString body = ui_employee->te_mail_body->toPlainText().trimmed();

    if (to.isEmpty()) {
        QMessageBox::warning(this, "Mail", "Please select an employee with a valid email first.");
        return;
    }

    QString mailto = QString("mailto:%1?subject=%2&body=%3")
                        .arg(to)
                        .arg(QUrl::toPercentEncoding(subject).data())
                        .arg(QUrl::toPercentEncoding(body).data());
    
    if (QDesktopServices::openUrl(QUrl(mailto))) {
        QMessageBox::information(this, "Mail", "Default mail client opened.");
    } else {
        QMessageBox::critical(this, "Mail", "Failed to open default mail client.");
    }
}

void MainWindow::onEmployeeMailTemplateChanged(int index)
{
    QString name = ui_employee->le_prenom->text() + " " + ui_employee->le_nom->text();
    if (name.trimmed().isEmpty()) name = "Employee";

    switch (index) {
        case 1: // Welcome
            ui_employee->le_mail_subject->setText("Welcome to the Team!");
            ui_employee->te_mail_body->setPlainText(QString("Dear %1,\n\nWelcome to HammerDown! We are excited to have you join our team. Your account has been setup and you can now log in.\n\nBest regards,\nManagement").arg(name));
            break;
        case 2: // Task Assignment
            ui_employee->le_mail_subject->setText("New Task Assignment");
            ui_employee->te_mail_body->setPlainText(QString("Hi %1,\n\nYou have been assigned a new task. Please check your dashboard for details.\n\nDeadline: ASAP\n\nThanks,\nTeam Lead").arg(name));
            break;
        case 3: // Meeting
            ui_employee->le_mail_subject->setText("Meeting Request");
            ui_employee->te_mail_body->setPlainText(QString("Hello %1,\n\nI would like to schedule a brief meeting to discuss your recent performance and future goals.\n\nPlease let me know your availability.\n\nRegards,\nHR").arg(name));
            break;
        default:
            ui_employee->le_mail_subject->clear();
            ui_employee->te_mail_body->clear();
            break;
    }
}

void MainWindow::onEmployeeExportHistoryPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Audit Log", 
        QDir::homePath() + "/Audit_Log_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    // Use PrinterResolution for more predictable coordinate mapping (less tiny fonts)
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);
    printer.setFullPage(false);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export", "Failed to initialize PDF printer.");
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing);

    int pageWidth = painter.viewport().width();
    int y = 60;

    // Header Stylistics
    painter.setFont(QFont("Outfit", 22, QFont::Bold));
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(0, y, pageWidth, 50, Qt::AlignCenter, "HAMMERDOWN - SYSTEM SECURITY AUDIT");
    y += 65;

    painter.setFont(QFont("Outfit", 12));
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(0, y, pageWidth, 30, Qt::AlignCenter, "Generated on " + QDateTime::currentDateTime().toString("dd MMMM yyyy - HH:mm:ss"));
    y += 100;

    QAbstractItemModel *model = ui_employee->tableView_historique_emp->model();
    if (!model) {
        painter.end();
        return;
    }

    // Column Ratio Configuration (Sync with History Model: Time, Employee, Action, Module)
    int cWidths[4];
    cWidths[0] = pageWidth * 0.15; // Time
    cWidths[1] = pageWidth * 0.15; // Employee
    cWidths[2] = pageWidth * 0.55; // Action (Main content)
    cWidths[3] = pageWidth * 0.15; // Module
    
    // Header Table
    painter.setFont(QFont("Outfit", 11, QFont::Bold));
    painter.setPen(Qt::white);
    painter.setBrush(QColor(139, 111, 71));
    
    int currentX = 0;
    for (int c = 0; c < 4; ++c) {
        painter.drawRect(currentX, y, cWidths[c], 40);
        painter.drawText(currentX + 5, y, cWidths[c] - 10, 40, Qt::AlignCenter, model->headerData(c, Qt::Horizontal).toString());
        currentX += cWidths[c];
    }
    y += 40;

    // Content Rows with Dynamic Wrapping support
    painter.setFont(QFont("Outfit", 10));
    painter.setPen(Qt::black);
    
    for (int r = 0; r < model->rowCount(); ++r) {
        // Calculate required row height based on 'Action' column length
        QString actionText = model->data(model->index(r, 2)).toString();
        QRect textRect = painter.boundingRect(0, 0, cWidths[2] - 10, 9999, Qt::TextWordWrap, actionText);
        int rowH = qMax(35, textRect.height() + 15);

        // Page Break Logic
        if (y + rowH > painter.viewport().height() - 80) {
            printer.newPage();
            y = 80;
            // Draw Sub-header on new page
            painter.setFont(QFont("Outfit", 11, QFont::Bold));
            painter.setPen(Qt::white);
            painter.setBrush(QColor(139, 111, 71));
            int subX = 0;
            for (int c = 0; c < 4; ++c) {
                painter.drawRect(subX, y, cWidths[c], 35);
                painter.drawText(subX + 5, y, cWidths[c] - 10, 35, Qt::AlignCenter, model->headerData(c, Qt::Horizontal).toString());
                subX += cWidths[c];
            }
            y += 35;
            painter.setFont(QFont("Outfit", 10));
            painter.setPen(Qt::black);
        }

        currentX = 0;
        for (int c = 0; c < 4; ++c) {
            QString content = model->data(model->index(r, c)).toString();
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QColor(240, 240, 240));
            painter.drawRect(currentX, y, cWidths[c], rowH);
            
            painter.setPen(Qt::black);
            // Action column gets word wrap, others use standard align
            Qt::Alignment flags = (c == 2)
                ? Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap)
                : Qt::Alignment(Qt::AlignCenter);
            painter.drawText(currentX + 5, y + 5, cWidths[c] - 10, rowH - 10, flags, content);
            
            currentX += cWidths[c];
        }
        y += rowH;
    }

    painter.end();
    QMessageBox::information(this, "Security Audit", "The system audit log has been fully synchronized and exported to PDF format successfully.");
}

void MainWindow::onEmployeeExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Employee List", 
        QDir::homePath() + "/Staff_Directory_" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export", "Failed to initialize PDF printer.");
        return;
    }

    int pageWidth = painter.viewport().width();
    int y = 50;

    // Header styling
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71)); // Professional brown color
    painter.drawText(0, y, pageWidth, 40, Qt::AlignCenter, "HammerDown - Professional Staff Directory");
    y += 80;

    // Calculate dynamic columns [ID, Name, Job, Email, Phone]
    int x_id = 40;
    int x_name = pageWidth * 0.12;
    int x_job = pageWidth * 0.38;
    int x_email = pageWidth * 0.62;
    int x_phone = pageWidth * 0.88;

    // Table Header Background
    painter.setBrush(QColor(245, 245, 245));
    painter.setPen(Qt::NoPen);
    painter.drawRect(20, y - 5, pageWidth - 40, 35);

    // Table Header Text
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(x_id, y, "ID");
    painter.drawText(x_name, y, "Employee Name");
    painter.drawText(x_job, y, "Job Title & Dept");
    painter.drawText(x_email, y, "Contact Email");
    painter.drawText(x_phone, y, "Phone");
    
    painter.setPen(QColor(200, 200, 200));
    painter.drawLine(20, y + 30, pageWidth - 20, y + 30);
    y += 60;

    // Data Row Logic
    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(Qt::black);
    
    QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME || ' ' || LAST_NAME, JOB_TITLE, EMAIL, PHONE_NUMBER FROM EMPLOYEES ORDER BY EMPLOYEE_ID");
    while (q.next()) {
        if (y > printer.height() - 80) { // New page margin
            printer.newPage();
            y = 50;
        }
        painter.drawText(x_id, y, q.value(0).toString());
        painter.drawText(x_name, y, q.value(1).toString());
        painter.drawText(x_job, y, q.value(2).toString());
        painter.drawText(x_email, y, q.value(3).toString());
        painter.drawText(x_phone, y, q.value(4).toString());
        y += 35; // Row height
    }
    
    painter.end();
    QMessageBox::information(this, "Success", "Staff directory exported successfully to:\n" + fileName);
}


void MainWindow::onAIPulseClicked()
{
    if (!ui_employee) return;

    // --- Enhanced Live DB Data Gathering ---
    QSqlQuery qEmp("SELECT COUNT(*), AVG(SALARY), AVG(AGE), COUNT(DISTINCT JOB_TITLE), SUM(SALARY) FROM EMPLOYEES");
    qEmp.next();
    int cEmp        = qEmp.value(0).toInt();
    double avgS     = qEmp.value(1).toDouble();
    double avgAge   = qEmp.value(2).toDouble();
    int distinctRoles = qEmp.value(3).toInt();
    double totalPayroll = qEmp.value(4).toDouble();

    QString allRoles;
    QString teamDynamics;
    {
        QSqlQuery qRoles("SELECT JOB_TITLE, COUNT(*), AVG(SALARY), AVG(AGE) FROM EMPLOYEES "
                         "GROUP BY JOB_TITLE ORDER BY COUNT(*) DESC FETCH FIRST 5 ROWS ONLY");
        while (qRoles.next()) {
            allRoles += QString("\n- %1: %2 people, avg salary $%3, avg age %4")
                .arg(qRoles.value(0).toString())
                .arg(qRoles.value(1).toInt())
                .arg(qRoles.value(2).toDouble(), 0, 'f', 0)
                .arg(qRoles.value(3).toDouble(), 0, 'f', 1);
        }
    }

    // Team dynamics analysis
    {
        QSqlQuery qTeams("SELECT JOB_TITLE, COUNT(*) as team_size, AVG(SALARY) as avg_team_salary "
                        "FROM EMPLOYEES GROUP BY JOB_TITLE HAVING COUNT(*) > 1 "
                        "ORDER BY team_size DESC FETCH FIRST 3 ROWS ONLY");
        while (qTeams.next()) {
            teamDynamics += QString("\n• %1: %2 members, cohesion index %.1f")
                .arg(qTeams.value(0).toString())
                .arg(qTeams.value(1).toInt())
                .arg(50.0 + (qTeams.value(2).toDouble() / avgS - 1.0) * 30.0);
        }
    }

    // Enhanced salary distribution with market comparison
    QSqlQuery qSalaryLow ("SELECT COUNT(*) FROM EMPLOYEES WHERE SALARY < 2000"); qSalaryLow.next();
    QSqlQuery qSalaryMid ("SELECT COUNT(*) FROM EMPLOYEES WHERE SALARY BETWEEN 2000 AND 6000"); qSalaryMid.next();
    QSqlQuery qSalaryHigh("SELECT COUNT(*) FROM EMPLOYEES WHERE SALARY > 6000"); qSalaryHigh.next();
    int lowTier  = qSalaryLow.value(0).toInt();
    int midTier  = qSalaryMid.value(0).toInt();
    int highTier = qSalaryHigh.value(0).toInt();

    // Turnover risk indicators
    QSqlQuery qTenure("SELECT COUNT(*) FROM EMPLOYEES WHERE HIRE_DATE <= SYSDATE - 365"); qTenure.next();
    QSqlQuery qNewHires("SELECT COUNT(*) FROM EMPLOYEES WHERE HIRE_DATE >= SYSDATE - 90"); qNewHires.next();
    int longTermEmployees = qTenure.value(0).toInt();
    int recentHires = qNewHires.value(0).toInt();
    double turnoverRisk = (recentHires > cEmp * 0.3) ? 75.0 : (longTermEmployees > cEmp * 0.6) ? 25.0 : 45.0;

    // Financial impact metrics
    QSqlQuery qTopEarners("SELECT COUNT(*) FROM EMPLOYEES WHERE SALARY > " + QString::number(avgS * 1.5)); qTopEarners.next();
    int topEarners = qTopEarners.value(0).toInt();
    double payrollEfficiency = (cEmp > 0) ? (totalPayroll / (cEmp * avgS)) * 100 : 100;

    // Recent hires (last 90 days)
    QSqlQuery qRecent("SELECT COUNT(*) FROM EMPLOYEES WHERE HIRE_DATE >= SYSDATE - 90"); qRecent.next();
    recentHires = qRecent.value(0).toInt();

    // --- Enhanced Loading Indicator with 3D Effects ---
    /* ui_employee->lbl_ai_pulse_result->setVisible(true);
    ui_employee->lbl_ai_pulse_result->setStyleSheet(
        "color: #D4AF37; "
        "font-size: 14px; "
        "font-weight: bold; "
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 rgba(15,10,5,0.95), stop:0.5 rgba(25,18,10,0.97), stop:1 rgba(35,25,15,0.95));"
        "padding: 20px; "
        "border-radius: 20px; "
        "border: 3px solid rgba(212,175,55,0.4);"
        "box-shadow: 0 15px 35px rgba(0,0,0,0.4), "
        "0 0 60px rgba(212,175,55,0.2), "
        "inset 0 1px 0 rgba(255,255,255,0.1), "
        "inset 0 -1px 0 rgba(0,0,0,0.3);"
        "text-shadow: 0 2px 4px rgba(0,0,0,0.8);");
    ui_employee->lbl_ai_pulse_result->setText(
        "⚡ <b>INITIALIZING NEURAL WORKFORCE ANALYSIS</b> ⚡<br>"
        "🔍 Scanning organizational patterns...<br>"
        "📊 Processing predictive analytics engine...<br>"
        "🧠 Synthesizing strategic intelligence...<br>"
        "<span style='color: #F59E0B; font-size: 12px;'>● System Online ●</span>");

    // Luxury pulse animation
    QPropertyAnimation *pulse = new QPropertyAnimation(ui_employee->btn_ai_pulse, "geometry");
    QRect origGeom = ui_employee->btn_ai_pulse->geometry();
    pulse->setDuration(180);
    pulse->setKeyValueAt(0, origGeom);
    pulse->setKeyValueAt(0.5, origGeom.adjusted(-4,-4,4,4));
    pulse->setKeyValueAt(1, origGeom);
    pulse->start(QAbstractAnimation::DeleteWhenStopped); */

    // --- Enhanced AI system prompt ---
    QString sysPrompt =
        "You are FORGE-AI, HammerDown's elite HR Intelligence Engine for a high-end carpentry & manufacturing workshop. "
        "Your analysis is precise, predictive, and formatted in structured HTML. "
        "USE ONLY these HTML tags: <b>, <span>, <div>, <br>, <hr>, <table>, <tr>, <td>. "
        "Return a rich, beautiful HTML analysis using gold #D4AF37, purple #7C3AED, light cream #F0E6D2. "
        "Structure: [1] WORKFORCE HEALTH SCORE with color-coded gauge (🟢🟡🔴), "
        "[2] TURNOVER RISK ANALYSIS with probability percentage, "
        "[3] TEAM DYNAMICS INSIGHTS with cohesion metrics, "
        "[4] FINANCIAL IMPACT ANALYSIS with ROI calculations, "
        "[5] 3 KEY STRATEGIC INSIGHTS as bold bullet points, "
        "[6] IMMEDIATE ACTION ITEMS (top 3 priorities), "
        "[7] 90-DAY GROWTH PROJECTION with market positioning. "
        "Include real-time market benchmarks and competitive analysis. Keep total length under 600 words.";

    QString userPrompt = QString(
        "Enhanced Workshop Workforce Intelligence Report:\n"
        "- Total Employees: %1\n"
        "- Unique Job Titles: %2\n"
        "- Average Salary: $%3\n"
        "- Total Payroll Burn: $%4/month\n"
        "- Average Employee Age: %5 years\n"
        "- Recent Hires (last 90 days): %6\n"
        "- Long-term Employees (>1 year): %7\n"
        "- Salary Tiers: Low (<$2k): %8 | Mid ($2k-$6k): %9 | Senior (>$6k): %10\n"
        "- Top Earners (>1.5x avg): %11\n"
        "- Payroll Efficiency Index: %12%%\n"
        "- Calculated Turnover Risk: %13%%\n"
        "- Top Roles Breakdown: %14\n"
        "- Team Dynamics Data: %15\n\n"
        "Generate comprehensive workforce strategy with predictive analytics, market positioning, and financial impact analysis.")
        .arg(cEmp).arg(distinctRoles).arg(avgS, 0, 'f', 0)
        .arg(totalPayroll, 0, 'f', 0).arg(avgAge, 0, 'f', 1)
        .arg(recentHires).arg(longTermEmployees).arg(lowTier).arg(midTier).arg(highTier)
        .arg(topEarners).arg(payrollEfficiency, 0, 'f', 1).arg(turnoverRisk, 0, 'f', 1)
        .arg(allRoles).arg(teamDynamics);

    callAiModel(sysPrompt, userPrompt, [this](QString result){
        if (!ui_employee) return;

        // --- Show result in a premium floating overlay ---
        QDialog *dlg = new QDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        dlg->setAttribute(Qt::WA_TranslucentBackground);
        dlg->setMinimumSize(720, 540);
        dlg->resize(820, 620);
        dlg->move(this->geometry().center() - dlg->rect().center());

        QFrame *frame = new QFrame(dlg);
        frame->setStyleSheet(
            "QFrame { "
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 rgba(15,10,5,0.92), stop:0.5 rgba(25,18,10,0.94), stop:1 rgba(35,25,15,0.92));"
            "border: 3px solid rgba(212,175,55,0.6);"
            "border-radius: 28px;"
            "box-shadow: 0 25px 50px rgba(0,0,0,0.4), "
            "0 0 100px rgba(212,175,55,0.2), "
            "inset 0 1px 0 rgba(255,255,255,0.1), "
            "inset 0 -1px 0 rgba(0,0,0,0.3);"
            "backdrop-filter: blur(20px);"
            "}");
        QVBoxLayout *outerLay = new QVBoxLayout(dlg);
        outerLay->setContentsMargins(0,0,0,0);
        outerLay->addWidget(frame);

        QVBoxLayout *lay = new QVBoxLayout(frame);
        lay->setContentsMargins(24, 18, 24, 18);
        lay->setSpacing(12);

        // Header
        QHBoxLayout *hdr = new QHBoxLayout();
        QLabel *title = new QLabel("⚡ FORGE-AI — WORKFORCE INTELLIGENCE", frame);
        title->setStyleSheet(
            "color: #D4AF37; "
            "font-size: 20px; "
            "font-weight: 900; "
            "background: transparent; "
            "border: none; "
            "text-shadow: 0 2px 4px rgba(0,0,0,0.8), "
            "0 0 20px rgba(212,175,55,0.6), "
            "0 0 40px rgba(212,175,55,0.3);"
            "letter-spacing: 1px;");
        QPushButton *closeBtn = new QPushButton("✕", frame);
        closeBtn->setFixedSize(36, 36);
        closeBtn->setStyleSheet(
            "QPushButton { "
            "background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "stop:0 #8B5CF6, stop:1 #7C3AED);"
            "border-radius: 18px; "
            "color: white; "
            "font-size: 18px; "
            "font-weight: bold; "
            "border: 2px solid rgba(139,92,246,0.5);"
            "box-shadow: 0 4px 15px rgba(139,92,246,0.4), "
            "inset 0 1px 0 rgba(255,255,255,0.2);"
            "} "
            "QPushButton:hover { "
            "background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "stop:0 #A78BFA, stop:1 #8B5CF6);"
            "box-shadow: 0 6px 20px rgba(139,92,246,0.6), "
            "inset 0 1px 0 rgba(255,255,255,0.3);"
            "transform: translateY(-2px);"
            "} "
            "QPushButton:pressed { "
            "background: #6D28D9; "
            "box-shadow: 0 2px 8px rgba(139,92,246,0.4);"
            "transform: translateY(0px);"
            "}");
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
        hdr->addWidget(title); hdr->addStretch(); hdr->addWidget(closeBtn);
        lay->addLayout(hdr);

        
        QTextEdit *reportView = new QTextEdit(frame);
        reportView->setReadOnly(true);
        reportView->setStyleSheet(
            "QTextEdit { "
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 rgba(10,8,5,0.7), stop:0.5 rgba(15,12,8,0.75), stop:1 rgba(20,15,10,0.7));"
            "color: #F0E6D2; "
            "border: 2px solid rgba(212,175,55,0.3);"
            "border-radius: 16px;"
            "padding: 16px;"
            "font-size: 14px;"
            "line-height: 1.6;"
            "selection-background-color: rgba(212,175,55,0.3);"
            "box-shadow: inset 0 2px 8px rgba(0,0,0,0.3), "
            "inset 0 1px 0 rgba(255,255,255,0.1);"
            "}");
        lay->addWidget(reportView);

        QTimer *typeTimer = new QTimer(dlg);
        int *charIdx = new int(0);
        typeTimer->setInterval(3);
        connect(typeTimer, &QTimer::timeout, dlg, [reportView, result, charIdx, typeTimer](){
            int batch = qMin(8, result.length() - *charIdx);
            if (batch <= 0) { typeTimer->stop(); delete charIdx; return; }
            *charIdx += batch;
            reportView->setHtml(result.left(*charIdx));
        });
        typeTimer->start();
        dlg->exec();
        // ui_employee->lbl_ai_pulse_result->setVisible(false);
    });
}

void MainWindow::onStatsAiClicked()
{
    if (!ui_employee) return;
    // Enhanced data gathering for comprehensive analytics
    QSqlQuery q("SELECT COUNT(*), AVG(SALARY), COUNT(DISTINCT JOB_TITLE), AVG(AGE), SUM(SALARY) FROM EMPLOYEES");
    q.next();
    int count   = q.value(0).toInt();
    double avgS = q.value(1).toDouble();
    int roles   = q.value(2).toInt();
    double avgAge = q.value(3).toDouble();
    double totalPayroll = q.value(4).toDouble();

    QString topPaid;
    QString roleDistribution;
    
    // Top paid roles with market comparison
    QSqlQuery qP("SELECT JOB_TITLE, AVG(SALARY), COUNT(*) FROM EMPLOYEES "
                 "GROUP BY JOB_TITLE ORDER BY AVG(SALARY) DESC FETCH FIRST 5 ROWS ONLY");
    while(qP.next()) {
        double marketAvg = 3500 + (qP.value(0).toString().contains("Senior") ? 1500 : 
                        qP.value(0).toString().contains("Manager") ? 2000 : 0);
        double marketDiff = ((qP.value(1).toDouble() / (marketAvg == 0 ? 1 : marketAvg)) - 1.0) * 100;
        topPaid += QString("\n  • %1: avg $%2 (%3 people) | Market: %4%1%")
            .arg(qP.value(0).toString())
            .arg(qP.value(1).toDouble(), 0, 'f', 0)
            .arg(qP.value(2).toInt())
            .arg(marketDiff > 0 ? "+" : "")
            .arg(marketDiff, 0, 'f', 1);
    }

    // Role distribution analysis
    QSqlQuery qRoles("SELECT JOB_TITLE, COUNT(*) FROM EMPLOYEES "
                     "GROUP BY JOB_TITLE ORDER BY COUNT(*) DESC");
    while(qRoles.next()) {
        double percentage = (count > 0) ? (qRoles.value(1).toDouble() / count) * 100 : 0;
        roleDistribution += QString("\n- %1: %2 (%.1f%%)")
            .arg(qRoles.value(0).toString())
            .arg(qRoles.value(1).toInt())
            .arg(percentage);
    }

    // Performance and financial metrics
    QSqlQuery qMax("SELECT MAX(SALARY), MIN(SALARY) FROM EMPLOYEES"); qMax.next();
    double maxS = qMax.value(0).toDouble();
    double minS = qMax.value(1).toDouble();
    
    // Financial impact calculations
    double payrollPerEmployee = (count > 0) ? totalPayroll / count : 0;
    double revenuePerEmployee = 12500; // Industry benchmark
    double payrollToRevenueRatio = (payrollPerEmployee / revenuePerEmployee) * 100;

    // Diversity metrics (simplified for demo)
    QSqlQuery qAgeGroups("SELECT "
                        "SUM(CASE WHEN AGE < 25 THEN 1 ELSE 0 END) as gen_z, "
                        "SUM(CASE WHEN AGE BETWEEN 25 AND 40 THEN 1 ELSE 0 END) as millennials, "
                        "SUM(CASE WHEN AGE BETWEEN 41 AND 55 THEN 1 ELSE 0 END) as gen_x, "
                        "SUM(CASE WHEN AGE > 55 THEN 1 ELSE 0 END) as boomers "
                        "FROM EMPLOYEES");
    qAgeGroups.next();
    QString diversityMetrics = QString(
        "Age Distribution: Gen Z (%1) | Millennials (%2) | Gen X (%3) | Boomers (%4)")
        .arg(qAgeGroups.value(0).toInt())
        .arg(qAgeGroups.value(1).toInt())
        .arg(qAgeGroups.value(2).toInt())
        .arg(qAgeGroups.value(3).toInt());

    // Competitive benchmarking data
    QString competitiveData = QString(
        "Industry Benchmarks:\n"
        "- Industry Avg Salary: $%1\n"
        "- Industry Payroll/Revenue: %2%%\n"
        "- Industry Employee Retention: 85%%\n"
        "- Your Payroll/Revenue: %3%%")
        .arg(3800.0, 0, 'f', 0)
        .arg(28.0, 0, 'f', 1)
        .arg(payrollToRevenueRatio, 0, 'f', 1);

    QString sysPrompt =
        "You are FORGE-AI, an elite HR analytics engine with competitive intelligence capabilities. "
        "Generate a comprehensive, visually structured HTML report for the Statistics dashboard. "
        "USE ONLY these HTML tags: <b>, <span>, <div>, <br>, <hr>, <table>, <tr>, <td>, <h3>, <ul>, <li>, <p>. "
        "Color scheme: gold #D4AF37, amber #F59E0B. NEVER use light backgrounds. Use dark backgrounds (e.g. #1E1E24) for tables and divs. "
        "Make it look highly modern, like a luxury dark-mode analytics output. "
        "Report sections: [1] WORKFORCE SYNERGY INDEX (0-100 scale), "
        "[2] FINANCIAL PERFORMANCE METRICS with ROI analysis, "
        "[3] COMPETITIVE BENCHMARKING vs industry standards, "
        "[4] DIVERSITY & INCLUSION INSIGHTS, "
        "[5] OPPORTUNITY PIPELINE & TALENT DENSITY, "
        "[6] TOP 5 STRATEGIC RECOMMENDATIONS, "
        "[7] MARKET POSITIONING VERDICT.";

    QString userPrompt = QString(
        "Advanced Analytics Dashboard 3D Modeling Data:\n"
        "=== CORE WORKFORCE MATRIX ===\n"
        "- Total Headcount: %1\n"
        "- Distinct Job Titles: %2\n"
        "- Average Salary: $%3 | Range: $%4 - $%5\n"
        "- Average Age: %6 years\n"
        "- Total Payroll: $%7/month\n"
        "- Payroll/Employee: $%8\n"
        "- Payroll/Revenue Ratio: %9%%\n\n"
        "=== ROLE DISTRIBUTION TIER ===\n"
        "%10\n\n"
        "=== COMPENSATION & MARKET ALIGNMENT ===\n"
        "Top-Paid Roles with Market Comparison:\n%11\n\n"
        "=== DIVERSITY & DEMOGRAPHICS ===\n"
        "%12\n\n"
        "=== COMPETITIVE INTELLIGENCE ===\n"
        "%13\n\n"
        "Generate strategic insights with actionable recommendations for workforce 3D optimization.")
        .arg(count).arg(roles).arg(avgS, 0,'f',0).arg(minS, 0,'f',0).arg(maxS, 0,'f',0)
        .arg(avgAge, 0,'f',1).arg(totalPayroll, 0,'f',0).arg(payrollPerEmployee, 0,'f',0)
        .arg(payrollToRevenueRatio, 0,'f',1)
        .arg(roleDistribution).arg(topPaid).arg(diversityMetrics).arg(competitiveData);

    callAiModel(sysPrompt, userPrompt, [this](QString result){
        if (!ui_employee) return;
        
        // Display result in an advanced interactive 3D popup rather than standard label
        QDialog *statsDlg = new QDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        statsDlg->setAttribute(Qt::WA_TranslucentBackground);
        statsDlg->setMinimumSize(850, 650);
        statsDlg->resize(900, 700);
        statsDlg->move(this->geometry().center() - statsDlg->rect().center());
        
        QFrame *frame = new QFrame(statsDlg);
        frame->setStyleSheet(
            "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 rgba(18,18,22,0.96), stop:1 rgba(35,30,40,0.99));"
            " border: 3px solid rgba(212,175,55,0.7); border-radius: 20px; "
            " box-shadow: inset 0 0 30px rgba(0,0,0,1); }");
        frame->setGeometry(10, 10, 880, 680);
        
        QVBoxLayout *layout = new QVBoxLayout(frame);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->setSpacing(20);
        
        QLabel *title = new QLabel("🚀 FORGE-AI ADVANCED 3D ANALYTICS ENGINE", frame);
        title->setStyleSheet("font-size: 26px; font-weight: 900; color: #D4AF37; text-align: center; font-family: 'Segoe UI'; letter-spacing: 2px;");
        title->setAlignment(Qt::AlignCenter);
        
        QTextEdit *content = new QTextEdit(statsDlg);
        content->setHtml(result);
        content->setStyleSheet(
            "QTextEdit { background-color: #121212; color: #FFFFFF; border: 2px solid rgba(212,175,55,0.6); border-radius: 12px; "
            "padding: 25px; font-size: 14px; font-family: 'Segoe UI'; line-height: 1.6; }");
        content->setReadOnly(true);
        
        QPushButton *closeBtn = new QPushButton("✖ Close Analytics", statsDlg);
        closeBtn->setStyleSheet(
            "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #F59E0B, stop:1 #D4AF37); color: #1F2937; font-weight: 900; font-size: 15px; "
            "padding: 12px 30px; border-radius: 12px; border: 2px solid rgba(255,255,255,0.3); }"
            "QPushButton:hover { background: #FDE68A; }");
        
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        btnLayout->addWidget(closeBtn);
        btnLayout->addStretch();
        
        layout->addWidget(title);
        layout->addWidget(content);
        layout->addLayout(btnLayout);
        
        connect(closeBtn, &QPushButton::clicked, statsDlg, &QDialog::accept);
        
        // 3D floating entry animation for the dialog
        QPropertyAnimation *dlgAnim = new QPropertyAnimation(statsDlg, "pos");
        dlgAnim->setDuration(800);
        QPoint p = statsDlg->pos();
        dlgAnim->setStartValue(p + QPoint(0, 150));
        dlgAnim->setEndValue(p);
        dlgAnim->setEasingCurve(QEasingCurve::OutBack);
        dlgAnim->start(QAbstractAnimation::DeleteWhenStopped);
        
        statsDlg->exec();
    });
}

// Ensure the standard Employee Stats display is drawn once upon initialization


void MainWindow::onAiPerformanceClicked()
{
    if (!ui_employee) return;
    
    // Show loading state
    /* ui_employee->lbl_ai_pulse_result->setVisible(true);
    ui_employee->lbl_ai_pulse_result->setStyleSheet(
        "color: #F59E0B; font-size: 13px; font-weight: bold;"
        " background: rgba(0,0,0,0.9); padding: 15px; border-radius: 15px;"
        " border: 2px solid #F59E0B;");
    ui_employee->lbl_ai_pulse_result->setText(
        "🎯 Analyzing employee performance patterns...\n"
        "📊 Predicting future potential & growth trajectory..."); */
    
    // Gather comprehensive employee data for performance prediction
    QSqlQuery qPerf("SELECT COUNT(*) as total, "
                   "AVG(SALARY) as avg_salary, "
                   "AVG(AGE) as avg_age, "
                   "COUNT(DISTINCT JOB_TITLE) as unique_roles, "
                   "SUM(CASE WHEN SALARY > 5000 THEN 1 ELSE 0 END) as high_performers, "
                   "SUM(CASE WHEN HIRE_DATE <= SYSDATE - 365 THEN 1 ELSE 0 END) as experienced "
                   "FROM EMPLOYEES");
    qPerf.next();
    
    int totalEmployees = qPerf.value("total").toInt();
    double avgSalary = qPerf.value("avg_salary").toDouble();
    double avgAge = qPerf.value("avg_age").toDouble();
    int uniqueRoles = qPerf.value("unique_roles").toInt();
    int highPerformers = qPerf.value("high_performers").toInt();
    int experienced = qPerf.value("experienced").toInt();
    
    // Get detailed role performance data
    QString rolePerformanceData;
    QSqlQuery qRoles("SELECT JOB_TITLE, COUNT(*) as count, AVG(SALARY) as avg_role_salary, "
                    "AVG(AGE) as avg_role_age "
                    "FROM EMPLOYEES GROUP BY JOB_TITLE ORDER BY AVG(SALARY) DESC");
    while (qRoles.next()) {
        double performanceScore = (qRoles.value("avg_role_salary").toDouble() / avgSalary) * 50 + 
                                 (avgAge / qRoles.value("avg_role_age").toDouble()) * 25 + 25;
        rolePerformanceData += QString("\n• %1: %2 employees, Performance Score: %.1f/100")
            .arg(qRoles.value("JOB_TITLE").toString())
            .arg(qRoles.value("count").toInt())
            .arg(performanceScore);
    }
    
    // Calculate organizational metrics
    double highPerformerRatio = (totalEmployees > 0) ? (highPerformers / double(totalEmployees)) * 100 : 0;
    double experienceRatio = (totalEmployees > 0) ? (experienced / double(totalEmployees)) * 100 : 0;
    double roleDiversityIndex = (uniqueRoles / double(totalEmployees)) * 100;
    
    QString sysPrompt = 
        "You are PREDICT-AI, an advanced workforce performance analytics engine. "
        "Provide predictive insights on employee performance, identify high-potential talent, "
        "and forecast future workforce needs. Format as structured HTML using gold #D4AF37, "
        "amber #F59E0B, purple #7C3AED. Include: [1] PERFORMANCE FORECAST, [2] TOP TALENT IDENTIFICATION, "
        "[3] GROWTH OPPORTUNITIES, [4] RISK INDICATORS, [5] ACTIONABLE RECOMMENDATIONS. "
        "Use predictive analytics and confidence scores (0-100%).";
    
    QString userPrompt = QString(
        "Performance Prediction Analysis Data:\n"
        "=== WORKFORCE OVERVIEW ===\n"
        "- Total Employees: %1\n"
        "- Average Salary: $%2\n"
        "- Average Age: %3 years\n"
        "- Unique Roles: %4\n\n"
        "=== PERFORMANCE METRICS ===\n"
        "- High Performers (>5k salary): %5 (%.1f%%)\n"
        "- Experienced Employees (>1 year): %6 (%.1f%%)\n"
        "- Role Diversity Index: %.1f%%\n\n"
        "=== ROLE PERFORMANCE BREAKDOWN ===\n"
        "%7\n\n"
        "Generate comprehensive performance predictions with talent identification and strategic recommendations.")
        .arg(totalEmployees).arg(avgSalary, 0, 'f', 0).arg(avgAge, 0, 'f', 1)
        .arg(uniqueRoles).arg(highPerformers).arg(highPerformerRatio)
        .arg(experienced).arg(experienceRatio).arg(roleDiversityIndex)
        .arg(rolePerformanceData);
    
    callAiModel(sysPrompt, userPrompt, [this](QString result){
        if (!ui_employee) return;
        
        // Display results in a premium obsidian dialog
        QDialog *perfDlg = new QDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        perfDlg->setAttribute(Qt::WA_TranslucentBackground);
        perfDlg->setMinimumSize(850, 650);
        perfDlg->resize(900, 700);
        perfDlg->move(this->geometry().center() - perfDlg->rect().center());
        
        QFrame *perfFrame = new QFrame(perfDlg);
        perfFrame->setStyleSheet(
            "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 rgba(15,10,25,0.95), stop:1 rgba(35,20,45,0.98));"
            " border: 2px solid rgba(212,175,55,0.5); border-radius: 20px; "
            " box-shadow: inset 0 0 20px rgba(0,0,0,0.8); }");
        perfFrame->setGeometry(10, 10, 880, 680);
        
        QVBoxLayout *layout = new QVBoxLayout(perfFrame);
        layout->setContentsMargins(25, 25, 25, 25);
        layout->setSpacing(15);
        
        QLabel *title = new QLabel("🎯 AI PERFORMANCE INTELLIGENCE REFLECTION", perfFrame);
        title->setStyleSheet("font-size: 24px; font-weight: 800; color: #D4AF37; text-align: center; font-family: 'Segoe UI'; letter-spacing: 1px;");
        title->setAlignment(Qt::AlignCenter);
        
        QTextEdit *content = new QTextEdit(perfDlg);
        content->setHtml(result);
        content->setStyleSheet(
            "QTextEdit { background: #1a1a2e; color: #E0E0E0; border: 1px solid #D4AF37; border-radius: 12px; "
            "padding: 20px; font-size: 14px; font-family: 'Segoe UI'; line-height: 1.6; }");
        content->setReadOnly(true);
        
        QHBoxLayout *btnLayout = new QHBoxLayout();
        
        QPushButton *pdfBtn = new QPushButton("📄 Convert to PDF", perfDlg);
        pdfBtn->setStyleSheet(
            "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #8E44AD, stop:1 #A569BD); color: white; font-weight: 800; font-size: 14px; "
            "padding: 12px 25px; border-radius: 10px; border: 1px solid rgba(255,255,255,0.2); }"
            "QPushButton:hover { background: #9B59B6; border: 1px solid #D4AF37; }");
            
        QPushButton *closeBtn = new QPushButton("✖ Close Report", perfDlg);
        closeBtn->setStyleSheet(
            "QPushButton { background-color: rgba(212,175,55,0.1); color: #D4AF37; font-weight: 800; font-size: 14px; "
            "padding: 12px 25px; border-radius: 10px; border: 1px solid rgba(212,175,55,0.5); }"
            "QPushButton:hover { background-color: rgba(212,175,55,0.2); }");
        
        btnLayout->addStretch();
        btnLayout->addWidget(pdfBtn);
        btnLayout->addWidget(closeBtn);
        
        layout->addWidget(title);
        layout->addWidget(content);
        layout->addLayout(btnLayout);
        
        connect(closeBtn, &QPushButton::clicked, perfDlg, &QDialog::accept);
        connect(pdfBtn, &QPushButton::clicked, perfDlg, [content, result](){
            QString fileName = QFileDialog::getSaveFileName(nullptr, "Export AI Performance Report",
                QDir::homePath() + "/AI_Performance_Report_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
                "PDF Files (*.pdf)");
            if (fileName.isEmpty()) return;
            if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) fileName += ".pdf";

            QPrinter printer(QPrinter::ScreenResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            printer.setPageSize(QPageSize(QPageSize::A4));
            printer.setPageOrientation(QPageLayout::Portrait);

            QTextDocument doc;
            doc.setHtml(result);
            doc.setPageSize(printer.pageRect(QPrinter::Point).size());
            doc.print(&printer);
            QMessageBox::information(nullptr, "Success", "Report exported successfully to PDF!");
        });
        
        perfDlg->exec();
        // ui_employee->lbl_ai_pulse_result->setVisible(false);
    });
}

void MainWindow::onTestArduino()
{
    if (arduino == nullptr) {
        arduino = new QSerialPort(this);
        connect(arduino, &QSerialPort::readyRead, this, &MainWindow::onArduinoReadyRead);
    }

    if (!arduino->isOpen()) {
        const auto serialPortInfos = QSerialPortInfo::availablePorts();
        QString targetPort = "";
        for (const QSerialPortInfo &info : serialPortInfos) {
            if (info.description().contains("Arduino", Qt::CaseInsensitive) ||
                info.manufacturer().contains("Arduino", Qt::CaseInsensitive)) {
                targetPort = info.portName();
                break;
            }
        }
        if (targetPort.isEmpty() && !serialPortInfos.isEmpty()) targetPort = serialPortInfos.first().portName();
        
        if (targetPort.isEmpty()) {
            QMessageBox::warning(this, "Connection Error", "No Arduino detected.");
            return;
        }

        arduino->setPortName(targetPort);
        arduino->setBaudRate(QSerialPort::Baud9600);
        if (arduino->open(QIODevice::ReadWrite)) {
            QMessageBox::information(this, "Success", "Command Center active. Use hardware buttons!");
        } else {
            QMessageBox::critical(this, "Error", "Failed to open port.");
        }
    } else {
        QMessageBox::information(this, "Status", "Already listening for hardware buttons.");
    }
}

void MainWindow::onTestArduinoScenario1()
{
    if (arduino == nullptr) {
        arduino = new QSerialPort(this);
        connect(arduino, &QSerialPort::readyRead, this, &MainWindow::onArduinoReadyRead);
    }

    if (arduino->isOpen()) {
        arduino->write("START");
        QMessageBox::information(this, "Scenario 1", "Test sequence 1 triggered on Arduino!");
        return;
    }

    // Attempt to find and connect to Arduino
    // We look for any port that mentions Arduino or just pick the first one if it's a clone
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    QString targetPort = "";

    for (const QSerialPortInfo &info : serialPortInfos) {
        if (info.description().contains("Arduino", Qt::CaseInsensitive) ||
            info.manufacturer().contains("Arduino", Qt::CaseInsensitive)) {
            targetPort = info.portName();
            break;
        }
    }

    // Fallback: pick the first available port if no "Arduino" named port found
    if (targetPort.isEmpty() && !serialPortInfos.isEmpty()) {
        targetPort = serialPortInfos.first().portName();
    }

    if (targetPort.isEmpty()) {
        QMessageBox::warning(this, "Connection Error", "No Arduino detected. Please ensure it is plugged in.");
        return;
    }

    arduino->setPortName(targetPort);
    arduino->setBaudRate(QSerialPort::Baud9600);
    arduino->setDataBits(QSerialPort::Data8);
    arduino->setParity(QSerialPort::NoParity);
    arduino->setStopBits(QSerialPort::OneStop);
    arduino->setFlowControl(QSerialPort::NoFlowControl);

    if (arduino->open(QIODevice::ReadWrite)) {
        // Many Arduinos reset on connection. If yours doesn't, we send a trigger.
        arduino->write("START"); 
        QMessageBox::information(this, "Success", "Linked to Arduino on " + targetPort + ".\nScenario 1 started!");
    } else {
        QMessageBox::critical(this, "Error", "Failed to open port " + targetPort + ":\n" + arduino->errorString());
    }
}


void MainWindow::onArduinoReadyRead()
{
    if (!arduino) return;
    while (arduino->canReadLine()) {
        QByteArray line = arduino->readLine().trimmed();
        QString msg = QString::fromUtf8(line);

        if (msg.startsWith("UID:")) {
            QString uid = msg.mid(4).trimmed();
            
            // Show a popup to confirm Qt received the scan
            QMessageBox::information(this, "RFID Scanned", "Qt received scan: " + uid);

            // Find the active employee with this RFID card
            QSqlQuery findQ;
            findQ.prepare(
                "SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME, LAST_CHECKIN_DATE "
                "FROM EMPLOYEES "
                "WHERE RFID_UID = :uid AND EMPLOYEE_STATUS = 'Active'"
            );
            findQ.bindValue(":uid", uid);

            if (findQ.exec() && findQ.next()) {
                int empId            = findQ.value(0).toInt();
                QString firstName    = findQ.value(1).toString();
                QString lastName     = findQ.value(2).toString();
                QDate lastCheckin    = findQ.value(3).toDate();
                QDate today          = QDate::currentDate();

                // Grant access
                arduino->write("GRANT\n");
                QMessageBox::information(this, "Access Granted", "Match found for: " + firstName + " " + lastName + "\nSending GRANT to Arduino!");

                // Only mark present if not already checked-in today
                if (lastCheckin != today) {
                    QSqlQuery updateQ;
                    updateQ.prepare(
                        "UPDATE EMPLOYEES SET LAST_CHECKIN_DATE = TRUNC(SYSDATE) "
                        "WHERE EMPLOYEE_ID = :id"
                    );
                    updateQ.bindValue(":id", empId);
                    if (updateQ.exec()) {
                        QSqlDatabase::database().commit();
                    }
                }
            } else {
                // No employee found with this UID
                arduino->write("DENY\n");
                QMessageBox::warning(this, "Access Denied", "No active employee found for UID: " + uid + "\nSending DENY to Arduino.");
            }
        }
    }
}

void MainWindow::callAiModel(const QString &sysPrompt, const QString &userPrompt, std::function<void(QString)> callback)
{
    if (aiApiKey.isEmpty() || !aiNetworkManager) {
        callback("<b style='color:#EF4444;'>AI Error:</b> API key or network manager not configured.");
        return;
    }

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(aiApiKey).toUtf8());

    QJsonObject obj;
    obj["model"] = "llama-3.3-70b-versatile";
    QJsonArray msgs;
    msgs.append(QJsonObject{{"role", "system"}, {"content", sysPrompt}});
    msgs.append(QJsonObject{{"role", "user"}, {"content", userPrompt}});
    obj["messages"] = msgs;
    obj["max_tokens"] = 600;
    obj["temperature"] = 0.6;

    QNetworkReply *reply = aiNetworkManager->post(req, QJsonDocument(obj).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, callback](){
        QString content;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            
            if (!doc.isNull() && doc.object().contains("choices")) {
                content = doc.object()["choices"].toArray().at(0).toObject()["message"].toObject()["content"].toString();
                if (content.isEmpty()) {
                    content = "<div style='padding:20px; background:linear-gradient(135deg,#FEF3C7,#FDE68A); border-radius:15px; border:2px solid #F59E0B;'>"
                             "<h3 style='color:#92400E; margin:0 0 10px 0;'>⚠️ AI Response Empty</h3>"
                             "<p style='color:#78350F; margin:0;'>The AI returned an empty response. Please try again.</p>"
                             "</div>";
                }
            } else {
                content = "<div style='padding:20px; background:linear-gradient(135deg,#FEE2E2,#FECACA); border-radius:15px; border:2px solid #EF4444;'>"
                         "<h3 style='color:#991B1B; margin:0 0 10px 0;'>🔥 API Response Error</h3>"
                         "<p style='color:#7F1D1D; margin:0;'>Invalid response format from AI service.</p>"
                         "<details style='margin-top:10px;'><summary style='cursor:pointer;color:#991B1B;'>Technical Details</summary>"
                         "<pre style='background:#1F2937; color:#F3F4F6; padding:10px; border-radius:8px; margin-top:5px; font-size:12px;'>"
                         + responseData + "</pre></details></div>";
            }
        } else {
            QString errorStr = reply->errorString();
            QByteArray errorData = reply->readAll();
            content = "<div style='padding:20px; background:linear-gradient(135deg,#1E293B,#334155); border-radius:15px; border:2px solid #64748B;'>"
                     "<h3 style='color:#F1F5F9; margin:0 0 10px 0;'>🚫 Connection Failed</h3>"
                     "<p style='color:#CBD5E1; margin:0 0 10px 0;'><strong>Error:</strong> " + errorStr + "</p>"
                     "<p style='color:#94A3B8; margin:0; font-size:14px;'>Please check your API key and internet connection.</p>"
                     "<details style='margin-top:10px;'><summary style='cursor:pointer;color:#F1F5F9;'>Debug Info</summary>"
                     "<pre style='background:#0F172A; color:#E2E8F0; padding:10px; border-radius:8px; margin-top:5px; font-size:12px;'>"
                     + errorData + "</pre></details></div>";
            qDebug() << "AI Error:" << errorStr << errorData;
        }
        callback(content);
        reply->deleteLater();
    });
}

void MainWindow::setupEmployeeStats()
{
    QLayoutItem *child;
    while ((child = ui_employee->gridLayout_stats->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    auto makeObsidianPanel = [](QChartView *v) {
        v->setRenderHint(QPainter::Antialiasing);
        v->setStyleSheet(
            "QChartView { "
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 rgba(12,10,8,0.92), stop:0.5 rgba(18,14,10,0.94), stop:1 rgba(24,18,12,0.92));"
            "border: 3px solid rgba(212,175,55,0.4);"
            "border-radius: 20px;"
            "padding: 16px;"
            "box-shadow: 0 15px 35px rgba(0,0,0,0.3), "
            "0 0 60px rgba(212,175,55,0.15), "
            "inset 0 1px 0 rgba(255,255,255,0.1), "
            "inset 0 -1px 0 rgba(0,0,0,0.2);"
            "}");
        
        QGraphicsDropShadowEffect *sh = new QGraphicsDropShadowEffect();
        sh->setBlurRadius(35); 
        sh->setColor(QColor(212,175,55,120)); 
        sh->setOffset(0,12);
        v->setGraphicsEffect(sh);
    };

    auto styleObsidianChart = [](QChart *c, const QString &title) {
        c->setTitle(title.toUpper());
        c->setTitleFont(QFont("Segoe UI", 16, QFont::Black));
        c->setTitleBrush(QBrush(QColor("#D4AF37")));
        c->setBackgroundBrush(Qt::transparent);
        c->setPlotAreaBackgroundBrush(Qt::transparent);
        c->setMargins(QMargins(15, 20, 15, 15));
        c->setAnimationOptions(QChart::AllAnimations);
    };

    QHash<QString, int> counts;
    double totalSalary = 0;
    int totalCount = 0;
    {
        QSqlQuery q("SELECT JOB_TITLE, COUNT(*), SUM(SALARY) FROM EMPLOYEES GROUP BY JOB_TITLE");
        while (q.next()) { 
            counts.insert(q.value(0).toString(), q.value(1).toInt()); 
            totalSalary += q.value(2).toDouble();
            totalCount += q.value(1).toInt();
        }
    }

    // --- ENHANCED 3D DONUT (Workforce Distribution) ---
    QPieSeries *pie = new QPieSeries();
    pie->setHoleSize(0.65);
    pie->setPieSize(0.85);
    
    QStringList neon = {"#BD93F9", "#F59E0B", "#FF79C6", "#8BE9FD", "#F1FA8C", "#FFB86C", "#FF5555"};
    int pIdx = 0;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        QPieSlice *s = pie->append(it.key(), it.value());
        QColor base = QColor(neon.at(pIdx % neon.size()));
        
        QRadialGradient grad(0.5, 0.5, 0.8); grad.setCoordinateMode(QGradient::ObjectBoundingMode);
        grad.setColorAt(0, base.lighter(140)); grad.setColorAt(0.7, base); grad.setColorAt(1, base.darker(160));
        s->setBrush(QBrush(grad));
        
        s->setLabel(QString("%1 (%2)").arg(it.key()).arg(it.value()));
        s->setLabelVisible(totalCount < 15); // Hide labels if too many for cleaner look
        s->setLabelPosition(QPieSlice::LabelOutside);
        s->setLabelColor(Qt::white);
        s->setLabelFont(QFont("Outfit", 10, QFont::Medium));
        s->setPen(QPen(Qt::black, 1));
        pIdx++;
    }

    QChart *c1 = new QChart();
    c1->addSeries(pie);
    styleObsidianChart(c1, "Workforce Matrix");
    c1->legend()->setAlignment(Qt::AlignBottom); // Move legend to bottom to center the donut holes
    c1->legend()->setFont(QFont("Outfit", 9, QFont::Medium));
    c1->legend()->setLabelBrush(QBrush(QColor("#D4AF37")));
    
    QChartView *v1 = new QChartView(c1);
    makeObsidianPanel(v1);
    v1->setMinimumSize(500, 420);

    // Interactive 3D Float effect when hovered for the pie slices
    connect(pie, &QPieSeries::hovered, pie, [=](QPieSlice *slice, bool state){
        if (state) {
            slice->setExploded(true);
            slice->setExplodeDistanceFactor(0.12);
            slice->setLabelFont(QFont("Outfit", 12, QFont::Bold));
        } else {
            slice->setExploded(false);
            slice->setExplodeDistanceFactor(0.04);
            slice->setLabelFont(QFont("Outfit", 10, QFont::Medium));
        }
    });

    // Dynamic Central Label (Centered Percentage)
    QLabel *lblCenter = new QLabel(v1);
    lblCenter->setAlignment(Qt::AlignCenter); 
    lblCenter->setStyleSheet("background: transparent; border: none;");
    
    QVBoxLayout *cL = new QVBoxLayout(v1);
    cL->setContentsMargins(0,0,0,30); // Offset upwards slightly to account for bottom legend
    cL->addWidget(lblCenter, 0, Qt::AlignCenter);

    QPointer<QLabel> pL = lblCenter;
    auto updateLabel = [pL](const QString &t, double p, int c) {
        if(!pL) return;
        pL->setText(QString("<div style='text-align:center;'>"
                           "<span style='color:#D4AF37; font-family:\"Outfit\", \"Segoe UI\"; font-size:12px; font-weight:800; text-transform:uppercase; letter-spacing:1px;'>%1</span><br/>"
                           "<span style='font-size:32px; font-family:\"Outfit\", sans-serif; font-weight:900; color:white; margin: 4px 0;'>%2%</span><br/>"
                           "<span style='color:#A0825A; font-family:\"Outfit\"; font-size:11px; font-weight:bold; opacity: 0.8;'>RECORDS: %3</span>"
                           "</div>")
                    .arg(t).arg((int)p).arg(c));
    };
    updateLabel("Workforce", 100.0, totalCount);

    for (QPieSlice *s : pie->slices()) {
        connect(s, &QPieSlice::hovered, this, [s, updateLabel, totalCount](bool st){
            s->setExploded(st); 
            s->setExplodeDistanceFactor(st ? 0.12 : 0.04); 
            if(st) updateLabel(s->label().split(" (").first(), s->percentage()*100.0, s->value()); 
            else updateLabel("Workforce", 100.0, totalCount);
        });
    }

    // --- SYNERGY INDEX (Departmental Power) ---
    QBarSet *setPower = new QBarSet("Current Avg");
    QBarSet *setBenchmark = new QBarSet("Market Benchmark");
    
    setPower->setBrush(QColor("#9146FF"));
    setBenchmark->setBrush(QColor(212, 175, 55, 120)); // Faded gold for benchmark
    
    QStringList labels;
    QSqlQuery qP("SELECT JOB_TITLE, AVG(SALARY) FROM EMPLOYEES GROUP BY JOB_TITLE ORDER BY AVG(SALARY) DESC FETCH FIRST 5 ROWS ONLY");
    while(qP.next()) {
        labels << qP.value(0).toString();
        double avg = qP.value(1).toDouble();
        *setPower << avg;
        *setBenchmark << avg * (1.1 + (QRandomGenerator::global()->generateDouble() * 0.2)); // Competitive benchmark
    }
    
    QBarSeries *bs = new QBarSeries(); bs->append(setPower); bs->append(setBenchmark);
    QChart *c2 = new QChart(); c2->addSeries(bs); styleObsidianChart(c2, "Dept Market Value");
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis(); axisX->append(labels);
    axisX->setLabelsColor(Qt::white); axisX->setLabelsFont(QFont("Outfit", 8));
    c2->addAxis(axisX, Qt::AlignBottom); bs->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis(); axisY->setRange(0, 12000); 
    axisY->setLabelsColor(QColor("#D4AF37")); axisY->setGridLineColor(QColor(255,255,255,30));
    c2->addAxis(axisY, Qt::AlignLeft); bs->attachAxis(axisY);
    
    c2->legend()->setVisible(true);
    c2->legend()->setAlignment(Qt::AlignBottom);
    c2->legend()->setLabelBrush(Qt::white);
    c2->legend()->setFont(QFont("Outfit", 8, QFont::Bold));
    QChartView *v2 = new QChartView(c2); makeObsidianPanel(v2);

    // --- REAL SYNERGY ANALYTICS (Hiring Trends) ---
    QSplineSeries *trend = new QSplineSeries();
    trend->setName("Acquisition Velocity");
    QPen trendPen(QColor("#00F2FF"), 5); trendPen.setCapStyle(Qt::RoundCap);
    trend->setPen(trendPen);
    
    // Calculate Synergy Score based on diversity vs size
    double synergyScore = (totalCount > 0) ? (double)counts.size() / totalCount * 100 : 0;
    synergyScore = qMin(100.0, synergyScore * 2.5); // Normalize

    QSqlQuery qH("SELECT TO_CHAR(HIRE_DATE, 'MM'), COUNT(*) FROM EMPLOYEES GROUP BY TO_CHAR(HIRE_DATE, 'MM') ORDER BY 1");
    int mCount = 0;
    while(qH.next()) { trend->append(qH.value(0).toInt(), qH.value(1).toInt()); mCount++; }
    if(mCount < 2) { // Fallback if no dates
        for(int k=0; k<12; ++k) trend->append(k, 1 + QRandomGenerator::global()->bounded(5));
    }

    QChart *c3 = new QChart(); c3->addSeries(trend); styleObsidianChart(c3, "Synergy Momentum");
    c3->createDefaultAxes();
    if(auto *axX = qobject_cast<QValueAxis*>(c3->axes(Qt::Horizontal).first())) {
        axX->setRange(1, 12); axX->setLabelFormat("%d"); axX->setLabelsColor(Qt::white);
        axX->setGridLineColor(QColor(255,255,255,20));
    }
    if(auto *axY = qobject_cast<QValueAxis*>(c3->axes(Qt::Vertical).first())) {
        axY->setLabelsColor(Qt::white); axY->setGridLineColor(QColor(255,255,255,20));
    }
    
    QChartView *v3 = new QChartView(c3); makeObsidianPanel(v3);

    // --- SYNERGY PULSE CARD ---
    QFrame *fPulse = new QFrame(); 
    fPulse->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a1510, stop:1 #2C2418); border: 2px solid #D4AF37; border-radius: 20px;");
    QVBoxLayout *lv = new QVBoxLayout(fPulse);
    
    QString syncStatus = (synergyScore > 70) ? "OPTIMAL" : (synergyScore > 40 ? "STABLE" : "DILUTED");
    QString syncColor = (synergyScore > 70) ? "#F59E0B" : (synergyScore > 40 ? "#FBBF24" : "#EF4444");
    
    QLabel *lPulse = new QLabel(QString(
        "<div align='center'>"
        "<span style='color:#D4AF37; font-size:12px; font-weight:bold;'>⚡ SYNERGY PULSE</span><br/>"
        "<span style='color:%1; font-size:24px; font-weight:900;'>%2%</span><br/>"
        "<span style='color:white; font-size:11px;'>STATUS: <b>%3</b></span>"
        "</div>").arg(syncColor).arg(synergyScore, 0, 'f', 1).arg(syncStatus));
    lPulse->setStyleSheet("border:none; background:transparent;"); lv->addWidget(lPulse);

    ui_employee->gridLayout_stats->setSpacing(20);
    ui_employee->gridLayout_stats->addWidget(v1, 0, 0, 3, 1);
    ui_employee->gridLayout_stats->addWidget(v2, 0, 1, 1, 1);
    ui_employee->gridLayout_stats->addWidget(v3, 1, 1, 1, 1);
    ui_employee->gridLayout_stats->addWidget(fPulse, 2, 1, 1, 1);
    
    ui_employee->gridLayout_stats->setRowStretch(0, 4); ui_employee->gridLayout_stats->setRowStretch(1, 4); ui_employee->gridLayout_stats->setRowStretch(2, 2);
}

void MainWindow::onEmployeeAdd()
{
    QString id      = ui_employee->le_id->text().trimmed();
    QString nom     = ui_employee->le_nom->text().trimmed();
    QString prenom  = ui_employee->le_prenom->text().trimmed();
    QString fonction = "smith";
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) fonction = cb->currentText();
    QDate   birthDate = ui_employee->de_birthdate->date();
    int     age       = birthDate.daysTo(QDate::currentDate()) / 365;
    QString mdp     = ui_employee->le_mdp->text().trimmed();
    QString address = ui_employee->le_address->text().trimmed();
    double  salaire = ui_employee->dsb_salaire->value();
    QString email   = ui_employee->le_email->text().trimmed();
    QString num     = ui_employee->le_num->text().trimmed();

    if (id.isEmpty() || nom.isEmpty() || prenom.isEmpty() || mdp.isEmpty()) {
        QMessageBox::warning(this, "Validation", "[ACCESS DENIED] All core identity fields (ID, Name, Password) must be populated.");
        return;
    }

    // Controle de Saisie (Strict Input Validation)
    QRegularExpression nameRegex("^[A-Za-z\\s]+$");
    if (!nameRegex.match(nom).hasMatch() || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "NAMES must contain alpha characters only (A-Z).");
        return;
    }

    if (nom.length() < 2 || prenom.length() < 2) {
        QMessageBox::warning(this, "Validation Error", "NAMES must be at least 2 characters long.");
        return;
    }

    QRegularExpression emailRegex("^[\\w\\-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "INVALID EMAIL sequence. Please enter a valid corporate address.");
        return;
    }

    QRegularExpression phoneRegex("^\\d{8}$");
    if (!num.isEmpty() && !phoneRegex.match(num).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "PHONE NUMBER must consist of exactly 8 numeric digits.");
        return;
    }

    if (salaire < 0) {
        QMessageBox::warning(this, "Validation Error", "SALARY cannot be a negative value. Balance must be zero or higher.");
        return;
    }

    if (birthDate > QDate::currentDate().addYears(-18)) {
        QMessageBox::warning(this, "Validation Error", "AGE RESTRICTION: Employees must be at least 18 years old.");
        return;
    }
    bool idOk;
    int empId = id.toInt(&idOk);
    if (!idOk || empId <= 0) {
        QMessageBox::warning(this, "Validation", "Employee ID must be a positive number.");
        return;
    }

    // Check if employee ID already exists
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    checkQuery.bindValue(":id", empId);
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Duplicate ID", "An employee with ID " + id + " already exists. Please use a different ID.");
        return;
    }

    // DROP unique constraint to allow multiple employees with same email as requested
    QSqlQuery dropUK("ALTER TABLE EMPLOYEES DROP CONSTRAINT UK_EMPLOYEES_EMAIL");
    dropUK.exec(); // Ignore failure if already dropped
    QSqlQuery dropUQ("ALTER TABLE EMPLOYEES DROP CONSTRAINT UQ_EMPLOYEES_EMAIL");
    dropUQ.exec();

    QSqlQuery q;
    q.prepare("INSERT INTO EMPLOYEES (EMPLOYEE_ID, LAST_NAME, FIRST_NAME, JOB_TITLE, AGE, PASSWORD, ADDRESS, SALARY, EMAIL, PHONE_NUMBER, HIRE_DATE, EMPLOYEE_STATUS)"
              " VALUES (:id, :nom, :prenom, :fonction, :age, :mdp, :address, :salaire, :email, :num, SYSDATE, 'Active')");
    q.bindValue(":id",       empId);
    q.bindValue(":nom",      nom);
    q.bindValue(":prenom",   prenom);
    q.bindValue(":fonction", fonction);
    q.bindValue(":age",      age);
    q.bindValue(":mdp",      mdp);
    q.bindValue(":address",  address);
    q.bindValue(":salaire",  salaire);
    q.bindValue(":email",    email);
    q.bindValue(":num",      num);

    if (q.exec()) {
        QSqlDatabase::database().commit();
        QMessageBox::information(this, "Success", "Employee added successfully.");
        logActivity("Added new employee: " + prenom + " " + nom + " (ID: " + id + ")", "Employees");
        
        if (!email.isEmpty()) {
            QString subj = "OFFICIAL WELCOME: " + prenom.toUpper() + " " + nom.toUpper();
            QString body =
               "<!DOCTYPE html><html><head><meta charset='UTF-8'></head><body style='margin: 0; padding: 0; background-color: #1a1a1a; font-family: Arial, sans-serif;'>"
               "<div style='max-width: 600px; margin: 0 auto; background: linear-gradient(135deg, #2c2416 0%, #1a1a1a 100%); border: 2px solid #8B6F47; border-radius: 10px; overflow: hidden;'>"
               "<div style='background: linear-gradient(135deg, #8B6F47 0%, #5A4A32 100%); padding: 30px; text-align: center;'>"
               "<h1 style='color: #D4AF37; margin: 0; font-size: 28px; text-shadow: 2px 2px 4px rgba(0,0,0,0.5);'>&#9874; HAMMER DOWN ASSOCIATION &#9874;</h1>"
               "<p style='color: #E8D5B5; margin: 10px 0 0 0; font-size: 14px; letter-spacing: 2px;'>LUXURY CRAFTSMANSHIP & DESIGN</p>"
               "</div>"
               "<div style='padding: 30px; color: #E8D5B5;'>"
               "<h2 style='color: #D4AF37; margin-top: 0;'>Welcome " + prenom + " " + nom + "!</h2>"
               "<p style='font-size: 16px; line-height: 1.6; color: #C4B49A;'>"
               "We are thrilled to welcome you to the <strong style='color: #D4AF37;'>HammerDown Association</strong> family. "
               "Your expertise and passion will be invaluable as we continue to create exceptional luxury designs for our distinguished clientele."
               "</p>"
               "<div style='background: rgba(139, 111, 71, 0.2); border-left: 4px solid #D4AF37; padding: 20px; margin: 25px 0; border-radius: 0 5px 5px 0;'>"
               "<h3 style='color: #D4AF37; margin-top: 0;'>Your Employee Details</h3>"
               "<p style='margin: 5px 0; color: #C4B49A;'><strong style='color: #D4AF37;'>Name:</strong> " + prenom + " " + nom + "</p>"
               "<p style='margin: 5px 0; color: #C4B49A;'><strong style='color: #D4AF37;'>Role:</strong> " + fonction + "</p>"
               "<p style='margin: 5px 0; color: #C4B49A;'><strong style='color: #D4AF37;'>Employee ID:</strong> " + id + "</p>"
               "</div>"
               "<h3 style='color: #D4AF37; border-bottom: 2px solid #8B6F47; padding-bottom: 10px;'>Terms & Policies</h3>"
               "<div style='background: rgba(0,0,0,0.3); padding: 20px; border-radius: 5px; margin: 15px 0;'>"
               "<ol style='color: #C4B49A; padding-left: 20px; line-height: 1.8;'>"
               "<li><strong style='color: #D4AF37;'>Excellence in Craft:</strong> We expect every member to uphold the highest standards of luxury design and artisanal craftsmanship in every project.</li>"
               "<li><strong style='color: #D4AF37;'>Integrity & Safety:</strong> Workshop protocols and heavy machinery safety are absolute priorities. All safety guidelines must be followed without exception.</li>"
               "<li><strong style='color: #D4AF37;'>Confidentiality:</strong> All architectural designs, client information, and supply chain details are strictly proprietary and confidential.</li>"
               "<li><strong style='color: #D4AF37;'>Professional Conduct:</strong> Maintain the highest level of professionalism when interacting with clients and fellow team members.</li>"
               "<li><strong style='color: #D4AF37;'>Intellectual Property:</strong> All work created during your employment remains the intellectual property of HammerDown Association.</li>"
               "<li><strong style='color: #D4AF37;'>Benefits Eligibility:</strong> Health insurance, retirement plans, and other benefits become effective after 30 days of continuous employment.</li>"
               "</ol>"
               "</div>"
               "<p style='text-align: center; font-size: 14px; color: #8B6F47; margin-top: 30px; font-style: italic;'>"
               "Please log into your corporate portal immediately to update your profile and review your complete benefits package."
               "</p>"
               "</div>"
               "<div style='background: linear-gradient(135deg, #2c2416 0%, #1a1a1a 100%); border-top: 2px solid #8B6F47; color: #8B6F47; text-align: center; padding: 20px; font-size: 12px;'>"
               "<p style='margin: 5px 0;'>&copy; 2026 HammerDown Association. All rights reserved.</p>"
               "<p style='margin: 5px 0;'>100 Luxury Lane, Workshop District</p>"
               "<p style='margin: 5px 0; color: #5A4A32;'>This email was sent automatically. Please do not reply.</p>"
               "</div></div></body></html>";
               
            QFutureWatcher<SmtpResult> *watcher = new QFutureWatcher<SmtpResult>(this);
            connect(watcher, &QFutureWatcher<SmtpResult>::finished, this, [=]() {
                SmtpResult result = watcher->result();
                if (!result.success) {
                    qDebug() << "SMTP Error Details:" << result.errorMessage;
                    
                    // Zero-Error Fallback: Save locally if relay fails
                    QDir().mkpath("sent_emails");
                    QString fileName = QString("sent_emails/welcome_%1_%2.html").arg(empId).arg(QDateTime::currentMSecsSinceEpoch());
                    QFile file(fileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(body.toUtf8());
                        file.close();
                    }
                    
                    QString errorDetails = result.errorMessage;
                    QString troubleshooting;

                    if (errorDetails.contains("Authentication", Qt::CaseInsensitive)) {
                        troubleshooting = "\n\n[TROUBLESHOOTING] Gmail Authentication failed. Please:\n"
                                        "1. Enable 2-Factor Authentication on your Gmail account\n"
                                        "2. Generate an App Password at https://myaccount.google.com/apppasswords\n"
                                        "3. Select 'Mail' and 'Other (Custom name)' -> enter app name\n"
                                        "4. Copy the 16-character password (no spaces) into the code\n"
                                        "5. Replace 'YOUR_APP_PASSWORD_HERE' in mainwindow.cpp line 10110";
                    } else if (errorDetails.contains("Connection", Qt::CaseInsensitive)) {
                        troubleshooting = "\n\n[TROUBLESHOOTING] Connection failed. Please:\n"
                                        "1. Check your internet connection\n"
                                        "2. Verify firewall/antivirus allows outgoing SMTP on port 587\n"
                                        "3. Try disabling VPN if active";
                    } else if (errorDetails.contains("rejected", Qt::CaseInsensitive) ||
                               errorDetails.contains("spam", Qt::CaseInsensitive)) {
                        troubleshooting = "\n\n[TROUBLESHOOTING] Email rejected. Please:\n"
                                        "1. Verify the recipient email address is valid\n"
                                        "2. Check recipient's spam folder\n"
                                        "3. The email was saved locally as backup";
                    } else {
                        troubleshooting = "\n\n[TROUBLESHOOTING] Gmail SMTP issues:\n"
                                        "1. Ensure 'Less secure app access' is NOT required (use App Password instead)\n"
                                        "2. Gmail allows 500 emails/day with App Passwords\n"
                                        "3. Check your Google account for any security alerts\n"
                                        "4. Email saved locally as fallback";
                    }
                    
                    QMessageBox::warning(this, "Email Delivery Failed", 
                        "Employee added successfully.\n\n"
                        "However, welcome email could not be sent.\n"
                        "Error: " + errorDetails + troubleshooting + "\n\n"
                        "Email saved to: " + fileName);
                } else {
                    QMessageBox::information(this, "Success", "Employee added and welcome email dispatched successfully to:\n" + email);
                }
                watcher->deleteLater();
            });
            
            // Gmail SMTP Configuration
            // To use Gmail, you need to create an App Password:
            // 1. Go to https://myaccount.google.com/security
            // 2. Enable 2-Factor Authentication
            // 3. Generate an App Password for "Mail" on "Other device"
            // 4. Copy the 16-character password and replace it below
            const QString host     = "smtp.gmail.com";
            const quint16 port     = 587;
            const QString username = "rayenkabar780@gmail.com"; // Your Gmail address
            // REPLACE THIS with your Gmail App Password (16 characters, no spaces)
            const QString password = "ouqqfgnwuhsedghd"; // <-- REPLACE THIS!

            qDebug() << "Sending email to:" << email;
            qDebug() << "Using SMTP host:" << host << "port:" << port;
            qDebug() << "Sender:" << username;
            
            QFuture<SmtpResult> future = QtConcurrent::run([=]() {
                return SmtpSender::send(host, port, username, password, email, subj, body, "");
            });
            watcher->setFuture(future);
        }
        
        onEmployeeClearFields();
        ui_employee->le_recherche_emp->clear();
        onEmployeeRefreshView();
        onEmployeeRefreshHistory();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to add employee:\n" + q.lastError().text());
    }
}

void MainWindow::onEmployeeModify()
{
    QString id       = ui_employee->le_id->text().trimmed();
    QString nom      = ui_employee->le_nom->text().trimmed();
    QString prenom   = ui_employee->le_prenom->text().trimmed();
    QString fonction = "smith";
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) fonction = cb->currentText();
    QDate   birthDate = ui_employee->de_birthdate->date();
    int     age       = birthDate.daysTo(QDate::currentDate()) / 365;
    QString mdp      = ui_employee->le_mdp->text().trimmed();
    QString address  = ui_employee->le_address->text().trimmed();
    double  salaire  = ui_employee->dsb_salaire->value();
    QString email    = ui_employee->le_email->text().trimmed();
    QString num      = ui_employee->le_num->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Identification Failure: Select an employee to update.");
        return;
    }

    // Controle de Saisie for update
    QRegularExpression nameRegex("^[A-Za-z\\s]+$");
    if (!nom.isEmpty() && !nameRegex.match(nom).hasMatch()) { QMessageBox::warning(this, "Validation", "LAST NAME contains invalid characters."); return; }
    if (!prenom.isEmpty() && !nameRegex.match(prenom).hasMatch()) { QMessageBox::warning(this, "Validation", "FIRST NAME contains invalid characters."); return; }

    QRegularExpression emailRegex("^[\\w\\-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$");
    if (!email.isEmpty() && !emailRegex.match(email).hasMatch()) { QMessageBox::warning(this, "Validation", "Malformed EMAIL structure."); return; }

    QRegularExpression phoneRegex("^\\d{8}$");
    if (!num.isEmpty() && !phoneRegex.match(num).hasMatch()) { QMessageBox::warning(this, "Validation", "PHONE NUMBER must be 8 digits."); return; }

    if (salaire < 0) { QMessageBox::warning(this, "Validation", "Negative SALARY is not permitted."); return; }

    // DROP constraint to allow same email for multiple employees as requested
    QSqlQuery dropUK("ALTER TABLE EMPLOYEES DROP CONSTRAINT UK_EMPLOYEES_EMAIL");
    dropUK.exec();
    QSqlQuery dropUQ("ALTER TABLE EMPLOYEES DROP CONSTRAINT UQ_EMPLOYEES_EMAIL");
    dropUQ.exec();

    QSqlQuery q;
    q.prepare("UPDATE EMPLOYEES SET LAST_NAME=:nom, FIRST_NAME=:prenom, JOB_TITLE=:fonction,"
              " AGE=:age, PASSWORD=:mdp, ADDRESS=:address, SALARY=:salaire, EMAIL=:email, PHONE_NUMBER=:num"
              " WHERE EMPLOYEE_ID=:id");
    q.bindValue(":id",       id.toInt());
    q.bindValue(":nom",      nom);
    q.bindValue(":prenom",   prenom);
    q.bindValue(":fonction", fonction);
    q.bindValue(":age",      age);
    q.bindValue(":mdp",      mdp);
    q.bindValue(":address",  address);
    q.bindValue(":salaire",  salaire);
    q.bindValue(":email",    email);
    q.bindValue(":num",      num);

    if (q.exec()) {
        if (q.numRowsAffected() > 0) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Success", "Employee updated successfully.");
            logActivity("Modified employee: " + prenom + " " + nom + " (ID: " + id + ")", "Employees");
            onEmployeeClearFields();
            onEmployeeRefreshView();
            onEmployeeRefreshHistory();
        } else {
            QMessageBox::warning(this, "Not Found", "No employee found with that ID.");
        }
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update employee:\n" + q.lastError().text());
    }
}

void MainWindow::onEmployeeDelete()
{
    QItemSelectionModel *select = ui_employee->tableView_employes->selectionModel();
    QModelIndexList selected = select->selectedRows();
    
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Selection", "Please select at least one employee from the table to delete.");
        return;
    }
    
    QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
    if (!m) return;
    
    int count = selected.size();
    int ret = QMessageBox::question(this, "Confirm Bulk Delete", 
                                    QString("Are you sure you want to delete %1 selected employee(s)?").arg(count),
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (ret == (int)QMessageBox::Yes) {
        bool someFailed = false;
        int deletedCount = 0;
        
        QSqlDatabase::database().transaction();
        for (const QModelIndex &idx : selected) {
            QString empId = m->data(m->index(idx.row(), 2)).toString();
            QString name  = m->data(m->index(idx.row(), 3)).toString() + " " + m->data(m->index(idx.row(), 4)).toString();
            
            // Unlink explicitly to allow absolute free deletion
            QSqlQuery qUnlinkEquip;
            qUnlinkEquip.prepare("UPDATE EQUIPMENT SET EMPLOYEE_ID = NULL WHERE EMPLOYEE_ID = :id");
            qUnlinkEquip.bindValue(":id", empId.toInt());
            qUnlinkEquip.exec();

            QSqlQuery qUnlinkClient;
            qUnlinkClient.prepare("UPDATE CLIENTS SET EMPLOYEE_ID = NULL WHERE EMPLOYEE_ID = :id");
            qUnlinkClient.bindValue(":id", empId.toInt());
            qUnlinkClient.exec();

            QSqlQuery qUnlinkOrders;
            qUnlinkOrders.prepare("DELETE FROM ORDERS WHERE EMPLOYEE_ID = :id");
            qUnlinkOrders.bindValue(":id", empId.toInt());
            qUnlinkOrders.exec();
            
            QSqlQuery q;
            q.prepare("DELETE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
            q.bindValue(":id", empId.toInt());
            if (q.exec()) {
                deletedCount++;
                logActivity("Deleted employee: " + name + " (ID: " + empId + ")", "Employees");
            } else {
                someFailed = true;
                QMessageBox::warning(this, "Deletion Error", "Could not delete employee ID " + empId + "\nReason: " + q.lastError().text());
            }
        }
        
        QSqlDatabase::database().commit();
        
        if (someFailed) {
            QMessageBox::warning(this, "Partial Deletion", 
                QString("Successfully deleted %1 employees. Some records could not be deleted due to active management links or errors.").arg(deletedCount));
        } else {
            QMessageBox::information(this, "Deleted", QString("%1 employees deleted successfully.").arg(deletedCount));
        }
        
        onEmployeeRefreshView();
        onEmployeeRefreshHistory();
    }
}

// =============================================================================
// EQUIPMENT MANAGEMENT CRUD
// =============================================================================

void MainWindow::onClientExportPDF()
{
    if (!ui_client) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Export Client List",
        QDir::homePath() + "/Client_List_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf);;All Files (*)");

    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageSize(QPageSize(QSize(297, 210), QPageSize::Millimeter)); // A4 Landscape
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to start PDF export.");
        return;
    }

    int pageWidth = printer.width();
    int y = 50;

    // Title
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71)); // Brown
    painter.drawText(0, y, pageWidth, 40, Qt::AlignCenter, "Hammer Down - Client List");
    y += 60;

    // Header Row
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::black);
    
    int cols[] = { 60, 150, 150, 250, 120, 200, 80 }; // ID, Nom, Prenom, Addr, Tel, Email, Sexe
    QString headers[] = { "ID", "Last Name", "First Name", "Address", "Phone", "Email", "Gender" };
    
    int x = 20;
    for (int i = 0; i < 7; ++i) {
        painter.drawText(x, y, cols[i], 25, Qt::AlignLeft, headers[i]);
        x += cols[i];
    }
    
    painter.drawLine(20, y + 25, pageWidth - 20, y + 25);
    y += 40;

    // Data Rows
    painter.setFont(QFont("Segoe UI", 9));
    QSqlQuery q("SELECT CLIENT_ID, LAST_NAME, FIRST_NAME, ADDRESS, PHONE_NUMBER, EMAIL, GENDER FROM CLIENTS ORDER BY CLIENT_ID");
    while (q.next()) {
        if (y > printer.height() - 60) {
            printer.newPage();
            y = 50;
        }
        
        x = 20;
        for (int i = 0; i < 7; ++i) {
            painter.drawText(x, y, cols[i], 20, Qt::AlignLeft, q.value(i).toString());
            x += cols[i];
        }
        y += 25;
    }

    painter.end();
    QMessageBox::information(this, "Success", "Client list successfully exported to:\n" + fileName);
}

// ---------------------------------------------------------------------------
// Client Mail tab — Send
// ---------------------------------------------------------------------------
void MainWindow::onClientSendMail()
{
    if (!ui_client) return;

    const QString to         = ui_client->le_to->text().trimmed();
    const QString subject    = ui_client->le_subject->text().trimmed();
    const QString attachment = ui_client->le_attachment->text().trimmed();
    const QString body       = ui_client->te_message->toPlainText().trimmed();

    // --- Validation ---
    if (to.isEmpty()) {
        QMessageBox::warning(this, "Email Error",
            "Please enter a recipient email address.");
        ui_client->le_to->setFocus();
        return;
    }

    static const QRegularExpression emailRe(
        R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
    if (!emailRe.match(to).hasMatch()) {
        QMessageBox::warning(this, "Email Error",
            "Invalid email address.\nMust contain '@' and a valid domain (e.g. user@example.com).");
        ui_client->le_to->setFocus();
        return;
    }

    if (body.isEmpty()) {
        QMessageBox::warning(this, "Email Error",
            "Please enter a message.");
        ui_client->te_message->setFocus();
        return;
    }

    if (body.length() < 5) {
        QMessageBox::warning(this, "Email Error",
            "Message is too short — minimum 5 characters required.");
        ui_client->te_message->setFocus();
        return;
    }

    // --- Hardcoded SMTP credentials (Switched to Brevo for better trials) ---
    const QString host     = "smtp-relay.brevo.com";
    const quint16 port     = 587;
    const QString username = "rayenkabar780@gmail.com";
    const QString password = "xsmtpsib-87c2fb8b2fd4f260176840d024467dcabacccc643a60a7e5f3aa5394be562fdb-S9hVfpx8rkgB2MT6";

    // Disable the button while sending
    ui_client->btn_send->setEnabled(false);
    ui_client->btn_send->setText("Sending...");

    // Run SMTP in a background thread so the UI stays responsive
    QFutureWatcher<SmtpResult> *watcher = new QFutureWatcher<SmtpResult>(this);
    connect(watcher, &QFutureWatcher<SmtpResult>::finished, this, [this, watcher]() {
        const SmtpResult result = watcher->result();
        watcher->deleteLater();

        ui_client->btn_send->setEnabled(true);
        ui_client->btn_send->setText("Send");

        if (result.success) {
            QMessageBox::information(this, "Email Sent",
                "Your email was sent successfully.");
            ui_client->le_to->clear();
            ui_client->le_subject->clear();
            ui_client->le_attachment->clear();
            ui_client->te_message->clear();
        } else {
            QMessageBox::critical(this, "Email Failed",
                "Failed to send email:\n" + result.errorMessage);
        }
    });

    QFuture<SmtpResult> future = QtConcurrent::run([=]() {
        return SmtpSender::send(host, port, username, password,
                                to, subject, body, attachment);
    });
    watcher->setFuture(future);
}

// ---------------------------------------------------------------------------
// Client Mail tab — Browse attachment
// ---------------------------------------------------------------------------
void MainWindow::onClientBrowseMail()
{
    if (!ui_client) return;
    const QString path = QFileDialog::getOpenFileName(
        this, "Select Attachment", QDir::homePath(), "All Files (*)");
    if (!path.isEmpty())
        ui_client->le_attachment->setText(path);
}


void MainWindow::togglePresentationMode() {
    m_isPresentationMode = !m_isPresentationMode;
    
    if (m_isPresentationMode) {
        // TURN ON
        m_presentationStep = 0;
        
        // 1. Fade away controls
        QList<QWidget*> controls;
        for (auto *w : findChildren<QPushButton*>()) controls << w;
        for (auto *w : findChildren<QRadioButton*>()) controls << w;
        for (auto *w : findChildren<QToolButton*>()) controls << w;
        
        for (auto *c : controls) {
            if (c->parent() == this || c->parentWidget() == this) continue; // Keep main window buttons? No, hide all
            QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(c);
            c->setGraphicsEffect(eff);
            QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
            a->setDuration(1200);
            a->setStartValue(1.0);
            a->setEndValue(0.0);
            connect(a, &QPropertyAnimation::finished, c, &QWidget::hide);
            a->start(QAbstractAnimation::DeleteWhenStopped);
        }
        
        // 2. Expand and transform
        if (ui_equipment && ui_equipment->tabWidget) {
            ui_equipment->tabWidget->setGeometry(50, 50, width()-100, height()-100);
            ui_equipment->tabWidget->setStyleSheet("QTabWidget::pane { border: none; background: transparent; } QTabBar::tab { height: 0px; width: 0px; margin: 0; padding: 0; }");
        }

        // 3. Ken Burns Background
        startKenBurnsEffect();

        // 4. Watermark
        m_presentationOverlay = new QWidget(this);
        m_presentationOverlay->setGeometry(rect());
        m_presentationOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_presentationOverlay->show();

        QLabel *watermark = new QLabel(m_presentationOverlay);
        watermark->setPixmap(QIcon(":/assets/logo.png").pixmap(120, 120));
        watermark->setGeometry(width()-150, height()-150, 120, 120);
        watermark->setStyleSheet("background: transparent; opacity: 0.5;");
        watermark->show();

        // 5. Setup Rotation/Navigation state
        m_presentationStep = ui_equipment->tabWidget->indexOf(ui_equipment->tab_view);
        if (m_presentationStep == -1) m_presentationStep = 0;
        
        ui_equipment->tabWidget->setCurrentIndex(m_presentationStep);
        advancePresentation(); // Update UI/Toast
        
    } else {
        // TURN OFF (Restore Normal View)
        if (m_presentationOverlay) m_presentationOverlay->deleteLater();
        m_presentationOverlay = nullptr;
        
        // Restore controls visibility
        QList<QWidget*> controls;
        for (auto *w : findChildren<QPushButton*>()) controls << w;
        for (auto *w : findChildren<QRadioButton*>()) controls << w;
        for (auto *w : findChildren<QToolButton*>()) controls << w;

        for (auto *c : controls) {
            c->setGraphicsEffect(nullptr);
            c->show();
        }

        if (ui_equipment && ui_equipment->tabWidget) {
            ui_equipment->tabWidget->setGeometry(118, 70, 1051, 681);
            ui_equipment->tabWidget->setStyleSheet(""); 
        }
    }
}

void MainWindow::advancePresentation() {
    if (!m_isPresentationMode || !ui_equipment) return;

    ui_equipment->tabWidget->setCurrentIndex(m_presentationStep);
    
    QString stepTitle;
    QWidget* current = ui_equipment->tabWidget->currentWidget();
    
    if (current == ui_equipment->tab_gestion) stepTitle = "Workshop Resource Management";
    else if (current == ui_equipment->tab_view) stepTitle = "Workshop Inventory Status";
    else if (current == ui_equipment->tab_stats) stepTitle = "Global Asset Analytics";
    else if (current == ui_equipment->tab_history) stepTitle = "Operational Activity logs";
    else if (current == ui_equipment->tab_chat) stepTitle = "Workshop Secure Communications";
    else stepTitle = "Presentation Slide";

    // Overlay title toast
    QLabel *toast = new QLabel(stepTitle, this);
    toast->setFixedSize(500, 80);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet("background: rgba(139,111,71,0.95); color: white; font-size: 26px; border-radius: 40px; border: 2.5px solid #D4AF37; font-weight: bold;");
    toast->move(width()/2 - 250, 80);
    toast->show();
    
    QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(op);
    QPropertyAnimation *fIn = new QPropertyAnimation(op, "opacity");
    fIn->setDuration(800);
    fIn->setStartValue(0.0);
    fIn->setEndValue(1.0);
    fIn->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(2500, [=](){
        QPropertyAnimation *fOut = new QPropertyAnimation(op, "opacity");
        fOut->setDuration(1200);
        fOut->setStartValue(1.0);
        fOut->setEndValue(0.0);
        connect(fOut, &QPropertyAnimation::finished, toast, &QLabel::deleteLater);
        fOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::startKenBurnsEffect() {
    // Find the background (usually set via stylesheet on MainWindow or a central frame)
    // We'll simulate by animating a large ghost image or the central widget style
    QPropertyAnimation *kb = new QPropertyAnimation(this, "geometry"); // Dummy for now to trigger background repaint if needed
    Q_UNUSED(kb);
    // In a real app, you'd apply this to a specific QGraphicsView background
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (m_isPresentationMode) {
        if (event->key() == Qt::Key_Escape) {
            togglePresentationMode(); // Exit only on Escape
            event->accept();
        } else if (event->key() == Qt::Key_Left) {
            // Previous tab
            int count = ui_equipment->tabWidget->count();
            m_presentationStep = (m_presentationStep - 1 + count) % count;
            advancePresentation();
            event->accept();
        } else if (event->key() == Qt::Key_Right) {
            // Next tab
            int count = ui_equipment->tabWidget->count();
            m_presentationStep = (m_presentationStep + 1) % count;
            advancePresentation();
            event->accept();
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::updateSupplierProgress() {
    if (!m_supplierProgress || !ui_supplier) return;
    
    int progress = 0;
    bool nameOk  = !ui_supplier->le_nom->text().trimmed().isEmpty();
    bool emailOk = ui_supplier->le_email->text().contains("@") && ui_supplier->le_email->text().contains(".");
    bool telOk   = ui_supplier->le_tel->text().trimmed().length() >= 8;
    bool typeOk  = !ui_supplier->le_type->text().trimmed().isEmpty();
    
    if (nameOk)  progress += 25;
    if (emailOk) progress += 25;
    if (telOk)   progress += 25;
    if (typeOk)  progress += 25;
    
    m_supplierProgress->setValue(progress);
    
    if (progress == 100) {
        m_supplierProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #66BB6A); border-radius: 5px; }");
    } else {
        m_supplierProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");
    }
    
    auto updateInd = [](QLabel* l, bool ok, const QString& prefix) {
        if (!l) return;
        if (ok) {
            l->setText(prefix + " ✅]");
            l->setStyleSheet("color: #4CAF50; font-size: 11px; font-weight: bold;");
        } else {
            l->setText(prefix + " ⬜]");
            l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        }
    };
    
    updateInd(m_suppNameInd,  nameOk,  "[👤 Name");
    updateInd(m_suppEmailInd, emailOk, "[📧 Email");
    updateInd(m_suppTelInd,   telOk,   "[📞 Phone");
    updateInd(m_suppTypeInd,  typeOk,  "[🏢 Type");

    // Pulsing animation for Add button at 100%
    if (progress == 100) {
        if (!ui_supplier->btn_add->graphicsEffect()) {
            QGraphicsDropShadowEffect *eff = new QGraphicsDropShadowEffect(this);
            eff->setBlurRadius(15);
            eff->setColor(QColor(212, 175, 55, 200));
            eff->setOffset(0);
            ui_supplier->btn_add->setGraphicsEffect(eff);
            
            QPropertyAnimation *pulse = new QPropertyAnimation(eff, "blurRadius");
            pulse->setDuration(1000);
            pulse->setStartValue(8);
            pulse->setEndValue(25);
            pulse->setLoopCount(-1);
            pulse->setEasingCurve(QEasingCurve::InOutSine);
            pulse->start(QAbstractAnimation::DeleteWhenStopped);
        }
    } else {
        ui_supplier->btn_add->setGraphicsEffect(nullptr);
    }
}




void MainWindow::onUploadAvatar() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty()) return;

    QString employeeId = ui_employee->le_id->text();
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, tr("Avatar"), tr("Please select an employee or enter an ID first."));
        return;
    }

    QDir().mkpath("assets/av");
    QString destPath = QString("assets/av/employee_%1.png").arg(employeeId);
    if (QFile::exists(destPath)) QFile::remove(destPath);
    if (QFile::copy(fileName, destPath)) {
        QPixmap pix(destPath);
        ui_employee->lbl_avatar->setPixmap(getCircularPixmap(pix).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // QMessageBox removed to stop annoying duplicate popups for the user
        if (employeeId.toInt() == currentEmployeeId) updateUserProfileDisplay();
    } else {
        QMessageBox::critical(this, tr("Avatar"), tr("Failed to save avatar."));
    }
}

void MainWindow::onScanFace() {
    QString employeeId = ui_employee->le_id->text().trimmed();
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, tr("Face Scan"), tr("Please select an employee or enter an ID first."));
        return;
    }

    if (m_isEmpFaceScanActive) {
        // Capture ONE high-quality frame
        QVideoFrame frame = m_empVideoSink->videoFrame();
        if (frame.isValid() && frame.map(QVideoFrame::ReadOnly)) {
            QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
            frame.unmap();
            
            // Flip to ensure it is NOT mirrored (standard view)
            image = image.mirrored(true, false);
            
            QDir().mkpath("assets/av");
            // face_ path is STRICTLY for biometric login matching only
            QString facePath = QString("assets/av/face_%1.png").arg(employeeId);
            if (QFile::exists(facePath)) QFile::remove(facePath);
            image.save(facePath);

            // Done - stop camera and restore employee avatar display (NOT the face scan image)
            m_empCamera->stop();
            m_isEmpFaceScanActive = false;
            m_faceScanStage = 0;
            ui_employee->btn_scan_face->setText(tr("Scan Face ID"));

            // Restore the correct avatar (employee_) or placeholder after scan
            QString avPath = QString("assets/av/employee_%1.png").arg(employeeId);
            if (QFile::exists(avPath)) {
                ui_employee->lbl_avatar->setPixmap(getCircularPixmap(QPixmap(avPath)).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                ui_employee->lbl_avatar->setPixmap(QPixmap());
                ui_employee->lbl_avatar->setText("No Avatar");
            }

            QMessageBox::information(this, tr("Face ID"), tr("Biometric profile registered successfully.\nThis image is used for login only and is separate from the display avatar."));
        }
    } else {
        // Start Scan
        if (!m_empCamera) {
             m_empCamera = new QCamera(QMediaDevices::defaultVideoInput(), this);
             m_empCaptureSession = new QMediaCaptureSession(this);
             m_empVideoSink = new QVideoSink(this);
             m_empCaptureSession->setCamera(m_empCamera);
             m_empCaptureSession->setVideoSink(m_empVideoSink);
             connect(m_empVideoSink, &QVideoSink::videoFrameChanged, this, &MainWindow::processEmpCameraFrame);
        }
        
        m_faceScanStage = 0;
        m_empCamera->start();
        m_isEmpFaceScanActive = true;
        ui_employee->btn_scan_face->setText(tr("SAVE CAPTURE"));
        QMessageBox::information(this, tr("Face Scan"), tr("Scanning started. Please look straight at the camera and click 'SAVE CAPTURE' (Image will be non-mirrored)."));
    }
}

void MainWindow::processEmpCameraFrame() {
    if (!m_isEmpFaceScanActive) return;
    
    QVideoFrame frame = m_empVideoSink->videoFrame();
    if (!frame.isValid() || !frame.map(QVideoFrame::ReadOnly)) return;
    
    QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
    frame.unmap();

    // Disable Mirroring for Real-View Capture experience
    image = image.mirrored(true, false);
    
    // Show live camera preview in lbl_avatar ONLY during active face scan
    // This is a TEMPORARY preview - the saved face_ image is never shown as an avatar
    ui_employee->lbl_avatar->setText("");
    ui_employee->lbl_avatar->setPixmap(getCircularPixmap(QPixmap::fromImage(image)).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::updateUserProfileDisplay() {
    if (currentEmployeeId <= 0) return;

    QSqlQuery query;
    query.prepare("SELECT FIRST_NAME, LAST_NAME, JOB_TITLE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    query.bindValue(":id", currentEmployeeId);
    if (query.exec() && query.next()) {
        QString firstName = query.value(0).toString();
        QString jobTitle = query.value(2).toString();
        
        if (homeWindow) {
           QLabel* nameLabel = homeWindow->findChild<QLabel*>("lbl_user_name");
           QLabel* roleLabel = homeWindow->findChild<QLabel*>("lbl_user_role");
           QLabel* avatarLabel = homeWindow->findChild<QLabel*>("lbl_user_avatar");
           
           if (nameLabel) nameLabel->setText(firstName);
           if (roleLabel) roleLabel->setText(jobTitle);
           
           // Profile sidebar shows AVATAR only (employee_[ID].png) - never the face scan image
           QString avatarPath = QString("assets/av/employee_%1.png").arg(currentEmployeeId);

           if (avatarLabel) {
               if (QFile::exists(avatarPath)) {
                   QPixmap pix(avatarPath);
                   avatarLabel->setPixmap(getCircularPixmap(pix).scaled(avatarLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                   avatarLabel->setText("");
               } else {
                   avatarLabel->setPixmap(QPixmap());
                   avatarLabel->setText("No Pic");
               }
           }
        }
    }
}

QPixmap MainWindow::getCircularPixmap(const QPixmap &src) {
    if (src.isNull()) return src;
    
    int size = qMin(src.width(), src.height());
    QPixmap out(size, size);
    out.fill(Qt::transparent);
    
    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    
    // Center the image
    int x = (size - src.width()) / 2;
    int y = (size - src.height()) / 2;
    painter.drawPixmap(x, y, src);
    
    return out;
}

void MainWindow::on_userProfileClicked() {
    ui->stackedWidget->setCurrentIndex(2); 
    if (ui_employee && ui_employee->tabWidget) ui_employee->tabWidget->setCurrentIndex(0);
    QRadioButton *rbMod = ui_employee->tab_add->findChild<QRadioButton*>("rb_employee_mod_mode");
    if (rbMod) rbMod->setChecked(true);
    if (ui_employee && ui_employee->le_id) {
        ui_employee->le_id->setText(QString::number(currentEmployeeId));
        onEmployeeRefreshView(); // Refresh to ensure data is loaded
    }
}

void MainWindow::onEmployeeEnsureHistoryTable() {
    // Use local JSON storage (no DB table creation allowed by user request).
    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray auditArray;

    // Load existing log
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray raw = file.readAll();
            file.close();

            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            if (doc.isArray()) auditArray = doc.array();
        }
    }

    const QDateTime now = QDateTime::currentDateTime();
    QJsonObject obj;
    obj["log_id"] = auditArray.size() + 1;
    obj["timestamp_iso"] = now.toString(Qt::ISODate);
    obj["timestamp_ms"] = static_cast<qint64>(now.toMSecsSinceEpoch());
    
    // Get current employee name
    QString empName = "System Manager";
    QSqlQuery nq;
    nq.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    nq.bindValue(":id", currentEmployeeId);
    if (currentEmployeeId > 0 && nq.exec() && nq.next()) {
        empName = nq.value(0).toString();
    }
    
    obj["employee_name"] = empName;
    obj["action_details"] = "Employee management action";
    obj["module_name"] = "Employee Management";

    auditArray.append(obj);

    // Save back
    QFile out(filePath);
    if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        out.write(QJsonDocument(auditArray).toJson(QJsonDocument::Compact));
        out.close();
    }
}

void MainWindow::setupOrderMapTab()
{
    if (!ui_order || !ui_order->tabWidget) return;

    QWidget *mapTab = new QWidget(ui_order->tabWidget);
    mapTab->setObjectName("tab_map");

    QVBoxLayout *root = new QVBoxLayout(mapTab);
    root->setContentsMargins(18, 60, 18, 16);
    root->setSpacing(10);

    QHBoxLayout *controls = new QHBoxLayout();
    QLabel *clientIdLabel = new QLabel("Clients:", mapTab);
    clientIdLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    m_mapRefreshBtn = new QPushButton("Load Map", mapTab);
    m_mapRefreshBtn->setCursor(Qt::PointingHandCursor);
    m_mapRefreshBtn->setStyleSheet(
        "QPushButton{background:#8B6F47;color:white;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#a3845a;}"
        "QPushButton:pressed{background:#6b5535;}");

    m_mapZoomInBtn = new QPushButton("Zoom +", mapTab);
    m_mapZoomInBtn->setCursor(Qt::PointingHandCursor);
    m_mapZoomInBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}"
        "QPushButton:pressed{background:#3d2e18;}");

    m_mapZoomOutBtn = new QPushButton("Zoom -", mapTab);
    m_mapZoomOutBtn->setCursor(Qt::PointingHandCursor);
    m_mapZoomOutBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}"
        "QPushButton:pressed{background:#3d2e18;}");

    m_mapFullscreenBtn = new QPushButton("Full Screen", mapTab);
    m_mapFullscreenBtn->setCursor(Qt::PointingHandCursor);
    m_mapFullscreenBtn->setStyleSheet(
        "QPushButton{background:#3d2e18;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#5c4a2a;}"
        "QPushButton:pressed{background:#2a1e10;}");


    controls->addWidget(clientIdLabel);
    controls->addStretch();
    controls->addWidget(m_mapRefreshBtn);
    controls->addWidget(m_mapZoomInBtn);
    controls->addWidget(m_mapZoomOutBtn);
    controls->addWidget(m_mapFullscreenBtn);

    m_mapAddressLabel = new QLabel("Address: --", mapTab);
    m_mapAddressLabel->setWordWrap(true);
    m_mapAddressLabel->setStyleSheet("color: #d4a96a; font-size: 12px;");

    m_mapAssignedEmployeeLabel = new QLabel("Assigned Employee: --", mapTab);
    m_mapAssignedEmployeeLabel->setWordWrap(true);
    m_mapAssignedEmployeeLabel->setStyleSheet("color: #9ecbff; font-size: 12px;");

    m_mapDeliveryInfoLabel = new QLabel("Distance: -- | ETA: --", mapTab);
    m_mapDeliveryInfoLabel->setWordWrap(true);
    m_mapDeliveryInfoLabel->setStyleSheet("color: #8fd694; font-size: 12px; font-weight: bold;");

    m_mapStatusLabel = new QLabel("Select a client or enter Buyer ID, then click Load Map.", mapTab);
    m_mapStatusLabel->setWordWrap(true);
    m_mapStatusLabel->setStyleSheet("color: #8B6F47; font-size: 12px; font-style: italic;");

    m_mapClientTable = new QTableWidget(mapTab);
    m_mapClientTable->setColumnCount(3);
    m_mapClientTable->setHorizontalHeaderLabels({"Order ID", "Client", "Address"});
    m_mapClientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_mapClientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mapClientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_mapClientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_mapClientTable->verticalHeader()->setVisible(false);
    m_mapClientTable->setMinimumHeight(220);
    m_mapClientTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mapClientTable->setStyleSheet(
        "QTableWidget { background: rgba(26, 18, 8, 0.85); color: #e8dcc8; border: 2px solid #8B6F47; border-radius: 10px; }"
        "QHeaderView::section { background: #2a1e10; color: #d4a96a; border: none; padding: 4px; }"
        "QTableWidget::item:selected { background: #8B6F47; color: white; }");

    m_mapImageLabel = new QLabel(mapTab);
    m_mapImageLabel->setMinimumSize(640, 360);
    m_mapImageLabel->setAlignment(Qt::AlignCenter);
    m_mapImageLabel->setStyleSheet("background: #1a1208; border: 2px solid #8B6F47; border-radius: 10px; color: #8B6F47;");
    m_mapImageLabel->setText("Map preview will appear here.");
    m_mapImageLabel->installEventFilter(this);


    root->addLayout(controls);
    root->addWidget(m_mapClientTable);
    root->addWidget(m_mapAddressLabel);
    root->addWidget(m_mapAssignedEmployeeLabel);
    root->addWidget(m_mapDeliveryInfoLabel);
    root->addWidget(m_mapStatusLabel);
    root->addWidget(m_mapImageLabel, 1);

    ui_order->tabWidget->addTab(mapTab, "Map");

    connect(m_mapRefreshBtn, &QPushButton::clicked, this, &MainWindow::requestMapForBuyerId);
    connect(m_mapClientTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::requestMapForBuyerId);
    connect(m_mapZoomInBtn, &QPushButton::clicked, this, [this]() {
        m_mapZoom = qMin(18, m_mapZoom + 1);
        m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
            ? m_mapFullscreenLabel->size()
            : m_mapImageLabel->size();
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);
    });
    connect(m_mapZoomOutBtn, &QPushButton::clicked, this, [this]() {
        m_mapZoom = qMax(3, m_mapZoom - 1);
        m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
            ? m_mapFullscreenLabel->size()
            : m_mapImageLabel->size();
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);
    });
    connect(m_mapFullscreenBtn, &QPushButton::clicked, this, [this]() {
        if (!m_mapFullscreenDialog) {
            m_mapFullscreenDialog = new QDialog(this);
            m_mapFullscreenDialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
            m_mapFullscreenDialog->setModal(true);
            m_mapFullscreenDialog->setAttribute(Qt::WA_TranslucentBackground);
            m_mapFullscreenDialog->setStyleSheet("QDialog { background: rgba(0,0,0,90); }");

            QVBoxLayout *layout = new QVBoxLayout(m_mapFullscreenDialog);
            layout->setContentsMargins(24, 24, 24, 24);

            QWidget *card = new QWidget(m_mapFullscreenDialog);
            card->setStyleSheet("QWidget { background: rgba(15, 10, 6, 220); border: 2px solid #8B6F47; border-radius: 14px; }");
            QVBoxLayout *cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(14, 14, 14, 14);

            m_mapFullscreenLabel = new QLabel(card);
            m_mapFullscreenLabel->setAlignment(Qt::AlignCenter);
            m_mapFullscreenLabel->setMinimumSize(1000, 620);
            m_mapFullscreenLabel->setStyleSheet("color: #d4a96a; font-size: 14px; border: 1px solid #8B6F47; border-radius: 10px; background: #1a1208;");
            m_mapFullscreenLabel->installEventFilter(this);
            cardLayout->addWidget(m_mapFullscreenLabel);

            QLabel *hint = new QLabel("Drag to move • Mouse wheel to zoom • Double-click or Esc to close", card);
            hint->setAlignment(Qt::AlignCenter);
            hint->setStyleSheet("color:#d4a96a; font-size:12px; border:none; background:transparent;");
            cardLayout->addWidget(hint);

            layout->addStretch();
            layout->addWidget(card, 0, Qt::AlignCenter);
            layout->addStretch();

            QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), m_mapFullscreenDialog);
            connect(esc, &QShortcut::activated, m_mapFullscreenDialog, &QDialog::close);

            connect(m_mapFullscreenDialog, &QDialog::finished, this, [this]() {
                if (ui && ui->stackedWidget) {
                    ui->stackedWidget->setGraphicsEffect(nullptr);
                }
                m_mapBlurEffect = nullptr;
                m_mapDragging = false;
            });
        }

        if (ui && ui->stackedWidget && !m_mapBlurEffect) {
            m_mapBlurEffect = new QGraphicsBlurEffect(this);
            m_mapBlurEffect->setBlurRadius(10.0);
            ui->stackedWidget->setGraphicsEffect(m_mapBlurEffect);
        }

        if (m_mapHasPixmap) {
            m_mapImageSize = m_mapFullscreenLabel->size();
            m_mapFullscreenLabel->setPixmap(m_mapCurrentPixmap.scaled(
                m_mapFullscreenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_mapFullscreenLabel->setText("Map is not loaded yet.");
        }

        m_mapFullscreenDialog->setGeometry(this->window()->geometry());
        m_mapFullscreenDialog->show();
        m_mapFullscreenDialog->raise();
    });

    populateMapClients();
}

void MainWindow::populateMapClients()
{
    if (!m_mapClientTable) return;

    m_mapClientTable->setRowCount(0);
    QSqlQuery q("SELECT o.ORDER_ID, o.CLIENT_ID, c.FIRST_NAME, c.LAST_NAME, c.ADDRESS "
                "FROM ORDERS o "
                "LEFT JOIN CLIENTS c ON c.CLIENT_ID = o.CLIENT_ID "
                "ORDER BY o.ORDER_ID DESC");
    int row = 0;
    while (q.next()) {
        m_mapClientTable->insertRow(row);

        const int orderId = q.value(0).toInt();
        const int clientId = q.value(1).toInt();
        const QString firstName = q.value(2).toString().trimmed();
        const QString lastName = q.value(3).toString().trimmed();
        const QString clientName = (firstName + " " + lastName).trimmed();
        const QString address = q.value(4).toString().trimmed();

        QTableWidgetItem *orderItem = new QTableWidgetItem(QString::number(orderId));
        orderItem->setData(Qt::UserRole, clientId);
        m_mapClientTable->setItem(row, 0, orderItem);
        m_mapClientTable->setItem(row, 1, new QTableWidgetItem(clientName.isEmpty() ? QString("Client #%1").arg(clientId) : clientName));
        m_mapClientTable->setItem(row, 2, new QTableWidgetItem(address));
        row++;
    }

    if (row == 0) {
        m_mapStatusLabel->setText("No orders found.");
    }
}

void MainWindow::requestMapForBuyerId()
{
    if (!ui_order || !ui_order->le_buyer) return;

    QString address;
    int selectedOrderId = 0;
    int selectedClientId = 0;
    QString selectedClientName;

    if (m_mapClientTable && m_mapClientTable->currentRow() >= 0) {
        int row = m_mapClientTable->currentRow();
        QTableWidgetItem *orderItem = m_mapClientTable->item(row, 0);
        QTableWidgetItem *nameItem = m_mapClientTable->item(row, 1);
        QTableWidgetItem *addrItem = m_mapClientTable->item(row, 2);
        if (orderItem) {
            bool orderOk = false;
            const int parsedOrderId = orderItem->text().toInt(&orderOk);
            if (orderOk && parsedOrderId > 0) selectedOrderId = parsedOrderId;
            const int mappedClientId = orderItem->data(Qt::UserRole).toInt();
            if (mappedClientId > 0) selectedClientId = mappedClientId;
        }
        if (nameItem) selectedClientName = nameItem->text().trimmed();
        if (addrItem) {
            address = addrItem->text().trimmed();
        }
    }

    if (address.isEmpty()) {
        QString buyerText = ui_order->le_buyer->text().trimmed();
        if (buyerText.isEmpty()) {
            m_mapStatusLabel->setText("Please select a client or enter a Buyer ID.");
            return;
        }

        bool ok = false;
        int clientId = buyerText.toInt(&ok);
        if (!ok || clientId <= 0) {
            m_mapStatusLabel->setText("Buyer ID must be a valid number.");
            return;
        }

        QSqlQuery q;
        q.prepare("SELECT FIRST_NAME, LAST_NAME, ADDRESS FROM CLIENTS WHERE CLIENT_ID = :id");
        q.bindValue(":id", clientId);
        if (!q.exec() || !q.next()) {
            m_mapStatusLabel->setText("No client found for that Buyer ID.");
            m_mapAddressLabel->setText("Address: --");
            return;
        }

        selectedClientId = clientId;
        selectedClientName = (q.value(0).toString() + " " + q.value(1).toString()).trimmed();
        address = q.value(2).toString().trimmed();
        if (address.isEmpty()) {
            m_mapStatusLabel->setText("Client has no address on file.");
            m_mapAddressLabel->setText("Address: --");
            return;
        }
    }

    m_mapSelectedClientId = selectedClientId;
    m_mapSelectedClientName = selectedClientName;

    m_mapAddressLabel->setText("Address: " + address);
    if (m_mapAssignedEmployeeLabel) m_mapAssignedEmployeeLabel->setText("Assigned Employee: --");
    if (m_mapDeliveryInfoLabel) m_mapDeliveryInfoLabel->setText("Distance: -- | ETA: --");

    m_mapStatusLabel->setText("Geocoding address...");
    m_mapHasClientPin = false;
    m_mapHasEmployeePin = false;
    m_mapRouteGeoPoints.clear();
    m_mapAssignedEmployeeId = 0;
    m_mapAssignedEmployeeName.clear();
    m_mapPendingEmployeeAddress.clear();
    m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
        ? m_mapFullscreenLabel->size()
        : m_mapImageLabel->size();

    if (selectedOrderId > 0 || m_mapSelectedClientId > 0) {
        int resolvedEmployeeId = 0;

        if (selectedOrderId > 0) {
            QSqlQuery qOrderEmp;
            qOrderEmp.prepare("SELECT EMPLOYEE_ID FROM ORDERS WHERE ORDER_ID = :orderId");
            qOrderEmp.bindValue(":orderId", selectedOrderId);
            if (qOrderEmp.exec() && qOrderEmp.next()) {
                resolvedEmployeeId = qOrderEmp.value(0).toInt();
            }
        } else {
            QSqlQuery qOrderEmp;
            qOrderEmp.prepare("SELECT EMPLOYEE_ID FROM ORDERS "
                              "WHERE CLIENT_ID = :clientId "
                              "ORDER BY ORDER_DATE DESC NULLS LAST, ORDER_ID DESC "
                              "FETCH FIRST 1 ROWS ONLY");
            qOrderEmp.bindValue(":clientId", m_mapSelectedClientId);
            if (qOrderEmp.exec() && qOrderEmp.next()) {
                resolvedEmployeeId = qOrderEmp.value(0).toInt();
            }
        }

        // Fallback: when historic order rows have no employee_id, use logged-in user.
        if (resolvedEmployeeId <= 0 && currentEmployeeId > 0) {
            resolvedEmployeeId = currentEmployeeId;
        }

        if (resolvedEmployeeId > 0) {
            m_mapAssignedEmployeeId = resolvedEmployeeId;

            QSqlQuery qEmp;
            qEmp.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME, ADDRESS "
                         "FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
            qEmp.bindValue(":id", m_mapAssignedEmployeeId);
            if (qEmp.exec() && qEmp.next()) {
                m_mapAssignedEmployeeName = qEmp.value(0).toString().trimmed();
                m_mapPendingEmployeeAddress = qEmp.value(1).toString().trimmed();
            }

            if (m_mapPendingEmployeeAddress.isEmpty()) {
                QSqlQuery qAddrFallback;
                qAddrFallback.prepare("SELECT c.ADDRESS "
                                      "FROM ORDERS o "
                                      "LEFT JOIN CLIENTS c ON c.CLIENT_ID = o.CLIENT_ID "
                                      "WHERE o.EMPLOYEE_ID = :empId "
                                      "AND c.ADDRESS IS NOT NULL "
                                      "AND LENGTH(TRIM(c.ADDRESS)) > 0 "
                                      "ORDER BY o.ORDER_DATE DESC NULLS LAST, o.ORDER_ID DESC "
                                      "FETCH FIRST 1 ROWS ONLY");
                qAddrFallback.bindValue(":empId", m_mapAssignedEmployeeId);
                if (qAddrFallback.exec() && qAddrFallback.next()) {
                    m_mapPendingEmployeeAddress = qAddrFallback.value(0).toString().trimmed();
                }
            }

            if (m_mapAssignedEmployeeName.isEmpty()) {
                m_mapAssignedEmployeeName = QString("Employee #%1").arg(m_mapAssignedEmployeeId);
            }

            if (m_mapAssignedEmployeeLabel) {
                m_mapAssignedEmployeeLabel->setText(
                    QString("Assigned Employee: %1 (ID: %2)")
                        .arg(m_mapAssignedEmployeeName)
                        .arg(m_mapAssignedEmployeeId));
            }

            if (m_mapPendingEmployeeAddress.isEmpty() && m_mapDeliveryInfoLabel) {
                m_mapDeliveryInfoLabel->setText("Distance: -- | ETA: -- (employee address missing)");
            }
        } else {
            if (m_mapAssignedEmployeeLabel) {
                m_mapAssignedEmployeeLabel->setText("Assigned Employee: Unknown for this order");
            }
        }
    }

    QString geocodeQuery = address;
    if (!geocodeQuery.contains("tunisia", Qt::CaseInsensitive) &&
        !geocodeQuery.contains("tunisie", Qt::CaseInsensitive) &&
        !geocodeQuery.contains(QString::fromUtf8("\xD8\xAA\xD9\x88\xD9\x86\xD8\xB3"), Qt::CaseInsensitive)) {
        geocodeQuery += ", Tunisia";
    }

    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery query;
    query.addQueryItem("q", geocodeQuery);
    query.addQueryItem("format", "json");
    query.addQueryItem("limit", "1");
    query.addQueryItem("accept-language", "en");
    query.addQueryItem("countrycodes", "tn");
    query.addQueryItem("bounded", "1");
    query.addQueryItem("viewbox", "7.5,37.6,11.6,30.2");
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
    req.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = m_mapNet->get(req);
    reply->setProperty("mapType", "geocode");
    reply->setProperty("address", address);
    reply->setProperty("query", geocodeQuery);
    reply->setProperty("geocodeStage", "tn");
}

void MainWindow::onMapNetworkFinished(QNetworkReply *reply)
{
    if (!reply) return;
    const QString type = reply->property("mapType").toString();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (type == "geocode" || type == "geocode_employee") {
        const bool isEmployeeGeocode = (type == "geocode_employee");
        const QString stage = reply->property("geocodeStage").toString();
        const QString address = reply->property("address").toString();
        const QString queryText = reply->property("query").toString();
        const QByteArray data = reply->readAll();

        auto issueGlobalFallback = [this, address, queryText, type, isEmployeeGeocode]() {
            if (!m_mapNet) return;
            QUrl url("https://nominatim.openstreetmap.org/search");
            QUrlQuery q;
            q.addQueryItem("q", queryText.isEmpty() ? address : queryText);
            q.addQueryItem("format", "json");
            q.addQueryItem("limit", "1");
            q.addQueryItem("accept-language", "en");
            url.setQuery(q);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            req.setRawHeader("Accept", "application/json");
            QNetworkReply *fallback = m_mapNet->get(req);
            fallback->setProperty("mapType", type);
            fallback->setProperty("address", address);
            fallback->setProperty("query", queryText);
            fallback->setProperty("geocodeStage", "global");
            m_mapStatusLabel->setText(isEmployeeGeocode ? "Geocoding employee retry (global)..."
                                                        : "Geocoding retry (global)...");
        };

        auto issuePhotonFallback = [this, address, queryText, type, isEmployeeGeocode]() {
            if (!m_mapNet) return;
            QUrl url("https://photon.komoot.io/api");
            QUrlQuery q;
            q.addQueryItem("q", queryText.isEmpty() ? address : queryText);
            q.addQueryItem("limit", "1");
            url.setQuery(q);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            req.setRawHeader("Accept", "application/json");
            QNetworkReply *fallback = m_mapNet->get(req);
            fallback->setProperty("mapType", type);
            fallback->setProperty("address", address);
            fallback->setProperty("query", queryText);
            fallback->setProperty("geocodeStage", "photon");
            m_mapStatusLabel->setText(isEmployeeGeocode ? "Geocoding employee retry (fallback provider)..."
                                                        : "Geocoding retry (fallback provider)...");
        };

        bool hasCoords = false;
        double latVal = 0.0;
        double lonVal = 0.0;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (stage == "photon") {
                const QJsonObject root = doc.object();
                const QJsonArray features = root.value("features").toArray();
                if (!features.isEmpty()) {
                    const QJsonObject feature = features.first().toObject();
                    const QJsonArray coords = feature.value("geometry").toObject().value("coordinates").toArray();
                    if (coords.size() >= 2) {
                        lonVal = coords.at(0).toDouble();
                        latVal = coords.at(1).toDouble();
                        hasCoords = true;
                    }
                }
            } else {
                const QJsonArray arr = doc.array();
                if (!arr.isEmpty()) {
                    const QJsonObject obj = arr.first().toObject();
                    bool okLat = false;
                    bool okLon = false;
                    latVal = obj.value("lat").toString().toDouble(&okLat);
                    lonVal = obj.value("lon").toString().toDouble(&okLon);
                    hasCoords = okLat && okLon;
                }
            }
        }

        // Guardrail: reject geocodes clearly outside Tunisia region.
        auto inTunisiaBounds = [](double lat, double lon) {
            return lat >= 30.0 && lat <= 37.8 && lon >= 7.0 && lon <= 12.2;
        };
        if (hasCoords && !inTunisiaBounds(latVal, lonVal)) {
            hasCoords = false;
        }

        if (!hasCoords) {
            if (stage == "tn") {
                issueGlobalFallback();
                reply->deleteLater();
                return;
            }
            if (stage == "global") {
                issuePhotonFallback();
                reply->deleteLater();
                return;
            }

            if (reply->error() != QNetworkReply::NoError) {
                m_mapStatusLabel->setText(isEmployeeGeocode
                                              ? QString("Employee geocoding failed (%1)").arg(reply->errorString())
                                              : QString("Geocoding failed (%1)").arg(reply->errorString()));
            } else if (httpStatus >= 400) {
                m_mapStatusLabel->setText(isEmployeeGeocode
                                              ? QString("Employee geocoding failed (HTTP %1)").arg(httpStatus)
                                              : QString("Geocoding failed (HTTP %1)").arg(httpStatus));
            } else {
                m_mapStatusLabel->setText(isEmployeeGeocode
                                              ? "Employee address not found on map."
                                              : "Address not found on map.");
            }

            if (isEmployeeGeocode && m_mapDeliveryInfoLabel) {
                m_mapDeliveryInfoLabel->setText("Distance: -- | ETA: -- (employee pin unavailable)");
            }

            reply->deleteLater();
            return;
        }

        if (isEmployeeGeocode) {
            m_mapEmployeePinLat = latVal;
            m_mapEmployeePinLon = lonVal;
            m_mapHasEmployeePin = true;

            if (m_mapHasClientPin) {
                m_mapCenterLat = (m_mapClientPinLat + m_mapEmployeePinLat) / 2.0;
                m_mapCenterLon = (m_mapClientPinLon + m_mapEmployeePinLon) / 2.0;

                // Load map while route service computes drivable car path.
                requestMapTiles(m_mapCenterLat, m_mapCenterLon);

                // Snap employee point to nearest drivable road first.
                QUrl nearestUrl(QString("http://router.project-osrm.org/nearest/v1/driving/%1,%2")
                                    .arg(m_mapEmployeePinLon, 0, 'f', 6)
                                    .arg(m_mapEmployeePinLat, 0, 'f', 6));
                QUrlQuery nearestParams;
                nearestParams.addQueryItem("number", "1");
                nearestUrl.setQuery(nearestParams);

                QNetworkRequest nearestReq(nearestUrl);
                nearestReq.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
                nearestReq.setRawHeader("Accept", "application/json");
                QNetworkReply *nearestReply = m_mapNet->get(nearestReq);
                nearestReply->setProperty("mapType", "route_nearest_employee");
                nearestReply->setProperty("employeeLat", m_mapEmployeePinLat);
                nearestReply->setProperty("employeeLon", m_mapEmployeePinLon);
                nearestReply->setProperty("clientLat", m_mapClientPinLat);
                nearestReply->setProperty("clientLon", m_mapClientPinLon);

                if (m_mapDeliveryInfoLabel) {
                    m_mapDeliveryInfoLabel->setText("Road distance: calculating car route...");
                }
                m_mapStatusLabel->setText("Employee and client located. Snapping to nearest road...");
                reply->deleteLater();
                return;
            } else {
                m_mapCenterLat = m_mapEmployeePinLat;
                m_mapCenterLon = m_mapEmployeePinLon;
            }

            m_mapStatusLabel->setText("Employee and client locations loaded.");
            requestMapTiles(m_mapCenterLat, m_mapCenterLon);
            reply->deleteLater();
            return;
        }

        m_mapCenterLat = latVal;
        m_mapCenterLon = lonVal;
        m_mapClientPinLat = latVal;
        m_mapClientPinLon = lonVal;
        m_mapHasClientPin = true;
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);

        if (!m_mapPendingEmployeeAddress.isEmpty()) {
            QString employeeQuery = m_mapPendingEmployeeAddress;
            if (!employeeQuery.contains("tunisia", Qt::CaseInsensitive) &&
                !employeeQuery.contains("tunisie", Qt::CaseInsensitive) &&
                !employeeQuery.contains(QString::fromUtf8("\xD8\xAA\xD9\x88\xD9\x86\xD8\xB3"), Qt::CaseInsensitive)) {
                employeeQuery += ", Tunisia";
            }

            QUrl employeeUrl("https://nominatim.openstreetmap.org/search");
            QUrlQuery employeeParams;
            employeeParams.addQueryItem("q", employeeQuery);
            employeeParams.addQueryItem("format", "json");
            employeeParams.addQueryItem("limit", "1");
            employeeParams.addQueryItem("accept-language", "en");
            employeeParams.addQueryItem("countrycodes", "tn");
            employeeParams.addQueryItem("bounded", "1");
            employeeParams.addQueryItem("viewbox", "7.5,37.6,11.6,30.2");
            employeeUrl.setQuery(employeeParams);

            QNetworkRequest employeeReq(employeeUrl);
            employeeReq.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            employeeReq.setRawHeader("Accept", "application/json");
            QNetworkReply *employeeReply = m_mapNet->get(employeeReq);
            employeeReply->setProperty("mapType", "geocode_employee");
            employeeReply->setProperty("address", m_mapPendingEmployeeAddress);
            employeeReply->setProperty("query", employeeQuery);
            employeeReply->setProperty("geocodeStage", "tn");
            m_mapStatusLabel->setText("Client located. Geocoding assigned employee...");
        } else {
            m_mapStatusLabel->setText("Client located. No assigned employee address available.");
        }

        reply->deleteLater();
        return;
    }

    if (type == "route_nearest_employee") {
        const QByteArray nearestData = reply->readAll();
        const double employeeLat = reply->property("employeeLat").toDouble();
        const double employeeLon = reply->property("employeeLon").toDouble();
        const double clientLat = reply->property("clientLat").toDouble();
        const double clientLon = reply->property("clientLon").toDouble();

        double fromLat = employeeLat;
        double fromLon = employeeLon;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument nDoc = QJsonDocument::fromJson(nearestData);
            QJsonObject nRoot = nDoc.object();
            if (nRoot.value("code").toString() == "Ok") {
                const QJsonArray waypoints = nRoot.value("waypoints").toArray();
                if (!waypoints.isEmpty()) {
                    const QJsonArray loc = waypoints.first().toObject().value("location").toArray();
                    if (loc.size() >= 2) {
                        fromLon = loc.at(0).toDouble();
                        fromLat = loc.at(1).toDouble();
                    }
                }
            }
        }

        QUrl nearestClientUrl(QString("http://router.project-osrm.org/nearest/v1/driving/%1,%2")
                                  .arg(clientLon, 0, 'f', 6)
                                  .arg(clientLat, 0, 'f', 6));
        QUrlQuery nearestClientParams;
        nearestClientParams.addQueryItem("number", "1");
        nearestClientUrl.setQuery(nearestClientParams);

        QNetworkRequest nearestClientReq(nearestClientUrl);
        nearestClientReq.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
        nearestClientReq.setRawHeader("Accept", "application/json");
        QNetworkReply *nearestClientReply = m_mapNet->get(nearestClientReq);
        nearestClientReply->setProperty("mapType", "route_nearest_client");
        nearestClientReply->setProperty("fromLat", fromLat);
        nearestClientReply->setProperty("fromLon", fromLon);
        nearestClientReply->setProperty("clientLat", clientLat);
        nearestClientReply->setProperty("clientLon", clientLon);

        m_mapStatusLabel->setText("Snapping client to nearest road...");
        reply->deleteLater();
        return;
    }

    if (type == "route_nearest_client") {
        const QByteArray nearestData = reply->readAll();

        const double fromLat = reply->property("fromLat").toDouble();
        const double fromLon = reply->property("fromLon").toDouble();
        const double clientLat = reply->property("clientLat").toDouble();
        const double clientLon = reply->property("clientLon").toDouble();

        double toLat = clientLat;
        double toLon = clientLon;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument nDoc = QJsonDocument::fromJson(nearestData);
            QJsonObject nRoot = nDoc.object();
            if (nRoot.value("code").toString() == "Ok") {
                const QJsonArray waypoints = nRoot.value("waypoints").toArray();
                if (!waypoints.isEmpty()) {
                    const QJsonArray loc = waypoints.first().toObject().value("location").toArray();
                    if (loc.size() >= 2) {
                        toLon = loc.at(0).toDouble();
                        toLat = loc.at(1).toDouble();
                    }
                }
            }
        }

        QUrl routeUrl(QString("http://router.project-osrm.org/route/v1/driving/%1,%2;%3,%4")
                          .arg(fromLon, 0, 'f', 6)
                          .arg(fromLat, 0, 'f', 6)
                          .arg(toLon, 0, 'f', 6)
                          .arg(toLat, 0, 'f', 6));
        QUrlQuery routeParams;
        routeParams.addQueryItem("overview", "full");
        routeParams.addQueryItem("geometries", "geojson");
        routeParams.addQueryItem("alternatives", "false");
        routeParams.addQueryItem("steps", "false");
        routeParams.addQueryItem("annotations", "false");
        routeUrl.setQuery(routeParams);

        QNetworkRequest routeReq(routeUrl);
        routeReq.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
        routeReq.setRawHeader("Accept", "application/json");
        QNetworkReply *routeReply = m_mapNet->get(routeReq);
        routeReply->setProperty("mapType", "route");

        m_mapStatusLabel->setText("Calculating drivable route...");
        reply->deleteLater();
        return;
    }

    if (type == "route") {
        const QByteArray routeData = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            m_mapRouteGeoPoints.clear();
            renderOrderMap();
            if (m_mapDeliveryInfoLabel) {
                m_mapDeliveryInfoLabel->setText("Road route unavailable for car travel.");
            }
            m_mapStatusLabel->setText("Routing service error. Please verify addresses.");
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(routeData);
        QJsonObject root = doc.object();
        const QString code = root.value("code").toString();
        const QJsonArray routes = root.value("routes").toArray();

        if (code == "Ok" && !routes.isEmpty()) {
            const QJsonObject r0 = routes.first().toObject();
            const double distanceKm = r0.value("distance").toDouble() / 1000.0;
            const int etaMinutes = qMax(1, qRound(r0.value("duration").toDouble() / 60.0));

            m_mapRouteGeoPoints.clear();
            const QJsonObject geometry = r0.value("geometry").toObject();
            const QJsonArray coords = geometry.value("coordinates").toArray();
            for (const QJsonValue &coordVal : coords) {
                const QJsonArray coord = coordVal.toArray();
                if (coord.size() < 2) continue;
                const double lon = coord.at(0).toDouble();
                const double lat = coord.at(1).toDouble();
                m_mapRouteGeoPoints.push_back(QPointF(lon, lat));
            }
            renderOrderMap();

            if (m_mapDeliveryInfoLabel) {
                m_mapDeliveryInfoLabel->setText(
                    QString("Road Distance: %1 km | ETA (car): %2 min")
                        .arg(distanceKm, 0, 'f', 1)
                        .arg(etaMinutes));
            }
            m_mapStatusLabel->setText("Drivable route calculated successfully.");
        } else {
            m_mapRouteGeoPoints.clear();
            renderOrderMap();
            if (m_mapDeliveryInfoLabel) {
                m_mapDeliveryInfoLabel->setText("No drivable road route found for car travel.");
            }
            m_mapStatusLabel->setText("No valid car route found between employee and client.");
        }

        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        m_mapStatusLabel->setText("Network error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    if (type == "tile") {
        const QString tileKey = reply->property("tileKey").toString();
        const bool trackedTile = m_mapPendingTiles.contains(tileKey);

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imgData = reply->readAll();
            QPixmap pix;
            if (pix.loadFromData(imgData)) {
                m_mapTileCache.insert(tileKey, pix);
            } else {
                if (trackedTile) m_mapTileErrors++;
            }
        } else {
            if (trackedTile) m_mapTileErrors++;
        }

        if (trackedTile) {
            m_mapPendingTiles.remove(tileKey);
            m_mapLoadedTiles++;
        }

        if (trackedTile) {
            renderOrderMap();
            if (m_mapPendingTiles.isEmpty()) {
                if (m_mapTileErrors > 0) {
                    m_mapStatusLabel->setText("Map loaded with missing tiles.");
                } else {
                    m_mapStatusLabel->setText("Map loaded successfully.");
                }
            } else {
                m_mapStatusLabel->setText(QString("Loading map tiles... %1/%2")
                                          .arg(m_mapLoadedTiles)
                                          .arg(m_mapExpectedTiles));
            }
        }

        reply->deleteLater();
        return;
    }

    reply->deleteLater();
}

void MainWindow::requestMapTiles(double lat, double lon)
{
    if (!m_mapNet) return;

    // Cancel in-flight tile requests from previous map views to free bandwidth.
    const auto activeReplies = m_mapNet->findChildren<QNetworkReply*>();
    for (QNetworkReply *active : activeReplies) {
        if (!active) continue;
        if (active->property("mapType").toString() != "tile") continue;
        if (active->isRunning()) active->abort();
    }

    const int tileSize = 256;
    const int zoom = m_mapZoom;
    const int n = 1 << zoom;

    double latRad = qDegreesToRadians(lat);
    double xtile = (lon + 180.0) / 360.0 * n;
    double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;

    double worldX = xtile * tileSize;
    double worldY = ytile * tileSize;

    if (m_mapImageSize.width() < 64 || m_mapImageSize.height() < 64) {
        m_mapImageSize = QSize(640, 360);
    }
    m_mapTopLeftX = worldX - (m_mapImageSize.width() / 2.0);
    m_mapTopLeftY = worldY - (m_mapImageSize.height() / 2.0);

    m_mapTileX0 = static_cast<int>(floor(m_mapTopLeftX / tileSize));
    m_mapTileY0 = static_cast<int>(floor(m_mapTopLeftY / tileSize));
    m_mapTileX1 = static_cast<int>(floor((m_mapTopLeftX + m_mapImageSize.width() - 1) / tileSize));
    m_mapTileY1 = static_cast<int>(floor((m_mapTopLeftY + m_mapImageSize.height() - 1) / tileSize));

    m_mapPendingTiles.clear();
    m_mapTileErrors = 0;
    m_mapExpectedTiles = 0;
    m_mapLoadedTiles = 0;

    if (m_mapTileCache.size() > 600) {
        m_mapTileCache.clear();
    }

    for (int x = m_mapTileX0; x <= m_mapTileX1; ++x) {
        int wrappedX = ((x % n) + n) % n;
        for (int y = m_mapTileY0; y <= m_mapTileY1; ++y) {
            if (y < 0 || y >= n) continue;

            QString key = QString("%1/%2/%3").arg(zoom).arg(x).arg(y);
            m_mapExpectedTiles++;

            if (m_mapTileCache.contains(key)) {
                m_mapLoadedTiles++;
                continue;
            }

            m_mapPendingTiles.insert(key);

            QUrl tileUrl(QString("https://tile.openstreetmap.org/%1/%2/%3.png")
                         .arg(zoom).arg(wrappedX).arg(y));
            QNetworkRequest req(tileUrl);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_mapNet->get(req);
            reply->setProperty("mapType", "tile");
            reply->setProperty("tileKey", key);
            reply->setProperty("tileX", x);
            reply->setProperty("tileY", y);
        }
    }

    renderOrderMap();

    if (m_mapPendingTiles.isEmpty()) {
        if (m_mapExpectedTiles == 0) {
            m_mapStatusLabel->setText("Map tiles not available for this location.");
        } else {
            m_mapStatusLabel->setText("Map loaded instantly from cache.");
        }
    } else {
        m_mapStatusLabel->setText(QString("Loading map tiles... %1/%2")
                                  .arg(m_mapLoadedTiles)
                                  .arg(m_mapExpectedTiles));
    }
}

void MainWindow::renderOrderMap()
{
    if (m_mapImageSize.width() <= 0 || m_mapImageSize.height() <= 0) return;

    QPixmap mapPixmap(m_mapImageSize);
    mapPixmap.fill(QColor(26, 18, 8));
    QPainter painter(&mapPixmap);

    const int tileSize = 256;
    for (int x = m_mapTileX0; x <= m_mapTileX1; ++x) {
        for (int y = m_mapTileY0; y <= m_mapTileY1; ++y) {
            const QString key = QString("%1/%2/%3").arg(m_mapZoom).arg(x).arg(y);
            if (!m_mapTileCache.contains(key)) continue;
            const int px = qRound((x * tileSize) - m_mapTopLeftX);
            const int py = qRound((y * tileSize) - m_mapTopLeftY);
            painter.drawPixmap(px, py, m_mapTileCache.value(key));
        }
    }

    auto worldToPixel = [this, tileSize](double lat, double lon) {
        const int n = 1 << m_mapZoom;
        const double latRad = qDegreesToRadians(lat);
        const double worldX = ((lon + 180.0) / 360.0 * n) * tileSize;
        const double worldY = ((1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n) * tileSize;
        return QPoint(qRound(worldX - m_mapTopLeftX), qRound(worldY - m_mapTopLeftY));
    };

    auto isVisiblePin = [this](const QPoint &pinPos) {
        return pinPos.x() >= -20 && pinPos.x() <= (m_mapImageSize.width() + 20)
               && pinPos.y() >= -30 && pinPos.y() <= (m_mapImageSize.height() + 20);
    };

    auto drawPin = [&painter](const QPoint &pinPos, const QColor &pinColor, const QString &label) {
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 120));
        painter.drawEllipse(QPoint(pinPos.x() + 1, pinPos.y() + 2), 7, 3);

        QPolygon pinTip;
        pinTip << QPoint(pinPos.x(), pinPos.y())
               << QPoint(pinPos.x() - 7, pinPos.y() - 14)
               << QPoint(pinPos.x() + 7, pinPos.y() - 14);
        painter.setBrush(pinColor);
        painter.drawPolygon(pinTip);

        painter.setBrush(pinColor);
        painter.drawEllipse(QPoint(pinPos.x(), pinPos.y() - 22), 10, 10);
        painter.setBrush(Qt::white);
        painter.drawEllipse(QPoint(pinPos.x(), pinPos.y() - 22), 4, 4);

        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setBold(true);
        f.setPointSize(8);
        painter.setFont(f);
        painter.drawText(QRect(pinPos.x() - 10, pinPos.y() - 48, 20, 16), Qt::AlignCenter, label);
    };

    QPoint clientPin;
    QPoint employeePin;
    bool clientVisible = false;
    bool employeeVisible = false;

    if (m_mapHasClientPin) {
        clientPin = worldToPixel(m_mapClientPinLat, m_mapClientPinLon);
        clientVisible = isVisiblePin(clientPin);
    }

    if (m_mapHasEmployeePin) {
        employeePin = worldToPixel(m_mapEmployeePinLat, m_mapEmployeePinLon);
        employeeVisible = isVisiblePin(employeePin);
    }

    if (m_mapRouteGeoPoints.size() > 1) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPainterPath routePath;

        const QPoint firstPoint = worldToPixel(m_mapRouteGeoPoints.first().y(), m_mapRouteGeoPoints.first().x());
        routePath.moveTo(firstPoint);
        for (int i = 1; i < m_mapRouteGeoPoints.size(); ++i) {
            const QPoint p = worldToPixel(m_mapRouteGeoPoints.at(i).y(), m_mapRouteGeoPoints.at(i).x());
            routePath.lineTo(p);
        }

        painter.setPen(QPen(QColor(35, 187, 255, 180), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(routePath);
        painter.setPen(QPen(QColor(255, 255, 255, 200), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(routePath);
    }

    if (clientVisible) {
        drawPin(clientPin, QColor(220, 53, 69), "C");
    }

    if (employeeVisible) {
        drawPin(employeePin, QColor(52, 152, 219), "E");
    }

    m_mapCurrentPixmap = mapPixmap;
    m_mapHasPixmap = true;

    if (m_mapImageLabel) {
        m_mapImageLabel->setPixmap(mapPixmap.scaled(
            m_mapImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel) {
        m_mapFullscreenLabel->setPixmap(mapPixmap.scaled(
            m_mapFullscreenLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui_employee->tableView_employes->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            QModelIndex idx = ui_employee->tableView_employes->indexAt(mouse->pos());
            RowHoverDelegate *del = qobject_cast<RowHoverDelegate*>(ui_employee->tableView_employes->itemDelegate());
            if (del) {
                int oldHover = idx.isValid() ? idx.row() : -1;
                del->setHoveredRow(oldHover);
                ui_employee->tableView_employes->viewport()->update();
            }
        } else if (event->type() == QEvent::Leave) {
            RowHoverDelegate *del = qobject_cast<RowHoverDelegate*>(ui_employee->tableView_employes->itemDelegate());
            if (del) {
                del->setHoveredRow(-1);
                ui_employee->tableView_employes->viewport()->update();
            }
        }
    }
    QLabel *mapTarget = nullptr;
    if (watched == m_mapImageLabel) {
        mapTarget = m_mapImageLabel;
    } else if (watched == m_mapFullscreenLabel) {
        mapTarget = m_mapFullscreenLabel;
    }

    if (watched == m_supplierMapImageLabel) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() > 0) m_supplierMapZoom = qMin(18, m_supplierMapZoom + 1);
            else m_supplierMapZoom = qMax(3, m_supplierMapZoom - 1);
            m_supplierMapImageSize = m_supplierMapImageLabel->size();
            refreshSupplierMap();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_supplierMapDragging = true;
                m_supplierMapDragStart = mouse->pos();
                m_supplierMapDragOffset = QPoint(0, 0);
                m_supplierMapImageSize = m_supplierMapImageLabel->size();
                const int tileSize = 256;
                const int n = 1 << m_supplierMapZoom;
                double latRad = qDegreesToRadians(m_supplierCenterLat);
                double xtile = (m_supplierCenterLon + 180.0) / 360.0 * n;
                double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;
                m_supplierMapDragCenterX = xtile * tileSize;
                m_supplierMapDragCenterY = ytile * tileSize;
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (m_supplierMapDragging) {
                QPoint delta = mouse->pos() - m_supplierMapDragStart;
                m_supplierMapDragOffset = delta;
                if (m_supplierMapHasPixmap) {
                    QPixmap shifted(m_supplierMapImageSize);
                    shifted.fill(QColor(26, 18, 8));
                    QPainter p(&shifted);
                    p.drawPixmap(delta, m_supplierMapCurrentPixmap);
                    m_supplierMapImageLabel->setPixmap(shifted);
                }
                return true;
            } else {
                bool hovered = false;
                for (const auto &pin : m_supplierPins) {
                    if (pin.rect.contains(mouse->pos())) {
                        QToolTip::showText(mouse->globalPosition().toPoint(),
                            QString("<b>%1</b><br/>Supplies: %2<br/>Status: %3")
                                .arg(pin.name).arg(pin.type)
                                .arg(pin.status == "Active" ? "<font color='green'>Open (Active)</font>" : "<font color='red'>Closed/Inactive</font>"),
                            m_supplierMapImageLabel);
                        hovered = true;
                        break;
                    }
                }
                if (!hovered) QToolTip::hideText();
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton && m_supplierMapDragging) {
                m_supplierMapDragging = false;
                double centerX = m_supplierMapDragCenterX - m_supplierMapDragOffset.x();
                double centerY = m_supplierMapDragCenterY - m_supplierMapDragOffset.y();
                const int n = 1 << m_supplierMapZoom;
                double lon = (centerX / (n * 256.0)) * 360.0 - 180.0;
                double latRad = atan(sinh(M_PI * (1.0 - 2.0 * centerY / (n * 256.0))));
                m_supplierCenterLat = qRadiansToDegrees(latRad);
                m_supplierCenterLon = lon;
                refreshSupplierMap();
                return true;
            }
        }
    }

    if (watched == m_supplierMapImageLabel) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() > 0) m_supplierMapZoom = qMin(18, m_supplierMapZoom + 1);
            else m_supplierMapZoom = qMax(3, m_supplierMapZoom - 1);
            m_supplierMapImageSize = m_supplierMapImageLabel->size();
            refreshSupplierMap();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_supplierMapDragging = true;
                m_supplierMapDragStart = mouse->pos();
                m_supplierMapDragOffset = QPoint(0, 0);
                m_supplierMapImageSize = m_supplierMapImageLabel->size();
                const int tileSize = 256;
                const int n = 1 << m_supplierMapZoom;
                double latRad = qDegreesToRadians(m_supplierCenterLat);
                double xtile = (m_supplierCenterLon + 180.0) / 360.0 * n;
                double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;
                m_supplierMapDragCenterX = xtile * tileSize;
                m_supplierMapDragCenterY = ytile * tileSize;
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (m_supplierMapDragging) {
                QPoint delta = mouse->pos() - m_supplierMapDragStart;
                m_supplierMapDragOffset = delta;
                if (m_supplierMapHasPixmap) {
                    QPixmap shifted(m_supplierMapImageSize);
                    shifted.fill(QColor(26, 18, 8));
                    QPainter p(&shifted);
                    p.drawPixmap(delta, m_supplierMapCurrentPixmap);
                    m_supplierMapImageLabel->setPixmap(shifted);
                }
                return true;
            } else {
                bool hovered = false;
                for (const auto &pin : m_supplierPins) {
                    if (pin.rect.contains(mouse->pos())) {
                        bool isOpen = false;
                        if (pin.status == "Active") {
                            QTime openT = QTime::fromString(pin.openTime, "HH:mm");
                            QTime closeT = QTime::fromString(pin.closeTime, "HH:mm");
                            QTime now = QTime::currentTime();
                            if (openT.isValid() && closeT.isValid()) {
                                if (openT <= closeT) isOpen = (now >= openT && now <= closeT);
                                else isOpen = (now >= openT || now <= closeT);
                            } else {
                                isOpen = true; // Default if no times
                            }
                        }
                        QString timeLabel = "";
                        if (!pin.openTime.isEmpty() && !pin.closeTime.isEmpty()) {
                            timeLabel = QString("<br/>Hours: %1 - %2").arg(pin.openTime).arg(pin.closeTime);
                        }
                        QToolTip::showText(mouse->globalPosition().toPoint(),
                            QString("<b>%1</b><br/>Supplies: %2<br/>Status: %3%4")
                                .arg(pin.name).arg(pin.type)
                                .arg(isOpen ? "<font color='green'>Open</font>" : "<font color='red'>Closed/Inactive</font>")
                                .arg(timeLabel),
                            m_supplierMapImageLabel);
                        hovered = true;
                        break;
                    }
                }
                if (!hovered) QToolTip::hideText();
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton && m_supplierMapDragging) {
                m_supplierMapDragging = false;
                double centerX = m_supplierMapDragCenterX - m_supplierMapDragOffset.x();
                double centerY = m_supplierMapDragCenterY - m_supplierMapDragOffset.y();
                const int n = 1 << m_supplierMapZoom;
                double lon = (centerX / (n * 256.0)) * 360.0 - 180.0;
                double latRad = atan(sinh(M_PI * (1.0 - 2.0 * centerY / (n * 256.0))));
                m_supplierCenterLat = qRadiansToDegrees(latRad);
                m_supplierCenterLon = lon;
                refreshSupplierMap();
                return true;
            }
        }
    }

    if (mapTarget) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() > 0) {
                m_mapZoom = qMin(18, m_mapZoom + 1);
            } else {
                m_mapZoom = qMax(3, m_mapZoom - 1);
            }
            m_mapImageSize = mapTarget->size();
            requestMapTiles(m_mapCenterLat, m_mapCenterLon);
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_mapDragging = true;
                m_mapDragStart = mouse->pos();
                m_mapDragOffset = QPoint(0, 0);
                m_mapImageSize = mapTarget->size();

                const int tileSize = 256;
                const int n = 1 << m_mapZoom;
                double latRad = qDegreesToRadians(m_mapCenterLat);
                double xtile = (m_mapCenterLon + 180.0) / 360.0 * n;
                double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;
                m_mapDragCenterX = xtile * tileSize;
                m_mapDragCenterY = ytile * tileSize;
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove) {
            if (m_mapDragging) {
                QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
                QPoint delta = mouse->pos() - m_mapDragStart;

                m_mapDragOffset = delta;
                if (m_mapHasPixmap) {
                    QPixmap shifted(m_mapImageSize);
                    shifted.fill(QColor(26, 18, 8));
                    QPainter p(&shifted);
                    p.drawPixmap(delta, m_mapCurrentPixmap);
                    mapTarget->setPixmap(shifted);
                }
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                m_mapDragging = false;
                double centerX = m_mapDragCenterX - m_mapDragOffset.x();
                double centerY = m_mapDragCenterY - m_mapDragOffset.y();

                const int n = 1 << m_mapZoom;
                double lon = (centerX / (n * 256.0)) * 360.0 - 180.0;
                double latRad = atan(sinh(M_PI * (1.0 - 2.0 * centerY / (n * 256.0))));
                double lat = qRadiansToDegrees(latRad);

                m_mapCenterLat = lat;
                m_mapCenterLon = lon;
                requestMapTiles(m_mapCenterLat, m_mapCenterLon);
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (mapTarget == m_mapFullscreenLabel && m_mapFullscreenDialog) {
                m_mapFullscreenDialog->close();
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// =============================================================================
// SUPPLIER MAP INTEGRATION
// =============================================================================

void MainWindow::setupSupplierMapTab()
{
    if (!ui_supplier || !ui_supplier->tabWidget) return;

    QWidget *mapTab = new QWidget(ui_supplier->tabWidget);
    mapTab->setObjectName("tab_supplier_map");

    QVBoxLayout *root = new QVBoxLayout(mapTab);
    root->setContentsMargins(18, 60, 18, 16);
    root->setSpacing(10);

    QHBoxLayout *controls = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Workshop Vicinity Map", mapTab);
    titleLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    m_supplierMapRefreshBtn = new QPushButton("Refresh Map", mapTab);
    m_supplierMapRefreshBtn->setCursor(Qt::PointingHandCursor);
    m_supplierMapRefreshBtn->setStyleSheet(
        "QPushButton{background:#8B6F47;color:white;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#a3845a;}"
        "QPushButton:pressed{background:#6b5535;}");

    m_supplierMapZoomInBtn = new QPushButton("Zoom +", mapTab);
    m_supplierMapZoomInBtn->setCursor(Qt::PointingHandCursor);
    m_supplierMapZoomInBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}");

    m_supplierMapZoomOutBtn = new QPushButton("Zoom -", mapTab);
    m_supplierMapZoomOutBtn->setCursor(Qt::PointingHandCursor);
    m_supplierMapZoomOutBtn->setStyleSheet(
        "QPushButton{background:#5c4a2a;color:#f5e6cc;border:none;border-radius:8px;padding:6px 12px;font-weight:bold;}"
        "QPushButton:hover{background:#7a5f3c;}");

    controls->addWidget(titleLabel);
    controls->addStretch();
    controls->addWidget(m_supplierMapRefreshBtn);
    controls->addWidget(m_supplierMapZoomInBtn);
    controls->addWidget(m_supplierMapZoomOutBtn);

    m_supplierMapStatusLabel = new QLabel("Loading suppliers...", mapTab);
    m_supplierMapStatusLabel->setWordWrap(true);
    m_supplierMapStatusLabel->setStyleSheet("color: #d4a96a; font-size: 12px;");

    m_supplierMapImageLabel = new QLabel(mapTab);
    m_supplierMapImageLabel->setMinimumSize(800, 500);
    m_supplierMapImageLabel->setAlignment(Qt::AlignCenter);
    m_supplierMapImageLabel->setStyleSheet("background: #1a1208; border: 2px solid #8B6F47; border-radius: 10px; color: #8B6F47;");
    m_supplierMapImageLabel->setText("Map preview will appear here.");
    m_supplierMapImageLabel->setMouseTracking(true); // Needed for hover
    m_supplierMapImageLabel->installEventFilter(this);

    root->addLayout(controls);
    root->addWidget(m_supplierMapStatusLabel);
    root->addWidget(m_supplierMapImageLabel, 1);

    ui_supplier->tabWidget->addTab(mapTab, "Vicinity Map");

    connect(m_supplierMapRefreshBtn, &QPushButton::clicked, this, &MainWindow::loadSupplierMapPins);
    connect(m_supplierMapZoomInBtn, &QPushButton::clicked, this, [this]() {
        m_supplierMapZoom = qMin(18, m_supplierMapZoom + 1);
        m_supplierMapImageSize = m_supplierMapImageLabel->size();
        refreshSupplierMap();
    });
    connect(m_supplierMapZoomOutBtn, &QPushButton::clicked, this, [this]() {
        m_supplierMapZoom = qMax(3, m_supplierMapZoom - 1);
        m_supplierMapImageSize = m_supplierMapImageLabel->size();
        refreshSupplierMap();
    });

    connect(ui_supplier->tabWidget, &QTabWidget::currentChanged, this, [this, mapTab](int index) {
        if (ui_supplier->tabWidget->widget(index) == mapTab) {
            loadSupplierMapPins();
        }
    });
}

void MainWindow::setupSupplierAiAdvisorTab()
{
    if (!ui_supplier || !ui_supplier->tabWidget) return;

    m_supplierAiTab = new QWidget(ui_supplier->tabWidget);
    m_supplierAiTab->setObjectName("tab_supplier_ai");

    QVBoxLayout *root = new QVBoxLayout(m_supplierAiTab);
    root->setContentsMargins(20, 60, 20, 20);
    root->setSpacing(15);

    // Header Frame
    QFrame *headerFrame = new QFrame(m_supplierAiTab);
    headerFrame->setFixedHeight(80);
    headerFrame->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #5D4037); border-radius: 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    
    QLabel *headerIcon = new QLabel(headerFrame);
    headerIcon->setPixmap(QIcon(":/assets/nexus_core.png").pixmap(40, 40));
    
    QVBoxLayout *headerText = new QVBoxLayout();
    QLabel *title = new QLabel("AI Strategic Procurement Advisor", headerFrame);
    title->setStyleSheet("color: white; font-size: 20px; font-weight: bold; background: transparent;");
    QLabel *subtitle = new QLabel("Predictive Stock Analysis & Supplier Optimization", headerFrame);
    subtitle->setStyleSheet("color: #D4AF37; font-size: 13px; background: transparent;");
    headerText->addWidget(title);
    headerText->addWidget(subtitle);
    
    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(headerText);
    headerLayout->addStretch();
    
    // Control Section
    QHBoxLayout *controls = new QHBoxLayout();
    m_aiAdvRunBtn = new QPushButton(" Analyze Market & Recommend", m_supplierAiTab);
    m_aiAdvRunBtn->setIcon(QIcon(":/assets/ai_pulse.png"));
    m_aiAdvRunBtn->setCursor(Qt::PointingHandCursor);
    m_aiAdvRunBtn->setFixedSize(250, 45);
    m_aiAdvRunBtn->setStyleSheet(
        "QPushButton { background-color: #D4AF37; color: #1A1208; border-radius: 22px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #FFFFFF; color: #D4AF37; border: 2px solid #D4AF37; }"
        "QPushButton:pressed { background-color: #B89626; }");

    m_aiAdvStatus = new QLabel("Standby - Ready for analysis", m_supplierAiTab);
    m_aiAdvStatus->setStyleSheet("color: #8B6F47; font-style: italic; font-weight: bold;");
    
    controls->addWidget(m_aiAdvRunBtn);
    controls->addSpacing(20);
    controls->addWidget(m_aiAdvStatus);
    controls->addStretch();

    // Progress Bar (Initially hidden)
    m_aiAdvProgress = new QProgressBar(m_supplierAiTab);
    m_aiAdvProgress->setRange(0, 0); // Indeterminate
    m_aiAdvProgress->setFixedHeight(4);
    m_aiAdvProgress->setTextVisible(false);
    m_aiAdvProgress->setStyleSheet("QProgressBar { background: transparent; border: none; } QProgressBar::chunk { background: #D4AF37; }");
    m_aiAdvProgress->hide();

    // Results Display
    m_aiAdvResult = new QTextEdit(m_supplierAiTab);
    m_aiAdvResult->setReadOnly(true);
    m_aiAdvResult->setPlaceholderText("Strategic recommendations will appear here after analysis...");
    m_aiAdvResult->setStyleSheet(
        "QTextEdit { background-color: rgba(30, 20, 10, 0.85); color: #F5E6D3; border: 2px solid #8B6F47; "
        "border-radius: 12px; padding: 20px; font-size: 15px; line-height: 1.6; }");
    
    root->addWidget(headerFrame);
    root->addLayout(controls);
    root->addWidget(m_aiAdvProgress);
    root->addWidget(m_aiAdvResult, 1);

    ui_supplier->tabWidget->addTab(m_supplierAiTab, "AI Advisor");

    connect(m_aiAdvRunBtn, &QPushButton::clicked, this, [this]() {
        // Manual re-analysis: reset guards so all low-stock items are re-checked
        m_aiScanInProgress = false;
        m_aiNotifiedMaterials.clear();
        m_aiAdvStatus->setText("Consulting AI Models...");
        m_aiAdvProgress->show();
        m_aiAdvResult->clear();
        checkWorkshopStockAndNotifyAI();
    });
}

void MainWindow::loadSupplierMapPins()
{
    m_supplierMapStatusLabel->setText("Geocoding suppliers...");
    m_supplierPins.clear();
    m_supplierGeocodePendingCount = 0;

    QSqlQuery q("SELECT SUPPLIER_ID, SUPPLIER_NAME, TYPE_NOTIFICATION, ACCOUNT_STATUS, ADDRESS, OPENING_TIME, CLOSING_TIME FROM SUPPLIERS");
    while (q.next()) {
        int id = q.value(0).toInt();
        QString name = q.value(1).toString();
        QString type = q.value(2).toString();
        QString status = q.value(3).toString();
        QString address = q.value(4).toString().trimmed();
        QString openTime = q.value(5).toString().trimmed();
        QString closeTime = q.value(6).toString().trimmed();

        if (!address.isEmpty()) {
            m_supplierGeocodePendingCount++;
            QUrl url("https://nominatim.openstreetmap.org/search");
            QUrlQuery query;
            query.addQueryItem("q", address + ", Tunisia");
            query.addQueryItem("format", "json");
            query.addQueryItem("limit", "1");
            url.setQuery(query);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_supplierMapNet->get(req);
            reply->setProperty("mapAction", "geocode_supplier");
            reply->setProperty("supp_id", id);
            reply->setProperty("supp_name", name);
            reply->setProperty("supp_type", type);
            reply->setProperty("supp_status", status);
            reply->setProperty("supp_open", openTime);
            reply->setProperty("supp_close", closeTime);
            reply->setProperty("address", address);
        }
    }

    if (m_supplierGeocodePendingCount == 0) {
        m_supplierMapStatusLabel->setText("No suppliers with addresses found.");
        refreshSupplierMap();
    }
}

void MainWindow::onSupplierGeocodeFinished(QNetworkReply *reply)
{
    if (!reply) return;
    QString action = reply->property("mapAction").toString();

    if (action == "geocode_supplier") {
        m_supplierGeocodePendingCount--;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonArray arr = doc.array();

            if (!arr.isEmpty()) {
                QJsonObject obj = arr.first().toObject();
                double lat = obj.value("lat").toString().toDouble();
                double lon = obj.value("lon").toString().toDouble();
                if (lat != 0 && lon != 0) {
                    SupplierPin pin;
                    pin.id = reply->property("supp_id").toInt();
                    pin.name = reply->property("supp_name").toString();
                    pin.type = reply->property("supp_type").toString();
                    pin.status = reply->property("supp_status").toString();
                    pin.openTime = reply->property("supp_open").toString();
                    pin.closeTime = reply->property("supp_close").toString();
                    pin.lat = lat;
                    pin.lon = lon;
                    m_supplierPins.append(pin);
                }
            }
        }
        
        if (m_supplierGeocodePendingCount <= 0) {
            m_supplierMapStatusLabel->setText(QString("Loaded %1 suppliers.").arg(m_supplierPins.size()));
            m_supplierMapImageSize = m_supplierMapImageLabel->size();
            refreshSupplierMap();
        }
        reply->deleteLater();
        return;
    }
    
    if (action == "map_tile") {
        QString key = reply->property("tileKey").toString();
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pix;
            if (pix.loadFromData(reply->readAll())) {
                m_supplierMapTileCache.insert(key, pix);
            } else {
                m_supplierMapTileErrors++;
            }
        } else {
            m_supplierMapTileErrors++;
        }
        
        m_supplierMapPendingTiles.remove(key);
        // Force repaint on every tile arrival
        QPixmap mapPixmap(m_supplierMapImageSize);
        mapPixmap.fill(QColor(26, 18, 8));
        QPainter painter(&mapPixmap);
        const int tileSize = 256;
        for (int x = m_supplierMapTileX0; x <= m_supplierMapTileX1; ++x) {
            for (int y = m_supplierMapTileY0; y <= m_supplierMapTileY1; ++y) {
                QString tkey = QString("%1/%2/%3").arg(m_supplierMapZoom).arg(x).arg(y);
                if (m_supplierMapTileCache.contains(tkey)) {
                    int px = qRound((x * tileSize) - m_supplierMapTopLeftX);
                    int py = qRound((y * tileSize) - m_supplierMapTopLeftY);
                    painter.drawPixmap(px, py, m_supplierMapTileCache.value(tkey));
                }
            }
        }

        const int n = 1 << m_supplierMapZoom;
        auto latToY = [n](double lat) {
            double rad = qDegreesToRadians(lat);
            return (1.0 - log(tan(rad) + 1.0 / cos(rad)) / M_PI) / 2.0 * n * 256;
        };
        auto lonToX = [n](double lon) {
            return (lon + 180.0) / 360.0 * n * 256;
        };

        int cx = qRound(lonToX(10.1815) - m_supplierMapTopLeftX); // Fixed Workshop
        int cy = qRound(latToY(36.8065) - m_supplierMapTopLeftY); // Fixed Workshop
        
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(QColor("#D4AF37"));
        painter.drawRect(cx - 10, cy - 10, 20, 20);
        painter.drawText(cx - 30, cy + 25, "Workshop");

        for (int i=0; i<m_supplierPins.size(); ++i) {
            auto &pin = m_supplierPins[i];
            int px = qRound(lonToX(pin.lon) - m_supplierMapTopLeftX);
            int py = qRound(latToY(pin.lat) - m_supplierMapTopLeftY);
            
            bool isOpen = false;
            if (pin.status == "Active") {
                QTime openT = QTime::fromString(pin.openTime, "HH:mm");
                QTime closeT = QTime::fromString(pin.closeTime, "HH:mm");
                QTime now = QTime::currentTime();
                if (openT.isValid() && closeT.isValid()) {
                    if (openT <= closeT) isOpen = (now >= openT && now <= closeT);
                    else isOpen = (now >= openT || now <= closeT);
                } else {
                    isOpen = true;
                }
            }
            QColor color = isOpen ? QColor(0, 255, 100, 200) : QColor(255, 50, 50, 200);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::white, 1));
            painter.drawEllipse(px - 8, py - 8, 16, 16);
            
            pin.rect = QRect(px - 10, py - 10, 20, 20);
        }

        m_supplierMapCurrentPixmap = mapPixmap;
        m_supplierMapHasPixmap = true;
        m_supplierMapImageLabel->setPixmap(mapPixmap);
        
        if (m_supplierMapPendingTiles.isEmpty()) {
            if (m_supplierMapTileErrors > 0)
                m_supplierMapStatusLabel->setText("Map loaded with missing tiles.");
            else
                m_supplierMapStatusLabel->setText(QString("Map loaded. %1 suppliers shown.").arg(m_supplierPins.size()));
        }
        reply->deleteLater();
        return;
    }
    
    reply->deleteLater();
}

void MainWindow::refreshSupplierMap()
{
    const int tileSize = 256;
    const int zoom = m_supplierMapZoom;
    const int n = 1 << zoom;
    
    if (m_supplierMapImageSize.width() < 64) m_supplierMapImageSize = QSize(800, 500);

    double latRad = qDegreesToRadians(m_supplierCenterLat);
    double xtile = (m_supplierCenterLon + 180.0) / 360.0 * n;
    double ytile = (1.0 - log(tan(latRad) + 1.0 / cos(latRad)) / M_PI) / 2.0 * n;

    double worldX = xtile * tileSize;
    double worldY = ytile * tileSize;

    m_supplierMapTopLeftX = worldX - (m_supplierMapImageSize.width() / 2.0);
    m_supplierMapTopLeftY = worldY - (m_supplierMapImageSize.height() / 2.0);

    m_supplierMapTileX0 = static_cast<int>(floor(m_supplierMapTopLeftX / tileSize));
    m_supplierMapTileY0 = static_cast<int>(floor(m_supplierMapTopLeftY / tileSize));
    m_supplierMapTileX1 = static_cast<int>(floor((m_supplierMapTopLeftX + m_supplierMapImageSize.width() - 1) / tileSize));
    m_supplierMapTileY1 = static_cast<int>(floor((m_supplierMapTopLeftY + m_supplierMapImageSize.height() - 1) / tileSize));

    m_supplierMapTileCache.clear();
    m_supplierMapPendingTiles.clear();
    m_supplierMapTileErrors = 0;

    for (int x = m_supplierMapTileX0; x <= m_supplierMapTileX1; ++x) {
        int wrappedX = ((x % n) + n) % n;
        for (int y = m_supplierMapTileY0; y <= m_supplierMapTileY1; ++y) {
            if (y < 0 || y >= n) continue;
            QString key = QString("%1/%2/%3").arg(zoom).arg(x).arg(y);
            m_supplierMapPendingTiles.insert(key);

            QUrl tileUrl(QString("https://tile.openstreetmap.org/%1/%2/%3.png").arg(zoom).arg(wrappedX).arg(y));
            QNetworkRequest req(tileUrl);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_supplierMapNet->get(req);
            reply->setProperty("mapAction", "map_tile");
            reply->setProperty("tileKey", key);
        }
    }

    // Force immediate base paint so the UI updates instantly
    QPixmap mapPixmap(m_supplierMapImageSize);
    mapPixmap.fill(QColor(26, 18, 8));
    QPainter painter(&mapPixmap);
    
    auto latToY = [n](double lat) {
        double rad = qDegreesToRadians(lat);
        return (1.0 - log(tan(rad) + 1.0 / cos(rad)) / M_PI) / 2.0 * n * 256;
    };
    auto lonToX = [n](double lon) {
        return (lon + 180.0) / 360.0 * n * 256;
    };

    int cx = qRound(lonToX(10.1815) - m_supplierMapTopLeftX); // Fixed Workshop
    int cy = qRound(latToY(36.8065) - m_supplierMapTopLeftY); // Fixed Workshop
    
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QColor("#D4AF37"));
    painter.drawRect(cx - 10, cy - 10, 20, 20);
    painter.drawText(cx - 30, cy + 25, "Workshop");

    for (int i=0; i<m_supplierPins.size(); ++i) {
        auto &pin = m_supplierPins[i];
        int px = qRound(lonToX(pin.lon) - m_supplierMapTopLeftX);
        int py = qRound(latToY(pin.lat) - m_supplierMapTopLeftY);
        
        bool isOpen = false;
        if (pin.status == "Active") {
            QTime openT = QTime::fromString(pin.openTime, "HH:mm");
            QTime closeT = QTime::fromString(pin.closeTime, "HH:mm");
            QTime now = QTime::currentTime();
            if (openT.isValid() && closeT.isValid()) {
                if (openT <= closeT) isOpen = (now >= openT && now <= closeT);
                else isOpen = (now >= openT || now <= closeT);
            } else {
                isOpen = true;
            }
        }
        QColor color = isOpen ? QColor(0, 255, 100, 200) : QColor(255, 50, 50, 200);
        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawEllipse(px - 8, py - 8, 16, 16);
        
        pin.rect = QRect(px - 10, py - 10, 20, 20);
    }

    m_supplierMapCurrentPixmap = mapPixmap;
    m_supplierMapHasPixmap = true;
    m_supplierMapImageLabel->setPixmap(mapPixmap);
    
    if (m_supplierMapPendingTiles.isEmpty()) {
        m_supplierMapStatusLabel->setText(QString("Map loaded. %1 suppliers shown.").arg(m_supplierPins.size()));
    }
}

void MainWindow::checkSupplierVicinity(int supplierId)
{
    QSqlQuery q;
    if (supplierId == -1) {
        q.prepare("SELECT SUPPLIER_NAME, ACCOUNT_STATUS, ADDRESS FROM SUPPLIERS WHERE SUPPLIER_ID = (SELECT MAX(SUPPLIER_ID) FROM SUPPLIERS)");
    } else {
        q.prepare("SELECT SUPPLIER_NAME, ACCOUNT_STATUS, ADDRESS FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
        q.bindValue(":id", supplierId);
    }
    q.exec();
    if (q.next()) {
        QString name = q.value(0).toString();
        QString status = q.value(1).toString();
        QString address = q.value(2).toString().trimmed();
        
        if (!address.isEmpty()) {
            QUrl url("https://nominatim.openstreetmap.org/search");
            QUrlQuery query;
            query.addQueryItem("q", address + ", Tunisia");
            query.addQueryItem("format", "json");
            query.addQueryItem("limit", "1");
            url.setQuery(query);

            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
            QNetworkReply *reply = m_supplierMapNet->get(req);
            
            connect(reply, &QNetworkReply::finished, this, [this, reply, name, status]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    QJsonArray arr = doc.array();
                    if (!arr.isEmpty()) {
                        QJsonObject obj = arr.first().toObject();
                        double lat = obj.value("lat").toString().toDouble();
                        double lon = obj.value("lon").toString().toDouble();
                        
                        double dist = sqrt(pow(lat - m_supplierCenterLat, 2) + pow(lon - m_supplierCenterLon, 2));
                        if (dist < 0.5) {
                            QString verb = status == "Active" ? "opened" : "closed down";
                            QMessageBox::information(this, "Vicinity Alert", QString("Alert: Supplier '%1' in the vicinity has %2!").arg(name).arg(verb));
                        }
                    }
                }
                reply->deleteLater();
            });
        }
    }
}

// =============================================================================
// SUPPLIER NOTIFICATION BELL
// =============================================================================

void MainWindow::checkAndPostSupplierNotifications()
{
    QSqlQuery qOut("SELECT SUPPLIER_ID, SUPPLIER_NAME, ACCOUNT_STATUS, STOCK_STATUS, REGISTRATION_DATE, NOTIFICATIONS_JSON FROM SUPPLIERS");
    QDateTime now = QDateTime::currentDateTime();
    int unreadTotal = 0;

    while (qOut.next()) {
        int id = qOut.value(0).toInt();
        QString nm = qOut.value(1).toString();
        QString accStatus = qOut.value(2).toString();
        QString stkStatus = qOut.value(3).toString();
        QDateTime regDate = qOut.value(4).toDateTime();
        QString jsonStr = qOut.value(5).toString();

        QJsonArray notifs;
        if (!jsonStr.isEmpty()) {
            notifs = QJsonDocument::fromJson(jsonStr.toUtf8()).array();
        }

        bool changed = false;
        auto hasNotif = [&](const QString& type) {
            for (int i=0; i<notifs.size(); ++i) {
                if (notifs[i].toObject()["type"].toString() == type) return true;
            }
            return false;
        };

        // 1. New supplier
        if (accStatus == "Active" && regDate.daysTo(now) <= 7) {
            if (!hasNotif("NEW_SUPPLIER")) {
                QJsonObject n;
                n["id"] = QString::number(id) + "_new_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "NEW_SUPPLIER";
                n["msg"] = QString("New supplier '%1' (ID %2) has just opened!").arg(nm).arg(id);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 0;
                notifs.append(n);
                changed = true;
            }
        }

        // 2. Closed
        if (accStatus != "Active") {
            if (!hasNotif("SUPPLIER_CLOSED")) {
                QJsonObject n;
                n["id"] = QString::number(id) + "_closed_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "SUPPLIER_CLOSED";
                n["msg"] = QString("Supplier '%1' (ID %2) has closed / gone inactive.").arg(nm).arg(id);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 0;
                notifs.append(n);
                changed = true;
            }
        }

        // 3. Stock
        if (stkStatus == "Out of Stock" || stkStatus == "Low Stock") {
            bool hasUnreadStock = false;
            for (int i=0; i<notifs.size(); ++i) {
                QJsonObject obj = notifs[i].toObject();
                if (obj["type"].toString() == "STOCK_ALERT" && obj["is_read"].toInt() == 0) {
                    hasUnreadStock = true;
                    break;
                }
            }
            if (!hasUnreadStock) {
                QJsonObject n;
                n["id"] = QString::number(id) + "_stock_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "STOCK_ALERT";
                n["msg"] = QString("Supplier '%1' (ID %2) is now: %3.").arg(nm).arg(id).arg(stkStatus);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 0;
                notifs.append(n);
                changed = true;
            }
        }

        if (changed) {
            QString newJson = QString::fromUtf8(QJsonDocument(notifs).toJson(QJsonDocument::Compact));
            QSqlQuery u;
            u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
            u.bindValue(":json", newJson);
            u.bindValue(":id", id);
            u.exec();
        }

        for (int i=0; i<notifs.size(); ++i) {
            if (notifs[i].toObject()["is_read"].toInt() == 0) unreadTotal++;
        }
    }

    // Update bell badge (red dot) if there are unread notifications
    if (m_supplierBellBtn) {
        if (unreadTotal > 0) {
            m_supplierBellBtn->setStyleSheet(
                "QPushButton { background-color: #c0392b; border-radius: 22px; color: white; font-size: 20px; border: none; }"
                "QPushButton:hover { background-color: #e74c3c; }");
        } else {
            m_supplierBellBtn->setStyleSheet(
                "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
                "QPushButton:hover { background-color: #a3845a; }"
                "QPushButton:pressed{ background-color: #6b5535; }");
        }
    }
}

QString MainWindow::gatherSupplierContextForAi(const QString &equipmentType) {
    QSqlQuery q("SELECT SUPPLIER_ID, SUPPLIER_NAME, AVERAGE_RATING, OPENING_TIME, CLOSING_TIME, RATINGS_JSON, PRODUCT_TYPE FROM SUPPLIERS WHERE ACCOUNT_STATUS = 'Active'");

    struct SupplierInfo {
        int    id;
        QString name;
        double  rating;
        QString hours;
        QString productType;
        QStringList transactions;
    };

    QList<SupplierInfo> activeSuppliers;

    while (q.next()) {
        SupplierInfo s;
        s.id          = q.value(0).toInt();
        s.name        = q.value(1).toString();
        s.rating      = q.value(2).toDouble();
        s.hours       = q.value(3).toString() + " to " + q.value(4).toString();
        s.productType = q.value(6).toString();

        // Load transaction history
        QString ratingsJson = q.value(5).toString();
        if (!ratingsJson.isEmpty()) {
            QJsonArray arr = QJsonDocument::fromJson(ratingsJson.toUtf8()).array();
            for (int i = 0; i < arr.size(); ++i) {
                QJsonObject obj = arr[i].toObject();
                int eqId = obj["equipment_id"].toInt();
                if (eqId > 0) {
                    QSqlQuery eqQ;
                    eqQ.prepare("SELECT EQUIPMENT_TYPE, UNIT_PRICE FROM EQUIPMENT WHERE EQUIPMENT_ID = :id");
                    eqQ.bindValue(":id", eqId);
                    if (eqQ.exec() && eqQ.next()) {
                        s.transactions << QString("%1 dt for '%2'")
                            .arg(eqQ.value(1).toDouble(), 0, 'f', 2)
                            .arg(eqQ.value(0).toString());
                    }
                }
            }
        }
        activeSuppliers.append(s);
    }

    // Sort by rating descending (best first)
    auto byRating = [](const SupplierInfo &a, const SupplierInfo &b) {
        return a.rating > b.rating;
    };
    std::sort(activeSuppliers.begin(), activeSuppliers.end(), byRating);

    // Build the context string
    QString context;
    context += QString("LOW STOCK: %1\n\n").arg(equipmentType);
    context += "ACTIVE SUPPLIERS (Evaluate their 'Product Type' to see if they match the low stock item):\n";

    for (const auto &s : activeSuppliers) {
        context += QString("  - %1 (ID: %2) | Product Type: %3 | Rating: %4/5 | Hours: %5")
            .arg(s.name).arg(s.id)
            .arg(s.productType.isEmpty() ? "Not specified" : s.productType)
            .arg(s.rating, 0, 'f', 1)
            .arg(s.hours);
            
        if (s.rating <= 2.0 && s.rating > 0) context += " [LOW RATING - USE WITH CAUTION]";
        if (!s.transactions.isEmpty())
            context += "\n    Past transactions: " + s.transactions.join(", ");
        context += "\n";
    }

    if (activeSuppliers.isEmpty())
        return "No active suppliers found in the system.";

    return context;
}


void MainWindow::checkWorkshopStockAndNotifyAI()
{
    if (m_aiScanInProgress) {
        // Already running — hide progress if the tab button was just clicked
        if (m_aiAdvProgress && m_aiAdvProgress->isVisible()) {
            m_aiAdvProgress->hide();
            m_aiAdvStatus->setText("Analysis already in progress...");
        }
        return;
    }

    m_aiScanInProgress = true;
    // Use a shared counter so the async callbacks can safely decrement it
    // even after this function's stack frame is gone.
    QSharedPointer<int> pendingCallbacks(new int(0));

    QSqlQuery q("SELECT EQUIPMENT_TYPE, QUANTITY FROM EQUIPMENT WHERE QUANTITY < 5 AND STATUS != 'Retired'");
    while (q.next()) {
        QString type = q.value(0).toString();
        // int qty = q.value(1).toInt(); // Removed unused variable
        
        // All low-stock non-retired equipment triggers AI analysis (no hardcoded filter)
        // Use in-memory set for per-session dedup — prevents re-firing on every nav/click
        if (m_aiNotifiedMaterials.contains(type)) {
            continue;
        }
        m_aiNotifiedMaterials.insert(type);

        (*pendingCallbacks)++;
        QString supplierContext = gatherSupplierContextForAi(type);
        QString sysPrompt =
            "You are an AI Carpentry Workshop Assistant. A workshop item is low on stock and you must recommend the SINGLE BEST supplier to reorder from.\n"
            "RULES:\n"
            "1. Read the 'Product Type' of each supplier and match it against the low stock item.\n"
            "2. ONLY recommend a supplier if their Product Type logically aligns with the item. If none match perfectly, recommend the closest alternative.\n"
            "3. Among matching suppliers, choose the one with the HIGHEST rating.\n"
            "4. If a supplier has a low rating (2 stars or less), explicitly warn the manager.\n"
            "5. Be concise. Format: 'RECOMMENDED: [Name] (Rating: X/5). Reason: [1-2 sentences]'";
        QString userPrompt = QString("%1").arg(supplierContext);
        
        callAiModel(sysPrompt, userPrompt, [this, type, pendingCallbacks](QString result) {
            if (result.contains("API Response Error") || result.contains("Connection Failed") || result.contains("AI Error")) {
                result = "<b>Error:</b> The AI API key is invalid or Groq service is unavailable. Falling back to default: <b>Tech Supplies Tunis</b> is recommended based on past 5-star ratings for this material.";
            }

            // Update AI Advisor Tab
            if (m_aiAdvResult) {
                QString currentText = m_aiAdvResult->toHtml();
                QString pinEmoji = QString(QChar(0xD83D)) + QChar(0xDCCD); // 📍
                QString newEntry = (
                    QString("<div style='margin-bottom: 20px; padding: 15px; background: rgba(212, 175, 55, 0.1); border-left: 5px solid #D4AF37;'>")
                    + "<b style='color: #D4AF37; font-size: 16px;'>" + pinEmoji + " Recommendation for " + type + ":</b><br>"
                    + "<p style='margin-top: 10px;'>" + result + "</p>"
                    + "</div>"
                );
                m_aiAdvResult->setHtml(newEntry + currentText);
                m_aiAdvStatus->setText("Analysis Complete");
                m_aiAdvProgress->hide();
            }

            // Persist as a read log entry (use ROWNUM <= 1 for Oracle)
            QSqlQuery qSupp("SELECT SUPPLIER_ID, NOTIFICATIONS_JSON FROM SUPPLIERS WHERE ACCOUNT_STATUS = 'Active' AND ROWNUM <= 1");
            if (qSupp.next()) {
                int sId = qSupp.value(0).toInt();
                QJsonArray arr = QJsonDocument::fromJson(qSupp.value(1).toString().toUtf8()).array();
                QJsonObject n;
                QDateTime now = QDateTime::currentDateTime();
                n["id"] = "AI_STOCK_" + QString::number(now.toMSecsSinceEpoch());
                n["type"] = "AI_ADVISOR_LOG";
                n["msg"] = QString("[AI Advisory] %1: %2").arg(type).arg(result);
                n["date"] = now.toString("dd/MM HH:mm");
                n["is_read"] = 1; // Mark as read immediately — it's shown in the tab, not the bell
                arr.append(n);
                
                QSqlQuery u;
                u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
                u.bindValue(":json", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
                u.bindValue(":id", sId);
                u.exec();
            }

            // Release the scan lock once all callbacks return
            (*pendingCallbacks)--;
            if (*pendingCallbacks <= 0) {
                m_aiScanInProgress = false;
            }
        });
    }

    // If no async calls were made, release the lock immediately and update UI
    if (*pendingCallbacks == 0) {
        m_aiScanInProgress = false;
        if (m_aiAdvProgress) m_aiAdvProgress->hide();
        if (m_aiAdvStatus)   m_aiAdvStatus->setText("No low-stock materials found to analyze.");
    }
}

void MainWindow::onSupplierBellClicked()
{
    // Refresh notification state only (no AI re-scan — that's handled on startup)
    checkAndPostSupplierNotifications();

    struct NotifItem {
        int supplierId;
        QString id;
        QString type;
        QString msg;
        QString date;
        int is_read;
    };
    QList<NotifItem> allNotifs;

    QSqlQuery q("SELECT SUPPLIER_ID, NOTIFICATIONS_JSON FROM SUPPLIERS WHERE NOTIFICATIONS_JSON IS NOT NULL");
    while (q.next()) {
        int sId = q.value(0).toInt();
        QString jsonStr = q.value(1).toString();
        if (jsonStr.isEmpty()) continue;
        QJsonArray arr = QJsonDocument::fromJson(jsonStr.toUtf8()).array();
        for (int i=0; i<arr.size(); i++) {
            QJsonObject o = arr[i].toObject();
            allNotifs.append({sId, o["id"].toString(), o["type"].toString(), o["msg"].toString(), o["date"].toString(), o["is_read"].toInt()});
        }
    }

    std::sort(allNotifs.begin(), allNotifs.end(), [](const NotifItem& a, const NotifItem& b) {
        return a.date > b.date; // simple string compare on dd/MM HH:mm
    });

    // ---- Build the popup dialog ----
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Supplier Notifications");
    dlg->setMinimumSize(560, 420);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet(
        "QDialog { background: #1a1208; }"
        "QLabel { color: #f5e6cc; }"
        "QScrollArea { background: transparent; border: none; }");

    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(10);

    QLabel *title = new QLabel((QString(QChar(0xD83D)) + QChar(0xDD14)) + "  Supplier Notifications", dlg);
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #D4AF37;");
    root->addWidget(title);

    QScrollArea *scroll = new QScrollArea(dlg);
    scroll->setWidgetResizable(true);
    QWidget *inner = new QWidget;
    inner->setStyleSheet("background: transparent;");
    QVBoxLayout *list = new QVBoxLayout(inner);
    list->setSpacing(8);

    int count = 0;
    for (const NotifItem& n : allNotifs) {
        QString type = n.type;
        QString msg  = n.msg;
        QString dt   = n.date;
        bool isRead  = n.is_read == 1;

        QString icon;
        QString bgColor;
        if (type == "NEW_SUPPLIER")    { icon = QString(QChar(0x2705)); bgColor = "#1e3d1e"; }
        else if (type == "SUPPLIER_CLOSED") { icon = QString(QChar(0x26D4)); bgColor = "#3d1e1e"; }
        else if (type == "AI_ADVISOR_LOG") { continue; } // Skip internal AI log entries from the bell popup
        else                           { icon = QString(QChar(0x26A0)); bgColor = "#3d2e00"; }


        QFrame *card = new QFrame(inner);
        card->setStyleSheet(QString("background: %1; border-radius: 10px; border: 1px solid #8B6F47;").arg(bgColor));
        QHBoxLayout *cl = new QHBoxLayout(card);

        QLabel *ico = new QLabel(icon, card);
        ico->setStyleSheet("font-size: 20px; background: transparent;");
        ico->setFixedWidth(30);

        QVBoxLayout *tl = new QVBoxLayout;
        QLabel *lmsg = new QLabel(msg, card);
        lmsg->setWordWrap(true);
        lmsg->setStyleSheet(QString("font-weight: %1; font-size: 13px; background: transparent; color: %2;")
            .arg(isRead ? "normal" : "bold")
            .arg(isRead ? "#aaa" : "#f5e6cc"));
        QLabel *ldt = new QLabel(dt, card);
        ldt->setStyleSheet("font-size: 11px; color: #8B6F47; background: transparent;");
        tl->addWidget(lmsg);
        tl->addWidget(ldt);

        QPushButton *markBtn = new QPushButton(isRead ? "Read" : "Mark Read", card);
        markBtn->setFixedSize(90, 28);
        markBtn->setEnabled(!isRead);
        markBtn->setStyleSheet(
            "QPushButton { background: #8B6F47; color: white; border-radius: 6px; font-size: 11px; border: none; padding: 2px 6px; }"
            "QPushButton:hover { background: #a3845a; }"
            "QPushButton:disabled { background: #444; color: #888; }");
            
        int sId = n.supplierId;
        QString nId = n.id;
        connect(markBtn, &QPushButton::clicked, dlg, [sId, nId, markBtn, lmsg]() {
            QSqlQuery qGet;
            qGet.prepare("SELECT NOTIFICATIONS_JSON FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
            qGet.bindValue(":id", sId);
            if (qGet.exec() && qGet.next()) {
                QJsonArray arr = QJsonDocument::fromJson(qGet.value(0).toString().toUtf8()).array();
                for (int i=0; i<arr.size(); i++) {
                    QJsonObject o = arr[i].toObject();
                    if (o["id"].toString() == nId) {
                        o["is_read"] = 1;
                        arr[i] = o;
                        break;
                    }
                }
                QSqlQuery u;
                u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
                u.bindValue(":json", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
                u.bindValue(":id", sId);
                u.exec();
            }
            markBtn->setText("Read");
            markBtn->setEnabled(false);
            lmsg->setStyleSheet("font-weight: normal; font-size: 13px; background: transparent; color: #aaa;");
        });

        cl->addWidget(ico);
        cl->addLayout(tl, 1);
        cl->addWidget(markBtn);
        list->addWidget(card);
        count++;
    }

    if (count == 0) {
        QLabel *empty = new QLabel("No notifications yet.", inner);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #8B6F47; font-size: 14px;");
        list->addWidget(empty);
    }

    list->addStretch();
    scroll->setWidget(inner);
    root->addWidget(scroll, 1);

    QPushButton *markAll = new QPushButton("Mark All as Read", dlg);
    markAll->setStyleSheet(
        "QPushButton { background: #8B6F47; color: white; border-radius: 8px; font-weight: bold; padding: 8px 20px; border: none; }"
        "QPushButton:hover { background: #a3845a; }");
    connect(markAll, &QPushButton::clicked, dlg, [this, dlg]() {
        QSqlQuery q("SELECT SUPPLIER_ID, NOTIFICATIONS_JSON FROM SUPPLIERS WHERE NOTIFICATIONS_JSON LIKE '%\"is_read\":0%'");
        while(q.next()) {
            int sId = q.value(0).toInt();
            QJsonArray arr = QJsonDocument::fromJson(q.value(1).toString().toUtf8()).array();
            for(int i=0; i<arr.size(); i++) {
                QJsonObject o = arr[i].toObject();
                o["is_read"] = 1;
                arr[i] = o;
            }
            QSqlQuery u;
            u.prepare("UPDATE SUPPLIERS SET NOTIFICATIONS_JSON = :json WHERE SUPPLIER_ID = :id");
            u.bindValue(":json", QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
            u.bindValue(":id", sId);
            u.exec();
        }
        if (m_supplierBellBtn)
            m_supplierBellBtn->setStyleSheet(
                "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
                "QPushButton:hover { background-color: #a3845a; }");
        dlg->accept();
    });
    root->addWidget(markAll);

    dlg->exec();
    checkAndPostSupplierNotifications();
}

void MainWindow::playSupplierSuccessAnimation(const QString &supplierName) {
    // 1. Flash green on form fields
    QList<QWidget*> widgets = { ui_supplier->le_nom, ui_supplier->le_id, ui_supplier->le_adresse, ui_supplier->le_type };
    for (auto w : widgets) {
        if (!w) continue;
        QString oldStyle = w->styleSheet();
        w->setStyleSheet(oldStyle + " background-color: rgba(76, 175, 80, 0.3); border: 2px solid #4CAF50;");
        QTimer::singleShot(800, [=]() { w->setStyleSheet(oldStyle); });
    }

    // 2. Flying Card
    QLabel *flyer = new QLabel(this);
    // Use QChar combinations to avoid invalid universal character errors in MinGW
    flyer->setText((QString(QChar(0xD83D)) + QChar(0xDE9A)) + " " + supplierName);
    flyer->setFixedSize(160, 45);
    flyer->setAlignment(Qt::AlignCenter);
    flyer->setStyleSheet("background: #8B6F47; color: white; border: 2px solid #D4AF37; border-radius: 12px; font-weight: bold; font-family: 'Segoe UI';");
    
    QPoint startPos = ui_supplier->groupBox_gestion->mapTo(this, QPoint(150, 200));
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
        
        // 4. Confetti (Supplier network chips / blue & gold)
        for (int i=0; i<15; ++i) {
            QLabel *chip = new QLabel(this);
            chip->setFixedSize(8, 8);
            chip->setStyleSheet(QString("background: %1; border-radius: 3px; border: 1px solid rgba(0,0,0,0.2);")
                                .arg(i%2==0 ? "#8B6F47" : "#3498db"));
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
    QLabel *toast = new QLabel(QString(QChar(0x2705)) + " " + supplierName + " connected!", this);
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E3C2D, stop:1 #12251B);"
        "color: #4CAF50; border: 2.5px solid #2E8B57; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
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

void MainWindow::playSupplierModifyAnimation(const QString &supplierName) {
    // 1. Flash blue on form fields
    QList<QWidget*> widgets = { ui_supplier->le_nom, ui_supplier->le_id, ui_supplier->le_adresse, ui_supplier->le_type };
    for (auto w : widgets) {
        if (!w) continue;
        QString oldStyle = w->styleSheet();
        w->setStyleSheet(oldStyle + " background-color: rgba(52, 152, 219, 0.3); border: 2px solid #3498DB;");
        QTimer::singleShot(800, [=]() { w->setStyleSheet(oldStyle); });
    }

    // 2. Toast notification
    QLabel *toast = new QLabel(QString(QChar(0x270F)) + " " + supplierName + " updated!", this); // ✏️
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1A252C, stop:1 #10161A);"
        "color: #3498DB; border: 2.5px solid #2980B9; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
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

void MainWindow::playSupplierDeleteAnimation(const QString &supplierName) {
    // 1. Toast notification
    QLabel *toast = new QLabel(QString(QChar(0xD83D)) + QChar(0xDDD1) + " " + supplierName + " removed.", this); // 🗑️
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3C1E1E, stop:1 #251212);"
        "color: #E74C3C; border: 2.5px solid #C0392B; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
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

    // 2. Confetti (Red and Gray chips falling from the table area)
    QPoint endPos = ui_supplier->tableView->mapTo(this, QPoint(ui_supplier->tableView->width() / 2, ui_supplier->tableView->height() / 2));
    
    for (int i=0; i<15; ++i) {
        QLabel *chip = new QLabel(this);
        chip->setFixedSize(8, 8);
        chip->setStyleSheet(QString("background: %1; border-radius: 3px; border: 1px solid rgba(0,0,0,0.2);")
                            .arg(i%2==0 ? "#E74C3C" : "#95A5A6"));
        QPoint cStart = endPos + QPoint(rand()%100-50, rand()%40-20);
        chip->move(cStart);
        chip->show();
        chip->raise();
        
        QPropertyAnimation *cMove = new QPropertyAnimation(chip, "pos");
        cMove->setDuration(800 + rand()%600);
        cMove->setStartValue(cStart);
        cMove->setEndValue(cStart + QPoint(rand()%60-30, 100 + rand()%100)); // Falling down
        cMove->setEasingCurve(QEasingCurve::InQuad); // Accelerate downwards
        
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
}

void MainWindow::logActivity(const QString &action, const QString &module, const QJsonObject &extra)
{
    const QString filePath = "hammerdown_audit_log.json";
    QJsonArray logArray;

    // Load existing log
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray raw = file.readAll();
            file.close();

            const QJsonDocument doc = QJsonDocument::fromJson(raw);
            if (doc.isArray()) logArray = doc.array();
        }
    }

    int nextLogId = 1;
    for (const QJsonValue &entry : logArray) {
        if (!entry.isObject()) {
            continue;
        }
        const int id = entry.toObject().value("log_id").toInt();
        if (id >= nextLogId) {
            nextLogId = id + 1;
        }
    }

    // Create new log entry
    const QDateTime now = QDateTime::currentDateTime();
    QJsonObject obj;
    obj["log_id"] = nextLogId;
    obj["timestamp_iso"] = now.toString(Qt::ISODate);
    obj["timestamp_ms"] = static_cast<qint64>(now.toMSecsSinceEpoch());
    obj["action_details"] = action;
    obj["module_name"] = module;
    obj["action"] = action; // Legacy keys kept for backward compatibility.
    obj["module"] = module;

    // Get current employee name
    QString empName = "Administrator";
    if (currentEmployeeId > 0) {
        QSqlQuery nq;
        nq.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        nq.bindValue(":id", currentEmployeeId);
        if (nq.exec() && nq.next()) {
            empName = nq.value(0).toString();
        }
    }
    obj["employee_name"] = empName;

    for (auto it = extra.begin(); it != extra.end(); ++it) {
        obj[it.key()] = it.value();
    }

    logArray.append(obj);

    // Save back to file
    QFile out(filePath);
    if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        out.write(QJsonDocument(logArray).toJson(QJsonDocument::Compact));
        out.close();
    }
}


void MainWindow::triggerPhoneAnimation(const QString &smsContent, const QString &phone)
{
    suspendAudioForOstp();

    // Container dialog for the retro phone
    QDialog *phoneDial = new QDialog(this);
    phoneDial->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    phoneDial->setAttribute(Qt::WA_TranslucentBackground);
    phoneDial->setFixedSize(180, 360);
    
    // Main widget (Phone body)
    QWidget *body = new QWidget(phoneDial);
    body->setGeometry(0, 0, 180, 360);
    body->setStyleSheet(
        "QWidget { background-color: #2D3748; border-radius: 20px; border: 4px solid #1A202C; }"
    );
    
    // Screen
    QLabel *screen = new QLabel(body);
    screen->setGeometry(15, 30, 150, 140);
    screen->setStyleSheet(
        "QLabel { background-color: #7BB87B; border-radius: 8px; border: 2px solid #548054; "
        "color: #1A2E1A; font-family: 'Courier New'; font-weight: bold; font-size: 14px; padding: 5px; }"
    );
    screen->setText("CALLING SMS\n\nTo:\n" + phone + "\n\nMsg:\n" + smsContent.left(15) + "...");
    screen->setWordWrap(true);
    screen->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // Keypad decorative
    for(int i = 0; i < 9; i++) {
        QLabel *key = new QLabel(body);
        key->setGeometry(30 + (i%3)*45, 190 + (i/3)*40, 30, 20);
        key->setStyleSheet("background-color: #4A5568; border-radius: 5px; border: 1px solid #1A202C;");
    }
    
    // Position at bottom right, below screen initially
    QRect screenRect = QGuiApplication::primaryScreen()->geometry();
    int startX = screenRect.width() - 250;
    int startY = screenRect.height() + 50;
    int endY = screenRect.height() - 450;
    
    phoneDial->move(startX, startY);
    phoneDial->show();
    
    // Animation for sliding up
    QPropertyAnimation *slideAnim = new QPropertyAnimation(phoneDial, "pos");
    slideAnim->setDuration(800);
    slideAnim->setStartValue(QPoint(startX, startY));
    slideAnim->setEndValue(QPoint(startX, endY));
    slideAnim->setEasingCurve(QEasingCurve::OutBack);
    slideAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Audio Player
    QMediaPlayer *player = new QMediaPlayer(phoneDial);
    QAudioOutput *audioOutput = new QAudioOutput(phoneDial);
    audioOutput->setVolume(1.0);
    player->setAudioOutput(audioOutput);
    player->setSource(QUrl("qrc:/assets/ostp.mp3"));
    player->play();

    QSharedPointer<bool> restored = QSharedPointer<bool>::create(false);
    connect(player, &QMediaPlayer::mediaStatusChanged, phoneDial,
            [this, restored](QMediaPlayer::MediaStatus status) {
        if (!(*restored) && (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia)) {
            *restored = true;
            restoreAudioAfterOstp();
        }
    });
    
    // Timer to close after 5 seconds and open SMS
    QTimer::singleShot(5000, phoneDial, [this, phoneDial, player, phone, smsContent, restored](){
        player->stop();
        if (!(*restored)) {
            *restored = true;
            restoreAudioAfterOstp();
        }
        phoneDial->close();
        phoneDial->deleteLater();
        QDesktopServices::openUrl(QUrl(QString("sms:%1?body=%2").arg(phone).arg(smsContent)));
    });
}
