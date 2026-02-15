#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QFrame>
#include <QMessageBox>

namespace Ui { class LoginFrame; }

class LoginWindow : public QFrame {
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccessful();

private slots:
    void handleLogin();

private:
    Ui::LoginFrame *ui;
};

#endif // LOGINWINDOW_H
