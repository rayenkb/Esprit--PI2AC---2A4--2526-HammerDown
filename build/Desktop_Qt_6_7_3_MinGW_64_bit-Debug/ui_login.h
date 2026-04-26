/********************************************************************************
** Form generated from reading UI file 'login.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_LoginFrame
{
public:
    QGridLayout *mainGridLayout;
    QSpacerItem *topSpacer;
    QLineEdit *login_id;
    QSpacerItem *gap1;
    QLineEdit *login_pass;
    QPushButton *btn_forgot_password;
    QPushButton *login;
    QLabel *lbl_camera_preview;
    QSpacerItem *leftSpacer;
    QSpacerItem *rightSpacer;
    QPushButton *btn_face_login;

    void setupUi(QFrame *LoginFrame)
    {
        if (LoginFrame->objectName().isEmpty())
            LoginFrame->setObjectName("LoginFrame");
        LoginFrame->resize(1336, 675);
        LoginFrame->setStyleSheet(QString::fromUtf8("#LoginFrame {\n"
"    border-image: url(:/assets/login.png) 0 0 0 0 stretch stretch;\n"
"}\n"
""));
        mainGridLayout = new QGridLayout(LoginFrame);
        mainGridLayout->setSpacing(0);
        mainGridLayout->setObjectName("mainGridLayout");
        mainGridLayout->setContentsMargins(0, 0, 0, 0);
        topSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        mainGridLayout->addItem(topSpacer, 0, 0, 1, 5);

        login_id = new QLineEdit(LoginFrame);
        login_id->setObjectName("login_id");
        QFont font;
        font.setFamilies({QString::fromUtf8("MS Shell Dlg 2")});
        font.setPointSize(16);
        login_id->setFont(font);
        login_id->setStyleSheet(QString::fromUtf8("background: transparent; border: none; color: white;"));
        login_id->setFrame(false);

        mainGridLayout->addWidget(login_id, 1, 1, 1, 3);

        gap1 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        mainGridLayout->addItem(gap1, 2, 1, 1, 3);

        login_pass = new QLineEdit(LoginFrame);
        login_pass->setObjectName("login_pass");
        QFont font1;
        font1.setPointSize(16);
        login_pass->setFont(font1);
        login_pass->setStyleSheet(QString::fromUtf8("background: transparent; border: none; color: white; margin-top: 5px;"));
        login_pass->setFrame(false);
        login_pass->setEchoMode(QLineEdit::EchoMode::Password);

        mainGridLayout->addWidget(login_pass, 3, 1, 1, 3, Qt::AlignTop);

        btn_forgot_password = new QPushButton(LoginFrame);
        btn_forgot_password->setObjectName("btn_forgot_password");
        btn_forgot_password->setMinimumSize(QSize(200, 30));
        btn_forgot_password->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_forgot_password->setStyleSheet(QString::fromUtf8("background: transparent; border: none;"));

        mainGridLayout->addWidget(btn_forgot_password, 4, 2, 1, 2);

        login = new QPushButton(LoginFrame);
        login->setObjectName("login");
        login->setGeometry(QRect(0, 0, 200, 40));
        login->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        login->setStyleSheet(QString::fromUtf8("#login {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 5px; }"));
        login->setFlat(true);

        mainGridLayout->addWidget(login, 5, 2, 1, 1);

        lbl_camera_preview = new QLabel(LoginFrame);
        lbl_camera_preview->setObjectName("lbl_camera_preview");
        lbl_camera_preview->setVisible(false);
        lbl_camera_preview->setMinimumSize(QSize(400, 400));
        lbl_camera_preview->setMaximumSize(QSize(400, 400));
        lbl_camera_preview->setStyleSheet(QString::fromUtf8("border: 4px solid #8B6F47; border-radius: 200px; background-color: black;"));
        lbl_camera_preview->setAlignment(Qt::AlignmentFlag::AlignCenter);

        mainGridLayout->addWidget(lbl_camera_preview, 6, 1, 1, 3);

        leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        mainGridLayout->addItem(leftSpacer, 0, 0, 7, 1);

        rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        mainGridLayout->addItem(rightSpacer, 0, 4, 7, 1);

        btn_face_login = new QPushButton(LoginFrame);
        btn_face_login->setObjectName("btn_face_login");
        btn_face_login->setMinimumSize(QSize(180, 44));
        btn_face_login->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_face_login->setStyleSheet(QString::fromUtf8("#btn_face_login {\n"
"    background-color: rgba(139, 111, 71, 0.3);\n"
"    border: 1.5px solid #8B6F47;\n"
"    border-radius: 22px;\n"
"    color: white;\n"
"    font-weight: bold;\n"
"    font-size: 14px;\n"
"    margin-right: 30px;\n"
"    margin-bottom: 20px;\n"
"}\n"
"#btn_face_login:hover { background-color: rgba(139, 111, 71, 0.5); border-color: #D4AF37; }"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/id.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_face_login->setIcon(icon);
        btn_face_login->setIconSize(QSize(24, 24));
        btn_face_login->setFlat(true);

        mainGridLayout->addWidget(btn_face_login, 6, 4, 1, 1, Qt::AlignRight|Qt::AlignBottom);

        mainGridLayout->setRowStretch(0, 205);
        mainGridLayout->setRowStretch(1, 50);
        mainGridLayout->setRowStretch(2, 43);
        mainGridLayout->setRowStretch(3, 55);
        mainGridLayout->setRowStretch(4, 140);
        mainGridLayout->setRowStretch(5, 30);
        mainGridLayout->setRowStretch(6, 152);
        mainGridLayout->setColumnStretch(0, 470);
        mainGridLayout->setColumnStretch(1, 70);
        mainGridLayout->setColumnStretch(2, 240);
        mainGridLayout->setColumnStretch(3, 191);
        mainGridLayout->setColumnStretch(4, 365);

        retranslateUi(LoginFrame);

        QMetaObject::connectSlotsByName(LoginFrame);
    } // setupUi

    void retranslateUi(QFrame *LoginFrame)
    {
        LoginFrame->setWindowTitle(QCoreApplication::translate("LoginFrame", "Log In", nullptr));
        btn_forgot_password->setText(QString());
        login->setText(QString());
        lbl_camera_preview->setText(QString());
#if QT_CONFIG(tooltip)
        btn_face_login->setToolTip(QCoreApplication::translate("LoginFrame", "Login with Face Recognition", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_face_login->setText(QCoreApplication::translate("LoginFrame", " Face Scan Login", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginFrame: public Ui_LoginFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGIN_H
