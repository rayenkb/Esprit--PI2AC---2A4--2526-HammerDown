#include "chatbotdialog.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QRandomGenerator>
#include <QVector>
#include <QSet>
#include <QtGlobal>
#include <QRegularExpression>
#include <QPixmap>
#include <QSslConfiguration>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>

ChatBotDialog::ChatBotDialog(QWidget *parent, bool isWeatherBot)
    : QDialog(parent),
      networkManager(new QNetworkAccessManager(this)),
      m_isWeatherBot(isWeatherBot)
{
    setupUI();

    // OpenRouter API key
    apiKey = "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";
    imageApiUrl = "";
    imageModel = "";
    retryCount = 0;
    rateLimitRetries = 0;

    // Fallback model list — prefer OpenRouter auto-routing first.
    modelList << "openrouter/auto"
              << "meta-llama/llama-3.1-8b-instruct:free"
              << "mistralai/mistral-7b-instruct:free"
              << "google/gemma-2-9b-it:free";

    // Seed conversation with system prompt
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = buildSystemPrompt();
    conversationHistory.append(systemMsg);

    connect(networkManager, &QNetworkAccessManager::finished, this, &ChatBotDialog::onApiReplyFinished);

    if (m_isWeatherBot) {
        // Fetch real-time weather to inject into AI context (Tunis, Tunisia)
        // Using user-provided key: 002875fe9691ae3bfd2862f1586f91ca
        QUrl weatherUrl("https://api.openweathermap.org/data/3.0/onecall?lat=36.8065&lon=10.1815&units=metric&appid=002875fe9691ae3bfd2862f1586f91ca");
        networkManager->get(QNetworkRequest(weatherUrl));
    }
}

ChatBotDialog::~ChatBotDialog()
{
}

void ChatBotDialog::setupUI()
{
    setWindowTitle("HammerDown Assistant");
    setMinimumSize(360, 480);
    resize(420, 580);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QFrame *mainFrame = new QFrame(this);
    mainFrame->setObjectName("chatMainFrame");

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(mainFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Title Bar
    titleBar = new QFrame(mainFrame);
    titleBar->setObjectName("chatTitleBar");
    titleBar->setFixedHeight(52);
    titleBar->installEventFilter(this);

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(16, 0, 12, 0);

    QLabel *botIcon = new QLabel(titleBar);
    if (m_isWeatherBot) {
        botIcon->setPixmap(QPixmap(":/assets/meteorology.png").scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        botIcon->setText("\xF0\x9F\xA4\x96");
        botIcon->setStyleSheet("font-size: 22px; background: transparent; border: none;");
    }

    QLabel *titleLabel = new QLabel(m_isWeatherBot ? "Weather AI Expert" : "HammerDown Assistant", titleBar);
    titleLabel->setObjectName("chatTitle");

    QPushButton *closeBtn = new QPushButton(titleBar);
    closeBtn->setObjectName("chatCloseBtn");
    closeBtn->setFixedSize(38, 38);
    closeBtn->setAutoDefault(false);
    closeBtn->setDefault(false);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip("Close chat");
    closeBtn->setText("✕");
    closeBtn->setStyleSheet("background-color: #d4a96a; color: #1a1208; border: none; border-radius: 19px; font-size: 22px; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

    titleLayout->addWidget(botIcon);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);

    sizeGrip = new QSizeGrip(mainFrame);
    sizeGrip->setObjectName("chatSizeGrip");
    sizeGrip->setFixedSize(16, 16);

    // Chat Area
    scrollArea = new QScrollArea(mainFrame);
    scrollArea->setObjectName("chatScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    chatContainer = new QWidget();
    chatContainer->setObjectName("chatContainer");
    chatLayout = new QVBoxLayout(chatContainer);
    chatLayout->setContentsMargins(12, 12, 12, 12);
    chatLayout->setSpacing(8);
    chatLayout->addStretch();
    scrollArea->setWidget(chatContainer);

    // Typing Indicator
    typingIndicator = new QLabel("Assistant is typing...", mainFrame);
    typingIndicator->setObjectName("typingIndicator");
    typingIndicator->setVisible(false);

    // Input Area
    QFrame *inputFrame = new QFrame(mainFrame);
    inputFrame->setObjectName("chatInputFrame");
    inputFrame->setFixedHeight(56);

    QHBoxLayout *inputLayout = new QHBoxLayout(inputFrame);
    inputLayout->setContentsMargins(12, 8, 12, 8);
    inputLayout->setSpacing(8);

    inputField = new QLineEdit(inputFrame);
    inputField->setObjectName("chatInput");
    inputField->setPlaceholderText("Type a message...");
    connect(inputField, &QLineEdit::returnPressed, this, &ChatBotDialog::sendMessage);

    sendButton = new QPushButton("\xE2\x9E\xA4", inputFrame);
    sendButton->setObjectName("chatSendBtn");
    sendButton->setFixedSize(38, 38);
    sendButton->setAutoDefault(false);
    sendButton->setDefault(false);
    sendButton->setCursor(Qt::PointingHandCursor);
    connect(sendButton, &QPushButton::clicked, this, &ChatBotDialog::sendMessage);

    inputLayout->addWidget(inputField);
    inputLayout->addWidget(sendButton);

    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(scrollArea, 1);
    mainLayout->addWidget(typingIndicator);
    mainLayout->addWidget(inputFrame);

    mainFrame->setStyleSheet(R"(
        #chatMainFrame {
            background-color: #1a1208;
            border: 2px solid #8B6F47;
            border-radius: 14px;
        }
        #chatTitleBar {
            background-color: #2a1e10;
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
            border-bottom: 1px solid #3d2e18;
        }
        #chatTitle {
            color: #d4a96a;
            font-size: 15px;
            font-weight: 700;
            background: transparent;
            border: none;
        }
        #chatCloseBtn {
            background-color: transparent;
            color: #8B6F47;
            border: none;
            border-radius: 15px;
            font-size: 16px;
            font-weight: bold;
        }
        #chatCloseBtn:hover {
            background-color: rgba(217, 83, 79, 0.3);
            color: #ff6b6b;
        }
        #chatScrollArea {
            background-color: transparent;
            border: none;
        }
        #chatContainer {
            background-color: transparent;
        }
        #typingIndicator {
            color: #8B6F47;
            font-size: 12px;
            font-style: italic;
            padding: 4px 16px;
            background: transparent;
        }
        #chatInputFrame {
            background-color: #2a1e10;
            border-top: 1px solid #3d2e18;
            border-bottom-left-radius: 12px;
            border-bottom-right-radius: 12px;
        }
        #chatInput {
            background-color: #1a1208;
            color: #e8dcc8;
            border: 1px solid #3d2e18;
            border-radius: 18px;
            padding: 6px 14px;
            font-size: 13px;
        }
        #chatInput:focus {
            border: 1px solid #8B6F47;
        }
        #chatSendBtn {
            background-color: #8B6F47;
            color: #ffffff;
            border: none;
            border-radius: 19px;
            font-size: 16px;
            font-weight: bold;
        }
        #chatSendBtn:hover {
            background-color: #a3845a;
        }
        #chatSendBtn:pressed {
            background-color: #6b5535;
        }
        QScrollBar:vertical {
            border: none;
            background: #1a1208;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #3d2e18;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #8B6F47;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    if (m_isWeatherBot) {
        appendMessage("MeteoBot", "Bonjour! I'm your Weather & Carpentry Expert. I can advise you on how today's weather in Tunisia will affect your wood, glue, and workshop safety. Ask me anything!", false);
    } else {
        appendMessage("Assistant", "Welcome to HammerDown!\nI'm your AI assistant. Ask me anything about this app — employees, clients, orders, equipment, suppliers, or any feature!", false);
    }
}

bool ChatBotDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragStartPos = mouseEvent->globalPosition().toPoint();
                m_windowStartPos = frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            if (m_dragging) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                QPoint delta = mouseEvent->globalPosition().toPoint() - m_dragStartPos;
                move(m_windowStartPos + delta);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
        }
    }

    return QDialog::eventFilter(watched, event);
}

void ChatBotDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (sizeGrip) {
        const int margin = 6;
        sizeGrip->move(width() - sizeGrip->width() - margin,
                       height() - sizeGrip->height() - margin);
        sizeGrip->raise();
    }
}

void ChatBotDialog::appendMessage(const QString &sender, const QString &text, bool isUser)
{
    QFrame *bubble = new QFrame(chatContainer);
    bubble->setObjectName(isUser ? "userBubble" : "botBubble");

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(2);

    QLabel *senderLabel = new QLabel(sender, bubble);
    senderLabel->setObjectName("senderLabel");

    QLabel *messageLabel = new QLabel(text, bubble);
    messageLabel->setObjectName("messageText");
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    bubbleLayout->addWidget(senderLabel);
    bubbleLayout->addWidget(messageLabel);

    if (isUser) {
        bubble->setStyleSheet(R"(
            #userBubble {
                background-color: #8B6F47;
                border-radius: 12px;
                border-bottom-right-radius: 4px;
                margin-left: 60px;
            }
            #senderLabel { color: #f5e6cc; font-size: 11px; font-weight: 600; background: transparent; border: none; }
            #messageText { color: #ffffff; font-size: 13px; background: transparent; border: none; }
        )");
    } else {
        bubble->setStyleSheet(R"(
            #botBubble {
                background-color: #2a1e10;
                border: 1px solid #3d2e18;
                border-radius: 12px;
                border-bottom-left-radius: 4px;
                margin-right: 60px;
            }
            #senderLabel { color: #d4a96a; font-size: 11px; font-weight: 600; background: transparent; border: none; }
            #messageText { color: #e8dcc8; font-size: 13px; background: transparent; border: none; }
        )");
    }

    chatLayout->insertWidget(chatLayout->count() - 1, bubble);

    QTimer::singleShot(50, this, [this]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    });
}

