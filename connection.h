#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QProcessEnvironment>

static bool createConnection()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString dbHost = env.value("DB_HOST", "localhost");
    int dbPort = env.value("DB_PORT", "1521").toInt();
    QString dbName = env.value("DB_NAME", "source_2a4");
    QString dbUser = env.value("DB_USER", "SYSTEM");
    QString dbPass = env.value("DB_PASS", "esprit1");

    // METHOD 1: Try Oracle native driver first (QOCI)
    QSqlDatabase db = QSqlDatabase::addDatabase("QOCI");
    db.setHostName(dbHost);
    db.setPort(dbPort);
    db.setDatabaseName(dbName);
    db.setUserName(dbUser);
    db.setPassword(dbPass);

    if (!db.open()) {
        qDebug() << "QOCI driver failed, trying ODBC...";
        
        // METHOD 2: Fallback to ODBC
        QSqlDatabase::removeDatabase("qt_sql_default_connection");
        db = QSqlDatabase::addDatabase("QODBC");
        
        // Option A: Use TNS name (if configured in tnsnames.ora)
        db.setDatabaseName(dbName);
        
        /* Option B: Full connection string (uncomment if Option A doesn't work)
        db.setDatabaseName(
            QString("DRIVER={Oracle in OraClient12Home1};"
                    "DBQ=%1:%2/%3;"
                    "UID=%4;"
                    "PWD=%5;")
            .arg(dbHost).arg(dbPort).arg(dbName).arg(dbUser).arg(dbPass)
        );
        */
        
        db.setUserName(dbUser);
        db.setPassword(dbPass);
        
        if (!db.open()) {
            qDebug() << "Database connection failed!";
            qDebug() << "Error: " << db.lastError().text();
            qDebug() << "Driver error: " << db.lastError().driverText();
            qDebug() << "Database error: " << db.lastError().databaseText();
            
            // List available drivers
            qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();
            return false;
        }
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
