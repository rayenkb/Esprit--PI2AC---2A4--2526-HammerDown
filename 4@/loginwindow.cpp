#include "loginwindow.h"
#include "ui_login.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::LoginFrame)
{
    ui->setupUi(this);
    
    // Ensure the frame expands to fill available space
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    connect(ui->login, &QPushButton::clicked, this, &LoginWindow::handleLogin);
}

void LoginWindow::handleLogin() {
    // Bypass authentication as requested
    emit loginSuccessful();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}
