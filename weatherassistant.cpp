#include "weatherassistant.h"

#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSequentialAnimationGroup>
#include <QWheelEvent>
#include <QtMath>
#include <cmath>

WeatherAssistant::WeatherAssistant(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(760, 860);
    setMouseTracking(true);

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);

    m_closeButton = new QPushButton("Exit", this);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setFixedSize(78, 34);
    m_closeButton->setStyleSheet(
        "QPushButton { background: rgba(0,0,0,0.55); color: #f5e6d3; border: 1px solid rgba(255,255,255,0.2); border-radius: 12px; font-size: 14px; font-weight: bold; padding: 0 10px; }"
        "QPushButton:hover { background: rgba(180,30,30,0.85); color: white; border-color: rgba(255,160,160,0.9); }"
    );
    connect(m_closeButton, &QPushButton::clicked, this, &WeatherAssistant::hideAnimated);

    m_verticalScroll = new QScrollBar(Qt::Vertical, this);
    m_verticalScroll->setFixedWidth(14);
    m_verticalScroll->setStyleSheet(
        "QScrollBar:vertical { background: rgba(0,0,0,0.25); border: 1px solid rgba(255,255,255,0.18); border-radius: 6px; }"
        "QScrollBar::handle:vertical { background: rgba(193,127,62,0.95); border-radius: 5px; min-height: 36px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    );
    connect(m_verticalScroll, &QScrollBar::valueChanged, this, [this](int value) {
        if (!parentWidget()) return;
        const QRect bounds = parentWidget()->rect();
        const int minY = 0;
        const int maxY = qMax(0, bounds.height() - height());
        move(x(), qBound(minY, value, maxY));
    });
    m_verticalScroll->setSingleStep(12);
    m_verticalScroll->setPageStep(56);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &WeatherAssistant::onWeatherReceived);

    adviceNetworkManager = new QNetworkAccessManager(this);
    connect(adviceNetworkManager, &QNetworkAccessManager::finished, this, &WeatherAssistant::onAdviceReceived);

    hammerSound = new QSoundEffect(this);
    hammerSound->setSource(QUrl("qrc:/assets/botawk.mp3"));
    hammerSound->setVolume(0.4);

    ambientSound = new QSoundEffect(this);
    ambientSound->setSource(QUrl("qrc:/assets/forge.mp3"));
    ambientSound->setVolume(0.15);
    ambientSound->setLoopCount(QSoundEffect::Infinite);

    m_now = QDateTime::currentDateTime();

    m_renderTimer = new QTimer(this);
    m_renderTimer->setInterval(16);
    connect(m_renderTimer, &QTimer::timeout, this, &WeatherAssistant::onRenderTick);
    m_renderTimer->start();

    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &WeatherAssistant::onClockTick);
    m_clockTimer->start();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1800000);
    connect(m_refreshTimer, &QTimer::timeout, this, &WeatherAssistant::refreshWeather);
    m_refreshTimer->start();

    m_adviceFull = "Initializing weather intelligence...";
    m_adviceVisible.clear();
    rebuildDerivedData();
    rebuildHourlyForecast();
    refreshWeather();
}

WeatherAssistant::~WeatherAssistant() = default;

void WeatherAssistant::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = event->pos();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void WeatherAssistant::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || !parentWidget()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QRect bounds = parentWidget()->rect();
    const int currentX = x();
    const int targetY = mapToParent(event->pos() - m_dragOffset).y();
    const int minY = 0;
    const int maxY = qMax(0, bounds.height() - height());

    const int clampedY = qBound(minY, targetY, maxY);
    move(currentX, clampedY);
    if (m_verticalScroll) {
        QSignalBlocker blocker(m_verticalScroll);
        m_verticalScroll->setValue(clampedY);
    }
    event->accept();
}

void WeatherAssistant::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void WeatherAssistant::wheelEvent(QWheelEvent *event)
{
    if (!m_verticalScroll || !parentWidget()) {
        QWidget::wheelEvent(event);
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QWidget::wheelEvent(event);
        return;
    }

    const int steps = delta / 120;
    const int target = m_verticalScroll->value() - (steps * m_verticalScroll->singleStep() * 2);
    m_verticalScroll->setValue(qBound(m_verticalScroll->minimum(), target, m_verticalScroll->maximum()));
    event->accept();
}

void WeatherAssistant::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_closeButton) {
        m_closeButton->move(width() - 96, 18);
        m_closeButton->raise();
    }
    if (m_verticalScroll) {
        m_verticalScroll->setGeometry(width() - 20, 70, 12, height() - 88);
        m_verticalScroll->raise();

        if (parentWidget()) {
            const int maxY = qMax(0, parentWidget()->height() - height());
            m_verticalScroll->setRange(0, maxY);
            m_verticalScroll->setSingleStep(qMax(8, height() / 28));
            m_verticalScroll->setPageStep(qMax(24, height() / 5));
            QSignalBlocker blocker(m_verticalScroll);
            m_verticalScroll->setValue(y());
        }
    }
}

void WeatherAssistant::onClockTick()
{
    m_now = QDateTime::currentDateTime();
    update();
}

void WeatherAssistant::refreshWeather()
{
    const QString urlStr = "http://api.openweathermap.org/data/2.5/weather?q=Tunis,TN&units=metric&appid=3f674291abc67961141f0d7be861b6a0";
    QNetworkRequest request{QUrl(urlStr)};
    networkManager->get(request);
}

WeatherAssistant::WeatherKind WeatherAssistant::classifyKind(const QString &condition) const
{
    const QString c = condition.toLower();
    if (c.contains("thunder")) return Thunderstorm;
    if (c.contains("snow")) return Snowy;
    if (c.contains("rain") || c.contains("drizzle")) return Rainy;
    if (c.contains("cloud") || c.contains("mist") || c.contains("fog")) return Cloudy;
    return Clear;
}

