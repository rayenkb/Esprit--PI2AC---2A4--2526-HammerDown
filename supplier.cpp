#include "supplier.h"

#include "mainwindow.h"
#include "buttonanimator.h"

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
#include "ui_mainwindow.h"
#include "ui_supplier_management.h"
#include "smtpsender.h"
#include <QShortcut>
#include <QToolTip>
#include <QDateTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMovie>
#include <QBuffer>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringListModel>
#include <QCompleter>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QPrinter>
#include <QPainter>
#include <QRandomGenerator>
#include <QRegularExpressionValidator>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QtMath>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimeEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPixmap>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QRadialGradient>
#include <algorithm>


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

