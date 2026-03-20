#include "loreguidewidget.h"
#include <QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSvgRenderer>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>

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
      m_bgMusic(bgMusic)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground, true);
    setMinimumSize(300, 420);
    resize(340, 520);

    m_pixNormal  = svgToPixmap(":/assets/guide_normal.svg",  110, 147);
    m_pixExcited = svgToPixmap(":/assets/guide_excited.svg", 110, 147);
    m_pixConfused= svgToPixmap(":/assets/guide_confused.svg",110, 147);

    // Model list with fallback (nvidia first since it was working)
    m_modelList << "nvidia/nemotron-nano-9b-v2:free"
                << "openrouter/free"
                << "meta-llama/llama-3.3-70b-instruct:free"
                << "google/gemma-3-27b-it:free";

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

    QLabel *titleLbl = new QLabel("✦  The Guiding Light", m_titleBar);
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
    m_typingLabel = new QLabel("Thinking...\nAnalyzing your request", this);
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
    m_input->setPlaceholderText("Speak to the guide...");
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
        appendBubble("Hello! I'm The Guiding Light assistant. How can I help you?", false);
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
    QLabel *bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setTextFormat(Qt::PlainText);

    if (isUser) {
        bubble->setStyleSheet(
            "background-color:#3d2e18;"
            "color:#e8dcc8;"
            "border:1px solid #5c4a2a;"
            "border-radius:12px 12px 4px 12px;"
            "padding:8px 12px;"
            "font-size:12px;"
            "font-family:'Georgia',serif;");
    } else {
        bubble->setStyleSheet(
            "background-color:#2a1e10;"
            "color:#d4a96a;"
            "border:1px solid #8B6F47;"
            "border-radius:12px 12px 12px 4px;"
            "padding:8px 12px;"
            "font-size:12px;"
            "font-family:'Georgia',serif;"
            "font-style:italic;");
    }

    bubble->setMaximumWidth(260);
    bubble->adjustSize();

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

    // Fade the bubble in
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(bubble);
    bubble->setGraphicsEffect(eff);
    eff->setOpacity(0.0);
    QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity", bubble);
    anim->setDuration(500);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

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

    if (isImageRequest(text))
        callImageApi(extractImagePrompt(text));
    else
        callApi(text);
}