void WeatherAssistant::rebuildDerivedData()
{
    int efficiency = 100;
    if (m_humidity > 70) efficiency -= 20;
    if (m_tempC > 30.0) efficiency -= 15;
    if (m_kind == Rainy || m_kind == Thunderstorm) efficiency -= 40;
    if (m_kind == Snowy) efficiency -= 18;
    if (m_kind == Cloudy) efficiency -= 6;
    m_workshopScore = qBound(10, efficiency, 100);

    m_lumberMoisture = 6.0 + (m_humidity * 0.15);

    int baseUv = 4;
    if (m_kind == Clear) baseUv = (m_tempC > 30.0) ? 9 : 7;
    else if (m_kind == Cloudy) baseUv = 4;
    else if (m_kind == Rainy) baseUv = 2;
    else if (m_kind == Thunderstorm) baseUv = 1;
    else if (m_kind == Snowy) baseUv = 3;
    m_uvIndex = qBound(0, baseUv, 11);
}

void WeatherAssistant::rebuildHourlyForecast()
{
    m_hourly.clear();
    for (int i = 0; i < 6; ++i) {
        HourForecast h;
        h.timeLabel = m_now.addSecs((i + 1) * 3600).time().toString("hh:mm");
        const double wave = qSin((m_globalTime * 0.5f) + (i * 0.65f));
        const double drift = qCos((m_globalTime * 0.25f) + (i * 0.4f));
        h.temp = m_tempC + (wave * 1.6) - (i * 0.15) + (drift * 0.4);
        int rain = 0;
        if (m_kind == Thunderstorm) rain = 70 + (i * 3);
        else if (m_kind == Rainy) rain = 45 + (i * 4);
        else if (m_kind == Cloudy) rain = 18 + (i * 2);
        else if (m_kind == Snowy) rain = 35 + (i * 2);
        else rain = 5 + i;
        h.rainChance = qBound(0, rain, 100);
        h.kind = m_kind;
        m_hourly.push_back(h);
    }
}

void WeatherAssistant::onWeatherReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject obj = doc.object();
    const QJsonObject main = obj.value("main").toObject();
    const QJsonObject wind = obj.value("wind").toObject();

    m_tempC = main.value("temp").toDouble();
    m_feelsLikeC = main.value("feels_like").toDouble(m_tempC);
    m_humidity = main.value("humidity").toInt();
    m_pressureHpa = main.value("pressure").toDouble();
    m_visibilityKm = obj.value("visibility").toDouble() / 1000.0;
    m_windKmh = wind.value("speed").toDouble() * 3.6;
    m_windDeg = wind.value("deg").toInt();

    const QJsonArray weatherArray = obj.value("weather").toArray();
    QString condition = "Clear";
    if (!weatherArray.isEmpty()) {
        condition = weatherArray.at(0).toObject().value("main").toString("Clear");
        m_conditionDescription = weatherArray.at(0).toObject().value("description").toString(condition);
    }

    m_conditionText = condition.toUpper();
    m_kind = classifyKind(condition);

    const QJsonObject sys = obj.value("sys").toObject();
    const qint64 sunriseEpoch = sys.value("sunrise").toVariant().toLongLong();
    const qint64 sunsetEpoch = sys.value("sunset").toVariant().toLongLong();
    if (sunriseEpoch > 0) m_sunrise = QDateTime::fromSecsSinceEpoch(sunriseEpoch);
    if (sunsetEpoch > 0) m_sunset = QDateTime::fromSecsSinceEpoch(sunsetEpoch);

    rebuildDerivedData();
    rebuildHourlyForecast();
    updateAdvice(m_tempC, m_humidity, condition);
    requestGroqAdvice(m_tempC, m_humidity, condition);

    hammerSound->play();
    reply->deleteLater();
    update();
}

void WeatherAssistant::requestGroqAdvice(double temp, int humidity, const QString &condition)
{
    const QString apiKey = qEnvironmentVariable("GROQ_API_KEY");
    if (apiKey.trimmed().isEmpty()) {
        return;
    }

    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = "You are a carpentry weather advisor. Return one concise practical recommendation in under 55 words.";

    QJsonObject usrMsg;
    usrMsg["role"] = "user";
    usrMsg["content"] = QString("Location: Tunis. Temp: %1C. Humidity: %2%%. Condition: %3. Provide one practical workshop advice.")
                        .arg(temp, 0, 'f', 1).arg(humidity).arg(condition);

    QJsonArray messages;
    messages.append(sysMsg);
    messages.append(usrMsg);

    QJsonObject body;
    body["model"] = "llama-3.3-70b-versatile";
    body["messages"] = messages;
    body["max_tokens"] = 120;
    body["temperature"] = 0.5;

    QNetworkRequest req(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());
    adviceNetworkManager->post(req, QJsonDocument(body).toJson());
}

void WeatherAssistant::onAdviceReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray choices = doc.object().value("choices").toArray();
    if (!choices.isEmpty()) {
        const QString advice = choices.at(0).toObject().value("message").toObject().value("content").toString().trimmed();
        if (!advice.isEmpty()) {
            m_adviceFull = advice;
            m_adviceVisible.clear();
            m_typewriterProgress = 0.0f;
        }
    }
    reply->deleteLater();
}

