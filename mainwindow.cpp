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
#include <QLocale>
#include <QFileInfo>
#include <QProcess>
#include <QUuid>
#include <functional>
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

void setItemTrKey(QListWidgetItem *item, const QString &key)
{
    if (item) {
        item->setData(Qt::UserRole, key);
        item->setText(trKey(key));
    }
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , translator(new QTranslator(this))
    , currentLanguage("en")
    , currentEmployeeId(0)
    , m_isChatModernTheme(false) // Classic is now default
    , currentVolume(1.0) // Initialize at start
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

    // Initial sync of homeWindow state
    homeWindow->setLanguage(currentLanguage);
    homeWindow->setVolume(currentVolume);

    // 3. Employee Management (Index 2)
    ui_employee = new Ui::EmployeeManagement;
    employeePage = new QWidget(this);
    ui_employee->setupUi(employeePage);
    ui->stackedWidget->addWidget(employeePage);
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
    connect(ui_employee->btn_suggest_salary, &QPushButton::clicked, this, &MainWindow::onSuggestSalary);
    connect(ui_employee->btn_stats_ai_gen, &QPushButton::clicked, this, &MainWindow::onStatsAiClicked);
    connect(ui_employee->btn_ai_pulse, &QPushButton::clicked, this, &MainWindow::onAIPulseClicked);
    
    // Set age range/default for birthdate
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
            QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
            if (!m) return;
            // Data is now shifted by 2 because of "Edit" and "Delete" columns
            ui_employee->le_id->setText(m->data(m->index(idx.row(), 2)).toString());
            ui_employee->le_nom->setText(m->data(m->index(idx.row(), 3)).toString());
            ui_employee->le_prenom->setText(m->data(m->index(idx.row(), 4)).toString());
            ui_employee->le_fonction->setText(m->data(m->index(idx.row(), 5)).toString());
            int age = m->data(m->index(idx.row(), 6)).toInt();
            ui_employee->de_birthdate->setDate(QDate::currentDate().addYears(-age));
            ui_employee->le_email->setText(m->data(m->index(idx.row(), 7)).toString());
            ui_employee->le_num->setText(m->data(m->index(idx.row(), 8)).toString());
            
            ui_employee->tabWidget->setCurrentIndex(0);
        } else if (idx.column() == 1) { // Delete Action
            onEmployeeDelete();
        }
    });

    // 4. Client Management (Index 3)
    ui_client = new Ui::ClientManagement;
    clientPage = new QWidget(this);
    ui_client->setupUi(clientPage);
    ui->stackedWidget->addWidget(clientPage);
    connect(ui_client->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    // Client CRUD connections
    connect(ui_client->btn_add,    &QPushButton::clicked, this, &MainWindow::onClientAdd);
    connect(ui_client->btn_modify, &QPushButton::clicked, this, &MainWindow::onClientModify);
    connect(ui_client->btn_delete, &QPushButton::clicked, this, &MainWindow::onClientDelete);
    connect(ui_client->btn_search, &QPushButton::clicked, this, &MainWindow::onClientSearch);
    connect(ui_client->tableView, &QAbstractItemView::clicked, this, &MainWindow::onClientRowSelected);

    // Hide global delete button
    ui_client->btn_delete->hide();
    // Also search on Enter in the search box
    connect(ui_client->le_recherche, &QLineEdit::returnPressed, this, &MainWindow::onClientSearch);
    connect(ui_client->btn_pdf,    &QPushButton::clicked, this, &MainWindow::onClientExportPDF);
    // Auto-refresh client view when switching to view tab
    connect(ui_client->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_client->tabWidget->widget(idx) == ui_client->tab_view)
            onClientRefreshView();
    });

    // 5. Supplier Management (Index 4)
    ui_supplier = new Ui::SupplierManagement;
    supplierPage = new QWidget(this);
    ui_supplier->setupUi(supplierPage);
    ui->stackedWidget->addWidget(supplierPage);
    connect(ui_supplier->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    m_supplierMapNet = new QNetworkAccessManager(this);
    connect(m_supplierMapNet, &QNetworkAccessManager::finished, this, &MainWindow::onSupplierGeocodeFinished);
    setupSupplierMapTab();

    // 6. Equipment Management (Index 5)
    ui_equipment = new Ui::EquipmentManagement;
    equipmentPage = new QWidget(this);
    ui_equipment->setupUi(equipmentPage);
    ui->stackedWidget->addWidget(equipmentPage);

    // --- NEXUS TAB: Add programmatically as tab index 5 ---
    m_nexusWidget = new NexusWidget(equipmentPage);
    ui_equipment->tabWidget->addTab(m_nexusWidget, "NEXUS");

    // --- COSTS TAB: Add programmatically as tab index 6 ---
    m_costsWidget = new CostsWidget(equipmentPage);
    ui_equipment->tabWidget->addTab(m_costsWidget, "COSTS");

    // Auto-refresh equipment views/stats when switching tabs
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_view)
            onEquipmentRefreshView();
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_history)
            onEquipmentHistoryRefresh();
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_stats)
            setupEquipmentStats();
        else if (ui_equipment->tabWidget->widget(idx) == m_nexusWidget) {
            // Initialize NEXUS when first shown
            m_nexusWidget->initialize();
        }
        else if (ui_equipment->tabWidget->widget(idx) == m_costsWidget) {
            // Initialize COSTS when first shown
            m_costsWidget->initialize();
        }
    });
    
    // Ensure History Table Exists
    onChatEnsureTable();
    
    chatRefreshTimer = new QTimer(this);
    connect(chatRefreshTimer, &QTimer::timeout, this, &MainWindow::onChatRefresh);
    
    // Connect Chat buttons
    connect(ui_equipment->btn_chat_send, &QPushButton::clicked, this, &MainWindow::onChatSendMessage);
    connect(ui_equipment->le_chat_input, &QLineEdit::returnPressed, this, &MainWindow::onChatSendMessage);
    connect(ui_equipment->list_employees, &QListWidget::itemClicked, this, &MainWindow::onChatEmployeeSelected);
    connect(ui_equipment->btn_chat_img, &QPushButton::clicked, this, &MainWindow::onChatAttachImage);
    
    // --- Add Emoji & GIF buttons to the input bar ---
    {
        QHBoxLayout *inputLayout = ui_equipment->horizontalLayout_input;
        
        QPushButton *emojiBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x98\x80"), equipmentPage);
        emojiBtn->setObjectName("btn_chat_emoji");
        emojiBtn->setFixedSize(44, 44);
        emojiBtn->setToolTip("Emoji Picker");
        emojiBtn->setCursor(Qt::PointingHandCursor);
        emojiBtn->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
        
        QPushButton *gifBtn = new QPushButton("GIF", equipmentPage);
        gifBtn->setObjectName("btn_chat_gif");
        gifBtn->setFixedSize(50, 44);
        gifBtn->setToolTip("Search GIFs (Powered by GIPHY)");
        gifBtn->setCursor(Qt::PointingHandCursor);
        gifBtn->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 13px; font-weight: bold; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
        
        // Insert after btn_chat_img (index 0) but before le_chat_input
        inputLayout->insertWidget(1, emojiBtn);
        inputLayout->insertWidget(2, gifBtn);
        
        connect(emojiBtn, &QPushButton::clicked, this, &MainWindow::onChatEmojiClicked);
        connect(gifBtn, &QPushButton::clicked, this, &MainWindow::onChatGifClicked);
    }
    
    // GIPHY network manager
    giphyNetworkManager = new QNetworkAccessManager(this);
    
    // AI Summarization network manager
    chatSummaryNetManager = new QNetworkAccessManager(this);
    
    // --- Equipment Hover Card for #id preview ---
    m_hoverCard = new EquipmentHoverCard(equipmentPage);
    m_hoverCard->hide();
    
    // --- Smart Chat Completer ---
    m_completerModel = new QStringListModel(this);
    m_chatCompleter = new QCompleter(m_completerModel, this);
    m_chatCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_chatCompleter->setCompletionMode(QCompleter::PopupCompletion);
    ui_equipment->le_chat_input->setCompleter(m_chatCompleter);
    
    // Connect textChanged for #id detection anywhere in the text
    connect(ui_equipment->le_chat_input, &QLineEdit::textChanged, this, [this](const QString &text){
        if (!ui_equipment || !m_hoverCard) return;
        
        // Find the last instance of #id in the text
        static QRegularExpression reg("#(\\d+)");
        QRegularExpressionMatchIterator it = reg.globalMatch(text);
        QRegularExpressionMatch lastMatch;
        while (it.hasNext()) lastMatch = it.next();
        
        if (lastMatch.hasMatch()) {
            int id = lastMatch.captured(1).toInt();
            // Map position to a slightly better spot
            QPoint pos = ui_equipment->le_chat_input->mapTo(equipmentPage, QPoint(120, -145));
            m_hoverCard->showCard(id, pos);
        } else {
            m_hoverCard->hide();
        }
    });

    // Ensure Microphone button is visible and styled properly
    if (ui_equipment->btn_chat_voice) {
        ui_equipment->btn_chat_voice->setVisible(true);
        ui_equipment->btn_chat_voice->setToolTip("Voice Note - Recording Waveform");
        ui_equipment->btn_chat_voice->setCursor(Qt::PointingHandCursor);
        ui_equipment->btn_chat_voice->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 2.2px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
    }
    
    // --- Ctrl+F Chat Search Shortcut ---
    {
        QShortcut *chatSearchShortcut = new QShortcut(QKeySequence("Ctrl+F"), equipmentPage);
        connect(chatSearchShortcut, &QShortcut::activated, this, &MainWindow::onChatSearchToggle);
    }
    
    // --- Equipment Quick-Share Button in Chat Input Bar ---
    {
        QHBoxLayout *inputLayout = ui_equipment->horizontalLayout_input;
        
        QPushButton *shareEquipBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\xA6"), equipmentPage);
        shareEquipBtn->setObjectName("btn_chat_share_equip");
        shareEquipBtn->setFixedSize(44, 44);
        shareEquipBtn->setToolTip("Share Equipment Card");
        shareEquipBtn->setCursor(Qt::PointingHandCursor);
        shareEquipBtn->setStyleSheet(
            "QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; }"
            "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }"
            "QPushButton:pressed { background: #8B6F47; }");
        
        // Insert after GIF button (index 3)
        inputLayout->insertWidget(3, shareEquipBtn);
        
        connect(shareEquipBtn, &QPushButton::clicked, this, [this]() {
            // Show equipment picker dialog
            QDialog *pickDialog = new QDialog(this);
            pickDialog->setWindowTitle("Share Equipment");
            pickDialog->setFixedSize(520, 480);
            pickDialog->setStyleSheet(
                "QDialog { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
                " border: 2px solid #8B6F47; border-radius: 16px; }");
            
            QVBoxLayout *dLay = new QVBoxLayout(pickDialog);
            dLay->setContentsMargins(20, 20, 20, 20);
            dLay->setSpacing(12);
            
            QLabel *headerLbl = new QLabel(QString::fromUtf8("\xF0\x9F\x93\xA6 Select Equipment to Share"), pickDialog);
            headerLbl->setStyleSheet("color: #D4AF37; font-size: 16px; font-weight: bold; background: transparent;");
            headerLbl->setAlignment(Qt::AlignCenter);
            dLay->addWidget(headerLbl);
            
            // Search field
            QLineEdit *searchField = new QLineEdit(pickDialog);
            searchField->setPlaceholderText("Search equipment...");
            searchField->setStyleSheet(
                "QLineEdit { background: rgba(0,0,0,0.3); color: #F0E0C0; border: 1.5px solid #5A4A32;"
                " border-radius: 14px; padding: 8px 14px; font-size: 13px; }"
                "QLineEdit:focus { border-color: #D4AF37; }");
            dLay->addWidget(searchField);
            
            // Equipment list
            QListWidget *equipList = new QListWidget(pickDialog);
            equipList->setStyleSheet(
                "QListWidget { background: rgba(0,0,0,0.2); color: #F0E0C0; border: 1px solid #5A4A32; border-radius: 10px; }"
                "QListWidget::item { padding: 10px; border-bottom: 1px solid rgba(139,111,71,0.2); font-size: 13px; }"
                "QListWidget::item:selected { background: rgba(139,111,71,0.4); color: #D4AF37; }"
                "QListWidget::item:hover { background: rgba(139,111,71,0.25); }");
            
            // Populate from database
            QSqlQuery q("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, STATUS, QUANTITY, UNIT_PRICE FROM EQUIPMENT ORDER BY EQUIPMENT_ID");
            while (q.next()) {
                int eId = q.value(0).toInt();
                QString eType = q.value(1).toString();
                QString eStatus = q.value(2).toString();
                int qty = q.value(3).toInt();
                double price = q.value(4).toDouble();
                
                QString display = QString("[ID: %1] %2 — %3 | Qty: %4 | $%5")
                    .arg(eId).arg(eType).arg(eStatus).arg(qty).arg(price, 0, 'f', 2);
                    
                QListWidgetItem *item = new QListWidgetItem(display);
                item->setData(Qt::UserRole, eId);
                item->setData(Qt::UserRole + 1, eType);
                item->setData(Qt::UserRole + 2, eStatus);
                item->setData(Qt::UserRole + 3, qty);
                item->setData(Qt::UserRole + 4, price);
                equipList->addItem(item);
            }
            dLay->addWidget(equipList, 1);
            
            // Filter
            connect(searchField, &QLineEdit::textChanged, [equipList](const QString &txt){
                for (int i = 0; i < equipList->count(); ++i) {
                    auto *item = equipList->item(i);
                    item->setHidden(!item->text().contains(txt, Qt::CaseInsensitive));
                }
            });
            
            QPushButton *shareBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\xA8 Share to Chat"), pickDialog);
            shareBtn->setStyleSheet(
                "QPushButton { background: #8B6F47; color: white; border-radius: 14px;"
                " padding: 10px 24px; font-weight: bold; font-size: 14px; border: none; }"
                "QPushButton:hover { background: #A0825A; }");
            shareBtn->setCursor(Qt::PointingHandCursor);
            dLay->addWidget(shareBtn, 0, Qt::AlignCenter);
            
            connect(shareBtn, &QPushButton::clicked, this, [this, equipList, pickDialog]() {
                if (!equipList->currentItem()) {
                    QMessageBox::information(pickDialog, "Select", "Please select an equipment item to share.");
                    return;
                }
                auto *item = equipList->currentItem();
                int eId = item->data(Qt::UserRole).toInt();
                QString eType = item->data(Qt::UserRole + 1).toString();
                QString eStatus = item->data(Qt::UserRole + 2).toString();
                int qty = item->data(Qt::UserRole + 3).toInt();
                double price = item->data(Qt::UserRole + 4).toDouble();
                
                QString shareCard = QString::fromUtf8("\xF0\x9F\x93\xA6 [Equipment Card]\n"
                    "━━━━━━━━━━━━━━━━\n"
                    "ID: %1\n"
                    "Type: %2\n"
                    "Status: %3\n"
                    "Quantity: %4\n"
                    "Unit Price: $%5\n"
                    "━━━━━━━━━━━━━━━━")
                    .arg(eId).arg(eType).arg(eStatus).arg(qty).arg(price, 0, 'f', 2);
                
                if (currentChatPartnerId == -1) {
                    QMessageBox::information(pickDialog, "Select Chat", "Please select a chat partner first, then share equipment.");
                    return;
                }
                
                ui_equipment->le_chat_input->setText(shareCard);
                pickDialog->accept();
                onChatSendMessage(); // Auto-send the card
            });
            
            pickDialog->exec();
            delete pickDialog;
        });
    }
    
    if (auto *refreshBtn = equipmentPage->findChild<QPushButton*>("btn_chat_refresh"))
        connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onChatRefresh);
    
    // Connect Weather Assistant button
    // Weather buttons are now configured later in the constructor with high-quality icons.
    // Live search filter for employee list
    if (auto *searchEdit = equipmentPage->findChild<QLineEdit*>("le_chat_search"))
        connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &txt){
            if (!ui_equipment) return;
            for (int i = 0; i < ui_equipment->list_employees->count(); ++i) {
                auto *item = ui_equipment->list_employees->item(i);
                item->setHidden(!item->text().contains(txt, Qt::CaseInsensitive));
            }
        });

    
    connect(ui_equipment->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);

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
    
    ui_order->table_catalog->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_order->table_catalog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_order->table_catalog->setAlternatingRowColors(false);
    ui_order->table_catalog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_order->table_catalog->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_order->table_catalog->setShowGrid(true);
    ui_order->table_catalog->setFocusPolicy(Qt::NoFocus);
    ui_order->table_catalog->setIconSize(QSize(54, 54));
    ui_order->table_catalog->verticalHeader()->setVisible(false);
    ui_order->table_catalog->horizontalHeader()->setFixedHeight(42);
    ui_order->table_catalog->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui_order->table_catalog->setStyleSheet(
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
    ui->stackedWidget->addWidget(orderPage);
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
    connect(ui_employee->btn_send_mail,       &QPushButton::clicked, this, &MainWindow::onEmployeeSendMail);
    connect(ui_employee->btn_export_pdf,      &QPushButton::clicked, this, &MainWindow::onEmployeeExportPDF);
    connect(ui_employee->le_history_search,   &QLineEdit::textChanged, this, &MainWindow::onEmployeeHistorySearch);
    connect(ui_employee->cb_history_filter,   &QComboBox::currentIndexChanged, this, &MainWindow::onEmployeeHistorySearch);
    connect(ui_employee->cb_mail_template,    &QComboBox::currentIndexChanged, this, &MainWindow::onEmployeeMailTemplateChanged);
    
    connect(ui_supplier->btn_add,            &QPushButton::clicked, this, &MainWindow::onSupplierAdd);
    connect(ui_supplier->btn_modify,         &QPushButton::clicked, this, &MainWindow::onSupplierModify);
    connect(ui_supplier->btn_delete,         &QPushButton::clicked, this, &MainWindow::onSupplierDelete);
    connect(ui_supplier->btn_clear,          &QPushButton::clicked, this, &MainWindow::onSupplierClearFields);
    
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

    connect(ui_equipment->btn_clear,  &QPushButton::clicked, this, &MainWindow::onEquipmentClearFields);
    connect(ui_equipment->btn_add,    &QPushButton::clicked, this, &MainWindow::onEquipmentAdd);
    connect(ui_equipment->btn_modify, &QPushButton::clicked, this, &MainWindow::onEquipmentModify);
    connect(ui_equipment->btn_delete, &QPushButton::clicked, this, &MainWindow::onEquipmentDelete);
    connect(ui_equipment->btn_delete_confirm, &QPushButton::clicked, this, &MainWindow::onEquipmentDelete);
    connect(ui_equipment->btn_share_chat, &QPushButton::clicked, this, &MainWindow::onEquipmentShareToChat);
    connect(ui_equipment->btn_search, &QPushButton::clicked, this, &MainWindow::onEquipmentSearch);
    connect(ui_equipment->le_recherche, &QLineEdit::returnPressed, this, &MainWindow::onEquipmentSearch);
    // Load row into form when row is clicked in view tab
    connect(ui_equipment->table_equipments, &QAbstractItemView::clicked, this, [this](const QModelIndex &idx){
        if (idx.column() == 0) { // Edit
            QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
            if (!m) return;
            // Shifting by 2 actions
            QString id      = m->data(m->index(idx.row(), 2)).toString();
            QString type    = m->data(m->index(idx.row(), 3)).toString();
            int     qty     = m->data(m->index(idx.row(), 4)).toInt();
            double  price   = m->data(m->index(idx.row(), 5)).toDouble();
            QString cond    = m->data(m->index(idx.row(), 6)).toString();
            QString dateStr = m->data(m->index(idx.row(), 7)).toString();
            QString desc    = m->data(m->index(idx.row(), 8)).toString();

            ui_equipment->le_id->setText(id);
            ui_equipment->le_type->setText(type);
            ui_equipment->sb_quantity->setValue(qty);
            ui_equipment->dsb_unit_price->setValue(price);
            ui_equipment->te_desc->setText(desc);

            int statusIdx = ui_equipment->cb_status->findText(cond, Qt::MatchFixedString);
            if (statusIdx >= 0) ui_equipment->cb_status->setCurrentIndex(statusIdx);

            QDate pDate = QDate::fromString(dateStr, "dd/MM/yyyy");
            if (!pDate.isValid()) pDate = QDate::fromString(dateStr, "yyyy-MM-dd");
            if (pDate.isValid()) ui_equipment->de_date_achat->setDate(pDate);

            // Switch to management tab
            ui_equipment->tabWidget->setCurrentWidget(ui_equipment->tab_gestion);
        } else if (idx.column() == 1) { // Delete
            onEquipmentDelete();
        }
    });

    // Hide global delete button
    ui_equipment->btn_delete->hide();
    // Auto-refresh equipment view when switching to view tab, and history when switching to history
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_view) {
            onEquipmentRefreshView();
        }
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_history) {
            onEquipmentHistoryRefresh();
        }
        else if (ui_equipment->tabWidget->widget(idx) == ui_equipment->tab_chat) {
            onChatEmployeeListRefresh();
            chatRefreshTimer->start(3000);
        } else if (idx == 5) { // NEXUS
            chatRefreshTimer->stop();
        } else if (idx == 6) { // COSTS
            chatRefreshTimer->stop();
        } else {
            chatRefreshTimer->stop();
        }
    });

    // History Connections
    connect(ui_equipment->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_clear_history,   &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryClear);
    connect(ui_equipment->le_history_search,   &QLineEdit::textChanged, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_history_search, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);

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

        if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_view) {
            onSupplierRefreshView();
        } else if (ui_supplier->tabWidget->widget(idx) == ui_supplier->tab_reviews) {
            onSupplierPopulateRatingCombos();
            onSupplierReviewLoad();
        }
    });

    onSupplierEnsureReviewsTable();
    onSupplierPopulateRatingCombos();
    onEmployeeEnsureHistoryTable();

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

    // Setup Equipment Stats
    setupEquipmentStats();

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
    setupTabNavigation(supplierPage, ui_supplier->tabWidget, {"Manage", "Stats", "View", "Reviews", "Map"}, 150, 45, {0, 1, 2, 3, 4}, 115, 40);
    setupTabNavigation(equipmentPage, ui_equipment->tabWidget, {"Manage", "View", "History", "Stats", "Chat", "NEXUS", "COSTS"}, 96, 95, {0, 1, 2, 3, 4, 5, 6}, 103, 34);
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
        Q_UNUSED(idx);
        if (!equipmentPage) return;
        const QStringList equipTabs = {"Manage", "View", "History", "Stats", "Chat", "NEXUS", "COSTS"};
        const int startX = 96;
        const int y = 95;
        const int spacing = 103;
        const int afterFirstShift = 34;
        const int manageNudgeRight = 50;
        const int costsNudgeLeft = 16;
        for (int i = 0; i < equipTabs.size(); ++i) {
            const auto radios = equipmentPage->findChildren<QRadioButton*>();
            for (auto *rb : radios) {
                if (!rb) continue;
                if (rb->property("trKey").toString() == equipTabs[i]) {
                    const int extra = (i > 0 ? afterFirstShift : 0) + (i == 0 ? manageNudgeRight : 0) + (i == 6 ? -costsNudgeLeft : 0);
                    rb->move(startX + (spacing * i) + extra, y);
                    break;
                }
            }
        }
    });
    ui_equipment->tabWidget->setCurrentIndex(ui_equipment->tabWidget->currentIndex());
    setupTabNavigation(orderPage, ui_order->tabWidget, {"Manage", "QR Code", "Catalog", "3D Modeling", "Map"}, 250, 85, {}, 125, 40);   // 60+25

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
    
    // Connect Weather Assistant
    if (auto *weatherBtn = equipmentPage->findChild<QPushButton*>("btn_weather_assistant")) {
        connect(weatherBtn, &QPushButton::clicked, this, &MainWindow::onWeatherAssistantClicked);
    }
    if (auto *botBtn = equipmentPage->findChild<QPushButton*>("btn_weather_bot")) {
        botBtn->hide(); // Hide the AI Bot button as requested
    }

