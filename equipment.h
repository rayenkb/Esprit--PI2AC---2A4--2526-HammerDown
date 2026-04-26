#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <QPoint>
#include <QListWidgetItem>

class QProgressBar;
class QLabel;
class QTimer;
class QMediaRecorder;
class QAudioOutput;
class QNetworkAccessManager;
class QWidget;
class NexusWidget;
class CostsWidget;
class EquipmentHoverCard;

// Equipment declarations extracted from MainWindow and grouped by section.
#define MAINWINDOW_EQUIPMENT_PUBLIC_DECLS \
    void setupEquipmentStats(); \
    void ensureEquipmentHistoryDatabaseObjects(); \
    void setupEquipmentModes(); \
    void setupEquipmentConnections();

#define MAINWINDOW_EQUIPMENT_SLOT_DECLS \
    void on_gs_equipment_clicked(); \
    void on_nav_equipments_clicked(); \
    void onEquipmentClearFields(); \
    void onEquipmentShareToChat(); \
    void onEquipmentAdd(); \
    void onEquipmentModify(); \
    void onEquipmentDelete(); \
    void onEquipmentRefreshView(); \
    void onEquipmentSearch(); \
    void onEquipmentHistoryRefresh(); \
    void onEquipmentHistorySearch(); \
    void onEquipmentHistoryClear(); \
    void onEquipmentCustomContextMenu(const QPoint &pos); \
    void onEquipmentHistoryCustomContextMenu(const QPoint &pos, int tableIdx); \
    void onEquipmentExportPDF(); \
    void onEquipmentExportStatsPDF(); \
    void onEquipmentBulkUpdateStatus(); \
    void onEquipmentDeleteAll(); \
    void onChatEnsureTable(); \
    void onChatSendMessage(); \
    void onChatRefresh(); \
    void onChatEmployeeListRefresh(); \
    void onChatEmployeeSelected(QListWidgetItem *item); \
    void onChatAttachImage(); \
    void onChatDeleteMessage(int index); \
    void onChatSettingsClicked(); \
    void onChatEmojiClicked(); \
    void onChatGifClicked(); \
    void onChatSearchToggle(); \
    void onChatStartRecord(); \
    void onChatStopRecord(); \
    void onChatVoiceToggled(); \
    void onWeatherAssistantClicked(); \
    void setupChatForgeVisuals(); \
    void enforceChatTabTopOffset();

#define MAINWINDOW_EQUIPMENT_PRIVATE_DECLS \
    QProgressBar *m_equipProgress = nullptr; \
    QLabel *m_eqTypeInd = nullptr, *m_eqDateInd = nullptr, *m_eqPriceInd = nullptr, *m_eqDescInd = nullptr; \
    void updateEquipProgress(); \
    void playEquipSuccessAnimation(const QString &equipName); \
    NexusWidget *m_nexusWidget = nullptr; \
    CostsWidget *m_costsWidget = nullptr; \
    EquipmentHoverCard *m_hoverCard = nullptr; \
    QTimer *equipmentSyncTimer = nullptr; \
    QMediaRecorder *m_recorder = nullptr; \
    bool m_isRecording = false; \
    QAudioOutput *chatAudioOutput = nullptr; \
    QMediaPlayer *chatAudioPlayer = nullptr; \
    QMediaCaptureSession *m_captureSession = nullptr; \
    QAudioInput *m_audioInput = nullptr; \
    QCompleter *m_chatCompleter = nullptr; \
    QStringListModel *m_completerModel = nullptr; \
    QWidget *m_chatAmbientLayer = nullptr; \
    QByteArray pendingChatImage; \
    int currentChatPartnerId = -1; \
    QNetworkAccessManager *aiNetworkManager = nullptr; \
    QString aiApiKey; \
    QNetworkAccessManager *giphyNetworkManager = nullptr; \
    QNetworkAccessManager *chatSummaryNetManager = nullptr; \
    QTimer *chatRefreshTimer = nullptr; \
    bool m_isChatModernTheme = false; \
    bool m_resumeChatAfterOstp = false; \
    qint64 m_chatResumePosAfterOstp = 0; \
    void shakeWidget(QWidget *w); \
    void showUnreadMessagesSplash();

#endif // EQUIPMENT_H
