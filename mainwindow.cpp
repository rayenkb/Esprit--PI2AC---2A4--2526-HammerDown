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

    // Sort controls: field combobox + direction toggle button
    connect(ui_client->cb_sort_field,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int){ onClientSearch(); });
    connect(ui_client->btn_sort_order, &QPushButton::clicked, this, [this]() {
        const bool wasAsc = ui_client->btn_sort_order->text().contains(QStringLiteral("Ascending"));
        ui_client->btn_sort_order->setText(wasAsc ? QStringLiteral("Descending \u25BC")
                                                  : QStringLiteral("Ascending \u25B2"));
        onClientSearch();
    });
    // Mail tab buttons
    connect(ui_client->btn_send,   &QPushButton::clicked, this, &MainWindow::onClientSendMail);
    connect(ui_client->btn_browse, &QPushButton::clicked, this, &MainWindow::onClientBrowseMail);

    // Hide SMTP config fields — credentials are hardcoded in onClientSendMail
    ui_client->l_smtp->hide();  ui_client->le_smtp->hide();
    ui_client->l_port->hide();  ui_client->le_port->hide();
    ui_client->l_user->hide();  ui_client->le_user->hide();
    ui_client->l_pass->hide();  ui_client->le_pass->hide();

    // Auto-refresh client view/stats when switching tabs
    connect(ui_client->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_client->tabWidget->widget(idx) == ui_client->tab_view)
            onClientRefreshView();
        else if (ui_client->tabWidget->widget(idx) == ui_client->tab_stats)
            setupClientStats();
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


// ---------------------------------------------------------------------------
//  Arduino LCD helpers — sends two lines to a 16x2 LCD over serial.
//  Arduino sketch should read serial and call lcd.print() for each line.
//  Protocol: "LCD:<line1>|<line2>\n"
// ---------------------------------------------------------------------------




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

void MainWindow::pauseHomeAudioForWeather()
{
    if (!homeAudioPlayer) return;
    if (m_audioSuspendedForOstp) return;
    if (homeAudioPlayer->playbackState() != QMediaPlayer::PlayingState) return;

    m_homeAudioWeatherResumePos = homeAudioPlayer->position();
    m_homeAudioPausedByWeather = true;
    homeAudioPlayer->pause();
}

void MainWindow::resumeHomeAudioAfterWeather()
{
    if (!m_homeAudioPausedByWeather || !homeAudioPlayer) {
        m_homeAudioPausedByWeather = false;
        return;
    }
    if (m_audioSuspendedForOstp) {
        return;
    }

    if (homeAudioOutput) homeAudioOutput->setVolume(homeOstmVolume(currentVolume));
    homeAudioPlayer->setPosition(m_homeAudioWeatherResumePos);
    homeAudioPlayer->play();
    m_homeAudioPausedByWeather = false;
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

    resumePlayer(loginAudioPlayer, m_resumeLoginAfterOstp, m_loginResumePosAfterOstp);
    resumePlayer(homeAudioPlayer, m_resumeHomeAfterOstp, m_homeResumePosAfterOstp);
    resumePlayer(tutorialLoopAudioPlayer, m_resumeTutorialAfterOstp, m_tutorialResumePosAfterOstp);

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