void ChatBotDialog::appendImageMessage(const QString &sender, const QPixmap &pixmap, const QString &caption, bool isUser)
{
    QFrame *bubble = new QFrame(chatContainer);
    bubble->setObjectName(isUser ? "userBubble" : "botBubble");

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(6);

    QLabel *senderLabel = new QLabel(sender, bubble);
    senderLabel->setObjectName("senderLabel");

    QLabel *imageLabel = new QLabel(bubble);
    imageLabel->setObjectName("imageBubble");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setScaledContents(false);

    int maxWidth = 240;
    QPixmap scaled = pixmap.scaledToWidth(maxWidth, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaled);
    imageLabel->setFixedSize(scaled.size());

    QLabel *captionLabel = new QLabel(caption, bubble);
    captionLabel->setObjectName("messageText");
    captionLabel->setWordWrap(true);
    captionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    bubbleLayout->addWidget(senderLabel);
    bubbleLayout->addWidget(imageLabel);
    bubbleLayout->addWidget(captionLabel);

    if (isUser) {
        bubble->setStyleSheet(R"(
            #userBubble {
                background-color: #8B6F47;
                border-radius: 12px;
                border-bottom-right-radius: 4px;
                margin-left: 60px;
            }
            #senderLabel { color: #f5e6cc; font-size: 11px; font-weight: 600; background: transparent; border: none; }
            #messageText { color: #ffffff; font-size: 12px; background: transparent; border: none; }
        )");
    } else {
        bubble->setStyleSheet(R"(
            #botBubble {
                background-color: #2a1e10;
                border: 1px solid #3d2e18;
                border-radius: 12px;
                border-bottom-left-radius: 4px;
                margin-right: 60px;
            }
            #senderLabel { color: #d4a96a; font-size: 11px; font-weight: 600; background: transparent; border: none; }
            #messageText { color: #e8dcc8; font-size: 12px; background: transparent; border: none; }
        )");
    }

    chatLayout->insertWidget(chatLayout->count() - 1, bubble);

    QTimer::singleShot(50, this, [this]() {
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    });
}

void ChatBotDialog::sendMessage()
{
    QString text = inputField->text().trimmed();
    if (text.isEmpty()) return;

    appendMessage("You", text, true);
    inputField->clear();

    typingIndicator->setVisible(true);
    inputField->setEnabled(false);
    sendButton->setEnabled(false);

    QString localResponse;
    if (handleLocalCommand(text, &localResponse)) {
        typingIndicator->setVisible(false);
        inputField->setEnabled(true);
        sendButton->setEnabled(true);
        appendMessage(m_isWeatherBot ? "MeteoBot" : "Assistant", localResponse, false);
        return;
    }

    if (isImageRequest(text))
        callImageApi(extractImagePrompt(text));
    else
        callApi(text);
}

void ChatBotDialog::sendExternalMessage(const QString &text)
{
    if (text.trimmed().isEmpty()) return;
    appendMessage("You", text, true);
    typingIndicator->setVisible(true);
    inputField->setEnabled(false);
    sendButton->setEnabled(false);

    QString localResponse;
    if (handleLocalCommand(text, &localResponse)) {
        typingIndicator->setVisible(false);
        inputField->setEnabled(true);
        sendButton->setEnabled(true);
        appendMessage(m_isWeatherBot ? "MeteoBot" : "Assistant", localResponse, false);
        return;
    }

    if (isImageRequest(text))
        callImageApi(extractImagePrompt(text));
    else
        callApi(text);
}

bool ChatBotDialog::handleLocalCommand(const QString &text, QString *responseOut)
{
    if (!responseOut) return false;

    QString trimmed = text.trimmed();
    QString lower = trimmed.toLower();

    // Allow a single user prompt to request multiple random batches.
    QRegularExpression addRandomAnyRx("\\badd\\s+(\\d+)\\s+random\\s+(orders?|employees?|clients?|suppliers?|equipment|equipments|equipement|equipements)\\b");
    QRegularExpressionMatchIterator addRandomIt = addRandomAnyRx.globalMatch(lower);
    if (addRandomIt.hasNext()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }

        QStringList batchResponses;
        while (addRandomIt.hasNext()) {
            QRegularExpressionMatch m = addRandomIt.next();
            int count = m.captured(1).toInt();
            QString entity = m.captured(2);

            if (entity.startsWith("order")) {
                batchResponses << handleAddRandomOrders(count);
            } else if (entity.startsWith("employee")) {
                batchResponses << handleAddRandomEmployees(count);
            } else if (entity.startsWith("client")) {
                batchResponses << handleAddRandomClients(count);
            } else if (entity.startsWith("supplier")) {
                batchResponses << handleAddRandomSuppliers(count);
            } else {
                batchResponses << handleAddRandomEquipment(count);
            }
        }

        *responseOut = batchResponses.join("\n\n");
        return true;
    }

    QRegularExpression addRandomRx("^add\\s+(\\d+)\\s+random\\s+(orders?|employees?|clients?|suppliers?|equipment|equipments|equipement|equipements)\\s*(now)?$");
    QRegularExpressionMatch addRandomMatch = addRandomRx.match(lower);
    if (addRandomMatch.hasMatch()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }

        int count = addRandomMatch.captured(1).toInt();
        QString entity = addRandomMatch.captured(2);

        if (entity.startsWith("order")) {
            *responseOut = handleAddRandomOrders(count);
        } else if (entity.startsWith("employee")) {
            *responseOut = handleAddRandomEmployees(count);
        } else if (entity.startsWith("client")) {
            *responseOut = handleAddRandomClients(count);
        } else if (entity.startsWith("supplier")) {
            *responseOut = handleAddRandomSuppliers(count);
        } else {
            *responseOut = handleAddRandomEquipment(count);
        }
        return true;
    }

    QRegularExpression addOrdersRx("^add\\s+(\\d+)\\s+random\\s+orders?$");
    QRegularExpressionMatch addMatch = addOrdersRx.match(lower);
    if (addMatch.hasMatch()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }
        int count = addMatch.captured(1).toInt();
        *responseOut = handleAddRandomOrders(count);
        return true;
    }

    QRegularExpression listOrdersRx("^(show|list)\\s+orders(\\s+(\\d+))?$" );
    QRegularExpressionMatch listMatch = listOrdersRx.match(lower);
    if (listMatch.hasMatch()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }
        int limit = listMatch.captured(3).isEmpty() ? 20 : listMatch.captured(3).toInt();
        if (limit <= 0) limit = 20;
        if (limit > 50) limit = 50;
        QString sql = QString("SELECT * FROM (SELECT ORDER_ID, CLIENT_ID, EMPLOYEE_ID, ORDER_TYPE, ORDER_STATUS, TOTAL_QUANTITY, TOTAL_PRICE, PAYMENT_STATUS FROM ORDERS ORDER BY ORDER_ID) WHERE ROWNUM <= %1").arg(limit);
        *responseOut = executeSqlCommand(sql);
        return true;
    }

    QRegularExpression deleteOrderRx("^delete\\s+order\\s+(\\d+)$");
    QRegularExpressionMatch deleteMatch = deleteOrderRx.match(lower);
    if (deleteMatch.hasMatch()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }
        int orderId = deleteMatch.captured(1).toInt();
        QString sql = QString("DELETE FROM ORDERS WHERE ORDER_ID = %1").arg(orderId);
        *responseOut = executeSqlCommand(sql);
        return true;
    }

    QRegularExpression updateOrderStatusRx("^update\\s+order\\s+(\\d+)\\s+status\\s+(pending|processing|completed|cancelled)$");
    QRegularExpressionMatch statusMatch = updateOrderStatusRx.match(lower);
    if (statusMatch.hasMatch()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }
        int orderId = statusMatch.captured(1).toInt();
        QString status = statusMatch.captured(2);
        status[0] = status[0].toUpper();
        QString sql = QString("UPDATE ORDERS SET ORDER_STATUS = '%1' WHERE ORDER_ID = %2").arg(status, QString::number(orderId));
        *responseOut = executeSqlCommand(sql);
        return true;
    }

    QRegularExpression updatePaymentRx("^update\\s+order\\s+(\\d+)\\s+payment\\s+status\\s+(unpaid|partial|paid)$");
    QRegularExpressionMatch payMatch = updatePaymentRx.match(lower);
    if (payMatch.hasMatch()) {
        if (!QSqlDatabase::database().isOpen()) {
            *responseOut = "Database connection is not available. Please check your DB settings.";
            return true;
        }
        int orderId = payMatch.captured(1).toInt();
        QString status = payMatch.captured(2);
        status[0] = status[0].toUpper();
        QString sql = QString("UPDATE ORDERS SET PAYMENT_STATUS = '%1' WHERE ORDER_ID = %2").arg(status, QString::number(orderId));
        *responseOut = executeSqlCommand(sql);
        return true;
    }

    return false;
}

