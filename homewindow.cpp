#include "homewindow.h"
#include "ui_home.h"
#include "chatbotdialog.h"
#include "loreguidewidget.h"
#include <QDialog>
#include <QAudioOutput>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QFrame>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QMediaPlayer>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVideoWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QApplication>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QStackedWidget>
#include <QVariantAnimation>
#include <QTransform>
#include <QEvent>
#include <QtMath>

HomeWindow::HomeWindow(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::HomeFrame),
    m_currentLanguage("en"),
    m_currentVolume(1.0),
    m_isStandardMode(true),
    m_animationTriggered(false),
    m_helpButtonStyleCaptured(false),
    m_animationAudioPlayer(nullptr),
    m_animationAudioOutput(nullptr),
    m_currentGuide(nullptr)
{
    ui->setupUi(this);
    
    // Create chatbot dialog (reusable, hidden by default)
    chatBotDialog = new ChatBotDialog(this);
    
    // Connect buttons to their handlers
    connect(ui->gs_employes,    &QPushButton::clicked, this, &HomeWindow::handleEmployes);
    connect(ui->gs_client,      &QPushButton::clicked, this, &HomeWindow::handleClient);
    connect(ui->gs_order,        &QPushButton::clicked, this, &HomeWindow::handleOrder);
    connect(ui->gs_fournisseur, &QPushButton::clicked, this, &HomeWindow::handleFournisseur);
    connect(ui->gs_equipment,   &QPushButton::clicked, this, &HomeWindow::handleEquipment);
    connect(ui->btn_settings,   &QPushButton::clicked, this, &HomeWindow::handleSettingsClicked);
    connect(ui->btn_disconnect,  &QPushButton::clicked, this, &HomeWindow::handleDisconnect);
    connect(ui->btn_chat,        &QPushButton::clicked, this, &HomeWindow::handleChatBot);
    connect(ui->btn_help,         &QPushButton::clicked, this, &HomeWindow::handleHelp);
    
    // Apply professional styling
    setupHomeButtons();
}

HomeWindow::~HomeWindow()
{
    delete ui;
}

bool HomeWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (ui && watched == ui->btn_settings) {
        if (event->type() == QEvent::Enter) {
            m_settingsHoverActive = true;
            if (m_settingsTiltAnim) {
                m_settingsTiltAnim->start();
            }
        } else if (event->type() == QEvent::Leave) {
            m_settingsHoverActive = false;
            if (m_settingsTiltAnim) {
                m_settingsTiltAnim->stop();
            }
            if (!m_settingsGearPixmap.isNull()) {
                ui->btn_settings->setIcon(QIcon(m_settingsGearPixmap));
            }
        }
    }
    return QFrame::eventFilter(watched, event);
}

void HomeWindow::retranslateUI()
{
    ui->retranslateUi(this);
}

void HomeWindow::handleEmployes()    { emit employesClicked();    }
void HomeWindow::handleClient()      { emit clientClicked();      }
void HomeWindow::handleOrder()        { emit orderClicked();        }
void HomeWindow::handleFournisseur() { emit fournisseurClicked(); }
void HomeWindow::handleEquipment()   { emit equipmentClicked();   }
void HomeWindow::handleDisconnect()  { emit disconnectClicked();   }

