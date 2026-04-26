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


// ==================== QR Code Helper ====================









// ==================== QR Code Tab Functions ====================




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



// ==================== Supplier Management ====================





// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────






// ==================== Supplier Delivery Rating System ====================






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







// =============================================================================
// SUPPLIER NOTIFICATION BELL
// =============================================================================


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
        else if (msg == "REQ:1") {
            // STOCK IN
            QSqlQuery q("SELECT COUNT(*) FROM EQUIPMENT WHERE STATUS = 'Available'");
            if (q.next()) arduino->write("TOTAL:" + QByteArray::number(q.value(0).toInt()) + "\n");
        }
        else if (msg == "REQ:2") {
            // USER OUT
            int empId = 0;
            QModelIndex index = ui_employee->tableView_employes->currentIndex();
            if (index.isValid()) empId = ui_employee->tableView_employes->model()->data(ui_employee->tableView_employes->model()->index(index.row(), 0)).toInt();

            QSqlQuery q;
            q.prepare("SELECT COUNT(*) FROM EQUIPMENT WHERE EMPLOYEE_ID = ?");
            q.addBindValue(empId);
            if (q.exec() && q.next()) arduino->write("TOTAL:" + QByteArray::number(q.value(0).toInt()) + "\n");
        }
        else if (msg == "REQ:3") {
            // URGENT
            QSqlQuery q("SELECT COUNT(*) FROM EQUIPMENT WHERE STATUS = 'Under Maintenance' OR NEXT_MAINTENANCE <= SYSDATE");
            if (q.next()) {
                int count = q.value(0).toInt();
                arduino->write("TOTAL:" + QByteArray::number(count) + "\n");
                if (count > 0) arduino->write("ALARM\n");
            }
        }
        // ===== BUTTON FUNCTIONS =====
        // B1: COUNTERS - Short=Active Employees, Long=Equipments In Use
        else if (msg == "B1:SHORT") {
            // Count of active employees
            QSqlQuery q("SELECT COUNT(*) FROM EMPLOYEES WHERE EMPLOYEE_STATUS = 'Active'");
            if (q.next()) {
                int count = q.value(0).toInt();
                arduino->write("TOTAL:" + QByteArray::number(count) + "\n");
                QMessageBox::information(this, "Active Employees", "Total active employees: " + QString::number(count));
            }
        }
        else if (msg == "B1:LONG") {
            // Count of equipments in use
            QSqlQuery q("SELECT COUNT(*) FROM EQUIPMENT WHERE STATUS = 'In Use'");
            if (q.next()) {
                int count = q.value(0).toInt();
                arduino->write("TOTAL:" + QByteArray::number(count) + "\n");
                QMessageBox::information(this, "Equipment In Use", "Total equipment in use: " + QString::number(count));
            }
        }

        // B2: ID CYCLES - Short=Equipment IDs in use, Long=Employee IDs using equipment
        else if (msg == "B2:SHORT") {
            // Get all equipment IDs in use
            QSqlQuery q("SELECT EQUIPMENT_ID FROM EQUIPMENT WHERE STATUS = 'In Use' ORDER BY EQUIPMENT_ID");
            QStringList ids;
            while (q.next()) {
                ids.append(QString::number(q.value(0).toInt()));
            }

            if (ids.isEmpty()) {
                arduino->write("ALARM\n");
                QMessageBox::information(this, "No Equipment", "No equipment currently in use!");
            } else {
                QString idList = ids.join(",");
                arduino->write("CYCLE:" + idList.toUtf8() + "\n");
                QMessageBox::information(this, "Equipment In Use", "Cycling through equipment IDs:\n" + ids.join(", "));
            }
        }
        else if (msg == "B2:LONG") {
            // Get all employee IDs who have equipment
            QSqlQuery q("SELECT DISTINCT EMPLOYEE_ID FROM EQUIPMENT WHERE EMPLOYEE_ID IS NOT NULL ORDER BY EMPLOYEE_ID");
            QStringList ids;
            while (q.next()) {
                ids.append(QString::number(q.value(0).toInt()));
            }

            if (ids.isEmpty()) {
                arduino->write("ALARM\n");
                QMessageBox::information(this, "No Users", "No employees currently using equipment!");
            } else {
                QString idList = ids.join(",");
                arduino->write("CYCLE:" + idList.toUtf8() + "\n");
                QMessageBox::information(this, "Active Users", "Employees using equipment:\n" + ids.join(", "));
            }
        }

        // B3: Maintenance Alert - Short=Urgent Count (buzzer if >0), Long=Report Problem (buzzer confirm)
        else if (msg == "B3:SHORT") {
            QSqlQuery q("SELECT COUNT(*) FROM EQUIPMENT WHERE STATUS = 'Under Maintenance' OR (NEXT_MAINTENANCE <= SYSDATE AND STATUS != 'Retired')");
            if (q.next()) {
                int urgent = q.value(0).toInt();
                arduino->write("TOTAL:" + QByteArray::number(urgent % 10) + "\n");
                if (urgent > 0) {
                    arduino->write("ALARM\n");
                    QMessageBox::warning(this, "URGENT!", QString::number(urgent) + " equipment need maintenance NOW!");
                } else {
                    QMessageBox::information(this, "Maintenance Status", "No urgent maintenance. All equipment healthy!");
                }
            }
        }
        else if (msg == "B3:LONG") {
            // Get equipment IDs needing maintenance
            QSqlQuery q("SELECT EQUIPMENT_ID FROM EQUIPMENT WHERE STATUS = 'Under Maintenance' OR (NEXT_MAINTENANCE <= SYSDATE AND STATUS != 'Retired') ORDER BY EQUIPMENT_ID");
            m_maintenanceEquipmentIds.clear();
            QStringList idList;
            while (q.next()) {
                int id = q.value(0).toInt();
                m_maintenanceEquipmentIds.append(QString::number(id));
                idList.append(QString::number(id));
            }

            if (m_maintenanceEquipmentIds.isEmpty()) {
                arduino->write("TOTAL:0\n");
                QMessageBox::information(this, "No Maintenance Needed", "All equipment is in good condition!");

                // Hide notification button if visible
                if (homeWindow) {
                    QPushButton* maintBtn = homeWindow->findChild<QPushButton*>("btn_maintenance_notif");
                    if (maintBtn) maintBtn->setVisible(false);
                }
            } else {
                arduino->write("TOTAL:" + QByteArray::number(m_maintenanceEquipmentIds.size()) + "\n");
                arduino->write("ALARM\n");

                // Show notification button on home screen
                if (homeWindow) {
                    QPushButton* maintBtn = homeWindow->findChild<QPushButton*>("btn_maintenance_notif");
                    if (maintBtn) {
                        maintBtn->setVisible(true);
                        maintBtn->setText("🔧");
                        maintBtn->setToolTip(QString("%1 equipment(s) need maintenance").arg(m_maintenanceEquipmentIds.size()));
                    }
                }

                QMessageBox::warning(this, "Maintenance Required!",
                    QString("%1 equipment(s) need maintenance!\n\nEquipment IDs: %2\n\nClick the 🔧 button on the home screen to view details.")
                    .arg(m_maintenanceEquipmentIds.size())
                    .arg(idList.join(", ")));
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
