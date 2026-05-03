#include "clientmanagement.h"
#include "mainwindow.h"

namespace {
QString trKey(const QString &key)
{
    return QCoreApplication::translate("QObject", key.toUtf8().constData());
}
void setTrKey(QWidget *widget, const QString &key)
{
    if (widget) widget->setProperty("trKey", key);
}
}

#include "ui_mainwindow.h"
#include "ui_client_management.h"
#include "smtpsender.h"
#include <QStandardItemModel>
#include <QHeaderView>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QPageLayout>
#include <QPageSize>
#include <QRadioButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QTableView>
#include <QPushButton>
#include <QListWidget>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPair>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChart>

// =============================================================================
//  CLIENT STATS TAB
// =============================================================================

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
        const int totalGender = maleCount + femaleCount;
        const double malePct   = totalGender > 0 ? (100.0 * maleCount   / totalGender) : 0.0;
        const double femalePct = totalGender > 0 ? (100.0 * femaleCount / totalGender) : 0.0;

        series->append(trKey("Male"), maleCount);
        series->append(trKey("Female"), femaleCount);

        QPieSlice *sliceMale = series->slices().at(0);
        sliceMale->setBrush(goldColor);
        sliceMale->setLabel(QString("%1  %2 (%3%)")
                            .arg(trKey("Male"))
                            .arg(maleCount)
                            .arg(QString::number(malePct, 'f', 1)));
        sliceMale->setLabelVisible(maleCount > 0);
        sliceMale->setLabelColor(Qt::white);
        sliceMale->setLabelFont(chartFont);

        QPieSlice *sliceFemale = series->slices().at(1);
        sliceFemale->setBrush(silverColor);
        sliceFemale->setLabel(QString("%1  %2 (%3%)")
                              .arg(trKey("Female"))
                              .arg(femaleCount)
                              .arg(QString::number(femalePct, 'f', 1)));
        sliceFemale->setLabelVisible(femaleCount > 0);
        sliceFemale->setLabelColor(Qt::white);
        sliceFemale->setLabelFont(chartFont);
        if (femaleCount > 0) sliceFemale->setExploded();
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
        while (q.next() && count < 5) {
            QString dom = q.value(0).toString().trimmed();
            int c = q.value(1).toInt();
            if (dom.isEmpty()) continue;
            dom = dom.split('.').first().toUpper();
            *domainSet << c;
            categories << dom;
            if (c > maxVal) maxVal = c;
            count++;
        }
    }

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
    chartViewBar->setMinimumWidth(220);
    chartViewBar->setStyleSheet("background: transparent; border: 2px solid #8B6F47; border-radius: 10px;");

    QHBoxLayout *hbox = qobject_cast<QHBoxLayout*>(ui_client->widget_chart->layout());
    hbox->addWidget(summaryBox,    1);
    hbox->addWidget(chartViewPie,  3);
    hbox->addWidget(chartViewBar,  3);

    // --- Connect Arduino LCD buttons ---
    connect(ui_client->btn_lcd_total_clients, &QPushButton::clicked, this, &MainWindow::onClientLCDTotalClients);
    connect(ui_client->btn_lcd_gender_dist,   &QPushButton::clicked, this, &MainWindow::onClientLCDGenderDist);
    connect(ui_client->btn_lcd_top_email,     &QPushButton::clicked, this, &MainWindow::onClientLCDTopEmail);
}

// =============================================================================
//  ARDUINO LCD HELPERS
// =============================================================================

