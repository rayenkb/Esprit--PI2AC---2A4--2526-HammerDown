#ifndef CHATBOTDIALOG_H
#define CHATBOTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollArea>
#include <QScrollBar>
#include <QFrame>
#include <QTimer>
#include <QSizeGrip>
#include <QPoint>

class ChatBotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatBotDialog(QWidget *parent = nullptr, bool isWeatherBot = false);
    ~ChatBotDialog();

    void setWeatherMode(bool enable) { m_isWeatherBot = enable; }
    bool isWeatherMode() const { return m_isWeatherBot; }
    // Called externally to inject a message and get AI response
    void sendExternalMessage(const QString &text);

private slots:
    void sendMessage();
    void onApiReplyFinished(QNetworkReply *reply);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void appendMessage(const QString &sender, const QString &text, bool isUser);
    void appendImageMessage(const QString &sender, const QPixmap &pixmap, const QString &caption, bool isUser);
    void setupUI();
    QString buildSystemPrompt() const;
    void trimConversationHistory(int maxNonSystemMessages);
    bool handleLocalCommand(const QString &text, QString *responseOut);
    QString handleAddRandomOrders(int count);
    void callApi(const QString &userMessage);
    void callImageApi(const QString &prompt);
    void retryWithNextModel();
    bool isImageRequest(const QString &text) const;
    QString extractImagePrompt(const QString &text) const;
    QString processResponse(const QString &response);
    QString executeSqlCommand(const QString &sql);

    QVBoxLayout *chatLayout;
    QScrollArea *scrollArea;
    QWidget *chatContainer;
    QLineEdit *inputField;
    QPushButton *sendButton;
    QLabel *typingIndicator;
    QFrame *titleBar;
    QSizeGrip *sizeGrip;
    bool m_dragging = false;
    QPoint m_dragStartPos;
    QPoint m_windowStartPos;
    QNetworkAccessManager *networkManager;
    // OpenAI-format conversation history
    QJsonArray conversationHistory;
    QString apiKey;
    QString imageApiUrl;
    QString imageModel;
    bool m_isWeatherBot;
    int retryCount;
    QString pendingUserMessage;
    QStringList modelList;
};

#endif // CHATBOTDIALOG_H