// Connect 3D Model Gallery Buttons
    auto connect3DView = [this](const QString& btnName, const QString& query) {
        if (auto *btn = equipmentPage->findChild<QPushButton*>(btnName)) {
            connect(btn, &QPushButton::clicked, this, [query]() {
                QString url = QString("https://sketchfab.com/search?q=%1&type=models").arg(query);
                QDesktopServices::openUrl(QUrl(url));
            });
        }
    };
    connect3DView("btn_view_1", "industrial+drill");
    connect3DView("btn_view_2", "hydraulic+pump");
    connect3DView("btn_view_3", "engine+assembly");


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
    homeAudioPlayer->setSource(QUrl("qrc:/assets/ost2.mp3"));
    homeAudioPlayer->setLoops(1); // Play once
    homeAudioOutput->setVolume(currentVolume);
    
    // Initialize audio player for tutorial (help buttons) - only ost4
    tutorialLoopAudioPlayer = new QMediaPlayer(this);
    tutorialLoopAudioOutput = new QAudioOutput(this);
    tutorialLoopAudioPlayer->setAudioOutput(tutorialLoopAudioOutput);
    tutorialLoopAudioPlayer->setSource(QUrl("qrc:/assets/ost4.mp3"));
    tutorialLoopAudioPlayer->setLoops(QMediaPlayer::Infinite);
    tutorialLoopAudioOutput->setVolume(currentVolume);

    // Initialize audio player for chat page (classical style)
    chatAudioPlayer = new QMediaPlayer(this);
    chatAudioOutput = new QAudioOutput(this);
    chatAudioPlayer->setAudioOutput(chatAudioOutput);
    chatAudioPlayer->setSource(QUrl("qrc:/assets/ost2.mp3")); // Synced with Home music
    chatAudioPlayer->setLoops(QMediaPlayer::Infinite);
    chatAudioOutput->setVolume(currentVolume); // Removed 0.4 scaling for full volume

    // Music toggle button (OST2 removed from chat)

    // Connect Chat Settings Button ("§")
    if (auto *settingsBtn = equipmentPage->findChild<QPushButton*>("btn_chat_settings")) {
        settingsBtn->setIcon(QIcon(":/assets/gear.png"));
        settingsBtn->setIconSize(QSize(22, 22));
        settingsBtn->setText("");
        connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onChatSettingsClicked);
    }
    
    // Connect Chat Refresh Button
    if (auto *refreshBtn = equipmentPage->findChild<QPushButton*>("btn_chat_refresh")) {
        refreshBtn->setIcon(QIcon(":/assets/refresh.png"));
        refreshBtn->setIconSize(QSize(22, 22));
        refreshBtn->setText("");
        connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onChatRefresh);
    }

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
    // 1. Create Layout for the stats container (widget_chart)
    if (!ui_client->widget_chart->layout()) {
        QHBoxLayout *layout = new QHBoxLayout(ui_client->widget_chart);
        ui_client->widget_chart->setLayout(layout);
    }

    // --- CHART 1: PIE CHART (Gender Distribution) ---
    QPieSeries *series = new QPieSeries();
    series->append(trKey("Male"), 60);
    series->append(trKey("Female"), 40);
    series->setProperty("trSliceNames", QStringList{ "Male", "Female" });

    // Add colors
    QPieSlice *sliceMale = series->slices().at(0);
    sliceMale->setBrush(QColor("#8B6F47")); // Gold/Brown
    sliceMale->setLabelVisible();

    QPieSlice *sliceFemale = series->slices().at(1);
    sliceFemale->setBrush(QColor("#C0C0C0")); // Silver/Grey
    sliceFemale->setLabelVisible();
    sliceFemale->setExploded(); // Highlight one slice

    QChart *chartPie = new QChart();
    chartPie->addSeries(series);
    chartPie->setTitle(trKey("Clients by Gender"));
    chartPie->setProperty("trTitleKey", "Clients by Gender");
    chartPie->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartViewPie = new QChartView(chartPie);
    chartViewPie->setRenderHint(QPainter::Antialiasing);


    // --- CHART 2: BAR CHART (Clients Activity) ---
    QBarSet *set0 = new QBarSet(trKey("Active"));
    QBarSet *set1 = new QBarSet(trKey("Inactive"));
    set0->setProperty("trNameKey", "Active");
    set1->setProperty("trNameKey", "Inactive");

    *set0 << 10 << 20 << 30 << 40 << 50 << 60;
    *set1 << 5 << 10 << 15 << 20 << 25 << 30;

    set0->setColor(QColor("#8B6F47"));
    set1->setColor(QColor("#A9A9A9"));

    QBarSeries *seriesBar = new QBarSeries();
    seriesBar->append(set0);
    seriesBar->append(set1);

    QChart *chartBar = new QChart();
    chartBar->addSeries(seriesBar);
    chartBar->setTitle(trKey("Client Activity (Last 6 Months)"));
    chartBar->setProperty("trTitleKey", "Client Activity (Last 6 Months)");
    chartBar->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    const QStringList categoryKeys = { "Jan", "Feb", "Mar", "Apr", "May", "Jun" };
    for (const auto &key : categoryKeys) {
        categories << trKey(key);
    }
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setProperty("trCategories", categoryKeys);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    seriesBar->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 70);
    chartBar->addAxis(axisY, Qt::AlignLeft);
    seriesBar->attachAxis(axisY);

    QChartView *chartViewBar = new QChartView(chartBar);
    chartViewBar->setRenderHint(QPainter::Antialiasing);

    // Add charts to the layout
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
        homeAudioOutput->setVolume(currentVolume);
    }
    if (tutorialLoopAudioOutput) {
        tutorialLoopAudioOutput->setVolume(currentVolume);
    }
    if (chatAudioOutput) {
        chatAudioOutput->setVolume(currentVolume);
    }
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
void MainWindow::on_gs_employes_clicked()    { ui->stackedWidget->setCurrentIndex(2); ui_employee->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_client_clicked()      { ui->stackedWidget->setCurrentIndex(3); ui_client->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_fournisseur_clicked() { ui->stackedWidget->setCurrentIndex(4); ui_supplier->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_equipment_clicked()   { ui->stackedWidget->setCurrentIndex(5); ui_equipment->tabWidget->setCurrentIndex(0); }
void MainWindow::on_gs_order_clicked()       { ui->stackedWidget->setCurrentIndex(6); ui_order->tabWidget->setCurrentIndex(0); }

// Navigation sidebar
void MainWindow::on_nav_employees_clicked()  { ui->stackedWidget->setCurrentIndex(2); ui_employee->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_clients_clicked()    { ui->stackedWidget->setCurrentIndex(3); ui_client->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_suppliers_clicked()  { ui->stackedWidget->setCurrentIndex(4); ui_supplier->tabWidget->setCurrentIndex(0); }
void MainWindow::on_nav_equipments_clicked() { ui->stackedWidget->setCurrentIndex(5); ui_equipment->tabWidget->setCurrentIndex(0); }
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
    
    // Stop OST1 immediately and start OST2 when coming from a management page
    int prevIndex = ui->stackedWidget->currentIndex();
    if (prevIndex >= 2 && prevIndex <= 6) {
        loginAudioPlayer->stop();
        homeAudioOutput->setVolume(currentVolume);
        homeAudioPlayer->setPosition(0);
        homeAudioPlayer->play();
    }
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
    QString role = ui_employee->le_fonction->text().trimmed();
    double currentSalary = ui_employee->dsb_salaire->value();

    if (role.isEmpty()) {
        ui_employee->lbl_salary_insight->setText("Market Avg: --");
        ui_employee->lbl_salary_insight->setStyleSheet("color: #D4AF37; font-size: 11px; font-weight: bold; background: transparent;");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT AVG(SALARY) FROM EMPLOYEES WHERE JOB_TITLE = :role");
    q.bindValue(":role", role);
    
    if (q.exec() && q.next()) {
        double avg = q.value(0).toDouble();
        if (avg > 0) {
            QString trend = (currentSalary > avg) ? "↑ High" : (currentSalary < avg) ? "↓ Low" : "● Fair";
            QString color = (currentSalary > avg * 1.5) ? "#FF5252" : (currentSalary > avg) ? "#D4AF37" : "#4CAF50";
            
            ui_employee->lbl_salary_insight->setText(QString("Market Avg: $%1 (%2)").arg(avg, 0, 'f', 0).arg(trend));
            ui_employee->lbl_salary_insight->setStyleSheet(QString("color: %1; font-size: 11px; font-weight: bold; background: transparent;").arg(color));
        } else {
            ui_employee->lbl_salary_insight->setText("New Role: Competitive Area");
            ui_employee->lbl_salary_insight->setStyleSheet("color: #D4AF37; font-size: 11px; font-weight: bold; background: transparent;");
        }
    }
}

void MainWindow::onSuggestSalary()
{
    if (!ui_employee) return;
    QString role = ui_employee->le_fonction->text().trimmed();
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
    int qrSize = qr.getSize();
    int imgSize = (qrSize + border * 2) * pixelSize;

    QImage img(imgSize, imgSize, QImage::Format_RGB32);
    img.fill(Qt::white);

    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {
            if (qr.getModule(x, y)) {
                for (int dy = 0; dy < pixelSize; dy++) {
                    for (int dx = 0; dx < pixelSize; dx++) {
                        img.setPixel((x + border) * pixelSize + dx,
                                     (y + border) * pixelSize + dy,
                                     qRgb(0, 0, 0));
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

void MainWindow::onOrderRefreshCatalog()
{
    if (!ui_order) return;
    
    QSqlQuery query;
    if (!query.exec("SELECT order_id, order_type, total_quantity, total_price, client_id FROM ORDERS ORDER BY order_id")) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to load orders.\n\nTechnical details: " + query.lastError().databaseText());
        return;
    }
    
    // Clear existing rows
    ui_order->table_catalog->setRowCount(0);
    
    int row = 0;
    while (query.next()) {
        ui_order->table_catalog->insertRow(row);
        
        // Order ID
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(query.value(0).toString());
        orderIdItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 0, orderIdItem);
        
        // Type
        QTableWidgetItem *typeItem = new QTableWidgetItem(trKey(query.value(1).toString()));
        typeItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        ui_order->table_catalog->setItem(row, 1, typeItem);
        
        // Quantity
        QTableWidgetItem *qtyItem = new QTableWidgetItem(query.value(2).toString());
        qtyItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 2, qtyItem);
        
        double unitPrice = query.value(3).toDouble();
        double totalPrice = unitPrice * query.value(2).toInt();
        QTableWidgetItem *unitPriceItem = new QTableWidgetItem(QString::number(unitPrice, 'f', 2));
        unitPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 3, unitPriceItem);
        
        // Total Price
        QTableWidgetItem *totalPriceItem = new QTableWidgetItem(QString::number(totalPrice, 'f', 2));
        totalPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 4, totalPriceItem);
        
        // Buyer ID
        QTableWidgetItem *buyerItem = new QTableWidgetItem(query.value(4).toString());
        buyerItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 5, buyerItem);
        
        // QR Code thumbnail
        QString qrText = "Order #" + query.value(0).toString()
                        + " | Type: " + query.value(1).toString()
                        + " | Qty: " + query.value(2).toString()
                        + " | Unit: $" + QString::number(unitPrice, 'f', 2)
                        + " | Total: $" + QString::number(totalPrice, 'f', 2)
                        + " | Client: " + query.value(4).toString();
        QPixmap qrPix = generateQrPixmap(qrText, 2, 1);
        QTableWidgetItem *qrItem = new QTableWidgetItem();
        qrItem->setData(Qt::DecorationRole, qrPix.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation));
        qrItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 6, qrItem);
        ui_order->table_catalog->setRowHeight(row, 62);
        
        row++;
    }
    
    // Keep columns stretched so the table always fills available width.
    ui_order->table_catalog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::onOrderSearchCatalog()
{
    if (!ui_order) return;
    
    QString searchText = ui_order->le_catalog_search->text().trimmed();
    
    if (searchText.isEmpty()) {
        onOrderRefreshCatalog();
        return;
    }
    
    QSqlQuery query;
    query.prepare("SELECT order_id, order_type, total_quantity, total_price, client_id FROM ORDERS "
                  "WHERE CAST(order_id AS VARCHAR2(50)) LIKE :search "
                  "OR UPPER(order_type) LIKE :search "
                  "OR CAST(client_id AS VARCHAR2(50)) LIKE :search "
                  "ORDER BY order_id");
    query.bindValue(":search", "%" + searchText.toUpper() + "%");
    
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", 
            "Failed to search orders.\n\nTechnical details: " + query.lastError().databaseText());
        return;
    }
    
    // Clear existing rows
    ui_order->table_catalog->setRowCount(0);
    
    int row = 0;
    while (query.next()) {
        ui_order->table_catalog->insertRow(row);
        
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(query.value(0).toString());
        orderIdItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 0, orderIdItem);

        QTableWidgetItem *typeItem = new QTableWidgetItem(trKey(query.value(1).toString()));
        typeItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        ui_order->table_catalog->setItem(row, 1, typeItem);

        QTableWidgetItem *qtyItem = new QTableWidgetItem(query.value(2).toString());
        qtyItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 2, qtyItem);
        
        double unitPrice = query.value(3).toDouble();
        double totalPrice = unitPrice * query.value(2).toInt();
        QTableWidgetItem *unitPriceItem = new QTableWidgetItem(QString::number(unitPrice, 'f', 2));
        unitPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 3, unitPriceItem);
        
        QTableWidgetItem *totalPriceItem = new QTableWidgetItem(QString::number(totalPrice, 'f', 2));
        totalPriceItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui_order->table_catalog->setItem(row, 4, totalPriceItem);
        
        QTableWidgetItem *buyerItem = new QTableWidgetItem(query.value(4).toString());
        buyerItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 5, buyerItem);
        
        // QR Code thumbnail
        QString qrText = "Order #" + query.value(0).toString()
                        + " | Type: " + query.value(1).toString()
                        + " | Qty: " + query.value(2).toString()
                        + " | Unit: $" + QString::number(unitPrice, 'f', 2)
                        + " | Total: $" + QString::number(totalPrice, 'f', 2)
                        + " | Client: " + query.value(4).toString();
        QPixmap qrPix = generateQrPixmap(qrText, 2, 1);
        QTableWidgetItem *qrItem = new QTableWidgetItem();
        qrItem->setData(Qt::DecorationRole, qrPix.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation));
        qrItem->setTextAlignment(Qt::AlignCenter);
        ui_order->table_catalog->setItem(row, 6, qrItem);
        ui_order->table_catalog->setRowHeight(row, 62);
        
        row++;
    }
    
    ui_order->table_catalog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", SUPPLIER_ID AS \"ID\", "
        "SUPPLIER_NAME AS \"Company\", ADDRESS AS \"Address\","
        " EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\", TYPE_NOTIFICATION AS \"Type\","
        " POSTAL_CODE AS \"Postal Code\""
        " FROM SUPPLIERS ORDER BY SUPPLIER_ID"
    );

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
              " TYPE_NOTIFICATION, POSTAL_CODE, REGISTRATION_DATE, ACCOUNT_STATUS, OPENING_TIME, CLOSING_TIME)"
              " VALUES (:id, :nom, :addr, :email, :tel, :type, :cp, SYSDATE, 'Active', :openTime, :closeTime)");
    q.bindValue(":id",        suppId);
    q.bindValue(":nom",       nom);
    q.bindValue(":addr",      addr);
    q.bindValue(":email",     email);
    q.bindValue(":tel",       tel);
    q.bindValue(":type",      type);
    q.bindValue(":cp",        cp);
    q.bindValue(":openTime",  openTime);
    q.bindValue(":closeTime", closeTime);

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
              " TYPE_NOTIFICATION=:type, POSTAL_CODE=:cp, OPENING_TIME=:openTime, CLOSING_TIME=:closeTime"
              " WHERE SUPPLIER_ID=:id");
    q.bindValue(":nom",       nom);
    q.bindValue(":addr",      addr);
    q.bindValue(":email",     email);
    q.bindValue(":tel",       tel);
    q.bindValue(":type",      type);
    q.bindValue(":cp",        cp);
    q.bindValue(":openTime",  openTime);
    q.bindValue(":closeTime", closeTime);
    q.bindValue(":id",        suppId);

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
    hq.prepare("SELECT OPENING_TIME, CLOSING_TIME FROM SUPPLIERS WHERE SUPPLIER_ID = :id");
    hq.bindValue(":id", suppId.toInt());
    if (hq.exec() && hq.next()) {
        QString ot = hq.value(0).toString();
        QString ct = hq.value(1).toString();
        if (m_teOpeningHour)
            m_teOpeningHour->setTime(ot.isEmpty() ? QTime(8, 0) : QTime::fromString(ot, "HH:mm"));
        if (m_teClosingHour)
            m_teClosingHour->setTime(ct.isEmpty() ? QTime(18, 0) : QTime::fromString(ct, "HH:mm"));
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
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", SUPPLIER_ID AS \"ID\", "
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
    // Protocol: sms:<number>?body=<message>
    // This allows Windows to open the "Phone Link" app (or default handler) 
    // to send the message using your synced mobile device.
    
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
    // Ignore ORA-01430 if they already exist
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
    QSqlQuery qEquip("SELECT EQUIPMENT_ID, DESCRIPTION FROM EQUIPMENT ORDER BY DESCRIPTION");
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
    QSqlQuery qEq("SELECT EQUIPMENT_ID, DESCRIPTION FROM EQUIPMENT");
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

void MainWindow::onEquipmentClearFields()
{
    if (ui_equipment) {
        ui_equipment->le_id->clear();
        ui_equipment->le_type->clear();
        ui_equipment->de_date_achat->setDate(QDate::currentDate());
        ui_equipment->te_desc->clear();
        ui_equipment->sb_quantity->setValue(0);

        ui_equipment->dsb_unit_price->setValue(0.0);
        ui_equipment->cb_status->setCurrentIndex(0); // Reset to 'Available'
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
    ui_client->le_id->setValidator(idVal);
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
    QString queryStr = "SELECT TO_CHAR(LOG_DATE, 'DD/MM/YYYY HH24:MI') AS \"Timestamp\", "
                       "EMPLOYEE_NAME AS \"Operative\", "
                       "ACTION_DETAILS AS \"Action Sequence\" "
                       "FROM APP_HISTORY WHERE MODULE_NAME = 'Clients' ORDER BY LOG_DATE DESC";

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(queryStr);
    
    if (model->lastError().isValid()) {
        QMessageBox::warning(this, "Trace Error", "Failed to retrieve Cyber Trace:\n" + model->lastError().text());
        return;
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

void MainWindow::setupEquipmentStats()
{
    if (!ui_equipment || !ui_equipment->tab_stats) return;

    // ─── 1. Absolute Cleanup ───
    // We use a named container to ensure 100% cleanup of previous dashboard instances
    QWidget *oldContainer = ui_equipment->tab_stats->findChild<QWidget*>("dashContainer");
    if (oldContainer) {
        oldContainer->deleteLater();
        // We must also clear the layout to prevent constraints fighting
        if (ui_equipment->tab_stats->layout()) delete ui_equipment->tab_stats->layout();
    }
    
    // Hide ANY legacy widgets left from the .ui file to prevent messiness
    for (auto *w : ui_equipment->tab_stats->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        w->hide();
    }

    QWidget *dashContainer = new QWidget(ui_equipment->tab_stats);
    dashContainer->setObjectName("dashContainer");
    dashContainer->show();

    QVBoxLayout *mainLayoutWrapper = new QVBoxLayout(ui_equipment->tab_stats);
    mainLayoutWrapper->setContentsMargins(0,0,0,0);
    mainLayoutWrapper->addWidget(dashContainer);

    QVBoxLayout *mainLay = new QVBoxLayout(dashContainer);
    mainLay->setContentsMargins(30, 20, 30, 30);
    mainLay->setSpacing(25);

    // ─── 2. Data Acquisition ──────────
    int total = 0, available = 0, inUse = 0, maint = 0, retired = 0;
    double totalVal = 0.0, lastMonthVal = 0.0;
    int lastMonthTotal = 0;
    QString lm = QDate::currentDate().addMonths(-1).toString("yyyy-MM");

    QSqlQuery qSt("SELECT STATUS, COUNT(*), SUM(UNIT_PRICE) FROM EQUIPMENT GROUP BY STATUS");
    while (qSt.next()) {
        QString s = qSt.value(0).toString();
        int c = qSt.value(1).toInt();
        total += c;
        totalVal += qSt.value(2).toDouble();
        if (s == "Available") available = c;
        else if (s == "In Use") inUse = c;
        else if (s == "Under Maintenance") maint = c;
        else if (s == "Retired") retired = c;
    }
    
    QSqlQuery qTrend;
    qTrend.prepare("SELECT * FROM (SELECT COUNT(*), SUM(UNIT_PRICE) FROM EQUIPMENT WHERE TO_CHAR(PURCHASE_DATE, 'YYYY-MM') = :m) WHERE ROWNUM <= 1");
    qTrend.bindValue(":m", lm);
    if (qTrend.exec() && qTrend.next()) {
        lastMonthTotal = qTrend.value(0).toInt();
        lastMonthVal = qTrend.value(1).toDouble();
    }

    // ─── 3. Header & KPI Cards ────────
    QHBoxLayout *headerLay = new QHBoxLayout();
    headerLay->setContentsMargins(10, 0, 10, 0);

    // Styled PDF Report Button
    QPushButton *reportBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\x84 Generate Intelligence Report"), dashContainer);
    reportBtn->setFixedSize(280, 48);
    reportBtn->setCursor(Qt::PointingHandCursor);
    reportBtn->setStyleSheet(
        "QPushButton { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #D4AF37, stop:0.5 #B8925A, stop:1 #8B6F47);"
        "  color: #1A1208; font-weight: 900; font-size: 13px; border-radius: 24px; "
        "  border: 1px solid rgba(255,255,255,0.4); letter-spacing: 1.2px; text-transform: uppercase; "
        "  padding: 0 25px; "
        "} "
        "QPushButton:hover { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F0E0C0, stop:1 #D4AF37);"
        "  color: black; border-color: white;"
        "} "
        "QPushButton:pressed { background: #5A4A32; color: #D4AF37; }"
    );
    connect(reportBtn, &QPushButton::clicked, this, &MainWindow::onEquipmentExportStatsPDF);
    headerLay->addStretch();
    headerLay->addWidget(reportBtn);
    mainLay->addLayout(headerLay);

    QHBoxLayout *cardsLay = new QHBoxLayout();
    cardsLay->setSpacing(20);
    cardsLay->addWidget(new StatCard("TOTAL FLEET", QString::number(total), QString::number(qAbs(total - lastMonthTotal)), total >= lastMonthTotal, dashContainer));
    cardsLay->addWidget(new StatCard("VALUATION", QString::number(totalVal, 'f', 0) + " DT", QString::number(qAbs(totalVal - lastMonthVal), 'f', 0), totalVal >= lastMonthVal, dashContainer));
    cardsLay->addWidget(new StatCard("OPERATIONAL", QString::number(total > 0 ? (available * 100 / total) : 0) + "%", "STABLE", true, dashContainer));
    mainLay->addLayout(cardsLay);

    // ─── 4. Main Content Area ─────────
    QHBoxLayout *content = new QHBoxLayout();
    content->setSpacing(30);

    // Left: Donut
    QGroupBox *boxL = new QGroupBox("Status Distribution Matrix", dashContainer);
    boxL->setStyleSheet("QGroupBox { background: rgba(30, 20, 10, 0.4); border: 2px solid #8B6F47; border-radius: 16px; margin-top: 20px; color: #D4AF37; font-weight: bold; font-size: 14px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 5px 15px; background: #1A1208; border-radius: 8px; }");
    QVBoxLayout *vL = new QVBoxLayout(boxL);
    AnimatedDonutChart *donut = new AnimatedDonutChart(boxL);
    donut->setData({
        {"Available", (double)available, QColor("#4CAF50")},
        {"In Use", (double)inUse, QColor("#2196F3")},
        {"Maintenance", (double)maint, QColor("#FF9800")},
        {"Retired", (double)retired, QColor("#F44336")}
    });
    donut->setMinimumSize(320, 320);
    vL->addWidget(donut);
    content->addWidget(boxL, 3);
    donut->startAnimation();

    // Right Column: Ranking and Spending Chart
    QVBoxLayout *vR = new QVBoxLayout();
    vR->setSpacing(20);
    
    QGroupBox *rankB = new QGroupBox("Utilization Ranking", dashContainer);
    rankB->setStyleSheet(boxL->styleSheet());
    QVBoxLayout *rL = new QVBoxLayout(rankB);
    rL->setContentsMargins(15, 30, 15, 15);
    
    QSqlQuery rq("SELECT EQUIPMENT_TYPE, COUNT(*) as cnt FROM EQUIPMENT GROUP BY EQUIPMENT_TYPE ORDER BY cnt DESC");
    int ri = 1;
    bool hasData = false;
    while(rq.next() && ri <= 5) {
        hasData = true;
        QString type = rq.value(0).toString();
        int count = rq.value(1).toInt();
        QLabel *rl = new QLabel(QString::fromUtf8("   %1. %2 — %3 activations").arg(ri++).arg(type).arg(count), rankB);
        rl->setStyleSheet("color: #F0E0C0; font-size: 13px; font-weight: 600; padding: 10px; border-bottom: 1px solid rgba(212,175,55,0.12);");
        rL->addWidget(rl);
    }
    if (!hasData) {
        QLabel *eL = new QLabel("No gear tracked yet", rankB);
        eL->setStyleSheet("color: rgba(212,175,55,0.4); font-style: italic;");
        eL->setAlignment(Qt::AlignCenter);
        rL->addWidget(eL);
    }
    rL->addStretch();
    vR->addWidget(rankB, 3); // Increased stretch to fill space

    // Spending Chart
    QGroupBox *sB = new QGroupBox("Spending Volatility", dashContainer);
    sB->setFixedHeight(160);
    sB->setStyleSheet(boxL->styleSheet());
    QHBoxLayout *sL = new QHBoxLayout(sB);
    sL->setContentsMargins(15, 25, 15, 10);
    sL->setSpacing(10);
    
    for (int i=5; i>=0; --i) {
        QVBoxLayout *barCol = new QVBoxLayout();
        QWidget *bar = new QWidget();
        int h = 20 + (rand() % 70);
        bar->setFixedSize(16, h);
        bar->setStyleSheet(
            "QWidget { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #D4AF37, stop:1 rgba(212,175,55,0.1));"
            "border-radius: 4px; border: 1px solid rgba(139,111,71,0.5); }"
            "QWidget:hover { background: #D4AF37; border-color: white; }"
        );
        
        QLabel *l = new QLabel(QDate::currentDate().addMonths(-i).toString("MMM"), sB);
        l->setStyleSheet("color: #B8925A; font-size: 9px; font-weight: bold; background: transparent;");
        l->setAlignment(Qt::AlignCenter);
        
        barCol->addStretch();
        barCol->addWidget(bar, 0, Qt::AlignCenter);
        barCol->addWidget(l);
        sL->addLayout(barCol);
    }
    vR->addWidget(sB, 2);

    content->addLayout(vR, 2);
    mainLay->addLayout(content);
}




void MainWindow::setupSupplierStats()
{
    if (!ui_supplier) return;

    // --- 1. Top Level Metrics (Aggregated from Database) ---
    QSqlQuery qMetrics;
    
    // Average Quality / Speed surrogate from ratings
    qMetrics.exec("SELECT AVG(AVERAGE_RATING) FROM SUPPLIERS WHERE AVERAGE_RATING > 0");
    double avgRating = 0;
    if (qMetrics.next()) avgRating = qMetrics.value(0).toDouble();
    
    int qualityScore = qBound(0.0, avgRating * 20.0, 100.0); // Scale 1-5 to 0-100
    int speedScore = qBound(0.0, (avgRating - 0.5) * 20.0, 100.0); // Simulating variation
    
    ui_supplier->pb_quality->setValue(qualityScore);
    ui_supplier->pb_speed->setValue(speedScore);

    // Retention / Accuracy surrogate
    ui_supplier->lbl_percent_retention->setText(QString::number(qMin(100, 80 + int(avgRating * 4))) + "%");
    ui_supplier->lbl_percent_accuracy->setText(QString::number(qMin(100, 75 + int(avgRating * 5))) + "%");

    // --- 2. Chart 1: Product Categories (Real Data) ---
    QPieSeries *seriesCat = new QPieSeries();
    seriesCat->setHoleSize(0.45);
    
    QSqlQuery qCats("SELECT TYPE_NOTIFICATION, COUNT(*) FROM SUPPLIERS GROUP BY TYPE_NOTIFICATION");
    int catIdx = 0;
    QStringList catColors = {"#D4AF37", "#8B6F47", "#5D4037", "#2E1A0C", "#A0825A"};
    while(qCats.next()) {
        QString cat = qCats.value(0).toString();
        int count = qCats.value(1).toInt();
        if (cat.isEmpty()) cat = trKey("Other");
        QPieSlice *slice = seriesCat->append(cat, count);
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

    // --- 3. Chart 2: Monthly Satisfaction Trend (Real Data) ---
    QBarSet *setScore = new QBarSet(trKey("Avg Rating"));
    QStringList categories;
    
    QMap<int, QList<double>> monthScores;
    QSqlQuery qGet("SELECT RATINGS_JSON FROM SUPPLIERS WHERE RATINGS_JSON IS NOT NULL");
    while (qGet.next()) {
        QString json = qGet.value(0).toString();
        if (json.isEmpty()) continue;
        QJsonArray arr = QJsonDocument::fromJson(json.toUtf8()).array();
        for (int i=0; i<arr.size(); i++) {
            QJsonObject obj = arr[i].toObject();
            QDate d = QDate::fromString(obj["date"].toString().left(10), "yyyy-MM-dd");
            int m = d.month(); // 1-12
            if (m >= 1 && m <= 12) {
                monthScores[m].append(obj["rating"].toDouble());
            }
        }
    }
    
    QList<int> months = monthScores.keys();
    std::sort(months.begin(), months.end());
    for (int m : months) {
        double sum = 0;
        for (double val : monthScores[m]) sum += val;
        categories << QDate(2000, m, 1).toString("Mon");
        *setScore << (sum / monthScores[m].size());
    }
    setScore->setColor(QColor("#D4AF37"));

    QBarSeries *seriesTrend = new QBarSeries();
    seriesTrend->append(setScore);

    QChart *chartTrend = new QChart();
    chartTrend->addSeries(seriesTrend);
    chartTrend->setTitle(trKey("Monthly Satisfaction Trend"));
    chartTrend->setTitleBrush(QBrush(QColor("#D4AF37")));
    chartTrend->setAnimationOptions(QChart::SeriesAnimations);
    chartTrend->setBackgroundBrush(Qt::transparent);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(Qt::white);
    chartTrend->addAxis(axisX, Qt::AlignBottom);
    seriesTrend->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 5);
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

    // --- 4. Top Performer Card (Dynamic) ---
    QSqlQuery qTop(
        "SELECT SUPPLIER_NAME, AVERAGE_RATING "
        "FROM SUPPLIERS "
        "WHERE AVERAGE_RATING > 0 "
        "ORDER BY AVERAGE_RATING DESC"
    );
    if (qTop.next()) {
        QString topName = qTop.value(0).toString();
        double topRating = qTop.value(1).toDouble();
        ui_supplier->lbl_top_performer->setText(
            trKey("🏆 Top Performer: ") + topName + "\n" +
            trKey("Avg Rating: ") + QString::number(topRating, 'f', 1) + "/5.0\n" +
            trKey("Status: Optimal")
        );
    }
}

void MainWindow::setupClientCalendar()
{
    // 1. Create the Tab Widget if it doesn't exist (it should, 'tabWidget')
    // We will add a new tab to it.
    QWidget *calendarTab = new QWidget();
    
    // Layout for the new tab
    QHBoxLayout *mainLayout = new QHBoxLayout(calendarTab);
    
    // --- Left Side: Calendar ---
    QCalendarWidget *calendar = new QCalendarWidget();
    calendar->setGridVisible(true);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    
    // Professional Styling
    calendar->setStyleSheet(R"(
        QCalendarWidget QToolButton {
            color: #333;
            icon-size: 24px;
            font-weight: bold;
            background-color: #E0E0E0;
            border-radius: 5px;
            margin: 5px;
        }
        QCalendarWidget QMenu {
            width: 150px;
            left: 20px;
            color: white;
            font-size: 14px;
            background-color: #8B6F47;
        }
        QCalendarWidget QSpinBox {
            width: 80px;
            font-size: 14px;
            color: #8B6F47;
            font-weight: bold;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar { 
            background-color: white; 
            border: 1px solid #C4C4C4;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            padding: 5px;
        }
        QCalendarWidget QAbstractItemView:enabled {
            font-size: 14px;
            color: #333;
            background-color: white;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
    )");

    // --- Right Side: Events Panel ---
    QGroupBox *eventGroup = new QGroupBox(trKey("Upcoming Events"));
    eventGroup->setProperty("trTitleKey", "Upcoming Events");
    eventGroup->setStyleSheet(R"(
        QGroupBox {
            border: 1px solid #D0D0D0;
            border-radius: 8px;
            margin-top: 20px;
            background: rgba(255, 255, 255, 0.9);
            font-weight: bold;
            color: #8B6F47;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 10px;
            background-color: white;
        }
    )");
    
    QVBoxLayout *eventLayout = new QVBoxLayout(eventGroup);
    
    QLabel *lblDate = new QLabel(trKey("Select a date..."));
    setTrKey(lblDate, "Select a date...");
    lblDate->setStyleSheet("font-size: 16px; font-weight: bold; color: #555; margin-bottom: 10px;");
    lblDate->setAlignment(Qt::AlignCenter);
    
    QListWidget *eventList = new QListWidget();
    eventList->setStyleSheet(R"(
        QListWidget {
            border: none;
            background: transparent;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #EEE;
        }
        QListWidget::item:selected {
            background-color: rgba(139, 111, 71, 0.1);
            color: #333;
        }
    )");
    
    // Initial Mock Events
    {
        auto *item = new QListWidgetItem();
        setItemTrKey(item, "📅  09:00 AM - Team Sync");
        eventList->addItem(item);
    }
    {
        auto *item = new QListWidgetItem();
        setItemTrKey(item, "💼  11:30 AM - Client Meeting (John Doe)");
        eventList->addItem(item);
    }
    {
        auto *item = new QListWidgetItem();
        setItemTrKey(item, "📊  02:00 PM - Quarterly Review");
        eventList->addItem(item);
    }
    
    eventLayout->addWidget(lblDate);
    eventLayout->addWidget(eventList);
    
    // --- Connect Interaction ---
    connect(calendar, &QCalendarWidget::clicked, [lblDate, eventList](const QDate &date){
        lblDate->setText(date.toString("dddd, MMMM d, yyyy"));
        
        // Mocking dynamic events based on day logic
        eventList->clear();
        if (date.day() % 3 == 0) {
            auto *item = new QListWidgetItem();
            setItemTrKey(item, "✅  No events scheduled.");
            eventList->addItem(item);
        } else if (date.day() % 2 == 0) {
            auto *item1 = new QListWidgetItem();
            setItemTrKey(item1, "📞  10:00 AM - Call with Supplier");
            eventList->addItem(item1);

            auto *item2 = new QListWidgetItem();
            setItemTrKey(item2, "🛒  01:00 PM - Order #1234 Delivery");
            eventList->addItem(item2);

            auto *item3 = new QListWidgetItem();
            setItemTrKey(item3, "📝  04:00 PM - Sign Contract");
            eventList->addItem(item3);
        } else {
            auto *item1 = new QListWidgetItem();
            setItemTrKey(item1, "📅  09:00 AM - Team Sync");
            eventList->addItem(item1);

            auto *item2 = new QListWidgetItem();
            setItemTrKey(item2, "💼  11:30 AM - Client Meeting");
            eventList->addItem(item2);

            auto *item3 = new QListWidgetItem();
            setItemTrKey(item3, "📊  03:00 PM - Strategy Workshop");
            eventList->addItem(item3);
        }
    });

    // Add widgets to main layout
    mainLayout->addWidget(calendar, 70); // 70% width
    mainLayout->addWidget(eventGroup, 30); // 30% width
    
    // Add the new tab
    ui_client->tabWidget->addTab(calendarTab, trKey("Smart Calendar"));
    setTabTextTr(ui_client->tabWidget, calendarTab, "Smart Calendar");
    
    // Set an icon if available, or just text
    // ui_client->tabWidget->setTabIcon(..., QIcon(":/assets/icon_calendar.png"));
}

void MainWindow::setupEmployeeModes()
{
    // Rename tab_add to "Manage Employees"
    setTabTextTr(ui_employee->tabWidget, ui_employee->tab_add, "Manage Employees");

    // tab_modify does not exist in this UI, so no need to remove it.


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
        } else {
            ui_employee->group_add->setTitle(trKey("Modify Employee"));
            ui_employee->btn_add->setVisible(false);
            ui_employee->btn_modify->setVisible(true);
            ui_employee->le_id->setEnabled(false);
        }
    };
    
    rbAdd->connect(rbAdd, &QRadioButton::toggled, updateUI);
    rbMod->connect(rbMod, &QRadioButton::toggled, [=](bool checked){ updateUI(!checked); });
    
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

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_supplier->groupBox_gestion->move(20, 70);

    ui_supplier->groupBox_gestion->setProperty("trTitleAddKey", "Add New Supplier");
    ui_supplier->groupBox_gestion->setProperty("trTitleModKey", "Manage Existing Supplier");
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
    m_supplierBellBtn->setGeometry(1060, 8, 44, 44);
    m_supplierBellBtn->setStyleSheet(
        "QPushButton { background-color: #8B6F47; border-radius: 22px; color: white; font-size: 20px; border: none; }"
        "QPushButton:hover { background-color: #a3845a; }"
        "QPushButton:pressed{ background-color: #6b5535; }");
    m_supplierBellBtn->setCursor(Qt::PointingHandCursor);
    m_supplierBellBtn->setToolTip("Supplier Notifications");
    m_supplierBellBtn->show();
    connect(m_supplierBellBtn, &QPushButton::clicked, this, &MainWindow::onSupplierBellClicked);
    // Defer notification scan until after all setup is complete
    QTimer::singleShot(1500, this, &MainWindow::checkAndPostSupplierNotifications);
    // ---


    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_supplier->groupBox_gestion->setTitle(trKey("Add New Supplier"));
            ui_supplier->btn_add->setVisible(true);
            ui_supplier->btn_modify->setVisible(false);
            ui_supplier->btn_delete->setVisible(false);
        } else {
            ui_supplier->groupBox_gestion->setTitle(trKey("Manage Existing Supplier"));
            ui_supplier->btn_add->setVisible(false);
            ui_supplier->btn_modify->setVisible(true);
            ui_supplier->btn_delete->setVisible(true);
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


void MainWindow::setupEquipmentModes()
{
    // Ensure audit log table exists


    // tab_gestion
    setTabTextTr(ui_equipment->tabWidget, ui_equipment->tab_gestion, "Manage Equipment");

    QRadioButton *rbAdd = new QRadioButton(trKey("Add Equipment"), ui_equipment->tab_gestion);
    QRadioButton *rbMod = new QRadioButton(trKey("Manage Equipment"), ui_equipment->tab_gestion);
    rbAdd->setObjectName("rb_equipment_add_mode");
    rbMod->setObjectName("rb_equipment_mod_mode");
    setTrKey(rbAdd, "Add Equipment");
    setTrKey(rbMod, "Manage Equipment");

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 180, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_equipment->groupBox_gestion->move(20, 70);

    ui_equipment->groupBox_gestion->setProperty("trTitleAddKey", "Add Equipment");
    ui_equipment->groupBox_gestion->setProperty("trTitleModKey", "Manage Equipment");
    ui_equipment->groupBox_gestion->setProperty("trModeAddRadio", "rb_equipment_add_mode");
    ui_equipment->groupBox_gestion->setProperty("trModeModRadio", "rb_equipment_mod_mode");

    // --- Form Completion Progress Bar ---
    m_equipProgress = new QProgressBar(ui_equipment->groupBox_gestion);
    m_equipProgress->setRange(0, 100);
    m_equipProgress->setValue(0);
    m_equipProgress->setTextVisible(false);
    m_equipProgress->setFixedHeight(12);
    m_equipProgress->setStyleSheet(
        "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #D4AF37); border-radius: 5px; }");

    QLabel *pTitle = new QLabel("Form Completion:", ui_equipment->groupBox_gestion);
    pTitle->setObjectName("lbl_prog_title");
    pTitle->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 11px; font-family: 'Segoe UI';");

    auto makeInd = [&](const QString &txt, const QString &obj) {
        QLabel *l = new QLabel(txt, ui_equipment->groupBox_gestion);
        l->setObjectName(obj);
        l->setStyleSheet("color: rgba(255,255,255,0.4); font-size: 11px; font-weight: bold;");
        return l;
    };
    m_eqTypeInd = makeInd("[🔨 Type ⬜]", "ind_type"); 
    m_eqDateInd = makeInd("[📅 Date ⬜]", "ind_date"); 
    m_eqPriceInd= makeInd("[💰 Price ⬜]", "ind_price"); 
    m_eqDescInd = makeInd("[📝 Desc ⬜]", "ind_desc");

    ui_equipment->label_type->setText(QString::fromUtf8("\xF0\x9F\x94\xA7 Type:"));
    ui_equipment->label_date_achat->setText(QString::fromUtf8("\xF0\x9F\x93\x85 Purchase Date:"));
    ui_equipment->label_unit_price->setText(QString::fromUtf8("\xF0\x9F\x92\xB0 Unit Price (dt):"));
    ui_equipment->label_etat->setText(QString::fromUtf8("\xF0\x9F\x93\x8A Status:"));
    ui_equipment->label_quantity->setText(QString::fromUtf8("\xF0\x9F\x93\xA6 Quantity:"));
    ui_equipment->label_desc->setText(QString::fromUtf8("\xF0\x9F\x93\x9D Description:"));

    const QString compactLabelStyle = "color: white; font-size: 11px; font-weight: bold; background: transparent;";
    ui_equipment->label_type->setStyleSheet(compactLabelStyle);
    ui_equipment->label_date_achat->setStyleSheet(compactLabelStyle);
    ui_equipment->label_unit_price->setStyleSheet(compactLabelStyle);
    ui_equipment->label_etat->setStyleSheet(compactLabelStyle);
    ui_equipment->label_quantity->setStyleSheet(compactLabelStyle);
    ui_equipment->label_desc->setStyleSheet(compactLabelStyle);
    ui_equipment->label_id->setStyleSheet(compactLabelStyle);

    ui_equipment->le_type->setFixedHeight(32);
    ui_equipment->de_date_achat->setFixedHeight(32);
    ui_equipment->dsb_unit_price->setFixedHeight(32);
    ui_equipment->cb_status->setFixedHeight(32);
    ui_equipment->sb_quantity->setFixedHeight(32);
    ui_equipment->te_desc->setFixedHeight(80);

    auto updateUI = [=](bool isAdd) {
        // Shift amount for other fields when ID is hidden
        int yOffset = isAdd ? 22 : 0;
        
        m_equipProgress->setVisible(isAdd);
        pTitle->setVisible(isAdd);
        m_eqTypeInd->setVisible(isAdd);
        m_eqDateInd->setVisible(isAdd);
        m_eqPriceInd->setVisible(isAdd);
        m_eqDescInd->setVisible(isAdd);

        if(isAdd) {
            ui_equipment->groupBox_gestion->setTitle(trKey("Add Equipment"));
            ui_equipment->btn_add->setVisible(true);
            ui_equipment->btn_modify->setVisible(false);
            ui_equipment->btn_delete->setVisible(false);
            
            ui_equipment->le_id->setVisible(false);
            ui_equipment->label_id->setVisible(false);

            // Position progress elements
            pTitle->move(30, 45);
            m_equipProgress->setGeometry(30, 68, 555, 12);
            m_eqTypeInd->move(30, 85);
            m_eqDateInd->move(130, 85);
            m_eqPriceInd->move(230, 85);
            m_eqDescInd->move(330, 85);

        } else {
            ui_equipment->groupBox_gestion->setTitle(trKey("Manage Equipment"));
            ui_equipment->btn_add->setVisible(false);
            ui_equipment->btn_modify->setVisible(true);
            ui_equipment->btn_delete->setVisible(true);
            
            ui_equipment->le_id->setVisible(true);
            ui_equipment->label_id->setVisible(true);
            ui_equipment->le_id->setEnabled(true);
            ui_equipment->le_id->setPlaceholderText("");
            ui_equipment->label_id->move(ui_equipment->label_id->x(), 70);
            ui_equipment->le_id->move(ui_equipment->le_id->x(), 70);
        }

        // Compact field stack to match the target form proportions
        const int fieldGap = 42;  // tighter vertical gap between fields
        const int typeY = 110 + yOffset;
        const int dateY = typeY + fieldGap;
        const int priceY = dateY + fieldGap;
        const int statusY = priceY + fieldGap;
        const int qtyY = statusY + fieldGap;
        const int descY = qtyY + fieldGap;

        ui_equipment->label_type->move(ui_equipment->label_type->x(), typeY);
        ui_equipment->le_type->move(ui_equipment->le_type->x(), typeY);

        ui_equipment->label_date_achat->move(ui_equipment->label_date_achat->x(), dateY);
        ui_equipment->de_date_achat->move(ui_equipment->de_date_achat->x(), dateY);

        ui_equipment->label_unit_price->move(ui_equipment->label_unit_price->x(), priceY);
        ui_equipment->dsb_unit_price->move(ui_equipment->dsb_unit_price->x(), priceY);

        ui_equipment->label_etat->move(ui_equipment->label_etat->x(), statusY);
        ui_equipment->cb_status->move(ui_equipment->cb_status->x(), statusY);

        ui_equipment->label_quantity->move(ui_equipment->label_quantity->x(), qtyY);
        ui_equipment->sb_quantity->move(ui_equipment->sb_quantity->x(), qtyY);

        ui_equipment->label_desc->move(ui_equipment->label_desc->x(), descY);
        ui_equipment->te_desc->move(ui_equipment->te_desc->x(), descY);
        ui_equipment->te_desc->setFixedHeight(80);

        // Keep clear space under description so buttons never overlap
        int btnY = descY + 80 + 18;
        ui_equipment->btn_add->move(ui_equipment->btn_add->x(), btnY);
        ui_equipment->btn_modify->move(ui_equipment->btn_modify->x(), btnY);
        ui_equipment->btn_delete->move(ui_equipment->btn_delete->x(), btnY);
        ui_equipment->btn_clear->move(ui_equipment->btn_clear->x(), btnY);
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);

    // Initial resets
    ui_equipment->dsb_unit_price->setValue(0.0);
    ui_equipment->de_date_achat->setDate(QDate::currentDate());

    // --- Connect progress update signals ---
    connect(ui_equipment->le_type, &QLineEdit::textChanged, this, &MainWindow::updateEquipProgress);
    connect(ui_equipment->de_date_achat, &QDateEdit::dateChanged, this, &MainWindow::updateEquipProgress);
    connect(ui_equipment->dsb_unit_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateEquipProgress);
    connect(ui_equipment->te_desc, &QTextEdit::textChanged, this, &MainWindow::updateEquipProgress);
    
    // Initial calculation
    updateEquipProgress();


    // Connect the View tab "?" help button directly (findChild from parent page
    // doesn't reliably cross the QTabWidget boundary, so we connect it here
    // where we have direct access to ui_equipment)
    connect(ui_equipment->btn_help_view, &QToolButton::clicked, this, [this]() {
        showTutorialOverlay(
            "This is the equipment view tutorial. Here you can search, view, modify, and delete equipment."
        );
        ui_equipment->btn_help_view->setChecked(false);
    });

    // Wire up History tab buttons
    connect(ui_equipment->btn_refresh_history, &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryRefresh);
    connect(ui_equipment->btn_history_search,  &QPushButton::clicked, this, &MainWindow::onEquipmentHistorySearch);
    connect(ui_equipment->le_history_search,   &QLineEdit::returnPressed, this, &MainWindow::onEquipmentHistorySearch);
    connect(ui_equipment->btn_export_history,  &QPushButton::clicked, this, &MainWindow::onEquipmentExportPDF);
    connect(ui_equipment->btn_clear_history,   &QPushButton::clicked, this, &MainWindow::onEquipmentHistoryClear);
    connect(ui_equipment->btn_export_stats,    &QPushButton::clicked, this, &MainWindow::onEquipmentExportStatsPDF);
    connect(ui_equipment->btn_bulk_update_status, &QPushButton::clicked, this, &MainWindow::onEquipmentBulkUpdateStatus);

    // Auto-refresh history or chat when switching tabs
    connect(ui_equipment->tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        QWidget *selected = ui_equipment->tabWidget->widget(idx);
        
        // Stop chat timer by default unless on chat tab
        chatRefreshTimer->stop();
        
        if (selected == ui_equipment->tab_history) {
            onEquipmentHistoryRefresh();
        } else if (selected == ui_equipment->tab_stats) {
            setupEquipmentStats();
        } else if (selected == ui_equipment->tab_chat) {
            onChatRefresh();
            onChatEmployeeListRefresh();
            chatRefreshTimer->start(3000); // 3 seconds refresh
        }
    });

    // Voice chat removed as requested
    if (ui_equipment->btn_chat_voice) {
        ui_equipment->btn_chat_voice->hide();
    }

    // Connect Show Name button
    if (auto *showNameBtn = equipmentPage->findChild<QPushButton*>("btn_show_name")) {
        // Functionality removed as requested
        showNameBtn->hide();
    }


    // --- Contrôle de Saisie (Input Validation) ---
    // Only letters and spaces for Type
    QRegularExpression typeRegex("^[a-zA-Z\\s]*$");
    QRegularExpressionValidator *typeVal = new QRegularExpressionValidator(typeRegex, this);
    ui_equipment->le_type->setValidator(typeVal);
    
    // Ensure price is at least 0.01
    ui_equipment->dsb_unit_price->setMinimum(0.00); 

    // Visual feedback for Type
    connect(ui_equipment->le_type, &QLineEdit::textChanged, this, [=](const QString &text){
        if(text.trimmed().isEmpty()) {
            ui_equipment->le_type->setStyleSheet("border: 2px solid #D32F2F; background: #FFEBEE; border-radius: 8px;"); // Red
            ui_equipment->le_type->setToolTip("Type is required!");
        } else {
            ui_equipment->le_type->setStyleSheet(""); // Restore default search style if needed or use previous styling
            ui_equipment->le_type->setToolTip("");
        }
    });

    // Disable Add button if inputs invalid
    auto validateForm = [=](){
        bool isValid = !ui_equipment->le_type->text().trimmed().isEmpty() &&
                       !ui_equipment->te_desc->toPlainText().trimmed().isEmpty() &&
                       ui_equipment->dsb_unit_price->value() > 0;
        ui_equipment->btn_add->setEnabled(isValid);
    };
    connect(ui_equipment->le_type, &QLineEdit::textChanged, validateForm);
    connect(ui_equipment->te_desc, &QTextEdit::textChanged, validateForm);
    connect(ui_equipment->dsb_unit_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged), validateForm);
    
    validateForm();

    // --- NEXUS Integration: Context Menus ---
    ui_equipment->table_equipments->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->table_equipments, &QTableView::customContextMenuRequested, this, &MainWindow::onEquipmentCustomContextMenu);

    ui_equipment->tableView_history_add->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->tableView_history_add, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){ onEquipmentHistoryCustomContextMenu(pos, 0); });
    
    ui_equipment->tableView_history_modify->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->tableView_history_modify, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){ onEquipmentHistoryCustomContextMenu(pos, 1); });

    ui_equipment->tableView_historique->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_equipment->tableView_historique, &QTableView::customContextMenuRequested, this, [this](const QPoint &pos){ onEquipmentHistoryCustomContextMenu(pos, 2); });
}

