#include "voicecommandengine.h"

#include <QLibrary>
#include <QAudioSource>
#include <QAudioFormat>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

// ---------------------------------------------------------------------------
// Vosk C-API types and function pointers (resolved at runtime via QLibrary)
// This avoids any compile-time or link-time dependency on libvosk.
// ---------------------------------------------------------------------------
struct VoskModel      {};
struct VoskRecognizer {};
using fn_vosk_recognizer_set_grammar = void (*)(VoskRecognizer*, const char*);
static fn_vosk_recognizer_set_grammar p_recognizer_set_grammar = nullptr;
using fn_vosk_set_log_level              = void        (*)(int);
using fn_vosk_model_new                  = VoskModel*  (*)(const char*);
using fn_vosk_model_free                 = void        (*)(VoskModel*);
using fn_vosk_recognizer_new             = VoskRecognizer* (*)(VoskModel*, float);
using fn_vosk_recognizer_free            = void        (*)(VoskRecognizer*);
using fn_vosk_recognizer_accept_waveform = int         (*)(VoskRecognizer*, const char*, int);
using fn_vosk_recognizer_result          = const char* (*)(VoskRecognizer*);

static fn_vosk_set_log_level              p_set_log_level              = nullptr;
static fn_vosk_model_new                  p_model_new                  = nullptr;
static fn_vosk_model_free                 p_model_free                 = nullptr;
static fn_vosk_recognizer_new             p_recognizer_new             = nullptr;
static fn_vosk_recognizer_free            p_recognizer_free            = nullptr;
static fn_vosk_recognizer_accept_waveform p_recognizer_accept_waveform = nullptr;
static fn_vosk_recognizer_result          p_recognizer_result          = nullptr;

// Walk up from startDir looking for relPath (e.g. "vosk/libvosk.dll").
// Returns the absolute path if found, empty string otherwise.
static QString findUpward(const QString &startDir, const QString &relPath)
{
    QDir dir(startDir);
    for (int i = 0; i < 8; ++i) {
        const QString candidate = dir.absoluteFilePath(relPath);
        if (QFileInfo::exists(candidate))
            return candidate;
        if (!dir.cdUp())
            break;
    }
    return {};
}

static QString s_loadError; // populated on failure, read by init()

static bool loadVoskLibrary()
{
    static bool loaded = false;
    static bool tried  = false;
    if (tried) return loaded;
    tried = true;

    const QString appDir = QCoreApplication::applicationDirPath();

    // Walk up from the exe, checking vosk/libvosk.dll and libvosk.dll at each level
    QString dllPath = findUpward(appDir, "vosk/libvosk.dll");
    if (dllPath.isEmpty())
        dllPath = findUpward(appDir, "libvosk.dll");

    QLibrary lib;
    bool found = false;

    if (!dllPath.isEmpty()) {
        const QString voskDir = QFileInfo(dllPath).absolutePath();
#ifdef Q_OS_WIN
        // Add vosk folder to DLL search so its bundled GCC runtime is found
        // before any older GCC runtime on PATH.
        SetDllDirectoryA(voskDir.toLocal8Bit().constData());
#endif
        lib.setFileName(dllPath);
        found = lib.load();
#ifdef Q_OS_WIN
        SetDllDirectoryA(nullptr);
#endif
        if (!found)
            s_loadError = QString("Path: %1\nError: %2").arg(dllPath, lib.errorString());
    }

    // Fallback: let the OS find it on PATH
    if (!found) {
        lib.setFileName("libvosk");
        found = lib.load();
        if (!found)
            s_loadError += QString("\nFallback error: %1").arg(lib.errorString());
    }

    if (!found) return false;

#define RESOLVE(fn) p_##fn = (fn_vosk_##fn) lib.resolve("vosk_" #fn); \
                    if (!p_##fn) { s_loadError = "Missing export: vosk_" #fn; return false; }
#define RESOLVE_OPT(fn) p_##fn = (fn_vosk_##fn) lib.resolve("vosk_" #fn); // optional, null is ok

    RESOLVE_OPT(recognizer_set_grammar) // not present in all builds
    RESOLVE(set_log_level)
    RESOLVE(model_new)
    RESOLVE(model_free)
    RESOLVE(recognizer_new)
    RESOLVE(recognizer_free)
    RESOLVE(recognizer_accept_waveform)
    RESOLVE(recognizer_result)
#undef RESOLVE
#undef RESOLVE_OPT

    loaded = true;
    return true;
}

// ---------------------------------------------------------------------------

VoiceCommandEngine::VoiceCommandEngine(const QString &modelPath, QObject *parent)
    : QObject(parent), m_modelPath(modelPath)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(150);
    connect(m_timer, &QTimer::timeout, this, &VoiceCommandEngine::processAudio);
}

