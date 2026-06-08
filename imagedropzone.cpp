#include "imagedropzone.h"

ImageDropZone::ImageDropZone(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_AcceptDrops, true);
    setAlignment(Qt::AlignCenter);
    resetStyle();
    setText(tr("Drag & Drop\nImage Here"));
}

void ImageDropZone::resetStyle()
{
    setStyleSheet(
        "QLabel {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 rgba(139,111,71,0.15), stop:1 rgba(139,111,71,0.05));"
        "  border: 2px dashed #8B6F47;"
        "  border-radius: 12px;"
        "  color: #CCCCCC;"
        "  font-size: 13px;"
        "  font-style: italic;"
        "}"
    );
}

void ImageDropZone::dragEnterEvent(QDragEnterEvent *event)
{
    // Be lenient: Accept if it has any URLs/Files, we filter more strictly in dropEvent
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        setStyleSheet(
            "QLabel {"
            "  background: rgba(139,111,71,0.3);"
            "  border: 3px dashed #FFD700;"
            "  border-radius: 12px;"
            "  color: #FFD700;"
            "  font-size: 14px;"
            "  font-weight: bold;"
            "}"
        );
        setText(tr("\u2705 Drop to analyze!"));
    } else {
        event->ignore();
    }
}

void ImageDropZone::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void ImageDropZone::dragLeaveEvent(QDragLeaveEvent *)
{
    resetStyle();
    setText(tr("Drag & Drop\nImage Here"));
}

void ImageDropZone::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) { event->ignore(); return; }
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        if (isImageFile(path)) {
            event->acceptProposedAction();
            emit imageDropped(path);
            return;
        }
    }
    event->ignore();
}

bool ImageDropZone::isImageFile(const QString &path)
{
    QString low = path.toLower();
    return low.endsWith(".png") || low.endsWith(".jpg") ||
           low.endsWith(".jpeg") || low.endsWith(".bmp") ||
           low.endsWith(".gif")  || low.endsWith(".webp");
}

bool ImageDropZone::isImageDrop(const QMimeData *mime)
{
    if (!mime || !mime->hasUrls()) return false;
    for (const QUrl &u : mime->urls())
        if (isImageFile(u.toLocalFile())) return true;
    return false;
}