void WeatherAssistant::updateAdvice(double temp, int humidity, const QString &condition)
{
    const QString c = condition.toLower();

    if (humidity > 70) {
        m_adviceFull = "High humidity today: extend drying/curing windows, pre-condition lumber indoors, and avoid precision joinery until moisture stabilizes.";
    } else if (c.contains("rain") || c.contains("storm")) {
        m_adviceFull = "Rain risk: seal stock edges, keep materials elevated from floor, and prioritize protected indoor operations and moisture checks.";
    } else if (temp > 30.0) {
        m_adviceFull = "Hot conditions accelerate glue skin-over. Use smaller adhesive batches, reduce open-time exposure, and clamp sooner than usual.";
    } else if (temp < 12.0) {
        m_adviceFull = "Cool air slows curing: extend clamp time, warm adhesives before use, and verify bond strength before machining.";
    } else {
        m_adviceFull = "Good working conditions: ideal window for precision cutting, joinery calibration, and finish prep with consistent dimensional behavior.";
    }

    m_adviceVisible.clear();
    m_typewriterProgress = 0.0f;
}

QColor WeatherAssistant::temperatureColor(double temp) const
{
    if (temp >= 30.0) return QColor("#FF7043");
    if (temp <= 12.0) return QColor("#64B5F6");
    return QColor("#C17F3E");
}

QColor WeatherAssistant::uvColor(int uv) const
{
    if (uv >= 8) return QColor("#e53935");
    if (uv >= 6) return QColor("#fb8c00");
    if (uv >= 3) return QColor("#fbc02d");
    return QColor("#66bb6a");
}

void WeatherAssistant::onRenderTick()
{
    m_globalTime += 0.016f;

    if (m_openProgress < 1.0f) {
        m_openProgress = qMin(1.0f, m_openProgress + 0.03f);
    }
    if (m_numberProgress < 1.0f) {
        m_numberProgress = qMin(1.0f, m_numberProgress + 0.02f);
    }

    if (!m_adviceFull.isEmpty()) {
        m_typewriterProgress = qMin(1.0f, m_typewriterProgress + 0.02f);
        const int chars = qMin(m_adviceFull.size(), int(m_adviceFull.size() * m_typewriterProgress));
        m_adviceVisible = m_adviceFull.left(chars);
    }

    const float flashWindow = std::fmod(m_globalTime, 8.0f);
    m_lightningFlash = ((m_kind == Rainy || m_kind == Thunderstorm) && flashWindow < 0.12f);

    update();
}