QString ChatBotDialog::handleAddRandomOrders(int count)
{
    if (count <= 0) return "Please provide a positive number of orders to add.";
    if (count > 50) count = 50;

    QVector<int> clientIds;
    QSqlQuery clientQuery("SELECT CLIENT_ID FROM CLIENTS");
    while (clientQuery.next())
        clientIds.append(clientQuery.value(0).toInt());

    QVector<int> employeeIds;
    QSqlQuery employeeQuery("SELECT EMPLOYEE_ID FROM EMPLOYEES");
    while (employeeQuery.next())
        employeeIds.append(employeeQuery.value(0).toInt());

    if (clientIds.isEmpty() || employeeIds.isEmpty()) {
        return "Cannot add orders: CLIENTS or EMPLOYEES table is empty.";
    }

    const QStringList orderTypes = {"Chair", "Table", "Cabinet", "Wardrobe", "Other"};
    const QStringList orderStatuses = getAllowedColumnValues("ORDERS", "ORDER_STATUS");
    const QStringList paymentStatuses = getAllowedColumnValues("ORDERS", "PAYMENT_STATUS");
    if (orderStatuses.isEmpty() || paymentStatuses.isEmpty()) {
        return "Cannot add orders: valid ORDER_STATUS/PAYMENT_STATUS values were not found in DB constraints/defaults/existing data.";
    }

    int nextOrderId = 1;
    QSqlQuery nextIdQuery;
    if (nextIdQuery.exec("SELECT NVL(MAX(order_id), 0) + 1 FROM ORDERS") && nextIdQuery.next()) {
        nextOrderId = nextIdQuery.value(0).toInt();
    }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO ORDERS (order_id, client_id, employee_id, order_type, total_quantity, total_price, order_date, order_status, payment_status) "
                        "VALUES (:id, :client, :employee, :type, :quantity, :price, SYSDATE, :status, :payment)");

    int success = 0;
    QStringList errors;
    for (int i = 0; i < count; ++i) {
        int clientId = clientIds.at(QRandomGenerator::global()->bounded(clientIds.size()));
        int employeeId = employeeIds.at(QRandomGenerator::global()->bounded(employeeIds.size()));
        QString type = orderTypes.at(QRandomGenerator::global()->bounded(orderTypes.size()));
        QString status = orderStatuses.at(QRandomGenerator::global()->bounded(orderStatuses.size()));
        QString payment = paymentStatuses.at(QRandomGenerator::global()->bounded(paymentStatuses.size()));
        int quantity = QRandomGenerator::global()->bounded(1, 51);
        double price = 50.0 + (QRandomGenerator::global()->generateDouble() * 1950.0);

        bool inserted = false;
        for (int attempt = 0; attempt < 3 && !inserted; ++attempt) {
            insertQuery.bindValue(":id", nextOrderId);
            insertQuery.bindValue(":client", clientId);
            insertQuery.bindValue(":employee", employeeId);
            insertQuery.bindValue(":type", type);
            insertQuery.bindValue(":quantity", quantity);
            insertQuery.bindValue(":price", price);
            insertQuery.bindValue(":status", status);
            insertQuery.bindValue(":payment", payment);

            if (insertQuery.exec()) {
                success++;
                nextOrderId++;
                inserted = true;
                break;
            }

            QString dbError = insertQuery.lastError().databaseText();
            if (dbError.contains("ORA-00001")) {
                QSqlQuery refreshIdQuery;
                if (refreshIdQuery.exec("SELECT NVL(MAX(order_id), 0) + 1 FROM ORDERS") && refreshIdQuery.next()) {
                    nextOrderId = refreshIdQuery.value(0).toInt();
                    continue;
                }
            }

            errors << dbError;
            break;
        }

        if (!inserted && errors.size() < (i + 1)) {
            errors << QString("Failed to insert order at batch index %1.").arg(i + 1);
        }
    }

    QString result = QString("[OK: Inserted %1 of %2 random orders]").arg(success).arg(count);
    if (!errors.isEmpty()) {
        result += "\nErrors:";
        int maxErr = qMin(3, errors.size());
        for (int i = 0; i < maxErr; ++i)
            result += "\n- " + errors.at(i);
    }
    return result;
}

int ChatBotDialog::getNextId(const QString &tableName, const QString &idColumn, int fallback)
{
    QSqlQuery q;
    QString sql = QString("SELECT NVL(MAX(%1), 0) + 1 FROM %2").arg(idColumn, tableName);
    if (q.exec(sql) && q.next()) {
        int value = q.value(0).toInt();
        return value > 0 ? value : fallback;
    }
    return fallback;
}

QStringList ChatBotDialog::getColumnValuesFromCheckConstraints(const QString &tableName, const QString &columnName)
{
    QSqlQuery q;
    q.prepare("SELECT uc.SEARCH_CONDITION_VC "
              "FROM USER_CONSTRAINTS uc "
              "JOIN USER_CONS_COLUMNS ucc ON uc.CONSTRAINT_NAME = ucc.CONSTRAINT_NAME "
              "WHERE uc.CONSTRAINT_TYPE = 'C' "
              "AND uc.TABLE_NAME = :tableName "
              "AND ucc.COLUMN_NAME = :columnName");
    q.bindValue(":tableName", tableName.toUpper());
    q.bindValue(":columnName", columnName.toUpper());

    QStringList values;
    QSet<QString> seenUpper;
    if (q.exec()) {
        QRegularExpression quotedValueRx("'((?:''|[^'])*)'");
        while (q.next()) {
            QString condition = q.value(0).toString();
            if (condition.trimmed().isEmpty()) {
                continue;
            }

            QRegularExpressionMatchIterator it = quotedValueRx.globalMatch(condition);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                QString value = m.captured(1);
                value.replace("''", "'");
                QString key = value.toUpper();
                if (!value.isEmpty() && !seenUpper.contains(key)) {
                    seenUpper.insert(key);
                    values << value;
                }
            }
        }
    }

    return values;
}

QString ChatBotDialog::getColumnDefaultValue(const QString &tableName, const QString &columnName)
{
    QSqlQuery q;
    q.prepare("SELECT DATA_DEFAULT "
              "FROM USER_TAB_COLUMNS "
              "WHERE TABLE_NAME = :tableName "
              "AND COLUMN_NAME = :columnName");
    q.bindValue(":tableName", tableName.toUpper());
    q.bindValue(":columnName", columnName.toUpper());

    if (!q.exec() || !q.next()) {
        return QString();
    }

    QString raw = q.value(0).toString().trimmed();
    if (raw.isEmpty()) {
        return QString();
    }

    raw.remove('\n');
    raw.remove('\r');
    raw = raw.trimmed();

    while (raw.startsWith('(') && raw.endsWith(')') && raw.size() >= 2) {
        raw = raw.mid(1, raw.size() - 2).trimmed();
    }

    QRegularExpression quotedLiteralRx("^'((?:''|[^'])*)'$");
    QRegularExpressionMatch m = quotedLiteralRx.match(raw);
    if (!m.hasMatch()) {
        return QString();
    }

    QString value = m.captured(1);
    value.replace("''", "'");
    return value.trimmed();
}

QStringList ChatBotDialog::getDistinctColumnValues(const QString &tableName, const QString &columnName)
{
    static const QRegularExpression identRx("^[A-Z0-9_]+$", QRegularExpression::CaseInsensitiveOption);
    if (!identRx.match(tableName).hasMatch() || !identRx.match(columnName).hasMatch()) {
        return {};
    }

    QStringList values;
    QSet<QString> seenUpper;
    QString sql = QString("SELECT DISTINCT %1 FROM %2 WHERE %1 IS NOT NULL FETCH FIRST 100 ROWS ONLY")
                      .arg(columnName, tableName);

    QSqlQuery q;
    if (q.exec(sql)) {
        while (q.next()) {
            QString value = q.value(0).toString().trimmed();
            QString key = value.toUpper();
            if (!value.isEmpty() && !seenUpper.contains(key)) {
                seenUpper.insert(key);
                values << value;
            }
        }
    }

    return values;
}

QStringList ChatBotDialog::getAllowedColumnValues(const QString &tableName, const QString &columnName)
{
    QStringList values;
    QSet<QString> seenUpper;

    auto appendUnique = [&](const QStringList &items) {
        for (const QString &item : items) {
            QString value = item.trimmed();
            QString key = value.toUpper();
            if (!value.isEmpty() && !seenUpper.contains(key)) {
                seenUpper.insert(key);
                values << value;
            }
        }
    };

    appendUnique(getColumnValuesFromCheckConstraints(tableName, columnName));

    QString defaultValue = getColumnDefaultValue(tableName, columnName);
    if (!defaultValue.isEmpty()) {
        appendUnique({defaultValue});
    }

    appendUnique(getDistinctColumnValues(tableName, columnName));
    return values;
}