void MainWindow::setupOrderModes()
{
    // tab_manage
    int idx = ui_order->tabWidget->indexOf(ui_order->tab_manage);
    if(idx != -1) {
        setTabTextTr(ui_order->tabWidget, ui_order->tab_manage, "Manage Orders");
    }

    // Panel matching the "Log Delivery Rating" group box style
    QGroupBox *orderPanel = new QGroupBox(trKey("Manage Orders"), ui_order->tab_manage);
    orderPanel->setObjectName("order_manage_panel");
    orderPanel->setGeometry(28, 60, 615, 440);
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
        "  subcontrol-position: top left;"
        "  padding: 2px 12px;"
        "  background-color: #8B6F47;"
        "  font-weight: bold;"
        "  color: white;"
        "  border-radius: 4px;"
        "}"
    );
    orderPanel->lower();
    orderPanel->show();

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
        // Home page: stop OST1 only — OST2 is started by on_btn_home_clicked when coming from management
        if (loginAudioPlayer && loginAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(loginAudioOutput, [this](){ loginAudioPlayer->stop(); });
        }
    } else {
        // All other pages: stop home-page audio (OST2 + animation track)
        homeWindow->stopHomeAudio();
        if (homeAudioPlayer && homeAudioPlayer->playbackState() == QMediaPlayer::PlayingState) {
            fadeOut(homeAudioOutput, [this](){ homeAudioPlayer->stop(); });
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
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", CLIENT_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
        " FROM CLIENTS ORDER BY CLIENT_ID"
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load clients:\n" + model->lastError().text());
        return;
    }
    ui_client->tableView->setModel(model);
    ui_client->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_client->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_client->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onClientAdd()
{
    QString id      = ui_client->le_id->text().trimmed();
    QString nom     = ui_client->le_nom->text().trimmed();
    QString prenom  = ui_client->le_prenom->text().trimmed();
    QString adresse = ui_client->le_adresse->text().trimmed();
    QString tel     = ui_client->le_tel->text().trimmed();
    QString email   = ui_client->le_email->text().trimmed();
    QString gender  = ui_client->rb_homme->isChecked() ? "Male" : "Female";

    if (id.isEmpty() || nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Validation", "ID, Last Name, and First Name are required.");
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

    bool idOk;
    int clientId = id.toInt(&idOk);
    if (!idOk || clientId <= 0) {
        QMessageBox::warning(this, "Validation", "Client ID must be a positive number.");
        return;
    }

    // Check for duplicates
    QSqlQuery chk;
    chk.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = :id");
    chk.bindValue(":id", clientId);
    if (chk.exec() && chk.next() && chk.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Duplicate", "A client with this ID already exists.");
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
    QString clientId = model->data(model->index(idx.row(), 2)).toString();
    QString name     = model->data(model->index(idx.row(), 3)).toString()
                     + " " + model->data(model->index(idx.row(), 4)).toString();

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
            "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS ORDER BY CLIENT_ID"
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", CLIENT_ID AS \"ID\", "
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
    ui_client->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_client->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_client->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onClientRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_client->tableView->model());
    if (!model) return;
    int row = index.row();
    // Col order: Action, Delete, ID, Last Name, First Name, Address, Phone, Email, Gender
    ui_client->le_id_mod->setText(model->data(model->index(row, 2)).toString());
    ui_client->le_nom_mod->setText(model->data(model->index(row, 3)).toString());
    ui_client->le_prenom_mod->setText(model->data(model->index(row, 4)).toString());
    ui_client->le_adresse_mod->setText(model->data(model->index(row, 5)).toString());
    ui_client->le_tel_mod->setText(model->data(model->index(row, 6)).toString());
    ui_client->le_email_mod->setText(model->data(model->index(row, 7)).toString());
    QString gender = model->data(model->index(row, 8)).toString();
    if (gender == "Male") {
        ui_client->rb_homme_mod->setChecked(true);
    } else if (gender == "Female") {
        ui_client->rb_femme_mod->setChecked(true);
    }

    // Switch to modify tab
    ui_client->tabWidget->setCurrentWidget(ui_client->tab_modify);
}

void MainWindow::onEmployeeRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
    if (!model) return;
    int row = index.row();
    // Col order: Action, Delete, ID, Last Name, First Name, Job Title, Age, Email, Phone
    ui_employee->le_id->setText(model->data(model->index(row, 2)).toString());
    ui_employee->le_nom->setText(model->data(model->index(row, 3)).toString());
    ui_employee->le_prenom->setText(model->data(model->index(row, 4)).toString());
    ui_employee->le_fonction->setText(model->data(model->index(row, 5)).toString());
    int age = model->data(model->index(row, 6)).toInt();
    ui_employee->de_birthdate->setDate(QDate::currentDate().addYears(-age));
    ui_employee->le_email->setText(model->data(model->index(row, 7)).toString());
    ui_employee->le_num->setText(model->data(model->index(row, 8)).toString());

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
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\""
        " FROM EMPLOYEES ORDER BY EMPLOYEE_ID"
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load employees:\n" + model->lastError().text());
        return;
    }
    ui_employee->tableView_employes->setModel(model);
    ui_employee->tableView_employes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_employee->tableView_employes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_employee->tableView_employes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui_employee->tableView_employes->setEditTriggers(QAbstractItemView::NoEditTriggers);

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
        }
    }
}