void HomeWindow::handleChatBot()
{
    // Standard and animation modes now use the same chatbot dialog behavior.
    QWidget *topLevel = this->window();
    if (topLevel) {
        QPoint bottomRight = topLevel->mapToGlobal(topLevel->rect().bottomRight());
        chatBotDialog->move(bottomRight.x() - chatBotDialog->width() - 20,
                            bottomRight.y() - chatBotDialog->height() - 20);
    }

    if (!m_isStandardMode) {
        chatBotDialog->setWindowOpacity(0.0);
    }

    chatBotDialog->show();
    chatBotDialog->raise();
    chatBotDialog->activateWindow();

    if (!m_isStandardMode) {
        QPropertyAnimation *fadeIn = new QPropertyAnimation(chatBotDialog, "windowOpacity", chatBotDialog);
        fadeIn->setDuration(220);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void HomeWindow::handleSettingsClicked()
{
    auto *blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(8.0);
    this->setGraphicsEffect(blurEffect);

    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setModal(true);
    dialog.setObjectName("languageDialog");
    dialog.setStyleSheet(R"(
        QDialog#languageDialog { background-color: transparent; }
        QFrame#dialogCard {
            background-color: #ffffff;
            border: 2px solid #8B6F47;
            border-radius: 10px;
        }
        QLabel#dialogTitle {
            color: #2f2f2f;
            font-size: 16px;
            font-weight: 600;
        }
        QLabel#volumeLabel {
            color: #4a3b26;
            font-size: 13px;
            font-weight: 600;
        }
        QLabel#volumeValue {
            color: #7a5f3c;
            font-size: 12px;
        }
        QSlider::groove:horizontal {
            height: 6px;
            background: #e6e0d8;
            border-radius: 3px;
        }
        QSlider::sub-page:horizontal {
            background: #8B6F47;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            width: 14px;
            height: 14px;
            margin: -4px 0;
            border-radius: 7px;
            background: #ffffff;
            border: 2px solid #8B6F47;
        }
        QSlider::handle:horizontal:hover {
            background: #f7f2ea;
        }
        QPushButton#primaryBtn {
            background-color: #8B6F47;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 8px 14px;
        }
        QPushButton#primaryBtn:hover { background-color: #7a5f3c; }
        QPushButton#primaryBtn:checked {
            background-color: #d4a96a;
            color: #2a1e10;
            font-weight: bold;
        }
        QPushButton#primaryBtn:!checked {
            background-color: #6b5535;
            color: #c0c0c0;
        }
        QPushButton#closeBtn {
            background-color: #d5d5d5;
            color: #333333;
            border: none;
            border-radius: 6px;
            padding: 8px 14px;
        }
        QPushButton#closeBtn:hover { background-color: #c7c7c7; }
    )");

    QVBoxLayout *outerLayout = new QVBoxLayout(&dialog);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *card = new QFrame(&dialog);
    card->setObjectName("dialogCard");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 16);
    cardLayout->setSpacing(10);

    QLabel *title = new QLabel("Select Language", card);
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);

    QLabel *volumeLabel = new QLabel("Volume", card);
    volumeLabel->setObjectName("volumeLabel");

    QLabel *volumeValue = new QLabel(QString("%1%").arg(100), card);
    volumeValue->setObjectName("volumeValue");
    volumeValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QSlider *volumeSlider = new QSlider(Qt::Horizontal, card);
    volumeSlider->setRange(0, 150);
    volumeSlider->setValue(qRound(m_currentVolume * 100.0));
    
    // Initial sync of volume value label
    volumeValue->setText(QString("%1%").arg(volumeSlider->value()));

    QPushButton *englishButton = new QPushButton("English", card);
    englishButton->setObjectName("primaryBtn");
    englishButton->setCursor(Qt::PointingHandCursor);
    englishButton->setCheckable(true);
    englishButton->setChecked(m_currentLanguage == "en");

    QPushButton *frenchButton = new QPushButton("Francais", card);
    frenchButton->setObjectName("primaryBtn");
    frenchButton->setCursor(Qt::PointingHandCursor);
    frenchButton->setCheckable(true);
    frenchButton->setChecked(m_currentLanguage == "fr");

    // --- New: Standard/Animation buttons ---
    QLabel *modeLabel = new QLabel("Mode", card);
    modeLabel->setObjectName("volumeLabel");
    
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(8);
    QPushButton *standardButton = new QPushButton("Standard", card);
    QPushButton *animationButton = new QPushButton("Animation", card);
    standardButton->setCheckable(true);
    animationButton->setCheckable(true);
    standardButton->setObjectName("primaryBtn");
    animationButton->setObjectName("primaryBtn");
    standardButton->setCursor(Qt::PointingHandCursor);
    animationButton->setCursor(Qt::PointingHandCursor);
    modeLayout->addWidget(standardButton);
    modeLayout->addWidget(animationButton);
    
    // Only one can be checked
    QObject::connect(standardButton, &QPushButton::clicked, [this, standardButton, animationButton, &dialog]() {
        standardButton->setChecked(true);
        animationButton->setChecked(false);
        m_isStandardMode = true;
        // Ensure btn_chat is visible when switching back to Standard mode
        ui->btn_chat->setVisible(true);
        dialog.accept();
        if (m_animationTriggered) {
            playReverseAnimation();
        }
    });
    QObject::connect(animationButton, &QPushButton::clicked, [this, standardButton, animationButton]() {
        animationButton->setChecked(true);
        standardButton->setChecked(false);
        m_isStandardMode = false;
        // Allow animation to be triggered again when switching back to Animation mode
        m_animationTriggered = false;
        ui->btn_help->setEnabled(true);
    });
    // Show current selection based on m_isStandardMode
    if (m_isStandardMode) {
        standardButton->setChecked(true);
        animationButton->setChecked(false);
    } else {
        animationButton->setChecked(true);
        standardButton->setChecked(false);
    }

    QPushButton *closeButton = new QPushButton("Close", card);
    closeButton->setObjectName("closeBtn");
    closeButton->setCursor(Qt::PointingHandCursor);

    cardLayout->addWidget(title);
    cardLayout->addWidget(volumeLabel);
    cardLayout->addWidget(volumeValue);
    cardLayout->addWidget(volumeSlider);
    cardLayout->addWidget(modeLabel);
    cardLayout->addLayout(modeLayout);
    cardLayout->addWidget(englishButton);
    cardLayout->addWidget(frenchButton);
    cardLayout->addWidget(closeButton);

    outerLayout->addWidget(card);

    QObject::connect(englishButton, &QPushButton::clicked, &dialog, [&]() {
        emit languageChanged("en");
        dialog.accept();
    });
    QObject::connect(frenchButton, &QPushButton::clicked, &dialog, [&]() {
        emit languageChanged("fr");
        dialog.accept();
    });
    QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    QAudioOutput *audioOutput = new QAudioOutput(&dialog);
    QMediaPlayer *player = new QMediaPlayer(&dialog);
    player->setAudioOutput(audioOutput);
    player->setSource(QUrl("qrc:/assets/forge.mp3"));
    audioOutput->setVolume(volumeSlider->value() / 100.0);

    QObject::connect(volumeSlider, &QSlider::valueChanged, &dialog, [&](int value) {
        volumeValue->setText(QString("%1%").arg(value));
        audioOutput->setVolume(value / 100.0);
        emit volumeChanged(value / 100.0);
        if (player->playbackState() != QMediaPlayer::PlayingState) {
            player->setPosition(0);
            player->play();
        }
    });

    dialog.setFixedSize(260, 410);

    QPoint center = this->mapToGlobal(this->rect().center());
    dialog.move(center.x() - dialog.width() / 2, center.y() - dialog.height() / 2);

    dialog.exec();

    this->setGraphicsEffect(nullptr);
}

