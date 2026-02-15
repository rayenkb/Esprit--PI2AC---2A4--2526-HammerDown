#ifndef BUTTONANIMATOR_H
#define BUTTONANIMATOR_H

#include <QObject>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEvent>

class ButtonAnimator : public QObject
{
    Q_OBJECT

public:
    static void applyHoverAnimation(QPushButton* button);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    explicit ButtonAnimator(QPushButton* button, QObject* parent = nullptr);
    
    QPushButton* m_button;
    QPropertyAnimation* m_animation;
    QString m_originalStyle;
    QString m_hoverStyle;
};

#endif // BUTTONANIMATOR_H
