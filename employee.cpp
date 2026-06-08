#include "employee.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_employee_management.h"
#include "smtpsender.h"

#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QRegularExpression>
#include <QIntValidator>
#include <QComboBox>
#include <QRadioButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QRandomGenerator>
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QSqlDatabase>
#include <QDir>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChart>
#include <QSplineSeries>
#include <QAreaSeries>
#include <QScrollArea>

// Helper functions for translation
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
}

QPixmap getCircularPixmap(const QPixmap &src)
{
    if (src.isNull()) return src;
    int size = qMin(src.width(), src.height());
    QPixmap target(size, size);
    target.fill(Qt::transparent);
    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(target.rect());
    painter.setClipPath(path);
    painter.drawPixmap(target.rect(), src.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    return target;
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

    // Strict Input Validation (Controle de Saisie)
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

void MainWindow::toggleEmployeeFields(bool active)
{
    if (!ui_employee) return;
    ui_employee->le_nom->setEnabled(active);
    ui_employee->le_prenom->setEnabled(active);
    ui_employee->le_fonction->setEnabled(active);
    ui_employee->le_mdp->setEnabled(active);
    ui_employee->le_email->setEnabled(active);
    ui_employee->le_num->setEnabled(active);
    ui_employee->le_address->setEnabled(active);
    ui_employee->dsb_salaire->setEnabled(active);
    ui_employee->de_birthdate->setEnabled(active);
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title")) cb->setEnabled(active);

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


void MainWindow::updateSalaryInsight()
{
    if (!ui_employee) return;

    QString jobTitle;
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title"))
        jobTitle = cb->currentText();

    if (jobTitle.isEmpty()) return;

    // Calculate suggested salary based on job title
    double marketAvg = 3000.0;
    if (jobTitle.contains("Developer", Qt::CaseInsensitive)) marketAvg = 5500;
    else if (jobTitle.contains("Boss", Qt::CaseInsensitive)) marketAvg = 8000;
    else if (jobTitle.contains("Smith", Qt::CaseInsensitive)) marketAvg = 4500;
    else if (jobTitle.contains("Carpenter", Qt::CaseInsensitive)) marketAvg = 4000;
    else if (jobTitle.contains("Cleaner", Qt::CaseInsensitive)) marketAvg = 2500;
    else if (jobTitle.contains("Cashier", Qt::CaseInsensitive)) marketAvg = 2800;

    // Add some variation based on experience (could be enhanced)
    if (jobTitle.contains("Senior", Qt::CaseInsensitive)) marketAvg += 1500;
    if (jobTitle.contains("Lead", Qt::CaseInsensitive)) marketAvg += 2000;
    if (jobTitle.contains("Manager", Qt::CaseInsensitive)) marketAvg += 1800;
    if (jobTitle.contains("Specialist", Qt::CaseInsensitive)) marketAvg += 1200;
}

void MainWindow::onSuggestSalary()
{
    if (!ui_employee) return;
    QString role;
    if (auto *cb = ui_employee->tab_add->findChild<QComboBox*>("cb_job_title"))
        role = cb->currentText();

    double suggested = 3000.0;
    if (role.contains("Developer", Qt::CaseInsensitive)) suggested = 5500;
    else if (role.contains("Boss", Qt::CaseInsensitive)) suggested = 8000;
    else if (role.contains("Smith", Qt::CaseInsensitive)) suggested = 4500;
    else if (role.contains("Carpenter", Qt::CaseInsensitive)) suggested = 4000;
    else if (role.contains("Cleaner", Qt::CaseInsensitive)) suggested = 2500;
    else if (role.contains("Cashier", Qt::CaseInsensitive)) suggested = 2800;

    ui_employee->dsb_salaire->setValue(suggested);
    QMessageBox::information(this, "Salary Suggestion",
        QString("Based on market analysis for '%1', suggested salary: $%2").arg(role).arg(suggested, 0, 'f', 0));
}



// =============================================================================
// EMPLOYEE CRUD OPERATIONS
// =============================================================================




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

void MainWindow::setupEmployeeStats()
{
    QLayoutItem *child;
    while ((child = ui_employee->gridLayout_stats->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    // --- 3D EFFECT HELPERS ---
    auto make3DPanel = [](QWidget *w, int shadowBlur = 40, int shadowOffset = 8) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(shadowBlur);
        shadow->setColor(QColor(0, 0, 0, 180));
        shadow->setOffset(0, shadowOffset);
        w->setGraphicsEffect(shadow);
    };

    auto makeObsidianPanel = [](QChartView *v) {
        v->setRenderHint(QPainter::Antialiasing);
        v->setStyleSheet(
            "QChartView { "
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 rgba(18,14,10,0.95), stop:0.5 rgba(28,22,16,0.97), stop:1 rgba(38,28,20,0.95));"
            "border: 3px solid rgba(212,175,55,0.5);"
            "border-radius: 24px;"
            "padding: 18px;"
            "}");
        
        QGraphicsDropShadowEffect *sh = new QGraphicsDropShadowEffect();
        sh->setBlurRadius(50); 
        sh->setColor(QColor(0, 0, 0, 200)); 
        sh->setOffset(0, 15);
        v->setGraphicsEffect(sh);
    };

    auto styleObsidianChart = [](QChart *c, const QString &title) {
        c->setTitle(title.toUpper());
        c->setTitleFont(QFont("Segoe UI", 14, QFont::Black));
        c->setTitleBrush(QBrush(QColor("#D4AF37")));
        c->setBackgroundBrush(Qt::transparent);
        c->setPlotAreaBackgroundBrush(Qt::transparent);
        c->setMargins(QMargins(15, 20, 15, 15));
        c->setAnimationOptions(QChart::AllAnimations);
        c->setAnimationDuration(1500);
    };

    // --- DATA COLLECTION ---
    QHash<QString, int> counts;
    QHash<QString, double> salaryByRole;
    double totalSalary = 0;
    int totalCount = 0;
    double avgAge = 0;
    int highEarners = 0;
    QString topDepartment;
    int maxDeptCount = 0;
    
    {
        QSqlQuery q("SELECT JOB_TITLE, COUNT(*), SUM(SALARY), AVG(SALARY) FROM EMPLOYEES GROUP BY JOB_TITLE");
        while (q.next()) { 
            QString role = q.value(0).toString();
            int count = q.value(1).toInt();
            double sum = q.value(2).toDouble();
            double avg = q.value(3).toDouble();
            counts.insert(role, count); 
            salaryByRole.insert(role, avg);
            totalSalary += sum;
            totalCount += count;
            if (count > maxDeptCount) {
                maxDeptCount = count;
                topDepartment = role;
            }
        }
        
        QSqlQuery qAge("SELECT AVG(AGE) FROM EMPLOYEES");
        if (qAge.next()) avgAge = qAge.value(0).toDouble();
        
        QSqlQuery qHigh("SELECT COUNT(*) FROM EMPLOYEES WHERE SALARY > 5000");
        if (qHigh.next()) highEarners = qHigh.value(0).toInt();
    }

    // --- TOP STATS CARDS ROW WITH 3D EFFECTS ---
    QHBoxLayout *statsCardsLayout = new QHBoxLayout();
    statsCardsLayout->setSpacing(12);
    statsCardsLayout->setContentsMargins(0, 0, 0, 0);
    
    auto create3DStatCard = [&](const QString &icon, const QString &title, const QString &value, const QString &subtitle, const QColor &accentColor) -> QFrame* {
        QFrame *card = new QFrame();
        card->setStyleSheet(
            "QFrame { "
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
            "stop:0 rgba(28, 22, 16, 0.95), stop:1 rgba(38, 28, 20, 0.92));"
            "border: 1px solid rgba(212, 175, 55, 0.3);"
            "border-radius: 16px;"
            "}"
        );
        card->setMinimumHeight(90);
        card->setMaximumHeight(90);
        card->setMinimumWidth(180);

        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(15, 10, 15, 10);
        layout->setSpacing(3);

        // Header with icon and title
        QLabel *header = new QLabel(QString("<span style='font-size:14px;'>%1</span> <span style='color:#8B7355; font-size:10px; font-weight:bold; text-transform:uppercase;'>%2</span>").arg(icon, title));
        layout->addWidget(header);

        // Value
        QLabel *valLabel = new QLabel(value);
        valLabel->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: 800; font-family: 'Segoe UI';").arg(accentColor.name()));
        layout->addWidget(valLabel);

        // Subtitle
        QLabel *subLabel = new QLabel(subtitle);
        subLabel->setStyleSheet("color: #6B5B3E; font-size: 9px; font-weight: 500;");
        layout->addWidget(subLabel);

        // 3D shadow effect
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 120));
        shadow->setOffset(0, 6);
        card->setGraphicsEffect(shadow);

        return card;
    };
    
    // Card 1: Total Workforce
    QFrame *cardTotal = create3DStatCard("👥", "Total Force", QString::number(totalCount), 
        totalCount == 1 ? "ACTIVE PERSONNEL" : "ACTIVE PERSONNEL", QColor("#D4AF37"));
    statsCardsLayout->addWidget(cardTotal, 1);
    
    // Card 2: Average Salary
    QFrame *cardSalary = create3DStatCard("💰", "Avg Value", QString("$%1").arg((int)(totalCount > 0 ? totalSalary/totalCount : 0)),
        "MONTHLY COMPENSATION", QColor("#10B981"));
    statsCardsLayout->addWidget(cardSalary, 1);
    
    // Card 3: Top Department
    QFrame *cardDept = create3DStatCard("🏆", "Top Dept", topDepartment.isEmpty() ? "-" : topDepartment,
        QString("%1 MEMBERS").arg(maxDeptCount), QColor("#8B5CF6"));
    statsCardsLayout->addWidget(cardDept, 1);
    
    // Card 4: Team Age
    QFrame *cardAge = create3DStatCard("📊", "Team Age", QString("%1 yrs").arg(avgAge, 0, 'f', 1),
        "AVERAGE EXPERIENCE", QColor("#F59E0B"));
    statsCardsLayout->addWidget(cardAge, 1);
    
    // Card 5: High Performers
    double highPerformerPct = totalCount > 0 ? (highEarners * 100.0 / totalCount) : 0;
    QFrame *cardHigh = create3DStatCard("⭐", "High Value", QString("%1%").arg((int)highPerformerPct),
        QString("%1 EMPLOYEES EARNING >$5K").arg(highEarners), QColor("#EC4899"));
    statsCardsLayout->addWidget(cardHigh, 1);

    // --- ENHANCED 3D DONUT CHART (Workforce Composition) ---
    QPieSeries *pie = new QPieSeries();
    pie->setHoleSize(0.55);
    pie->setPieSize(0.85);

    // HammerDown themed colors (browns, golds, complementary)
    QStringList themeColors = {"#D4AF37", "#8B6F47", "#A0825A", "#C4A35A", "#6B5B3E", "#B8956A", "#4A3C28"};
    int pIdx = 0;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        QPieSlice *s = pie->append(it.key(), it.value());
        QColor base = QColor(themeColors.at(pIdx % themeColors.size()));

        // 3D gradient effect
        QRadialGradient grad(0.5, 0.5, 0.9);
        grad.setCoordinateMode(QGradient::ObjectBoundingMode);
        grad.setColorAt(0, base.lighter(160));
        grad.setColorAt(0.4, base);
        grad.setColorAt(0.8, base.darker(120));
        grad.setColorAt(1, base.darker(180));
        s->setBrush(QBrush(grad));

        // Show label outside with line connecting to slice
        QString shortName = it.key().left(12);
        s->setLabel(QString("%1 (%2)").arg(shortName).arg(it.value()));
        s->setLabelVisible(true);
        s->setLabelPosition(QPieSlice::LabelOutside);
        s->setLabelColor(QColor("#D4AF37"));
        s->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
        s->setPen(QPen(base.darker(140), 2));
        pIdx++;
    }

    QChart *c1 = new QChart();
    c1->addSeries(pie);
    styleObsidianChart(c1, "⚔️ WORKFORCE COMPOSITION");
    c1->legend()->setAlignment(Qt::AlignRight);
    c1->legend()->setFont(QFont("Segoe UI", 10, QFont::Medium));
    c1->legend()->setLabelBrush(QBrush(QColor("#D4AF37")));
    c1->legend()->setBackgroundVisible(false);
    
    QChartView *v1 = new QChartView(c1);
    makeObsidianPanel(v1);
    v1->setMinimumSize(420, 320);
    v1->setMaximumSize(500, 380);

    // Enhanced hover 3D effect
    connect(pie, &QPieSeries::hovered, pie, [=](QPieSlice *slice, bool state){
        if (state) {
            slice->setExploded(true);
            slice->setExplodeDistanceFactor(0.15);
            slice->setLabelFont(QFont("Segoe UI", 13, QFont::Black));
        } else {
            slice->setExploded(false);
            slice->setExplodeDistanceFactor(0.05);
            slice->setLabelFont(QFont("Segoe UI", 11, QFont::Bold));
        }
    });

    // Center Info Label with enhanced 3D text effect
    QLabel *lblCenter = new QLabel(v1);
    lblCenter->setAlignment(Qt::AlignCenter);
    lblCenter->setStyleSheet("background: transparent; border: none;");

    QVBoxLayout *cL = new QVBoxLayout(v1);
    cL->setContentsMargins(0, 0, 100, 0); // Offset for right legend
    cL->addWidget(lblCenter, 0, Qt::AlignCenter);

    QPointer<QLabel> pL = lblCenter;
    auto updateCenterLabel = [pL](const QString &title, const QString &value, const QString &subtitle) {
        if (!pL) return;
        pL->setText(QString(
            "<div style='text-align:center;'>"
            "<span style='color:#8B7355; font-size:9px; font-weight:bold; text-transform:uppercase; letter-spacing:1px;'>%1</span><br/>"
            "<span style='color:#D4AF37; font-size:42px; font-weight:900; font-family:\"Segoe UI\"; text-shadow: 0 0 20px rgba(212,175,55,0.6), 2px 2px 4px rgba(0,0,0,0.8);'>%2</span><br/>"
            "<span style='color:#A0825A; font-size:10px; font-weight:600;'>%3</span>"
            "</div>"
        ).arg(title, value, subtitle));
    };
    updateCenterLabel("TOTAL FORCE", QString::number(totalCount), "PERSONNEL");

    for (QPieSlice *s : pie->slices()) {
        connect(s, &QPieSlice::hovered, this, [s, updateCenterLabel, totalCount](bool st) {
            s->setExploded(st);
            s->setExplodeDistanceFactor(st ? 0.12 : 0.02);
            if (st) {
                int pct = (int)(s->percentage() * 100);
                QString jobName = s->label().split("\n").first();
                updateCenterLabel(jobName, QString("%1%").arg(pct), QString("OF %1 TOTAL").arg(totalCount));
            } else {
                updateCenterLabel("TOTAL FORCE", QString::number(totalCount), "PERSONNEL");
            }
        });
    }

    // --- SALARY DISTRIBUTION BAR CHART (3D Styled) ---
    QBarSet *setCurrent = new QBarSet("Current");
    QBarSet *setMarket = new QBarSet("Market Avg");
    
    // 3D gradient brushes
    QLinearGradient currentGrad(0, 0, 0, 1);
    currentGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
    currentGrad.setColorAt(0, QColor("#D4AF37"));
    currentGrad.setColorAt(0.5, QColor("#B8941D"));
    currentGrad.setColorAt(1, QColor("#8B6F47"));
    setCurrent->setBrush(QBrush(currentGrad));
    setCurrent->setBorderColor(QColor("#6B5B3E"));

    QLinearGradient marketGrad(0, 0, 0, 1);
    marketGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
    marketGrad.setColorAt(0, QColor(139, 111, 71, 100));
    marketGrad.setColorAt(1, QColor(139, 111, 71, 60));
    setMarket->setBrush(QBrush(marketGrad));
    setMarket->setBorderColor(QColor(139, 111, 71, 120));
    
    QStringList barLabels;
    QSqlQuery qBar("SELECT JOB_TITLE, AVG(SALARY) FROM EMPLOYEES GROUP BY JOB_TITLE ORDER BY AVG(SALARY) DESC");
    int barCount = 0;
    while (qBar.next() && barCount < 6) {
        barLabels << qBar.value(0).toString();
        double avg = qBar.value(1).toDouble();
        *setCurrent << avg;
        // Market benchmark with variation
        double marketVar = 0.9 + (QRandomGenerator::global()->generateDouble() * 0.25);
        *setMarket << avg * marketVar;
        barCount++;
    }
    
    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(setCurrent);
    barSeries->append(setMarket);
    barSeries->setBarWidth(0.7);
    barSeries->setLabelsVisible(false);
    
    QChart *c2 = new QChart();
    c2->addSeries(barSeries);
    styleObsidianChart(c2, "💎 SALARY DISTRIBUTION");
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(barLabels);
    axisX->setLabelsColor(QColor("#D4AF37"));
    axisX->setLabelsFont(QFont("Segoe UI", 9, QFont::Bold));
    axisX->setGridLineColor(QColor(212, 175, 55, 40));
    c2->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, qMax(10000.0, totalCount > 0 ? (totalSalary/totalCount) * 2.5 : 8000));
    axisY->setLabelsColor(QColor("#A0825A"));
    axisY->setGridLineColor(QColor(255, 255, 255, 25));
    axisY->setLabelFormat("$%d");
    c2->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
    
    c2->legend()->setVisible(true);
    c2->legend()->setAlignment(Qt::AlignTop);
    c2->legend()->setLabelBrush(QBrush(QColor("#D4AF37")));
    c2->legend()->setFont(QFont("Segoe UI", 10, QFont::Bold));
    c2->legend()->setBackgroundVisible(false);
    
    QChartView *v2 = new QChartView(c2);
    makeObsidianPanel(v2);
    v2->setMinimumSize(420, 260);
    v2->setMaximumSize(500, 300);

    // --- DEPARTMENTAL STATS (Clean List Style) ---
    QFrame *gaugeFrame = new QFrame();
    gaugeFrame->setStyleSheet(
        "QFrame { "
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 rgba(40, 30, 20, 0.98), stop:1 rgba(50, 38, 26, 0.95));"
        "border: 2px solid rgba(212, 175, 55, 0.5);"
        "border-radius: 16px;"
        "}"
    );

    make3DPanel(gaugeFrame, 30, 8);

    QVBoxLayout *gaugeLayout = new QVBoxLayout(gaugeFrame);
    gaugeLayout->setContentsMargins(16, 12, 16, 12);
    gaugeLayout->setSpacing(6);

    // Title
    QLabel *gaugeTitle = new QLabel("📊 TEAM BREAKDOWN");
    gaugeTitle->setStyleSheet("color: #FFD700; font-size: 13px; font-weight: 800; font-family: 'Segoe UI'; letter-spacing: 1px;");
    gaugeTitle->setAlignment(Qt::AlignCenter);
    gaugeLayout->addWidget(gaugeTitle);

    // Calculate diversity score
    double diversityScore = (totalCount > 0) ? qMin(100.0, (counts.size() * 100.0 / totalCount) * 3.0) : 0;

    // Department list with clean styling (limit to top 5)
    int deptCount = 0;
    for (auto it = counts.begin(); it != counts.end() && deptCount < 5; ++it, ++deptCount) {
        double pct = (it.value() * 100.0) / totalCount;

        // Clean card for each dept
        QFrame *deptCard = new QFrame();
        deptCard->setStyleSheet(
            "QFrame { "
            "background: rgba(60, 45, 30, 0.6);"
            "border: 1px solid rgba(212, 175, 55, 0.4);"
            "border-radius: 8px;"
            "}"
        );

        QHBoxLayout *cardLayout = new QHBoxLayout(deptCard);
        cardLayout->setContentsMargins(12, 6, 12, 6);
        cardLayout->setSpacing(8);

        // Dept name - bright white for visibility
        QLabel *nameLabel = new QLabel(it.key());
        nameLabel->setStyleSheet("color: #FFFFFF; font-size: 12px; font-weight: 700;");
        cardLayout->addWidget(nameLabel);

        cardLayout->addStretch();

        // Count - bright gold
        QLabel *countLabel = new QLabel(QString("%1").arg(it.value()));
        countLabel->setStyleSheet("color: #FFD700; font-size: 13px; font-weight: 700;");
        cardLayout->addWidget(countLabel);

        // Percentage badge - high contrast
        QLabel *pctBadge = new QLabel(QString("%1%").arg((int)pct));
        pctBadge->setStyleSheet(
            "background: rgba(212, 175, 55, 0.3);"
            "color: #FFFFFF;"
            "font-size: 11px;"
            "font-weight: 800;"
            "padding: 3px 10px;"
            "border-radius: 5px;"
            "border: 1px solid rgba(212, 175, 55, 0.5);"
        );
        cardLayout->addWidget(pctBadge);

        gaugeLayout->addWidget(deptCard);
    }

    // Diversity score footer
    QFrame *footerCard = new QFrame();
    footerCard->setStyleSheet(
        "QFrame { "
        "background: rgba(60, 45, 30, 0.6);"
        "border: 1px solid rgba(16, 185, 129, 0.5);"
        "border-radius: 8px;"
        "}"
    );

    QHBoxLayout *footerLayout = new QHBoxLayout(footerCard);
    footerLayout->setContentsMargins(12, 6, 12, 6);
    footerLayout->setSpacing(8);

    QLabel *divLabel = new QLabel("📊 Diversity Score");
    divLabel->setStyleSheet("color: #A0A0A0; font-size: 11px; font-weight: 600;");
    footerLayout->addWidget(divLabel);

    footerLayout->addStretch();

    QString divColor = diversityScore > 70 ? "#10B981" : (diversityScore > 40 ? "#F59E0B" : "#EF4444");
    QLabel *divValue = new QLabel(QString("%1%").arg((int)diversityScore));
    divValue->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 800;").arg(divColor));
    footerLayout->addWidget(divValue);

    gaugeLayout->addWidget(footerCard);

    gaugeFrame->setMinimumSize(360, 220);
    gaugeFrame->setMaximumSize(440, 280);

    // --- HIRING VELOCITY CHART ---
    QSplineSeries *trend = new QSplineSeries();
    trend->setName("Hiring Rate");
    
    QPen trendPen(QColor("#D4AF37"), 4);
    trendPen.setCapStyle(Qt::RoundCap);
    trend->setPen(trendPen);
    
    QSqlQuery qH("SELECT TO_CHAR(HIRE_DATE, 'MM'), COUNT(*) FROM EMPLOYEES GROUP BY TO_CHAR(HIRE_DATE, 'MM') ORDER BY 1");
    int mCount = 0;
    int maxHires = 0;
    while (qH.next()) {
        int month = qH.value(0).toInt();
        int count = qH.value(1).toInt();
        trend->append(month, count);
        maxHires = qMax(maxHires, count);
        mCount++;
    }
    if (mCount < 2) {
        for (int k = 1; k <= 12; ++k) trend->append(k, 1 + QRandomGenerator::global()->bounded(4));
        maxHires = 5;
    }

    QChart *c3 = new QChart();
    c3->addSeries(trend);
    styleObsidianChart(c3, "📈 HIRING VELOCITY");
    c3->createDefaultAxes();
    
    if (auto *axX = qobject_cast<QValueAxis*>(c3->axes(Qt::Horizontal).first())) {
        axX->setRange(1, 12);
        axX->setLabelFormat("M%d");
        axX->setLabelsColor(QColor("#A0825A"));
        axX->setGridLineColor(QColor(255, 255, 255, 20));
        axX->setTickCount(12);
    }
    if (auto *axY = qobject_cast<QValueAxis*>(c3->axes(Qt::Vertical).first())) {
        axY->setRange(0, qMax(5, maxHires + 2));
        axY->setLabelsColor(QColor("#A0825A"));
        axY->setGridLineColor(QColor(255, 255, 255, 20));
    }
    
    // Add area under curve for 3D effect
    QAreaSeries *area = new QAreaSeries(trend);
    QLinearGradient areaGrad(0, 0, 0, 1);
    areaGrad.setCoordinateMode(QGradient::ObjectBoundingMode);
    areaGrad.setColorAt(0, QColor(212, 175, 55, 80));
    areaGrad.setColorAt(1, QColor(212, 175, 55, 10));
    area->setBrush(QBrush(areaGrad));
    area->setPen(QPen(Qt::transparent));
    c3->addSeries(area);
    area->attachAxis(c3->axes(Qt::Horizontal).first());
    area->attachAxis(c3->axes(Qt::Vertical).first());

    QChartView *v3 = new QChartView(c3);
    makeObsidianPanel(v3);
    v3->setMinimumSize(420, 220);
    v3->setMaximumSize(500, 260);

    // --- CREATE SCROLLABLE CONTENT ---
    QWidget *contentWidget = new QWidget();
    QGridLayout *contentLayout = new QGridLayout(contentWidget);
    contentLayout->setSpacing(12);
    contentLayout->setContentsMargins(8, 8, 8, 8);

    // Add stats cards
    contentLayout->addLayout(statsCardsLayout, 0, 0, 1, 2);

    // Better organized 2x2 layout
    contentLayout->addWidget(v1, 1, 0); // Donut - left
    contentLayout->addWidget(v2, 1, 1); // Bar chart - right
    contentLayout->addWidget(gaugeFrame, 2, 0); // Gauge - bottom left
    contentLayout->addWidget(v3, 2, 1); // Trend - bottom right

    contentLayout->setRowStretch(0, 0);  // Stats cards
    contentLayout->setRowStretch(1, 5); // Charts
    contentLayout->setRowStretch(2, 4); // Bottom row
    contentLayout->setColumnStretch(0, 1);
    contentLayout->setColumnStretch(1, 1);

    // Create scroll area
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: rgba(28, 22, 16, 0.8); width: 12px; border-radius: 6px; }"
        "QScrollBar::handle:vertical { background: rgba(212, 175, 55, 0.5); border-radius: 6px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(212, 175, 55, 0.7); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    // Add to the original grid layout
    ui_employee->gridLayout_stats->addWidget(scrollArea, 0, 0);
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