QString ChatBotDialog::handleAddRandomEmployees(int count)
{
    if (count <= 0) return "Please provide a positive number of employees to add.";
    if (count > 50) count = 50;

    const QStringList firstNames = {"Adam", "Lina", "Sami", "Nour", "Youssef", "Maya", "Rami", "Salma"};
    const QStringList lastNames = {"Ben Ali", "Trabelsi", "Mansour", "Haddad", "Gharbi", "Jaziri", "Ayari", "Kefi"};
    const QStringList jobs = {"Carpenter", "Designer", "Technician", "Manager", "Installer"};
    const QStringList departments = {"Production", "Design", "Operations", "Sales", "Maintenance"};
    const QStringList statuses = getAllowedColumnValues("EMPLOYEES", "EMPLOYEE_STATUS");
    if (statuses.isEmpty()) {
        return "Cannot add employees: no valid EMPLOYEE_STATUS values found in DB constraints/defaults/existing data.";
    }

    int nextId = getNextId("EMPLOYEES", "EMPLOYEE_ID");

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO EMPLOYEES (EMPLOYEE_ID, FIRST_NAME, LAST_NAME, JOB_TITLE, EMAIL, PHONE_NUMBER, SALARY, DEPARTMENT, AGE, EMPLOYEE_STATUS) "
                        "VALUES (:id, :first, :last, :job, :email, :phone, :salary, :dept, :age, :status)");

    int success = 0;
    QStringList errors;
    for (int i = 0; i < count; ++i) {
        QString first = firstNames.at(QRandomGenerator::global()->bounded(firstNames.size()));
        QString last = lastNames.at(QRandomGenerator::global()->bounded(lastNames.size()));
        QString job = jobs.at(QRandomGenerator::global()->bounded(jobs.size()));
        QString dept = departments.at(QRandomGenerator::global()->bounded(departments.size()));
        QString status = statuses.at(QRandomGenerator::global()->bounded(statuses.size()));
        int age = QRandomGenerator::global()->bounded(20, 56);
        double salary = 1200.0 + (QRandomGenerator::global()->generateDouble() * 3800.0);
        QString email = QString("%1.%2%3@hammerdown.tn").arg(first.toLower().remove(' '), last.toLower().remove(' '), QString::number(nextId));
        QString phone = QString::number(QRandomGenerator::global()->bounded(10000000, 100000000));

        bool inserted = false;
        for (int attempt = 0; attempt < 3 && !inserted; ++attempt) {
            insertQuery.bindValue(":id", nextId);
            insertQuery.bindValue(":first", first);
            insertQuery.bindValue(":last", last);
            insertQuery.bindValue(":job", job);
            insertQuery.bindValue(":email", email);
            insertQuery.bindValue(":phone", phone);
            insertQuery.bindValue(":salary", salary);
            insertQuery.bindValue(":dept", dept);
            insertQuery.bindValue(":age", age);
            insertQuery.bindValue(":status", status);

            if (insertQuery.exec()) {
                success++;
                nextId++;
                inserted = true;
                break;
            }

            QString dbError = insertQuery.lastError().databaseText();
            if (dbError.contains("ORA-00001")) {
                nextId = getNextId("EMPLOYEES", "EMPLOYEE_ID", nextId + 1);
                continue;
            }
            errors << dbError;
            break;
        }
    }

    QString result = QString("[OK: Inserted %1 of %2 random employees]").arg(success).arg(count);
    if (!errors.isEmpty()) {
        result += "\nErrors:";
        for (int i = 0; i < qMin(3, errors.size()); ++i)
            result += "\n- " + errors.at(i);
    }
    return result;
}

QString ChatBotDialog::handleAddRandomClients(int count)
{
    if (count <= 0) return "Please provide a positive number of clients to add.";
    if (count > 50) count = 50;

    const QStringList firstNames = {"Hedi", "Amira", "Karim", "Sarra", "Walid", "Ines", "Fares", "Rania"};
    const QStringList lastNames = {"Mabrouk", "Cherif", "Ben Salem", "Khalfallah", "Mejri", "Boussetta", "Chaari", "Sfaxi"};
    const QStringList genders = {"Male", "Female"};
    const QStringList statuses = getAllowedColumnValues("CLIENTS", "STATUS");
    if (statuses.isEmpty()) {
        return "Cannot add clients: no valid STATUS values found in DB constraints/defaults/existing data.";
    }
    const QStringList streets = {"Avenue Habib Bourguiba", "Rue de Marseille", "Avenue de la Liberte", "Rue d'Alger"};

    int nextId = getNextId("CLIENTS", "CLIENT_ID");

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO CLIENTS (CLIENT_ID, FIRST_NAME, LAST_NAME, EMAIL, PHONE_NUMBER, ADDRESS, GENDER, AGE, ACCOUNT_BALANCE, STATUS) "
                        "VALUES (:id, :first, :last, :email, :phone, :address, :gender, :age, :balance, :status)");

    int success = 0;
    QStringList errors;
    for (int i = 0; i < count; ++i) {
        QString first = firstNames.at(QRandomGenerator::global()->bounded(firstNames.size()));
        QString last = lastNames.at(QRandomGenerator::global()->bounded(lastNames.size()));
        QString gender = genders.at(QRandomGenerator::global()->bounded(genders.size()));
        QString status = statuses.at(QRandomGenerator::global()->bounded(statuses.size()));
        QString address = QString("%1, Tunis").arg(streets.at(QRandomGenerator::global()->bounded(streets.size())));
        int age = QRandomGenerator::global()->bounded(21, 66);
        double balance = QRandomGenerator::global()->generateDouble() * 10000.0;
        QString email = QString("%1.%2%3@client.tn").arg(first.toLower().remove(' '), last.toLower().remove(' '), QString::number(nextId));
        QString phone = QString::number(QRandomGenerator::global()->bounded(10000000, 100000000));

        bool inserted = false;
        for (int attempt = 0; attempt < 3 && !inserted; ++attempt) {
            insertQuery.bindValue(":id", nextId);
            insertQuery.bindValue(":first", first);
            insertQuery.bindValue(":last", last);
            insertQuery.bindValue(":email", email);
            insertQuery.bindValue(":phone", phone);
            insertQuery.bindValue(":address", address);
            insertQuery.bindValue(":gender", gender);
            insertQuery.bindValue(":age", age);
            insertQuery.bindValue(":balance", balance);
            insertQuery.bindValue(":status", status);

            if (insertQuery.exec()) {
                success++;
                nextId++;
                inserted = true;
                break;
            }

            QString dbError = insertQuery.lastError().databaseText();
            if (dbError.contains("ORA-00001")) {
                nextId = getNextId("CLIENTS", "CLIENT_ID", nextId + 1);
                continue;
            }
            errors << dbError;
            break;
        }
    }

    QString result = QString("[OK: Inserted %1 of %2 random clients]").arg(success).arg(count);
    if (!errors.isEmpty()) {
        result += "\nErrors:";
        for (int i = 0; i < qMin(3, errors.size()); ++i)
            result += "\n- " + errors.at(i);
    }
    return result;
}

QString ChatBotDialog::handleAddRandomSuppliers(int count)
{
    if (count <= 0) return "Please provide a positive number of suppliers to add.";
    if (count > 50) count = 50;

    const QStringList prefixes = {"Atlas", "Nord", "Cedar", "Prime", "Delta", "Sahara", "Olive", "Nova"};
    const QStringList suffixes = {"Wood", "Supply", "Materials", "Trade", "Systems", "Partners"};
    const QStringList statuses = getAllowedColumnValues("SUPPLIERS", "ACCOUNT_STATUS");
    if (statuses.isEmpty()) {
        return "Cannot add suppliers: no valid ACCOUNT_STATUS values found in DB constraints/defaults/existing data.";
    }

    int nextId = getNextId("SUPPLIERS", "SUPPLIER_ID");

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, EMAIL, PHONE_NUMBER, ADDRESS, POSTAL_CODE, DELIVERY_RATING, QUALITY_RATING, ACCOUNT_STATUS) "
                        "VALUES (:id, :name, :email, :phone, :address, :postal, :delivery, :quality, :status)");

    int success = 0;
    QStringList errors;
    for (int i = 0; i < count; ++i) {
        QString name = QString("%1 %2").arg(prefixes.at(QRandomGenerator::global()->bounded(prefixes.size())),
                                             suffixes.at(QRandomGenerator::global()->bounded(suffixes.size())));
        QString normalizedName = name.toLower();
        normalizedName.replace(' ', '.');
        QString email = QString("contact%1@%2.tn").arg(nextId).arg(normalizedName);
        QString phone = QString::number(QRandomGenerator::global()->bounded(10000000, 100000000));
        QString address = QString("Zone Industrielle %1, Tunis").arg(QRandomGenerator::global()->bounded(1, 25));
        QString postal = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
        double delivery = 2.5 + (QRandomGenerator::global()->generateDouble() * 2.5);
        double quality = 2.5 + (QRandomGenerator::global()->generateDouble() * 2.5);
        QString status = statuses.at(QRandomGenerator::global()->bounded(statuses.size()));

        bool inserted = false;
        for (int attempt = 0; attempt < 3 && !inserted; ++attempt) {
            insertQuery.bindValue(":id", nextId);
            insertQuery.bindValue(":name", name);
            insertQuery.bindValue(":email", email);
            insertQuery.bindValue(":phone", phone);
            insertQuery.bindValue(":address", address);
            insertQuery.bindValue(":postal", postal);
            insertQuery.bindValue(":delivery", delivery);
            insertQuery.bindValue(":quality", quality);
            insertQuery.bindValue(":status", status);

            if (insertQuery.exec()) {
                success++;
                nextId++;
                inserted = true;
                break;
            }

            QString dbError = insertQuery.lastError().databaseText();
            if (dbError.contains("ORA-00001")) {
                nextId = getNextId("SUPPLIERS", "SUPPLIER_ID", nextId + 1);
                continue;
            }
            errors << dbError;
            break;
        }
    }

    QString result = QString("[OK: Inserted %1 of %2 random suppliers]").arg(success).arg(count);
    if (!errors.isEmpty()) {
        result += "\nErrors:";
        for (int i = 0; i < qMin(3, errors.size()); ++i)
            result += "\n- " + errors.at(i);
    }
    return result;
}

