#ifndef WELCOMENOTIFICATIONBAR_H
#define WELCOMENOTIFICATIONBAR_H

#include <QWidget>
#include <QLabel>
#include <QMovie>
#include <QPropertyAnimation>
#include <QTimer>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

class WelcomeNotificationBar : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeNotificationBar(const QString &employeeName, const QString &managementName, QWidget *parent = nullptr);
    ~WelcomeNotificationBar();

    void startEntrance();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void startExit();
    void onEntranceFinished();
    void startShimmer();
    void onShimmerUpdate();

private:
    QString m_employeeName;
    QString m_managementName;
    QLabel *m_gifLabel;
    QMovie *m_movie;
    qreal m_shimmerPos = -1.5; // Starts off-screen left
    QTimer *m_shimmerTimer;
    bool m_isExiting = false;
    
    void setupUi();
};

#endif // WELCOMENOTIFICATIONBAR_H