void HomeWindow::playReverseAnimation()
{
    if (!m_animationTriggered) return;

    // Stop botawk audio if it is playing
    if (m_animationAudioPlayer) {
        m_animationAudioPlayer->stop();
    }

    // Close guide if it is open
    if (m_currentGuide) {
        m_currentGuide->hide();
        m_currentGuide->deleteLater();
        m_currentGuide = nullptr;
    }

    // Play ger.mp3 to completion
    QMediaPlayer *player = new QMediaPlayer(this);
    QAudioOutput *audioOut = new QAudioOutput(this);
    audioOut->setVolume(m_currentVolume);
    player->setAudioOutput(audioOut);
    player->setSource(QUrl("qrc:/assets/ger.mp3"));
    player->play();
    connect(player, &QMediaPlayer::mediaStatusChanged, player, [player, audioOut](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            player->deleteLater();
            audioOut->deleteLater();
        }
    });

    QPoint helpPos = ui->btn_help->pos();
    QPoint chatPos = ui->btn_chat->pos();

    // Orb at chat button
    QLabel *orb = new QLabel(this);
    orb->setGeometry(chatPos.x(), chatPos.y(), 50, 50);
    orb->setStyleSheet(R"(
        QLabel {
            background-color: qradialgradient(cx:0.5, cy:0.5, radius:0.5,
                fx:0.5, fy:0.5, stop:0 #ffffff, stop:0.4 #ffd700, stop:1 #ff8c00);
            border-radius: 25px;
        }
    )");

    QGraphicsOpacityEffect *orbEff = new QGraphicsOpacityEffect(orb);
    orb->setGraphicsEffect(orbEff);
    orbEff->setOpacity(1.0);
    orb->show();
    orb->raise();

    // Dark overlay (start black, fade out)
    QWidget *dark = new QWidget(this);
    dark->setGeometry(this->rect());
    dark->setStyleSheet("background: black;");
    QGraphicsOpacityEffect *darkEff = new QGraphicsOpacityEffect(dark);
    dark->setGraphicsEffect(darkEff);
    darkEff->setOpacity(1.0);
    dark->show();
    dark->raise();
    orb->raise();

    auto makeReverseBubble = [this](const QString &text) -> QLabel* {
        QLabel *b = new QLabel(this);
        b->setText(text);
        b->setWordWrap(true);
        b->setAlignment(Qt::AlignCenter);
        b->setStyleSheet(R"(
            QLabel {
                background-color: rgba(15, 8, 2, 220);
                color: #ffd700;
                border: 2px solid #8B6F47;
                border-radius: 16px;
                padding: 20px 32px;
                font-size: 20px;
                font-weight: bold;
                font-family: 'Georgia', serif;
                letter-spacing: 1px;
            }
        )");
        b->setFixedWidth(540);
        b->adjustSize();
        b->move((this->width()  - b->width())  / 2,
                (this->height() - b->height()) / 2);
        b->setVisible(false);
        b->raise();
        return b;
    };

    QLabel *b1 = makeReverseBubble("A strange light has formed...");
    QLabel *b2 = makeReverseBubble("Guiding light has been awoken.");
    b1->raise();
    b2->raise();
    orb->raise();

    auto fadeBubble = [](QLabel *b, int inMs, int holdMs, int outMs) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(b);
        b->setGraphicsEffect(eff);
        eff->setOpacity(0.0);
        b->setVisible(true);

        QPropertyAnimation *fadeIn = new QPropertyAnimation(eff, "opacity", b);
        fadeIn->setDuration(inMs);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

        QTimer::singleShot(inMs + holdMs, b, [b, eff, outMs]() {
            QPropertyAnimation *fadeOut = new QPropertyAnimation(eff, "opacity", b);
            fadeOut->setDuration(outMs);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            QObject::connect(fadeOut, &QPropertyAnimation::finished, b, &QLabel::hide);
            fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });
    };

    // Reverse order: show b2 then b1 while the screen is dark
    QTimer::singleShot(200, this, [b2, fadeBubble]() {
        fadeBubble(b2, 300, 1200, 300);
    });
    QTimer::singleShot(2300, this, [b1, fadeBubble]() {
        fadeBubble(b1, 300, 1200, 300);
    });

    auto *seq = new QSequentialAnimationGroup(this);

    // Keep the screen black while the text shows (0-3000ms)
    seq->addAnimation(new QPauseAnimation(3000));

    // Orb flies back + dark fades out (3000-5500ms)
    auto *par = new QParallelAnimationGroup();
    auto *flyBack = new QPropertyAnimation(orb, "pos");
    flyBack->setDuration(2500);
    flyBack->setStartValue(chatPos);
    flyBack->setEndValue(helpPos);
    flyBack->setEasingCurve(QEasingCurve::InOutCubic);
    par->addAnimation(flyBack);

    auto *darkOut = new QPropertyAnimation(darkEff, "opacity");
    darkOut->setDuration(2500);
    darkOut->setStartValue(1.0);
    darkOut->setEndValue(0.0);
    par->addAnimation(darkOut);
    seq->addAnimation(par);

    // Orb fades out (5500-6500ms)
    auto *orbOut = new QPropertyAnimation(orbEff, "opacity");
    orbOut->setDuration(1000);
    orbOut->setStartValue(1.0);
    orbOut->setEndValue(0.0);
    seq->addAnimation(orbOut);

    // Pause to reach 7s total (6500-7000ms)
    seq->addAnimation(new QPauseAnimation(500));

    QObject::connect(seq, &QSequentialAnimationGroup::finished, this, [this, dark, orb]() {
        dark->deleteLater();
        orb->deleteLater();
        ui->btn_help->setVisible(true);
        ui->btn_help->setEnabled(true);
        ui->btn_help->setText(m_helpButtonStyleCaptured ? m_helpButtonTextOriginal : "?");
        ui->btn_help->setStyleSheet(m_helpButtonStyleCaptured ? m_helpButtonStyleOriginal : "");
        ui->btn_chat->setVisible(true);
        m_animationTriggered = false;
    });

    seq->start(QAbstractAnimation::DeleteWhenStopped);
}

