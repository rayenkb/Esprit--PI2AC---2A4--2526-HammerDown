#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QFrame>
#include <QMenu>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>

class QVariantAnimation;

class ChatBotDialog;

namespace Ui {
class HomeFrame;
}

class HomeWindow : public QFrame
{
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();
    void retranslateUI();
    void stopHomeAudio();
    void suspendActiveAudioForOverlay();
    void resumeSuspendedAudioAfterOverlay();
    bool isAnimationMode() const { return !m_isStandardMode; }

    // Setters for state synchronization
    void setLanguage(const QString &lang) { m_currentLanguage = lang; }
    void setVolume(qreal vol) {
        m_currentVolume = vol;
        if (m_animationAudioOutput) m_animationAudioOutput->setVolume(vol);
    }
    void setMode(bool isStandard) { m_isStandardMode = isStandard; }

signals:
    void employesClicked();
    void clientClicked();
    void orderClicked();
    void fournisseurClicked();
    void equipmentClicked();
    void languageChanged(const QString &language);
    void volumeChanged(qreal volume);
    void disconnectClicked();
    void settingsDialogOpened();
    void settingsDialogClosed();
    void botawkAnimationStarted();
    void gerPlaybackFinished();
    void tutorialOpened();
    void tutorialClosed();

private slots:
    void handleEmployes();
    void handleClient();
    void handleOrder();
    void handleFournisseur();
    void handleEquipment();
    void handleSettingsClicked();
    void handleDisconnect();
    void handleChatBot();
    void handleHelp();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::HomeFrame *ui;
    ChatBotDialog *chatBotDialog;
    
    QString m_currentLanguage;
    qreal m_currentVolume;
    bool m_isStandardMode;
    bool m_animationTriggered; // Prevent re-triggering animation until mode changes
    bool m_helpButtonStyleCaptured;
    QString m_helpButtonTextOriginal;
    QString m_helpButtonStyleOriginal;
    
    QMediaPlayer *m_animationAudioPlayer; // Persistent audio for animation sequence
    QAudioOutput *m_animationAudioOutput; // Matching output for animation audio
    class LoreGuideWidget *m_currentGuide; // Track the guide widget
    QVariantAnimation *m_settingsTiltAnim = nullptr;
    QPixmap m_settingsGearPixmap;
    bool m_settingsHoverActive = false;
    bool m_animationAudioSuspended = false;
    qint64 m_animationAudioResumePosition = 0;
    
    void setupHomeButtons();
    void stopAnimationAudio();
    void playReverseAnimation();
};

#endif // HOMEWINDOW_H