VoiceCommandEngine::~VoiceCommandEngine()
{
    stopListening();
    if (m_rec   && p_recognizer_free) p_recognizer_free(m_rec);
    if (m_model && p_model_free)      p_model_free(m_model);
}

bool VoiceCommandEngine::init()
{
    if (!loadVoskLibrary()) {
        emit initFailed("libvosk.dll could not be loaded.\n\n" + s_loadError);
        return false;
    }

    p_set_log_level(-1);

    m_model = p_model_new(m_modelPath.toUtf8().constData());
    if (!m_model) {
        emit initFailed("Voice model not found at: " + m_modelPath
                        + "\nPlace the 'vosk-model' folder next to HammerDown.exe.");
        return false;
    }

    m_rec = p_recognizer_new(m_model, 16000.0f);
    if (!m_rec) {
        emit initFailed("Failed to create Vosk recognizer.");
        return false;
    }
    // Lock recognizer to navigation vocabulary only
static const char *grammar = R"([
  "go home","open home","home",
  "open clients","open client","go to clients","clients",
  "open employees","open employee","go to employees","employees",
  "open equipment","go to equipment","equipment",
  "open suppliers","open supplier","go to suppliers","suppliers",
  "open orders","open order","go to orders","orders",
  "add client","view clients","modify client","client statistics",
  "send email","calendar","cyber trace","data matrix",
  "add employee","view employees","employee statistics","personnel records","contact",
  "manage equipment","view equipment","equipment history","equipment statistics",
  "chat","nexus","costs",
  "manage suppliers","view supplier","supplier statistics","supplier reviews","vicinity map",
  "manage orders","qr code","catalog","modeling","map",
  "start listening","stop listening"
])";
if (p_recognizer_set_grammar)
        p_recognizer_set_grammar(m_rec, grammar);


    QAudioFormat fmt;
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);

    m_source = new QAudioSource(fmt, this);
    m_ready  = true;
    return true;
}

void VoiceCommandEngine::startListening()
{
    if (!m_ready || m_listening) return;
    m_device    = m_source->start();
    m_listening = true;
    m_timer->start();
    emit listeningChanged(true);
}

void VoiceCommandEngine::stopListening()
{
    if (!m_listening) return;
    m_timer->stop();
    m_source->stop();
    m_device    = nullptr;
    m_listening = false;
    emit listeningChanged(false);
}

void VoiceCommandEngine::toggleListening()
{
    m_listening ? stopListening() : startListening();
}

void VoiceCommandEngine::processAudio()
{
    if (!m_device || !m_rec) return;
    const QByteArray data = m_device->readAll();
    if (data.isEmpty()) return;

    if (p_recognizer_accept_waveform(m_rec, data.constData(), data.size())) {
        const QJsonDocument doc = QJsonDocument::fromJson(
                                      QByteArray(p_recognizer_result(m_rec)));
        const QString text = doc.object().value("text").toString().trimmed().toLower();
        if (!text.isEmpty())
            emit commandDetected(text);
    }
}
