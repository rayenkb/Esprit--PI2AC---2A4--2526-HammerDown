#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QStringList>

static void resetDefaultConnection()
{
    const QString connName = QStringLiteral("qt_sql_default_connection");
    if (QSqlDatabase::contains(connName)) {
        {
            QSqlDatabase db = QSqlDatabase::database(connName, false);
            if (db.isValid()) db.close();
        }
        QSqlDatabase::removeDatabase(connName);
    }
}

static bool tryQoci(const QString &serviceName)
{
    resetDefaultConnection();
    QSqlDatabase db = QSqlDatabase::addDatabase("QOCI");
    db.setHostName("localhost");
    db.setPort(1521);
    db.setDatabaseName(serviceName);
    db.setUserName("skrrt");
    db.setPassword("exprix");
    if (db.open()) return true;

    qDebug() << "QOCI failed for" << serviceName << ":" << db.lastError().text();
    return false;
}

static bool tryQodbc(const QString &serviceName)
{
    resetDefaultConnection();
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setUserName("skrrt");
    db.setPassword("exprix");

    // Try DSN/TNS alias first.
    db.setDatabaseName(serviceName);
    if (db.open()) return true;
    qDebug() << "QODBC alias failed for" << serviceName << ":" << db.lastError().text();

    // Then try direct Oracle connect strings with common ODBC driver names.
    const QStringList driverNames = {
        "Oracle in OraClient12Home1",
        "Oracle in OraDB21Home1",
        "Oracle in instantclient_21_14",
        "Oracle ODBC Driver"
    };

    for (const QString &drv : driverNames) {
        db.setDatabaseName(QString("DRIVER={%1};DBQ=localhost:1521/%2;UID=skrrt;PWD=exprix;")
                               .arg(drv, serviceName));
        if (db.open()) {
            qDebug() << "QODBC connected using" << drv << "for" << serviceName;
            return true;
        }
        qDebug() << "QODBC direct failed using" << drv << "for" << serviceName << ":" << db.lastError().text();
    }

    return false;
}

static bool createConnection()
{
    const QStringList servicesToTry = {"XEPDB1", "xe", "Source_Projet2A"};
    const QStringList drivers = QSqlDatabase::drivers();

    qDebug() << "Available SQL drivers:" << drivers;

    bool connected = false;

    if (drivers.contains("QOCI")) {
        for (const QString &svc : servicesToTry) {
            if (tryQoci(svc)) {
                connected = true;
                qDebug() << "Connected via QOCI to" << svc;
                break;
            }
        }
    }

    if (!connected && drivers.contains("QODBC")) {
        for (const QString &svc : servicesToTry) {
            if (tryQodbc(svc)) {
                connected = true;
                qDebug() << "Connected via QODBC to" << svc;
                break;
            }
        }
    }

    if (!connected) {
        qDebug() << "Database connection failed for all drivers/services.";
        return false;
    }
    
    qDebug() << "Database connected successfully!";
    
    // Test query to verify connection
    QSqlQuery testQuery;
    if (testQuery.exec("SELECT 1 FROM DUAL")) {
        qDebug() << "Database test query successful!";
    } else {
        qDebug() << "Database test query failed: " << testQuery.lastError().text();
    }
    
    return true;
}

#endif // CONNECTION_H
