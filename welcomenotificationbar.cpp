#include "welcomenotificationbar.h"
#include <QEasingCurve>
#include <QFrame>

WelcomeNotificationBar::WelcomeNotificationBar(const QString &employeeName, const QString &managementName, QWidget *parent)
    : QWidget(parent), m_employeeName(employeeName), m_managementName(managementName)
{
    // Fix bar dimensions - Larger for PREMIUM impact and visibility
    setFixedHeight(80);
    if (parent) {
        setFixedWidth(parent->width());
    }

    setupUi();

    // Setup Shadow - Deep elevation
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 180)); 
    setGraphicsEffect(shadow);

    // Initial position: above screen
    move(0, -height());
    
    // Shimmer Timer
    m_shimmerTimer = new QTimer(this);
    connect(m_shimmerTimer, &QTimer::timeout, this, &WelcomeNotificationBar::onShimmerUpdate);
}

WelcomeNotificationBar::~WelcomeNotificationBar() {
    if (m_movie) {
        m_movie->stop();
    }
}

void WelcomeNotificationBar::setupUi() {
    QHBoxLayout *mainLay = new QHBoxLayout(this);
    mainLay->setContentsMargins(20, 0, 20, 0); // More room
    mainLay->setSpacing(0);

    // LEFT: Welcome GIF - Large and clear
    m_gifLabel = new QLabel(this);
    m_gifLabel->setFixedSize(64, 64);
    m_movie = new QMovie(":/assets/welcome.gif");
    m_gifLabel->setMovie(m_movie);
    m_movie->setScaledSize(QSize(64, 64)); 
    m_movie->start();
    mainLay->addWidget(m_gifLabel);

    // DIVIDER
    mainLay->addSpacing(20);
    QFrame *divider = new QFrame(this);
    divider->setFixedSize(2, 48); // Bolder divider
    divider->setStyleSheet("background-color: rgba(193, 127, 62, 100);"); 
    mainLay->addWidget(divider);
    mainLay->addSpacing(25);

    // TEXTS
    QLabel *welcomeMsg = new QLabel(QString("Welcome to %1").arg(m_managementName), this);
    welcomeMsg->setStyleSheet("color: #F5E6D3; font-size: 12px; font-weight: bold; background: transparent;");
    mainLay->addWidget(welcomeMsg);

    QLabel *dot = new QLabel(" · ", this);
    dot->setStyleSheet("color: #C17F3E; font-size: 12px; background: transparent;");
    mainLay->addWidget(dot);

    QLabel *personalMsg = new QLabel(QString("Good to see you, %1").arg(m_employeeName), this);
    personalMsg->setStyleSheet("color: #C17F3E; font-size: 14px; font-weight: normal; background: transparent;");
    mainLay->addWidget(personalMsg);

    mainLay->addStretch();

    // CLOSE BUTTON
    QPushButton *closeBtn = new QPushButton("×", this);
    closeBtn->setFixedSize(18, 18);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton { color: rgba(245, 230, 211, 89); font-size: 18px; border: none; background: transparent; font-weight: bold; }"
        "QPushButton:hover { color: #F5E6D3; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &WelcomeNotificationBar::startExit);
    mainLay->addWidget(closeBtn);
}

void WelcomeNotificationBar::startEntrance() {
    raise();
    show();
    
    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(400);
    anim->setStartValue(QRect(0, -height(), width(), height()));
    anim->setEndValue(QRect(0, 0, width(), height()));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(anim, &QPropertyAnimation::finished, this, &WelcomeNotificationBar::onEntranceFinished);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WelcomeNotificationBar::onEntranceFinished() {
    // 3 seconds timer to stay visible
    QTimer::singleShot(3000, this, [this](){
        if (!m_isExiting) startExit();
    });

    // 100ms delay then shimmer
    QTimer::singleShot(100, this, &WelcomeNotificationBar::startShimmer);
}

void WelcomeNotificationBar::startShimmer() {
    m_shimmerPos = -0.5; // Start sweep
    m_shimmerTimer->start(16); // ~60fps
}

void WelcomeNotificationBar::onShimmerUpdate() {
    m_shimmerPos += 0.04; // Sweep speed
    if (m_shimmerPos > 2.0) {
        m_shimmerTimer->stop();
        m_shimmerPos = -1.5;
    }
    update();
}

void WelcomeNotificationBar::startExit() {
    if (m_isExiting) return;
    m_isExiting = true;

    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(300);
    anim->setStartValue(QRect(0, 0, width(), height()));
    anim->setEndValue(QRect(0, -height(), width(), height()));
    anim->setEasingCurve(QEasingCurve::InCubic);
    
    connect(anim, &QPropertyAnimation::finished, this, &QWidget::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WelcomeNotificationBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // BACKGROUND
    QLinearGradient bgGrad(0, 0, width(), 0);
    bgGrad.setColorAt(0.0, QColor("#2A1F0E"));
    bgGrad.setColorAt(0.3, QColor("#1A1208"));
    bgGrad.setColorAt(1.0, QColor("#1A1208"));
    p.fillRect(rect(), bgGrad);

    // BOTTOM BORDER
    p.setPen(QPen(QColor("#C17F3E"), 1.5));
    p.drawLine(0, height() - 1, width(), height() - 1);

    // SHIMMER SWEEP
    if (m_shimmerPos > -1.0 && m_shimmerPos < 2.0) {
        qreal x = m_shimmerPos * width();
        qreal shimmerWidth = width() * 0.3;
        
        QLinearGradient shimmerGrad(x - shimmerWidth, 0, x, 0);
        shimmerGrad.setColorAt(0, Qt::transparent);
        shimmerGrad.setColorAt(0.5, QColor(255, 255, 255, 30)); // 0.12 opacity approx
        shimmerGrad.setColorAt(1, Qt::transparent);
        
        p.fillRect(rect(), shimmerGrad);
    }
}

void WelcomeNotificationBar::resizeEvent(QResizeEvent *event) {
    if (parentWidget()) {
        setFixedWidth(parentWidget()->width());
    }
    QWidget::resizeEvent(event);
}
