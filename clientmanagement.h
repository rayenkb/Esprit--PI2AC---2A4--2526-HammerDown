#ifndef CLIENTMANAGEMENT_H
#define CLIENTMANAGEMENT_H

#include <QModelIndex>
#include <QString>

class QTableView;
class QFrame;
class QSerialPort;

// Client management declarations extracted from MainWindow and grouped by section.

#define MAINWINDOW_CLIENT_PUBLIC_DECLS \
    void setupClientStats(); \
    void setupClientManagement(); \
    void setupClientDataMatrix(); \
    void setupClientCalendar();

#define MAINWINDOW_CLIENT_SLOT_DECLS \
    void onClientClearFields(); \
    void onClientModClearFields(); \
    void onClientAdd(); \
    void onClientModify(); \
    void onClientDelete(); \
    void onClientRefreshView(); \
    void onClientSearch(); \
    void onClientExportPDF(); \
    void onClientRowSelected(const QModelIndex &index); \
    void onClientCyberTraceRefresh(); \
    void onClientSendMail(); \
    void onClientBrowseMail(); \
    void sendToArduinoLCD(const QString &line1, const QString &line2); \
    void onClientLCDTotalClients(); \
    void onClientLCDGenderDist(); \
    void onClientLCDTopEmail();

#define MAINWINDOW_CLIENT_PRIVATE_DECLS \
    QTableView *m_clientCyberTable = nullptr; \
    QFrame     *m_clientMatrixFrame = nullptr;

#endif // CLIENTMANAGEMENT_H
