#include <QApplication>
#include "mainwindow.h"
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    if (!createConnection()) {
        // Handle connection failure if necessary, e.g., show a message box
        // For now, allow the app to continue, but database features won't work
    }

    MainWindow w;
    w.show();
    return a.exec();
}
