#include <QApplication>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QIcon>
#include "mainwindow.h"
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/assets/logo.png"));
    
    if (!createConnection()) {
        QMessageBox::warning(nullptr, "Database Connection Error",
                           "Failed to connect to the database.\n"
                           "Please check your database configuration and try again.\n\n"
                           "The application will continue but database features won't work.");
    } else {
        // Wrap test queries in a scope to ensure they are destroyed before MainWindow starts
        {
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT COUNT(*) FROM CLIENTS WHERE CLIENT_ID = 1");
            
            if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() == 0) {
                // Client doesn't exist, insert it
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO CLIENTS (CLIENT_ID, FIRST_NAME, LAST_NAME) "
                                  "VALUES (1, 'Test', 'Client')");
                
                if (insertQuery.exec()) {
                    qDebug() << "Test client with CLIENT_ID=1 inserted successfully";
                } else {
                    qDebug() << "Failed to insert test client:" << insertQuery.lastError().databaseText();
                }
            } else {
                qDebug() << "Test client with CLIENT_ID=1 already exists or query failed";
            }
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