void WeatherAssistant::showAnimated()
{
    if (!parentWidget()) return;

    m_isAnimatingOut = false;
    m_openProgress = 0.0f;
    m_numberProgress = 0.0f;
    m_typewriterProgress = 0.0f;
    m_adviceVisible.clear();

    const QRect parentRect = parentWidget()->rect();
    const QRect endRect((parentRect.width() - width()) / 2,
                        qMax(72, (parentRect.height() - height()) / 2),
                        width(), height());

    const int startW = int(width() * 0.85);
    const int startH = int(height() * 0.85);
    const QRect startRect(endRect.center().x() - startW / 2,
                          endRect.center().y() - startH / 2,
                          startW, startH);

    setGeometry(startRect);
    m_opacityEffect->setOpacity(0.0);
    show();
    raise();

    auto *geomAnim = new QPropertyAnimation(this, "geometry");
    geomAnim->setDuration(400);
    geomAnim->setStartValue(startRect);
    geomAnim->setEndValue(endRect);
    geomAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeAnim->setDuration(400);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *group = new QParallelAnimationGroup(this);
    group->addAnimation(geomAnim);
    group->addAnimation(fadeAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    if (m_verticalScroll) {
        m_verticalScroll->show();
        if (parentWidget()) {
            const int maxY = qMax(0, parentWidget()->height() - height());
            m_verticalScroll->setRange(0, maxY);
            m_verticalScroll->setSingleStep(qMax(8, height() / 28));
            m_verticalScroll->setPageStep(qMax(24, height() / 5));
        }
    }

    QTimer::singleShot(420, this, [this]() {
        if (!m_verticalScroll || !parentWidget()) return;
        QSignalBlocker blocker(m_verticalScroll);
        m_verticalScroll->setValue(y());
    });

    ambientSound->play();
}

void WeatherAssistant::hideAnimated()
{
    if (m_isAnimatingOut || !isVisible()) return;
    m_isAnimatingOut = true;

    const QRect endRect = geometry();
    const int outW = int(width() * 0.85);
    const int outH = int(height() * 0.85);
    const QRect startRect = geometry();
    const QRect targetRect(endRect.center().x() - outW / 2,
                           endRect.center().y() - outH / 2,
                           outW, outH);

    auto *geomAnim = new QPropertyAnimation(this, "geometry");
    geomAnim->setDuration(280);
    geomAnim->setStartValue(startRect);
    geomAnim->setEndValue(targetRect);
    geomAnim->setEasingCurve(QEasingCurve::InCubic);

    auto *fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeAnim->setDuration(280);
    fadeAnim->setStartValue(m_opacityEffect->opacity());
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::InCubic);

    auto *group = new QParallelAnimationGroup(this);
    group->addAnimation(geomAnim);
    group->addAnimation(fadeAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [this]() {
        hide();
        m_isAnimatingOut = false;
        ambientSound->stop();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void WeatherAssistant::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setRenderHint(QPainter::LosslessImageRendering, true);

    drawSkyBackground(p);
    drawGlassOverlay(p);

    const qreal mx = 22;
    qreal y = 18;

    drawTopSection(p, QRectF(mx, y, width() - mx * 2, 170)); y += 180;
    drawMainCards(p, QRectF(mx, y, width() - mx * 2, 180)); y += 190;
    drawExtendedBoxes(p, QRectF(mx, y, width() - mx * 2, 86)); y += 94;
    drawPerformanceBar(p, QRectF(mx, y, width() - mx * 2, 68)); y += 74;

    drawMoistureGauge(p, QRectF(mx, y, 170, 170));
    drawIconsRow(p, QRectF(mx + 182, y + 18, width() - mx * 2 - 182, 84));
    drawAdviceBox(p, QRectF(mx + 182, y + 108, width() - mx * 2 - 182, 62));
    y += 180;

    drawHourlyRow(p, QRectF(mx, y, width() - mx * 2, 96));
}

void WeatherAssistant::drawSkyBackground(QPainter &p)
{
    QLinearGradient sky(0, 0, 0, height());
    if (m_kind == Clear) {
        sky.setColorAt(0.0, QColor("#2B1B10"));
        sky.setColorAt(1.0, QColor("#6B4428"));
    } else if (m_kind == Cloudy) {
        sky.setColorAt(0.0, QColor("#24170F"));
        sky.setColorAt(1.0, QColor("#5A3A24"));
    } else if (m_kind == Snowy) {
        sky.setColorAt(0.0, QColor("#3A281A"));
        sky.setColorAt(1.0, QColor("#7A5A3D"));
    } else {
        sky.setColorAt(0.0, QColor("#20140D"));
        sky.setColorAt(1.0, QColor("#4A2F1E"));
    }
    p.fillRect(rect(), sky);

    if (m_kind == Clear) {
        const QPointF c(width() - 120, 95);
        const qreal sunR = 34;
        QRadialGradient corona(c, 72);
        corona.setColorAt(0, QColor(255, 220, 120, 180));
        corona.setColorAt(1, QColor(255, 220, 120, 0));
        p.setBrush(corona);
        p.setPen(Qt::NoPen);
        p.drawEllipse(c, 72, 72);

        p.setBrush(QColor(255, 205, 90));
        p.drawEllipse(c, sunR, sunR);

        p.save();
        p.translate(c);
        p.rotate(m_globalTime * 25.0);
        p.setPen(QPen(QColor(255, 220, 140, 220), 2));
        for (int i = 0; i < 12; ++i) {
            p.drawLine(QPointF(0, -44), QPointF(0, -58));
            p.rotate(30);
        }
        p.restore();
    }

    if (m_kind == Cloudy || m_kind == Rainy || m_kind == Thunderstorm) {
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 7; ++i) {
            const qreal px = std::fmod((i * 140.0) + (m_globalTime * (m_kind == Cloudy ? 8.0 : 14.0)), width() + 260.0) - 130.0;
            const qreal py = 70 + (i % 3) * 45 + qSin(m_globalTime + i) * 5.0;
            const QColor col = (m_kind == Cloudy) ? QColor(220, 225, 235, 70) : QColor(160, 170, 190, 62);
            p.setBrush(col);
            p.drawEllipse(QRectF(px, py, 95, 42));
            p.drawEllipse(QRectF(px + 26, py - 18, 72, 52));
            p.drawEllipse(QRectF(px + 56, py - 8, 64, 44));
        }
    }

    if (m_kind == Rainy || m_kind == Thunderstorm) {
        p.setPen(QPen(QColor(180, 210, 255, m_kind == Thunderstorm ? 170 : 130), m_kind == Thunderstorm ? 1.8 : 1.2));
        const int count = (m_kind == Thunderstorm) ? 220 : 150;
        for (int i = 0; i < count; ++i) {
            const qreal x = std::fmod((i * 19.0) + (m_globalTime * (m_kind == Thunderstorm ? 320.0 : 240.0)), width() + 120.0) - 60.0;
            const qreal y = std::fmod((i * 31.0) + (m_globalTime * (m_kind == Thunderstorm ? 420.0 : 300.0)), height() + 80.0) - 40.0;
            p.drawLine(QPointF(x, y), QPointF(x - 5.0, y + (m_kind == Thunderstorm ? 20.0 : 15.0)));
        }

        if (m_lightningFlash) {
            p.fillRect(rect(), QColor(220, 230, 255, 55));
        }
    }

    if (m_kind == Snowy) {
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 110; ++i) {
            const qreal x = std::fmod((i * 23.0) + (m_globalTime * 28.0), width() + 40.0) - 20.0;
            const qreal y = std::fmod((i * 41.0) + (m_globalTime * 68.0), height() + 30.0) - 15.0;
            const qreal r = 2.6 + (i % 4) * 0.4;
            p.save();
            p.translate(x, y);
            p.rotate((m_globalTime * 30.0) + i * 6.0);
            p.setPen(QPen(QColor(255, 255, 255, 210), 1));
            for (int a = 0; a < 3; ++a) {
                p.drawLine(QPointF(-r, 0), QPointF(r, 0));
                p.rotate(60);
            }
            p.restore();
        }
    }
}

void WeatherAssistant::drawGlassOverlay(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(45, 28, 17, 168));
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 20, 20);

    p.setPen(QPen(QColor(212, 175, 120, 52), 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(1, 1, -2, -2), 20, 20);
}

