#include "loreguidewidget.h"
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSvgRenderer>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QSet>

static QPixmap svgToPixmap(const QString &path, int w, int h)
{
    QSvgRenderer renderer(path);
    QPixmap pix(w, h);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    renderer.render(&p);
    return pix;
}

LoreGuideWidget::LoreGuideWidget(QMediaPlayer *bgMusic, QWidget *parent)
    : QWidget(parent),
      m_network(new QNetworkAccessManager(this)),
            m_apiKey("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"),
      m_retryCount(0),
            m_rateLimitRetries(0),
            m_sqlRetryCount(0),
      m_bgMusic(bgMusic)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumSize(300, 420);
    resize(340, 520);

    m_pixNormal  = svgToPixmap(":/assets/guide_normal.svg",  110, 147);
    m_pixExcited = svgToPixmap(":/assets/guide_excited.svg", 110, 147);
    m_pixConfused= svgToPixmap(":/assets/guide_confused.svg",110, 147);

    // Keep model strategy identical to the standard chatbot.
    m_modelList << "openrouter/auto"
                << "meta-llama/llama-3.1-8b-instruct:free"
                << "mistralai/mistral-7b-instruct:free"
                << "openrouter/free";

    setupUI();

    QJsonObject sys;
    sys["role"] = "system";
    sys["content"] = buildSystemPrompt();
    m_history.append(sys);

    connect(m_network, &QNetworkAccessManager::finished,
            this, &LoreGuideWidget::onApiReplyFinished);
}

LoreGuideWidget::~LoreGuideWidget() {}

void LoreGuideWidget::setupUI()
{
    setStyleSheet(R"(
        LoreGuideWidget {
            background-color: #1a1208;
            border: 2px solid #8B6F47;
            border-radius: 16px;
        }
    )");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Title bar ────────────────────────────────────────────
    m_titleBar = new QFrame(this);
    m_titleBar->setFixedHeight(44);
    m_titleBar->setStyleSheet(
        "background-color: #2a1e10;"
        "border-top-left-radius: 14px;"
        "border-top-right-radius: 14px;"
        "border-bottom: 1px solid #3d2e18;");
    m_titleBar->installEventFilter(this);

    QHBoxLayout *tl = new QHBoxLayout(m_titleBar);
    tl->setContentsMargins(14, 0, 10, 0);

    QLabel *titleLbl = new QLabel("HammerDown Assistant", m_titleBar);
    titleLbl->setStyleSheet("color:#d4a96a; font-size:14px; font-weight:700;"
                            "font-family:'Georgia',serif; background:transparent; border:none;");

    QPushButton *closeBtn = new QPushButton("✕", m_titleBar);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton{background:transparent;color:#8B6F47;border:none;font-size:15px;font-weight:bold;border-radius:14px;}"
        "QPushButton:hover{background:rgba(217,83,79,0.3);color:#ff6b6b;}");
    connect(closeBtn, &QPushButton::clicked, this, &LoreGuideWidget::closeRequested);

    tl->addWidget(titleLbl);
    tl->addStretch();
    tl->addWidget(closeBtn);

    m_sizeGrip = new QSizeGrip(this);
    m_sizeGrip->setFixedSize(14, 14);

    // ── Character image ───────────────────────────────────────
    m_characterLabel = new QLabel(this);
    m_characterLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_characterLabel->setFixedHeight(130);
    m_characterLabel->setStyleSheet("background:transparent; border:none;");
    m_characterLabel->setPixmap(m_pixNormal);
    m_characterLabel->setVisible(m_animationUnlocked);

    // ── Chat bubble scroll area ───────────────────────────────
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea{background:transparent;border:none;}"
        "QScrollBar:vertical{width:4px;background:#1a1208;}"
        "QScrollBar::handle:vertical{background:#3d2e18;border-radius:2px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}");

    m_bubbleContainer = new QWidget();
    m_bubbleContainer->setStyleSheet("background:transparent;");
    m_bubbleLayout = new QVBoxLayout(m_bubbleContainer);
    m_bubbleLayout->setContentsMargins(10, 8, 10, 8);
    m_bubbleLayout->setSpacing(6);
    m_bubbleLayout->addStretch();
    m_scrollArea->setWidget(m_bubbleContainer);

    // ── Typing indicator ─────────────────────────────────────
    m_typingLabel = new QLabel("Assistant is typing...", this);
    m_typingLabel->setStyleSheet(
        "color:#8B6F47;font-size:11px;font-style:italic;"
        "background:transparent;border:none;padding:2px 14px;");
    m_typingLabel->setVisible(false);

    // ── Input bar ────────────────────────────────────────────
    QFrame *inputFrame = new QFrame(this);
    inputFrame->setFixedHeight(52);
    inputFrame->setStyleSheet(
        "background-color:#2a1e10;"
        "border-bottom-left-radius:14px;"
        "border-bottom-right-radius:14px;"
        "border-top:1px solid #3d2e18;");

    QHBoxLayout *il = new QHBoxLayout(inputFrame);
    il->setContentsMargins(10, 7, 10, 7);
    il->setSpacing(8);

    m_input = new QLineEdit(inputFrame);
    m_input->setPlaceholderText("Type a message...");
    m_input->setStyleSheet(
        "QLineEdit{background:#1a1208;color:#e8dcc8;border:1px solid #3d2e18;"
        "border-radius:16px;padding:5px 12px;font-size:12px;font-family:'Georgia',serif;}"
        "QLineEdit:focus{border:1px solid #8B6F47;}");
    connect(m_input, &QLineEdit::returnPressed, this, &LoreGuideWidget::sendMessage);

    m_sendBtn = new QPushButton("➤", inputFrame);
    m_sendBtn->setFixedSize(34, 34);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton{background:#8B6F47;color:#fffbe0;border:none;border-radius:17px;"
        "font-size:15px;font-weight:bold;}"
        "QPushButton:hover{background:#d4a96a;}"
        "QPushButton:pressed{background:#5c4a2a;}");
    connect(m_sendBtn, &QPushButton::clicked, this, &LoreGuideWidget::sendMessage);

    il->addWidget(m_input);
    il->addWidget(m_sendBtn);

    root->addWidget(m_titleBar);
    root->addWidget(m_characterLabel);
    root->addWidget(m_scrollArea, 1);
    root->addWidget(m_typingLabel);
    root->addWidget(inputFrame);

    // Opening message
    QTimer::singleShot(400, this, [this](){
        appendBubble("Welcome to HammerDown!\nI'm your AI assistant. Ask me anything about this app - employees, clients, orders, equipment, suppliers, or any feature!", false);
    });
}

void LoreGuideWidget::setAnimationUnlocked(bool enabled)
{
    m_animationUnlocked = enabled;
    if (m_characterLabel) {
        m_characterLabel->setVisible(enabled);
        if (enabled)
            setCharacterState(Normal);
    }
}