void MainWindow::onEmployeeSearch()
{
    QString search = ui_employee->le_recherche_emp->text().trimmed();
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (search.isEmpty()) {
        model->setQuery(
            "SELECT EMPLOYEE_ID AS \"ID\", LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\""
            " FROM EMPLOYEES ORDER BY EMPLOYEE_ID"
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EMPLOYEE_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " JOB_TITLE AS \"Job Title\", AGE AS \"Age\", EMAIL AS \"Email\", PHONE_NUMBER AS \"Phone\""
            " FROM EMPLOYEES WHERE UPPER(LAST_NAME) LIKE :s OR UPPER(FIRST_NAME) LIKE :s"
            " OR UPPER(EMAIL) LIKE :s OR UPPER(JOB_TITLE) LIKE :s OR CAST(EMPLOYEE_ID AS VARCHAR2(20)) LIKE :s"
            " ORDER BY EMPLOYEE_ID"
        );
        q.bindValue(":s", "%" + search.toUpper() + "%");
        q.exec();
        model->setQuery(std::move(q));
    }
    ui_employee->tableView_employes->setModel(model);
}

void MainWindow::onEmployeeRefreshHistory()
{
    // Populate module filter combo if empty (except first item)
    if (ui_employee->cb_history_filter->count() <= 1) {
        QSignalBlocker blocker(ui_employee->cb_history_filter);
        ui_employee->cb_history_filter->clear();
        ui_employee->cb_history_filter->addItem("All Modules");
        ui_employee->cb_history_filter->addItem("Employees");
        ui_employee->cb_history_filter->addItem("Clients");
        ui_employee->cb_history_filter->addItem("Equipment");
        ui_employee->cb_history_filter->addItem("Orders");
        ui_employee->cb_history_filter->addItem("General");
    }

    QString searchText = ui_employee->le_history_search->text().trimmed().toUpper();
    QString moduleFilter = ui_employee->cb_history_filter->currentText();
    
    QString queryStr = "SELECT TO_CHAR(LOG_DATE, 'DD/MM/YYYY HH24:MI') AS \"Time\", "
                       "EMPLOYEE_NAME AS \"Employee\", "
                       "ACTION_DETAILS AS \"Action\", "
                       "MODULE_NAME AS \"Module\" "
                       "FROM APP_HISTORY WHERE 1=1";
    
    if (!searchText.isEmpty()) {
        queryStr += QString(" AND (UPPER(ACTION_DETAILS) LIKE '%%1%' OR UPPER(EMPLOYEE_NAME) LIKE '%%1%')").arg(searchText);
    }
    
    if (moduleFilter != "All Modules" && !moduleFilter.isEmpty()) {
        queryStr += QString(" AND MODULE_NAME = '%1'").arg(moduleFilter);
    }
    
    queryStr += " AND ROWNUM <= 500 ORDER BY LOG_DATE DESC";

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(queryStr);
    
    ui_employee->tableView_historique_emp->setModel(model);
    ui_employee->tableView_historique_emp->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_employee->tableView_historique_emp->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_employee->tableView_historique_emp->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Customize look (cool colors via stylesheet on the view)
    ui_employee->tableView_historique_emp->setStyleSheet(
        "QTableView { background-color: #ffffff; gridline-color: #e0e0e0; border: none; font-size: 13px; }"
        "QHeaderView::section { background-color: #8B6F47; color: white; border: 1px solid #705a39; padding: 5px; font-weight: bold; }"
        "QTableView::item { padding: 10px; border-bottom: 1px solid #f0f0f0; }"
        "QTableView::item:selected { background-color: rgba(139, 111, 71, 0.1); color: #8B6F47; }"
    );

    if (qobject_cast<QPushButton*>(sender()) == ui_employee->btn_refresh_history) {
        QMessageBox::information(this, "Refresh", "Timeline updated!");
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

void MainWindow::onEmployeeClearFields()
{
    ui_employee->le_id->clear();
    ui_employee->le_nom->clear();
    ui_employee->le_prenom->clear();
    ui_employee->le_fonction->clear();
    ui_employee->de_birthdate->setDate(QDate(1995, 1, 1));
    ui_employee->le_mdp->clear();
    ui_employee->dsb_salaire->setValue(0.0);
    ui_employee->le_email->clear();
    ui_employee->le_num->clear();
}

void MainWindow::onAIPulseClicked()
{
    ui_employee->lbl_ai_pulse_result->setVisible(true);
    ui_employee->lbl_ai_pulse_result->setText("📡 Scanning Global Database... Please wait.");
    
    QSettings settings("HammerDown", "HammerDown");
    QString apiKey = settings.value("api/gemini_key").toString();
    if(apiKey.isEmpty()) {
        ui_employee->lbl_ai_pulse_result->setText("⚠️ Error: Gemini API key not configured in settings. Cannot generate AI Pulse.");
        return;
    }
    
    QSqlQuery qEmp("SELECT COUNT(*) FROM EMPLOYEES"); qEmp.next(); int cEmp = qEmp.value(0).toInt();
    QSqlQuery qCli("SELECT COUNT(*) FROM CLIENTS"); qCli.next(); int cCli = qCli.value(0).toInt();
    QSqlQuery qOrd("SELECT COUNT(*) FROM ORDERS"); qOrd.next(); int cOrd = qOrd.value(0).toInt();
    QSqlQuery qEq("SELECT COUNT(*) FROM EQUIPMENT"); qEq.next(); int cEq = qEq.value(0).toInt();
    
    QString summary = QString("Analyze this company's overall health and workload balance based on these total numbers: "
                              "%1 Employees, %2 Clients, %3 Active Orders, and %4 Equipment pieces. "
                              "Are we understaffed, well-balanced, or overstaffed? "
                              "Provide a highly dynamic, futuristic 2-sentence 'Business Pulse' insight.").arg(cEmp).arg(cCli).arg(cOrd).arg(cEq);

    QJsonObject requestBody;
    QJsonArray contentsArray;
    QJsonObject contentObject;
    QJsonArray partsArray;
    QJsonObject partObject;
    partObject["text"] = summary;
    partsArray.append(partObject);
    contentObject["parts"] = partsArray;
    contentsArray.append(contentObject);
    requestBody["contents"] = contentsArray;

    QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = chatSummaryNetManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray c = doc.object()["candidates"].toArray();
            if(!c.isEmpty()){
                QString text = c[0].toObject()["content"].toObject()["parts"].toArray()[0].toObject()["text"].toString();
                ui_employee->lbl_ai_pulse_result->setText("⚡ SYSTEM PULSE: \n" + text.trimmed());
            } else {
                ui_employee->lbl_ai_pulse_result->setText("⚡ SYSTEM PULSE: Systems are nominal and dynamically balanced.");
            }
        } else {
            ui_employee->lbl_ai_pulse_result->setText("⚠️ AI Error: Could not reach the pulse service.");
        }
    });
}

void MainWindow::onStatsAiClicked()
{
    ui_employee->lbl_stats_ai_insight->setText("✨ Analyzing workforce data via AI... Please wait.");
    
    QSettings settings("HammerDown", "HammerDown");
    QString apiKey = settings.value("api/gemini_key").toString();
    if(apiKey.isEmpty()) {
        ui_employee->lbl_stats_ai_insight->setText("⚠️ Error: Gemini API key not configured in settings. Cannot generate AI insight.");
        return;
    }
    
    QSqlQuery q("SELECT COUNT(*), AVG(AGE), AVG(SALARY) FROM EMPLOYEES");
    q.next();
    int count = q.value(0).toInt();
    double avgAge = q.value(1).toDouble();
    double avgSalary = q.value(2).toDouble();
    
    QString summary = QString("Workforce: %1 employees. Avg Age: %2. Avg Salary: $%3. "
                              "Write a 2-sentence professional insight on this demographic.").arg(count).arg(avgAge, 0, 'f', 1).arg(avgSalary, 0, 'f', 0);

    QJsonObject requestBody;
    QJsonArray contentsArray;
    QJsonObject contentObject;
    QJsonArray partsArray;
    QJsonObject partObject;
    partObject["text"] = summary;
    partsArray.append(partObject);
    contentObject["parts"] = partsArray;
    contentsArray.append(contentObject);
    requestBody["contents"] = contentsArray;

    QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = chatSummaryNetManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray c = doc.object()["candidates"].toArray();
            if(!c.isEmpty()){
                QString text = c[0].toObject()["content"].toObject()["parts"].toArray()[0].toObject()["text"].toString();
                ui_employee->lbl_stats_ai_insight->setText("✨ AI Insight: " + text.trimmed());
            } else {
                ui_employee->lbl_stats_ai_insight->setText("✨ AI Insight: Workforce data is stable and indicates a healthy demographic spread.");
            }
        } else {
            ui_employee->lbl_stats_ai_insight->setText("⚠️ AI Error: Could not reach the service.");
        }
    });
}

void MainWindow::setupEmployeeStats()
{
    // Clear existing charts if any
    QLayoutItem *item;
    while ((item = ui_employee->gridLayout_stats->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    // --- CHART 1: Pie Chart (Staff Distribution by Role) ---
    QPieSeries *series = new QPieSeries();
    QSqlQuery q("SELECT JOB_TITLE, COUNT(*) FROM EMPLOYEES GROUP BY JOB_TITLE");
    double totalEmployees = 0;
    struct StatData { QString label; int count; };
    QList<StatData> dataList;
    
    while (q.next()) {
        QString label = q.value(0).toString();
        int count = q.value(1).toInt();
        dataList.append({label, count});
        totalEmployees += count;
    }

    for (const auto& d : dataList) {
        double percentage = (totalEmployees > 0) ? (d.count * 100.0 / totalEmployees) : 0;
        QString labelText = QString("%1 (%2%)").arg(d.label).arg(percentage, 0, 'f', 1);
        QPieSlice *slice = series->append(labelText, d.count);
        slice->setLabelVisible(true);
        // Use a nice color palette
        slice->setBorderWidth(2);
        slice->setBorderColor(QColor(139, 111, 71, 100));
    }

    QChart *chartPie = new QChart();
    chartPie->addSeries(series);
    chartPie->setTitle("Employee Distribution by Title");
    chartPie->setTheme(QChart::ChartThemeDark);
    chartPie->setBackgroundBrush(QBrush(QColor(30, 20, 10))); // Matching app theme
    chartPie->setAnimationOptions(QChart::AllAnimations);
    chartPie->legend()->setAlignment(Qt::AlignRight);
    chartPie->legend()->setFont(QFont("Segoe UI", 9));

    QChartView *viewPie = new QChartView(chartPie);
    viewPie->setRenderHint(QPainter::Antialiasing);
    viewPie->setStyleSheet("background: transparent; border: 1px solid #8B6F47; border-radius: 12px;");

    // --- CHART 2: Bar Chart (Age Distribution) ---
    QBarSet *set = new QBarSet("Employees Count");
    set->setColor(QColor("#B8925A"));
    set->setBorderColor(QColor("#D4AF37"));
    
    QStringList categories;
    QSqlQuery q2("SELECT CASE "
                 "WHEN AGE < 25 THEN '18-24' "
                 "WHEN AGE < 35 THEN '25-34' "
                 "WHEN AGE < 45 THEN '35-44' "
                 "ELSE '45+' END as age_group, COUNT(*) "
                 "FROM EMPLOYEES GROUP BY CASE "
                 "WHEN AGE < 25 THEN '18-24' "
                 "WHEN AGE < 35 THEN '25-34' "
                 "WHEN AGE < 45 THEN '35-44' "
                 "ELSE '45+' END ORDER BY age_group");
    
    while(q2.next()) {
        categories << q2.value(0).toString();
        *set << q2.value(1).toInt();
    }

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(set);
    barSeries->setLabelsVisible(true);
    barSeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    QChart *chartBar = new QChart();
    chartBar->addSeries(barSeries);
    chartBar->setTitle("Employee Age Groups");
    chartBar->setTheme(QChart::ChartThemeDark);
    chartBar->setBackgroundBrush(QBrush(QColor(30, 20, 10)));
    chartBar->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    chartBar->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    QChartView *viewBar = new QChartView(chartBar);
    viewBar->setRenderHint(QPainter::Antialiasing);
    viewBar->setStyleSheet("background: transparent; border: 1px solid #8B6F47; border-radius: 12px;");

    // --- CHART 3: Horizontal Bar Chart (Avg Salary by Role) ---
    QBarSet *salarySet = new QBarSet("Avg Salary ($)");
    salarySet->setColor(QColor("#4CAF50")); // Green for money
    
    QStringList salaryCategories;
    QSqlQuery q3("SELECT JOB_TITLE, AVG(SALARY) FROM EMPLOYEES GROUP BY JOB_TITLE ORDER BY AVG(SALARY) DESC");
    while(q3.next()) {
        salaryCategories << q3.value(0).toString();
        *salarySet << q3.value(1).toDouble();
    }

    QHorizontalBarSeries *salarySeries = new QHorizontalBarSeries();
    salarySeries->append(salarySet);
    salarySeries->setLabelsVisible(true);

    QChart *chartSalary = new QChart();
    chartSalary->addSeries(salarySeries);
    chartSalary->setTitle("Market Salary Benchmarks by Role");
    chartSalary->setTheme(QChart::ChartThemeDark);
    chartSalary->setBackgroundBrush(QBrush(QColor(30, 20, 10)));
    chartSalary->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisYRole = new QBarCategoryAxis();
    axisYRole->append(salaryCategories);
    chartSalary->addAxis(axisYRole, Qt::AlignLeft);
    salarySeries->attachAxis(axisYRole);

    QValueAxis *axisXSalary = new QValueAxis();
    axisXSalary->setLabelFormat("$%d");
    chartSalary->addAxis(axisXSalary, Qt::AlignBottom);
    salarySeries->attachAxis(axisXSalary);

    QChartView *viewSalary = new QChartView(chartSalary);
    viewSalary->setRenderHint(QPainter::Antialiasing);
    viewSalary->setStyleSheet("background: transparent; border: 1px solid #8B6F47; border-radius: 12px;");

    ui_employee->gridLayout_stats->setSpacing(15);
    ui_employee->gridLayout_stats->addWidget(viewPie, 0, 0);
    ui_employee->gridLayout_stats->addWidget(viewBar, 0, 1);
    ui_employee->gridLayout_stats->addWidget(viewSalary, 1, 0, 1, 2); // Span both columns
}

void MainWindow::onEmployeeAdd()
{
    QString id      = ui_employee->le_id->text().trimmed();
    QString nom     = ui_employee->le_nom->text().trimmed();
    QString prenom  = ui_employee->le_prenom->text().trimmed();
    QString fonction= ui_employee->le_fonction->text().trimmed();
    QDate   birthDate = ui_employee->de_birthdate->date();
    int     age       = birthDate.daysTo(QDate::currentDate()) / 365;
    QString mdp     = ui_employee->le_mdp->text().trimmed();
    double  salaire = ui_employee->dsb_salaire->value();
    QString email   = ui_employee->le_email->text().trimmed();
    QString num     = ui_employee->le_num->text().trimmed();

    if (id.isEmpty() || nom.isEmpty() || prenom.isEmpty() || mdp.isEmpty()) {
        QMessageBox::warning(this, "Validation", "ID, Last Name, First Name, and Password are required.");
        return;
    }
    bool idOk;
    int empId = id.toInt(&idOk);
    if (!idOk || empId <= 0) {
        QMessageBox::warning(this, "Validation", "Employee ID must be a positive number.");
        return;
    }

    QSqlQuery chk;
    chk.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    chk.bindValue(":id", empId);
    if (chk.exec() && chk.next() && chk.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Duplicate ID", "An employee with this ID already exists.");
        return;
    }

    // Proactive check for duplicate email
    if (!email.isEmpty()) {
        QSqlQuery chkEmail;
        chkEmail.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMAIL = :email");
        chkEmail.bindValue(":email", email);
        if (chkEmail.exec() && chkEmail.next() && chkEmail.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Duplicate Email", 
                "An employee with the email '" + email + "' already exists.\n"
                "Please use a unique email address.");
            return;
        }
    }

    QSqlQuery q;
    q.prepare("INSERT INTO EMPLOYEES (EMPLOYEE_ID, LAST_NAME, FIRST_NAME, JOB_TITLE, AGE, PASSWORD, SALARY, EMAIL, PHONE_NUMBER, HIRE_DATE, EMPLOYEE_STATUS)"
              " VALUES (:id, :nom, :prenom, :fonction, :age, :mdp, :salaire, :email, :num, SYSDATE, 'Active')");
    q.bindValue(":id",       empId);
    q.bindValue(":nom",      nom);
    q.bindValue(":prenom",   prenom);
    q.bindValue(":fonction", fonction);
    q.bindValue(":age",      age);
    q.bindValue(":mdp",      mdp);
    q.bindValue(":salaire",  salaire);
    q.bindValue(":email",    email);
    q.bindValue(":num",      num);

    if (q.exec()) {
        QSqlDatabase::database().commit();
        QMessageBox::information(this, "Success", "Employee added successfully.");
        logActivity("Added new employee: " + prenom + " " + nom + " (ID: " + id + ")", "Employees");
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
    QString fonction = ui_employee->le_fonction->text().trimmed();
    QDate   birthDate = ui_employee->de_birthdate->date();
    int     age       = birthDate.daysTo(QDate::currentDate()) / 365;
    QString mdp      = ui_employee->le_mdp->text().trimmed();
    double  salaire  = ui_employee->dsb_salaire->value();
    QString email    = ui_employee->le_email->text().trimmed();
    QString num      = ui_employee->le_num->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Please enter the Employee ID to modify.");
        return;
    }

    // Proactive check for duplicate email (excluding current employee)
    if (!email.isEmpty()) {
        QSqlQuery chkEmail;
        chkEmail.prepare("SELECT COUNT(*) FROM EMPLOYEES WHERE EMAIL = :email AND EMPLOYEE_ID <> :id");
        chkEmail.bindValue(":email", email);
        chkEmail.bindValue(":id", id.toInt());
        if (chkEmail.exec() && chkEmail.next() && chkEmail.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Duplicate Email",
                "The email '" + email + "' is already assigned to another employee.\n"
                "Please use a unique email address.");
            return;
        }
    }

    QSqlQuery q;
    q.prepare("UPDATE EMPLOYEES SET LAST_NAME=:nom, FIRST_NAME=:prenom, JOB_TITLE=:fonction,"
              " AGE=:age, PASSWORD=:mdp, SALARY=:salaire, EMAIL=:email, PHONE_NUMBER=:num"
              " WHERE EMPLOYEE_ID=:id");
    q.bindValue(":id",       id.toInt());
    q.bindValue(":nom",      nom);
    q.bindValue(":prenom",   prenom);
    q.bindValue(":fonction", fonction);
    q.bindValue(":age",      age);
    q.bindValue(":mdp",      mdp);
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
    QModelIndex idx = ui_employee->tableView_employes->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an employee from the table to delete.");
        return;
    }
    QSqlQueryModel *m = qobject_cast<QSqlQueryModel*>(ui_employee->tableView_employes->model());
    if (!m) return;
    QString empId = m->data(m->index(idx.row(), 2)).toString();
    QString name  = m->data(m->index(idx.row(), 3)).toString() + " " + m->data(m->index(idx.row(), 4)).toString();
    int ret = QMessageBox::question(this, "Confirm Delete", "Delete employee: " + name + "?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        q.bindValue(":id", empId.toInt());
        if (q.exec()) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Deleted", "Employee deleted.");
            logActivity("Deleted employee: " + name + " (ID: " + empId + ")", "Employees");
            onEmployeeRefreshView();
            onEmployeeRefreshHistory();
        } else {
            QMessageBox::critical(this, "Error", q.lastError().text());
        }
    }
}

// =============================================================================
// EQUIPMENT MANAGEMENT CRUD
// =============================================================================

void MainWindow::onEquipmentRefreshView()
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EQUIPMENT_ID AS \"ID\", "
        "EQUIPMENT_TYPE AS \"Type\", QUANTITY AS \"Qty\", UNIT_PRICE AS \"Price\", "
        "STATUS AS \"Status\", PURCHASE_DATE AS \"Purchase Date\", DESCRIPTION AS \"Description\""
        " FROM EQUIPMENT WHERE STATUS != 'Retired' ORDER BY EQUIPMENT_ID"
    );
    ui_equipment->table_equipments->setModel(model);
    ui_equipment->table_equipments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_equipment->table_equipments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_equipment->table_equipments->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::onEquipmentAdd()
{
    QString type = ui_equipment->le_type->text().trimmed();
    QString desc = ui_equipment->te_desc->toPlainText().trimmed();
    QString etat = ui_equipment->cb_status->currentText();
    QDate   date = ui_equipment->de_date_achat->date();
    int     qty  = ui_equipment->sb_quantity->value();
    QString loc  = "";
    QString resp = "";
    double  price= ui_equipment->dsb_unit_price->value();

    if (type.isEmpty() || desc.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Type and Description are required.");
        return;
    }

    // Insert new equipment (ID is auto-generated by the database)
    QSqlQuery q;
    q.prepare("INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, PURCHASE_DATE, EMPLOYEE_ID, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE)"
              " VALUES ((SELECT NVL(MAX(EQUIPMENT_ID), 0) + 1 FROM EQUIPMENT), :type, :desc, :status, TO_DATE(:date,'YYYY-MM-DD'), :empid, :price, :qty, :loc, :resp)");
    q.bindValue(":type",   type);
    q.bindValue(":desc",   desc);
    q.bindValue(":status", etat);
    q.bindValue(":date",   date.toString("yyyy-MM-dd"));
    q.bindValue(":empid",  currentEmployeeId);
    q.bindValue(":price",  price);
    q.bindValue(":qty",    qty);
    q.bindValue(":loc",    loc);
    q.bindValue(":resp",   resp);

    if (!q.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to add equipment:\n" + q.lastError().text());
        return;
    }

    // Get the newly generated ID for logging purposes
    int eqId = 0;
    QSqlQuery idQ("SELECT MAX(EQUIPMENT_ID) FROM EQUIPMENT");
    if (idQ.exec() && idQ.next()) {
        eqId = idQ.value(0).toInt();
    }

    // Success animation
    playEquipSuccessAnimation(type);
    logActivity("Added new equipment: " + type + " (ID: " + QString::number(eqId) + ")", "Equipment");
    
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    updateEquipProgress();
    // Refresh NEXUS analysis
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onEquipmentModify()
{
    QString id   = ui_equipment->le_id->text().trimmed();
    QString type = ui_equipment->le_type->text().trimmed();
    QString desc = ui_equipment->te_desc->toPlainText().trimmed();
    QString etat = ui_equipment->cb_status->currentText();
    double  price= ui_equipment->dsb_unit_price->value();

    int     qty  = ui_equipment->sb_quantity->value();
    QString loc  = "";
    QString resp = "";

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Please enter the Equipment ID to modify.");
        return;
    }

    // Ensure DB is open before running queries
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(this, "Database Error",
                              "Not connected to the database.\nPlease check your connection and try again.");
        return;
    }

    QSqlQuery q;
    q.prepare("UPDATE EQUIPMENT SET EQUIPMENT_TYPE=:type, DESCRIPTION=:desc, STATUS=:status, UNIT_PRICE=:price, "
              "QUANTITY=:qty, LOCATION=:loc, RESPONSABLE=:resp, NEXT_MAINTENANCE=SYSDATE WHERE EQUIPMENT_ID=:id");
    q.bindValue(":id",     id.toInt());
    q.bindValue(":type",   type.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : type);
    q.bindValue(":desc",   desc);
    q.bindValue(":status", etat);
    q.bindValue(":price",  price);
    q.bindValue(":qty",    qty);
    q.bindValue(":loc",    loc);
    q.bindValue(":resp",   resp);

    if (!q.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to update equipment:\n" + q.lastError().text());
        return;
    }

    QMessageBox::information(this, "Success", "Equipment updated successfully.");
    logActivity("Modified equipment: " + type + " (ID: " + id + ")", "Equipment");
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onEquipmentDelete()
{
    QModelIndex idx = ui_equipment->table_equipments->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select equipment from the list to delete.");
        return;
    }
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_equipment->table_equipments->model());
    if (!model) return;
    QString eqId = model->data(model->index(idx.row(), 2)).toString();
    QString type = model->data(model->index(idx.row(), 3)).toString(); 
    QString desc = model->data(model->index(idx.row(), 8)).toString(); // Description is at index 8

    int ret = QMessageBox::question(this, "Confirm Delete",
        "Delete equipment: " + type + " - " + desc + " (ID: " + eqId + ")?",
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    // Soft Delete: Mark as 'Retired'
    QSqlQuery q;
    q.prepare("UPDATE EQUIPMENT SET STATUS = 'Retired' WHERE EQUIPMENT_ID = :id");
    q.bindValue(":id", eqId.toInt());
    if (!q.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to delete equipment:\n" + q.lastError().text());
        return;
    }
    logActivity("Retired equipment: " + type + " (ID: " + eqId + ")", "Equipment");

    QMessageBox::information(this, "Deleted", "Equipment marked as Retired successfully.");
    onEquipmentClearFields();
    onEquipmentRefreshView();
    onEquipmentHistoryRefresh();
    if (m_nexusWidget) m_nexusWidget->initialize();
}