void WeatherAssistant::drawConditionIcon(QPainter &p, const QRectF &rect, WeatherKind kind, float t)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);

    const QPointF c = rect.center();
    const qreal s = qMin(rect.width(), rect.height()) * 0.45;

    if (kind == Clear) {
        p.setBrush(QColor(255, 205, 90));
        p.setPen(Qt::NoPen);
        p.drawEllipse(c, s * 0.55, s * 0.55);

        p.translate(c);
        p.rotate(t * 40.0f);
        p.setPen(QPen(QColor(255, 230, 160, 220), 3));
        for (int i = 0; i < 12; ++i) {
            p.drawLine(QPointF(0, -s * 0.9), QPointF(0, -s * 1.25));
            p.rotate(30);
        }
    } else if (kind == Cloudy) {
        const qreal dx = qSin(t * 1.1f) * 5.0;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(226, 233, 245, 220));
        p.drawEllipse(QRectF(c.x() - s + dx, c.y() - s * 0.2, s * 1.2, s * 0.85));
        p.drawEllipse(QRectF(c.x() - s * 0.4 + dx, c.y() - s * 0.55, s * 1.1, s * 0.95));
        p.drawEllipse(QRectF(c.x() + s * 0.2 + dx, c.y() - s * 0.3, s, s * 0.75));
    } else if (kind == Snowy) {
        p.translate(c);
        p.rotate(t * 55.0f);
        p.setPen(QPen(QColor(230, 240, 255, 230), 3));
        for (int i = 0; i < 3; ++i) {
            p.drawLine(QPointF(-s, 0), QPointF(s, 0));
            p.rotate(60);
        }
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(175, 188, 210, 220));
        p.drawEllipse(QRectF(c.x() - s, c.y() - s * 0.4, s * 1.2, s * 0.85));
        p.drawEllipse(QRectF(c.x() - s * 0.35, c.y() - s * 0.72, s * 1.0, s * 0.95));
        p.drawEllipse(QRectF(c.x() + s * 0.25, c.y() - s * 0.5, s * 0.95, s * 0.78));

        p.setPen(QPen(QColor(140, 200, 255, 240), 2));
        for (int i = -1; i <= 3; ++i) {
            const qreal rx = c.x() + (i * s * 0.35) + qSin(t * 3.0f + i) * 2.0;
            const qreal ry = c.y() + s * 0.15 + (i % 2) * 3;
            p.drawLine(QPointF(rx, ry), QPointF(rx - 4, ry + s * 0.55));
        }
    }

    p.restore();
}

void WeatherAssistant::drawTopSection(QPainter &p, const QRectF &rect)
{
    const QColor amber("#C17F3E");

    p.setPen(QColor(245, 230, 211, 220));
    QFont locFont = p.font();
    locFont.setPixelSize(15);
    locFont.setBold(true);
    p.setFont(locFont);
    p.drawText(QRectF(rect.x() + 8, rect.y() + 2, rect.width() * 0.55, 28), Qt::AlignLeft | Qt::AlignVCenter,
               QString("%1").arg(m_location));

    QFont dtFont = p.font();
    dtFont.setPixelSize(12);
    dtFont.setBold(false);
    p.setFont(dtFont);
    p.setPen(QColor(215, 210, 200, 200));
    p.drawText(QRectF(rect.x() + 8, rect.y() + 28, rect.width() * 0.55, 24), Qt::AlignLeft | Qt::AlignVCenter,
               QString("%1  |  %2").arg(m_now.date().toString("dddd, dd MMM yyyy"), m_now.time().toString("hh:mm:ss")));

    QFont condFont = p.font();
    condFont.setPixelSize(28);
    condFont.setBold(true);
    condFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
    p.setFont(condFont);
    p.setPen(QColor(0, 0, 0, 180)); p.drawText(QRectF(rect.x() + 10, rect.y() + 58, rect.width() - 140, 42), Qt::AlignLeft | Qt::AlignVCenter, m_conditionText);
    p.setPen(QColor(85, 45, 18, 180)); p.drawText(QRectF(rect.x() + 8, rect.y() + 56, rect.width() - 140, 42), Qt::AlignLeft | Qt::AlignVCenter, m_conditionText);
    p.setPen(amber); p.drawText(QRectF(rect.x() + 6, rect.y() + 54, rect.width() - 140, 42), Qt::AlignLeft | Qt::AlignVCenter, m_conditionText);

    p.setPen(QColor(240, 235, 230, 175));
    QFont desc = p.font();
    desc.setPixelSize(12);
    desc.setBold(false);
    p.setFont(desc);
    p.drawText(QRectF(rect.x() + 8, rect.y() + 103, rect.width() - 140, 22), Qt::AlignLeft | Qt::AlignVCenter, m_conditionDescription);

    drawConditionIcon(p, QRectF(rect.right() - 96, rect.y() + 18, 80, 80), m_kind, m_globalTime);
}

