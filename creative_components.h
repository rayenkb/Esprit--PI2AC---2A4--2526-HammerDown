#ifndef CREATIVE_COMPONENTS_H
#define CREATIVE_COMPONENTS_H

#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QEasingCurve>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <QKeyEvent>

// ============================================================================
// 1. RADIAL COMMAND WHEEL — Game-like circular menu
// ============================================================================
class RadialCommandMenu : public QFrame {
    Q_OBJECT
public:
    explicit RadialCommandMenu(QWidget *parent = nullptr);
    void showMenu(const QPoint &globalPos, int equipmentId);
    void animateOpen();
    void animateClose();

signals:
    void actionSelected(const QString &action, int id);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *) override { m_hoverIdx = -1; update(); }
    void hideEvent(QHideEvent *) override { m_hoverIdx = -1; }
    void keyPressEvent(QKeyEvent *e) override { if(e->key() == Qt::Key_Escape) animateClose(); }

private:
    struct Action { QString name; QString description; QColor color; };
    QList<Action> m_actions;
    int m_hoverIdx = -1;
    int m_activeId = -1;
    QRect getItemRect(int i) const;
};

// ============================================================================
// 2. DIGITAL TWIN IDENTITY CARD — Premium side-panel slide-out
// ============================================================================
class IdentityCard : public QFrame {
    Q_OBJECT
public:
    explicit IdentityCard(QWidget *parent = nullptr);
    void setup(const QString &name, const QString &type, const QString &status, double price, int id);
    void animateOpen();
    void animateClose();

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    void drawBlueprint(QPainter &p, const QRect &r);
    void drawHealthGauge(QPainter &p, const QRect &r, int score);
    void drawQRCode(QPainter &p, const QRect &r, int id);

    QString m_name, m_type, m_status;
    double m_price;
    int m_id;
    int m_healthScore = 85;
    float m_scanLineY = 0.0f;
    QTimer *m_scanTimer;
};

// ============================================================================
// 3. NEURAL CHAT BUBBLE — High-fidelity glassmorphism message widget
// ============================================================================
class ChatBubble : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)
public:
    explicit ChatBubble(const QString &msg, const QString &time, bool isMe, const QString &senderName, int index, QWidget *parent = nullptr);
    void setImage(const QByteArray &data);
    void setGif(const QString &url);
    void setVoiceNote(bool isVoice);
    
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal o) { m_opacity = o; update(); }
    qreal offset() const { return m_offset; }
    void setOffset(qreal o) { m_offset = o; update(); }

    void animateEntrance();

signals:
    void deleteRequested(int index);
    void gif_loaded();

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override { m_hover = false; update(); }
    void mousePressEvent(QMouseEvent *e) override;
    QSize sizeHint() const override;

private:
    QString m_message, m_time, m_sender;
    bool m_isMe;
    int m_index;
    bool m_hover = false;
    qreal m_opacity = 0.0;
    qreal m_offset = 20.0;
    
    QPixmap m_image;
    bool m_hasImage = false;
    
    QString m_gifUrl;
    QMovie *m_gifMovie = nullptr;
    bool m_hasGif = false;
    
    bool m_isVoiceNote = false;
    QRect m_deleteRect;
};

// ============================================================================
// 4. CHAT EMPLOYEE DELEGATE — Premium list item rendering
// ============================================================================
#include <QStyledItemDelegate>
class ChatEmployeeDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // CREATIVE_COMPONENTS_H