// Finds the Arduino's COM port automatically, opens it at 9600 baud,
// then sends two lines of text using the protocol: "LCD:line1|line2\n"
// The Arduino sketch reads this string and prints each part to its LCD row.
void MainWindow::sendToArduinoLCD(const QString &line1, const QString &line2)
{
    // Create the serial port object once and reuse it
    if (!arduino) {
        arduino = new QSerialPort(this);
        connect(arduino, &QSerialPort::readyRead, this, &MainWindow::onArduinoReadyRead);
    }
    if (!arduino->isOpen()) {
        // Scan all available COM ports and pick the one that belongs to an Arduino
        const auto ports = QSerialPortInfo::availablePorts();
        QString portName;
        for (const QSerialPortInfo &info : ports) {
            if (info.description().contains("Arduino", Qt::CaseInsensitive) ||
                info.manufacturer().contains("Arduino", Qt::CaseInsensitive)) {
                portName = info.portName(); break;
            }
        }
        // Fallback: use the first available port if no Arduino label found
        if (portName.isEmpty() && !ports.isEmpty()) portName = ports.first().portName();
        if (portName.isEmpty()) {
            QMessageBox::warning(this, "Arduino", "No Arduino detected on any serial port.");
            return;
        }
        // Open the port at 9600 baud — must match the baud rate in the Arduino sketch
        arduino->setPortName(portName);
        arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        if (!arduino->open(QIODevice::ReadWrite)) {
            QMessageBox::critical(this, "Arduino", "Failed to open port: " + arduino->portName());
            return;
        }
    }
    // Trim each line to 16 chars (LCD width), build the message and send it
    QString l1 = line1.trimmed().left(16);
    QString l2 = line2.trimmed().left(16);
    QString msg = "LCD:" + l1 + "|" + l2 + "\n";
    arduino->write(msg.toUtf8());
}

// Button 1 — queries the total number of clients and sends it to the LCD
void MainWindow::onClientLCDTotalClients()
{
    QSqlQuery q;
    int total = 0;
    if (q.exec("SELECT COUNT(*) FROM CLIENTS") && q.next())
        total = q.value(0).toInt();
    sendToArduinoLCD("Clients:", QString::number(total));
}

// Button 2 — queries how many clients are male vs female and sends both counts
void MainWindow::onClientLCDGenderDist()
{
    QSqlQuery q;
    int male = 0, female = 0;
    if (q.exec("SELECT GENDER, COUNT(*) FROM CLIENTS GROUP BY GENDER")) {
        while (q.next()) {
            QString g = q.value(0).toString().trimmed();
            if (g.compare("Male",   Qt::CaseInsensitive) == 0) male   = q.value(1).toInt();
            if (g.compare("Female", Qt::CaseInsensitive) == 0) female = q.value(1).toInt();
        }
    }
    sendToArduinoLCD(QString("Male: %1").arg(male),
                     QString("Female: %1").arg(female));
}

// Button 3 — finds the most used email provider and sends its name and count
void MainWindow::onClientLCDTopEmail()
{
    QSqlQuery q;
    QString topDomain = "N/A";
    int topCount = 0;
    QString sql = "SELECT SUBSTR(EMAIL, INSTR(EMAIL, '@') + 1) AS DOMAIN, COUNT(*) AS C "
                  "FROM CLIENTS WHERE EMAIL LIKE '%@%' "
                  "GROUP BY SUBSTR(EMAIL, INSTR(EMAIL, '@') + 1) "
                  "ORDER BY C DESC";
    if (q.exec(sql) && q.next()) {
        topDomain = q.value(0).toString().split('.').first().toUpper().left(10);
        topCount  = q.value(1).toInt();
    }
    sendToArduinoLCD("Top Email:", QString("%1 (%2)").arg(topDomain).arg(topCount));
}

// =============================================================================
//  CLIENT CRUD — CLEAR FIELDS
// =============================================================================

void MainWindow::onClientClearFields()
{
    if (ui_client) {
        ui_client->le_nom->clear();
        ui_client->le_prenom->clear();
        ui_client->le_adresse->clear();
        ui_client->le_tel->clear();
        ui_client->le_email->clear();
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
        ui_client->rb_homme_mod->setAutoExclusive(false);
        ui_client->rb_femme_mod->setAutoExclusive(false);
        ui_client->rb_homme_mod->setChecked(false);
        ui_client->rb_femme_mod->setChecked(false);
        ui_client->rb_homme_mod->setAutoExclusive(true);
        ui_client->rb_femme_mod->setAutoExclusive(true);
    }
}

// =============================================================================
//  CLIENT MANAGEMENT SETUP
// =============================================================================

