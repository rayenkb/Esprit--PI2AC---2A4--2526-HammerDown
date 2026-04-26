#include "order.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>

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

static QPixmap generateQrPixmap(const QString &text, int pixelSize = 8, int border = 4)
{
    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(text.toUtf8().constData(), QrCode::Ecc::MEDIUM);
    const int qrSize = qr.getSize();
    const int qrPixelSize = (qrSize + border * 2) * pixelSize;
    const int framePadding = qMax(6, pixelSize * 2);
    const int imgSize = qrPixelSize + framePadding * 2;
    const QColor bgColor(245, 236, 219);
    const QColor moduleColor(34, 29, 22);
    const QColor frameColor(139, 111, 71);
    QImage img(imgSize, imgSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(bgColor);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(frameColor, qMax(2, pixelSize / 2)));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(framePadding / 2, framePadding / 2, imgSize - framePadding, imgSize - framePadding, framePadding * 0.55, framePadding * 0.55);
    painter.end();
    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {
            if (qr.getModule(x, y)) {
                for (int dy = 0; dy < pixelSize; dy++) {
                    for (int dx = 0; dx < pixelSize; dx++) {
                        img.setPixelColor(framePadding + (x + border) * pixelSize + dx, framePadding + (y + border) * pixelSize + dy, moduleColor);
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
#include "ui_order_management.h"
#include "qrcodegen.h"
#include <QShortcut>
#include <QToolTip>
#include <QDateTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
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
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTabWidget>
#include <QPixmap>
#include <QComboBox>
#include <QSpinBox>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QProgressBar>
#include <algorithm>


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