QString ChatBotDialog::handleAddRandomEquipment(int count)
{
    if (count <= 0) return "Please provide a positive number of equipment records to add.";
    if (count > 50) count = 50;

    QVector<int> employeeIds;
    QSqlQuery employeeQuery("SELECT EMPLOYEE_ID FROM EMPLOYEES");
    while (employeeQuery.next())
        employeeIds.append(employeeQuery.value(0).toInt());

    if (employeeIds.isEmpty()) {
        return "Cannot add equipment: EMPLOYEES table is empty (RESPONSABLE is required).";
    }

    const QStringList types = {"Drill", "Saw", "Sander", "Compressor", "Workstation", "Safety Kit"};
    const QStringList statuses = getAllowedColumnValues("EQUIPMENT", "STATUS");
    if (statuses.isEmpty()) {
        return "Cannot add equipment: no valid STATUS values found in DB constraints/defaults/existing data.";
    }
    const QStringList locations = {"Warehouse A", "Warehouse B", "Workshop 1", "Workshop 2", "Site Storage"};

    int nextId = getNextId("EQUIPMENT", "EQUIPMENT_ID");

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, PURCHASE_DATE, EMPLOYEE_ID, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE, COUT_ACQUISITION) "
                        "VALUES (:id, :type, :description, :status, SYSDATE, :employee_id, :unit_price, :qty, :location, :responsable, :cout)");

    int success = 0;
    QStringList errors;
    for (int i = 0; i < count; ++i) {
        QString type = types.at(QRandomGenerator::global()->bounded(types.size()));
        QString status = statuses.at(QRandomGenerator::global()->bounded(statuses.size()));
        int qty = QRandomGenerator::global()->bounded(1, 31);
        double unitPrice = 80.0 + (QRandomGenerator::global()->generateDouble() * 2920.0);
        double cout = unitPrice * qty;
        QString description = QString("%1 for carpentry operations").arg(type);
        QString location = locations.at(QRandomGenerator::global()->bounded(locations.size()));
        int responsable = employeeIds.at(QRandomGenerator::global()->bounded(employeeIds.size()));

        bool inserted = false;
        for (int attempt = 0; attempt < 3 && !inserted; ++attempt) {
            insertQuery.bindValue(":id", nextId);
            insertQuery.bindValue(":type", type);
            insertQuery.bindValue(":description", description);
            insertQuery.bindValue(":status", status);
            insertQuery.bindValue(":employee_id", responsable);
            insertQuery.bindValue(":unit_price", unitPrice);
            insertQuery.bindValue(":qty", qty);
            insertQuery.bindValue(":location", location);
            insertQuery.bindValue(":responsable", responsable);
            insertQuery.bindValue(":cout", cout);

            if (insertQuery.exec()) {
                success++;
                nextId++;
                inserted = true;
                break;
            }

            QString dbError = insertQuery.lastError().databaseText();
            if (dbError.contains("ORA-00001")) {
                nextId = getNextId("EQUIPMENT", "EQUIPMENT_ID", nextId + 1);
                continue;
            }
            errors << dbError;
            break;
        }
    }

    QString result = QString("[OK: Inserted %1 of %2 random equipment records]").arg(success).arg(count);
    if (!errors.isEmpty()) {
        result += "\nErrors:";
        for (int i = 0; i < qMin(3, errors.size()); ++i)
            result += "\n- " + errors.at(i);
    }
    return result;
}

bool ChatBotDialog::isImageRequest(const QString &text) const
{
    QString lower = text.toLower();
    bool hasImageWord = lower.contains("image") || lower.contains("photo") || lower.contains("picture") ||
                        lower.contains("illustration") || lower.contains("art") || lower.contains("draw") || lower.contains("paint");
    bool hasImageVerb = lower.contains("generate") || lower.contains("draw") || lower.contains("create") ||
                        lower.contains("render") || lower.contains("paint") || lower.contains("make");
    return hasImageVerb && hasImageWord;
}

QString ChatBotDialog::extractImagePrompt(const QString &text) const
{
    QString original = text.trimmed();
    QString lower = original.toLower();

    const QStringList prefixes = {
        "generate me an image of ",
        "generate an image of ",
        "generate image of ",
        "generate me a photo of ",
        "generate a photo of ",
        "create an image of ",
        "create a photo of ",
        "create an illustration of ",
        "draw an image of ",
        "draw a picture of ",
        "make an image of ",
        "make a photo of ",
        "image of ",
        "photo of ",
        "picture of "
    };

    for (const QString &prefix : prefixes) {
        if (lower.startsWith(prefix)) {
            QString prompt = original.mid(prefix.length()).trimmed();
            return prompt.isEmpty() ? original : prompt;
        }
    }

    return original;
}

void ChatBotDialog::callImageApi(const QString &prompt)
{
    QUrl url("https://ancient-hat-ee03.yassinebenmustapha05.workers.dev/generate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(60000);

    QJsonObject body;
    body["prompt"] = prompt;
    body["size"] = "1024x1024";

    QJsonDocument doc(body);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    reply->setProperty("replyType", "image");
    reply->setProperty("prompt", prompt);
}

QString ChatBotDialog::buildSystemPrompt() const
{
    if (m_isWeatherBot) {
        return R"(You are "MeteoBot" — the official Weather & Carpentry Expert for the HammerDown application.
Your mission is to provide expert advice on how weather conditions (Temperature, Humidity, Rain) in Tunisia affect carpentry work.
)";
    }

    QString base = R"(You are "HammerDown AI Assistant" — a professional enterprise intelligence agent.
You provide precise, data-driven insights. You have FULL access to the database.

DATABASE SCHEMA:
- EMPLOYEES(EMPLOYEE_ID NUMBER PK, FIRST_NAME, LAST_NAME, JOB_TITLE, EMAIL, PHONE_NUMBER, SALARY NUMBER(12,2), DEPARTMENT, AGE NUMBER, EMPLOYEE_STATUS)
- CLIENTS(CLIENT_ID NUMBER PK, FIRST_NAME, LAST_NAME, EMAIL, PHONE_NUMBER, ADDRESS, GENDER, AGE NUMBER, ACCOUNT_BALANCE NUMBER(15,2), STATUS)
- ORDERS(ORDER_ID NUMBER PK, CLIENT_ID FK, EMPLOYEE_ID FK, ORDER_TYPE, ORDER_STATUS, TOTAL_QUANTITY NUMBER, TOTAL_PRICE NUMBER(15,2), PAYMENT_STATUS)
- EQUIPMENT(EQUIPMENT_ID NUMBER PK, EQUIPMENT_TYPE, QUANTITY NUMBER, UNIT_PRICE NUMBER(12,2), STATUS, DESCRIPTION, LOCATION, NOTES, NEXT_MAINTENANCE DATE, COUT_ACQUISITION NUMBER(12,2), RESPONSABLE)
- SUPPLIERS(SUPPLIER_ID NUMBER PK, SUPPLIER_NAME, EMAIL, PHONE_NUMBER, ADDRESS, POSTAL_CODE, DELIVERY_RATING NUMBER(3,2), QUALITY_RATING NUMBER(3,2), ACCOUNT_STATUS)

You can INSERT, UPDATE, DELETE, or SELECT data. When the user asks you to add, modify, remove, or read records, output the SQL inside [EXECUTE_SQL]...[/EXECUTE_SQL] tags. Rules:
- Use Oracle SQL syntax.
- SQL inside each [EXECUTE_SQL] tag must be exactly one plain statement (no multi-statement batches, no extra prose, no comments).
- Do NOT chain statements with semicolons. A single optional trailing semicolon is fine.
- Multiline formatting is allowed; line breaks do not make it multiple statements.
- Prefer one [EXECUTE_SQL] block that fully solves the user request.
- Only operate on these tables: EMPLOYEES, CLIENTS, ORDERS, EQUIPMENT, SUPPLIERS.
- For INSERT: respect DB constraints (NOT NULL, CHECK, FK, UNIQUE), include all required columns, and never set required columns to NULL.
- For ORDERS.ORDER_TYPE: use only these exact values: Chair, Table, Cabinet, Wardrobe, Other.
- For SUPPLIERS.PHONE_NUMBER: store exactly 8 digits (e.g. '12345678'), without country code prefix like +216.
- For INSERT: use the next available ID or let the sequence/trigger handle it.
- Always keep SQL valid and directly executable.
- For requests like deleting/updating N random rows, do it in one statement with a subquery, e.g. [EXECUTE_SQL]DELETE FROM ORDERS WHERE ORDER_ID IN (SELECT ORDER_ID FROM (SELECT ORDER_ID FROM ORDERS ORDER BY DBMS_RANDOM.VALUE) WHERE ROWNUM <= 5)[/EXECUTE_SQL]
- Example: [EXECUTE_SQL]INSERT INTO EMPLOYEES (FIRST_NAME, LAST_NAME, JOB_TITLE) VALUES ('John', 'Smith', 'Blacksmith')[/EXECUTE_SQL] Employee added.
 - For reading data, use SELECT queries inside [EXECUTE_SQL]...[/EXECUTE_SQL] tags, and keep results small (limit rows).
 - NEVER use DROP, TRUNCATE, ALTER, CREATE, GRANT, REVOKE, RENAME, BEGIN/DECLARE blocks, or any DDL/PLSQL commands. Only SELECT/INSERT/UPDATE/DELETE are allowed.)";

    return base;
}

void ChatBotDialog::trimConversationHistory(int maxNonSystemMessages)
{
    if (maxNonSystemMessages <= 0) return;

    QJsonArray systemMessages;
    QJsonArray nonSystemMessages;

    for (const QJsonValue &val : conversationHistory) {
        QJsonObject msg = val.toObject();
        if (msg["role"].toString() == "system")
            systemMessages.append(msg);
        else
            nonSystemMessages.append(msg);
    }

    if (nonSystemMessages.size() <= maxNonSystemMessages) return;

    QJsonArray trimmed;
    for (const QJsonValue &val : systemMessages)
        trimmed.append(val);

    int start = nonSystemMessages.size() - maxNonSystemMessages;
    for (int i = start; i < nonSystemMessages.size(); ++i)
        trimmed.append(nonSystemMessages.at(i));

    conversationHistory = trimmed;
}