void WeatherAssistant::drawMainCards(QPainter &p, const QRectF &rect)
{
    const qreal gap = 14;
    const qreal w = (rect.width() - gap) * 0.5;
    const QRectF left(rect.x(), rect.y(), w, rect.height());
    const QRectF right(rect.x() + w + gap, rect.y(), w, rect.height());

    auto drawCardFrame = [&p](const QRectF &r) {
        p.setPen(QPen(QColor(255, 255, 255, 35), 1.0));
        p.setBrush(QColor(10, 14, 20, 130));
        p.drawRoundedRect(r, 14, 14);
    };
    drawCardFrame(left);
    drawCardFrame(right);

    const double tempV = m_tempC * m_numberProgress;
    const double feelsV = m_feelsLikeC * m_numberProgress;
    const int humV = int(m_humidity * m_numberProgress);

    p.setPen(QColor(235, 230, 220, 180));
    QFont l = p.font(); l.setPixelSize(12); l.setBold(true); p.setFont(l);
    p.drawText(QRectF(left.x() + 14, left.y() + 10, left.width() - 20, 18), Qt::AlignLeft, "TEMPERATURE");

    QFont v = p.font(); v.setPixelSize(52); v.setBold(true); p.setFont(v);
    p.setPen(temperatureColor(m_tempC));
    p.drawText(QRectF(left.x() + 14, left.y() + 28, left.width() - 24, 64), Qt::AlignLeft | Qt::AlignVCenter,
               QString::number(tempV, 'f', 1) + "°");

    QFont s = p.font(); s.setPixelSize(13); s.setBold(false); p.setFont(s);
    p.setPen(QColor(220, 215, 205, 185));
    p.drawText(QRectF(left.x() + 14, left.y() + 94, left.width() - 24, 22), Qt::AlignLeft, QString("Feels like %1°C").arg(feelsV, 0, 'f', 1));

    // Thermometer graphic
    QRectF tRect(left.x() + left.width() - 68, left.y() + 78, 40, 92);
    p.setPen(QPen(QColor(230, 235, 245, 190), 2));
    p.setBrush(QColor(20, 30, 40, 120));
    p.drawRoundedRect(QRectF(tRect.x() + 13, tRect.y(), 14, 68), 7, 7);
    p.drawEllipse(QRectF(tRect.x() + 7, tRect.y() + 58, 26, 26));

    const qreal fillRatio = qBound(0.0, (m_tempC + 10.0) / 50.0, 1.0) * m_numberProgress;
    const QRectF mercury(tRect.x() + 15, tRect.y() + 68 - (fillRatio * 66.0), 10, fillRatio * 66.0);
    QLinearGradient mg(mercury.topLeft(), mercury.bottomLeft());
    mg.setColorAt(0.0, QColor("#ef5350"));
    mg.setColorAt(1.0, QColor("#42a5f5"));
    p.setBrush(mg);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(mercury, 5, 5);
    p.drawEllipse(QRectF(tRect.x() + 10, tRect.y() + 61, 20, 20));

    p.setPen(QColor(235, 230, 220, 180));
    p.setFont(l);
    p.drawText(QRectF(right.x() + 14, right.y() + 10, right.width() - 20, 18), Qt::AlignLeft, "HUMIDITY");

    p.setFont(v);
    p.setPen(QColor("#64B5F6"));
    p.drawText(QRectF(right.x() + 14, right.y() + 28, right.width() - 24, 64), Qt::AlignLeft | Qt::AlignVCenter,
               QString::number(humV) + "%");

    // Water drop with fill + wave
    QPainterPath drop;
    const QPointF dc(right.x() + right.width() - 48, right.y() + 118);
    drop.moveTo(dc.x(), dc.y() - 44);
    drop.cubicTo(dc.x() + 24, dc.y() - 8, dc.x() + 16, dc.y() + 28, dc.x(), dc.y() + 42);
    drop.cubicTo(dc.x() - 16, dc.y() + 28, dc.x() - 24, dc.y() - 8, dc.x(), dc.y() - 44);

    p.setPen(QPen(QColor(215, 235, 255, 190), 2));
    p.setBrush(QColor(35, 60, 90, 100));
    p.drawPath(drop);

    p.save();
    p.setClipPath(drop);
    const qreal level = (1.0 - (m_humidity / 100.0) * m_numberProgress);
    const qreal topY = dc.y() - 44 + (88.0 * level);
    QPainterPath wave;
    wave.moveTo(dc.x() - 32, dc.y() + 42);
    for (int i = -32; i <= 32; ++i) {
        const qreal x = dc.x() + i;
        const qreal y = topY + qSin((i * 0.2) + m_globalTime * 4.0f) * 2.5;
        wave.lineTo(x, y);
    }
    wave.lineTo(dc.x() + 32, dc.y() + 42);
    wave.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(90, 170, 255, 180));
    p.drawPath(wave);
    p.restore();
}

void WeatherAssistant::drawExtendedBoxes(QPainter &p, const QRectF &rect)
{
    const int n = 6;
    const qreal gap = 8;
    const qreal boxW = (rect.width() - (gap * (n - 1))) / n;

    QStringList titles = {"WIND", "PRESS", "VIS", "UV", "SUNRISE", "SUNSET"};
    QStringList values;
    values << QString("%1 km/h").arg(m_windKmh * m_numberProgress, 0, 'f', 1)
           << QString("%1 hPa").arg(m_pressureHpa * m_numberProgress, 0, 'f', 0)
           << QString("%1 km").arg(m_visibilityKm * m_numberProgress, 0, 'f', 1)
           << QString::number(int(m_uvIndex * m_numberProgress))
           << (m_sunrise.isValid() ? m_sunrise.time().toString("hh:mm") : "--:--")
           << (m_sunset.isValid() ? m_sunset.time().toString("hh:mm") : "--:--");

    for (int i = 0; i < n; ++i) {
        const float delay = i * 0.08f;
        const float cardT = qBound(0.0f, (m_openProgress - delay) / 0.9f, 1.0f);
        const float ease = 1.0f - std::pow(1.0f - cardT, 3.0f);

        QRectF r(rect.x() + i * (boxW + gap), rect.y() + (1.0f - ease) * 16.0f, boxW, rect.height());

        p.setPen(QPen(QColor(255, 255, 255, int(32 * ease)), 1));
        p.setBrush(QColor(9, 14, 22, int(150 * ease)));
        p.drawRoundedRect(r, 10, 10);

        QFont tf = p.font(); tf.setPixelSize(10); tf.setBold(true); p.setFont(tf);
        p.setPen(QColor(220, 214, 205, int(190 * ease)));
        p.drawText(QRectF(r.x() + 8, r.y() + 6, r.width() - 16, 14), Qt::AlignLeft, titles[i]);

        QFont vf = p.font(); vf.setPixelSize(11); vf.setBold(false); p.setFont(vf);
        p.setPen(i == 3 ? uvColor(m_uvIndex) : QColor(190, 210, 235, int(220 * ease)));
        p.drawText(QRectF(r.x() + 8, r.y() + 32, r.width() - 16, 16), Qt::AlignLeft | Qt::AlignVCenter, values[i]);

        // tiny painter icon area
        const QPointF ic(r.x() + r.width() - 16, r.y() + 20);
        p.setPen(QPen(QColor(210, 225, 240, int(180 * ease)), 1.4));
        if (i == 0) {
            p.save(); p.translate(ic); p.rotate(m_windDeg * m_numberProgress); p.drawLine(QPointF(-8, 0), QPointF(8, 0)); p.drawLine(QPointF(8, 0), QPointF(4, -3)); p.drawLine(QPointF(8, 0), QPointF(4, 3)); p.restore();
        } else if (i == 1) {
            p.drawEllipse(ic, 7, 7);
            p.drawLine(ic, QPointF(ic.x() + qCos(m_globalTime) * 6, ic.y() - qSin(m_globalTime) * 6));
        } else if (i == 2) {
            p.drawEllipse(ic, 6, 6); p.drawLine(QPointF(ic.x() - 9, ic.y()), QPointF(ic.x() + 9, ic.y()));
        } else if (i == 3) {
            p.setPen(QPen(uvColor(m_uvIndex), 1.5)); p.drawEllipse(ic, 7, 7);
        } else if (i == 4 || i == 5) {
            p.drawArc(QRectF(ic.x() - 7, ic.y() - 7, 14, 14), 0, 180 * 16);
        }
    }
}