void LoreGuideWidget::setCharacterState(CharacterState state)
{
    if (!m_animationUnlocked) return;

    switch (state) {
    case Excited:  m_characterLabel->setPixmap(m_pixExcited);  break;
    case Confused: m_characterLabel->setPixmap(m_pixConfused); break;
    default:       m_characterLabel->setPixmap(m_pixNormal);   break;
    }
}

bool LoreGuideWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragStartPos = mouseEvent->globalPosition().toPoint();
                m_windowStartPos = pos();
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

    return QWidget::eventFilter(watched, event);
}

void LoreGuideWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_sizeGrip) {
        const int margin = 6;
        m_sizeGrip->move(width() - m_sizeGrip->width() - margin,
                         height() - m_sizeGrip->height() - margin);
        m_sizeGrip->raise();
    }
}

void LoreGuideWidget::appendBubble(const QString &text, bool isUser)
{
    QFrame *bubble = new QFrame(m_bubbleContainer);
    bubble->setObjectName(isUser ? "userBubble" : "botBubble");

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(12, 8, 12, 8);
    bubbleLayout->setSpacing(2);

    QLabel *senderLabel = new QLabel(isUser ? "You" : "Assistant", bubble);
    senderLabel->setObjectName("senderLabel");

    QLabel *messageLabel = new QLabel(text, bubble);
    messageLabel->setObjectName("messageText");
    messageLabel->setWordWrap(true);
    messageLabel->setTextFormat(Qt::PlainText);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    bubbleLayout->addWidget(senderLabel);
    bubbleLayout->addWidget(messageLabel);

    bubble->setMaximumWidth(280);

    if (isUser) {
        bubble->setStyleSheet(
            "#userBubble {"
            " background-color: #8B6F47;"
            " border-radius: 12px;"
            " border-bottom-right-radius: 4px;"
            "}"
            "#senderLabel { color: #f5e6cc; font-size: 11px; font-weight: 600; background: transparent; border: none; }"
            "#messageText { color: #ffffff; font-size: 13px; background: transparent; border: none; }");
    } else {
        bubble->setStyleSheet(
            "#botBubble {"
            " background-color: #2a1e10;"
            " border: 1px solid #3d2e18;"
            " border-radius: 12px;"
            " border-bottom-left-radius: 4px;"
            "}"
            "#senderLabel { color: #d4a96a; font-size: 11px; font-weight: 600; background: transparent; border: none; }"
            "#messageText { color: #e8dcc8; font-size: 13px; background: transparent; border: none; }");
    }

    QHBoxLayout *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    if (isUser) {
        row->addStretch();
        row->addWidget(bubble);
    } else {
        row->addWidget(bubble);
        row->addStretch();
    }

    // Insert before the final stretch
    int idx = m_bubbleLayout->count() - 1;
    m_bubbleLayout->insertLayout(idx, row);

    // Scroll to bottom
    QTimer::singleShot(50, this, [this](){
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum());
    });
}

void LoreGuideWidget::sendMessage()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;
    m_input->clear();

    appendBubble(text, true);
    setCharacterState(Confused);

    m_typingLabel->setVisible(true);
    m_sendBtn->setEnabled(false);
    m_input->setEnabled(false);

    QString localResponse;
    if (handleLocalCommand(text, &localResponse)) {
        m_typingLabel->setVisible(false);
        m_sendBtn->setEnabled(true);
        m_input->setEnabled(true);
        appendBubble(localResponse, false);
        setCharacterState(Excited);
        QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
        return;
    }

    if (isImageRequest(text))
        callImageApi(extractImagePrompt(text));
    else
        callApi(text);
}

