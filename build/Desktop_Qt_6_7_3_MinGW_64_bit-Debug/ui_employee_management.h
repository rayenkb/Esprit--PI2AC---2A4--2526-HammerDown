/********************************************************************************
** Form generated from reading UI file 'employee_management.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EMPLOYEE_MANAGEMENT_H
#define UI_EMPLOYEE_MANAGEMENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_EmployeeManagement
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_add;
    QGroupBox *group_add;
    QLabel *label_id;
    QLabel *label_nom;
    QLabel *label_prenom;
    QLabel *label_fonction;
    QLabel *label_age;
    QLabel *label_mdp;
    QLabel *label_salaire;
    QLabel *label_email;
    QLabel *label_num;
    QLineEdit *le_id;
    QLineEdit *le_nom;
    QLineEdit *le_prenom;
    QLineEdit *le_fonction;
    QDateEdit *de_birthdate;
    QLineEdit *le_mdp;
    QDoubleSpinBox *dsb_salaire;
    QLabel *lbl_salary_insight;
    QPushButton *btn_suggest_salary;
    QLineEdit *le_email;
    QLineEdit *le_num;
    QPushButton *btn_add;
    QPushButton *btn_modify;
    QPushButton *btn_cancel;
    QLabel *lbl_avatar;
    QPushButton *btn_upload_avatar;
    QPushButton *btn_scan_face;
    QPushButton *btn_clear;
    QToolButton *btn_help_add;
    QLabel *lbl_hint_add;
    QWidget *tab_view;
    QFrame *frame_view_stats;
    QHBoxLayout *horizontalLayout_view_stats;
    QLabel *lbl_stat_total;
    QLabel *lbl_stat_avg_salary;
    QLabel *lbl_stat_avg_age;
    QSpacerItem *horizontalSpacer_view_stats;
    QLabel *label_titre_liste_emp;
    QLineEdit *le_recherche_emp;
    QPushButton *btn_refresh_emp;
    QTableView *tableView_employes;
    QPushButton *btn_delete;
    QPushButton *btn_ai_pulse;
    QLabel *lbl_ai_pulse_result;
    QWidget *tab_stats;
    QLabel *label_stats_title;
    QWidget *widget_chart_emp;
    QGridLayout *gridLayout_stats;
    QPushButton *btn_stats_ai_gen;
    QLabel *lbl_stats_ai_insight;
    QWidget *tab_history;
    QFrame *frame_history_header;
    QHBoxLayout *horizontalLayout_history_header;
    QLabel *label_titre_histo;
    QSpacerItem *horizontalSpacer_history;
    QComboBox *cb_history_filter;
    QLineEdit *le_history_search;
    QTableView *tableView_historique_emp;
    QFrame *frame_history_footer;
    QHBoxLayout *horizontalLayout_footer;
    QPushButton *btn_export_pdf;
    QSpacerItem *horizontalSpacer_footer;
    QPushButton *btn_refresh_history;
    QWidget *tab_mail;
    QGroupBox *group_mail;
    QLabel *label_mail_to;
    QLineEdit *le_mail_to;
    QLabel *label_mail_subject;
    QLineEdit *le_mail_subject;
    QTextEdit *te_mail_body;
    QPushButton *btn_send_mail;
    QComboBox *cb_mail_template;
    QPushButton *btn_return_home;

    void setupUi(QWidget *EmployeeManagement)
    {
        if (EmployeeManagement->objectName().isEmpty())
            EmployeeManagement->setObjectName("EmployeeManagement");
        EmployeeManagement->resize(1322, 800);
        EmployeeManagement->setStyleSheet(QString::fromUtf8("\n"
"    #EmployeeManagement {\n"
"        border-image: url(:/assets/background.png) 0 0 0 0 stretch stretch;\n"
"    }\n"
"    QPushButton {\n"
"        border-image: url(:/assets/button_bg.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"    }\n"
"    QTabWidget::pane {\n"
"        border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch;\n"
"    }\n"
"\n"
"QLineEdit {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"    border: 2px solid #8B6F47;\n"
"    border-radius: 8px;\n"
"    padding: 3px 12px;\n"
"    font-size: 14px;\n"
"    color: #333;\n"
"    selection-background-color: #8B6F47;\n"
"    selection-color: white;\n"
"}\n"
"QLineEdit:hover {\n"
"    border: 2px solid #A0825A;\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"}\n"
"QLineEdit:focus {\n"
"    border: 2px solid #8B4513; \n"
"    background: #FFFAF0;\n"
"}\n"
"\n"
"   "));
        tabWidget = new QTabWidget(EmployeeManagement);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(110, 70, 1051, 681));
        QFont font;
        font.setFamilies({QString::fromUtf8("Gadugi")});
        font.setPointSize(10);
        font.setBold(true);
        tabWidget->setFont(font);
        tabWidget->setStyleSheet(QString::fromUtf8("\n"
"      QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"      QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"      QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"      QWidget#tab_add, QWidget#tab_view, QWidget#tab_delete, QWidget#tab_modify, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_calendar { background: transparent; }\n"
"     "));
        tab_add = new QWidget();
        tab_add->setObjectName("tab_add");
        group_add = new QGroupBox(tab_add);
        group_add->setObjectName("group_add");
        group_add->setGeometry(QRect(20, 20, 1321, 721));
        group_add->setStyleSheet(QString::fromUtf8("\n"
"        QGroupBox { background-color: rgba(255, 255, 255, 0.03); border: 2px solid rgba(212, 175, 55, 0.15); border-radius: 25px; margin-top: 30px; }\n"
"        QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 6px 30px; background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #A0825A); font-weight: 800; color: white; border-radius: 12px; font-size: 15px; text-transform: uppercase; letter-spacing: 2px; }\n"
"        QLineEdit {\n"
"            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"            border: 2px solid #8B6F47;\n"
"            border-radius: 8px;\n"
"            padding: 3px 12px;\n"
"            font-size: 14px;\n"
"            color: #333;\n"
"            selection-background-color: #8B6F47;\n"
"            selection-color: white;\n"
"        }\n"
"        QLineEdit:hover {\n"
"            border: 2px solid #A0825A;\n"
"            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                        "stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"        }\n"
"        QLineEdit:focus {\n"
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"       "));
        label_id = new QLabel(group_add);
        label_id->setObjectName("label_id");
        label_id->setGeometry(QRect(200, 60, 150, 30));
        label_id->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_nom = new QLabel(group_add);
        label_nom->setObjectName("label_nom");
        label_nom->setGeometry(QRect(200, 110, 150, 30));
        label_nom->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_prenom = new QLabel(group_add);
        label_prenom->setObjectName("label_prenom");
        label_prenom->setGeometry(QRect(200, 160, 150, 30));
        label_prenom->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_fonction = new QLabel(group_add);
        label_fonction->setObjectName("label_fonction");
        label_fonction->setGeometry(QRect(200, 210, 150, 30));
        label_fonction->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_fonction->setAlignment(Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        label_age = new QLabel(group_add);
        label_age->setObjectName("label_age");
        label_age->setGeometry(QRect(200, 260, 150, 30));
        label_age->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_mdp = new QLabel(group_add);
        label_mdp->setObjectName("label_mdp");
        label_mdp->setGeometry(QRect(200, 310, 150, 30));
        label_mdp->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_salaire = new QLabel(group_add);
        label_salaire->setObjectName("label_salaire");
        label_salaire->setGeometry(QRect(200, 360, 150, 30));
        label_salaire->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_email = new QLabel(group_add);
        label_email->setObjectName("label_email");
        label_email->setGeometry(QRect(200, 410, 150, 30));
        label_email->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_num = new QLabel(group_add);
        label_num->setObjectName("label_num");
        label_num->setGeometry(QRect(200, 460, 150, 30));
        label_num->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        le_id = new QLineEdit(group_add);
        le_id->setObjectName("le_id");
        le_id->setGeometry(QRect(400, 60, 250, 30));
        le_nom = new QLineEdit(group_add);
        le_nom->setObjectName("le_nom");
        le_nom->setGeometry(QRect(400, 110, 250, 30));
        le_prenom = new QLineEdit(group_add);
        le_prenom->setObjectName("le_prenom");
        le_prenom->setGeometry(QRect(400, 160, 250, 30));
        le_fonction = new QLineEdit(group_add);
        le_fonction->setObjectName("le_fonction");
        le_fonction->setGeometry(QRect(400, 210, 250, 30));
        de_birthdate = new QDateEdit(group_add);
        de_birthdate->setObjectName("de_birthdate");
        de_birthdate->setGeometry(QRect(400, 260, 250, 30));
        de_birthdate->setStyleSheet(QString::fromUtf8("QDateEdit {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"    border: 2px solid #8B6F47;\n"
"    border-radius: 8px;\n"
"    padding: 3px 12px;\n"
"    font-size: 14px;\n"
"    color: #333;\n"
"}\n"
"QDateEdit:hover { border: 2px solid #A0825A; }\n"
"QDateEdit:focus { border: 2px solid #8B4513; background: #FFFAF0; }\n"
""));
        de_birthdate->setCalendarPopup(true);
        le_mdp = new QLineEdit(group_add);
        le_mdp->setObjectName("le_mdp");
        le_mdp->setGeometry(QRect(400, 310, 250, 30));
        dsb_salaire = new QDoubleSpinBox(group_add);
        dsb_salaire->setObjectName("dsb_salaire");
        dsb_salaire->setGeometry(QRect(400, 360, 150, 30));
        dsb_salaire->setStyleSheet(QString::fromUtf8("QLineEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QTextEdit {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"    border: 2px solid #8B6F47;\n"
"    border-radius: 8px;\n"
"    padding: 3px 12px;\n"
"    font-size: 14px;\n"
"    color: #333;\n"
"    selection-background-color: #8B6F47;\n"
"    selection-color: white;\n"
"}\n"
"QSpinBox:hover, QDoubleSpinBox:hover {\n"
"    border: 2px solid #A0825A;\n"
"}\n"
""));
        dsb_salaire->setMaximum(9999.989999999999782);
        lbl_salary_insight = new QLabel(group_add);
        lbl_salary_insight->setObjectName("lbl_salary_insight");
        lbl_salary_insight->setGeometry(QRect(560, 360, 140, 30));
        lbl_salary_insight->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-size: 11px; font-weight: bold; background: transparent;"));
        btn_suggest_salary = new QPushButton(group_add);
        btn_suggest_salary->setObjectName("btn_suggest_salary");
        btn_suggest_salary->setGeometry(QRect(710, 365, 20, 20));
        btn_suggest_salary->setStyleSheet(QString::fromUtf8("QPushButton { background: #8B6F47; color: white; border-radius: 10px; font-size: 12px; font-weight: bold; border: none; }\n"
"QPushButton:hover { background: #A0825A; }"));
        le_email = new QLineEdit(group_add);
        le_email->setObjectName("le_email");
        le_email->setGeometry(QRect(400, 410, 250, 30));
        le_num = new QLineEdit(group_add);
        le_num->setObjectName("le_num");
        le_num->setGeometry(QRect(400, 460, 250, 30));
        btn_add = new QPushButton(group_add);
        btn_add->setObjectName("btn_add");
        btn_add->setGeometry(QRect(750, 310, 150, 40));
        btn_add->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"             "));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/add.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_add->setIcon(icon);
        btn_add->setIconSize(QSize(24, 24));
        btn_modify = new QPushButton(group_add);
        btn_modify->setObjectName("btn_modify");
        btn_modify->setGeometry(QRect(750, 380, 150, 40));
        btn_modify->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"             "));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/assets/modify.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_modify->setIcon(icon1);
        btn_modify->setIconSize(QSize(24, 24));
        btn_cancel = new QPushButton(group_add);
        btn_cancel->setObjectName("btn_cancel");
        btn_cancel->setGeometry(QRect(750, 450, 150, 40));
        btn_cancel->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        lbl_avatar = new QLabel(group_add);
        lbl_avatar->setObjectName("lbl_avatar");
        lbl_avatar->setGeometry(QRect(750, 30, 150, 150));
        lbl_avatar->setStyleSheet(QString::fromUtf8("border: 2px dashed #8B6F47; border-radius: 75px; background: rgba(255, 255, 255, 0.1); color: white;"));
        lbl_avatar->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_upload_avatar = new QPushButton(group_add);
        btn_upload_avatar->setObjectName("btn_upload_avatar");
        btn_upload_avatar->setGeometry(QRect(750, 190, 150, 40));
        btn_upload_avatar->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        btn_scan_face = new QPushButton(group_add);
        btn_scan_face->setObjectName("btn_scan_face");
        btn_scan_face->setGeometry(QRect(750, 240, 150, 40));
        btn_scan_face->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/assets/chat_bot.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_scan_face->setIcon(icon2);
        btn_scan_face->setIconSize(QSize(24, 24));
        btn_clear = new QPushButton(group_add);
        btn_clear->setObjectName("btn_clear");
        btn_clear->setGeometry(QRect(750, 520, 150, 40));
        btn_clear->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/assets/clear.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_clear->setIcon(icon3);
        btn_clear->setIconSize(QSize(24, 24));
        btn_help_add = new QToolButton(group_add);
        btn_help_add->setObjectName("btn_help_add");
        btn_help_add->setGeometry(QRect(990, 20, 30, 30));
        btn_help_add->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"        QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"));
        btn_help_add->setCheckable(true);
        lbl_hint_add = new QLabel(group_add);
        lbl_hint_add->setObjectName("lbl_hint_add");
        lbl_hint_add->setGeometry(QRect(200, 520, 800, 60));
        lbl_hint_add->setVisible(false);
        lbl_hint_add->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        tabWidget->addTab(tab_add, QString());
        tab_view = new QWidget();
        tab_view->setObjectName("tab_view");
        frame_view_stats = new QFrame(tab_view);
        frame_view_stats->setObjectName("frame_view_stats");
        frame_view_stats->setGeometry(QRect(20, 600, 800, 50));
        frame_view_stats->setStyleSheet(QString::fromUtf8("QFrame#frame_view_stats { background: rgba(139, 111, 71, 0.2); border: 2px solid #8B6F47; border-radius: 12px; }"));
        horizontalLayout_view_stats = new QHBoxLayout(frame_view_stats);
        horizontalLayout_view_stats->setObjectName("horizontalLayout_view_stats");
        horizontalLayout_view_stats->setContentsMargins(15, 5, 15, 5);
        lbl_stat_total = new QLabel(frame_view_stats);
        lbl_stat_total->setObjectName("lbl_stat_total");
        lbl_stat_total->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold; font-size: 14px; text-shadow: 1px 1px 2px black;"));

        horizontalLayout_view_stats->addWidget(lbl_stat_total);

        lbl_stat_avg_salary = new QLabel(frame_view_stats);
        lbl_stat_avg_salary->setObjectName("lbl_stat_avg_salary");
        lbl_stat_avg_salary->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold; font-size: 14px;"));

        horizontalLayout_view_stats->addWidget(lbl_stat_avg_salary);

        lbl_stat_avg_age = new QLabel(frame_view_stats);
        lbl_stat_avg_age->setObjectName("lbl_stat_avg_age");
        lbl_stat_avg_age->setStyleSheet(QString::fromUtf8("color: #B8925A; font-weight: bold; font-size: 14px;"));

        horizontalLayout_view_stats->addWidget(lbl_stat_avg_age);

        horizontalSpacer_view_stats = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_view_stats->addItem(horizontalSpacer_view_stats);

        label_titre_liste_emp = new QLabel(tab_view);
        label_titre_liste_emp->setObjectName("label_titre_liste_emp");
        label_titre_liste_emp->setGeometry(QRect(720, 70, 300, 30));
        label_titre_liste_emp->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;"));
        label_titre_liste_emp->setAlignment(Qt::AlignmentFlag::AlignCenter);
        le_recherche_emp = new QLineEdit(tab_view);
        le_recherche_emp->setObjectName("le_recherche_emp");
        le_recherche_emp->setGeometry(QRect(20, 70, 250, 30));
        btn_refresh_emp = new QPushButton(tab_view);
        btn_refresh_emp->setObjectName("btn_refresh_emp");
        btn_refresh_emp->setGeometry(QRect(280, 65, 110, 40));
        tableView_employes = new QTableView(tab_view);
        tableView_employes->setObjectName("tableView_employes");
        tableView_employes->setGeometry(QRect(20, 120, 1011, 470));
        tableView_employes->setStyleSheet(QString::fromUtf8("\n"
"        QHeaderView::section { background-color: #8B6F47; color: white; border: none; padding: 4px; }\n"
"        QTableView { border: 1px solid #8B6F47; background-color: white; color: #333333; selection-background-color: #E0E0E0; selection-color: black; }\n"
"      "));
        btn_delete = new QPushButton(tab_view);
        btn_delete->setObjectName("btn_delete");
        btn_delete->setGeometry(QRect(850, 605, 150, 40));
        btn_delete->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #A31D1D; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #D32F2F; }"));
        btn_ai_pulse = new QPushButton(tab_view);
        btn_ai_pulse->setObjectName("btn_ai_pulse");
        btn_ai_pulse->setGeometry(QRect(420, 65, 250, 40));
        btn_ai_pulse->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #4CAF50; border-radius: 20px; color: white; font-weight: bold; font-size: 13px; border: 2px solid #D4AF37; }\n"
"QPushButton:hover { background-color: #66BB6A; border: 2px solid #FFF; box-shadow: 0px 0px 15px rgba(76, 175, 80, 0.8); }"));
        lbl_ai_pulse_result = new QLabel(tab_view);
        lbl_ai_pulse_result->setObjectName("lbl_ai_pulse_result");
        lbl_ai_pulse_result->setGeometry(QRect(420, 120, 590, 90));
        lbl_ai_pulse_result->setStyleSheet(QString::fromUtf8("color: #FFF; font-size: 13px; font-weight: bold; background: rgba(0, 0, 0, 0.85); padding: 15px; border-radius: 15px; border: 2px solid #D4AF37;"));
        lbl_ai_pulse_result->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
        lbl_ai_pulse_result->setWordWrap(true);
        lbl_ai_pulse_result->setVisible(false);
        tabWidget->addTab(tab_view, QString());
        tab_stats = new QWidget();
        tab_stats->setObjectName("tab_stats");
        label_stats_title = new QLabel(tab_stats);
        label_stats_title->setObjectName("label_stats_title");
        label_stats_title->setGeometry(QRect(720, 30, 300, 30));
        label_stats_title->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;"));
        label_stats_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        widget_chart_emp = new QWidget(tab_stats);
        widget_chart_emp->setObjectName("widget_chart_emp");
        widget_chart_emp->setGeometry(QRect(20, 150, 1011, 501));
        gridLayout_stats = new QGridLayout(widget_chart_emp);
        gridLayout_stats->setObjectName("gridLayout_stats");
        btn_stats_ai_gen = new QPushButton(tab_stats);
        btn_stats_ai_gen->setObjectName("btn_stats_ai_gen");
        btn_stats_ai_gen->setGeometry(QRect(20, 70, 220, 50));
        btn_stats_ai_gen->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #5D3FD3; border-radius: 25px; color: white; font-weight: bold; font-size: 14px; border: 2px solid #D4AF37; }\n"
"QPushButton:hover { background-color: #7B68EE; border-color: #FFF; }"));
        lbl_stats_ai_insight = new QLabel(tab_stats);
        lbl_stats_ai_insight->setObjectName("lbl_stats_ai_insight");
        lbl_stats_ai_insight->setGeometry(QRect(260, 70, 771, 70));
        lbl_stats_ai_insight->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-size: 13px; font-style: italic; background: rgba(93, 63, 211, 0.1); padding: 10px; border-radius: 12px; border: 1px solid rgba(212, 175, 55, 0.3);"));
        lbl_stats_ai_insight->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
        lbl_stats_ai_insight->setWordWrap(true);
        tabWidget->addTab(tab_stats, QString());
        tab_history = new QWidget();
        tab_history->setObjectName("tab_history");
        frame_history_header = new QFrame(tab_history);
        frame_history_header->setObjectName("frame_history_header");
        frame_history_header->setGeometry(QRect(20, 15, 1011, 80));
        frame_history_header->setStyleSheet(QString::fromUtf8("QFrame#frame_history_header {\n"
"    background: rgba(44, 34, 21, 0.9);\n"
"    border: 1px solid rgba(212, 175, 55, 0.4);\n"
"    border-radius: 12px;\n"
"}"));
        horizontalLayout_history_header = new QHBoxLayout(frame_history_header);
        horizontalLayout_history_header->setObjectName("horizontalLayout_history_header");
        horizontalLayout_history_header->setContentsMargins(20, -1, 20, -1);
        label_titre_histo = new QLabel(frame_history_header);
        label_titre_histo->setObjectName("label_titre_histo");
        label_titre_histo->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: white; font-family: 'Gadugi'; text-shadow: 1px 1px 2px rgba(0,0,0,0.5);"));

        horizontalLayout_history_header->addWidget(label_titre_histo);

        horizontalSpacer_history = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_history_header->addItem(horizontalSpacer_history);

        cb_history_filter = new QComboBox(frame_history_header);
        cb_history_filter->addItem(QString());
        cb_history_filter->setObjectName("cb_history_filter");
        cb_history_filter->setMinimumSize(QSize(160, 36));

        horizontalLayout_history_header->addWidget(cb_history_filter);

        le_history_search = new QLineEdit(frame_history_header);
        le_history_search->setObjectName("le_history_search");
        le_history_search->setMinimumSize(QSize(220, 36));

        horizontalLayout_history_header->addWidget(le_history_search);

        tableView_historique_emp = new QTableView(tab_history);
        tableView_historique_emp->setObjectName("tableView_historique_emp");
        tableView_historique_emp->setGeometry(QRect(20, 110, 1011, 481));
        tableView_historique_emp->setStyleSheet(QString::fromUtf8("\n"
"        QTableView { border: 2px solid #8B6F47; background: rgba(255, 255, 255, 0.95); gridline-color: #D4A96A; font-family: 'Segoe UI'; font-size: 13px; border-radius: 8px; }\n"
"        QHeaderView::section { background: #8B6F47; color: white; padding: 10px; border: none; font-weight: bold; }\n"
"        QTableView::item:selected { background: #D4AF37; color: white; }\n"
"      "));
        frame_history_footer = new QFrame(tab_history);
        frame_history_footer->setObjectName("frame_history_footer");
        frame_history_footer->setGeometry(QRect(20, 605, 1011, 60));
        frame_history_footer->setStyleSheet(QString::fromUtf8("QFrame#frame_history_footer { background: rgba(18, 14, 10, 0.95); border-top: 2px solid rgba(212, 175, 55, 0.3); border-bottom-left-radius: 20px; border-bottom-right-radius: 20px; }"));
        horizontalLayout_footer = new QHBoxLayout(frame_history_footer);
        horizontalLayout_footer->setObjectName("horizontalLayout_footer");
        btn_export_pdf = new QPushButton(frame_history_footer);
        btn_export_pdf->setObjectName("btn_export_pdf");
        btn_export_pdf->setMinimumSize(QSize(160, 40));
        btn_export_pdf->setStyleSheet(QString::fromUtf8("QPushButton { background: #A31D1D; color: white; border-radius: 20px; font-weight: bold; font-size: 13px; } QPushButton:hover { background: #D32F2F; }"));

        horizontalLayout_footer->addWidget(btn_export_pdf);

        horizontalSpacer_footer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_footer->addItem(horizontalSpacer_footer);

        btn_refresh_history = new QPushButton(frame_history_footer);
        btn_refresh_history->setObjectName("btn_refresh_history");
        btn_refresh_history->setMinimumSize(QSize(180, 40));
        btn_refresh_history->setStyleSheet(QString::fromUtf8("QPushButton { background: #8B6F47; color: white; border-radius: 20px; font-weight: bold; font-size: 13px; } QPushButton:hover { background: #A0825A; }"));

        horizontalLayout_footer->addWidget(btn_refresh_history);

        tabWidget->addTab(tab_history, QString());
        tab_mail = new QWidget();
        tab_mail->setObjectName("tab_mail");
        group_mail = new QGroupBox(tab_mail);
        group_mail->setObjectName("group_mail");
        group_mail->setGeometry(QRect(20, 20, 1011, 631));
        label_mail_to = new QLabel(group_mail);
        label_mail_to->setObjectName("label_mail_to");
        label_mail_to->setGeometry(QRect(50, 50, 100, 30));
        le_mail_to = new QLineEdit(group_mail);
        le_mail_to->setObjectName("le_mail_to");
        le_mail_to->setGeometry(QRect(150, 50, 400, 30));
        le_mail_to->setReadOnly(true);
        label_mail_subject = new QLabel(group_mail);
        label_mail_subject->setObjectName("label_mail_subject");
        label_mail_subject->setGeometry(QRect(50, 100, 100, 30));
        le_mail_subject = new QLineEdit(group_mail);
        le_mail_subject->setObjectName("le_mail_subject");
        le_mail_subject->setGeometry(QRect(150, 100, 800, 30));
        te_mail_body = new QTextEdit(group_mail);
        te_mail_body->setObjectName("te_mail_body");
        te_mail_body->setGeometry(QRect(50, 150, 900, 350));
        btn_send_mail = new QPushButton(group_mail);
        btn_send_mail->setObjectName("btn_send_mail");
        btn_send_mail->setGeometry(QRect(800, 550, 150, 40));
        cb_mail_template = new QComboBox(group_mail);
        cb_mail_template->addItem(QString());
        cb_mail_template->addItem(QString());
        cb_mail_template->addItem(QString());
        cb_mail_template->addItem(QString());
        cb_mail_template->setObjectName("cb_mail_template");
        cb_mail_template->setGeometry(QRect(50, 550, 250, 30));
        tabWidget->addTab(tab_mail, QString());
        btn_return_home = new QPushButton(EmployeeManagement);
        btn_return_home->setObjectName("btn_return_home");
        btn_return_home->setGeometry(QRect(1250, 10, 50, 50));
        btn_return_home->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_return_home->setStyleSheet(QString::fromUtf8("\n"
"    #btn_return_home {\n"
"        border-image: url(:/assets/return.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"        background-color: transparent;\n"
"    }\n"
"    #btn_return_home:hover {\n"
"         background-color: rgba(255, 255, 255, 0.2);\n"
"         border-radius: 25px;\n"
"    }\n"
"   "));

        retranslateUi(EmployeeManagement);
        QObject::connect(btn_help_add, &QToolButton::toggled, lbl_hint_add, &QLabel::setVisible);

        tabWidget->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(EmployeeManagement);
    } // setupUi

    void retranslateUi(QWidget *EmployeeManagement)
    {
        EmployeeManagement->setWindowTitle(QCoreApplication::translate("EmployeeManagement", "Employee Management", nullptr));
        group_add->setTitle(QCoreApplication::translate("EmployeeManagement", "Add Employee", nullptr));
        label_id->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\206\224 Employee ID:", nullptr));
        label_nom->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\221\244 Last Name:", nullptr));
        label_prenom->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\221\244 First Name:", nullptr));
        label_fonction->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\222\274 Job Title:", nullptr));
        label_age->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\223\205 Birth Date:", nullptr));
        label_mdp->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\224\222 Password:", nullptr));
        label_salaire->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\222\260 Salary:", nullptr));
        label_email->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\223\247 Email Address:", nullptr));
        label_num->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\223\236 Number:", nullptr));
        lbl_salary_insight->setText(QCoreApplication::translate("EmployeeManagement", "Market Avg: --", nullptr));
#if QT_CONFIG(tooltip)
        btn_suggest_salary->setToolTip(QCoreApplication::translate("EmployeeManagement", "Suggest Market Salary", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_suggest_salary->setText(QCoreApplication::translate("EmployeeManagement", "\342\230\205", nullptr));
        btn_add->setText(QCoreApplication::translate("EmployeeManagement", "Add", nullptr));
        btn_modify->setText(QCoreApplication::translate("EmployeeManagement", "Modify", nullptr));
        btn_cancel->setText(QCoreApplication::translate("EmployeeManagement", "Cancel", nullptr));
        lbl_avatar->setText(QCoreApplication::translate("EmployeeManagement", "No Avatar", nullptr));
        btn_upload_avatar->setText(QCoreApplication::translate("EmployeeManagement", "Upload Avatar", nullptr));
        btn_scan_face->setText(QCoreApplication::translate("EmployeeManagement", "Scan Face ID", nullptr));
        btn_clear->setText(QCoreApplication::translate("EmployeeManagement", "Clear Fields", nullptr));
        btn_help_add->setText(QCoreApplication::translate("EmployeeManagement", "?", nullptr));
        lbl_hint_add->setText(QCoreApplication::translate("EmployeeManagement", "Quick Info: Use this form to add or modify employee records. Passwords and Salary are sensitive data. Job Title helps in team assignment.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_add), QCoreApplication::translate("EmployeeManagement", "Add Employee", nullptr));
        lbl_stat_total->setText(QCoreApplication::translate("EmployeeManagement", "Total Personnel: --", nullptr));
        lbl_stat_avg_salary->setText(QCoreApplication::translate("EmployeeManagement", "Avg Salary: --", nullptr));
        lbl_stat_avg_age->setText(QCoreApplication::translate("EmployeeManagement", "Avg Age: --", nullptr));
        label_titre_liste_emp->setText(QCoreApplication::translate("EmployeeManagement", "Employee List", nullptr));
        le_recherche_emp->setPlaceholderText(QCoreApplication::translate("EmployeeManagement", "Search employees...", nullptr));
        btn_refresh_emp->setText(QCoreApplication::translate("EmployeeManagement", "Refresh", nullptr));
        btn_delete->setText(QCoreApplication::translate("EmployeeManagement", "Delete Selected", nullptr));
        btn_ai_pulse->setText(QCoreApplication::translate("EmployeeManagement", "\342\232\241 Company AI Pulse Tracker", nullptr));
        lbl_ai_pulse_result->setText(QCoreApplication::translate("EmployeeManagement", "AI Pulse Initialization...", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_view), QCoreApplication::translate("EmployeeManagement", "View Employees", nullptr));
        label_stats_title->setText(QCoreApplication::translate("EmployeeManagement", "Employee Statistics", nullptr));
        btn_stats_ai_gen->setText(QCoreApplication::translate("EmployeeManagement", "\342\234\250 Generate AI Insights", nullptr));
        lbl_stats_ai_insight->setText(QCoreApplication::translate("EmployeeManagement", "Your automated workforce analysis will appear here. Click the button to get started.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_stats), QCoreApplication::translate("EmployeeManagement", "Statistics", nullptr));
        label_titre_histo->setText(QCoreApplication::translate("EmployeeManagement", "TIMELINE & STAFF DIRECTORY", nullptr));
        cb_history_filter->setItemText(0, QCoreApplication::translate("EmployeeManagement", "All Personnel", nullptr));

        le_history_search->setPlaceholderText(QCoreApplication::translate("EmployeeManagement", "Search across records...", nullptr));
        btn_export_pdf->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\223\204 Export Audit Log", nullptr));
        btn_refresh_history->setText(QCoreApplication::translate("EmployeeManagement", "\360\237\224\204 Synchronize Timeline", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_history), QCoreApplication::translate("EmployeeManagement", "\360\237\223\213 Personnel Records", nullptr));
        group_mail->setTitle(QCoreApplication::translate("EmployeeManagement", "Send Email to Employee", nullptr));
        label_mail_to->setText(QCoreApplication::translate("EmployeeManagement", "Recipient:", nullptr));
        label_mail_subject->setText(QCoreApplication::translate("EmployeeManagement", "Subject:", nullptr));
        btn_send_mail->setText(QCoreApplication::translate("EmployeeManagement", "Send Email", nullptr));
        cb_mail_template->setItemText(0, QCoreApplication::translate("EmployeeManagement", "Select Template...", nullptr));
        cb_mail_template->setItemText(1, QCoreApplication::translate("EmployeeManagement", "Welcome Email", nullptr));
        cb_mail_template->setItemText(2, QCoreApplication::translate("EmployeeManagement", "Task Assignment", nullptr));
        cb_mail_template->setItemText(3, QCoreApplication::translate("EmployeeManagement", "Meeting Request", nullptr));

        tabWidget->setTabText(tabWidget->indexOf(tab_mail), QCoreApplication::translate("EmployeeManagement", "Contact", nullptr));
        btn_return_home->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class EmployeeManagement: public Ui_EmployeeManagement {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EMPLOYEE_MANAGEMENT_H
