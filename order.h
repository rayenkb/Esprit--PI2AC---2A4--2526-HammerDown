#ifndef ORDER_H
#define ORDER_H

#include <QPoint>
#include <QNetworkReply>

class QTableWidget;
class QTabWidget;
class QNetworkAccessManager;
class QLabel;
class QPushButton;
class QDialog;
class QGraphicsBlurEffect;

// Order declarations extracted from MainWindow and grouped by section.
#define MAINWINDOW_ORDER_PUBLIC_DECLS \
    void setupOrderModes(); \
    void setupOrderMapTab(); \
    void setupOrderCatalogResolutionTabs();

#define MAINWINDOW_ORDER_SLOT_DECLS \
    void onOrderClearFields(); \
    void onOrderAdd(); \
    void onOrderModify(); \
    void onOrderDelete(); \
    void onOrderDeleteAll(); \
    void onOrderLoad(); \
    void onOrderRefreshCatalog(); \
    void onOrderSearchCatalog(); \
    void onOrderExportCatalog(); \
    void onOrderImportCatalog(); \
    void onOrderPrintCatalog(); \
    void onGenerateQR(); \
    void onSaveQR(); \
    void onPrintQR(); \
    void onMapNetworkFinished(QNetworkReply *reply);

#define MAINWINDOW_ORDER_PRIVATE_DECLS \
    void configureOrderCatalogTable(QTableWidget *table); \
    bool populateOrderCatalogTable(QTableWidget *table, const QString &searchText, bool resolvedOnly); \
    void markOrderAsPaid(int orderId); \
    void markOrderAsUnpaid(int orderId); \
    void requestMapForBuyerId(); \
    void populateMapClients(); \
    void requestMapTiles(double lat, double lon); \
    void renderOrderMap(); \
    QTabWidget *m_orderCatalogStatusTabs = nullptr; \
    QTableWidget *m_orderCatalogUnresolvedTable = nullptr; \
    QTableWidget *m_orderCatalogResolvedTable = nullptr; \
    QNetworkAccessManager *m_mapNet = nullptr; \
    QLabel *m_mapImageLabel = nullptr; \
    QLabel *m_mapStatusLabel = nullptr; \
    QLabel *m_mapAddressLabel = nullptr; \
    QLabel *m_mapAssignedEmployeeLabel = nullptr; \
    QLabel *m_mapDeliveryInfoLabel = nullptr; \
    QPushButton *m_mapRefreshBtn = nullptr; \
    QTableWidget *m_mapClientTable = nullptr; \
    QPushButton *m_mapZoomInBtn = nullptr; \
    QPushButton *m_mapZoomOutBtn = nullptr; \
    QPushButton *m_mapFullscreenBtn = nullptr; \
    QDialog *m_mapFullscreenDialog = nullptr; \
    QLabel *m_mapFullscreenLabel = nullptr; \
    QGraphicsBlurEffect *m_mapBlurEffect = nullptr; \
    QHash<QString, QPixmap> m_mapTileCache; \
    QSet<QString> m_mapPendingTiles; \
    int m_mapZoom = 14; \
    QSize m_mapImageSize = QSize(640, 360); \
    double m_mapTopLeftX = 0.0; \
    double m_mapTopLeftY = 0.0; \
    int m_mapTileX0 = 0; \
    int m_mapTileY0 = 0; \
    int m_mapTileX1 = 0; \
    int m_mapTileY1 = 0; \
    int m_mapTileErrors = 0; \
    int m_mapExpectedTiles = 0; \
    int m_mapLoadedTiles = 0; \
    double m_mapCenterLat = 36.8065; \
    double m_mapCenterLon = 10.1815; \
    bool m_mapHasClientPin = false; \
    double m_mapClientPinLat = 0.0; \
    double m_mapClientPinLon = 0.0; \
    int m_mapSelectedClientId = 0; \
    QString m_mapSelectedClientName; \
    bool m_mapHasEmployeePin = false; \
    double m_mapEmployeePinLat = 0.0; \
    double m_mapEmployeePinLon = 0.0; \
    QVector<QPointF> m_mapRouteGeoPoints; \
    int m_mapAssignedEmployeeId = 0; \
    QString m_mapAssignedEmployeeName; \
    QString m_mapPendingEmployeeAddress; \
    bool m_mapDragging = false; \
    QPoint m_mapDragStart; \
    double m_mapDragCenterX = 0.0; \
    double m_mapDragCenterY = 0.0; \
    QPoint m_mapDragOffset; \
    QPixmap m_mapCurrentPixmap; \
    bool m_mapHasPixmap = false;

#endif // ORDER_H
