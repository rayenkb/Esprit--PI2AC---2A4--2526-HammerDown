#include "buttonanimator.h"
#include <QEvent>
#include <QStyle>

ButtonAnimator::ButtonAnimator(QPushButton* button, QObject* parent)
    : QObject(parent)
    , m_button(button)
{
    m_button->installEventFilter(this);
    
    // Store original style
    m_originalStyle = m_button->styleSheet();
    
    // Create a hover style that adds a bright white border
    // We'll apply this directly when hovering
    m_hoverStyle = m_originalStyle;
    
    // Remove any existing border and add our hover border
    if (!m_hoverStyle.isEmpty() && !m_hoverStyle.endsWith(";")) {
        m_hoverStyle += ";";
    }
    m_hoverStyle += " border: 3px solid white; border-radius: 5px;";
}

void ButtonAnimator::applyHoverAnimation(QPushButton* button)
{
    // Create and attach animator (it will auto-delete with button)
    new ButtonAnimator(button, button);
}

bool ButtonAnimator::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_button) {
        if (event->type() == QEvent::Enter) {
            // Mouse entered - apply hover style
            m_button->setStyleSheet(m_hoverStyle);
            m_button->update();
        }
        else if (event->type() == QEvent::Leave) {
            // Mouse left - restore original style
            m_button->setStyleSheet(m_originalStyle);
            m_button->update();
        }
    }
    return QObject::eventFilter(obj, event);
}

