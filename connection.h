#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

static bool createConnection()
{
    // METHOD 1: Try Oracle native driver first (QOCI)
    QSqlDatabase db = QSqlDatabase::addDatabase("QOCI");
    db.setHostName("localhost");
    db.setPort(1521);
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("RAMII");
    db.setPassword("rami123");

    if (!db.open()) {
        qDebug() << "QOCI driver failed, trying ODBC...";

        // METHOD 2: Fallback to ODBC
        QSqlDatabase::removeDatabase("qt_sql_default_connection");
        db = QSqlDatabase::addDatabase("QODBC");

        // Option A: Use TNS name (if configured in tnsnames.ora)
        db.setDatabaseName("Source_Projet2A");

        /* Option B: Full connection string (uncomment if Option A doesn't work)
        db.setDatabaseName(
            "DRIVER={Oracle in OraClient12Home1};"
            "DBQ=localhost:1521/Source_Projet2A;"
            "UID=skrrt;"
            "PWD=exprix;"
        );
        */

        db.setUserName("RAMII");
        db.setPassword("rami123");
        
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