bool LoreGuideWidget::handleLocalCommand(const QString &text, QString *responseOut)
{
    if (!responseOut) return false;

    QString trimmed = text.trimmed();
    QString lower = trimmed.toLower();

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

QString LoreGuideWidget::handleAddRandomOrders(int count)
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

int LoreGuideWidget::getNextId(const QString &tableName, const QString &idColumn, int fallback)
{
    QSqlQuery q;
    QString sql = QString("SELECT NVL(MAX(%1), 0) + 1 FROM %2").arg(idColumn, tableName);
    if (q.exec(sql) && q.next()) {
        int value = q.value(0).toInt();
        return value > 0 ? value : fallback;
    }
    return fallback;
}

QStringList LoreGuideWidget::getColumnValuesFromCheckConstraints(const QString &tableName, const QString &columnName)
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

QString LoreGuideWidget::getColumnDefaultValue(const QString &tableName, const QString &columnName)
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

QStringList LoreGuideWidget::getDistinctColumnValues(const QString &tableName, const QString &columnName)
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

QStringList LoreGuideWidget::getAllowedColumnValues(const QString &tableName, const QString &columnName)
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

QString LoreGuideWidget::handleAddRandomEmployees(int count)
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

QString LoreGuideWidget::handleAddRandomClients(int count)
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

QString LoreGuideWidget::handleAddRandomSuppliers(int count)
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

QString LoreGuideWidget::handleAddRandomEquipment(int count)
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

void LoreGuideWidget::callApi(const QString &userMessage, bool isSystemRetry)
{
    if (!userMessage.isEmpty()) {
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = userMessage;
        m_history.append(userMsg);
        m_pendingUserMessage = userMessage;
        m_retryCount = 0;
        m_rateLimitRetries = 0;
        if (!isSystemRetry) {
            m_sqlRetryCount = 0;
        }
    } else if ((!m_pendingUserMessage.isEmpty() && m_history.isEmpty()) ||
               (!m_history.isEmpty() && m_history.last().toObject()["role"].toString() != "user")) {
        // If we are retrying and the user message was popped off, re-add it.
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = m_pendingUserMessage;
        m_history.append(userMsg);
    }

    // Keep prompt aligned with latest live DB context for every request.
    QString freshPrompt = buildSystemPrompt();
    for (int i = 0; i < m_history.size(); ++i) {
        QJsonObject msg = m_history[i].toObject();
        if (msg["role"].toString() == "system") {
            msg["content"] = freshPrompt;
            m_history[i] = msg;
            break;
        }
    }

    trimConversationHistory(6);

    QString currentModel = m_modelList.value(m_retryCount, m_modelList.first());

    QUrl url("https://openrouter.ai/api/v1/chat/completions");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    req.setRawHeader("HTTP-Referer", "https://hammerdown.app");
    req.setRawHeader("X-Title", "HammerDown Assistant");
    req.setTransferTimeout(60000);

    // Configure SSL to avoid TLS issues on Windows.
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(sslConfig);

    QJsonObject body;
    body["model"] = currentModel;
    body["messages"] = m_history;
    body["max_tokens"] = 500;
    body["temperature"] = 0.2;
    body["top_p"] = 0.8;

    QNetworkReply *reply = m_network->post(req, QJsonDocument(body).toJson());
    reply->setProperty("replyType", "chat");
}

void LoreGuideWidget::retryWithNextModel()
{
    m_retryCount++;
    if (m_retryCount < m_modelList.size()) {
        if (!m_history.isEmpty()) m_history.removeLast();
        callApi("");
    }
}

void LoreGuideWidget::trimConversationHistory(int maxNonSystemMessages)
{
    if (maxNonSystemMessages <= 0) return;

    QJsonArray systemMessages;
    QJsonArray nonSystemMessages;

    for (const QJsonValue &val : m_history) {
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

    m_history = trimmed;
}

bool LoreGuideWidget::isImageRequest(const QString &text) const
{
    QString lower = text.toLower();
    bool hasImageWord = lower.contains("image") || lower.contains("photo") || lower.contains("picture") ||
                        lower.contains("illustration") || lower.contains("art") || lower.contains("draw") || lower.contains("paint");
    bool hasImageVerb = lower.contains("generate") || lower.contains("draw") || lower.contains("create") ||
                        lower.contains("render") || lower.contains("paint") || lower.contains("make");
    return hasImageVerb && hasImageWord;
}

QString LoreGuideWidget::extractImagePrompt(const QString &text) const
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

void LoreGuideWidget::callImageApi(const QString &prompt)
{
    QUrl url("https://ancient-hat-ee03.yassinebenmustapha05.workers.dev/generate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(60000);

    QJsonObject body;
    body["prompt"] = prompt;
    body["size"] = "1024x1024";

    QJsonDocument doc(body);
    QNetworkReply *reply = m_network->post(request, doc.toJson());
    reply->setProperty("replyType", "image");
    reply->setProperty("prompt", prompt);
}

void LoreGuideWidget::appendImageBubble(const QPixmap &pixmap, const QString &caption)
{
    QWidget *bubbleWidget = new QWidget(m_bubbleContainer);
    bubbleWidget->setMaximumWidth(240);
    
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleWidget);
    bubbleLayout->setContentsMargins(8, 8, 8, 8);
    bubbleLayout->setSpacing(4);

    QLabel *imageLabel = new QLabel(bubbleWidget);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setPixmap(pixmap.scaledToWidth(220, Qt::SmoothTransformation));

    QLabel *captionLabel = new QLabel(caption, bubbleWidget);
    captionLabel->setWordWrap(true);
    captionLabel->setStyleSheet(
        "color:#d4a96a;"
        "font-size:11px;"
        "font-family:'Georgia',serif;"
        "font-style:italic;");

    bubbleLayout->addWidget(imageLabel);
    bubbleLayout->addWidget(captionLabel);

    bubbleWidget->setStyleSheet(
        "background-color:#2a1e10;"
        "border:1px solid #8B6F47;"
        "border-radius:12px 12px 12px 4px;"
        "padding:8px 12px;");

    QHBoxLayout *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(bubbleWidget);
    row->addStretch();

    int idx = m_bubbleLayout->count() - 1;
    m_bubbleLayout->insertLayout(idx, row);

    QTimer::singleShot(50, this, [this](){
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum());
    });
}

void LoreGuideWidget::onApiReplyFinished(QNetworkReply *reply)
{
    QString replyType = reply->property("replyType").toString();
    const QString replyUrl = reply->url().toString();
    const bool isWorkerImage = replyUrl.contains("workers.dev/generate");

    if (replyType == "image" || isWorkerImage) {
        m_typingLabel->setVisible(false);
        m_input->setEnabled(true);
        m_sendBtn->setEnabled(true);

        QByteArray data = reply->readAll();
        QString prompt = reply->property("prompt").toString();
        if (reply->error() != QNetworkReply::NoError) {
            appendBubble("Image request failed: " + reply->errorString(), false);
            reply->deleteLater();
            setCharacterState(Normal);
            return;
        }

        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
        if (contentType.startsWith("image/")) {
            QPixmap pix;
            if (pix.loadFromData(data))
                appendImageBubble(pix, prompt);
            else
                appendBubble("Image decode failed.", false);
            setCharacterState(Excited);
            QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            if (obj.contains("url")) {
                QUrl imageUrl(obj["url"].toString());
                QNetworkReply *imgReply = m_network->get(QNetworkRequest(imageUrl));
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
                    appendImageBubble(pix, prompt);
                else
                    appendBubble("Image decode failed.", false);
                setCharacterState(Excited);
                QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
            } else if (obj.contains("b64") || obj.contains("b64_json")) {
                QString b64 = obj.contains("b64") ? obj["b64"].toString()
                             : obj["b64_json"].toString();
                QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw))
                    appendImageBubble(pix, prompt);
                else
                    appendBubble("Image decode failed.", false);
                setCharacterState(Excited);
                QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
            } else if (obj.contains("data") && obj["data"].isArray()) {
                QJsonArray images = obj["data"].toArray();
                if (!images.isEmpty()) {
                    QJsonObject first = images.first().toObject();
                    if (first.contains("b64_json")) {
                        QByteArray raw = QByteArray::fromBase64(first["b64_json"].toString().toUtf8());
                        QPixmap pix;
                        if (pix.loadFromData(raw))
                            appendImageBubble(pix, prompt);
                        else
                            appendBubble("Image decode failed.", false);
                        setCharacterState(Excited);
                        QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                    } else if (first.contains("url")) {
                        QUrl imageUrl(first["url"].toString());
                        QNetworkReply *imgReply = m_network->get(QNetworkRequest(imageUrl));
                        imgReply->setProperty("replyType", "image_download");
                        imgReply->setProperty("prompt", prompt);
                    } else {
                        appendBubble("Image request failed: unsupported response format.", false);
                        setCharacterState(Normal);
                    }
                } else {
                    appendBubble("Image request failed: no image data returned.", false);
                    setCharacterState(Normal);
                }
            } else {
                appendBubble("Image request failed: unsupported response format.", false);
                setCharacterState(Normal);
            }
        } else {
            QString text = QString::fromUtf8(data);
            QRegularExpression rx("data:image/[^;]+;base64,([A-Za-z0-9+/=]+)");
            QRegularExpressionMatch match = rx.match(text);
            if (match.hasMatch()) {
                QByteArray raw = QByteArray::fromBase64(match.captured(1).toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw))
                    appendImageBubble(pix, prompt);
                else
                    appendBubble("Image decode failed.", false);
                setCharacterState(Excited);
                QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
            } else {
                appendBubble("Image request failed: invalid JSON response.", false);
                setCharacterState(Normal);
            }
        }

        reply->deleteLater();
        return;
    }

    if (replyType == "image_download") {
        m_typingLabel->setVisible(false);
        m_input->setEnabled(true);
        m_sendBtn->setEnabled(true);

        QByteArray raw = reply->readAll();
        QPixmap pix;
        if (pix.loadFromData(raw)) {
            appendImageBubble(pix, reply->property("prompt").toString());
            setCharacterState(Excited);
            QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
        } else {
            appendBubble("Image download failed.", false);
            setCharacterState(Normal);
        }
        reply->deleteLater();
        return;
    }

    m_typingLabel->setVisible(false);
    m_input->setEnabled(true);
    m_sendBtn->setEnabled(true);

    QByteArray data = reply->readAll();
    QString responseText;

    if (reply->error() != QNetworkReply::NoError) {
        QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        int statusCode = httpStatus.isValid() ? httpStatus.toInt() : 0;

        if (statusCode == 429 && m_rateLimitRetries < 2) {
            m_rateLimitRetries++;
            reply->deleteLater();
            if (!m_history.isEmpty()) m_history.removeLast();
            int delayMs = m_rateLimitRetries * 3000;
            m_typingLabel->setVisible(true);
            m_input->setEnabled(false);
            m_sendBtn->setEnabled(false);
            QTimer::singleShot(delayMs, this, [this]() {
                callApi("");
            });
            return;
        }

        if (m_retryCount + 1 < m_modelList.size()) {
            reply->deleteLater();
            m_rateLimitRetries = 0;
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
        if (!m_history.isEmpty()) m_history.removeLast();
        setCharacterState(Confused);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            QJsonObject root = doc.object();
            if (root.contains("error")) {
                QJsonObject errObj = root["error"].toObject();
                int errCode = errObj["code"].toInt();
                if (errCode == 429 && m_rateLimitRetries < 2) {
                    m_rateLimitRetries++;
                    reply->deleteLater();
                    if (!m_history.isEmpty()) m_history.removeLast();
                    int delayMs = m_rateLimitRetries * 3000;
                    m_typingLabel->setVisible(true);
                    m_input->setEnabled(false);
                    m_sendBtn->setEnabled(false);
                    QTimer::singleShot(delayMs, this, [this]() {
                        callApi("");
                    });
                    return;
                }

                if (m_retryCount + 1 < m_modelList.size()) {
                    reply->deleteLater();
                    m_rateLimitRetries = 0;
                    retryWithNextModel();
                    return;
                }

                QString errMsg = errObj["message"].toString();
                QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                QString statusInfo = httpStatus.isValid() ? QString(" [HTTP %1]").arg(httpStatus.toInt()) : "";
                responseText = QString("API error%1: %2").arg(statusInfo, errMsg);
                if (!m_history.isEmpty()) m_history.removeLast();
                setCharacterState(Confused);
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
                        if (m_retryCount + 1 < m_modelList.size()) {
                            reply->deleteLater();
                            retryWithNextModel();
                            return;
                        }
                        setCharacterState(Confused);
                    } else {
                        QJsonObject assistantMsg;
                        assistantMsg["role"] = "assistant";
                        assistantMsg["content"] = responseText;
                        m_history.append(assistantMsg);
                        setCharacterState(Excited);
                        QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                    }
                }
            }
        } else {
            responseText = QString::fromUtf8(data).left(300);
            setCharacterState(Confused);
        }
    }

    if (responseText.trimmed().isEmpty()) {
        QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QString statusText = status.isValid() ? QString(" (HTTP %1)").arg(status.toInt()) : QString();
        responseText = "Assistant returned an empty response" + statusText + ".";
        setCharacterState(Confused);
    }

    QString pendingUpper = m_pendingUserMessage.trimmed().toUpper();
    bool isDirectSql = pendingUpper.startsWith("SELECT") || pendingUpper.startsWith("INSERT") ||
                       pendingUpper.startsWith("UPDATE") || pendingUpper.startsWith("DELETE");
    if ((responseText.startsWith("Connection error") || responseText.startsWith("API error")) && isDirectSql) {
        responseText = "AI service is unavailable right now. Executing your SQL directly:\n" + executeSqlCommand(m_pendingUserMessage);
    }

    responseText = processResponse(responseText);
    appendBubble(responseText, false);
    reply->deleteLater();
}

