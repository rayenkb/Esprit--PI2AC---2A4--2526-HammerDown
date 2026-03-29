#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("amine");
    db.setUserName("amine"); // PLCHOLDER: Update with actual username
    db.setPassword("amine14"); // PLCHOLDER: Update with actual password

    if (!db.open()) {
        qDebug() << "Database error: " << db.lastError().text();
        return false;
    }
    
    qDebug() << "Database connected successfully!";
    return true;
}

#endif // CONNECTION_H
