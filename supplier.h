#ifndef SUPPLIER_H
#define SUPPLIER_H

#include <QPoint>
#include <QNetworkReply>

class QProgressBar;
class QLabel;
class QTimeEdit;
class QPushButton;
class QWidget;
class QTextEdit;
class QNetworkAccessManager;

// Supplier declarations extracted from MainWindow and grouped by section.
#define MAINWINDOW_SUPPLIER_PUBLIC_DECLS \
    void setupSupplierStats(); \
    void setupSupplierModes(); \
    void setupSupplierMapTab(); \
    void setupSupplierAiAdvisorTab(); \
    void refreshSupplierMap(); \
    void loadSupplierMapPins(); \
    void checkSupplierVicinity(int supplierId = -1); \
    QString gatherSupplierContextForAi(const QString &materialType); \
    void checkWorkshopStockAndNotifyAI(); \
    void checkAndPostSupplierNotifications();

#define MAINWINDOW_SUPPLIER_SLOT_DECLS \
    void onSupplierClearFields(); \
    void onSupplierAdd(); \
    void onSupplierModify(); \
    void onSupplierDelete(); \
    void onSupplierDeleteAll(); \
    void onSupplierExportPDF(); \
    void onSupplierPrint(); \
    void onSupplierLoad(const QModelIndex &index); \
    void onSupplierSearch(); \
    void onSupplierRefreshView(); \
    void onSupplierSendSMS(); \
    void onSupplierUploadImage(); \
    void onSupplierGeocodeFinished(QNetworkReply *reply); \
    void onSupplierBellClicked(); \
    void onSupplierEnsureReviewsTable(); \
    void onSupplierReviewLoad(); \
    void onSupplierReviewSubmit(); \
    void onSupplierReviewRatingChanged(int value); \
    void onSupplierPopulateRatingCombos(); \
    void triggerPhoneAnimation(const QString &smsContent, const QString &phone);

#define MAINWINDOW_SUPPLIER_PRIVATE_DECLS \
    struct SupplierPin { \
        int id; \
        QString name; \
        QString type; \
        QString status; \
        QString openTime; \
        QString closeTime; \
        double lat; \
        double lon; \
        QRect rect; \
    }; \
    QList<SupplierPin> m_supplierPins; \
    int m_supplierGeocodePendingCount = 0; \
    QProgressBar *m_supplierProgress = nullptr; \
    QLabel *m_suppNameInd = nullptr; \
    QLabel *m_suppEmailInd = nullptr; \
    QLabel *m_suppTelInd = nullptr; \
    QLabel *m_suppTypeInd = nullptr; \
    void updateSupplierProgress(); \
    void playSupplierSuccessAnimation(const QString &supplierName); \
    void playSupplierModifyAnimation(const QString &supplierName); \
    void playSupplierDeleteAnimation(const QString &supplierName); \
    QNetworkAccessManager *m_supplierMapNet = nullptr; \
    QLabel *m_supplierMapImageLabel = nullptr; \
    QLabel *m_supplierMapStatusLabel = nullptr; \
    QPushButton *m_supplierMapRefreshBtn = nullptr; \
    QPushButton *m_supplierMapZoomInBtn = nullptr; \
    QPushButton *m_supplierMapZoomOutBtn = nullptr; \
    QTimeEdit *m_teOpeningHour = nullptr; \
    QTimeEdit *m_teClosingHour = nullptr; \
    QPushButton *m_supplierBellBtn = nullptr; \
    QWidget *m_supplierAiTab = nullptr; \
    QLabel *m_aiAdvStatus = nullptr; \
    QTextEdit *m_aiAdvResult = nullptr; \
    QPushButton *m_aiAdvRunBtn = nullptr; \
    QProgressBar *m_aiAdvProgress = nullptr; \
    QHash<QString, QPixmap> m_supplierMapTileCache; \
    QSet<QString> m_supplierMapPendingTiles; \
    int m_supplierMapZoom = 13; \
    QSize m_supplierMapImageSize = QSize(800, 500); \
    double m_supplierMapTopLeftX = 0.0; \
    double m_supplierMapTopLeftY = 0.0; \
    int m_supplierMapTileX0 = 0; \
    int m_supplierMapTileY0 = 0; \
    int m_supplierMapTileX1 = 0; \
    int m_supplierMapTileY1 = 0; \
    int m_supplierMapTileErrors = 0; \
    double m_supplierCenterLat = 36.8065; \
    double m_supplierCenterLon = 10.1815; \
    bool m_supplierMapDragging = false; \
    QPoint m_supplierMapDragStart; \
    double m_supplierMapDragCenterX = 0.0; \
    double m_supplierMapDragCenterY = 0.0; \
    QPoint m_supplierMapDragOffset; \
    QPixmap m_supplierMapCurrentPixmap; \
    bool m_supplierMapHasPixmap = false; \
    bool m_aiScanInProgress = false; \
    bool m_aiAdvisorStartupDone = false; \
    QSet<QString> m_aiNotifiedMaterials;

#endif // SUPPLIER_H