void MainWindow::setupClientManagement()
{
    setTabTextTr(ui_client->tabWidget, ui_client->tab_add, "Manage Clients");

    ui_client->group_modify->setParent(ui_client->tab_add);
    ui_client->group_add->move(20, 70);
    ui_client->group_modify->move(20, 70);
    ui_client->group_modify->setVisible(false);
    ui_client->group_add->setVisible(true);

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

    connect(rbAdd, &QRadioButton::toggled, [=](bool checked) {
        if (checked) {
            ui_client->group_add->setVisible(true);
            ui_client->group_modify->setVisible(false);
        }
    });

    connect(rbMod, &QRadioButton::toggled, [=](bool checked) {
        if (checked) {
            ui_client->group_add->setVisible(false);
            ui_client->group_modify->setVisible(true);
        }
    });

    int modifyTabIndex = ui_client->tabWidget->indexOf(ui_client->tab_modify);
    if (modifyTabIndex != -1) {
        ui_client->tabWidget->removeTab(modifyTabIndex);
    }

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

    QWidget *traceTab = new QWidget();
    ui_client->tabWidget->addTab(traceTab, "Cyber Trace");
    m_clientCyberTable = new QTableView(traceTab);
    m_clientCyberTable->setGeometry(20, 20, 1200, 660);
    m_clientCyberTable->setStyleSheet(
        "QTableView { background: rgba(0,0,0,0.6); gridline-color: #5A4A32; border: 1px solid #8B6F47; color: #D4AF37; font-family: 'Consolas'; }"
        "QHeaderView::section { background: rgba(139,111,71,0.3); border: 1px solid #8B6F47; color: #D4AF37; font-weight: bold; }"
        "QTableView::item:selected { background: rgba(139,111,71,0.5); border: 1px solid #D4AF37; }");

    QPushButton *btn_refresh_trace = new QPushButton("UPDATE LOG", traceTab);
    btn_refresh_trace->setGeometry(1070, 690, 150, 40);
    btn_refresh_trace->setStyleSheet(
        "QPushButton { background: rgba(139, 111, 71, 0.4); border: 1px solid #8B6F47; border-radius: 5px; color: #D4AF37; font-weight: bold; font-family: 'Consolas'; }"
        "QPushButton:hover { background: rgba(139, 111, 71, 0.8); border: 1px solid #D4AF37; }");

    QWidget *matrixTab = new QWidget();
    ui_client->tabWidget->addTab(matrixTab, "Data Matrix");
    m_clientMatrixFrame = new QFrame(matrixTab);
    m_clientMatrixFrame->setGeometry(100, 100, 1040, 550);
    m_clientMatrixFrame->setStyleSheet("background: rgba(10, 10, 10, 0.7); border: 2px solid #8B6F47; border-radius: 10px;");

    connect(btn_refresh_trace, &QPushButton::clicked, this, &MainWindow::onClientCyberTraceRefresh);

    setupClientDataMatrix();
}

// =============================================================================
//  CYBER TRACE
// =============================================================================

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
        return bt < at;
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

    QGraphicsOpacityEffect *traceEff = new QGraphicsOpacityEffect(this);
    m_clientCyberTable->setGraphicsEffect(traceEff);
    QPropertyAnimation *traceAnim = new QPropertyAnimation(traceEff, "opacity");
    traceAnim->setDuration(600);
    traceAnim->setStartValue(0.1);
    traceAnim->setEndValue(1.0);
    traceAnim->setEasingCurve(QEasingCurve::InBack);
    traceAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

// =============================================================================
//  DATA MATRIX
// =============================================================================

