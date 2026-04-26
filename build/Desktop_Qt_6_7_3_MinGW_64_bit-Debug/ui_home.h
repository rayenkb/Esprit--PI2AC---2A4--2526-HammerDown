/********************************************************************************
** Form generated from reading UI file 'home.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOME_H
#define UI_HOME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_HomeFrame
{
public:
    QPushButton *btn_settings;
    QPushButton *btn_disconnect;
    QPushButton *btn_chat;
    QFrame *profile_frame;
    QHBoxLayout *profileLayout;
    QLabel *lbl_user_avatar;
    QVBoxLayout *userInfoLayout;
    QLabel *lbl_user_name;
    QLabel *lbl_user_role;
    QPushButton *btn_help;
    QGridLayout *mainGridLayout;
    QPushButton *gs_employes;
    QPushButton *gs_client;
    QPushButton *gs_order;
    QPushButton *gs_equipment;
    QPushButton *gs_fournisseur;
    QSpacerItem *topSpacer;
    QSpacerItem *bottomSpacer;
    QSpacerItem *leftSpacer;
    QSpacerItem *rightSpacer;

    void setupUi(QFrame *HomeFrame)
    {
        if (HomeFrame->objectName().isEmpty())
            HomeFrame->setObjectName("HomeFrame");
        HomeFrame->resize(1311, 657);
        HomeFrame->setStyleSheet(QString::fromUtf8("#HomeFrame {\n"
" border-image: url(:/assets/home.png) 0 0 0 0 stretch stretch;\n"
"}"));
        HomeFrame->setFrameShape(QFrame::Shape::StyledPanel);
        HomeFrame->setFrameShadow(QFrame::Shadow::Raised);
        btn_settings = new QPushButton(HomeFrame);
        btn_settings->setObjectName("btn_settings");
        btn_settings->setGeometry(QRect(20, 20, 50, 50));
        btn_settings->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_settings->setStyleSheet(QString::fromUtf8("\n"
"     QPushButton {\n"
"         background-color: transparent;\n"
"         border: none;\n"
"         border-radius: 25px;\n"
"         image: url(:/assets/gear.png);\n"
"     }\n"
"     QPushButton:hover {\n"
"         background-color: rgba(255, 255, 255, 0.2);\n"
"     }\n"
"     QPushButton:pressed {\n"
"         background-color: rgba(255, 255, 255, 0.3);\n"
"     }\n"
"    "));
        btn_settings->setFlat(true);
        btn_disconnect = new QPushButton(HomeFrame);
        btn_disconnect->setObjectName("btn_disconnect");
        btn_disconnect->setGeometry(QRect(80, 20, 50, 50));
        btn_disconnect->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_disconnect->setStyleSheet(QString::fromUtf8("\n"
"     QPushButton {\n"
"         background-color: rgba(90, 60, 30, 0.55);\n"
"         border: 2px solid #8B6F47;\n"
"         border-radius: 25px;\n"
"         image: url(:/assets/disconnect.png);\n"
"         padding: 6px;\n"
"     }\n"
"     QPushButton:hover {\n"
"         background-color: rgba(139, 111, 71, 0.75);\n"
"         border: 2px solid #d4a96a;\n"
"     }\n"
"     QPushButton:pressed {\n"
"         background-color: rgba(60, 35, 10, 0.85);\n"
"     }\n"
"    "));
        btn_disconnect->setFlat(true);
        btn_chat = new QPushButton(HomeFrame);
        btn_chat->setObjectName("btn_chat");
        btn_chat->setGeometry(QRect(1171, 20, 50, 50));
        btn_chat->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_chat->setStyleSheet(QString::fromUtf8("\n"
"     QPushButton {\n"
"         background-color: rgba(90, 60, 30, 0.55);\n"
"         border: 2px solid #8B6F47;\n"
"         border-radius: 25px;\n"
"         image: url(:/assets/chat_bot.svg);\n"
"         padding: 8px;\n"
"     }\n"
"     QPushButton:hover {\n"
"         background-color: rgba(139, 111, 71, 0.75);\n"
"         border: 2px solid #d4a96a;\n"
"     }\n"
"     QPushButton:pressed {\n"
"         background-color: rgba(60, 35, 10, 0.85);\n"
"     }\n"
"    "));
        btn_chat->setFlat(true);
        profile_frame = new QFrame(HomeFrame);
        profile_frame->setObjectName("profile_frame");
        profile_frame->setGeometry(QRect(900, 15, 250, 60));
        profile_frame->setStyleSheet(QString::fromUtf8("#profile_frame {\n"
"    background: rgba(50, 40, 30, 0.6);\n"
"    border: 1px solid #8B6F47;\n"
"    border-radius: 30px;\n"
"}\n"
"QLabel { background: transparent; border: none; color: white; }"));
        profileLayout = new QHBoxLayout(profile_frame);
        profileLayout->setSpacing(10);
        profileLayout->setObjectName("profileLayout");
        profileLayout->setContentsMargins(5, 5, 5, 5);
        lbl_user_avatar = new QLabel(profile_frame);
        lbl_user_avatar->setObjectName("lbl_user_avatar");
        lbl_user_avatar->setMinimumSize(QSize(50, 50));
        lbl_user_avatar->setMaximumSize(QSize(50, 50));
        lbl_user_avatar->setStyleSheet(QString::fromUtf8("border: 1.5px solid #d4a96a; border-radius: 25px; background: rgba(255,255,255,0.1);"));
        lbl_user_avatar->setScaledContents(true);

        profileLayout->addWidget(lbl_user_avatar);

        userInfoLayout = new QVBoxLayout();
        userInfoLayout->setSpacing(0);
        userInfoLayout->setObjectName("userInfoLayout");
        lbl_user_name = new QLabel(profile_frame);
        lbl_user_name->setObjectName("lbl_user_name");
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        lbl_user_name->setFont(font);

        userInfoLayout->addWidget(lbl_user_name);

        lbl_user_role = new QLabel(profile_frame);
        lbl_user_role->setObjectName("lbl_user_role");
        QFont font1;
        font1.setPointSize(8);
        lbl_user_role->setFont(font1);
        lbl_user_role->setStyleSheet(QString::fromUtf8("color: #D4AF37;"));

        userInfoLayout->addWidget(lbl_user_role);


        profileLayout->addLayout(userInfoLayout);

        btn_help = new QPushButton(HomeFrame);
        btn_help->setObjectName("btn_help");
        btn_help->setGeometry(QRect(1251, 20, 50, 50));
        btn_help->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help->setStyleSheet(QString::fromUtf8("\n"
"     QPushButton {\n"
"         background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f4e4c1, stop:1 #d4a96a);\n"
"         border: 2px solid #8B6F47;\n"
"         border-radius: 25px;\n"
"         color: #2a1e10;\n"
"         font-size: 24px;\n"
"         font-weight: bold;\n"
"     }\n"
"     QPushButton:hover {\n"
"         background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fff5e6, stop:1 #e8c578);\n"
"         border: 2px solid #a3845a;\n"
"     }\n"
"     QPushButton:pressed {\n"
"         background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d4a96a, stop:1 #8B6F47);\n"
"         color: #ffffff;\n"
"     }\n"
"    "));
        btn_help->setFlat(true);
        mainGridLayout = new QGridLayout(HomeFrame);
        mainGridLayout->setSpacing(0);
        mainGridLayout->setObjectName("mainGridLayout");
        mainGridLayout->setContentsMargins(0, 0, 0, 0);
        gs_employes = new QPushButton(HomeFrame);
        gs_employes->setObjectName("gs_employes");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(gs_employes->sizePolicy().hasHeightForWidth());
        gs_employes->setSizePolicy(sizePolicy);
        gs_employes->setMinimumSize(QSize(0, 115));
        gs_employes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_employes->setStyleSheet(QString::fromUtf8("QPushButton { background-color: transparent; border: 3px solid #d9534f; border-radius: 15px; }\n"
"QPushButton:hover { background-color: rgba(217, 83, 79, 0.2); border: 3px solid #ff6b6b; }\n"
"QPushButton:pressed { background-color: rgba(217, 83, 79, 0.3); }"));
        gs_employes->setFlat(true);

        mainGridLayout->addWidget(gs_employes, 1, 1, 1, 2);

        gs_client = new QPushButton(HomeFrame);
        gs_client->setObjectName("gs_client");
        sizePolicy.setHeightForWidth(gs_client->sizePolicy().hasHeightForWidth());
        gs_client->setSizePolicy(sizePolicy);
        gs_client->setMinimumSize(QSize(0, 115));
        gs_client->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_client->setStyleSheet(QString::fromUtf8("QPushButton { background-color: transparent; border: 3px solid #f0ad4e; border-radius: 15px; }\n"
"QPushButton:hover { background-color: rgba(240, 173, 78, 0.2); border: 3px solid #ffca7a; }\n"
"QPushButton:pressed { background-color: rgba(240, 173, 78, 0.3); }"));
        gs_client->setFlat(true);

        mainGridLayout->addWidget(gs_client, 1, 4, 1, 2);

        gs_order = new QPushButton(HomeFrame);
        gs_order->setObjectName("gs_order");
        sizePolicy.setHeightForWidth(gs_order->sizePolicy().hasHeightForWidth());
        gs_order->setSizePolicy(sizePolicy);
        gs_order->setMinimumSize(QSize(0, 115));
        gs_order->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_order->setStyleSheet(QString::fromUtf8("QPushButton { background-color: transparent; border: 3px solid #d9534f; border-radius: 15px; }\n"
"QPushButton:hover { background-color: rgba(217, 83, 79, 0.2); border: 3px solid #ff6b6b; }\n"
"QPushButton:pressed { background-color: rgba(217, 83, 79, 0.3); }"));
        gs_order->setFlat(true);

        mainGridLayout->addWidget(gs_order, 3, 1, 1, 2);

        gs_equipment = new QPushButton(HomeFrame);
        gs_equipment->setObjectName("gs_equipment");
        sizePolicy.setHeightForWidth(gs_equipment->sizePolicy().hasHeightForWidth());
        gs_equipment->setSizePolicy(sizePolicy);
        gs_equipment->setMinimumSize(QSize(0, 115));
        gs_equipment->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_equipment->setStyleSheet(QString::fromUtf8("QPushButton { background-color: transparent; border: 3px solid #5cb85c; border-radius: 15px; }\n"
"QPushButton:hover { background-color: rgba(92, 184, 92, 0.2); border: 3px solid #7ed67e; }\n"
"QPushButton:pressed { background-color: rgba(92, 184, 92, 0.3); }"));
        gs_equipment->setFlat(true);

        mainGridLayout->addWidget(gs_equipment, 3, 4, 1, 2);

        gs_fournisseur = new QPushButton(HomeFrame);
        gs_fournisseur->setObjectName("gs_fournisseur");
        sizePolicy.setHeightForWidth(gs_fournisseur->sizePolicy().hasHeightForWidth());
        gs_fournisseur->setSizePolicy(sizePolicy);
        gs_fournisseur->setMinimumSize(QSize(0, 115));
        gs_fournisseur->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_fournisseur->setStyleSheet(QString::fromUtf8("QPushButton { background-color: transparent; border: 3px solid #d9534f; border-radius: 15px; }\n"
"QPushButton:hover { background-color: rgba(217, 83, 79, 0.2); border: 3px solid #ff6b6b; }\n"
"QPushButton:pressed { background-color: rgba(217, 83, 79, 0.3); }"));
        gs_fournisseur->setFlat(true);

        mainGridLayout->addWidget(gs_fournisseur, 5, 2, 1, 3);

        topSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        mainGridLayout->addItem(topSpacer, 0, 0, 1, 7);

        bottomSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        mainGridLayout->addItem(bottomSpacer, 6, 0, 1, 7);

        leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        mainGridLayout->addItem(leftSpacer, 0, 0, 7, 1);

        rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        mainGridLayout->addItem(rightSpacer, 0, 6, 7, 1);

        mainGridLayout->setRowStretch(0, 130);
        mainGridLayout->setRowStretch(1, 115);
        mainGridLayout->setRowStretch(2, 20);
        mainGridLayout->setRowStretch(3, 115);
        mainGridLayout->setRowStretch(4, 20);
        mainGridLayout->setRowStretch(5, 115);
        mainGridLayout->setRowStretch(6, 142);
        mainGridLayout->setColumnStretch(0, 260);
        mainGridLayout->setColumnStretch(1, 130);
        mainGridLayout->setColumnStretch(2, 231);
        mainGridLayout->setColumnStretch(3, 49);
        mainGridLayout->setColumnStretch(4, 171);
        mainGridLayout->setColumnStretch(5, 200);
        mainGridLayout->setColumnStretch(6, 270);

        retranslateUi(HomeFrame);

        QMetaObject::connectSlotsByName(HomeFrame);
    } // setupUi

    void retranslateUi(QFrame *HomeFrame)
    {
        btn_settings->setText(QString());
#if QT_CONFIG(tooltip)
        btn_disconnect->setToolTip(QCoreApplication::translate("HomeFrame", "Disconnect", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_disconnect->setText(QString());
#if QT_CONFIG(tooltip)
        btn_chat->setToolTip(QCoreApplication::translate("HomeFrame", "Chat Bot", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_chat->setText(QString());
        lbl_user_avatar->setText(QString());
        lbl_user_name->setText(QCoreApplication::translate("HomeFrame", "First Name", nullptr));
        lbl_user_role->setText(QCoreApplication::translate("HomeFrame", "Job Title", nullptr));
#if QT_CONFIG(tooltip)
        btn_help->setToolTip(QCoreApplication::translate("HomeFrame", "Help", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_help->setText(QCoreApplication::translate("HomeFrame", "?", nullptr));
        gs_employes->setText(QString());
        gs_client->setText(QString());
        gs_order->setText(QString());
        gs_equipment->setText(QString());
        gs_fournisseur->setText(QString());
        (void)HomeFrame;
    } // retranslateUi

};

namespace Ui {
    class HomeFrame: public Ui_HomeFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOME_H