QString LoreGuideWidget::processResponse(const QString &response)
{
    QString result = response;

    // Match EXECUTE_SQL tags case-insensitively and tolerate whitespace/newlines around tag names.
    QRegularExpression taggedRx(
        R"(\[\s*EXECUTE_SQL\s*\](.*?)\[\s*/\s*EXECUTE_SQL\s*\])",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);

    QStringList successfulSqls;
    QRegularExpressionMatchIterator it = taggedRx.globalMatch(result);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString sql = match.captured(1).trimmed();

        // Sanitize: if AI put multiple alternatives or junk inside the tags, keep only the first valid SQL statement.
        // Remove lines starting with "---", "OR", "-- ", or that are clearly not SQL.
        QStringList sqlLines = sql.split('\n');
        QStringList cleanLines;
        bool foundSqlStart = false;
        for (const QString &line : sqlLines) {
            QString stripped = line.trimmed();
            // Skip separator lines and "OR" alternatives
            if (stripped.startsWith("---") || stripped.startsWith("===") ||
                stripped.compare("OR", Qt::CaseInsensitive) == 0 ||
                stripped.startsWith("-- OR") || stripped.startsWith("--OR")) {
                if (foundSqlStart) break;  // Stop at separator after SQL started = AI gave alternatives
                continue;
            }
            // Skip empty lines before SQL
            if (stripped.isEmpty() && !foundSqlStart) continue;

            // Check if this line starts actual SQL
            QString upper = stripped.toUpper();
            if (!foundSqlStart && (upper.startsWith("SELECT") || upper.startsWith("INSERT") ||
                                    upper.startsWith("UPDATE") || upper.startsWith("DELETE"))) {
                foundSqlStart = true;
            }
            if (foundSqlStart) {
                cleanLines << line;
            }
        }
        QString cleanSql = cleanLines.isEmpty() ? sql : cleanLines.join('\n').trimmed();

        QString execResult = executeSqlCommand(cleanSql);
        if ((execResult.startsWith("[Blocked:") || execResult.startsWith("[Error:")) && m_sqlRetryCount < 3) {
            m_sqlRetryCount++;
            QString msg = "Your previous SQL command failed with the following error:\n" +
                          execResult +
                          "\nPlease review the database constraints, correct the SQL, and provide a new [EXECUTE_SQL] block without markdown.";
            if (!successfulSqls.isEmpty()) {
                msg += "\n\nNOTE: The following queries in your previous response were ALREADY SUCCESSFUL. Do NOT generate them again. Only provide the remaining queries and the corrected query:\n" + successfulSqls.join("\n");
            }
            QTimer::singleShot(100, this, [this, msg]() { callApi(msg, true); });
            result.replace(match.captured(0), "⚙️ Correcting query based on constraints (Retry " + QString::number(m_sqlRetryCount) + "/3)...");
            return result; // Stop processing further tags; wait for retry
        }
        successfulSqls.append(cleanSql);
        result.replace(match.captured(0), execResult);
    }

    // If model output was truncated and closing tag is missing, execute the tail anyway.
    QRegularExpression openOnlyRx(R"(\[\s*EXECUTE_SQL\s*\](.*)$)",
                                  QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch openOnlyMatch = openOnlyRx.match(result);
    if (openOnlyMatch.hasMatch() && !result.contains(QRegularExpression(R"(\[\s*/\s*EXECUTE_SQL\s*\])", QRegularExpression::CaseInsensitiveOption))) {
        QString sql = openOnlyMatch.captured(1).trimmed();
        QString execResult = executeSqlCommand(sql);
        if ((execResult.startsWith("[Blocked:") || execResult.startsWith("[Error:")) && m_sqlRetryCount < 3) {
            m_sqlRetryCount++;
            QString msg = "Your previous SQL command failed with the following error:\n" +
                          execResult +
                          "\nPlease review the database constraints, correct the SQL, and provide a new [EXECUTE_SQL] block without markdown.";
            if (!successfulSqls.isEmpty()) {
                msg += "\n\nNOTE: The following queries in your previous response were ALREADY SUCCESSFUL. Do NOT generate them again. Only provide the remaining queries and the corrected query:\n" + successfulSqls.join("\n");
            }
            QTimer::singleShot(100, this, [this, msg]() { callApi(msg, true); });
            result.replace(openOnlyMatch.captured(0), "⚙️ Correcting query based on constraints (Retry " + QString::number(m_sqlRetryCount) + "/3)...");
            return result;
        }
        result.replace(openOnlyMatch.captured(0), execResult);
    }

    return result;
}

