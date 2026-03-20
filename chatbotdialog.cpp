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

    // Fallback model list — if one model is down, try the next
    modelList << "nvidia/nemotron-nano-9b-v2:free"
              << "openrouter/free"
              << "meta-llama/llama-3.3-70b-instruct:free"
              << "google/gemma-3-27b-it:free";

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

    const QStringList orderTypes = {"Custom Furniture", "Repair", "Installation", "Design", "Consulting"};
    const QStringList orderStatuses = {"Pending", "Processing", "Completed"};
    const QStringList paymentStatuses = {"Unpaid", "Partial", "Paid"};

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO ORDERS (order_id, client_id, employee_id, order_type, total_quantity, total_price, order_date, order_status, payment_status) "
                        "VALUES (NULL, :client, :employee, :type, :quantity, :price, SYSDATE, :status, :payment)");

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

        insertQuery.bindValue(":client", clientId);
        insertQuery.bindValue(":employee", employeeId);
        insertQuery.bindValue(":type", type);
        insertQuery.bindValue(":quantity", quantity);
        insertQuery.bindValue(":price", price);
        insertQuery.bindValue(":status", status);
        insertQuery.bindValue(":payment", payment);

        if (insertQuery.exec()) {
            success++;
        } else {
            errors << insertQuery.lastError().databaseText();
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
- Only one statement per tag. Use multiple tags for multiple statements.
- For INSERT: use the next available ID or let the sequence/trigger handle it.
- Always confirm what you did after the SQL.
- Example: "I'll add that employee now. [EXECUTE_SQL]INSERT INTO EMPLOYEES (FIRST_NAME, LAST_NAME, JOB_TITLE) VALUES ('John', 'Smith', 'Blacksmith')[/EXECUTE_SQL] Done! John Smith has been added."
 - For reading data, use SELECT queries inside [EXECUTE_SQL]...[/EXECUTE_SQL] tags, and keep results small (limit rows).
 - NEVER use DROP TABLE, TRUNCATE, ALTER TABLE, or any DDL commands. Only SELECT/INSERT/UPDATE/DELETE are allowed.)";

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
    } else if (!pendingUserMessage.isEmpty() && conversationHistory.isEmpty() || 
               (!conversationHistory.isEmpty() && conversationHistory.last().toObject()["role"].toString() != "user")) {
        // If we're retrying and the user message was popped off, re-add it
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = pendingUserMessage;
        conversationHistory.append(userMsg);
    }

    // Keep the payload small to speed up responses.
    trimConversationHistory(10);

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
    body["max_tokens"] = 300;

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
        // Network-level error — try next model
        if (retryCount + 1 < modelList.size()) {
            reply->deleteLater();
            retryWithNextModel();
            return;
        }
        QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QString statusInfo = httpStatus.isValid() ? QString(" [HTTP %1]").arg(httpStatus.toInt()) : "";
        responseText = "Connection error" + statusInfo + ": " + reply->errorString();
        if (!data.isEmpty()) {
            QJsonDocument errDoc = QJsonDocument::fromJson(data);
            if (!errDoc.isNull() && errDoc.object().contains("error")) {
                QString apiMsg = errDoc.object()["error"].toObject()["message"].toString();
                if (!apiMsg.isEmpty())
                    responseText += "\nDetails: " + apiMsg;
            }
        }
        if (!conversationHistory.isEmpty()) conversationHistory.removeLast();
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            QJsonObject root = doc.object();
            if (root.contains("error")) {
                // API returned an error — try next model
                if (retryCount + 1 < modelList.size()) {
                    reply->deleteLater();
                    retryWithNextModel();
                    return;
                }
                QJsonObject errObj = root["error"].toObject();
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

    if (!m_isWeatherBot)
        responseText = processResponse(responseText);
    appendMessage(m_isWeatherBot ? "MeteoBot" : "Assistant", responseText, false);
    reply->deleteLater();
}

QString ChatBotDialog::processResponse(const QString &response)
{
    QString result = response;
    QRegularExpression rx(R"(\[EXECUTE_SQL\](.*?)\[/EXECUTE_SQL\])", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = rx.globalMatch(result);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString sql = match.captured(1).trimmed();
        QString execResult = executeSqlCommand(sql);
        result.replace(match.captured(0), execResult);
    }
    return result;
}

QString ChatBotDialog::executeSqlCommand(const QString &sql)
{
    // Safety: block DDL and any non-DML statements
    QString upper = sql.toUpper().trimmed();
    if (upper.startsWith("DROP") || upper.startsWith("TRUNCATE") ||
        upper.startsWith("ALTER") || upper.startsWith("CREATE")) {
        return "[Blocked: DDL commands are not allowed]";
    }

    const bool isInsert = upper.startsWith("INSERT");
    const bool isUpdate = upper.startsWith("UPDATE");
    const bool isDelete = upper.startsWith("DELETE");
    const bool isSelect = upper.startsWith("SELECT");
    if (!isInsert && !isUpdate && !isDelete && !isSelect) {
        return "[Blocked: only SELECT, INSERT, UPDATE, and DELETE are allowed]";
    }

    QSqlQuery q;
    if (q.exec(sql)) {
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
        return "[Error: " + q.lastError().text() + "]";
    }
}