void ChatBotDialog::callApi(const QString &userMessage)
{
    if (!userMessage.isEmpty()) {
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = userMessage;
        conversationHistory.append(userMsg);
        pendingUserMessage = userMessage;
        retryCount = 0;
        rateLimitRetries = 0;
    } else if ((!pendingUserMessage.isEmpty() && conversationHistory.isEmpty()) || 
               (!conversationHistory.isEmpty() && conversationHistory.last().toObject()["role"].toString() != "user")) {
        // If we're retrying and the user message was popped off, re-add it
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = pendingUserMessage;
        conversationHistory.append(userMsg);
    }

    // Keep the payload small to speed up responses.
    trimConversationHistory(6);

    QString currentModel = modelList.value(retryCount, modelList.first());

    QUrl url("https://openrouter.ai/api/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    request.setRawHeader("HTTP-Referer", "https://hammerdown.app");
    request.setRawHeader("X-Title", "HammerDown Assistant");
    request.setTransferTimeout(60000);

    // Configure SSL to avoid TLS issues on Windows
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);

    QJsonObject body;
    body["model"] = currentModel;
    body["messages"] = conversationHistory;
    body["max_tokens"] = 500;
    body["temperature"] = 0.2;
    body["top_p"] = 0.8;

    QJsonDocument doc(body);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    reply->setProperty("replyType", "chat");
}

void ChatBotDialog::retryWithNextModel()
{
    retryCount++;
    if (retryCount < modelList.size()) {
        // Remove the last user message (will be re-added by callApi)
        if (!conversationHistory.isEmpty()) conversationHistory.removeLast();
        callApi(""); // empty string = retry, reuse pendingUserMessage context
    }
}

void ChatBotDialog::onApiReplyFinished(QNetworkReply *reply)
{
    if (reply->url().toString().contains("openweathermap")) {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QJsonObject current = obj["current"].toObject();
            double temp = current["temp"].toDouble();
            int hum = current["humidity"].toInt();
            QString cond = current["weather"].toArray()[0].toObject()["main"].toString();

            QJsonObject weatherContext;
            weatherContext["role"] = "system";
            weatherContext["content"] = QString("REAL-TIME CONTEXT: Weather in Tunisia is %1°C, %2% Humidity, %3.").arg(temp).arg(hum).arg(cond);
            conversationHistory.append(weatherContext);
        }
        reply->deleteLater();
        return;
    }

    QString replyType = reply->property("replyType").toString();
    const QString replyUrl = reply->url().toString();
    const bool isWorkerImage = replyUrl.contains("workers.dev/generate");
    if (replyType == "image" || isWorkerImage) {
        typingIndicator->setVisible(false);
        inputField->setEnabled(true);
        sendButton->setEnabled(true);

        QByteArray data = reply->readAll();
        QString prompt = reply->property("prompt").toString();
        if (reply->error() != QNetworkReply::NoError) {
            appendMessage("Assistant", "Image request failed: " + reply->errorString(), false);
            reply->deleteLater();
            return;
        }

        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
        if (contentType.startsWith("image/")) {
            QPixmap pix;
            if (pix.loadFromData(data))
                appendImageMessage("Assistant", pix, prompt, false);
            else
                appendMessage("Assistant", "Image decode failed.", false);
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            if (obj.contains("url")) {
                QUrl imageUrl(obj["url"].toString());
                QNetworkReply *imgReply = networkManager->get(QNetworkRequest(imageUrl));
                imgReply->setProperty("replyType", "image_download");
                imgReply->setProperty("prompt", prompt);
            } else if (obj.contains("image")) {
                QString imageField = obj["image"].toString();
                QString b64 = imageField;
                if (imageField.startsWith("data:image")) {
                    int comma = imageField.indexOf(',');
                    if (comma >= 0)
                        b64 = imageField.mid(comma + 1);
                }
                QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw))
                    appendImageMessage("Assistant", pix, prompt, false);
                else
                    appendMessage("Assistant", "Image decode failed.", false);
            } else if (obj.contains("b64") || obj.contains("b64_json")) {
                QString b64 = obj.contains("b64") ? obj["b64"].toString()
                             : obj["b64_json"].toString();
                QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw))
                    appendImageMessage("Assistant", pix, prompt, false);
                else
                    appendMessage("Assistant", "Image decode failed.", false);
            } else if (obj.contains("data") && obj["data"].isArray()) {
                QJsonArray images = obj["data"].toArray();
                if (!images.isEmpty()) {
                    QJsonObject first = images.first().toObject();
                    if (first.contains("b64_json")) {
                        QByteArray raw = QByteArray::fromBase64(first["b64_json"].toString().toUtf8());
                        QPixmap pix;
                        if (pix.loadFromData(raw))
                            appendImageMessage("Assistant", pix, prompt, false);
                        else
                            appendMessage("Assistant", "Image decode failed.", false);
                    } else if (first.contains("url")) {
                        QUrl imageUrl(first["url"].toString());
                        QNetworkReply *imgReply = networkManager->get(QNetworkRequest(imageUrl));
                        imgReply->setProperty("replyType", "image_download");
                        imgReply->setProperty("prompt", prompt);
                    } else {
                        appendMessage("Assistant", "Image request failed: unsupported response format.", false);
                    }
                } else {
                    appendMessage("Assistant", "Image request failed: no image data returned.", false);
                }
            } else {
                appendMessage("Assistant", "Image request failed: unsupported response format.", false);
            }
        } else {
            QString text = QString::fromUtf8(data);
            QRegularExpression rx("data:image/[^;]+;base64,([A-Za-z0-9+/=]+)");
            QRegularExpressionMatch match = rx.match(text);
            if (match.hasMatch()) {
                QByteArray raw = QByteArray::fromBase64(match.captured(1).toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw))
                    appendImageMessage("Assistant", pix, prompt, false);
                else
                    appendMessage("Assistant", "Image decode failed.", false);
            } else {
                appendMessage("Assistant", "Image request failed: invalid JSON response.", false);
            }
        }

        reply->deleteLater();
        return;
    }

    if (replyType == "image_download") {
        typingIndicator->setVisible(false);
        inputField->setEnabled(true);
        sendButton->setEnabled(true);

        QByteArray raw = reply->readAll();
        QPixmap pix;
        if (pix.loadFromData(raw)) {
            appendImageMessage("Assistant", pix, reply->property("prompt").toString(), false);
        } else {
            appendMessage("Assistant", "Image download failed.", false);
        }
        reply->deleteLater();
        return;
    }

    typingIndicator->setVisible(false);
    inputField->setEnabled(true);
    sendButton->setEnabled(true);

    QByteArray data = reply->readAll();
    QString responseText;

    if (reply->error() != QNetworkReply::NoError) {
        QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        int statusCode = httpStatus.isValid() ? httpStatus.toInt() : 0;

        // Special handling for HTTP 429 (Too Many Requests) — wait and retry
        if (statusCode == 429 && rateLimitRetries < 2) {
            rateLimitRetries++;
            reply->deleteLater();
            // Remove the user message so callApi re-adds it
            if (!conversationHistory.isEmpty()) conversationHistory.removeLast();
            int delayMs = rateLimitRetries * 3000; // 3s, 6s backoff
            typingIndicator->setVisible(true);
            inputField->setEnabled(false);
            sendButton->setEnabled(false);
            QTimer::singleShot(delayMs, this, [this]() {
                callApi(""); // retry with same model
            });
            return;
        }

        // Network-level error — try next model
        if (retryCount + 1 < modelList.size()) {
            reply->deleteLater();
            rateLimitRetries = 0; // reset for next model
            retryWithNextModel();
            return;
        }
        QString statusInfo = httpStatus.isValid() ? QString(" [HTTP %1]").arg(statusCode) : "";
        if (statusCode == 429) {
            responseText = "The AI service is currently busy (rate limited). Please wait a moment and try again.";
        } else {
            responseText = "Connection error" + statusInfo + ": " + reply->errorString();
        }
        if (!data.isEmpty()) {
            QJsonDocument errDoc = QJsonDocument::fromJson(data);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                QJsonObject errObj = errDoc.object()["error"].toObject();
                QString apiMsg = errObj["message"].toString();
                if (apiMsg.isEmpty())
                    apiMsg = errObj["code"].toString();
                if (!apiMsg.isEmpty() && statusCode != 429)
                    responseText += "\nDetails: " + apiMsg;

                QJsonObject metaObj = errObj["metadata"].toObject();
                QString providerRaw = metaObj["raw"].toString();
                if (!providerRaw.isEmpty() && statusCode != 429)
                    responseText += "\nProvider: " + providerRaw.left(240);
            }
        }
        if (!conversationHistory.isEmpty()) conversationHistory.removeLast();
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            QJsonObject root = doc.object();
            if (root.contains("error")) {
                QJsonObject errObj = root["error"].toObject();
                int errCode = errObj["code"].toInt();
                // API-level 429: wait and retry
                if (errCode == 429 && rateLimitRetries < 2) {
                    rateLimitRetries++;
                    reply->deleteLater();
                    if (!conversationHistory.isEmpty()) conversationHistory.removeLast();
                    int delayMs = rateLimitRetries * 3000;
                    typingIndicator->setVisible(true);
                    inputField->setEnabled(false);
                    sendButton->setEnabled(false);
                    QTimer::singleShot(delayMs, this, [this]() {
                        callApi("");
                    });
                    return;
                }
                // API returned an error — try next model
                if (retryCount + 1 < modelList.size()) {
                    reply->deleteLater();
                    rateLimitRetries = 0;
                    retryWithNextModel();
                    return;
                }
                QString errMsg = errObj["message"].toString();
                QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                QString statusInfo = httpStatus.isValid() ? QString(" [HTTP %1]").arg(httpStatus.toInt()) : "";
                responseText = QString("API error%1: %2").arg(statusInfo, errMsg);
                if (!conversationHistory.isEmpty()) conversationHistory.removeLast();
            } else {
                QJsonArray choices = root["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject firstChoice = choices[0].toObject();
                    QJsonObject msgObj = firstChoice["message"].toObject();
                    responseText = msgObj["content"].toString();
                    if (responseText.trimmed().isEmpty()) {
                        responseText = firstChoice["text"].toString();
                    }

                    if (responseText.trimmed().isEmpty()) {
                        if (retryCount + 1 < modelList.size()) {
                            reply->deleteLater();
                            retryWithNextModel();
                            return;
                        }
                    } else {
                        QJsonObject assistantMsg;
                        assistantMsg["role"] = "assistant";
                        assistantMsg["content"] = responseText;
                        conversationHistory.append(assistantMsg);
                    }
                }
            }
        } else {
            responseText = QString::fromUtf8(data).left(300);
        }
    }

    if (responseText.trimmed().isEmpty()) {
        QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QString statusText = status.isValid() ? QString(" (HTTP %1)").arg(status.toInt()) : QString();
        responseText = "Assistant returned an empty response" + statusText + ".";
    }

    // If AI provider is down, still support direct SQL execution for power users.
    QString pendingUpper = pendingUserMessage.trimmed().toUpper();
    bool isDirectSql = pendingUpper.startsWith("SELECT") || pendingUpper.startsWith("INSERT") ||
                       pendingUpper.startsWith("UPDATE") || pendingUpper.startsWith("DELETE");
    if ((responseText.startsWith("Connection error") || responseText.startsWith("API error")) && isDirectSql) {
        responseText = "AI service is unavailable right now. Executing your SQL directly:\n" + executeSqlCommand(pendingUserMessage);
    }

    if (!m_isWeatherBot)
        responseText = processResponse(responseText);
    appendMessage(m_isWeatherBot ? "MeteoBot" : "Assistant", responseText, false);
    reply->deleteLater();
}