QString LoreGuideWidget::executeSqlCommand(const QString &sql)
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

    // If AI includes explanatory prose in the SQL block, keep only the SQL
    // statement starting at the first supported SQL keyword.
    QRegularExpression firstSqlKeywordRx(
        R"(\b(SELECT|INSERT|UPDATE|DELETE|DECLARE|BEGIN)\b)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch keywordMatch = firstSqlKeywordRx.match(trimmedSql);
    if (keywordMatch.hasMatch() && keywordMatch.capturedStart() > 0) {
        QString prefix = trimmedSql.left(keywordMatch.capturedStart()).trimmed();
        if (!prefix.isEmpty()) {
            trimmedSql = trimmedSql.mid(keywordMatch.capturedStart()).trimmed();
        }
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

    // --- Helper lambda to extract a quoted string value from a SQL expression ---
    auto extractQuotedValue = [](const QString &expr) -> QString {
        QString v = expr.trimmed();
        if (v.startsWith('\'') && v.endsWith('\'') && v.size() >= 2) {
            v = v.mid(1, v.size() - 2);
            v.replace("''", "'");
        }
        return v;
    };

    // --- Helper lambda to split a top-level CSV (respecting quotes and parens) ---
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

    // --- Tunisian validation constants (must match DB CHECK constraints exactly) ---
    const QStringList allowedFirstNames = {
        "Mohamed", "Ahmed", "Yassine", "Amine", "Sami", "Walid", "Karim", "Fares", "Aymen", "Nader",
        "Ines", "Sarra", "Amira", "Meriem", "Rania", "Nour", "Asma", "Lina", "Yasmine", "Wafa",
        "Ali", "Hedi", "Slim", "Lotfi", "Habib", "Fatma", "Salma", "Dorra", "Mariem", "Olfa"
    };
    const QStringList allowedLastNames = {
        "Ben Ali", "Trabelsi", "Mansour", "Gharbi", "Jaziri", "Ayari", "Mejri", "Kefi", "Chaari", "Bouazizi",
        "Ben Salem", "Boussetta", "Sfaxi", "Mabrouk", "Haddad", "Cherif", "Khalfallah", "Dhaouadi", "Zribi", "Dridi",
        "Ben Amor", "Souissi", "Hamdi", "Bouzid", "Jebali"
    };
    const QStringList allowedOrderTypes = {"Chair", "Table", "Cabinet", "Wardrobe", "Other"};
    const QStringList allowedOrderStatuses = {"Pending", "Processing", "Completed", "Cancelled"};
    const QStringList allowedPaymentStatuses = {"Unpaid", "Partial", "Paid"};
    const QStringList allowedEmployeeStatuses = {"Active", "Inactive", "On Leave"};
    const QStringList allowedClientStatuses = {"Active", "Inactive", "Suspended"};
    const QStringList allowedGenders = {"Male", "Female", "Other"};
    const QStringList allowedEquipmentStatuses = {"Available", "In Use", "Under Maintenance", "Retired"};
    const QStringList allowedSupplierStatuses = {"Active", "Inactive", "Suspended"};
    const QRegularExpression phone8Rx("^\\d{8}$");

    // --- Helper lambda: check if a value is in a list (case-insensitive) ---
    auto isInList = [](const QString &value, const QStringList &list) -> bool {
        for (const QString &item : list) {
            if (value.compare(item, Qt::CaseInsensitive) == 0)
                return true;
        }
        return false;
    };

    // --- Helper lambda: validate a phone number (8 digits) ---
    auto validatePhone = [&phone8Rx](const QString &phone) -> bool {
        return phone8Rx.match(phone).hasMatch();
    };

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

        // Validate Tunisian first name
        const int fnIdx = insertColumns.indexOf("FIRST_NAME");
        if (fnIdx >= 0) {
            QString fnValue = extractQuotedValue(insertValues.at(fnIdx));
            if (!fnValue.isEmpty() && !isInList(fnValue, allowedFirstNames)) {
                return "[Blocked: FIRST_NAME must be a Tunisian name. Allowed: Mohamed, Ahmed, Yassine, Amine, Sami, Walid, Karim, Fares, Ines, Sarra, Amira, Meriem, Rania, Nour, Asma, Yasmine, etc.]";
            }
        }

        // Validate Tunisian last name
        const int lnIdx = insertColumns.indexOf("LAST_NAME");
        if (lnIdx >= 0) {
            QString lnValue = extractQuotedValue(insertValues.at(lnIdx));
            if (!lnValue.isEmpty() && !isInList(lnValue, allowedLastNames)) {
                return "[Blocked: LAST_NAME must be a Tunisian family name. Allowed: Ben Ali, Trabelsi, Mansour, Gharbi, Jaziri, Ayari, Mejri, Kefi, Haddad, Cherif, Dridi, etc.]";
            }
        }

        // Validate phone number (8 digits) on all tables
        const int phoneIdx = insertColumns.indexOf("PHONE_NUMBER");
        if (phoneIdx >= 0) {
            QString phoneValue = extractQuotedValue(insertValues.at(phoneIdx));
            if (!phoneValue.isEmpty() && !validatePhone(phoneValue)) {
                return "[Blocked: PHONE_NUMBER must be exactly 8 digits (Tunisian format, no +216). Example: 20123456]";
            }
        }

        // Validate GENDER (CLIENTS table)
        if (insertTableName == "CLIENTS") {
            const int genderIdx = insertColumns.indexOf("GENDER");
            if (genderIdx >= 0) {
                QString genderValue = extractQuotedValue(insertValues.at(genderIdx));
                if (!genderValue.isEmpty() && !isInList(genderValue, allowedGenders)) {
                    return "[Blocked: CLIENTS.GENDER must be one of Male, Female, Other]";
                }
            }
            const int cStatusIdx = insertColumns.indexOf("STATUS");
            if (cStatusIdx >= 0) {
                QString cStatusValue = extractQuotedValue(insertValues.at(cStatusIdx));
                if (!cStatusValue.isEmpty() && !isInList(cStatusValue, allowedClientStatuses)) {
                    return "[Blocked: CLIENTS.STATUS must be one of Active, Inactive, Suspended]";
                }
            }
        }

        // Validate EMPLOYEE_STATUS (EMPLOYEES table)
        if (insertTableName == "EMPLOYEES") {
            const int empStatusIdx = insertColumns.indexOf("EMPLOYEE_STATUS");
            if (empStatusIdx >= 0) {
                QString empStatusValue = extractQuotedValue(insertValues.at(empStatusIdx));
                if (!empStatusValue.isEmpty() && !isInList(empStatusValue, allowedEmployeeStatuses)) {
                    return "[Blocked: EMPLOYEE_STATUS must be one of Active, Inactive, On Leave (NOT Terminated)]";
                }
            }
        }

        // Validate ORDER_TYPE, ORDER_STATUS, PAYMENT_STATUS (ORDERS table)
        if (insertTableName == "ORDERS") {
            const int typeIdx = insertColumns.indexOf("ORDER_TYPE");
            if (typeIdx >= 0) {
                QString typeValue = extractQuotedValue(insertValues.at(typeIdx));
                if (!typeValue.isEmpty() && !isInList(typeValue, allowedOrderTypes)) {
                    return "[Blocked: ORDERS.ORDER_TYPE must be one of Chair, Table, Cabinet, Wardrobe, Other]";
                }
            }
            const int statusIdx = insertColumns.indexOf("ORDER_STATUS");
            if (statusIdx >= 0) {
                QString statusValue = extractQuotedValue(insertValues.at(statusIdx));
                if (!isInList(statusValue, allowedOrderStatuses)) {
                    return "[Blocked: ORDERS.ORDER_STATUS must be one of Pending, Processing, Completed, Cancelled]";
                }
            }
            const int payIdx = insertColumns.indexOf("PAYMENT_STATUS");
            if (payIdx >= 0) {
                QString payValue = extractQuotedValue(insertValues.at(payIdx));
                if (!isInList(payValue, allowedPaymentStatuses)) {
                    return "[Blocked: ORDERS.PAYMENT_STATUS must be one of Unpaid, Partial, Paid]";
                }
            }
        }

        // Validate EQUIPMENT.STATUS
        if (insertTableName == "EQUIPMENT") {
            const int eqStatusIdx = insertColumns.indexOf("STATUS");
            if (eqStatusIdx >= 0) {
                QString eqStatusValue = extractQuotedValue(insertValues.at(eqStatusIdx));
                if (!eqStatusValue.isEmpty() && !isInList(eqStatusValue, allowedEquipmentStatuses)) {
                    return "[Blocked: EQUIPMENT.STATUS must be one of Available, In Use, Under Maintenance, Retired]";
                }
            }
        }

        // Validate SUPPLIERS.ACCOUNT_STATUS
        if (insertTableName == "SUPPLIERS") {
            const int supStatusIdx = insertColumns.indexOf("ACCOUNT_STATUS");
            if (supStatusIdx >= 0) {
                QString supStatusValue = extractQuotedValue(insertValues.at(supStatusIdx));
                if (!supStatusValue.isEmpty() && !isInList(supStatusValue, allowedSupplierStatuses)) {
                    return "[Blocked: SUPPLIERS.ACCOUNT_STATUS must be one of Active, Inactive, Suspended]";
                }
            }
        }
    }

    // Constraint-aware guard for AI-generated UPDATE statements.
    if (isUpdate && !isPlSqlBlock) {
        // Parse SET clause assignments: UPDATE <table> SET col1 = val1, col2 = val2 WHERE ...
        QRegularExpression updateRx(
            R"(^\s*UPDATE\s+([A-Z0-9_]+)\s+SET\s+(.+?)(?:\s+WHERE\s+.+)?$)",
            QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch updateMatch = updateRx.match(trimmedSql);
        if (updateMatch.hasMatch()) {
            QString updateTableName = updateMatch.captured(1).toUpper();
            QString setClause = updateMatch.captured(2).trimmed();

            // Parse individual SET assignments
            QStringList assignments = splitTopLevelCsv(setClause);
            for (const QString &assignment : assignments) {
                int eqPos = assignment.indexOf('=');
                if (eqPos < 0) continue;
                QString colName = assignment.left(eqPos).trimmed().toUpper();
                colName.remove('"');
                QString valExpr = assignment.mid(eqPos + 1).trimmed();
                QString valStr = extractQuotedValue(valExpr);

                // Skip non-literal values (subqueries, functions, etc.)
                if (valExpr.trimmed().toUpper() == "NULL" || valExpr.contains('('))
                    continue;

                // Validate FIRST_NAME
                if (colName == "FIRST_NAME" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedFirstNames)) {
                        return "[Blocked: FIRST_NAME must be a Tunisian name. Allowed: Mohamed, Ahmed, Yassine, Amine, Sami, Walid, Karim, Ines, Sarra, Amira, Meriem, Nour, Asma, Yasmine, etc.]";
                    }
                }

                // Validate LAST_NAME
                if (colName == "LAST_NAME" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedLastNames)) {
                        return "[Blocked: LAST_NAME must be a Tunisian family name. Allowed: Ben Ali, Trabelsi, Mansour, Gharbi, Jaziri, Mejri, Kefi, Haddad, Cherif, Dridi, etc.]";
                    }
                }

                // Validate PHONE_NUMBER (8 digits)
                if (colName == "PHONE_NUMBER" && !valStr.isEmpty()) {
                    if (!validatePhone(valStr)) {
                        return "[Blocked: PHONE_NUMBER must be exactly 8 digits (Tunisian format, no +216). Example: 20123456]";
                    }
                }

                // Validate GENDER
                if (colName == "GENDER" && updateTableName == "CLIENTS" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedGenders)) {
                        return "[Blocked: CLIENTS.GENDER must be one of Male, Female, Other]";
                    }
                }

                // Validate CLIENT.STATUS
                if (colName == "STATUS" && updateTableName == "CLIENTS" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedClientStatuses)) {
                        return "[Blocked: CLIENTS.STATUS must be one of Active, Inactive, Suspended]";
                    }
                }

                // Validate EMPLOYEE_STATUS
                if (colName == "EMPLOYEE_STATUS" && updateTableName == "EMPLOYEES" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedEmployeeStatuses)) {
                        return "[Blocked: EMPLOYEE_STATUS must be one of Active, Inactive, On Leave (NOT Terminated)]";
                    }
                }

                // Validate ORDER_TYPE
                if (colName == "ORDER_TYPE" && updateTableName == "ORDERS" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedOrderTypes)) {
                        return "[Blocked: ORDERS.ORDER_TYPE must be one of Chair, Table, Cabinet, Wardrobe, Other]";
                    }
                }

                // Validate ORDER_STATUS
                if (colName == "ORDER_STATUS" && updateTableName == "ORDERS" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedOrderStatuses)) {
                        return "[Blocked: ORDERS.ORDER_STATUS must be one of Pending, Processing, Completed, Cancelled]";
                    }
                }

                // Validate PAYMENT_STATUS
                if (colName == "PAYMENT_STATUS" && updateTableName == "ORDERS" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedPaymentStatuses)) {
                        return "[Blocked: ORDERS.PAYMENT_STATUS must be one of Unpaid, Partial, Paid]";
                    }
                }

                // Validate EQUIPMENT.STATUS
                if (colName == "STATUS" && updateTableName == "EQUIPMENT" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedEquipmentStatuses)) {
                        return "[Blocked: EQUIPMENT.STATUS must be one of Available, In Use, Under Maintenance, Retired]";
                    }
                }

                // Validate SUPPLIERS.ACCOUNT_STATUS
                if (colName == "ACCOUNT_STATUS" && updateTableName == "SUPPLIERS" && !valStr.isEmpty()) {
                    if (!isInList(valStr, allowedSupplierStatuses)) {
                        return "[Blocked: SUPPLIERS.ACCOUNT_STATUS must be one of Active, Inactive, Suspended]";
                    }
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

QString LoreGuideWidget::buildSystemPrompt() const
{
    QString base = R"(You are "HammerDown AI Assistant" for a Tunisian carpentry company. You have FULL database access.

CRITICAL RULES FOR SQL OUTPUT:
- When you need to run SQL, put EXACTLY ONE valid Oracle SQL statement inside [EXECUTE_SQL]...[/EXECUTE_SQL] tags.
- Inside [EXECUTE_SQL] tags: ONLY pure SQL. NO text, NO comments, NO explanations, NO markdown, NO alternatives, NO "OR", NO dashes.
- NEVER put multiple statements or options inside one [EXECUTE_SQL] block.
- If you want to explain something, write it OUTSIDE the tags, BEFORE or AFTER them.
- The SQL MUST be a single complete valid Oracle SQL statement that can execute directly.
- Do NOT use semicolons inside the tags.
- Do NOT use FETCH FIRST syntax. Use ROWNUM for limiting rows.

DATABASE TABLES (Oracle) - columns marked [NN] are NOT NULL, [DEF=x] have defaults:

EMPLOYEES(EMPLOYEE_ID PK, FIRST_NAME [NN], LAST_NAME [NN], JOB_TITLE, EMAIL, PHONE_NUMBER, SALARY, PASSWORD, DEPARTMENT, AGE, HIRE_DATE [DEF=SYSDATE], EMPLOYEE_STATUS [DEF='Active'], ADDRESS [NN])
CLIENTS(CLIENT_ID PK, FIRST_NAME [NN], LAST_NAME [NN], EMAIL [UNIQUE], PHONE_NUMBER, ADDRESS, GENDER, AGE, ACCOUNT_BALANCE [DEF=0], REGISTRATION_DATE [DEF=SYSDATE], STATUS [DEF='Active'], ASSIGNED_EMPLOYEE_ID FK)
ORDERS(ORDER_ID PK, CLIENT_ID [NN] FK->CLIENTS, EMPLOYEE_ID [NN] FK->EMPLOYEES, ORDER_DATE [DEF=SYSDATE], ORDER_TYPE, ORDER_STATUS [DEF='Pending'], TOTAL_QUANTITY [DEF=0], TOTAL_PRICE [DEF=0], PAYMENT_STATUS [DEF='Unpaid'])
EQUIPMENT(EQUIPMENT_ID PK, EQUIPMENT_TYPE [NN], QUANTITY [DEF=0], UNIT_PRICE, STATUS [DEF='Available'], DESCRIPTION, EMPLOYEE_ID FK->EMPLOYEES, PURCHASE_DATE, LOCATION, NOTES, NEXT_MAINTENANCE, COUT_ACQUISITION, RESPONSABLE)
SUPPLIERS(SUPPLIER_ID PK, SUPPLIER_NAME [NN], EMAIL [UNIQUE], PHONE_NUMBER, ADDRESS, POSTAL_CODE, DELIVERY_RATING, QUALITY_RATING, ACCOUNT_STATUS [DEF='Active'])

EXACT ALLOWED VALUES (CHECK constraints - use EXACTLY these, case-sensitive):
- EMPLOYEE_STATUS: ONLY 'Active', 'Inactive', 'On Leave' (NOT 'Terminated')
- CLIENTS.STATUS: ONLY 'Active', 'Inactive', 'Suspended'
- CLIENTS.GENDER: ONLY 'Male', 'Female', 'Other'
- ORDER_TYPE: ONLY 'Chair', 'Wardrobe', 'Cabinet', 'Table', 'Other'
- ORDER_STATUS: ONLY 'Pending', 'Processing', 'Completed', 'Cancelled'
- PAYMENT_STATUS: ONLY 'Unpaid', 'Partial', 'Paid'
- EQUIPMENT.STATUS: ONLY 'Available', 'In Use', 'Under Maintenance', 'Retired'
- SUPPLIERS.ACCOUNT_STATUS: ONLY 'Active', 'Inactive', 'Suspended'

TUNISIAN DATA RULES:
- FIRST_NAME must be from: Mohamed, Ahmed, Yassine, Amine, Sami, Walid, Karim, Fares, Aymen, Nader, Ines, Sarra, Amira, Meriem, Rania, Nour, Asma, Lina, Yasmine, Wafa, Ali, Hedi, Slim, Lotfi, Habib, Fatma, Salma, Dorra, Mariem, Olfa
- LAST_NAME must be from: Ben Ali, Trabelsi, Mansour, Gharbi, Jaziri, Ayari, Mejri, Kefi, Chaari, Bouazizi, Ben Salem, Boussetta, Sfaxi, Mabrouk, Haddad, Cherif, Khalfallah, Dhaouadi, Zribi, Dridi, Ben Amor, Souissi, Hamdi, Bouzid, Jebali
- PHONE_NUMBER: exactly 8 digits, no country code. Example: '20123456'
- ADDRESS: Tunisian format. Example: '10 Avenue Habib Bourguiba, Tunis, Tunisia'
- EMAIL: use .tn domain
- For new IDs use: (SELECT NVL(MAX(ID_COL),0)+1 FROM TABLE)

CORRECT INSERT EXAMPLES:
- New employee: [EXECUTE_SQL]INSERT INTO EMPLOYEES (EMPLOYEE_ID, FIRST_NAME, LAST_NAME, JOB_TITLE, EMAIL, PHONE_NUMBER, SALARY, DEPARTMENT, AGE, EMPLOYEE_STATUS, ADDRESS) VALUES ((SELECT NVL(MAX(EMPLOYEE_ID),0)+1 FROM EMPLOYEES), 'Mohamed', 'Trabelsi', 'Carpenter', 'mohamed.trabelsi@hammerdown.tn', '20123456', 2500, 'Production', 30, 'Active', '10 Avenue Habib Bourguiba, Tunis, Tunisia')[/EXECUTE_SQL]
- New client: [EXECUTE_SQL]INSERT INTO CLIENTS (CLIENT_ID, FIRST_NAME, LAST_NAME, EMAIL, PHONE_NUMBER, ADDRESS, GENDER, AGE, ACCOUNT_BALANCE, STATUS) VALUES ((SELECT NVL(MAX(CLIENT_ID),0)+1 FROM CLIENTS), 'Ines', 'Mejri', 'ines.mejri@client.tn', '55123456', '5 Rue de Marseille, Sfax, Tunisia', 'Female', 28, 0, 'Active')[/EXECUTE_SQL]
- New order: [EXECUTE_SQL]INSERT INTO ORDERS (ORDER_ID, CLIENT_ID, EMPLOYEE_ID, ORDER_TYPE, ORDER_STATUS, TOTAL_QUANTITY, TOTAL_PRICE, PAYMENT_STATUS) VALUES ((SELECT NVL(MAX(ORDER_ID),0)+1 FROM ORDERS), 1, 1, 'Chair', 'Pending', 5, 500, 'Unpaid')[/EXECUTE_SQL]
- New equipment: [EXECUTE_SQL]INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, QUANTITY, UNIT_PRICE, STATUS, DESCRIPTION, LOCATION, RESPONSABLE, COUT_ACQUISITION) VALUES ((SELECT NVL(MAX(EQUIPMENT_ID),0)+1 FROM EQUIPMENT), 'Table Saw', 2, 1500, 'Available', 'Industrial table saw', 'Workshop A', 'Mohamed Trabelsi', 3000)[/EXECUTE_SQL]
- New supplier: [EXECUTE_SQL]INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, EMAIL, PHONE_NUMBER, ADDRESS, POSTAL_CODE, ACCOUNT_STATUS) VALUES ((SELECT NVL(MAX(SUPPLIER_ID),0)+1 FROM SUPPLIERS), 'Bois Tunisie SARL', 'contact@boistunisie.tn', '71234567', '20 Avenue de Carthage, Tunis, Tunisia', '1000', 'Active')[/EXECUTE_SQL]

