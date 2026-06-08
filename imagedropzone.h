#pragma once
#include <QLabel>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

// A QLabel subclass that natively handles drag-and-drop image files.
// Using a subclass is the ONLY reliable way to capture drop events on a
// child widget deep inside QGroupBox -> QTabWidget hierarchies.
class ImageDropZone : public QLabel
{
    Q_OBJECT
public:
    explicit ImageDropZone(QWidget *parent = nullptr);
    void resetStyle();

signals:
    void imageDropped(const QString &filePath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    static bool isImageFile(const QString &path);
    static bool isImageDrop(const QMimeData *mime);
};