void HomeWindow::handleHelp()
{
    if (!m_isStandardMode) {
        // === Animation Mode Easter Egg ===
        
        // Prevent re-triggering animation unless mode changed back to standard first
        if (m_animationTriggered) {
            return;
        }
        m_animationTriggered = true;

        // Play botawk audio — use persistent members so volume updates apply
        if (m_animationAudioPlayer) { m_animationAudioPlayer->stop(); m_animationAudioPlayer->deleteLater(); }
        if (m_animationAudioOutput) { m_animationAudioOutput->deleteLater(); }
        m_animationAudioPlayer = new QMediaPlayer(this);
        m_animationAudioOutput = new QAudioOutput(this);
        m_animationAudioOutput->setVolume(m_currentVolume);
        m_animationAudioPlayer->setAudioOutput(m_animationAudioOutput);
        m_animationAudioPlayer->setSource(QUrl("qrc:/assets/botawk.mp3"));
        m_animationAudioPlayer->play();

        // Disable to prevent re-triggering
        ui->btn_help->setEnabled(false);

        QPoint helpPos = ui->btn_help->pos(); // e.g. (1251, 20)
        QPoint chatPos = ui->btn_chat->pos(); // e.g. (1171, 20)

        // Glowing orb replaces the "?" button visually
        QLabel *orb = new QLabel(this);
        orb->setGeometry(helpPos.x(), helpPos.y(), 50, 50);
        orb->setStyleSheet(R"(
            QLabel {
                background-color: qradialgradient(cx:0.5, cy:0.5, radius:0.5,
                    fx:0.5, fy:0.5, stop:0 #ffffff, stop:0.4 #ffd700, stop:1 #ff8c00);
                border-radius: 25px;
            }
        )");

        // Cache original help button style once
        if (!m_helpButtonStyleCaptured) {
            m_helpButtonTextOriginal = ui->btn_help->text();
            m_helpButtonStyleOriginal = ui->btn_help->styleSheet();
            m_helpButtonStyleCaptured = true;
        }

        // Hide the help button text so orb appears in its place
        ui->btn_help->setText("");
        ui->btn_help->setStyleSheet("QPushButton { background: transparent; border: none; border-radius: 25px; }");

        QGraphicsOpacityEffect *orbEff = new QGraphicsOpacityEffect(orb);
        orb->setGraphicsEffect(orbEff);
        orbEff->setOpacity(0.0);
        orb->show();
        orb->raise();

        // Dark overlay covering the whole home frame
        QWidget *dark = new QWidget(this);
        dark->setGeometry(this->rect());
        dark->setStyleSheet("background: black;");
        QGraphicsOpacityEffect *darkEff = new QGraphicsOpacityEffect(dark);
        dark->setGraphicsEffect(darkEff);
        darkEff->setOpacity(0.0);
        dark->show();
        dark->raise();
        orb->raise(); // Keep orb above dark overlay

        // Build sequential animation: fade in orb → fly to chat → parallel(orb out + screen dark)
        auto *seq = new QSequentialAnimationGroup(this);

        // 1) Orb fades in (400ms)
        auto *fadeIn = new QPropertyAnimation(orbEff, "opacity");
        fadeIn->setDuration(400);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        seq->addAnimation(fadeIn);

        // 2) Orb flies to chat button (900ms)
        auto *flyTo = new QPropertyAnimation(orb, "pos");
        flyTo->setDuration(900);
        flyTo->setStartValue(helpPos);
        flyTo->setEndValue(chatPos);
        flyTo->setEasingCurve(QEasingCurve::InOutCubic);
        seq->addAnimation(flyTo);

        // 3) Parallel: orb fades out + dark fades in COMPLETELY (700ms)
        auto *par = new QParallelAnimationGroup();
        auto *orbOut = new QPropertyAnimation(orbEff, "opacity");
        orbOut->setDuration(700);
        orbOut->setStartValue(1.0);
        orbOut->setEndValue(0.0);
        par->addAnimation(orbOut);
        auto *darkIn = new QPropertyAnimation(darkEff, "opacity");
        darkIn->setDuration(700);
        darkIn->setStartValue(0.0);
        darkIn->setEndValue(1.0); // fully black
        par->addAnimation(darkIn);
        seq->addAnimation(par);

        // ── After animation: show text bubbles ────────────────────────
        QObject::connect(seq, &QSequentialAnimationGroup::finished, this,
                         [this, dark, orb]() {
            orb->hide();

            // Bubbles must be children of 'this' (not dark) so their own
            // QGraphicsOpacityEffect is not swallowed by dark's compositor.
            auto makeBubble = [this](const QString &text) -> QLabel* {
                QLabel *b = new QLabel(this);
                b->setText(text);
                b->setWordWrap(true);
                b->setAlignment(Qt::AlignCenter);
                b->setStyleSheet(R"(
                    QLabel {
                        background-color: rgba(15, 8, 2, 220);
                        color: #ffd700;
                        border: 2px solid #8B6F47;
                        border-radius: 16px;
                        padding: 20px 32px;
                        font-size: 20px;
                        font-weight: bold;
                        font-family: 'Georgia', serif;
                        letter-spacing: 1px;
                    }
                )");
                b->setFixedWidth(540);
                b->adjustSize();
                b->move((this->width()  - b->width())  / 2,
                        (this->height() - b->height()) / 2);
                b->setVisible(false);
                b->raise(); // always on top
                return b;
            };

            QLabel *b1 = makeBubble("A strange light has formed...");
            QLabel *b2 = makeBubble("Guiding light has been awoken.");

            // Bubble 1: appears after 3s, stays 4s, then bubble 2 appears after another 3s, stays 4s
            QGraphicsOpacityEffect *e1 = new QGraphicsOpacityEffect(b1);
            b1->setGraphicsEffect(e1);
            e1->setOpacity(0.0);
            b1->setVisible(false);

            QTimer::singleShot(3000, this, [this, b1, b2, e1]() {
                b1->setVisible(true);
                b1->raise();
                auto *a1in = new QPropertyAnimation(e1, "opacity", b1);
                a1in->setDuration(600);
                a1in->setStartValue(0.0);
                a1in->setEndValue(1.0);
                a1in->start(QAbstractAnimation::DeleteWhenStopped);

                QTimer::singleShot(4000, this, [this, b1, b2, e1]() {
                    auto *a1out = new QPropertyAnimation(e1, "opacity", b1);
                    a1out->setDuration(600);
                    a1out->setStartValue(1.0);
                    a1out->setEndValue(0.0);
                    a1out->start(QAbstractAnimation::DeleteWhenStopped);

                    QTimer::singleShot(700, this, [this, b1, b2]() {
                        b1->hide();
                        QGraphicsOpacityEffect *e2 = new QGraphicsOpacityEffect(b2);
                        b2->setGraphicsEffect(e2);
                        e2->setOpacity(0.0);
                        b2->setVisible(false);

                        QTimer::singleShot(3000, this, [this, b2, e2]() {
                            b2->setVisible(true);
                            b2->raise();
                            auto *a2in = new QPropertyAnimation(e2, "opacity", b2);
                            a2in->setDuration(600);
                            a2in->setStartValue(0.0);
                            a2in->setEndValue(1.0);
                            a2in->start(QAbstractAnimation::DeleteWhenStopped);

                            QTimer::singleShot(4000, this, [b2, e2]() {
                                auto *a2out = new QPropertyAnimation(e2, "opacity", b2);
                                a2out->setDuration(600);
                                a2out->setStartValue(1.0);
                                a2out->setEndValue(0.0);
                                a2out->start(QAbstractAnimation::DeleteWhenStopped);
                                QObject::connect(a2out, &QPropertyAnimation::finished,
                                                 b2, &QLabel::hide);
                            });
                        });
                    });
                });
            });
        });

        // ── At second 25: fade screen back + reveal LoreGuideWidget ──
        QTimer::singleShot(25000, this, [this, dark, darkEff]() {
            // Fade dark overlay out
            auto *darkOut = new QPropertyAnimation(darkEff, "opacity", dark);
            darkOut->setDuration(1200);
            darkOut->setStartValue(darkEff->opacity());
            darkOut->setEndValue(0.0);
            darkOut->start(QAbstractAnimation::DeleteWhenStopped);
            QObject::connect(darkOut, &QPropertyAnimation::finished, dark, &QWidget::deleteLater);

            // Hide btn_chat and btn_help — guide replaces them
            ui->btn_chat->setVisible(false);
            ui->btn_help->hide();  // permanently removed after animation

            m_currentGuide = new LoreGuideWidget(m_animationAudioPlayer, this);
            LoreGuideWidget *guide = m_currentGuide;
            guide->setAnimationUnlocked(true);
            // Position: anchored to top-right
            QPoint anchor = this->mapToGlobal(QPoint(this->width() - guide->width() - 10, 10));
            guide->move(anchor);

            QGraphicsOpacityEffect *gEff = new QGraphicsOpacityEffect(guide);
            guide->setGraphicsEffect(gEff);
            gEff->setOpacity(0.0);
            guide->show();
            guide->raise();

            auto *gFadeIn = new QPropertyAnimation(gEff, "opacity", guide);
            gFadeIn->setDuration(1000);
            gFadeIn->setStartValue(0.0);
            gFadeIn->setEndValue(1.0);
            gFadeIn->start(QAbstractAnimation::DeleteWhenStopped);

            // When guide is closed: song keeps playing (stops only when leaving home page)
            connect(guide, &LoreGuideWidget::closeRequested, this,
                    [this, guide, gEff]() {
                auto *gOut = new QPropertyAnimation(gEff, "opacity", guide);
                gOut->setDuration(600);
                gOut->setStartValue(1.0);
                gOut->setEndValue(0.0);
                gOut->start(QAbstractAnimation::DeleteWhenStopped);
                QObject::connect(gOut, &QPropertyAnimation::finished, this,
                                 [this, guide]() {
                    m_currentGuide = nullptr;
                    guide->deleteLater();
                    // m_animationAudioPlayer keeps playing — stops when user leaves home page
                    ui->btn_chat->setVisible(true);
                    // btn_help stays hidden
                });
            });
        });

        seq->start(QAbstractAnimation::DeleteWhenStopped);
        return;
    }

    QWidget *topLevel = this->window();
    int w = topLevel->width();
    int h = topLevel->height();
    QPoint origin = topLevel->mapToGlobal(QPoint(0, 0));

    // Frameless dialog that covers the main window
    QDialog *overlay = new QDialog(topLevel);
    overlay->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    overlay->setModal(true);
    overlay->setAttribute(Qt::WA_DeleteOnClose);
    overlay->setGeometry(origin.x(), origin.y(), w, h);
    overlay->setStyleSheet("QDialog { background-color: black; }");

    // QVideoWidget is the most reliable renderer on Windows (uses WMF/D3D correctly)
    QVideoWidget *videoWidget = new QVideoWidget(overlay);
    videoWidget->setGeometry(0, 0, w, h);

    QMediaPlayer *player = new QMediaPlayer(overlay);
    QAudioOutput *audioOut = new QAudioOutput(overlay);
    audioOut->setVolume(m_currentVolume);
    player->setAudioOutput(audioOut);
    player->setVideoOutput(videoWidget);
    // Keep video volume in sync with the settings slider while the video is playing
    connect(this, &HomeWindow::volumeChanged, audioOut, &QAudioOutput::setVolume);

    // Find local path for unkown.mp4 — try multiple relative locations
    QString appDir = QCoreApplication::applicationDirPath();
    QString videoPath = appDir + "/../../../assets/unkown.mp4";  // debug build: build/Qt.../debug/ -> root
    if (!QFile::exists(videoPath)) videoPath = appDir + "/../../assets/unkown.mp4";
    if (!QFile::exists(videoPath)) videoPath = appDir + "/assets/unkown.mp4";
    if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/assets/unkown.mp4";
    if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/../assets/unkown.mp4";
    if (!QFile::exists(videoPath)) videoPath = QDir::currentPath() + "/../../assets/unkown.mp4";
    player->setSource(QUrl::fromLocalFile(QFileInfo(videoPath).absoluteFilePath()));

    // Bubble — owned top-level Tool window so it floats above the native QVideoWidget surface.
    // Parented to overlay so it is logically tied to it (hides/shows with owner, no taskbar entry).
    QLabel *bubble = new QLabel(overlay);
    bubble->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    bubble->setAttribute(Qt::WA_ShowWithoutActivating);
    bubble->setAttribute(Qt::WA_DeleteOnClose);
    bubble->setText("You clicked yet nothing happened...\nMaybe the secret lays in the settings.");
    bubble->setWordWrap(true);
    bubble->setAlignment(Qt::AlignCenter);
    bubble->setStyleSheet(R"(
        QLabel {
            background-color: rgba(20, 10, 2, 200);
            color: #f5e6c8;
            border: 2px solid rgba(139, 111, 71, 230);
            border-radius: 18px;
            padding: 22px 32px;
            font-size: 18px;
            font-weight: 600;
            font-family: 'Georgia', serif;
        }
    )");
    bubble->setFixedWidth(580);
    bubble->adjustSize();
    bubble->setVisible(false);

    // After 3 seconds fade the bubble in — video keeps playing underneath
    QTimer::singleShot(3000, overlay, [bubble, overlay, w, h]() {
        // Use global screen coords because bubble is a top-level window
        QPoint globalCenter = overlay->mapToGlobal(QPoint(w / 2, h / 2));
        bubble->move(globalCenter.x() - bubble->width() / 2,
                     globalCenter.y() - bubble->height() / 2 - 20);

        QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(bubble);
        bubble->setGraphicsEffect(opacity);
        opacity->setOpacity(0.0);
        bubble->setVisible(true);
        bubble->raise();

        QPropertyAnimation *fadeIn = new QPropertyAnimation(opacity, "opacity", bubble);
        fadeIn->setDuration(800);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Close bubble when overlay closes
    QObject::connect(overlay, &QDialog::finished, bubble, &QWidget::close);

    // Close when video finishes
    QObject::connect(player, &QMediaPlayer::mediaStatusChanged, overlay,
                     [overlay](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia)
            overlay->close();
    });

    // Re-raise overlay when user alt-tabs back into the app
    QObject::connect(qApp, &QApplication::applicationStateChanged, overlay,
        [overlay](Qt::ApplicationState state) {
            if (state == Qt::ApplicationActive && overlay->isVisible()) {
                overlay->raise();
                overlay->activateWindow();
            }
        });

    overlay->show();
    player->play();
}

void HomeWindow::stopHomeAudio()
{
    if (m_animationAudioPlayer)
        m_animationAudioPlayer->stop();
}

void HomeWindow::stopAnimationAudio()
{
    stopHomeAudio();
}

void HomeWindow::setupHomeButtons()
{
    m_settingsGearPixmap = QPixmap(":/assets/gear.png");
    if (!m_settingsGearPixmap.isNull()) {
        ui->btn_settings->setText("");
        ui->btn_settings->setIcon(QIcon(m_settingsGearPixmap));
        ui->btn_settings->setIconSize(QSize(30, 30));
        ui->btn_settings->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                border: none;
                border-radius: 25px;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.2);
            }
            QPushButton:pressed {
                background-color: rgba(255, 255, 255, 0.3);
            }
        )");

        ui->btn_settings->installEventFilter(this);

        if (!m_settingsTiltAnim) {
            m_settingsTiltAnim = new QVariantAnimation(this);
            m_settingsTiltAnim->setStartValue(0.0);
            m_settingsTiltAnim->setEndValue(1.0);
            m_settingsTiltAnim->setDuration(700);
            m_settingsTiltAnim->setLoopCount(-1);

            connect(m_settingsTiltAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
                if (!ui || !ui->btn_settings || m_settingsGearPixmap.isNull() || !m_settingsHoverActive)
                    return;

                const qreal t = value.toReal();
                const qreal angle = qSin(t * (2.0 * M_PI)) * 8.0;

                QTransform transform;
                transform.rotate(angle);
                QPixmap rotated = m_settingsGearPixmap.transformed(transform, Qt::SmoothTransformation);
                ui->btn_settings->setIcon(QIcon(rotated));
            });
        }
    }

    // Employee button - Completely invisible
    QString employeeStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 20px; 
        }
        QPushButton:hover {
            background-color: transparent;
        }
        QPushButton:pressed {
            background-color: transparent;
        }
    )";
    ui->gs_employes->setStyleSheet(employeeStyle);
    ui->gs_employes->setFixedSize(380, 125); 
    ui->gs_employes->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Client button - Completely invisible
    QString clientStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 20px; 
        }
        QPushButton:hover {
            background-color: transparent;
        }
        QPushButton:pressed {
            background-color: transparent;
        }
    )";
    ui->gs_client->setStyleSheet(clientStyle);
    ui->gs_client->setFixedSize(380, 120); 
    ui->gs_client->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Order button - Completely invisible
    QString orderStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 20px; 
        }
        QPushButton:hover {
            background-color: transparent;
        }
        QPushButton:pressed {
            background-color: transparent;
        }
    )";
    ui->gs_order->setStyleSheet(orderStyle);
    ui->gs_order->setFixedSize(380, 105); 
    ui->gs_order->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Equipment button - Completely invisible
    QString equipmentStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 20px; 
        }
        QPushButton:hover {
            background-color: transparent;
        }
        QPushButton:pressed {
            background-color: transparent;
        }
    )";
    ui->gs_equipment->setStyleSheet(equipmentStyle);
    ui->gs_equipment->setFixedSize(380, 105); 
    ui->gs_equipment->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Supplier button - Completely invisible (Wider button)
    QString supplierStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 20px; 
        }
        QPushButton:hover {
            background-color: transparent;
        }
        QPushButton:pressed {
            background-color: transparent;
        }
    )";
    ui->gs_fournisseur->setStyleSheet(supplierStyle);
    ui->gs_fournisseur->setFixedSize(480, 90);
    ui->gs_fournisseur->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