CORRECT SELECT EXAMPLES:
- Show employees: [EXECUTE_SQL]SELECT * FROM (SELECT * FROM EMPLOYEES ORDER BY EMPLOYEE_ID) WHERE ROWNUM <= 20[/EXECUTE_SQL]
- Show orders: [EXECUTE_SQL]SELECT * FROM (SELECT * FROM ORDERS ORDER BY ORDER_ID) WHERE ROWNUM <= 20[/EXECUTE_SQL]

CORRECT UPDATE/DELETE EXAMPLES:
- Update order status: [EXECUTE_SQL]UPDATE ORDERS SET ORDER_STATUS = 'Completed' WHERE ORDER_ID = 10[/EXECUTE_SQL]
- Delete employee: [EXECUTE_SQL]DELETE FROM EMPLOYEES WHERE EMPLOYEE_ID = 5[/EXECUTE_SQL]

FORBIDDEN: DROP, TRUNCATE, ALTER, CREATE, GRANT, REVOKE, RENAME, BEGIN, DECLARE. Only SELECT/INSERT/UPDATE/DELETE.)";

    // Inject live database context so the AI knows what data exists
    if (QSqlDatabase::database().isOpen()) {
        QString liveContext = "\n\nLIVE DATABASE STATE (use these real values):";

        // Employee IDs + names (for FK references)
        QSqlQuery empQ;
        if (empQ.exec("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM (SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME FROM EMPLOYEES ORDER BY EMPLOYEE_ID) WHERE ROWNUM <= 25")) {
            QStringList empList;
            while (empQ.next()) {
                empList << QString("%1(%2 %3)").arg(empQ.value(0).toString(), empQ.value(1).toString(), empQ.value(2).toString());
            }
            if (!empList.isEmpty()) {
                liveContext += "\nAvailable EMPLOYEE_IDs: " + empList.join(", ");
            } else {
                liveContext += "\nEMPLOYEES table is EMPTY.";
            }
        }

        // Client IDs + names (for FK references)
        QSqlQuery cliQ;
        if (cliQ.exec("SELECT CLIENT_ID, FIRST_NAME, LAST_NAME FROM (SELECT CLIENT_ID, FIRST_NAME, LAST_NAME FROM CLIENTS ORDER BY CLIENT_ID) WHERE ROWNUM <= 25")) {
            QStringList cliList;
            while (cliQ.next()) {
                cliList << QString("%1(%2 %3)").arg(cliQ.value(0).toString(), cliQ.value(1).toString(), cliQ.value(2).toString());
            }
            if (!cliList.isEmpty()) {
                liveContext += "\nAvailable CLIENT_IDs: " + cliList.join(", ");
            } else {
                liveContext += "\nCLIENTS table is EMPTY.";
            }
        }

        // Supplier IDs + names
        QSqlQuery supQ;
        if (supQ.exec("SELECT SUPPLIER_ID, SUPPLIER_NAME FROM (SELECT SUPPLIER_ID, SUPPLIER_NAME FROM SUPPLIERS ORDER BY SUPPLIER_ID) WHERE ROWNUM <= 25")) {
            QStringList supList;
            while (supQ.next()) {
                supList << QString("%1(%2)").arg(supQ.value(0).toString(), supQ.value(1).toString());
            }
            if (!supList.isEmpty()) {
                liveContext += "\nAvailable SUPPLIER_IDs: " + supList.join(", ");
            } else {
                liveContext += "\nSUPPLIERS table is EMPTY.";
            }
        }

        // Equipment IDs
        QSqlQuery eqQ;
        if (eqQ.exec("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE FROM (SELECT EQUIPMENT_ID, EQUIPMENT_TYPE FROM EQUIPMENT ORDER BY EQUIPMENT_ID) WHERE ROWNUM <= 25")) {
            QStringList eqList;
            while (eqQ.next()) {
                eqList << QString("%1(%2)").arg(eqQ.value(0).toString(), eqQ.value(1).toString());
            }
            if (!eqList.isEmpty()) {
                liveContext += "\nAvailable EQUIPMENT_IDs: " + eqList.join(", ");
            } else {
                liveContext += "\nEQUIPMENT table is EMPTY.";
            }
        }

        // Row counts and next available IDs
        QStringList tables = {"EMPLOYEES", "CLIENTS", "ORDERS", "EQUIPMENT", "SUPPLIERS"};
        QStringList idCols = {"EMPLOYEE_ID", "CLIENT_ID", "ORDER_ID", "EQUIPMENT_ID", "SUPPLIER_ID"};
        for (int i = 0; i < tables.size(); ++i) {
            QSqlQuery countQ;
            QString sql = QString("SELECT COUNT(*), NVL(MAX(%1),0)+1 FROM %2").arg(idCols[i], tables[i]);
            if (countQ.exec(sql) && countQ.next()) {
                liveContext += QString("\n%1: %2 rows, next ID = %3").arg(tables[i]).arg(countQ.value(0).toInt()).arg(countQ.value(1).toInt());
            }
        }

        // Allowed ORDER_TYPE values from existing data
        QSqlQuery otQ;
        if (otQ.exec("SELECT DISTINCT ORDER_TYPE FROM ORDERS WHERE ORDER_TYPE IS NOT NULL")) {
            QStringList types;
            while (otQ.next()) types << otQ.value(0).toString();
            if (!types.isEmpty())
                liveContext += "\nExisting ORDER_TYPEs in use: " + types.join(", ");
        }

        base += liveContext;
    }

    return base;
}