void LoreGuideWidget::callApi(const QString &userMessage)
{
    if (!userMessage.isEmpty()) {
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = userMessage;
        m_history.append(userMsg);
        m_pendingUserMessage = userMessage;
        m_retryCount = 0;
    } else if (!m_pendingUserMessage.isEmpty()) {
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = m_pendingUserMessage;
        m_history.append(userMsg);
    }

    QString currentModel = m_modelList.value(m_retryCount, m_modelList.first());

    QJsonObject body;
    body["model"] = currentModel;
    body["messages"] = m_history;

    QNetworkRequest req(QUrl("https://openrouter.ai/api/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    req.setRawHeader("HTTP-Referer", "https://hammerdown.app");
    req.setRawHeader("X-Title", "HammerDown Assistant");
    req.setTransferTimeout(60000);

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(sslConfig);

    m_network->post(req, QJsonDocument(body).toJson());
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

    // Handle image responses
    if (replyType == "image") {
        m_typingLabel->setVisible(false);
        m_sendBtn->setEnabled(true);
        m_input->setEnabled(true);

        QByteArray data = reply->readAll();
        QString prompt = reply->property("prompt").toString();
        
        if (reply->error() != QNetworkReply::NoError) {
            appendBubble("Image generation failed: " + reply->errorString(), false);
            reply->deleteLater();
            setCharacterState(Normal);
            return;
        }

        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
        
        // Try direct image first
        if (contentType.startsWith("image/")) {
            QPixmap pix;
            if (pix.loadFromData(data)) {
                appendImageBubble(pix, prompt);
                setCharacterState(Excited);
                QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                reply->deleteLater();
                return;
            }
        }

        // Try as JSON
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            
            // Handle URL response
            if (obj.contains("url")) {
                QUrl imageUrl(obj["url"].toString());
                QNetworkReply *imgReply = m_network->get(QNetworkRequest(imageUrl));
                imgReply->setProperty("replyType", "image_download");
                imgReply->setProperty("prompt", prompt);
                reply->deleteLater();
                return;
            }
            
            // Handle base64 direct field
            if (obj.contains("image")) {
                QString imageField = obj["image"].toString();
                QString b64 = imageField;
                if (imageField.startsWith("data:image")) {
                    int comma = imageField.indexOf(',');
                    if (comma >= 0)
                        b64 = imageField.mid(comma + 1);
                }
                QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw)) {
                    appendImageBubble(pix, prompt);
                    setCharacterState(Excited);
                    QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                    reply->deleteLater();
                    return;
                }
            }
            
            // Handle b64 or b64_json
            if (obj.contains("b64") || obj.contains("b64_json")) {
                QString b64 = obj.contains("b64") ? obj["b64"].toString()
                             : obj["b64_json"].toString();
                QByteArray raw = QByteArray::fromBase64(b64.toUtf8());
                QPixmap pix;
                if (pix.loadFromData(raw)) {
                    appendImageBubble(pix, prompt);
                    setCharacterState(Excited);
                    QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                    reply->deleteLater();
                    return;
                }
            }
            
            // Handle data array
            if (obj.contains("data") && obj["data"].isArray()) {
                QJsonArray images = obj["data"].toArray();
                if (!images.isEmpty()) {
                    QJsonObject first = images.first().toObject();
                    if (first.contains("b64_json")) {
                        QByteArray raw = QByteArray::fromBase64(first["b64_json"].toString().toUtf8());
                        QPixmap pix;
                        if (pix.loadFromData(raw)) {
                            appendImageBubble(pix, prompt);
                            setCharacterState(Excited);
                            QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                            reply->deleteLater();
                            return;
                        }
                    } else if (first.contains("url")) {
                        QUrl imageUrl(first["url"].toString());
                        QNetworkReply *imgReply = m_network->get(QNetworkRequest(imageUrl));
                        imgReply->setProperty("replyType", "image_download");
                        imgReply->setProperty("prompt", prompt);
                        reply->deleteLater();
                        return;
                    }
                }
            }
        }
        
        // Try to parse as raw base64 in text
        QString text = QString::fromUtf8(data);
        QRegularExpression rx("data:image/[^;]+;base64,([A-Za-z0-9+/=]+)");
        QRegularExpressionMatch match = rx.match(text);
        if (match.hasMatch()) {
            QByteArray raw = QByteArray::fromBase64(match.captured(1).toUtf8());
            QPixmap pix;
            if (pix.loadFromData(raw)) {
                appendImageBubble(pix, prompt);
                setCharacterState(Excited);
                QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
                reply->deleteLater();
                return;
            }
        }
        
        // Last resort: try to load as raw image bytes
        QPixmap pix;
        if (pix.loadFromData(data)) {
            appendImageBubble(pix, prompt);
            setCharacterState(Excited);
            QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
            reply->deleteLater();
            return;
        }
        
        // All failed
        appendBubble("Image generation failed: unsupported response format.", false);
        setCharacterState(Normal);
        reply->deleteLater();
        return;
    }

    if (replyType == "image_download") {
        m_typingLabel->setVisible(false);
        m_sendBtn->setEnabled(true);
        m_input->setEnabled(true);

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

    // Handle text responses
    m_typingLabel->setVisible(false);
    m_sendBtn->setEnabled(true);
    m_input->setEnabled(true);

    QByteArray responseData = reply->readAll();
    QString responseText;

    if (reply->error() != QNetworkReply::NoError) {
        // Network error - try next model if available
        if (m_retryCount + 1 < m_modelList.size()) {
            reply->deleteLater();
            m_retryCount++;
            if (!m_history.isEmpty()) m_history.removeLast();
            callApi("");
            return;
        }
        QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QString statusInfo = httpStatus.isValid() ? QString(" [HTTP %1]").arg(httpStatus.toInt()) : "";
        responseText = "Connection error" + statusInfo + ". Please try again.";
        if (!m_history.isEmpty()) m_history.removeLast();
        setCharacterState(Confused);
    } else {
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject root = doc.object();
        
        if (root.contains("error")) {
            // API error - try next model if available
            if (m_retryCount + 1 < m_modelList.size()) {
                reply->deleteLater();
                m_retryCount++;
                if (!m_history.isEmpty()) m_history.removeLast();
                callApi("");
                return;
            }
            QJsonObject errObj = root["error"].toObject();
            QString errMsg = errObj["message"].toString();
            QVariant httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            QString statusInfo = httpStatus.isValid() ? QString(" [HTTP %1]").arg(httpStatus.toInt()) : "";
            responseText = QString("API error%1: %2").arg(statusInfo, errMsg);
            if (!m_history.isEmpty()) m_history.removeLast();
            setCharacterState(Confused);
        } else {
            QJsonArray choices = root["choices"].toArray();
            if (!choices.isEmpty()) {
                responseText = choices[0].toObject()["message"].toObject()["content"].toString().trimmed();
                QJsonObject assistantMsg;
                assistantMsg["role"] = "assistant";
                assistantMsg["content"] = responseText;
                m_history.append(assistantMsg);
                setCharacterState(Excited);
                QTimer::singleShot(2500, this, [this](){ setCharacterState(Normal); });
            } else {
                responseText = "No response received. Please try again.";
                if (!m_history.isEmpty()) m_history.removeLast();
                setCharacterState(Confused);
            }
        }
    }

    if (responseText.trimmed().isEmpty()) {
        QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QString statusText = status.isValid() ? QString(" (HTTP %1)").arg(status.toInt()) : QString();
        responseText = "Assistant returned an empty response" + statusText + ".";
    }

    responseText = processResponse(responseText);
    appendBubble(responseText, false);
    reply->deleteLater();
}