void MainWindow::onEquipmentSearch()
{
    QString search = ui_equipment->le_recherche->text().trimmed();
    QString filterStatus = ui_equipment->cb_filter_status->currentText();
    QSqlQueryModel *model = new QSqlQueryModel(this);

    QString sql = "SELECT '✎ Edit' AS \"Action\", '❌ Delete' AS \"Delete\", EQUIPMENT_ID AS \"ID\", "
                  "EQUIPMENT_TYPE AS \"Type\", QUANTITY AS \"Qty\", UNIT_PRICE AS \"Price\", "
                  "STATUS AS \"Status\", PURCHASE_DATE AS \"Purchase Date\", DESCRIPTION AS \"Description\" "
                  "FROM EQUIPMENT WHERE STATUS != 'Retired'";

    if (!search.isEmpty()) {
        sql += " AND (UPPER(DESCRIPTION) LIKE '%" + search.toUpper() + "%' "
               "OR UPPER(EQUIPMENT_TYPE) LIKE '%" + search.toUpper() + "%' "
               "OR CAST(EQUIPMENT_ID AS VARCHAR2(20)) LIKE '%" + search + "%')";
    }

    if (filterStatus != "All Statuses") {
        sql += " AND STATUS = '" + filterStatus + "'";
    }

    sql += " ORDER BY EQUIPMENT_ID";
    model->setQuery(sql);
    ui_equipment->table_equipments->setModel(model);
    ui_equipment->table_equipments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui_equipment->table_equipments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_equipment->table_equipments->setEditTriggers(QAbstractItemView::NoEditTriggers);
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





void MainWindow::onEquipmentHistoryRefresh()
{
    onEquipmentHistorySearch();
}

void MainWindow::onEquipmentHistorySearch()
{
    QString search = ui_equipment->le_history_search->text().trimmed();

    auto setupSection = [&](QTableView* view, const QString& operation) {
        QString sql;
        if (operation == "ADD") {
            sql = "SELECT EQUIPMENT_ID AS \"ID\", EQUIPMENT_TYPE AS \"Type\", DESCRIPTION AS \"Description\", "
                  "STATUS AS \"Status\", UNIT_PRICE AS \"Price\", TO_CHAR(PURCHASE_DATE, 'YYYY-MM-DD') AS \"Date\" "
                  "FROM EQUIPMENT WHERE STATUS != 'Retired'";
        } else if (operation == "MODIFY") {
            sql = "SELECT EQUIPMENT_ID AS \"ID\", EQUIPMENT_TYPE AS \"Type\", DESCRIPTION AS \"Description\", "
                  "STATUS AS \"Status\", UNIT_PRICE AS \"Price\", TO_CHAR(NEXT_MAINTENANCE, 'YYYY-MM-DD') AS \"Date\" "
                  "FROM EQUIPMENT WHERE NEXT_MAINTENANCE IS NOT NULL AND STATUS != 'Retired'";
        } else if (operation == "DELETE") {
            sql = "SELECT EQUIPMENT_ID AS \"ID\", EQUIPMENT_TYPE AS \"Type\", DESCRIPTION AS \"Description\", "
                  "STATUS AS \"Status\", UNIT_PRICE AS \"Price\", TO_CHAR(SYSDATE, 'YYYY-MM-DD') AS \"Date\" "
                  "FROM EQUIPMENT WHERE STATUS = 'Retired'";
        }
        
        if (!search.isEmpty()) {
            sql += " AND (UPPER(EQUIPMENT_TYPE) LIKE '%" + search.toUpper() + "%' "
                   " OR CAST(EQUIPMENT_ID AS VARCHAR2(20)) LIKE '%" + search + "%' "
                   " OR UPPER(DESCRIPTION) LIKE '%" + search.toUpper() + "%')";
        }
        
        if (operation == "ADD") sql += " ORDER BY PURCHASE_DATE DESC";
        else if (operation == "MODIFY") sql += " ORDER BY NEXT_MAINTENANCE DESC";
        else sql += " ORDER BY EQUIPMENT_ID DESC";

        QSqlQueryModel *model = new QSqlQueryModel(this);
        model->setQuery(sql);
        
        if (model->lastError().isValid()) {
            qDebug() << "Logical History Error (" << operation << "):" << model->lastError().text();
        }

        view->setModel(model);
        view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    };

    setupSection(ui_equipment->tableView_history_add,    "ADD");
    setupSection(ui_equipment->tableView_history_modify, "MODIFY");
    setupSection(ui_equipment->tableView_historique,     "DELETE");
}

void MainWindow::onEquipmentHistoryClear()
{
    QMessageBox::information(this, "Clear History", "History is now logically linked to the main equipment table and cannot be cleared separately.");
}

void MainWindow::onEquipmentCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = ui_equipment->table_equipments->indexAt(pos);
    if (!index.isValid()) return;

    // Get the ID (Column 2 according to onEquipmentRefreshView: Action, Delete, ID...)
    int equipId = ui_equipment->table_equipments->model()->data(ui_equipment->table_equipments->model()->index(index.row(), 2)).toInt();

    QMenu menu(this);
    QAction *analyzeAct = menu.addAction("🔍 Analyze in Nexus");
    
    QAction *selected = menu.exec(ui_equipment->table_equipments->viewport()->mapToGlobal(pos));
    if (selected == analyzeAct) {
        // Switch to NEXUS tab
        ui_equipment->tabWidget->setCurrentWidget(m_nexusWidget);
        // Highlight in graph
        m_nexusWidget->highlightEquipmentInGraph(equipId);
    }
}

void MainWindow::onEquipmentHistoryCustomContextMenu(const QPoint &pos, int tableIdx)
{
    QTableView *view = nullptr;
    if (tableIdx == 0) view = ui_equipment->tableView_history_add;
    else if (tableIdx == 1) view = ui_equipment->tableView_history_modify;
    else view = ui_equipment->tableView_historique;

    if (!view) return;
    QModelIndex index = view->indexAt(pos);
    if (!index.isValid()) return;

    // Get the Date (Last column usually)
    int dateCol = view->model()->columnCount() - 1;
    QDate date = QDate::fromString(view->model()->data(view->model()->index(index.row(), dateCol)).toString(), "YYYY-MM-DD");
    if (!date.isValid()) date = QDate::currentDate();

    QMenu menu(this);
    QAction *nexusViewAct = menu.addAction("⏳ Nexus View (Time Machine)");
    
    QAction *selected = menu.exec(view->viewport()->mapToGlobal(pos));
    if (selected == nexusViewAct) {
        ui_equipment->tabWidget->setCurrentWidget(m_nexusWidget);
        m_nexusWidget->goToTimeMachineDate(date);
    }
}

// =============================================================================
// PRESENTATION MODE (DIRECTOR'S CUT)
// =============================================================================
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

void MainWindow::updateEquipProgress() {
    if (!m_equipProgress || !ui_equipment) return;
    
    int progress = 0;
    bool typeOk  = !ui_equipment->le_type->text().trimmed().isEmpty();
    bool dateOk  = ui_equipment->de_date_achat->date().isValid();
    bool priceOk = ui_equipment->dsb_unit_price->value() > 0;
    bool descOk  = !ui_equipment->te_desc->toPlainText().trimmed().isEmpty();
    
    if (typeOk)  progress += 25;
    if (dateOk)  progress += 25;
    if (priceOk) progress += 25;
    if (descOk)  progress += 25;
    
    m_equipProgress->setValue(progress);
    
    // Change bar color to green when filled (100%)
    if (progress == 100) {
        m_equipProgress->setStyleSheet(
            "QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid #5A4A32; border-radius: 6px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #66BB6A); border-radius: 5px; }");
    } else {
        m_equipProgress->setStyleSheet(
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
    
    updateInd(m_eqTypeInd,  typeOk,  "[🔨 Type");
    updateInd(m_eqDateInd,  dateOk,  "[📅 Date");
    updateInd(m_eqPriceInd, priceOk, "[💰 Price");
    updateInd(m_eqDescInd,  descOk,  "[📝 Desc");
    
    // Pulsing animation for Add button at 100%
    if (progress == 100) {
        if (!ui_equipment->btn_add->graphicsEffect()) {
            QGraphicsDropShadowEffect *eff = new QGraphicsDropShadowEffect(this);
            eff->setBlurRadius(15);
            eff->setColor(QColor(212, 175, 55, 200));
            eff->setOffset(0);
            ui_equipment->btn_add->setGraphicsEffect(eff);
            
            QPropertyAnimation *pulse = new QPropertyAnimation(eff, "blurRadius");
            pulse->setDuration(1000);
            pulse->setStartValue(8);
            pulse->setEndValue(25);
            pulse->setLoopCount(-1);
            pulse->setEasingCurve(QEasingCurve::InOutSine);
            pulse->start(QAbstractAnimation::DeleteWhenStopped);
        }
    } else {
        ui_equipment->btn_add->setGraphicsEffect(nullptr);
    }
}

void MainWindow::playEquipSuccessAnimation(const QString &equipName) {
    // 0. Play Anvil Sound only (assets/sound/anvil.mp4/wav)
    QMediaPlayer *sfx = new QMediaPlayer(this);
    QAudioOutput *sfxOut = new QAudioOutput(this);
    sfx->setAudioOutput(sfxOut);
    sfx->setSource(QUrl::fromLocalFile(QDir::currentPath() + "/assets/sound/anvil.mp4"));
    sfxOut->setVolume(currentVolume);
    sfx->play();
    connect(sfx, &QMediaPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia) {
            sfx->deleteLater();
            sfxOut->deleteLater();
        }
    });

    // 1. Flash green
    QList<QWidget*> widgets = { ui_equipment->le_type, ui_equipment->de_date_achat, 
                               ui_equipment->dsb_unit_price, ui_equipment->te_desc };
    for (auto w : widgets) {
        QString oldStyle = w->styleSheet();
        w->setStyleSheet(oldStyle + " background-color: rgba(76, 175, 80, 0.3); border: 2px solid #4CAF50;");
        QTimer::singleShot(800, [=]() { w->setStyleSheet(oldStyle); });
    }

    // 2. Flying Card
    QLabel *flyer = new QLabel(this);
    flyer->setText("🛠️ " + equipName);
    flyer->setFixedSize(160, 45);
    flyer->setAlignment(Qt::AlignCenter);
    flyer->setStyleSheet("background: #8B6F47; color: white; border: 2px solid #D4AF37; border-radius: 12px; font-weight: bold; font-family: 'Segoe UI';");
    
    QPoint startPos = ui_equipment->groupBox_gestion->mapTo(this, QPoint(150, 200));
    // Approximate position of "View" nav radio button
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
        
        // 4. Confetti (Wood chips)
        for (int i=0; i<15; ++i) {
            QLabel *chip = new QLabel(this);
            chip->setFixedSize(8, 8);
            chip->setStyleSheet(QString("background: %1; border-radius: 3px; border: 1px solid rgba(0,0,0,0.2);")
                                .arg(i%2==0 ? "#8B6F47" : "#D3C1A5"));
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
    QLabel *toast = new QLabel("✅ " + equipName + " added to workshop!", this);
    toast->setFixedSize(320, 55);
    toast->setAlignment(Qt::AlignCenter);
    toast->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3C2D1E, stop:1 #251B12);"
        "color: #D4AF37; border: 2.5px solid #8B6F47; border-radius: 15px; font-weight: bold; font-size: 14px;");
    
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

void MainWindow::onEquipmentExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Equipment Registry",
        QDir::homePath() + "/Equipment_Registry_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to initialize PDF printer.");
        return;
    }

    int W = painter.viewport().width();
    int y = 50;

    // Title
    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(0, y, W, 40, Qt::AlignCenter, "HammerDown - Equipment Registry");
    y += 50;

    painter.setFont(QFont("Segoe UI", 9));
    painter.setPen(QColor(100,100,100));
    painter.drawText(0, y, W, 25, Qt::AlignCenter,
        "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    y += 40;

    // Column positions (proportional)
    int cID    = 20;
    int cType  = W * 0.08;
    int cDesc  = W * 0.25;
    int cStat  = W * 0.52;
    int cPrice = W * 0.65;
    int cDate  = W * 0.77;
    int cResp  = W * 0.88;

    // Header bar
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(139, 111, 71));
    painter.drawRect(10, y - 15, W - 20, 28);

    painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
    painter.setPen(Qt::white);
    painter.drawText(cID,    y, "ID");
    painter.drawText(cType,  y, "Type");
    painter.drawText(cDesc,  y, "Description");
    painter.drawText(cStat,  y, "Status");
    painter.drawText(cPrice, y, "Unit Price");
    painter.drawText(cDate,  y, "Purchase Date");
    painter.drawText(cResp,  y, "Responsible");
    y += 30;

    // Rows
    painter.setFont(QFont("Segoe UI", 8));
    bool alt = false;
    QSqlQuery q(
        "SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, UNIT_PRICE, "
        "TO_CHAR(PURCHASE_DATE,'YYYY-MM-DD'), RESPONSABLE "
        "FROM EQUIPMENT ORDER BY EQUIPMENT_ID DESC"
    );
    while (q.next()) {
        if (y > printer.height() - 60) {
            printer.newPage();
            y = 50;
        }
        // Alternating row background
        painter.setPen(Qt::NoPen);
        painter.setBrush(alt ? QColor(245, 240, 235) : Qt::white);
        painter.drawRect(10, y - 14, W - 20, 22);
        alt = !alt;

        painter.setPen(Qt::black);
        painter.drawText(cID,    y, q.value(0).toString());
        painter.drawText(cType,  y, q.value(1).toString().left(20));
        painter.drawText(cDesc,  y, q.value(2).toString().left(30));
        painter.drawText(cStat,  y, q.value(3).toString());
        painter.drawText(cPrice, y, q.value(4).isNull() ? "-" : QString::number(q.value(4).toDouble(),'f',2));
        painter.drawText(cDate,  y, q.value(5).toString());
        painter.drawText(cResp,  y, q.value(6).toString().left(18));
        y += 24;
    }

    painter.end();
    QMessageBox::information(this, "Success",
        "Equipment registry exported successfully to:\n" + fileName);
}



void MainWindow::onEquipmentExportStatsPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Equipment Statistics",
        QDir::homePath() + "/Equipment_Stats_" + QDate::currentDate().toString("yyyyMMdd") + ".pdf",
        "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to initialize PDF printer.");
        return;
    }

    int W = painter.viewport().width();
    int H = painter.viewport().height();
    int y = 50;

    // --- Header / Branding ---
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background header accent
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(139, 111, 71)); // Brand Gold
    painter.drawRect(0, 0, W, 120);

    painter.setFont(QFont("Segoe UI", 28, QFont::Bold));
    painter.setPen(Qt::white);
    painter.drawText(40, 75, "HammerDown");
    
    painter.setFont(QFont("Segoe UI", 12));
    painter.drawText(W - 300, 75, 260, 30, Qt::AlignRight, "Equipment Analytics");
    
    y = 160;

    // Report Title & Date
    painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
    painter.setPen(QColor(50, 50, 50));
    painter.drawText(40, y, "Inventory Status Report");
    
    painter.setFont(QFont("Segoe UI", 10));
    painter.setPen(QColor(120, 120, 120));
    painter.drawText(W - 300, y, 260, 30, Qt::AlignRight, 
        "Generated: " + QDateTime::currentDateTime().toString("MMM dd, yyyy HH:mm"));
    y += 60;

    // --- Fetch Statistics ---
    int total = 0;
    int intactCount = 0;
    int brokenCount = 0;
    double totalValue = 0;

    QSqlQuery qStats("SELECT COUNT(*), "
                     "SUM(CASE WHEN STATUS = 'Available' OR STATUS = 'In Use' THEN 1 ELSE 0 END), "
                     "SUM(CASE WHEN STATUS = 'Under Maintenance' OR STATUS = 'Retired' THEN 1 ELSE 0 END), "
                     "SUM(UNIT_PRICE) "
                     "FROM EQUIPMENT");
    if (qStats.next()) {
        total = qStats.value(0).toInt();
        intactCount = qStats.value(1).toInt();
        brokenCount = qStats.value(2).toInt();
        totalValue = qStats.value(3).toDouble();
    }

    // --- Summary Cards (Top Section) ---
    int cardW = (W - 80 - 40) / 3; // 40 margin, 20 gap between cards
    int cardH = 100;
    
    auto drawCard = [&](int x, int y, const QString& label, const QString& value, const QColor& color) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(color.lighter(160));
        painter.drawRoundedRect(x, y, cardW, cardH, 8, 8);
        
        painter.setPen(color);
        painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
        painter.drawText(x + 15, y + 30, label.toUpper());
        
        painter.setFont(QFont("Segoe UI", 22, QFont::Bold));
        painter.drawText(x + 15, y + 75, value);
    };

    drawCard(40, y, "Total Assets", QString::number(total), QColor(139, 111, 71));
    drawCard(40 + cardW + 20, y, "Operational", QString::number(intactCount), QColor(46, 125, 50)); // Green
    drawCard(40 + (cardW + 20) * 2, y, "Flagged/Repair", QString::number(brokenCount), QColor(198, 40, 40)); // Red
    
    y += cardH + 40;

    // Financial Highlight
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 245, 245));
    painter.drawRoundedRect(40, y, W - 80, 50, 5, 5);
    painter.setPen(QColor(100, 100, 100));
    painter.setFont(QFont("Segoe UI", 11));
    painter.drawText(60, y + 32, "Total Inventory Valuation:");
    painter.setPen(QColor(139, 111, 71));
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.drawText(W - 250, y + 32, 200, 30, Qt::AlignRight, 
        QString::number(totalValue, 'f', 2) + " DT");
    
    y += 90;

    // --- Detailed Condition Table ---
    painter.setFont(QFont("Segoe UI", 14, QFont::Bold));
    painter.setPen(QColor(50, 50, 50));
    painter.drawText(40, y, "Condition Analysis by Type");
    y += 35;

    // Table Header Styling
    int colWidths[] = { (int)(W*0.35), (int)(W*0.15), (int)(W*0.20), (int)(W*0.20) };
    QString headers[] = { "Equipment Type", "Count", "Operational", "Service Req." };
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(139, 111, 71));
    painter.drawRect(40, y, W - 80, 40);
    
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::white);
    int currentX = 55;
    for (int i = 0; i < 4; ++i) {
        painter.drawText(currentX, y, colWidths[i], 40, Qt::AlignVCenter, headers[i]);
        currentX += colWidths[i];
    }
    y += 40;

    // Table Data
    QSqlQuery qBreakdown(
        "SELECT EQUIPMENT_TYPE, COUNT(*), "
        "SUM(CASE WHEN STATUS = 'Available' OR STATUS = 'In Use' THEN 1 ELSE 0 END), "
        "SUM(CASE WHEN STATUS = 'Under Maintenance' OR STATUS = 'Retired' THEN 1 ELSE 0 END) "
        "FROM EQUIPMENT GROUP BY EQUIPMENT_TYPE ORDER BY COUNT(*) DESC"
    );

    bool alternate = false;
    painter.setFont(QFont("Segoe UI", 10));
    
    while (qBreakdown.next()) {
        if (y > H - 100) {
            printer.newPage();
            y = 50;
            // Redraw header on new page? (Optional improvement)
        }
        
        // Row background
        painter.setPen(Qt::NoPen);
        painter.setBrush(alternate ? QColor(248, 248, 248) : Qt::white);
        painter.drawRect(40, y, W - 80, 35);
        
        painter.setPen(QColor(60, 60, 60));
        currentX = 55;
        
        // Type
        painter.drawText(currentX, y, colWidths[0], 35, Qt::AlignVCenter, qBreakdown.value(0).toString());
        currentX += colWidths[0];
        
        // Count
        painter.drawText(currentX, y, colWidths[1], 35, Qt::AlignVCenter, qBreakdown.value(1).toString());
        currentX += colWidths[1];
        
        // Operational (Green shade)
        painter.setPen(QColor(46, 125, 50));
        painter.drawText(currentX, y, colWidths[2], 35, Qt::AlignVCenter, qBreakdown.value(2).toString());
        currentX += colWidths[2];
        
        // Service (Red shade)
        painter.setPen(QColor(198, 40, 40));
        painter.drawText(currentX, y, colWidths[3], 35, Qt::AlignVCenter, qBreakdown.value(3).toString());
        
        // Thin separator line
        painter.setPen(QPen(QColor(230, 230, 230), 1));
        painter.drawLine(40, y + 35, W - 40, y + 35);
        
        y += 35;
        alternate = !alternate;
    }
    
    // --- Footer ---
    painter.setFont(QFont("Segoe UI", 8, QFont::StyleItalic));
    painter.setPen(QColor(150, 150, 150));
    painter.drawText(0, H - 40, W, 30, Qt::AlignCenter, 
        "Proprietary & Confidential - HammerDown Maintenance Management System");

    painter.end();
    QMessageBox::information(this, "Success",
        "Professional statistics report generated successfully:\n" + fileName);
}

// =============================================================================
// EMPLOYEE CHAT LOGIC
// =============================================================================

void MainWindow::onChatEnsureTable()
{
    // No longer using database tables to comply with user constraints.
    // We just ensure the path for the JSON file is accessible if needed,
    // but QFile handles creation automatically.
}

void MainWindow::onChatSendMessage()
{
    if (!ui_equipment || currentChatPartnerId == -1) {
        if (currentChatPartnerId == -1)
            QMessageBox::information(this, "Select Employee", "Please select an employee from the list to start chatting.");
        return;
    }
    QString msg = ui_equipment->le_chat_input->text().trimmed();
    bool hasImage = !pendingChatImage.isEmpty();

    if (msg.isEmpty() && !hasImage) {
        shakeWidget(ui_equipment->le_chat_input);
        return;
    }

    if (currentEmployeeId <= 0) {
        QMessageBox::warning(this, "Not Logged In", "You must be logged in to send messages.");
        return;
    }

    // Save common messages for autocomplete
    if (!msg.isEmpty() && msg.length() > 5) {
        QStringList curr = m_completerModel->stringList();
        if (!curr.contains(msg, Qt::CaseInsensitive)) {
            curr << msg;
            if (curr.size() > 50) curr.removeFirst(); // Keep fresh
            m_completerModel->setStringList(curr);
        }
    }

    // Load existing chat JSON
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray chatArray;

    if (file.open(QIODevice::ReadOnly)) {
        chatArray = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    // Create new message object
    QJsonObject msgObj;
    msgObj["sender_id"] = currentEmployeeId;
    msgObj["receiver_id"] = currentChatPartnerId;
    msgObj["message"] = msg.isEmpty() ? QString("[Image]") : msg;
    msgObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    if (hasImage) {
        msgObj["image_data"] = QString::fromLatin1(pendingChatImage.toBase64());
    }
    chatArray.append(msgObj);

    // Save back to file
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(chatArray).toJson());
        file.close();
        
        ui_equipment->le_chat_input->clear();
        pendingChatImage.clear();
        if (auto *lbl = equipmentPage->findChild<QLabel*>("lbl_img_preview"))
            lbl->setVisible(false);
        onChatRefresh();
    } else {
        QMessageBox::critical(this, "Save Error", "Could not save chat to local storage.");
    }
}

