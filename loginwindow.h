#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QFrame>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCamera>
#include <QVideoSink>
#include <QMediaCaptureSession>
#include <QVideoFrame>
#include <QDir>
#include <QTimer>
#include <QMediaDevices>
#include <QCameraDevice>

namespace Ui { class LoginFrame; }

class LoginWindow : public QFrame {
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void retranslateUI();
    QPixmap getCircularPixmap(const QPixmap &src);

signals:
    void loginSuccessful(int employeeId);

private slots:
    void handleLogin();
    void handleFaceLogin();
    void handleForgotPassword();
    void processCameraFrame();

private:
    Ui::LoginFrame *ui;
    QCamera *m_camera = nullptr;
    QMediaCaptureSession *m_captureSession = nullptr;
    QVideoSink *m_videoSink = nullptr;
    bool m_isFaceLoginActive = false;
};

#endif // LOGINWINDOW_H
