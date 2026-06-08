#ifndef LOREGUIDEWIDGET_H
#define LOREGUIDEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSizeGrip>
#include <QPoint>

class LoreGuideWidget : public QWidget
{
    Q_OBJECT

public:
    enum CharacterState { Normal, Excited, Confused };

    explicit LoreGuideWidget(QMediaPlayer *bgMusic, QWidget *parent = nullptr);
    ~LoreGuideWidget();

    void setCharacterState(CharacterState state);
    void setBgMusic(QMediaPlayer *p) { m_bgMusic = p; }
    void setAnimationUnlocked(bool enabled);

signals:
    void closeRequested();

private slots:
    void sendMessage();
    void onApiReplyFinished(QNetworkReply *reply);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    bool handleLocalCommand(const QString &text, QString *responseOut);
    QString handleAddRandomOrders(int count);
    QString handleAddRandomEmployees(int count);
    QString handleAddRandomClients(int count);
    QString handleAddRandomSuppliers(int count);
    QString handleAddRandomEquipment(int count);
    int getNextId(const QString &tableName, const QString &idColumn, int fallback = 1);
    QStringList getColumnValuesFromCheckConstraints(const QString &tableName, const QString &columnName);
    QString getColumnDefaultValue(const QString &tableName, const QString &columnName);
    QStringList getDistinctColumnValues(const QString &tableName, const QString &columnName);
    QStringList getAllowedColumnValues(const QString &tableName, const QString &columnName);
    void callApi(const QString &userMessage, bool isSystemRetry = false);
    void retryWithNextModel();
    void callImageApi(const QString &prompt);
    bool isImageRequest(const QString &text) const;
    QString extractImagePrompt(const QString &text) const;
    void appendBubble(const QString &text, bool isUser);
    void appendImageBubble(const QPixmap &pixmap, const QString &caption);
    void trimConversationHistory(int maxNonSystemMessages);
    QString buildSystemPrompt() const;
    QString processResponse(const QString &response);
    QString executeSqlCommand(const QString &sql);

    QLabel          *m_characterLabel;
    QFrame          *m_titleBar;
    QSizeGrip       *m_sizeGrip;
    QScrollArea     *m_scrollArea;
    QWidget         *m_bubbleContainer;
    QVBoxLayout     *m_bubbleLayout;
    QLineEdit       *m_input;
    QPushButton     *m_sendBtn;
    QLabel          *m_typingLabel;

    QNetworkAccessManager *m_network;
    QJsonArray       m_history;
    QString          m_apiKey;
    QStringList      m_modelList;
    int              m_retryCount;
    int              m_rateLimitRetries;
    int              m_sqlRetryCount;
    QString          m_pendingUserMessage;

    QMediaPlayer    *m_bgMusic;  // the botawk player to keep alive

    QPixmap m_pixNormal;
    QPixmap m_pixExcited;
    QPixmap m_pixConfused;

    bool m_animationUnlocked = false;
    bool m_dragging = false;
    QPoint m_dragStartPos;
    QPoint m_windowStartPos;
};

#endif // LOREGUIDEWIDGET_H