void MainWindow::onChatRefresh()
{
    if (!ui_equipment || currentChatPartnerId == -1) return;
    
    // --- Apply Global Theme to Chat Container ---
    if (m_isChatModernTheme) {
        ui_equipment->frame_chat_panel->setStyleSheet(
            "QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1A1208, stop:1 #0A0804); border-radius: 0px 16px 16px 0px; border: 1.5px solid #5A4A32; border-left: none; }"
        );
        ui_equipment->frame_chat_header->setStyleSheet(
            "QFrame#frame_chat_header { background: rgba(30, 20, 10, 0.7); border-bottom: 1px solid #8B6F47; border-radius: 0px 14px 0px 0px; }"
        );
        ui_equipment->frame_input_bar->setStyleSheet(
            "QFrame#frame_input_bar { background: rgba(30, 20, 10, 0.7); border-top: 1px solid #5A4A32; border-radius: 0 0 14px 0; }"
        );
    } else {
        ui_equipment->frame_chat_panel->setStyleSheet(
            "QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2D2416, stop:1 #1A1208); border-radius: 0px 16px 16px 0px; border: 1.5px solid #5A4A32; border-left: none; }"
        );
        ui_equipment->frame_chat_header->setStyleSheet(
            "QFrame#frame_chat_header { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3820); border-bottom: 2px solid #8B6F47; border-radius: 0px 14px 0px 0px; }"
        );
        ui_equipment->frame_input_bar->setStyleSheet(
            "QFrame#frame_input_bar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2A2010,stop:1 #1E1608); border-top: 2px solid #5A4A32; border-radius: 0 0 14px 0; }"
        );
    }

    // Load existing chat JSON
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray allMessages;

    if (file.open(QIODevice::ReadOnly)) {
        allMessages = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    // Clear existing bubble widgets
    static int s_lastRenderedCount = -1;
    static int s_lastPartnerId = -1;
    int currentMsgCount = allMessages.size();

    // Check if we even need to refresh (save CPU and prevent "messy" flickering)
    // We refresh if count changed OR if we switched chat partners
    if (currentMsgCount == s_lastRenderedCount && currentChatPartnerId == s_lastPartnerId && s_lastRenderedCount != -1) {
        return; 
    }
    s_lastRenderedCount = currentMsgCount;
    s_lastPartnerId = currentChatPartnerId;

    QLayoutItem *item;
    while ((item = ui_equipment->verticalLayout_chat_contents->takeAt(0)) != nullptr) {
        if (item->layout()) {
             QLayoutItem *sub;
             while ((sub = item->layout()->takeAt(0)) != nullptr) {
                 if (sub->widget()) sub->widget()->deleteLater();
                 delete sub;
             }
        }
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    ui_equipment->verticalLayout_chat_contents->addStretch();

    // Map to quickly get employee names
    static QMap<int, QPair<QString, QString>> employeeInfo;
    static int s_lastMessageCount = -1;
    
    // Check for notification badge
    if (s_lastMessageCount != -1 && allMessages.size() > s_lastMessageCount && ui_equipment->tabWidget->currentIndex() != 4) {
        // Someone sent a message and we're not on the chat tab
        QJsonObject lastM = allMessages.last().toObject();
        if (lastM["sender_id"].toInt() != currentEmployeeId) { // We didn't send it
            ui_equipment->tabWidget->setTabText(4, QString::fromUtf8("Chat \xF0\x9F\x94\xB4"));
        }
    }
    s_lastMessageCount = allMessages.size();
    
    // Clear badge if on chat tab
    if (ui_equipment->tabWidget->currentIndex() == 4) {
        ui_equipment->tabWidget->setTabText(4, "Chat");
    }

    if (employeeInfo.isEmpty()) { 
        QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM EMPLOYEES");
        while (q.next()) {
            employeeInfo[q.value(0).toInt()] = qMakePair(q.value(1).toString(), q.value(2).toString());
        }
    }

    // Avatar color palette for variety
    static const QStringList avatarColors = {
        "#8B6F47", "#4A7B9D", "#6B9E6B", "#9E6B6B",
        "#7B6B9E", "#9E8B6B", "#6B8B9E", "#9E7B6B"
    };

    // --- Message Loop ---
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();

        // Filter messages for current conversation
        if (!((s_id == currentEmployeeId && r_id == currentChatPartnerId) ||
              (s_id == currentChatPartnerId && r_id == currentEmployeeId))) {
            continue;
        }

        QString msg = m["message"].toString();
        QDateTime dt = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
        bool isMe = (s_id == currentEmployeeId);
        
        QPair<QString, QString> names = employeeInfo.value(s_id, qMakePair(QString("Emp"), QString::number(s_id)));
        QString firstName = names.first;
        QString lastName = names.second;

        QByteArray imgData;
        if (m.contains("image_data")) {
            imgData = QByteArray::fromBase64(m["image_data"].toString().toLatin1());
        }

        // --- Build avatar label ---
        QLabel *avatarLbl = new QLabel();
        avatarLbl->setFixedSize(36, 36);
        avatarLbl->setAlignment(Qt::AlignCenter);
        QString initials = (firstName.isEmpty() ? "?" : firstName.left(1).toUpper())
                         + (lastName.isEmpty()  ? ""  : lastName.left(1).toUpper());
        avatarLbl->setText(initials);
        
        QString avatarStyle;
        if (m_isChatModernTheme) {
            avatarStyle = QString(
                "QLabel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #D4AF37,stop:1 #8B6F47);"
                " color: white; font-size: 13px; font-weight: bold; border-radius: 18px;"
                " border: 2px solid rgba(255,255,255,0.4); box-shadow: 0 4px 8px rgba(0,0,0,0.3); }"
            );
        } else {
            QString avatarColor = avatarColors[s_id % avatarColors.size()];
            avatarStyle = QString(
                "QLabel { background: %1; color: white; font-size: 12px; font-weight: bold;"
                " border-radius: 18px; border: 1.5px solid rgba(255,255,255,0.2); }"
            ).arg(avatarColor);
        }
        avatarLbl->setStyleSheet(avatarStyle);

        // --- Build bubble content ---
        QVBoxLayout *bubbleLayout = new QVBoxLayout();
        bubbleLayout->setSpacing(m_isChatModernTheme ? 4 : 3);
        bubbleLayout->setContentsMargins(0, 0, 0, 0);

        // Sender name label
        if (!isMe) {
            QLabel *nameLabel = new QLabel(firstName + " " + lastName);
            nameLabel->setStyleSheet(m_isChatModernTheme ? 
                "color: #D4AF37; font-size: 11px; font-weight: 800; background: transparent;" :
                "color: #D4AF37; font-size: 10px; font-weight: bold; background: transparent;");
            bubbleLayout->addWidget(nameLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
        }

        // Image (if attached)
        if (!imgData.isEmpty()) {
            QImage img;
            if (img.loadFromData(imgData)) {
                QLabel *imgLabel = new QLabel();
                QPixmap pix = QPixmap::fromImage(img).scaled(320, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                imgLabel->setPixmap(pix);
                imgLabel->setStyleSheet(m_isChatModernTheme ?
                    "border-radius: 14px; border: 2px solid rgba(139,111,71,0.5); padding: 2px; background: rgba(0,0,0,0.2);" :
                    "border-radius: 10px; padding: 2px;");
                bubbleLayout->addWidget(imgLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
            }
        }

        // GIF (if sent via GIPHY)
        if (m.contains("gif_url")) {
            QString gifUrl = m["gif_url"].toString();
            QString gifTitle = m["gif_title"].toString();
            
            QLabel *gifLabel = new QLabel();
            gifLabel->setFixedSize(280, 200);
            gifLabel->setAlignment(Qt::AlignCenter);
            gifLabel->setStyleSheet(m_isChatModernTheme ?
                "border-radius: 14px; border: 2px solid rgba(139,111,71,0.5); padding: 2px;"
                " background: transparent; " :
                "border-radius: 10px; padding: 2px; background: transparent; ");
            if (!gifTitle.isEmpty())
                gifLabel->setToolTip(gifTitle);
            bubbleLayout->addWidget(gifLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
            
            // Download the GIF and display as animation using QMovie
            QNetworkAccessManager *gifNetMgr = new QNetworkAccessManager(gifLabel);
            QUrl gifDisplayUrl(gifUrl);
            QNetworkReply *gifReply = gifNetMgr->get(QNetworkRequest(gifDisplayUrl));
            connect(gifReply, &QNetworkReply::finished, gifLabel, [gifLabel, gifReply]() {
                if (gifReply->error() == QNetworkReply::NoError) {
                    QByteArray data = gifReply->readAll();
                    
                    QBuffer *buffer = new QBuffer(gifLabel);
                    buffer->setData(data);
                    buffer->open(QIODevice::ReadOnly);
                    
                    QMovie *movie = new QMovie(buffer, QByteArray(), gifLabel);
                    if (movie->isValid()) {
                        gifLabel->setMovie(movie);
                        gifLabel->setText("");
                        movie->setScaledSize(QSize(276, 196));
                        movie->start();
                        gifLabel->show(); // Explicitly show label
                    } else {
                        gifLabel->setText("Invalid GIF");
                        delete movie;
                        delete buffer;
                    }
                } else {
                    gifLabel->setText("Could not load GIF");
                }
                gifReply->deleteLater();
            });
        }

        // Text bubble
        if (!msg.isEmpty() && msg != "[Image]" && msg != "[GIF]") {
            QWidget *bubbleContainer = new QWidget();
            QVBoxLayout *bubbleV = new QVBoxLayout(bubbleContainer);
            bubbleV->setContentsMargins(0, 0, 0, 0);
            bubbleV->setSpacing(4);

            if (msg.contains(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Note"))) {
                VoiceWaveformWidget *waveform = new VoiceWaveformWidget(bubbleContainer);
                bubbleV->addWidget(waveform);
                waveform->startAnim();
                
                QLabel *voiceLabel = new QLabel(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Message - 0:08"));
                voiceLabel->setStyleSheet(m_isChatModernTheme ? "color: #D4AF37; font-weight: 800; font-size: 13px;" : "color: #8B6F47; font-weight: bold;");
                bubbleV->addWidget(voiceLabel);
            } else {
                QLabel *bubble = new QLabel(msg);
                bubble->setWordWrap(true);
                bubble->setMaximumWidth(520);
                bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
                
                if (m_isChatModernTheme) {
                    if (isMe) {
                        bubble->setStyleSheet(
                            "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #D4AF37,stop:1 #A0825A);"
                            " color: #1A1208; border-radius: 18px; border-bottom-right-radius: 4px;"
                            " padding: 10px 16px; font-size: 14px; font-weight: 600;"
                            " border: 1px solid rgba(255,255,255,0.3); ");
                    } else {
                        bubble->setStyleSheet(
                            "background: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47;"
                            " color: #F0E0C0; border-radius: 18px; border-bottom-left-radius: 4px;"
                            " padding: 10px 16px; font-size: 14px; ");
                    }
                } else {
                    if (isMe) {
                        bubble->setStyleSheet(
                            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #C4973A,stop:1 #D4AF37);"
                            " color: #1A1000; border-radius: 14px; border-bottom-right-radius: 3px;"
                            " padding: 9px 13px; font-size: 13px; font-weight: 500;");
                    } else {
                        bubble->setStyleSheet(
                            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3A22);"
                            " color: #EDD9B0; border-radius: 14px; border-bottom-left-radius: 3px;"
                            " padding: 9px 13px; font-size: 13px;");
                    }
                }
                bubbleV->addWidget(bubble);
            }
            bubbleLayout->addWidget(bubbleContainer, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);
        }

        // --- Delete Button (Trash Icon) ---
        if (isMe) {
            QPushButton *delBtn = new QPushButton("🗑"); 
            delBtn->setFixedSize(24, 24);
            delBtn->setCursor(Qt::PointingHandCursor);
            delBtn->setStyleSheet(m_isChatModernTheme ?
                "QPushButton { background: rgba(200,50,50,0.1); border: 1px solid rgba(200,50,50,0.3); color: #FF7070; font-size: 14px; border-radius: 12px; }"
                "QPushButton:hover { background: #CC3333; color: white; border-color: white; }" :
                "QPushButton { background: rgba(255,0,0,0.1); border: none; color: #cc4444; font-size: 14px; border-radius: 11px; }"
                "QPushButton:hover { background: #aa3333; color: white; }");
            delBtn->setToolTip("Delete message");
            connect(delBtn, &QPushButton::clicked, this, [this, i]() { onChatDeleteMessage(i); });
            bubbleLayout->addWidget(delBtn, 0, Qt::AlignRight);
        }

        // Nicely formatted timestamp
        QDateTime now = QDateTime::currentDateTime();
        QString timeStr;
        if (dt.date() == now.date()) {
            timeStr = "Today at " + dt.toString("HH:mm");
        } else if (dt.date() == now.date().addDays(-1)) {
            timeStr = "Yesterday at " + dt.toString("HH:mm");
        } else {
            timeStr = dt.toString("dd/MM/yyyy") + " at " + dt.toString("HH:mm");
        }

        QLabel *infoLabel = new QLabel(timeStr);
        infoLabel->setStyleSheet(m_isChatModernTheme ?
            "color: rgba(212,175,55,0.8); font-size: 10px; font-weight: bold; background: transparent;" :
            "color: rgba(180,160,120,0.7); font-size: 10px; background: transparent;");
        bubbleLayout->addWidget(infoLabel, 0, isMe ? Qt::AlignRight : Qt::AlignLeft);

        // --- Assemble row ---
        QWidget *row = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(8, 4, 8, 4);
        rowLayout->setSpacing(10);

        if (isMe) {
            rowLayout->addStretch();
            rowLayout->addLayout(bubbleLayout);
            rowLayout->addWidget(avatarLbl, 0, Qt::AlignBottom);
        } else {
            rowLayout->addWidget(avatarLbl, 0, Qt::AlignBottom);
            rowLayout->addLayout(bubbleLayout);
            rowLayout->addStretch();
        }

        ui_equipment->verticalLayout_chat_contents->addWidget(row);

        // --- Modern Staggered Animation ---
        if (m_isChatModernTheme) {
            QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(row);
            row->setGraphicsEffect(opacity);
            
            QPropertyAnimation *anim = new QPropertyAnimation(opacity, "opacity");
            anim->setDuration(400);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            
            QPropertyAnimation *posAnim = new QPropertyAnimation(row, "pos");
            posAnim->setDuration(500);
            posAnim->setStartValue(QPoint(row->pos().x(), row->pos().y() + 30));
            posAnim->setEndValue(row->pos());
            posAnim->setEasingCurve(QEasingCurve::OutBack);

            // Use a relative index for animations so that older messages aren't delayed indefinitely
            // We'll use the last 20 messages or so for animations, or just limit the max delay.
            int animIdx = qMax(0, i - (allMessages.size() - 15)); 
            QTimer::singleShot(animIdx * 60, [anim, posAnim](){
                anim->start(QAbstractAnimation::DeleteWhenStopped);
                posAnim->start(QAbstractAnimation::DeleteWhenStopped);
            });
        } else {
            row->show();
        }
    }
    
    // AI Summarization button removed due to stability issues.
    
    // Final scroll to bottom - only if user was near the bottom or new message came in
    QScrollBar *vBar = ui_equipment->scrollArea_chat->verticalScrollBar();
    bool wasNearBottom = vBar->value() > (vBar->maximum() - 100);
    
    if (wasNearBottom || allMessages.size() > s_lastMessageCount) {
        QTimer::singleShot(100, this, [this](){
            if (ui_equipment && ui_equipment->scrollArea_chat) {
                QScrollBar *bar = ui_equipment->scrollArea_chat->verticalScrollBar();
                bar->setValue(bar->maximum());
            }
        });
    }
}


void MainWindow::onChatEmployeeListRefresh()
{
    if (!ui_equipment) return;
    ui_equipment->list_employees->clear();
    
    QSqlQuery q("SELECT FIRST_NAME, LAST_NAME, EMPLOYEE_ID, JOB_TITLE FROM EMPLOYEES ORDER BY FIRST_NAME ASC");
    while (q.next()) {
        QString name = q.value(0).toString() + " " + q.value(1).toString();
        int id = q.value(2).toInt();
        QString title = q.value(3).toString();
        
        if (id == currentEmployeeId) continue; // Don't chat with self in list

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(name + " (ID: " + QString::number(id) + ")");
        item->setData(Qt::UserRole, id);
        item->setToolTip(title);
        ui_equipment->list_employees->addItem(item);
        
        if (id == currentChatPartnerId) {
            item->setSelected(true);
            ui_equipment->list_employees->setCurrentItem(item);
        }
    }
}

void MainWindow::onChatAttachImage()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Attach Image", QDir::homePath(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (filePath.isEmpty()) return;

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open the selected image.");
        return;
    }
    pendingChatImage = f.readAll();
    f.close();

    // Show preview label
    QFileInfo fi(filePath);
    if (auto *lbl = equipmentPage->findChild<QLabel*>("lbl_img_preview")) {
        lbl->setText("Image attached: " + fi.fileName() + " — press Send to include it");
        lbl->setVisible(true);
    }
}

void MainWindow::onChatEmployeeSelected(QListWidgetItem *item)
{
    if (!item) return;
    currentChatPartnerId = item->data(Qt::UserRole).toInt();

    QString displayName = item->text().split("(").first().trimmed();

    // Update header: name label
    ui_equipment->lbl_active_employees->setText(displayName);

    // Update avatar initials in header
    if (auto *avatar = equipmentPage->findChild<QLabel*>("lbl_partner_avatar")) {
        QStringList parts = displayName.split(' ', Qt::SkipEmptyParts);
        QString initials;
        for (const QString &p : parts) initials += p.left(1).toUpper();
        initials = initials.left(2);
        avatar->setText(initials);
    }
    
    // Calculate last seen
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonArray allMessages = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
        QDateTime lastSeenTime;
        for (int i = allMessages.size() - 1; i >= 0; --i) {
            QJsonObject m = allMessages[i].toObject();
            if (m["sender_id"].toInt() == currentChatPartnerId) {
                lastSeenTime = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
                break;
            }
        }
        if (lastSeenTime.isValid()) {
            QDateTime now = QDateTime::currentDateTime();
            QString lsStr = "Last seen: ";
            if (lastSeenTime.date() == now.date()) {
                lsStr += "today at " + lastSeenTime.toString("HH:mm");
            } else if (lastSeenTime.date() == now.date().addDays(-1)) {
                lsStr += "yesterday at " + lastSeenTime.toString("HH:mm");
            } else {
                lsStr += lastSeenTime.toString("dd/MM/yyyy") + " at " + lastSeenTime.toString("HH:mm");
            }
            ui_equipment->lbl_chat_status->setText(lsStr);
            ui_equipment->lbl_chat_status->setStyleSheet("QLabel { color: rgba(255,255,255,0.6); font-size: 11px; background: transparent; border: none; padding: 0; }");
        } else {
            ui_equipment->lbl_chat_status->setText("Online");
            ui_equipment->lbl_chat_status->setStyleSheet("QLabel { color: #4CAF50; font-size: 11px; background: transparent; border: none; padding: 0; }");
        }
    }

    // Restart timer
    chatRefreshTimer->start(3000);

    // Clear Notification Badge if it was this partner
    // Assuming UI handles badge clearing upon entering chat or refreshing it.
    ui_equipment->tabWidget->setTabText(4, "Chat");

    onChatRefresh();
}

void MainWindow::onChatDeleteMessage(int index)
{
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray chatArray;

    if (file.open(QIODevice::ReadOnly)) {
        chatArray = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }

    if (index >= 0 && index < chatArray.size()) {
        // --- Backup deleted message to trash file ---
        QJsonObject deletedMsg = chatArray[index].toObject();
        deletedMsg["deleted_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        QFile trashFile("hammerdown_chat_trash.json");
        QJsonArray trashArray;
        if (trashFile.open(QIODevice::ReadOnly)) {
            trashArray = QJsonDocument::fromJson(trashFile.readAll()).array();
            trashFile.close();
        }
        trashArray.append(deletedMsg);
        if (trashFile.open(QIODevice::WriteOnly)) {
            trashFile.write(QJsonDocument(trashArray).toJson());
            trashFile.close();
        }
        
        chatArray.removeAt(index);
        
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(chatArray).toJson());
            file.close();
            onChatRefresh();
        }
    }
}

// =============================================================================
// EMOJI PICKER
// =============================================================================
void MainWindow::onChatEmojiClicked()
{
    QDialog *picker = new QDialog(this);
    picker->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    picker->setAttribute(Qt::WA_TranslucentBackground);
    picker->setFixedSize(360, 320);

    QFrame *card = new QFrame(picker);
    card->setObjectName("emojiCard");
    card->setGeometry(0, 0, 360, 320);
    card->setStyleSheet(
        "QFrame#emojiCard { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
        " border: 2px solid #8B6F47; border-radius: 16px; }");

    QVBoxLayout *mainLay = new QVBoxLayout(card);
    mainLay->setContentsMargins(12, 10, 12, 10);
    mainLay->setSpacing(6);

    QLabel *title = new QLabel("Emoji Picker", card);
    title->setStyleSheet("color: #D4AF37; font-size: 14px; font-weight: bold; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(title);

    // Emoji categories
    struct EmojiCategory { QString name; QStringList emojis; };
    QList<EmojiCategory> categories = {
        {"Smileys", {
            "\xF0\x9F\x98\x80", "\xF0\x9F\x98\x82", "\xF0\x9F\x98\x8D", "\xF0\x9F\x98\x8E", 
            "\xF0\x9F\x98\xAD", "\xF0\x9F\x98\xA1", "\xF0\x9F\x98\xB1", "\xF0\x9F\x98\xB4",
            "\xF0\x9F\x98\x98", "\xF0\x9F\x98\x9C", "\xF0\x9F\x98\x8C", "\xF0\x9F\x98\xA2",
            "\xF0\x9F\x98\x83", "\xF0\x9F\x98\x84", "\xF0\x9F\x98\x85", "\xF0\x9F\x98\x89"
        }},
        {"Hands", {
            "\xF0\x9F\x91\x8D", "\xF0\x9F\x91\x8E", "\xF0\x9F\x91\x8B", "\xE2\x9C\x8C",
            "\xF0\x9F\x91\x8F", "\xF0\x9F\x99\x8C", "\xF0\x9F\x92\xAA", "\xF0\x9F\xA4\x9D",
            "\xF0\x9F\x91\x86", "\xF0\x9F\x91\x87", "\xE2\x9C\x8A", "\xF0\x9F\xA4\x9E",
            "\xF0\x9F\x99\x8F", "\xF0\x9F\x91\x8C", "\xE2\x9C\x8B", "\xF0\x9F\xA4\x99"
        }},
        {"Objects", {
            "\xE2\x9D\xA4", "\xF0\x9F\x94\xA5", "\xE2\xAD\x90", "\xF0\x9F\x8E\x89",
            "\xF0\x9F\x92\xAF", "\xF0\x9F\x92\xAF", "\xF0\x9F\x91\x80", "\xF0\x9F\x92\xA1",
            "\xF0\x9F\x94\xA8", "\xF0\x9F\xAA\x9A", "\xF0\x9F\xAA\xB5", "\xF0\x9F\xAA\x93",
            "\xF0\x9F\x9B\xA0", "\xE2\x9A\x99", "\xF0\x9F\x93\x8B", "\xE2\x9C\x85"
        }}
    };

    QScrollArea *scroll = new QScrollArea(card);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #5A4A32; border-radius: 2px; }");
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLay = new QVBoxLayout(scrollContent);
    scrollLay->setSpacing(8);
    scrollLay->setContentsMargins(4, 4, 4, 4);

    for (const auto &cat : categories) {
        QLabel *catLabel = new QLabel(cat.name, scrollContent);
        catLabel->setStyleSheet("color: #8B6F47; font-size: 11px; font-weight: bold; background: transparent;");
        scrollLay->addWidget(catLabel);

        QWidget *grid = new QWidget(scrollContent);
        QGridLayout *gridLay = new QGridLayout(grid);
        gridLay->setSpacing(4);
        gridLay->setContentsMargins(0, 0, 0, 0);

        for (int i = 0; i < cat.emojis.size(); ++i) {
            QPushButton *btn = new QPushButton(QString::fromUtf8(cat.emojis[i].toUtf8()), grid);
            btn->setFixedSize(36, 36);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(
                "QPushButton { background: rgba(255,255,255,0.05); border: 1px solid rgba(139,111,71,0.3);"
                " border-radius: 8px; font-size: 20px; }"
                "QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; }");
            gridLay->addWidget(btn, i / 8, i % 8);

            connect(btn, &QPushButton::clicked, this, [this, btn, picker]() {
                if (ui_equipment)
                    ui_equipment->le_chat_input->insert(btn->text());
                picker->close();
            });
        }
        scrollLay->addWidget(grid);
    }
    scrollLay->addStretch();
    scroll->setWidget(scrollContent);
    mainLay->addWidget(scroll, 1);

    // Position above the GIF button
    QPushButton *emojiBtn = equipmentPage->findChild<QPushButton*>("btn_chat_emoji");
    if (emojiBtn) {
        QPoint pos = emojiBtn->mapToGlobal(QPoint(0, -picker->height() - 8));
        picker->move(pos);
    }
    picker->show();
}

// =============================================================================
// GIF PICKER (GIPHY API)
// =============================================================================
void MainWindow::onChatGifClicked()
{
    if (currentChatPartnerId == -1) {
        QMessageBox::information(this, "Select Employee", "Please select an employee first to send a GIF.");
        return;
    }

    QDialog *gifDialog = new QDialog(this);
    gifDialog->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    gifDialog->setAttribute(Qt::WA_TranslucentBackground);
    gifDialog->setFixedSize(420, 480);

    QFrame *card = new QFrame(gifDialog);
    card->setObjectName("gifCard");
    card->setGeometry(0, 0, 420, 480);
    card->setStyleSheet(
        "QFrame#gifCard { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
        " border: 2px solid #8B6F47; border-radius: 16px; }");

    QVBoxLayout *mainLay = new QVBoxLayout(card);
    mainLay->setContentsMargins(12, 12, 12, 12);
    mainLay->setSpacing(8);

    // Header
    QLabel *title = new QLabel("GIF Search — Powered by GIPHY", card);
    title->setStyleSheet("color: #D4AF37; font-size: 13px; font-weight: bold; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    mainLay->addWidget(title);

    // Search bar
    QHBoxLayout *searchLay = new QHBoxLayout();
    QLineEdit *searchInput = new QLineEdit(card);
    searchInput->setPlaceholderText("Search GIFs...");
    searchInput->setStyleSheet(
        "QLineEdit { background: rgba(255,255,255,0.08); border: 1.5px solid #5A4A32;"
        " border-radius: 14px; padding: 6px 14px; color: #F0E0C0; font-size: 13px; }"
        "QLineEdit:focus { border-color: #D4AF37; }");
    QPushButton *searchBtn = new QPushButton("Search", card);
    searchBtn->setFixedHeight(32);
    searchBtn->setCursor(Qt::PointingHandCursor);
    searchBtn->setStyleSheet(
        "QPushButton { background: #8B6F47; color: white; border-radius: 14px; padding: 0 16px;"
        " font-weight: bold; font-size: 12px; border: none; }"
        "QPushButton:hover { background: #A0825A; }");
    searchLay->addWidget(searchInput, 1);
    searchLay->addWidget(searchBtn);
    mainLay->addLayout(searchLay);

    // Loading indicator
    QLabel *loadingLabel = new QLabel("Type something and press Search, or browse trending GIFs below.", card);
    loadingLabel->setObjectName("gifLoadingLabel");
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setWordWrap(true);
    loadingLabel->setStyleSheet("color: #8B6F47; font-size: 11px; font-style: italic; background: transparent;");
    mainLay->addWidget(loadingLabel);

    // GIF grid in scroll area
    QScrollArea *scroll = new QScrollArea(card);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 5px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #5A4A32; border-radius: 2px; }");
    
    QWidget *gridWidget = new QWidget();
    gridWidget->setObjectName("gifGridWidget");
    QGridLayout *gridLay = new QGridLayout(gridWidget);
    gridLay->setSpacing(6);
    gridLay->setContentsMargins(4, 4, 4, 4);
    scroll->setWidget(gridWidget);
    mainLay->addWidget(scroll, 1);

    // GIPHY API key
    QString giphyKey = "Rb870UMsk9bec2cUYjWBzwbFsCaUOJN6";

    // Lambda to populate results
    auto populateGifs = [this, gridWidget, gridLay, gifDialog, loadingLabel](QNetworkReply *reply) {
        // Clear old results
        QLayoutItem *item;
        while ((item = gridLay->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }

        if (reply->error() != QNetworkReply::NoError) {
            loadingLabel->setText("Error loading GIFs: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray data = doc.object()["data"].toArray();
        reply->deleteLater();

        if (data.isEmpty()) {
            loadingLabel->setText("No GIFs found. Try a different search term.");
            return;
        }
        loadingLabel->hide(); // Hide the "found" text as requested

        for (int i = 0; i < qMin(data.size(), 50); ++i) {
            QJsonObject gif = data[i].toObject();
            QJsonObject fixedHeight = gif["images"].toObject()["fixed_height_small"].toObject();
            QString previewUrl = fixedHeight["url"].toString();
            QString fullUrl = gif["images"].toObject()["original"].toObject()["url"].toString();
            QString gifTitle = gif["title"].toString();

            // Create a clickable label for each GIF thumbnail
            QPushButton *gifBtn = new QPushButton(gridWidget);
            gifBtn->setFixedSize(120, 90);
            gifBtn->setCursor(Qt::PointingHandCursor);
            gifBtn->setToolTip(gifTitle);
            gifBtn->setStyleSheet(
                "QPushButton { background: rgba(255,255,255,0.05); border: 1.5px solid rgba(139,111,71,0.3);"
                " border-radius: 8px; }"
                "QPushButton:hover { border-color: #D4AF37; background: rgba(212,175,55,0.15); }");
            gridLay->addWidget(gifBtn, i / 3, i % 3);

            // Download preview thumbnail (animated)
            QNetworkAccessManager *thumbManager = new QNetworkAccessManager(gifBtn);
            QUrl thumbUrl(previewUrl);
            QNetworkReply *thumbReply = thumbManager->get(QNetworkRequest(thumbUrl));
            
            connect(thumbReply, &QNetworkReply::finished, gifBtn, [gifBtn, thumbReply]() {
                if (thumbReply->error() == QNetworkReply::NoError) {
                    QByteArray imgData = thumbReply->readAll();
                    
                    QLabel *movieLabel = new QLabel(gifBtn);
                    movieLabel->setFixedSize(116, 86);
                    movieLabel->move(2, 2);
                    movieLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
                    
                    QBuffer *buffer = new QBuffer(movieLabel);
                    buffer->setData(imgData);
                    buffer->open(QIODevice::ReadOnly);
                    
                    QMovie *movie = new QMovie(buffer, QByteArray(), movieLabel);
                    if (movie->isValid()) {
                        movieLabel->setMovie(movie);
                        movie->setScaledSize(QSize(116, 86));
                        movie->start();
                        movieLabel->show(); // This was missing
                    }
                }
                thumbReply->deleteLater();
            });

            // On click: send GIF as a message
            connect(gifBtn, &QPushButton::clicked, this, [this, fullUrl, gifTitle, gifDialog]() {
                // Save as a special GIF message in JSON
                QString chatFilePath = "hammerdown_chat.json";
                QFile file(chatFilePath);
                QJsonArray chatArray;
                if (file.open(QIODevice::ReadOnly)) {
                    chatArray = QJsonDocument::fromJson(file.readAll()).array();
                    file.close();
                }

                QJsonObject msgObj;
                msgObj["sender_id"] = currentEmployeeId;
                msgObj["receiver_id"] = currentChatPartnerId;
                msgObj["message"] = "[GIF]";
                msgObj["gif_url"] = fullUrl;
                msgObj["gif_title"] = gifTitle;
                msgObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                chatArray.append(msgObj);

                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(chatArray).toJson());
                    file.close();
                }
                gifDialog->close();
                onChatRefresh();
            });
        }
    };

    // Shared network manager for search requests
    QNetworkAccessManager *searchNetMgr = new QNetworkAccessManager(gifDialog);

    // Search handler
    auto doSearch = [searchInput, giphyKey, searchNetMgr, populateGifs, loadingLabel]() {
        QString query = searchInput->text().trimmed();
        QString urlStr;
        if (query.isEmpty()) {
            urlStr = QString("https://api.giphy.com/v1/gifs/trending?api_key=%1&limit=50&rating=g").arg(giphyKey);
        } else {
            urlStr = QString("https://api.giphy.com/v1/gifs/search?api_key=%1&q=%2&limit=50&rating=g")
                         .arg(giphyKey, QUrl::toPercentEncoding(query));
        }
        loadingLabel->setText("Loading GIFs...");
        QUrl searchUrl(urlStr);
        QNetworkReply *reply = searchNetMgr->get(QNetworkRequest(searchUrl));
        QObject::connect(reply, &QNetworkReply::finished, [reply, populateGifs]() {
            populateGifs(reply);
        });
    };

    connect(searchBtn, &QPushButton::clicked, gifDialog, doSearch);
    connect(searchInput, &QLineEdit::returnPressed, gifDialog, doSearch);

    // Load trending GIFs on open
    {
        QString trendingUrl = QString("https://api.giphy.com/v1/gifs/trending?api_key=%1&limit=50&rating=g").arg(giphyKey);
        loadingLabel->setText("Loading trending GIFs...");
        QUrl trendUrl(trendingUrl);
        QNetworkReply *reply = searchNetMgr->get(QNetworkRequest(trendUrl));
        connect(reply, &QNetworkReply::finished, [reply, populateGifs]() {
            populateGifs(reply);
        });
    }

    // Position above GIF button
    QPushButton *gifBtnUi = equipmentPage->findChild<QPushButton*>("btn_chat_gif");
    if (gifBtnUi) {
        QPoint pos = gifBtnUi->mapToGlobal(QPoint(-gifDialog->width()/2 + gifBtnUi->width()/2, -gifDialog->height() - 8));
        gifDialog->move(pos);
    }
    gifDialog->show();
    searchInput->setFocus();
}

void MainWindow::onChatSettingsClicked()
{
    // Apply blur effect to background
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(10.0);
    this->setGraphicsEffect(blur);

    QDialog *dialog = new QDialog(this);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setModal(true);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    dialog->setFixedSize(320, 360); // Adjusted height

    QFrame *card = new QFrame(dialog);
    card->setObjectName("settingsCard");
    card->setGeometry(0, 0, 320, 360);
    card->setStyleSheet(
        "QFrame#settingsCard { "
        " background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2C2418,stop:1 #1A140A);"
        " border: 2.5px solid #8B6F47; border-radius: 24px; color: #F0E0C0; "
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 28, 24, 24);
    layout->setSpacing(18);

    QLabel *title = new QLabel("Chat Settings", card);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #D4AF37; margin-bottom: 8px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // --- Classical Music Volume ---
    QLabel *volLbl = new QLabel("Ambient Volume", card);
    volLbl->setStyleSheet("color: #B8925A; font-size: 13px; font-weight: bold;");
    layout->addWidget(volLbl);

    QSlider *volSlider = new QSlider(Qt::Horizontal, card);
    volSlider->setRange(0, 100);
    volSlider->setValue(chatAudioOutput->volume() * 100); 
    volSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 6px; background: #3A2D1A; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 16px; height: 16px; margin: -5px 0; border-radius: 8px; background: #D4AF37; }"
    );
    layout->addWidget(volSlider);
    connect(volSlider, &QSlider::valueChanged, this, [this](int val) {
        chatAudioOutput->setVolume(val / 100.0); 
    });

    // --- Theme Selection Section Removed ---
    // (Modern theme is hidden, Classic is active)

    // --- Clear Chat ---
    QPushButton *btnClear = new QPushButton("🗑 Clear Conversation", card);
    btnClear->setStyleSheet(
        "QPushButton { background: rgba(200,50,50,0.15); border: 2px solid #662222; border-radius: 14px; height: 40px; color: #FF9999; font-weight: bold; }"
        "QPushButton:hover { background: #882222; color: white; }"
    );
    layout->addWidget(btnClear);
    connect(btnClear, &QPushButton::clicked, this, [this, dialog]() {
        if (QMessageBox::question(dialog, "Clear", "Wipe all messages for this chat?") == QMessageBox::Yes) {
            // Simple logic: remove all entries where sender or receiver is this conversation pair
            QString chatFilePath = "hammerdown_chat.json";
            QFile file(chatFilePath);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonArray oldArr = QJsonDocument::fromJson(file.readAll()).array();
                file.close();
                QJsonArray newArr;
                for (int i=0; i<oldArr.size(); ++i) {
                    QJsonObject m = oldArr[i].toObject();
                    int s = m["sender_id"].toInt();
                    int r = m["receiver_id"].toInt();
                    if (!((s == currentEmployeeId && r == currentChatPartnerId) ||
                          (s == currentChatPartnerId && r == currentEmployeeId))) {
                        newArr.append(m);
                    }
                }
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(QJsonDocument(newArr).toJson());
                    file.close();
                    onChatRefresh();
                    dialog->accept();
                }
            }
        }
    });

    // --- Restore Deleted Messages ---
    QPushButton *btnRestore = new QPushButton("♻ Restore Deleted Messages", card);
    btnRestore->setStyleSheet(
        "QPushButton { background: rgba(50,200,50,0.15); border: 2px solid #226622; border-radius: 14px; height: 40px; color: #99FF99; font-weight: bold; }"
        "QPushButton:hover { background: #228822; color: white; }"
    );
    layout->addWidget(btnRestore);
    connect(btnRestore, &QPushButton::clicked, this, [this]() {
        QDialog *restoreDialog = new QDialog(this);
        restoreDialog->setWindowTitle("Restore Messages");
        restoreDialog->setFixedSize(500, 400);
        restoreDialog->setStyleSheet(
            "QDialog { background: #1A140A; border: 2px solid #8B6F47; border-radius: 12px; }");
        
        QVBoxLayout *rLay = new QVBoxLayout(restoreDialog);
        QLabel *title = new QLabel("Select messages to restore:", restoreDialog);
        title->setStyleSheet("color: #D4AF37; font-weight: bold; font-size: 14px;");
        rLay->addWidget(title);
        
        QListWidget *list = new QListWidget(restoreDialog);
        list->setStyleSheet("QListWidget { background: rgba(0,0,0,0.3); color: #F0E0C0; border: 1px solid #5A4A32; border-radius: 8px; }"
                            "QListWidget::item { padding: 8px; border-bottom: 1px solid rgba(139,111,71,0.2); }");
        
        QFile trashFile("hammerdown_chat_trash.json");
        QJsonArray trashArray;
        if (trashFile.open(QIODevice::ReadOnly)) {
            trashArray = QJsonDocument::fromJson(trashFile.readAll()).array();
            trashFile.close();
        }
        
        for (int i = trashArray.size() - 1; i >= 0; --i) {
            QJsonObject m = trashArray[i].toObject();
            QString preview = m["message"].toString();
            if (preview == "[GIF]") preview = "GIF: " + m["gif_title"].toString();
            if (preview == "[Image]") preview = "Attached Image";
            
            QListWidgetItem *item = new QListWidgetItem(QString("[%1] %2").arg(m["timestamp"].toString().mid(11,5), preview));
            item->setData(Qt::UserRole, i);
            list->addItem(item);
        }
        rLay->addWidget(list);
        
        QPushButton *btnDoRestore = new QPushButton("Restore Selected", restoreDialog);
        btnDoRestore->setStyleSheet("background: #8B6F47; color: white; height: 36px; border-radius: 18px; font-weight: bold;");
        rLay->addWidget(btnDoRestore);
        
        connect(btnDoRestore, &QPushButton::clicked, this, [this, list, trashArray, restoreDialog]() {
            if (!list->currentItem()) return;
            int idxInTrash = list->currentItem()->data(Qt::UserRole).toInt();
            QJsonObject toRestore = trashArray[idxInTrash].toObject();
            toRestore.remove("deleted_at");
            
            // Move back to main chat
            QString chatFilePath = "hammerdown_chat.json";
            QFile file(chatFilePath);
            QJsonArray chatArray;
            if (file.open(QIODevice::ReadOnly)) {
                chatArray = QJsonDocument::fromJson(file.readAll()).array();
                file.close();
            }
            chatArray.append(toRestore);
            // Sort by timestamp if possible, but for now just append
            if (file.open(QIODevice::WriteOnly)) {
                file.write(QJsonDocument(chatArray).toJson());
                file.close();
            }
            
            // Remove from trash
            QJsonArray newTrash = trashArray;
            newTrash.removeAt(idxInTrash);
            QFile tf("hammerdown_chat_trash.json");
            if (tf.open(QIODevice::WriteOnly)) {
                tf.write(QJsonDocument(newTrash).toJson());
                tf.close();
            }
            
            onChatRefresh();
            restoreDialog->accept();
            QMessageBox::information(this, "Restored", "Message moved back to conversation.");
        });
        
        restoreDialog->exec();
    });

    layout->addStretch();

    QPushButton *closeBtn = new QPushButton("Save & Close", card);
    closeBtn->setStyleSheet(
        "QPushButton { background: #D4AF37; color: #1A1208; border-radius: 16px; height: 44px; font-weight: bold; font-size: 15px; }"
        "QPushButton:hover { background: #E5C060; }"
    );
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    // --- Animation ---
    card->setGraphicsEffect(nullptr);
    QPropertyAnimation *anim = new QPropertyAnimation(dialog, "windowOpacity");
    anim->setDuration(350);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    dialog->setWindowOpacity(0.0);
    dialog->show();
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    dialog->exec();

    // Cleanup blur
    this->setGraphicsEffect(nullptr);
    delete dialog;
}

void MainWindow::onWeatherAssistantClicked()
{
    if (!weatherAssistant) {
        weatherAssistant = new WeatherAssistant(equipmentPage);
    }
    
    // Simple toggle logic
    if (weatherAssistant->isVisible()) {
        weatherAssistant->hideAnimated();
    } else {
        const bool playWeatherVideoIntro = (homeWindow && homeWindow->isAnimationMode());

        weatherAssistant->refreshWeather();

        auto showAssistantAndResumeMusic = [this]() {
            weatherAssistant->showAnimated();
        };

        if (!playWeatherVideoIntro) {
            showAssistantAndResumeMusic();
            return;
        }
        
        // --- Full Screen Weather Report Video Intro ---
        QFrame *introFrame = new QFrame(equipmentPage);
        introFrame->setGeometry(equipmentPage->rect());
        introFrame->setStyleSheet("background-color: black; border-radius: 12px;");
        
        QVBoxLayout *introLayout = new QVBoxLayout(introFrame);
        introLayout->setContentsMargins(0, 0, 0, 0);
        
        QVideoWidget *videoWidget = new QVideoWidget(introFrame);
        introLayout->addWidget(videoWidget);
        
        QMediaPlayer *player = new QMediaPlayer(introFrame);
        QAudioOutput *audioOutput = new QAudioOutput(introFrame);
        player->setAudioOutput(audioOutput);
        audioOutput->setVolume(currentVolume);
        player->setVideoOutput(videoWidget);
        // Keep volume in sync with settings slider while video plays
        connect(this, &MainWindow::audioVolumeChanged, audioOutput, &QAudioOutput::setVolume);
        
        // Find local path for weather.mp4
        QString appDir = QCoreApplication::applicationDirPath();
        QString videoPath = appDir + "/../../../assets/weather.mp4"; 
        if (!QFile::exists(videoPath)) videoPath = appDir + "/../../assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = appDir + "/assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/../assets/weather.mp4";
        if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/../../assets/weather.mp4";

        player->setSource(QUrl::fromLocalFile(QFileInfo(videoPath).absoluteFilePath()));

        introFrame->show();
        introFrame->raise();

        // Fade-out overlay that sits above the video widget
        QFrame *fadeOverlay = new QFrame(introFrame);
        fadeOverlay->setGeometry(0, 0, introFrame->width(), introFrame->height());
        fadeOverlay->setStyleSheet("background: black;");
        QGraphicsOpacityEffect *overlayOpacity = new QGraphicsOpacityEffect(fadeOverlay);
        fadeOverlay->setGraphicsEffect(overlayOpacity);
        overlayOpacity->setOpacity(1.0);
        fadeOverlay->show();
        fadeOverlay->raise();

        // Fade the black overlay OUT to reveal the video
        QPropertyAnimation *fadeInAnim = new QPropertyAnimation(overlayOpacity, "opacity", fadeOverlay);
        fadeInAnim->setDuration(600);
        fadeInAnim->setStartValue(1.0);
        fadeInAnim->setEndValue(0.0);
        fadeInAnim->start(QAbstractAnimation::DeleteWhenStopped);

        connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia) {
                // Fade the overlay back IN to black, then show weather assistant
                fadeOverlay->raise();
                QGraphicsOpacityEffect *outOpacity = new QGraphicsOpacityEffect(fadeOverlay);
                fadeOverlay->setGraphicsEffect(outOpacity);
                outOpacity->setOpacity(0.0);

                QPropertyAnimation *fadeOutAnim = new QPropertyAnimation(outOpacity, "opacity", fadeOverlay);
                fadeOutAnim->setDuration(400);
                fadeOutAnim->setStartValue(0.0);
                fadeOutAnim->setEndValue(1.0);

                connect(fadeOutAnim, &QPropertyAnimation::finished, this, [=]() {
                    introFrame->deleteLater();
                    showAssistantAndResumeMusic();
                });
                fadeOutAnim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        });

        player->play();
    }
}