void MainWindow::setupClientDataMatrix()
{
    if (m_clientMatrixFrame->layout()) {
        QLayoutItem *item;
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
            total  = q.value(0).toInt();
            male   = q.value(1).toInt();
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

    for (int i = 1; i <= fullText.length(); ++i) {
        QTimer::singleShot(i * 25, statsLabel, [statsLabel, fullText, i]() {
            statsLabel->setText(fullText.left(i));
        });
    }

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    m_clientMatrixFrame->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(1200);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::InExpo);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

// =============================================================================
//  CLIENT CALENDAR
// =============================================================================

void MainWindow::setupClientCalendar()
{
    QWidget *calendarTab = ui_client->tab_calendar;
    for (QWidget *child : calendarTab->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
        child->hide();

    QHBoxLayout *mainLayout = new QHBoxLayout(calendarTab);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // ── LEFT: styled calendar frame ──
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
            background: #1C1610; border-radius: 14px 14px 0 0;
            padding: 8px 12px; min-height: 46px;
        }
        QCalendarWidget QToolButton {
            color: #D4AF37; font-size: 13px; font-weight: bold;
            background: transparent; border: none; padding: 6px 14px; border-radius: 8px;
        }
        QCalendarWidget QToolButton:hover { background: rgba(212,175,55,0.18); }
        QCalendarWidget QToolButton::menu-indicator { image: none; }
        QCalendarWidget QMenu {
            background: #2C2418; color: #D4AF37;
            border: 1px solid #8B6F47; border-radius: 6px;
        }
        QCalendarWidget QSpinBox {
            color: #D4AF37; background: transparent;
            font-size: 14px; font-weight: bold; border: none;
            selection-background-color: #8B6F47;
        }
        QCalendarWidget QAbstractItemView {
            background: transparent; color: #D8C9B0; font-size: 13px;
            selection-background-color: #8B6F47; selection-color: white;
            alternate-background-color: transparent; outline: none; gridline-color: transparent;
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

    // ── Fetch ALL date-bearing events from every module ──
    QMap<QString, QList<QPair<QString,QString>>> eventMap;

    // 1. Orders
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
    // 2. Clients
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(REGISTRATION_DATE,'YYYY-MM-DD'), "
                   "FIRST_NAME||' '||LAST_NAME FROM CLIENTS WHERE REGISTRATION_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"client", "Client joined:  " + q.value(1).toString()});
        }
    }
    // 3. Employees
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
    // 4. Equipment — purchase
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
    // 5. Equipment — maintenance
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(NEXT_MAINTENANCE,'YYYY-MM-DD'), EQUIPMENT_TYPE "
                   "FROM EQUIPMENT WHERE NEXT_MAINTENANCE IS NOT NULL AND STATUS != 'Retired'")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"maintenance", "Maintenance due:  " + q.value(1).toString()});
        }
    }
    // 6. Suppliers
    {
        QSqlQuery q;
        if (q.exec("SELECT TO_CHAR(REGISTRATION_DATE,'YYYY-MM-DD'), SUPPLIER_NAME "
                   "FROM SUPPLIERS WHERE REGISTRATION_DATE IS NOT NULL")) {
            while (q.next())
                eventMap[q.value(0).toString()].append(
                    {"supplier", "Supplier registered:  " + q.value(1).toString()});
        }
    }

    // Colour-code dates
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
            break;
        }
    }

    // Today highlight
    {
        QTextCharFormat todayFmt;
        todayFmt.setBackground(QColor("#8B6F47"));
        todayFmt.setForeground(QColor("#FFEFCF"));
        todayFmt.setFontWeight(QFont::Bold);
        calendar->setDateTextFormat(QDate::currentDate(), todayFmt);
    }

    // ── RIGHT: event detail panel ──
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

    mainLayout->addWidget(calFrame, 60);
    mainLayout->addWidget(rightPanel, 40);
}

// =============================================================================
//  CLIENT TABLE — REFRESH / SEARCH
// =============================================================================

static const QString kClientTableStyle =
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
    "QTableView::item { padding: 12px; border-bottom: 1px solid rgba(212, 175, 0, 0.03); }"
    "QScrollBar:vertical { background: rgba(15,12,8,0.85); width: 12px; border-radius: 6px; }"
    "QScrollBar::handle:vertical { background: #D4AF37; border-radius: 6px; min-height: 20px; }"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }"
    "QScrollBar:horizontal { background: rgba(15,12,8,0.85); height: 12px; border-radius: 6px; }"
    "QScrollBar::handle:horizontal { background: #D4AF37; border-radius: 6px; min-width: 20px; }"
    "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: none; background: none; }";

static void applyClientTableSetup(QTableView *tv)
{
    tv->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tv->horizontalHeader()->setMinimumSectionSize(120);
    tv->horizontalHeader()->setStretchLastSection(true);
    tv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    tv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    tv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);
    tv->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tv->verticalHeader()->setDefaultSectionSize(55);
    tv->setStyleSheet(kClientTableStyle);
}