QString LoreGuideWidget::processResponse(const QString &response)
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

QString LoreGuideWidget::executeSqlCommand(const QString &sql)
{
    // Safety: block DDL commands
    QString upper = sql.toUpper().trimmed();
    if (upper.startsWith("DROP") || upper.startsWith("TRUNCATE") ||
        upper.startsWith("ALTER") || upper.startsWith("CREATE")) {
        return "[Blocked: DDL commands are not allowed]";
    }

    QSqlQuery q;
    if (q.exec(sql)) {
        int affected = q.numRowsAffected();
        if (upper.startsWith("INSERT"))
            return QString("[OK: Inserted %1 row(s)]").arg(affected);
        else if (upper.startsWith("UPDATE"))
            return QString("[OK: Updated %1 row(s)]").arg(affected);
        else if (upper.startsWith("DELETE"))
            return QString("[OK: Deleted %1 row(s)]").arg(affected);
        else
            return QString("[OK: %1 row(s) affected]").arg(affected);
    } else {
        return "[Error: " + q.lastError().text() + "]";
    }
}

QString LoreGuideWidget::buildSystemPrompt() const
{
    QString base = R"(You are the official assistant for "HammerDown" — a professional enterprise management desktop application built with Qt 6 / C++ and Oracle Database. You help users with managing employees, clients, orders, equipment, and suppliers. Answer questions clearly and accurately. You have FULL access to the database. Below is the current live data from the database — use it to answer user questions precisely.

DATABASE SCHEMA:
- EMPLOYEES(EMPLOYEE_ID NUMBER PK, FIRST_NAME, LAST_NAME, JOB_TITLE, EMAIL, PHONE_NUMBER, SALARY NUMBER(12,2), DEPARTMENT, AGE NUMBER, EMPLOYEE_STATUS)
- CLIENTS(CLIENT_ID NUMBER PK, FIRST_NAME, LAST_NAME, EMAIL, PHONE_NUMBER, ADDRESS, GENDER, AGE NUMBER, ACCOUNT_BALANCE NUMBER(15,2), STATUS)
- ORDERS(ORDER_ID NUMBER PK, CLIENT_ID FK, EMPLOYEE_ID FK, ORDER_TYPE, ORDER_STATUS, TOTAL_QUANTITY NUMBER, TOTAL_PRICE NUMBER(15,2), PAYMENT_STATUS)
- EQUIPMENT(EQUIPMENT_ID NUMBER PK, EQUIPMENT_TYPE, QUANTITY NUMBER, UNIT_PRICE NUMBER(12,2), STATUS, DESCRIPTION, LOCATION, NOTES, NEXT_MAINTENANCE DATE, COUT_ACQUISITION NUMBER(12,2), RESPONSABLE)
- SUPPLIERS(SUPPLIER_ID NUMBER PK, SUPPLIER_NAME, EMAIL, PHONE_NUMBER, ADDRESS, POSTAL_CODE, DELIVERY_RATING NUMBER(3,2), QUALITY_RATING NUMBER(3,2), ACCOUNT_STATUS)

You can INSERT, UPDATE, or DELETE data. When the user asks you to add, modify, or remove records, output the SQL inside [EXECUTE_SQL]...[/EXECUTE_SQL] tags. Rules:
- Use Oracle SQL syntax.
- Only one statement per tag. Use multiple tags for multiple statements.
- For INSERT: use the next available ID or let the sequence/trigger handle it.
- Always confirm what you did after the SQL.
- Example: "I'll add that employee now. [EXECUTE_SQL]INSERT INTO EMPLOYEES (FIRST_NAME, LAST_NAME, JOB_TITLE) VALUES ('John', 'Smith', 'Blacksmith')[/EXECUTE_SQL] Done! John Smith has been added."
- NEVER use DROP TABLE, TRUNCATE, ALTER TABLE, or any DDL commands. Only DML (INSERT, UPDATE, DELETE) is allowed.)";

    // ── Inject live database context ──
    QString dbContext;

    // Orders
    {
        QSqlQuery q("SELECT ORDER_ID, CLIENT_ID, EMPLOYEE_ID, ORDER_TYPE, ORDER_STATUS, TOTAL_QUANTITY, TOTAL_PRICE, PAYMENT_STATUS FROM ORDERS ORDER BY ORDER_ID");
        QStringList rows;
        while (q.next()) {
            rows << QString("  - Order #%1 | Client ID: %2 | Employee ID: %3 | Type: %4 | Status: %5 | Qty: %6 | Total: $%7 | Payment: %8")
                        .arg(q.value(0).toInt())
                        .arg(q.value(1).toInt())
                        .arg(q.value(2).toInt())
                        .arg(q.value(3).toString())
                        .arg(q.value(4).toString())
                        .arg(q.value(5).toInt())
                        .arg(q.value(6).toDouble(), 0, 'f', 2)
                        .arg(q.value(7).toString());
        }
        if (!rows.isEmpty())
            dbContext += "\n\nCURRENT ORDERS (" + QString::number(rows.size()) + "):\n" + rows.join("\n");
        else
            dbContext += "\n\nCURRENT ORDERS: None found in database.";
    }

    // Employees
    {
        QSqlQuery q("SELECT EMPLOYEE_ID, FIRST_NAME, LAST_NAME, JOB_TITLE, DEPARTMENT, EMPLOYEE_STATUS FROM EMPLOYEES ORDER BY EMPLOYEE_ID");
        QStringList rows;
        while (q.next()) {
            rows << QString("  - ID: %1 | %2 %3 | Title: %4 | Dept: %5 | Status: %6")
                        .arg(q.value(0).toInt())
                        .arg(q.value(1).toString())
                        .arg(q.value(2).toString())
                        .arg(q.value(3).toString())
                        .arg(q.value(4).toString())
                        .arg(q.value(5).toString());
        }
        if (!rows.isEmpty())
            dbContext += "\n\nCURRENT EMPLOYEES (" + QString::number(rows.size()) + "):\n" + rows.join("\n");
    }

    // Clients
    {
        QSqlQuery q("SELECT CLIENT_ID, FIRST_NAME, LAST_NAME, EMAIL, PHONE_NUMBER, STATUS FROM CLIENTS ORDER BY CLIENT_ID");
        QStringList rows;
        while (q.next()) {
            rows << QString("  - ID: %1 | %2 %3 | Email: %4 | Phone: %5 | Status: %6")
                        .arg(q.value(0).toInt())
                        .arg(q.value(1).toString())
                        .arg(q.value(2).toString())
                        .arg(q.value(3).toString())
                        .arg(q.value(4).toString())
                        .arg(q.value(5).toString());
        }
        if (!rows.isEmpty())
            dbContext += "\n\nCURRENT CLIENTS (" + QString::number(rows.size()) + "):\n" + rows.join("\n");
    }

    // Equipment
    {
        QSqlQuery q("SELECT EQUIPMENT_ID, EQUIPMENT_TYPE, QUANTITY, STATUS, LOCATION FROM EQUIPMENT ORDER BY EQUIPMENT_ID");
        QStringList rows;
        while (q.next()) {
            rows << QString("  - ID: %1 | Type: %2 | Qty: %3 | Status: %4 | Location: %5")
                        .arg(q.value(0).toInt())
                        .arg(q.value(1).toString())
                        .arg(q.value(2).toInt())
                        .arg(q.value(3).toString())
                        .arg(q.value(4).toString());
        }
        if (!rows.isEmpty())
            dbContext += "\n\nCURRENT EQUIPMENT (" + QString::number(rows.size()) + "):\n" + rows.join("\n");
    }

    // Suppliers
    {
        QSqlQuery q("SELECT SUPPLIER_ID, SUPPLIER_NAME, EMAIL, PHONE_NUMBER, ACCOUNT_STATUS FROM SUPPLIERS ORDER BY SUPPLIER_ID");
        QStringList rows;
        while (q.next()) {
            rows << QString("  - ID: %1 | Name: %2 | Email: %3 | Phone: %4 | Status: %5")
                        .arg(q.value(0).toInt())
                        .arg(q.value(1).toString())
                        .arg(q.value(2).toString())
                        .arg(q.value(3).toString())
                        .arg(q.value(4).toString());
        }
        if (!rows.isEmpty())
            dbContext += "\n\nCURRENT SUPPLIERS (" + QString::number(rows.size()) + "):\n" + rows.join("\n");
    }

    return base + dbContext;
}
