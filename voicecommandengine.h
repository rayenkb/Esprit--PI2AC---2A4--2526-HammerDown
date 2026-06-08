#pragma once
#include <QObject>
#include <QTimer>

// Forward-declare Vosk structs so this header doesn't need vosk_api.h
struct VoskModel;
struct VoskRecognizer;
class  QAudioSource;
class  QIODevice;

class VoiceCommandEngine : public QObject
{
    Q_OBJECT
public:
    explicit VoiceCommandEngine(const QString &modelPath, QObject *parent = nullptr);
    ~VoiceCommandEngine();

    // Call once after construction. Returns false if model folder is missing.
    bool init();

    bool isReady()     const { return m_ready; }
    bool isListening() const { return m_listening; }

public slots:
    void startListening();
    void stopListening();
    void toggleListening();

signals:
    void commandDetected(const QString &text);   // lower-case recognised phrase
    void listeningChanged(bool active);
    void initFailed(const QString &reason);

private slots:
    void processAudio();

private:
    QString         m_modelPath;
    VoskModel      *m_model     = nullptr;
    VoskRecognizer *m_rec       = nullptr;
    QAudioSource   *m_source    = nullptr;
    QIODevice      *m_device    = nullptr;
    QTimer         *m_timer     = nullptr;
    bool            m_ready     = false;
    bool            m_listening = false;
};