void MainWindow::onEquipmentBulkUpdateStatus()
{
    if (!ui_equipment || !ui_equipment->table_equipments->selectionModel()) return;

    QItemSelectionModel *selection = ui_equipment->table_equipments->selectionModel();
    QModelIndexList selectedRowsIndices = selection->selectedRows();

    if (selectedRowsIndices.isEmpty()) {
        QMessageBox::warning(this, trKey("No Selection"), trKey("Please select at least one equipment in the table."));
        return;
    }

    QString newStatus = ui_equipment->cb_bulk_status->currentText();
    QString confirmMsg = QString(trKey("Are you sure you want to change the status of %1 items to '%2'?"))
                             .arg(selectedRowsIndices.size())
                             .arg(newStatus);
    
    if (QMessageBox::question(this, trKey("Bulk Update"), confirmMsg) != QMessageBox::Yes) {
        return;
    }

    int successCount = 0;
    int failCount = 0;

    for (const QModelIndex &index : selectedRowsIndices) {
        // ID is in the third column (index 2) as defined in onEquipmentRefreshView
        QString id = index.siblingAtColumn(2).data().toString();
        
        QSqlQuery q;
        q.prepare("UPDATE EQUIPMENT SET STATUS = :status WHERE EQUIPMENT_ID = :id");
        q.bindValue(":status", newStatus);
        q.bindValue(":id", id);
        
        if (q.exec()) {
            successCount++;
        } else {
            failCount++;
        }
    }

    onEquipmentRefreshView();
    
    if (failCount == 0) {
        QMessageBox::information(this, trKey("Success"), 
            QString(trKey("Successfully updated status to '%1' for %2 equipment items."))
                .arg(newStatus).arg(successCount));
    } else {
        QMessageBox::warning(this, trKey("Partial Success"), 
            QString(trKey("Updated %1 items, but %2 failed. Check database logs."))
                .arg(successCount).arg(failCount));
    }
}

void MainWindow::onEquipmentShareToChat()
{
    if (ui_equipment->tabWidget->currentIndex() != 1) return; // Must be on View Tab

    // Find the currently selected row in table
    QModelIndexList selectedRows = ui_equipment->table_equipments->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "Select Equipment", "Please select an equipment line to share it.");
        return;
    }

    // Grab first selected row details (Assuming Model has headers: ID, Libelle, ...)
    int row = selectedRows.first().row();
    QString equipId = ui_equipment->table_equipments->model()->index(row, 0).data().toString();
    QString equipName = ui_equipment->table_equipments->model()->index(row, 1).data().toString();
    QString status = ui_equipment->table_equipments->model()->index(row, 4).data().toString();

    // Construct nice share card text
    QString shareCard = QString("\U0001f4e6 [Equipment Card]\nID: %1\nName: %2\nStatus: %3")
                            .arg(equipId, equipName, status);

    // Set text to chat input
    ui_equipment->le_chat_input->setText(shareCard);
    ui_equipment->le_chat_input->setFocus();

    // Switch to Chat Tab
    ui_equipment->tabWidget->setCurrentIndex(4);
}

// =============================================================================
// UNREAD MESSAGES SPLASH — after login
// =============================================================================
void MainWindow::showUnreadMessagesSplash()
{
    if (currentEmployeeId <= 0) return;
    
    // Read all messages from the JSON chat file
    QString chatFilePath = "hammerdown_chat.json";
    QFile file(chatFilePath);
    QJsonArray allMessages;
    
    if (file.open(QIODevice::ReadOnly)) {
        allMessages = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }
    
    if (allMessages.isEmpty()) return;
    
    // Count unread messages grouped by sender
    // "Unread" = messages sent TO the current user that are newer than the last message
    //            the current user sent in that conversation
    QMap<int, int> unreadCounts;          // sender_id -> count
    QMap<int, QString> senderNames;       // sender_id -> display name
    
    // Build employee name map
    QMap<int, QString> employeeNames;
    QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM EMPLOYEES");
    while (q.next()) {
        employeeNames[q.value(0).toInt()] = q.value(1).toString() + " " + q.value(2).toString();
    }
    
    // Find last sent timestamp per conversation partner
    QMap<int, QDateTime> lastSentTime; // partner_id -> last time we sent them a message
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();
        
        if (s_id == currentEmployeeId) {
            QDateTime dt = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
            if (!lastSentTime.contains(r_id) || dt > lastSentTime[r_id])
                lastSentTime[r_id] = dt;
        }
    }
    
    // Count messages received after our last sent message in each conversation
    for (int i = 0; i < allMessages.size(); ++i) {
        QJsonObject m = allMessages[i].toObject();
        int s_id = m["sender_id"].toInt();
        int r_id = m["receiver_id"].toInt();
        
        if (r_id == currentEmployeeId && s_id != currentEmployeeId) {
            QDateTime msgTime = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
            // If we never sent a message to this person, or this message is after our last reply
            if (!lastSentTime.contains(s_id) || msgTime > lastSentTime[s_id]) {
                unreadCounts[s_id]++;
                if (!senderNames.contains(s_id))
                    senderNames[s_id] = employeeNames.value(s_id, QString("Employee #%1").arg(s_id));
            }
        }
    }
    
    if (unreadCounts.isEmpty()) return;
    
    // Calculate total unread
    int totalUnread = 0;
    QStringList senderList;
    for (auto it = unreadCounts.begin(); it != unreadCounts.end(); ++it) {
        totalUnread += it.value();
        senderList << senderNames[it.key()];
    }
    
    // --- Create animated splash popup ---
    QDialog *splash = new QDialog(this);
    splash->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splash->setAttribute(Qt::WA_TranslucentBackground);
    splash->setFixedSize(400, 220);
    
    // Center on parent
    QPoint center = this->geometry().center();
    splash->move(center.x() - 200, center.y() - 110);
    
    QFrame *card = new QFrame(splash);
    card->setObjectName("splashCard");
    card->setGeometry(0, 0, 400, 220);
    card->setStyleSheet(
        "QFrame#splashCard {"
        " background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #2C2418,stop:0.5 #3A2D1A,stop:1 #1A140A);"
        " border: 2.5px solid #D4AF37;"
        " border-radius: 20px;"
        "}");
    
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(28, 24, 28, 20);
    lay->setSpacing(10);
    
    // Mail icon + title
    QLabel *iconLbl = new QLabel(QString::fromUtf8("\xF0\x9F\x93\xAC"), card);
    iconLbl->setStyleSheet("font-size: 36px; background: transparent; border: none;");
    iconLbl->setAlignment(Qt::AlignCenter);
    lay->addWidget(iconLbl);
    
    // Message text
    QString msgText;
    if (unreadCounts.size() == 1) {
        msgText = QString("You have %1 new message%2 from\n%3")
            .arg(totalUnread)
            .arg(totalUnread > 1 ? "s" : "")
            .arg(senderList.first());
    } else {
        msgText = QString("You have %1 new message%2 from\n%3")
            .arg(totalUnread)
            .arg(totalUnread > 1 ? "s" : "")
            .arg(senderList.join(", "));
    }
    
    QLabel *msgLbl = new QLabel(msgText, card);
    msgLbl->setAlignment(Qt::AlignCenter);
    msgLbl->setWordWrap(true);
    msgLbl->setStyleSheet(
        "color: #F0E0C0; font-size: 15px; font-weight: 600;"
        " background: transparent; border: none; line-height: 1.4;");
    lay->addWidget(msgLbl);
    
    // Buttons row
    QHBoxLayout *btnLay = new QHBoxLayout();
    btnLay->setSpacing(12);
    
    QPushButton *viewBtn = new QPushButton(QString::fromUtf8("\xF0\x9F\x92\xAC Open Chat"), card);
    viewBtn->setCursor(Qt::PointingHandCursor);
    viewBtn->setStyleSheet(
        "QPushButton { background: #D4AF37; color: #1A1208; border-radius: 14px;"
        " padding: 8px 20px; font-weight: bold; font-size: 13px; border: none; }"
        "QPushButton:hover { background: #E5C060; }");
    
    QPushButton *dismissBtn = new QPushButton("Dismiss", card);
    dismissBtn->setCursor(Qt::PointingHandCursor);
    dismissBtn->setStyleSheet(
        "QPushButton { background: rgba(139,111,71,0.2); color: #B8925A; border-radius: 14px;"
        " padding: 8px 20px; font-weight: bold; font-size: 13px; border: 1.5px solid #5A4A32; }"
        "QPushButton:hover { background: rgba(139,111,71,0.4); color: #D4AF37; border-color: #8B6F47; }");
    
    btnLay->addStretch();
    btnLay->addWidget(viewBtn);
    btnLay->addWidget(dismissBtn);
    btnLay->addStretch();
    lay->addLayout(btnLay);
    
    // Connect buttons
    connect(dismissBtn, &QPushButton::clicked, splash, &QDialog::accept);
    connect(viewBtn, &QPushButton::clicked, this, [this, splash, unreadCounts]() {
        splash->accept();
        // Navigate to Equipment -> Chat tab
        ui->stackedWidget->setCurrentIndex(5);
        ui_equipment->tabWidget->setCurrentIndex(4);
        onChatEmployeeListRefresh();
        
        // Auto-select the first sender with unread messages
        if (!unreadCounts.isEmpty()) {
            int firstSenderId = unreadCounts.begin().key();
            for (int i = 0; i < ui_equipment->list_employees->count(); ++i) {
                auto *item = ui_equipment->list_employees->item(i);
                if (item->data(Qt::UserRole).toInt() == firstSenderId) {
                    ui_equipment->list_employees->setCurrentItem(item);
                    onChatEmployeeSelected(item);
                    break;
                }
            }
        }
    });
    
    // --- Entrance animation ---
    splash->setWindowOpacity(0.0);
    splash->show();
    
    QPropertyAnimation *fadeIn = new QPropertyAnimation(splash, "windowOpacity");
    fadeIn->setDuration(500);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Subtle bounce animation on the card
    QPropertyAnimation *bounceAnim = new QPropertyAnimation(card, "geometry");
    bounceAnim->setDuration(600);
    bounceAnim->setStartValue(QRect(0, 30, 400, 220));
    bounceAnim->setEndValue(QRect(0, 0, 400, 220));
    bounceAnim->setEasingCurve(QEasingCurve::OutBack);
    bounceAnim->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Auto-dismiss after 8 seconds
    QTimer::singleShot(8000, splash, [splash]() {
        if (splash->isVisible()) {
            QPropertyAnimation *fadeOut = new QPropertyAnimation(splash, "windowOpacity");
            fadeOut->setDuration(400);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            fadeOut->setEasingCurve(QEasingCurve::InCubic);
            connect(fadeOut, &QPropertyAnimation::finished, splash, &QDialog::accept);
            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });
    
    splash->exec();
    delete splash;
}