QString ChatBotDialog::processResponse(const QString &response)
{
    QString result = response;

    // Match EXECUTE_SQL tags case-insensitively and tolerate whitespace/newlines around tag names.
    QRegularExpression taggedRx(
        R"(\[\s*EXECUTE_SQL\s*\](.*?)\[\s*/\s*EXECUTE_SQL\s*\])",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator it = taggedRx.globalMatch(result);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString sql = match.captured(1).trimmed();
        QString execResult = executeSqlCommand(sql);
        result.replace(match.captured(0), execResult);
    }

    // If model output was truncated and closing tag is missing, execute the tail anyway.
    QRegularExpression openOnlyRx(R"(\[\s*EXECUTE_SQL\s*\](.*)$)",
                                  QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch openOnlyMatch = openOnlyRx.match(result);
    if (openOnlyMatch.hasMatch() && !result.contains(QRegularExpression(R"(\[\s*/\s*EXECUTE_SQL\s*\])", QRegularExpression::CaseInsensitiveOption))) {
        QString sql = openOnlyMatch.captured(1).trimmed();
        QString execResult = executeSqlCommand(sql);
        result.replace(openOnlyMatch.captured(0), execResult);
    }

    return result;
}

QString ChatBotDialog::executeSqlCommand(const QString &sql)
{
    QString trimmedSql = sql.trimmed();

    // Strip markdown fences if the model wrapped SQL in ```sql ... ```.
    QRegularExpression fenceRx(R"(^\s*```(?:sql)?\s*([\s\S]*?)\s*```\s*$)",
                               QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch fenceMatch = fenceRx.match(trimmedSql);
    if (fenceMatch.hasMatch()) {
        trimmedSql = fenceMatch.captured(1).trimmed();
    }

    if (trimmedSql.isEmpty()) {
        return "[Blocked: empty SQL command]";
    }

    // Normalize curly apostrophes to plain SQL apostrophes.
    trimmedSql.replace(QChar(0x2019), '\'');

    // Auto-repair common quote mistakes: apostrophe inside a string literal
    // such as d'Alger should become d''Alger.
    auto repairQuotedStrings = [](const QString &input, bool *changed, bool *unterminated) {
        QString out;
        out.reserve(input.size() + 8);
        bool inLiteral = false;
        bool didChange = false;

        for (int i = 0; i < input.size(); ++i) {
            QChar ch = input.at(i);
            if (ch != '\'') {
                out.append(ch);
                continue;
            }

            if (!inLiteral) {
                inLiteral = true;
                out.append(ch);
                continue;
            }

            if (i + 1 < input.size() && input.at(i + 1) == '\'') {
                out.append("''");
                ++i;
                continue;
            }

            QChar prev = (i > 0) ? input.at(i - 1) : QChar();
            QChar next = (i + 1 < input.size()) ? input.at(i + 1) : QChar();
            bool insideWord = prev.isLetterOrNumber() && next.isLetterOrNumber();

            if (insideWord) {
                out.append("''");
                didChange = true;
            } else {
                inLiteral = false;
                out.append(ch);
            }
        }

        if (changed) *changed = didChange;
        if (unterminated) *unterminated = inLiteral;
        return out;
    };

    bool quoteRepaired = false;
    bool unterminatedLiteral = false;
    trimmedSql = repairQuotedStrings(trimmedSql, &quoteRepaired, &unterminatedLiteral);
    if (unterminatedLiteral) {
        return "[Blocked: malformed quoted string. Text values must use single quotes and internal apostrophes must be doubled, e.g. 'Rue d''Alger']";
    }

    QString upper = trimmedSql.toUpper();
    const bool isPlSqlBlock = upper.startsWith("DECLARE") || upper.startsWith("BEGIN");

    if (!isPlSqlBlock) {
        // For plain SQL, allow a single trailing semicolon and strip comments.
        QRegularExpression blockCommentRx(R"(/\*[\s\S]*?\*/)");
        trimmedSql.remove(blockCommentRx);
        QRegularExpression lineCommentRx(R"(--[^\r\n]*)");
        trimmedSql.remove(lineCommentRx);
        trimmedSql = trimmedSql.trimmed();

        // Accept multiline SQL and semicolons inside string literals; only block
        // true statement separators (';') that are not trailing terminators.
        QVector<int> topLevelSemicolons;
        bool inLiteral = false;
        for (int i = 0; i < trimmedSql.size(); ++i) {
            QChar ch = trimmedSql.at(i);
            if (ch == '\'') {
                if (inLiteral && i + 1 < trimmedSql.size() && trimmedSql.at(i + 1) == '\'') {
                    ++i; // Escaped quote inside literal
                    continue;
                }
                inLiteral = !inLiteral;
                continue;
            }
            if (!inLiteral && ch == ';') {
                topLevelSemicolons.append(i);
            }
        }

        QSet<int> trailingSemicolonPositions;
        int cursor = trimmedSql.size() - 1;
        while (cursor >= 0) {
            while (cursor >= 0 && trimmedSql.at(cursor).isSpace()) {
                --cursor;
            }
            if (cursor >= 0 && trimmedSql.at(cursor) == ';' && topLevelSemicolons.contains(cursor)) {
                trailingSemicolonPositions.insert(cursor);
                --cursor;
            } else {
                break;
            }
        }

        bool hasInternalStatementSeparator = false;
        for (int pos : topLevelSemicolons) {
            if (!trailingSemicolonPositions.contains(pos)) {
                hasInternalStatementSeparator = true;
                break;
            }
        }

        if (hasInternalStatementSeparator) {
            return "[Blocked: only one plain SQL statement is allowed]";
        }

        if (!trailingSemicolonPositions.isEmpty()) {
            QString normalized;
            normalized.reserve(trimmedSql.size());
            for (int i = 0; i < trimmedSql.size(); ++i) {
                if (!trailingSemicolonPositions.contains(i)) {
                    normalized.append(trimmedSql.at(i));
                }
            }
            trimmedSql = normalized.trimmed();
        }
    }

    // Safety: block DDL, privilege changes, and PL/SQL blocks.
    upper = trimmedSql.toUpper();
    const QStringList blockedTokens = {
        " DROP ", " TRUNCATE ", " ALTER ", " CREATE ", " GRANT ", " REVOKE ",
        " RENAME ", " COMMENT ", " ANALYZE ", " MERGE ",
        " EXECUTE IMMEDIATE ", " COMMIT ", " ROLLBACK ", " SAVEPOINT "
    };

    QString padded = " " + upper + " ";
    for (const QString &token : blockedTokens) {
        if (padded.contains(token)) {
            return "[Blocked: schema/administrative SQL is not allowed]";
        }
    }

    // Enforce management-table scope.
    const QStringList allowedTables = {"EMPLOYEES", "CLIENTS", "ORDERS", "EQUIPMENT", "SUPPLIERS"};
    const bool isInsert = upper.startsWith("INSERT");
    const bool isUpdate = upper.startsWith("UPDATE");
    const bool isDelete = upper.startsWith("DELETE");
    const bool isSelect = upper.startsWith("SELECT");

    if (isPlSqlBlock) {
        // Allow PL/SQL only when it performs DML on allowed management tables.
        bool hasDml = upper.contains(" INSERT ") || upper.contains(" UPDATE ") || upper.contains(" DELETE ") || upper.contains(" SELECT ");
        if (!hasDml) {
            return "[Blocked: PL/SQL block must contain DML only]";
        }

        QRegularExpression tableRx("\\b(?:INTO|UPDATE|FROM|JOIN)\\s+([A-Z0-9_]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = tableRx.globalMatch(trimmedSql);
        bool foundAnyTable = false;
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            QString table = m.captured(1).toUpper();
            if (table == "DUAL") {
                continue;
            }
            foundAnyTable = true;
            if (!allowedTables.contains(table)) {
                return "[Blocked: PL/SQL target table is outside allowed management tables]";
            }
        }

        if (!foundAnyTable) {
            return "[Blocked: no allowed management table found in PL/SQL block]";
        }
    } else {
        if (!isInsert && !isUpdate && !isDelete && !isSelect) {
            return "[Blocked: only SELECT, INSERT, UPDATE, and DELETE are allowed]";
        }

        QString targetTable;
        if (isInsert) {
            QRegularExpression rx("^\\s*INSERT\\s+INTO\\s+([A-Z0-9_]+)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch m = rx.match(trimmedSql);
            if (m.hasMatch()) targetTable = m.captured(1).toUpper();
        } else if (isUpdate) {
            QRegularExpression rx("^\\s*UPDATE\\s+([A-Z0-9_]+)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch m = rx.match(trimmedSql);
            if (m.hasMatch()) targetTable = m.captured(1).toUpper();
        } else if (isDelete) {
            QRegularExpression rx("^\\s*DELETE\\s+FROM\\s+([A-Z0-9_]+)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch m = rx.match(trimmedSql);
            if (m.hasMatch()) targetTable = m.captured(1).toUpper();
        } else if (isSelect) {
            QRegularExpression fromJoinRx("\\b(?:FROM|JOIN)\\s+([A-Z0-9_]+)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator it = fromJoinRx.globalMatch(trimmedSql);
            bool hasAllowedTable = false;
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                QString table = m.captured(1).toUpper();
                if (allowedTables.contains(table)) {
                    hasAllowedTable = true;
                    break;
                }
            }
            if (!hasAllowedTable) {
                return "[Blocked: SELECT is limited to management tables only]";
            }
        }

        if (!targetTable.isEmpty() && !allowedTables.contains(targetTable)) {
            return "[Blocked: target table is outside allowed management tables]";
        }
    }

    // Constraint-aware guard for AI-generated INSERT statements.
    if (isInsert && !isPlSqlBlock) {
        QRegularExpression insertValuesRx(
            R"(^\s*INSERT\s+INTO\s+([A-Z0-9_]+)\s*\((.*?)\)\s*VALUES\s*\((.*)\)\s*$)",
            QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch insertMatch = insertValuesRx.match(trimmedSql);
        if (!insertMatch.hasMatch()) {
            return "[Blocked: INSERT must use explicit column list and VALUES(...) so constraints can be validated]";
        }

        QString insertTableName = insertMatch.captured(1).toUpper();
        QString columnsChunk = insertMatch.captured(2).trimmed();
        QString valuesChunk = insertMatch.captured(3).trimmed();

        auto splitTopLevelCsv = [](const QString &input) {
            QStringList out;
            QString current;
            current.reserve(input.size());
            bool inLiteral = false;
            int parenDepth = 0;

            for (int i = 0; i < input.size(); ++i) {
                const QChar ch = input.at(i);

                if (ch == '\'') {
                    current.append(ch);
                    if (inLiteral && i + 1 < input.size() && input.at(i + 1) == '\'') {
                        current.append('\'');
                        ++i;
                        continue;
                    }
                    inLiteral = !inLiteral;
                    continue;
                }

                if (!inLiteral) {
                    if (ch == '(') {
                        ++parenDepth;
                    } else if (ch == ')') {
                        if (parenDepth > 0) --parenDepth;
                    } else if (ch == ',' && parenDepth == 0) {
                        out << current.trimmed();
                        current.clear();
                        continue;
                    }
                }

                current.append(ch);
            }

            if (!current.trimmed().isEmpty()) {
                out << current.trimmed();
            }
            return out;
        };

        QStringList insertColumns = splitTopLevelCsv(columnsChunk);
        QStringList insertValues = splitTopLevelCsv(valuesChunk);

        if (insertColumns.isEmpty() || insertValues.isEmpty() || insertColumns.size() != insertValues.size()) {
            return "[Blocked: invalid INSERT column/value mapping]";
        }

        for (int i = 0; i < insertColumns.size(); ++i) {
            QString col = insertColumns.at(i).trimmed().toUpper();
            if (col.startsWith('"') && col.endsWith('"') && col.size() >= 2) {
                col = col.mid(1, col.size() - 2);
            }
            insertColumns[i] = col;
        }

        QSqlQuery requiredCols;
        requiredCols.prepare(
            "SELECT COLUMN_NAME, DATA_DEFAULT "
            "FROM USER_TAB_COLUMNS "
            "WHERE TABLE_NAME = :table AND NULLABLE = 'N'");
        requiredCols.bindValue(":table", insertTableName);

        if (!requiredCols.exec()) {
            return "[Error: failed to read table constraints before INSERT: " + requiredCols.lastError().text() + "]";
        }

        while (requiredCols.next()) {
            const QString requiredCol = requiredCols.value(0).toString().toUpper();
            const QString defaultExpr = requiredCols.value(1).toString().trimmed();

            const int idx = insertColumns.indexOf(requiredCol);
            const bool hasDefault = !defaultExpr.isEmpty();

            if (idx < 0) {
                if (!hasDefault) {
                    return "[Blocked: INSERT missing required column " + requiredCol + " on table " + insertTableName + "]";
                }
                continue;
            }

            const QString valueExpr = insertValues.at(idx).trimmed();
            if (valueExpr.compare("NULL", Qt::CaseInsensitive) == 0) {
                return "[Blocked: column " + requiredCol + " cannot be NULL]";
            }
        }

        if (insertTableName == "ORDERS") {
            const int typeIdx = insertColumns.indexOf("ORDER_TYPE");
            if (typeIdx >= 0) {
                QString typeExpr = insertValues.at(typeIdx).trimmed();
                QString typeValue = typeExpr;
                if (typeExpr.startsWith('\'') && typeExpr.endsWith('\'') && typeExpr.size() >= 2) {
                    typeValue = typeExpr.mid(1, typeExpr.size() - 2);
                    typeValue.replace("''", "'");
                }

                const QStringList allowedOrderTypes = {"Chair", "Table", "Cabinet", "Wardrobe", "Other"};
                bool okType = false;
                for (const QString &allowed : allowedOrderTypes) {
                    if (typeValue.compare(allowed, Qt::CaseInsensitive) == 0) {
                        okType = true;
                        break;
                    }
                }
                if (!okType) {
                    return "[Blocked: ORDERS.ORDER_TYPE must be one of Chair, Table, Cabinet, Wardrobe, Other]";
                }
            }
        }

        if (insertTableName == "SUPPLIERS") {
            const int phoneIdx = insertColumns.indexOf("PHONE_NUMBER");
            if (phoneIdx >= 0) {
                QString phoneExpr = insertValues.at(phoneIdx).trimmed();
                QString phoneValue = phoneExpr;
                if (phoneExpr.startsWith('\'') && phoneExpr.endsWith('\'') && phoneExpr.size() >= 2) {
                    phoneValue = phoneExpr.mid(1, phoneExpr.size() - 2);
                    phoneValue.replace("''", "'");
                }
                QRegularExpression phone8Rx("^\\d{8}$");
                if (!phone8Rx.match(phoneValue).hasMatch()) {
                    return "[Blocked: SUPPLIERS.PHONE_NUMBER must be exactly 8 digits without +216 (example: 12345678)]";
                }
            }
        }
    }

    QSqlQuery q;
    if (q.exec(trimmedSql)) {
        if (isSelect) {
            QSqlRecord rec = q.record();
            QStringList headers;
            for (int i = 0; i < rec.count(); ++i)
                headers << rec.fieldName(i);

            QStringList lines;
            lines << headers.join(" | ");

            int rowCount = 0;
            const int maxRows = 20;
            while (q.next() && rowCount < maxRows) {
                QStringList row;
                for (int i = 0; i < rec.count(); ++i)
                    row << q.value(i).toString();
                lines << row.join(" | ");
                rowCount++;
            }

            if (rowCount == 0)
                return "[OK: No rows returned]";
            if (q.next())
                lines << QString("...[truncated to %1 rows]").arg(maxRows);
            return "[OK]\n" + lines.join("\n");
        }

        int affected = q.numRowsAffected();
        if (isInsert)
            return QString("[OK: Inserted %1 row(s)]").arg(affected);
        else if (isUpdate)
            return QString("[OK: Updated %1 row(s)]").arg(affected);
        else if (isDelete)
            return QString("[OK: Deleted %1 row(s)]").arg(affected);
        else
            return QString("[OK: %1 row(s) affected]").arg(affected);
    } else {
        QString dbError = q.lastError().text();
        if (dbError.contains("ORA-01756", Qt::CaseInsensitive)) {
            return "[Blocked: malformed quoted string. Use single quotes for text and escape apostrophes by doubling them, e.g. 'Rue d''Alger']";
        }
        return "[Error: " + dbError + "]";
    }
}