void WeatherAssistant::drawPerformanceBar(QPainter &p, const QRectF &rect)
{
    p.setPen(QPen(QColor(255, 255, 255, 34), 1));
    p.setBrush(QColor(10, 16, 24, 140));
    p.drawRoundedRect(rect, 10, 10);

    QFont t = p.font(); t.setPixelSize(12); t.setBold(true); p.setFont(t);
    p.setPen(QColor(235, 230, 220, 200));
    p.drawText(QRectF(rect.x() + 10, rect.y() + 6, rect.width() - 20, 16), Qt::AlignLeft, "WORKSHOP PERFORMANCE");

    QRectF bar(rect.x() + 10, rect.y() + 28, rect.width() - 20, 20);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(28, 32, 40));
    p.drawRoundedRect(bar, 10, 10);

    const qreal ratio = (m_workshopScore / 100.0) * m_numberProgress;
    QRectF fill(bar.x(), bar.y(), bar.width() * ratio, bar.height());
    QLinearGradient g(fill.topLeft(), fill.topRight());
    g.setColorAt(0.0, QColor("#e53935"));
    g.setColorAt(0.5, QColor("#fbc02d"));
    g.setColorAt(1.0, QColor("#43a047"));
    p.setBrush(g);
    p.drawRoundedRect(fill, 10, 10);

    const qreal shineX = bar.x() + std::fmod(m_globalTime * 120.0f, bar.width() + 40.0f) - 20.0;
    QRectF shine(shineX, bar.y(), 20, bar.height());
    QLinearGradient sg(shine.topLeft(), shine.topRight());
    sg.setColorAt(0.0, QColor(255, 255, 255, 0));
    sg.setColorAt(0.5, QColor(255, 255, 255, 65));
    sg.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.setBrush(sg);
    p.drawRoundedRect(shine, 10, 10);

    QFont v = p.font(); v.setPixelSize(11); v.setBold(true); p.setFont(v);
    p.setPen(QColor(245, 240, 225));
    p.drawText(QRectF(bar.x() + 8, bar.y() + 1, bar.width() - 16, bar.height() - 2), Qt::AlignCenter,
               QString::number(int(m_workshopScore * m_numberProgress)) + "%");

    QStringList pills;
    pills << (m_tempC > 30.0 ? "Heat" : (m_tempC < 12.0 ? "Cold" : "Temp OK"))
          << (m_humidity > 70 ? "Humid" : "Humidity OK")
          << m_conditionText;

    qreal x = rect.x() + 10;
    QFont pf = p.font(); pf.setPixelSize(10); pf.setBold(false); p.setFont(pf);
    for (const QString &pill : pills) {
        const int w = QFontMetrics(pf).horizontalAdvance(pill) + 20;
        QRectF pr(x, rect.y() + 52, w, 14);
        p.setBrush(QColor(25, 30, 40, 170));
        p.setPen(QPen(QColor(200, 190, 170, 90), 1));
        p.drawRoundedRect(pr, 7, 7);
        p.setPen(QColor(225, 220, 205, 200));
        p.drawText(pr, Qt::AlignCenter, pill);
        x += w + 8;
    }
}

