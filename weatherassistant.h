#ifndef WEATHERASSISTANT_H
#define WEATHERASSISTANT_H

#include <QWidget>
#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSoundEffect>
#include <QTimer>
#include <QVector>

class QParallelAnimationGroup;
class QPainter;
class QPaintEvent;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;
class QScrollBar;

class WeatherAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherAssistant(QWidget *parent = nullptr);
    ~WeatherAssistant();

    void showAnimated();
    void hideAnimated();
    void refreshWeather();

private slots:
    void onWeatherReceived(QNetworkReply *reply);
    void onAdviceReceived(QNetworkReply *reply);
    void onRenderTick();
    void onClockTick();

private:
    enum WeatherKind {
        Clear,
        Cloudy,
        Rainy,
        Snowy,
        Thunderstorm
    };

    struct HourForecast {
        QString timeLabel;
        double temp = 0.0;
        int rainChance = 0;
        WeatherKind kind = Clear;
    };

    QNetworkAccessManager *networkManager = nullptr;
    QNetworkAccessManager *adviceNetworkManager = nullptr;
    QSoundEffect *hammerSound = nullptr;
    QSoundEffect *ambientSound = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;
    QPushButton *m_closeButton = nullptr;
    QTimer *m_renderTimer = nullptr;
    QTimer *m_clockTimer = nullptr;
    QTimer *m_refreshTimer = nullptr;

    QString m_location = "Tunis, TN";
    QString m_conditionText = "CLEAR";
    QString m_conditionDescription = "Clear";
    WeatherKind m_kind = Clear;
    QDateTime m_now;

    double m_tempC = 0.0;
    double m_feelsLikeC = 0.0;
    int m_humidity = 0;
    double m_pressureHpa = 0.0;
    double m_visibilityKm = 0.0;
    double m_windKmh = 0.0;
    int m_windDeg = 0;
    int m_uvIndex = 0;
    QDateTime m_sunrise;
    QDateTime m_sunset;
    int m_workshopScore = 0;
    double m_lumberMoisture = 0.0;

    float m_globalTime = 0.0f;
    float m_openProgress = 0.0f;
    float m_numberProgress = 0.0f;
    float m_typewriterProgress = 0.0f;
    bool m_lightningFlash = false;
    bool m_isAnimatingOut = false;

    QString m_adviceFull;
    QString m_adviceVisible;
    QVector<HourForecast> m_hourly;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void updateAdvice(double temp, int humidity, const QString &condition);
    void requestGroqAdvice(double temp, int humidity, const QString &condition);
    void rebuildDerivedData();
    void rebuildHourlyForecast();
    WeatherKind classifyKind(const QString &condition) const;

    void drawSkyBackground(QPainter &p);
    void drawGlassOverlay(QPainter &p);
    void drawTopSection(QPainter &p, const QRectF &rect);
    void drawMainCards(QPainter &p, const QRectF &rect);
    void drawExtendedBoxes(QPainter &p, const QRectF &rect);
    void drawPerformanceBar(QPainter &p, const QRectF &rect);
    void drawMoistureGauge(QPainter &p, const QRectF &rect);
    void drawIconsRow(QPainter &p, const QRectF &rect);
    void drawAdviceBox(QPainter &p, const QRectF &rect);
    void drawHourlyRow(QPainter &p, const QRectF &rect);

    void drawConditionIcon(QPainter &p, const QRectF &rect, WeatherKind kind, float t);
    QColor temperatureColor(double temp) const;
    QColor uvColor(int uv) const;

    bool m_dragging = false;
    QPoint m_dragOffset;
    QScrollBar *m_verticalScroll = nullptr;
};

#endif // WEATHERASSISTANT_H