void MainWindow::onClientRefreshView()
{
    QString orderBy = " ORDER BY CLIENT_ID ASC";
    if (ui_client->cb_sort_field && ui_client->btn_sort_order) {
        const bool asc = ui_client->btn_sort_order->text().contains(QStringLiteral("Ascending"));
        const QString dir = asc ? " ASC" : " DESC";
        switch (ui_client->cb_sort_field->currentIndex()) {
            case 1: orderBy = " ORDER BY UPPER(FIRST_NAME)" + dir + ", UPPER(LAST_NAME) ASC"; break;
            case 2: orderBy = " ORDER BY UPPER(LAST_NAME)"  + dir + ", UPPER(FIRST_NAME) ASC"; break;
            case 3: orderBy = " ORDER BY GENDER"            + dir + ", UPPER(LAST_NAME) ASC"; break;
            default: orderBy = " ORDER BY CLIENT_ID" + dir; break;
        }
    }
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(
        "SELECT CLIENT_ID AS \"ID\", "
        "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
        " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
        " FROM CLIENTS" + orderBy
    );
    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Database Error", "Failed to load clients:\n" + model->lastError().text());
        return;
    }
    ui_client->tableView->setModel(model);
    applyClientTableSetup(ui_client->tableView);

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_client->tableView);
    ui_client->tableView->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(600); a->setStartValue(0.0); a->setEndValue(1.0);
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