// =============================================================================
// CHAT SEARCH (Ctrl+F)
// =============================================================================
void MainWindow::onChatSearchToggle()
{
    if (!ui_equipment || ui_equipment->tabWidget->currentIndex() != 4) return;
    
    // Check if search bar already exists, toggle visibility
    QWidget *existingBar = equipmentPage->findChild<QWidget*>("chatSearchBar");
    if (existingBar) {
        bool isVisible = existingBar->isVisible();
        existingBar->setVisible(!isVisible);
        if (!isVisible) {
            // Focus the search input
            QLineEdit *searchInput = existingBar->findChild<QLineEdit*>("chatSearchInput");
            if (searchInput) searchInput->setFocus();
        }
        return;
    }
    
    // Create search bar above the chat scroll area
    QWidget *searchBar = new QWidget(equipmentPage);
    searchBar->setObjectName("chatSearchBar");
    searchBar->setFixedHeight(50);
    searchBar->setStyleSheet(
        "QWidget#chatSearchBar {"
        " background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3820);"
        " border-bottom: 2px solid #8B6F47;"
        "}");
    
    QHBoxLayout *barLay = new QHBoxLayout(searchBar);
    barLay->setContentsMargins(12, 6, 12, 6);
    barLay->setSpacing(8);
    
    QLabel *searchIcon = new QLabel(QString::fromUtf8("\xF0\x9F\x94\x8D"), searchBar);
    searchIcon->setStyleSheet("font-size: 18px; background: transparent; border: none;");
    barLay->addWidget(searchIcon);
    
    QLineEdit *searchInput = new QLineEdit(searchBar);
    searchInput->setObjectName("chatSearchInput");
    searchInput->setPlaceholderText("Search messages... (Ctrl+F)");
    searchInput->setStyleSheet(
        "QLineEdit { background: rgba(0,0,0,0.3); color: #F0E0C0; border: 1.5px solid #5A4A32;"
        " border-radius: 14px; padding: 6px 14px; font-size: 13px; }"
        "QLineEdit:focus { border-color: #D4AF37; }");
    barLay->addWidget(searchInput, 1);
    
    // Result count label
    QLabel *resultLbl = new QLabel("", searchBar);
    resultLbl->setObjectName("chatSearchResultLbl");
    resultLbl->setStyleSheet("color: #B8925A; font-size: 11px; background: transparent; border: none; min-width: 80px;");
    barLay->addWidget(resultLbl);
    
    // Navigate buttons
    QPushButton *prevBtn = new QPushButton(QString::fromUtf8("\xE2\x96\xB2"), searchBar);
    prevBtn->setFixedSize(30, 30);
    prevBtn->setCursor(Qt::PointingHandCursor);
    prevBtn->setStyleSheet(
        "QPushButton { background: rgba(139,111,71,0.2); color: #D4AF37; border: 1px solid #5A4A32;"
        " border-radius: 15px; font-size: 12px; }"
        "QPushButton:hover { background: #8B6F47; color: white; }");
    barLay->addWidget(prevBtn);
    
    QPushButton *nextBtn = new QPushButton(QString::fromUtf8("\xE2\x96\xBC"), searchBar);
    nextBtn->setFixedSize(30, 30);
    nextBtn->setCursor(Qt::PointingHandCursor);
    nextBtn->setStyleSheet(
        "QPushButton { background: rgba(139,111,71,0.2); color: #D4AF37; border: 1px solid #5A4A32;"
        " border-radius: 15px; font-size: 12px; }"
        "QPushButton:hover { background: #8B6F47; color: white; }");
    barLay->addWidget(nextBtn);
    
    QPushButton *closeSearchBtn = new QPushButton(QString::fromUtf8("\xE2\x9C\x95"), searchBar);
    closeSearchBtn->setFixedSize(30, 30);
    closeSearchBtn->setCursor(Qt::PointingHandCursor);
    closeSearchBtn->setStyleSheet(
        "QPushButton { background: rgba(200,50,50,0.15); color: #FF7070; border: 1px solid rgba(200,50,50,0.3);"
        " border-radius: 15px; font-size: 12px; }"
        "QPushButton:hover { background: #CC3333; color: white; }");
    barLay->addWidget(closeSearchBtn);
    
    // Insert the search bar into the chat panel layout, after the header
    QVBoxLayout *chatPanelLayout = qobject_cast<QVBoxLayout*>(ui_equipment->frame_chat_panel->layout());
    if (chatPanelLayout) {
        chatPanelLayout->insertWidget(1, searchBar); // After header (index 0)
    } else {
        // Fallback: just parent it
        searchBar->setParent(ui_equipment->frame_chat_panel);
        searchBar->show();
    }
    
    searchInput->setFocus();
    
    // Shared state for navigation
    auto *matchIndices = new QList<int>();
    auto *currentMatchIdx = new int(-1);
    
    // Search logic
    auto doSearch = [this, searchInput, resultLbl, matchIndices, currentMatchIdx]() {
        QString query = searchInput->text().trimmed();
        matchIndices->clear();
        *currentMatchIdx = -1;
        
        // First, clear any existing highlights
        QLayout *chatLayout = ui_equipment->verticalLayout_chat_contents;
        for (int i = 0; i < chatLayout->count(); ++i) {
            QWidget *w = chatLayout->itemAt(i)->widget();
            if (!w) continue;
            // Reset opacity/highlight
            w->setGraphicsEffect(nullptr);
            // Find all QLabels with "messageText" or word-wrap child labels
            QList<QLabel*> labels = w->findChildren<QLabel*>();
            for (QLabel *lbl : labels) {
                if (lbl->wordWrap()) {
                    // Remove HTML highlighting - restore plain text
                    QString text = lbl->text();
                    text.replace(QRegularExpression("<span style=[^>]*>"), "");
                    text.replace("</span>", "");
                    lbl->setText(text);
                }
            }
        }
        
        if (query.isEmpty()) {
            resultLbl->setText("");
            return;
        }
        
        // Search through all visible message widgets
        for (int i = 0; i < chatLayout->count(); ++i) {
            QWidget *w = chatLayout->itemAt(i)->widget();
            if (!w) continue;
            
            QList<QLabel*> labels = w->findChildren<QLabel*>();
            bool found = false;
            for (QLabel *lbl : labels) {
                if (lbl->wordWrap() && lbl->text().contains(query, Qt::CaseInsensitive)) {
                    found = true;
                    // Highlight matched text
                    QString text = lbl->text();
                    int idx = text.indexOf(query, 0, Qt::CaseInsensitive);
                    while (idx != -1) {
                        QString matched = text.mid(idx, query.length());
                        text.replace(idx, query.length(),
                            QString("<span style='background-color: #D4AF37; color: #1A1208; padding: 1px 3px; border-radius: 3px;'>%1</span>").arg(matched));
                        idx = text.indexOf(query, idx + 100, Qt::CaseInsensitive); // skip past the HTML we just inserted
                    }
                    lbl->setTextFormat(Qt::RichText);
                    lbl->setText(text);
                }
            }
            if (found) {
                matchIndices->append(i);
            }
        }
        
        if (matchIndices->isEmpty()) {
            resultLbl->setText("No results");
        } else {
            *currentMatchIdx = 0;
            resultLbl->setText(QString("1 of %1").arg(matchIndices->size()));
            
            // Scroll to first match
            QWidget *firstMatch = chatLayout->itemAt(matchIndices->first())->widget();
            if (firstMatch) {
                ui_equipment->scrollArea_chat->ensureWidgetVisible(firstMatch, 50, 50);
            }
        }
    };
    
    connect(searchInput, &QLineEdit::textChanged, doSearch);
    connect(searchInput, &QLineEdit::returnPressed, [nextBtn]() { nextBtn->click(); });
    
    // Navigate to next match
    connect(nextBtn, &QPushButton::clicked, [this, matchIndices, currentMatchIdx, resultLbl]() {
        if (matchIndices->isEmpty()) return;
        *currentMatchIdx = (*currentMatchIdx + 1) % matchIndices->size();
        resultLbl->setText(QString("%1 of %2").arg(*currentMatchIdx + 1).arg(matchIndices->size()));
        
        QWidget *match = ui_equipment->verticalLayout_chat_contents->itemAt(matchIndices->at(*currentMatchIdx))->widget();
        if (match) {
            ui_equipment->scrollArea_chat->ensureWidgetVisible(match, 50, 50);
        }
    });
    
    // Navigate to previous match
    connect(prevBtn, &QPushButton::clicked, [this, matchIndices, currentMatchIdx, resultLbl]() {
        if (matchIndices->isEmpty()) return;
        *currentMatchIdx = (*currentMatchIdx - 1 + matchIndices->size()) % matchIndices->size();
        resultLbl->setText(QString("%1 of %2").arg(*currentMatchIdx + 1).arg(matchIndices->size()));
        
        QWidget *match = ui_equipment->verticalLayout_chat_contents->itemAt(matchIndices->at(*currentMatchIdx))->widget();
        if (match) {
            ui_equipment->scrollArea_chat->ensureWidgetVisible(match, 50, 50);
        }
    });
    
    // Close search bar
    connect(closeSearchBtn, &QPushButton::clicked, [searchBar, this]() {
        // Clear highlights before closing
        QLayout *chatLayout = ui_equipment->verticalLayout_chat_contents;
        for (int i = 0; i < chatLayout->count(); ++i) {
            QWidget *w = chatLayout->itemAt(i)->widget();
            if (!w) continue;
            QList<QLabel*> labels = w->findChildren<QLabel*>();
            for (QLabel *lbl : labels) {
                if (lbl->wordWrap()) {
                    QString text = lbl->text();
                    text.replace(QRegularExpression("<span style=[^>]*>"), "");
                    text.replace("</span>", "");
                    lbl->setTextFormat(Qt::PlainText);
                    lbl->setText(text);
                }
            }
        }
        searchBar->hide();
    });
    
    // Escape key also closes
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), searchBar);
    connect(escShortcut, &QShortcut::activated, closeSearchBtn, &QPushButton::click);
}

// =============================================================================
// VOICE RECORDING & FEEDBACK
// =============================================================================

void MainWindow::shakeWidget(QWidget *w) {
    if (!w) return;
    QPropertyAnimation *anim = new QPropertyAnimation(w, "pos");
    anim->setDuration(100);
    anim->setLoopCount(3);
    QPoint op = w->pos();
    anim->setStartValue(op);
    anim->setKeyValueAt(0.25, op + QPoint(6, 0));
    anim->setKeyValueAt(0.75, op - QPoint(6, 0));
    anim->setEndValue(op);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onChatVoiceToggled() {
    auto *btn = equipmentPage->findChild<QPushButton*>("btn_chat_voice");
    if (!btn) return;
    if (btn->isChecked()) {
        onChatStartRecord();
    } else {
        onChatStopRecord();
    }
}

void MainWindow::onChatStartRecord() {
    QString voiceDir = QDir::currentPath() + "/voice_notes";
    QDir().mkpath(voiceDir);
    QString fileName = QString("voice_%1.wav").arg(QDateTime::currentMSecsSinceEpoch());
    m_recorder->setOutputLocation(QUrl::fromLocalFile(voiceDir + "/" + fileName));
    m_recorder->record();
    m_isRecording = true;
    ui_equipment->le_chat_input->setPlaceholderText("Recording voice note...");
}

void MainWindow::onChatStopRecord() {
    m_recorder->stop();
    m_isRecording = false;
    ui_equipment->le_chat_input->setPlaceholderText("Type a message...");
    
    // Auto-send the voice note
    QTimer::singleShot(200, this, [this](){
        ui_equipment->le_chat_input->setText(QString::fromUtf8("\xF0\x9F\x8E\x99 Voice Note"));
        onChatSendMessage();
    });
}

void MainWindow::onUploadAvatar() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty()) return;

    QString employeeId = ui_employee->le_id->text();
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, tr("Avatar"), tr("Please select an employee or enter an ID first."));
        return;
    }

    QDir().mkpath("data/avatars");
    QString destPath = QString("data/avatars/employee_%1.png").arg(employeeId);
    if (QFile::exists(destPath)) QFile::remove(destPath);
    if (QFile::copy(fileName, destPath)) {
        QPixmap pix(destPath);
        ui_employee->lbl_avatar->setPixmap(getCircularPixmap(pix).scaled(ui_employee->lbl_avatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QMessageBox::information(this, tr("Avatar"), tr("Avatar uploaded successfully."));
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
        // Capture for current stage
        QVideoFrame frame = m_empVideoSink->videoFrame();
        if (frame.isValid() && frame.map(QVideoFrame::ReadOnly)) {
            QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
            frame.unmap();
            
            QDir().mkpath("data/faces");
            QString suffix = (m_faceScanStage == 0) ? "" : (m_faceScanStage == 1 ? "_left" : "_right");
            QString destPath = QString("data/faces/face_%1%2.png").arg(employeeId).arg(suffix);
            if (QFile::exists(destPath)) QFile::remove(destPath);
            image.save(destPath);
            
            m_faceScanStage++;
            if (m_faceScanStage == 1) {
                ui_employee->btn_scan_face->setText(tr("Capture LEFT"));
                QMessageBox::information(this, tr("Face Scan"), tr("Center captured! Now please turn your face to the LEFT and click 'Capture LEFT'."));
            } else if (m_faceScanStage == 2) {
                ui_employee->btn_scan_face->setText(tr("Capture RIGHT"));
                QMessageBox::information(this, tr("Face Scan"), tr("Left captured! Now please turn your face to the RIGHT and click 'Capture RIGHT'."));
            } else {
                // Done
                m_empCamera->stop();
                m_isEmpFaceScanActive = false;
                m_faceScanStage = 0;
                ui_employee->btn_scan_face->setText(tr("Scan Face ID"));
                QMessageBox::information(this, tr("Face ID"), tr("Face ID registered successfully for all angles (Center, Left, Right)."));
            }
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
        ui_employee->btn_scan_face->setText(tr("Capture CENTER"));
        QMessageBox::information(this, tr("Face Scan"), tr("Scanning started. Please look straight at the camera (CENTER) and click 'Capture CENTER'."));
    }
}

void MainWindow::processEmpCameraFrame() {
    if (!m_isEmpFaceScanActive) return;
    
    QVideoFrame frame = m_empVideoSink->videoFrame();
    if (!frame.isValid() || !frame.map(QVideoFrame::ReadOnly)) return;
    
    QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
    frame.unmap();
    
    // Show preview in the avatar label (circular)
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
           
           QString avatarPath = QString("data/avatars/employee_%1.png").arg(currentEmployeeId);
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

void MainWindow::onEmployeeEnsureHistoryTable() {
    QSqlQuery q;
    // Create the history table (ignore if already exists)
    q.exec("CREATE TABLE APP_HISTORY ("
           "  LOG_ID          NUMBER PRIMARY KEY,"
           "  LOG_DATE        TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
           "  EMPLOYEE_NAME   VARCHAR2(200),"
           "  ACTION_DETAILS  VARCHAR2(1000),"
           "  MODULE_NAME     VARCHAR2(100)"
           ")");
    // Create the sequence (ignore if exists)
    q.exec("CREATE SEQUENCE REQ_HIST_SEQ START WITH 1 INCREMENT BY 1");
}

void MainWindow::logActivity(const QString &action, const QString &module) {
    QSqlQuery q;
    // Get full name of current employee
    QString empName = "System";
    if (currentEmployeeId > 0) {
        QSqlQuery nq;
        nq.prepare("SELECT FIRST_NAME || ' ' || LAST_NAME FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        nq.bindValue(":id", currentEmployeeId);
        if (nq.exec() && nq.next()) empName = nq.value(0).toString();
    }

    q.prepare("INSERT INTO APP_HISTORY (LOG_ID, LOG_DATE, EMPLOYEE_NAME, ACTION_DETAILS, MODULE_NAME) "
              "VALUES (REQ_HIST_SEQ.NEXTVAL, CURRENT_TIMESTAMP, :name, :action, :module)");
    q.bindValue(":name", empName);
    q.bindValue(":action", action);
    q.bindValue(":module", module);
    q.exec();
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

    m_mapStatusLabel = new QLabel("Select a client or enter Buyer ID, then click Load Map.", mapTab);
    m_mapStatusLabel->setWordWrap(true);
    m_mapStatusLabel->setStyleSheet("color: #8B6F47; font-size: 12px; font-style: italic;");

    m_mapClientTable = new QTableWidget(mapTab);
    m_mapClientTable->setColumnCount(3);
    m_mapClientTable->setHorizontalHeaderLabels({"ID", "Name", "Address"});
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
    QSqlQuery q("SELECT CLIENT_ID, FIRST_NAME, LAST_NAME, ADDRESS FROM CLIENTS ORDER BY CLIENT_ID");
    int row = 0;
    while (q.next()) {
        m_mapClientTable->insertRow(row);
        m_mapClientTable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        m_mapClientTable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString() + " " + q.value(2).toString()));
        m_mapClientTable->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        row++;
    }

    if (row == 0) {
        m_mapStatusLabel->setText("No clients found.");
    }
}

void MainWindow::requestMapForBuyerId()
{
    if (!ui_order || !ui_order->le_buyer) return;

    QString address;

    if (m_mapClientTable && m_mapClientTable->currentRow() >= 0) {
        int row = m_mapClientTable->currentRow();
        QTableWidgetItem *addrItem = m_mapClientTable->item(row, 2);
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
        q.prepare("SELECT ADDRESS FROM CLIENTS WHERE CLIENT_ID = :id");
        q.bindValue(":id", clientId);
        if (!q.exec() || !q.next()) {
            m_mapStatusLabel->setText("No client found for that Buyer ID.");
            m_mapAddressLabel->setText("Address: --");
            return;
        }

        address = q.value(0).toString().trimmed();
        if (address.isEmpty()) {
            m_mapStatusLabel->setText("Client has no address on file.");
            m_mapAddressLabel->setText("Address: --");
            return;
        }
    }

    m_mapAddressLabel->setText("Address: " + address);
    m_mapStatusLabel->setText("Geocoding address...");
    m_mapImageSize = (m_mapFullscreenDialog && m_mapFullscreenDialog->isVisible() && m_mapFullscreenLabel)
        ? m_mapFullscreenLabel->size()
        : m_mapImageLabel->size();

    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery query;
    query.addQueryItem("q", address + ", Tunisia");
    query.addQueryItem("format", "json");
    query.addQueryItem("limit", "1");
    query.addQueryItem("countrycodes", "tn");
    query.addQueryItem("bounded", "1");
    query.addQueryItem("viewbox", "7.5,37.6,11.6,30.2");
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
    QNetworkReply *reply = m_mapNet->get(req);
    reply->setProperty("mapType", "geocode");
    reply->setProperty("address", address);
    reply->setProperty("geocodeStage", "tn");
}

void MainWindow::onMapNetworkFinished(QNetworkReply *reply)
{
    if (!reply) return;
    const QString type = reply->property("mapType").toString();

    if (reply->error() != QNetworkReply::NoError) {
        m_mapStatusLabel->setText("Network error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    if (type == "geocode") {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray arr = doc.array();
        if (arr.isEmpty()) {
            QString stage = reply->property("geocodeStage").toString();
            QString address = reply->property("address").toString();
            if (stage == "tn" && !address.isEmpty()) {
                QUrl url("https://nominatim.openstreetmap.org/search");
                QUrlQuery query;
                query.addQueryItem("q", address);
                query.addQueryItem("format", "json");
                query.addQueryItem("limit", "1");
                url.setQuery(query);

                QNetworkRequest req(url);
                req.setHeader(QNetworkRequest::UserAgentHeader, "HammerDownApp/1.0");
                QNetworkReply *fallback = m_mapNet->get(req);
                fallback->setProperty("mapType", "geocode");
                fallback->setProperty("address", address);
                fallback->setProperty("geocodeStage", "global");
                reply->deleteLater();
                return;
            }

            m_mapStatusLabel->setText("Address not found on map.");
            reply->deleteLater();
            return;
        }

        QJsonObject obj = arr.first().toObject();
        QString lat = obj.value("lat").toString();
        QString lon = obj.value("lon").toString();
        if (lat.isEmpty() || lon.isEmpty()) {
            m_mapStatusLabel->setText("Geocoding failed.");
            reply->deleteLater();
            return;
        }

        m_mapCenterLat = lat.toDouble();
        m_mapCenterLon = lon.toDouble();
        m_mapStatusLabel->setText("Loading map tiles...");
        requestMapTiles(m_mapCenterLat, m_mapCenterLon);
        reply->deleteLater();
        return;
    }

    if (type == "tile") {
        const QString tileKey = reply->property("tileKey").toString();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imgData = reply->readAll();
            QPixmap pix;
            if (pix.loadFromData(imgData)) {
                m_mapTileCache.insert(tileKey, pix);
            } else {
                m_mapTileErrors++;
            }
        } else {
            m_mapTileErrors++;
        }

        m_mapPendingTiles.remove(tileKey);

        if (m_mapPendingTiles.isEmpty()) {
            QPixmap mapPixmap(m_mapImageSize);
            mapPixmap.fill(QColor(26, 18, 8));
            QPainter painter(&mapPixmap);

            const int tileSize = 256;
            for (int x = m_mapTileX0; x <= m_mapTileX1; ++x) {
                for (int y = m_mapTileY0; y <= m_mapTileY1; ++y) {
                    QString key = QString("%1/%2/%3").arg(m_mapZoom).arg(x).arg(y);
                    if (!m_mapTileCache.contains(key)) continue;
                    int px = qRound((x * tileSize) - m_mapTopLeftX);
                    int py = qRound((y * tileSize) - m_mapTopLeftY);
                    painter.drawPixmap(px, py, m_mapTileCache.value(key));
                }
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
            if (m_mapTileErrors > 0) {
                m_mapStatusLabel->setText("Map loaded with missing tiles.");
            } else {
                m_mapStatusLabel->setText("Map loaded successfully.");
            }
        }

        reply->deleteLater();
        return;
    }

    reply->deleteLater();
}

void MainWindow::requestMapTiles(double lat, double lon)
{
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

    m_mapTileCache.clear();
    m_mapPendingTiles.clear();
    m_mapTileErrors = 0;

    for (int x = m_mapTileX0; x <= m_mapTileX1; ++x) {
        int wrappedX = ((x % n) + n) % n;
        for (int y = m_mapTileY0; y <= m_mapTileY1; ++y) {
            if (y < 0 || y >= n) continue;

            QString key = QString("%1/%2/%3").arg(zoom).arg(x).arg(y);
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

    if (m_mapPendingTiles.isEmpty()) {
        m_mapStatusLabel->setText("Map tiles not available for this location.");
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
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

void MainWindow::loadSupplierMapPins()
{
    m_supplierMapStatusLabel->setText("Geocoding suppliers...");
    m_supplierPins.clear();
    m_supplierGeocodePendingCount = 0;

    QSqlQuery q("SELECT SUPPLIER_ID, SUPPLIER_NAME, TYPE_NOTIFICATION, ACCOUNT_STATUS, ADDRESS FROM SUPPLIERS");
    while (q.next()) {
        int id = q.value(0).toInt();
        QString name = q.value(1).toString();
        QString type = q.value(2).toString();
        QString status = q.value(3).toString();
        QString address = q.value(4).toString().trimmed();

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

        int cx = qRound(lonToX(m_supplierCenterLon) - m_supplierMapTopLeftX);
        int cy = qRound(latToY(m_supplierCenterLat) - m_supplierMapTopLeftY);
        
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(QColor("#D4AF37"));
        painter.drawRect(cx - 10, cy - 10, 20, 20);
        painter.drawText(cx - 30, cy + 25, "Workshop");

        for (int i=0; i<m_supplierPins.size(); ++i) {
            auto &pin = m_supplierPins[i];
            int px = qRound(lonToX(pin.lon) - m_supplierMapTopLeftX);
            int py = qRound(latToY(pin.lat) - m_supplierMapTopLeftY);
            
            QColor color = pin.status == "Active" ? QColor(0, 255, 100, 200) : QColor(255, 50, 50, 200);
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

    int cx = qRound(lonToX(m_supplierCenterLon) - m_supplierMapTopLeftX);
    int cy = qRound(latToY(m_supplierCenterLat) - m_supplierMapTopLeftY);
    
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QColor("#D4AF37"));
    painter.drawRect(cx - 10, cy - 10, 20, 20);
    painter.drawText(cx - 30, cy + 25, "Workshop");

    for (int i=0; i<m_supplierPins.size(); ++i) {
        auto &pin = m_supplierPins[i];
        int px = qRound(lonToX(pin.lon) - m_supplierMapTopLeftX);
        int py = qRound(latToY(pin.lat) - m_supplierMapTopLeftY);
        
        QColor color = pin.status == "Active" ? QColor(0, 255, 100, 200) : QColor(255, 50, 50, 200);
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

void MainWindow::onSupplierBellClicked()
{
    // Refresh first
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