void WeatherAssistant::drawMoistureGauge(QPainter &p, const QRectF &rect)
{
    const QPointF c = rect.center();
    const qreal radius = 56;
    const qreal angle = qBound(0.0, m_lumberMoisture / 30.0, 1.0) * m_numberProgress;

    p.setPen(QPen(QColor(255, 255, 255, 35), 1));
    p.setBrush(QColor(10, 16, 24, 140));
    p.drawRoundedRect(rect, 12, 12);

    QRectF arcR(c.x() - radius, c.y() - radius, radius * 2, radius * 2);
    p.setPen(QPen(QColor(40, 50, 65), 10, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcR, -210 * 16, 240 * 16);

    QConicalGradient cg(c, -120);
    cg.setColorAt(0.0, QColor("#43a047"));
    cg.setColorAt(0.5, QColor("#fbc02d"));
    cg.setColorAt(1.0, QColor("#e53935"));
    p.setPen(QPen(QBrush(cg), 10, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(arcR, -210 * 16, int(240 * angle) * 16);

    QFont v = p.font(); v.setPixelSize(24); v.setBold(true); p.setFont(v);
    p.setPen(QColor(230, 235, 240));
    p.drawText(QRectF(c.x() - 60, c.y() - 18, 120, 28), Qt::AlignCenter, QString::number(m_lumberMoisture * m_numberProgress, 'f', 1) + "%");

    QFont l = p.font(); l.setPixelSize(10); l.setBold(true); p.setFont(l);
    const QString rating = (m_lumberMoisture < 12.0) ? "IDEAL" : (m_lumberMoisture < 18.0 ? "ACCEPTABLE" : "TOO WET");
    p.setPen((rating == "IDEAL") ? QColor("#66bb6a") : (rating == "ACCEPTABLE" ? QColor("#fbc02d") : QColor("#ef5350")));
    p.drawText(QRectF(rect.x(), rect.bottom() - 22, rect.width(), 16), Qt::AlignCenter, rating);

    p.setPen(QColor(230, 225, 215, 170));
    p.drawText(QRectF(rect.x(), rect.y() + 8, rect.width(), 16), Qt::AlignCenter, "LUMBER MOISTURE");
}

void WeatherAssistant::drawIconsRow(QPainter &p, const QRectF &rect)
{
    const qreal gap = 18;
    const qreal d = 50;
    const qreal startX = rect.x() + 8;
    const qreal y = rect.y() + 2;

    for (int i = 0; i < 3; ++i) {
        const QRectF c(startX + i * (d + gap), y, d, d);
        p.setPen(QPen(QColor(255, 255, 255, 38), 1));
        p.setBrush(QColor(8, 14, 22, 155));
        p.drawEllipse(c);

        const QPointF cc = c.center();
        p.setPen(QPen(QColor(230, 220, 200), 2));
        if (i == 0) {
            p.drawLine(QPointF(cc.x(), cc.y() - 14), QPointF(cc.x(), cc.y() + 10));
            p.drawEllipse(QRectF(cc.x() - 5, cc.y() + 6, 10, 10));
        } else if (i == 1) {
            p.save(); p.translate(cc); p.rotate(m_globalTime * 70.0f); p.drawLine(QPointF(-12, 0), QPointF(12, 0)); p.drawLine(QPointF(12, 0), QPointF(7, -4)); p.drawLine(QPointF(12, 0), QPointF(7, 4)); p.restore();
        } else {
            p.drawEllipse(cc, 6, 6);
            p.save(); p.translate(cc); p.rotate(m_globalTime * 30.0f); for (int k = 0; k < 8; ++k) { p.drawLine(QPointF(0, -10), QPointF(0, -15)); p.rotate(45); } p.restore();
        }
    }
}

void WeatherAssistant::drawAdviceBox(QPainter &p, const QRectF &rect)
{
    const int pulse = int(100 + 55 * (0.5 + 0.5 * qSin(m_globalTime * 2.0f)));
    p.setPen(QPen(QColor(193, 127, 62, pulse), 1.4));
    p.setBrush(QColor(12, 18, 26, 155));
    p.drawRoundedRect(rect, 10, 10);

    QRectF accent(rect.x() + 3, rect.y() + 6, 3, rect.height() - 12);
    p.fillRect(accent, QColor(193, 127, 62, 200));

    QFont h = p.font(); h.setPixelSize(10); h.setBold(true); p.setFont(h);
    p.setPen(QColor("#C17F3E"));
    p.drawText(QRectF(rect.x() + 10, rect.y() + 4, rect.width() - 18, 14), Qt::AlignLeft, "SMART CARPENTRY ADVICE");

    QFont t = p.font(); t.setPixelSize(10); t.setBold(false); p.setFont(t);
    p.setPen(QColor(235, 230, 220, 215));
    p.drawText(QRectF(rect.x() + 10, rect.y() + 18, rect.width() - 16, rect.height() - 22), Qt::AlignLeft | Qt::TextWordWrap,
               m_adviceVisible.isEmpty() ? m_adviceFull : m_adviceVisible);
}

void WeatherAssistant::drawHourlyRow(QPainter &p, const QRectF &rect)
{
    p.setPen(QPen(QColor(255, 255, 255, 32), 1));
    p.setBrush(QColor(8, 14, 22, 120));
    p.drawRoundedRect(rect, 12, 12);

    QFont title = p.font(); title.setPixelSize(11); title.setBold(true); p.setFont(title);
    p.setPen(QColor(228, 222, 212, 210));
    p.drawText(QRectF(rect.x() + 10, rect.y() + 6, rect.width() - 20, 14), Qt::AlignLeft, "HOURLY FORECAST");

    if (m_hourly.isEmpty()) return;

    const qreal gap = 8;
    const qreal cardW = (rect.width() - 20 - (gap * 5)) / 6.0;
    const qreal cardH = 64;

    for (int i = 0; i < 6; ++i) {
        const float delay = i * 0.07f;
        const float t = qBound(0.0f, (m_openProgress - delay) / 0.95f, 1.0f);
        const float ease = 1.0f - std::pow(1.0f - t, 3.0f);
        QRectF c(rect.x() + 10 + i * (cardW + gap), rect.y() + 24 + (1.0f - ease) * 14.0f, cardW, cardH);

        p.setPen(QPen(QColor(255, 255, 255, int(30 * ease)), 1));
        p.setBrush(QColor(13, 20, 30, int(160 * ease)));
        p.drawRoundedRect(c, 8, 8);

        const HourForecast &h = m_hourly[i];
        QFont tf = p.font(); tf.setPixelSize(9); tf.setBold(true); p.setFont(tf);
        p.setPen(QColor(225, 220, 210, int(220 * ease)));
        p.drawText(QRectF(c.x() + 4, c.y() + 4, c.width() - 8, 12), Qt::AlignCenter, h.timeLabel);

        drawConditionIcon(p, QRectF(c.x() + c.width() / 2 - 11, c.y() + 18, 22, 22), h.kind, m_globalTime + i);

        QFont vf = p.font(); vf.setPixelSize(9); vf.setBold(false); p.setFont(vf);
        p.setPen(QColor(190, 210, 235, int(220 * ease)));
        p.drawText(QRectF(c.x() + 4, c.y() + 40, c.width() - 8, 10), Qt::AlignCenter, QString::number(h.temp, 'f', 1) + "°C");
        p.drawText(QRectF(c.x() + 4, c.y() + 51, c.width() - 8, 10), Qt::AlignCenter, QString::number(h.rainChance) + "%");
    }
}