void MainWindow::onClientSearch()
{
    QString orderBy = " ORDER BY CLIENT_ID ASC";
    if (ui_client->cb_sort_field && ui_client->btn_sort_order) {
        const bool asc = ui_client->btn_sort_order->text().contains(QStringLiteral("Ascending"));
        const QString dir = asc ? " ASC" : " DESC";
        switch (ui_client->cb_sort_field->currentIndex()) {
            case 1: orderBy = " ORDER BY UPPER(FIRST_NAME)" + dir + ", UPPER(LAST_NAME) ASC"; break;
            case 2: orderBy = " ORDER BY UPPER(LAST_NAME)"  + dir + ", UPPER(FIRST_NAME) ASC"; break;
            case 3: orderBy = " ORDER BY GENDER"            + dir + ", UPPER(LAST_NAME) ASC"; break;
            default: orderBy = " ORDER BY CLIENT_ID" + dir; break;
        }
    }

    QString search = ui_client->le_recherche->text().trimmed();
    QSqlQueryModel *model = new QSqlQueryModel(this);
    if (search.isEmpty()) {
        model->setQuery(
            "SELECT CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS" + orderBy
        );
    } else {
        QSqlQuery q;
        q.prepare(
            "SELECT CLIENT_ID AS \"ID\", "
            "LAST_NAME AS \"Last Name\", FIRST_NAME AS \"First Name\","
            " ADDRESS AS \"Address\", PHONE_NUMBER AS \"Phone\", EMAIL AS \"Email\", GENDER AS \"Gender\""
            " FROM CLIENTS WHERE UPPER(LAST_NAME) LIKE :s OR UPPER(FIRST_NAME) LIKE :s"
            " OR UPPER(EMAIL) LIKE :s OR CAST(CLIENT_ID AS VARCHAR2(20)) LIKE :s"
            + orderBy
        );
        q.bindValue(":s", "%" + search.toUpper() + "%");
        q.exec();
        model->setQuery(std::move(q));
    }
    ui_client->tableView->setModel(model);
    applyClientTableSetup(ui_client->tableView);

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(ui_client->tableView);
    ui_client->tableView->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
    a->setDuration(400); a->setStartValue(0.0); a->setEndValue(1.0);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

// =============================================================================
//  CLIENT CRUD — ADD / MODIFY / DELETE / ROW SELECTED
// =============================================================================

void MainWindow::onClientAdd()
{
    int clientId = 1;
    QSqlQuery qMex("SELECT CLIENT_ID FROM CLIENTS ORDER BY CLIENT_ID ASC");
    while (qMex.next()) {
        if (qMex.value(0).toInt() == clientId) clientId++;
        else if (qMex.value(0).toInt() > clientId) break;
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
    QString gender = ui_client->rb_homme->isChecked() ? "Male" : "Female";

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

    QString gender = ui_client->rb_homme_mod->isChecked() ? "Male" : "Female";
    QSqlQuery q;
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

void MainWindow::onClientRowSelected(const QModelIndex &index)
{
    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(ui_client->tableView->model());
    if (!model) return;
    int row = index.row();

    ui_client->le_id_mod->setText(model->data(model->index(row, 0)).toString());
    ui_client->le_nom_mod->setText(model->data(model->index(row, 1)).toString());
    ui_client->le_prenom_mod->setText(model->data(model->index(row, 2)).toString());
    ui_client->le_adresse_mod->setText(model->data(model->index(row, 3)).toString());
    ui_client->le_tel_mod->setText(model->data(model->index(row, 4)).toString());
    ui_client->le_email_mod->setText(model->data(model->index(row, 5)).toString());
    QString gender = model->data(model->index(row, 6)).toString();
    if (gender == "Male")   ui_client->rb_homme_mod->setChecked(true);
    else if (gender == "Female") ui_client->rb_femme_mod->setChecked(true);
}

// =============================================================================
//  CLIENT EXPORT PDF
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
    printer.setPageSize(QPageSize(QSize(297, 210), QPageSize::Millimeter));
    printer.setOutputFileName(fileName);

    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::critical(this, "Export Error", "Failed to start PDF export.");
        return;
    }

    int pageWidth = printer.width();
    int y = 50;

    painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
    painter.setPen(QColor(139, 111, 71));
    painter.drawText(0, y, pageWidth, 40, Qt::AlignCenter, "Hammer Down - Client List");
    y += 60;

    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(Qt::black);

    int cols[]    = { 60, 150, 150, 250, 120, 200, 80 };
    QString headers[] = { "ID", "Last Name", "First Name", "Address", "Phone", "Email", "Gender" };

    int x = 20;
    for (int i = 0; i < 7; ++i) {
        painter.drawText(x, y, cols[i], 25, Qt::AlignLeft, headers[i]);
        x += cols[i];
    }
    painter.drawLine(20, y + 25, pageWidth - 20, y + 25);
    y += 40;

    painter.setFont(QFont("Segoe UI", 9));
    QSqlQuery q("SELECT CLIENT_ID, LAST_NAME, FIRST_NAME, ADDRESS, PHONE_NUMBER, EMAIL, GENDER FROM CLIENTS ORDER BY CLIENT_ID");
    while (q.next()) {
        if (y > printer.height() - 60) { printer.newPage(); y = 50; }
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

// =============================================================================
//  CLIENT MAIL TAB
// =============================================================================

void MainWindow::onClientSendMail()
{
    if (!ui_client) return;

    const QString to         = ui_client->le_to->text().trimmed();
    const QString subject    = ui_client->le_subject->text().trimmed();
    const QString attachment = ui_client->le_attachment->text().trimmed();
    const QString body       = ui_client->te_message->toPlainText().trimmed();

    if (to.isEmpty()) {
        QMessageBox::warning(this, "Email Error", "Please enter a recipient email address.");
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
        QMessageBox::warning(this, "Email Error", "Please enter a message.");
        ui_client->te_message->setFocus();
        return;
    }
    if (body.length() < 5) {
        QMessageBox::warning(this, "Email Error", "Message is too short — minimum 5 characters required.");
        ui_client->te_message->setFocus();
        return;
    }

    const QString host     = "smtp-relay.brevo.com";
    const quint16 port     = 587;
    const QString username = "rayenkabar780@gmail.com";
    const QString password = "xsmtpsib-87c2fb8b2fd4f260176840d024467dcabacccc643a60a7e5f3aa5394be562fdb-S9hVfpx8rkgB2MT6";

    ui_client->btn_send->setEnabled(false);
    ui_client->btn_send->setText("Sending...");

    QFutureWatcher<SmtpResult> *watcher = new QFutureWatcher<SmtpResult>(this);
    connect(watcher, &QFutureWatcher<SmtpResult>::finished, this, [this, watcher]() {
        const SmtpResult result = watcher->result();
        watcher->deleteLater();
        ui_client->btn_send->setEnabled(true);
        ui_client->btn_send->setText("Send");
        if (result.success) {
            QMessageBox::information(this, "Email Sent", "Your email was sent successfully.");
            ui_client->le_to->clear();
            ui_client->le_subject->clear();
            ui_client->le_attachment->clear();
            ui_client->te_message->clear();
        } else {
            QMessageBox::critical(this, "Email Failed", "Failed to send email:\n" + result.errorMessage);
        }
    });

    QFuture<SmtpResult> future = QtConcurrent::run([=]() {
        return SmtpSender::send(host, port, username, password,
                                to, subject, body, attachment);
    });
    watcher->setFuture(future);
}

void MainWindow::onClientBrowseMail()
{
    if (!ui_client) return;
    const QString path = QFileDialog::getOpenFileName(
        this, "Select Attachment", QDir::homePath(), "All Files (*)");
    if (!path.isEmpty())
        ui_client->le_attachment->setText(path);
}
