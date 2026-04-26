/********************************************************************************
** Form generated from reading UI file 'client_management.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENT_MANAGEMENT_H
#define UI_CLIENT_MANAGEMENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ClientManagement
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_add;
    QGroupBox *group_add;
    QLabel *label_nom;
    QLabel *label_prenom;
    QLabel *label_adresse;
    QLabel *label_tel;
    QLabel *label_email;
    QLabel *label_sexe;
    QLineEdit *le_nom;
    QLineEdit *le_prenom;
    QLineEdit *le_adresse;
    QLineEdit *le_tel;
    QLineEdit *le_email;
    QRadioButton *rb_homme;
    QRadioButton *rb_femme;
    QPushButton *btn_add;
    QPushButton *btn_clear;
    QToolButton *btn_help_add;
    QLabel *lbl_hint_add;
    QWidget *tab_view;
    QLabel *label_titre_liste;
    QPushButton *btn_search;
    QLineEdit *le_recherche;
    QLabel *label_sort;
    QComboBox *cb_sort_field;
    QPushButton *btn_sort_order;
    QTableView *tableView;
    QPushButton *btn_pdf;
    QPushButton *btn_delete;
    QPushButton *btn_edit_view;
    QWidget *tab_modify;
    QGroupBox *group_modify;
    QLabel *label_id_mod;
    QLabel *label_nom_mod;
    QLabel *label_prenom_mod;
    QLabel *label_adresse_mod;
    QLabel *label_tel_mod;
    QLabel *label_email_mod;
    QLabel *label_sexe_mod;
    QLineEdit *le_id_mod;
    QLineEdit *le_nom_mod;
    QLineEdit *le_prenom_mod;
    QLineEdit *le_adresse_mod;
    QLineEdit *le_tel_mod;
    QLineEdit *le_email_mod;
    QRadioButton *rb_homme_mod;
    QRadioButton *rb_femme_mod;
    QPushButton *btn_modify;
    QPushButton *btn_clear_mod;
    QWidget *tab_stats;
    QLabel *label_stat_title;
    QFrame *widget_chart;
    QWidget *tab_mail;
    QLabel *label_mail_title;
    QLabel *l_smtp;
    QLineEdit *le_smtp;
    QLabel *l_port;
    QLineEdit *le_port;
    QLabel *l_user;
    QLineEdit *le_user;
    QLabel *l_pass;
    QLineEdit *le_pass;
    QLabel *l_to;
    QLineEdit *le_to;
    QLabel *l_subj;
    QLineEdit *le_subject;
    QLabel *l_att;
    QLineEdit *le_attachment;
    QPushButton *btn_browse;
    QLabel *l_msg;
    QTextEdit *te_message;
    QPushButton *btn_send;
    QWidget *tab_calendar;
    QLabel *label_calendar_title;
    QCalendarWidget *calendarWidget;
    QWidget *tab_cyber_trace;
    QTableView *tableView_cyber;
    QPushButton *btn_refresh_trace;
    QWidget *tab_data_matrix;
    QFrame *frame_matrix;
    QLabel *label_matrix_title;
    QPushButton *btn_return_home;
    QButtonGroup *bg_client_gender_add;
    QButtonGroup *bg_client_gender_mod;

    void setupUi(QWidget *ClientManagement)
    {
        if (ClientManagement->objectName().isEmpty())
            ClientManagement->setObjectName("ClientManagement");
        ClientManagement->resize(1322, 800);
        ClientManagement->setStyleSheet(QString::fromUtf8("\n"
"    #ClientManagement {\n"
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
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0,0,0,0.6), stop:1 rgba(20,15,10,0.8));\n"
"    border: 1px solid #8B6F47;\n"
"    border-left: 3px solid #D4AF37;\n"
"    border-radius: 0px;\n"
"    padding: 3px 12px;\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    font-family: 'Consolas';\n"
"    color: #D4AF37;\n"
"    selection-background-color: #8B6F47;\n"
"    selection-color: white;\n"
"}\n"
"QLineEdit:hover {\n"
"    border: 1px solid #D4AF37;\n"
"    border-left: 3px solid #FFD700;\n"
"    background: rgba(139, 111, 71, 0.2);\n"
"}\n"
"QLineEdit:focus {\n"
"    border: 1px solid #FFD700;"
                        "\n"
"    border-left: 4px solid #FFD700;\n"
"    background: rgba(40, 30, 20, 0.9);\n"
"}\n"
"\n"
"   "));
        tabWidget = new QTabWidget(ClientManagement);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(19, 20, 1241, 760));
        QFont font;
        font.setFamilies({QString::fromUtf8("Gadugi")});
        font.setPointSize(10);
        font.setBold(true);
        tabWidget->setFont(font);
        tabWidget->setStyleSheet(QString::fromUtf8("\n"
"      QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"      QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"      QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"      QWidget#tab_add, QWidget#tab_view, QWidget#tab_delete, QWidget#tab_modify, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_calendar, QWidget#tab_mail_search { background: transparent; }\n"
"     "));
        tab_add = new QWidget();
        tab_add->setObjectName("tab_add");
        group_add = new QGroupBox(tab_add);
        group_add->setObjectName("group_add");
        group_add->setGeometry(QRect(20, 20, 1321, 721));
        group_add->setStyleSheet(QString::fromUtf8("\n"
"        QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"        QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: transparent; border-bottom: 2px solid #8B6F47; font-family: 'Consolas'; color: #D4AF37; font-weight: bold; font-size: 16px; border-radius: 0px; }\n"
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
"            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"        }\n"
"        QLineEdit:focus {\n"
" "
                        "           border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"       "));
        label_nom = new QLabel(group_add);
        label_nom->setObjectName("label_nom");
        label_nom->setGeometry(QRect(200, 130, 150, 30));
        label_nom->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_prenom = new QLabel(group_add);
        label_prenom->setObjectName("label_prenom");
        label_prenom->setGeometry(QRect(200, 200, 150, 30));
        label_prenom->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_adresse = new QLabel(group_add);
        label_adresse->setObjectName("label_adresse");
        label_adresse->setGeometry(QRect(200, 270, 150, 30));
        label_adresse->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_tel = new QLabel(group_add);
        label_tel->setObjectName("label_tel");
        label_tel->setGeometry(QRect(200, 340, 150, 30));
        label_tel->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_email = new QLabel(group_add);
        label_email->setObjectName("label_email");
        label_email->setGeometry(QRect(200, 410, 150, 30));
        label_email->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_sexe = new QLabel(group_add);
        label_sexe->setObjectName("label_sexe");
        label_sexe->setGeometry(QRect(200, 480, 150, 30));
        label_sexe->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        le_nom = new QLineEdit(group_add);
        le_nom->setObjectName("le_nom");
        le_nom->setGeometry(QRect(400, 130, 250, 30));
        le_nom->setStyleSheet(QString::fromUtf8(""));
        le_prenom = new QLineEdit(group_add);
        le_prenom->setObjectName("le_prenom");
        le_prenom->setGeometry(QRect(400, 200, 250, 30));
        le_prenom->setStyleSheet(QString::fromUtf8(""));
        le_adresse = new QLineEdit(group_add);
        le_adresse->setObjectName("le_adresse");
        le_adresse->setGeometry(QRect(400, 270, 250, 30));
        le_adresse->setStyleSheet(QString::fromUtf8(""));
        le_tel = new QLineEdit(group_add);
        le_tel->setObjectName("le_tel");
        le_tel->setGeometry(QRect(400, 340, 250, 30));
        le_tel->setStyleSheet(QString::fromUtf8(""));
        le_email = new QLineEdit(group_add);
        le_email->setObjectName("le_email");
        le_email->setGeometry(QRect(400, 410, 250, 30));
        le_email->setStyleSheet(QString::fromUtf8(""));
        rb_homme = new QRadioButton(group_add);
        bg_client_gender_add = new QButtonGroup(ClientManagement);
        bg_client_gender_add->setObjectName("bg_client_gender_add");
        bg_client_gender_add->addButton(rb_homme);
        rb_homme->setObjectName("rb_homme");
        rb_homme->setGeometry(QRect(400, 480, 100, 30));
        rb_femme = new QRadioButton(group_add);
        bg_client_gender_add->addButton(rb_femme);
        rb_femme->setObjectName("rb_femme");
        rb_femme->setGeometry(QRect(510, 480, 100, 30));
        btn_add = new QPushButton(group_add);
        btn_add->setObjectName("btn_add");
        btn_add->setGeometry(QRect(870, 200, 150, 40));
        btn_add->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"             "));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/add.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_add->setIcon(icon);
        btn_add->setIconSize(QSize(24, 24));
        btn_clear = new QPushButton(group_add);
        btn_clear->setObjectName("btn_clear");
        btn_clear->setGeometry(QRect(750, 410, 150, 40));
        btn_clear->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/assets/clear.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_clear->setIcon(icon1);
        btn_clear->setIconSize(QSize(24, 24));
        btn_help_add = new QToolButton(group_add);
        btn_help_add->setObjectName("btn_help_add");
        btn_help_add->setGeometry(QRect(1180, 20, 30, 30));
        btn_help_add->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"            QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"));
        btn_help_add->setCheckable(true);
        lbl_hint_add = new QLabel(group_add);
        lbl_hint_add->setObjectName("lbl_hint_add");
        lbl_hint_add->setGeometry(QRect(200, 550, 800, 60));
        lbl_hint_add->setVisible(false);
        lbl_hint_add->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        tabWidget->addTab(tab_add, QString());
        tab_view = new QWidget();
        tab_view->setObjectName("tab_view");
        tab_view->setStyleSheet(QString::fromUtf8("\n"
"         QLineEdit {\n"
"             background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"             border: 2px solid #8B6F47;\n"
"             border-radius: 8px;\n"
"             padding: 3px 12px;\n"
"             font-size: 14px;\n"
"             color: #333;\n"
"             selection-background-color: #8B6F47;\n"
"             selection-color: white;\n"
"         }\n"
"         QLineEdit:hover {\n"
"             border: 2px solid #A0825A;\n"
"             background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"         }\n"
"         QLineEdit:focus {\n"
"             border: 2px solid #8B4513; \n"
"             background: #FFFAF0;\n"
"         }\n"
"      "));
        label_titre_liste = new QLabel(tab_view);
        label_titre_liste->setObjectName("label_titre_liste");
        label_titre_liste->setGeometry(QRect(500, 20, 300, 30));
        label_titre_liste->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;\n"
""));
        label_titre_liste->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_search = new QPushButton(tab_view);
        btn_search->setObjectName("btn_search");
        btn_search->setGeometry(QRect(100, 70, 150, 30));
        btn_search->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
""));
        le_recherche = new QLineEdit(tab_view);
        le_recherche->setObjectName("le_recherche");
        le_recherche->setGeometry(QRect(280, 70, 500, 30));
        le_recherche->setStyleSheet(QString::fromUtf8(""));
        label_sort = new QLabel(tab_view);
        label_sort->setObjectName("label_sort");
        label_sort->setGeometry(QRect(100, 110, 50, 30));
        label_sort->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-family: 'Outfit'; font-size: 14px; font-weight: bold; background: transparent;"));
        label_sort->setAlignment(Qt::AlignmentFlag::AlignVCenter|Qt::AlignmentFlag::AlignRight);
        cb_sort_field = new QComboBox(tab_view);
        cb_sort_field->addItem(QString());
        cb_sort_field->addItem(QString());
        cb_sort_field->addItem(QString());
        cb_sort_field->addItem(QString());
        cb_sort_field->setObjectName("cb_sort_field");
        cb_sort_field->setGeometry(QRect(160, 110, 180, 30));
        cb_sort_field->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #FFFFFF; border: 2px solid #8B6F47; border-radius: 8px; padding: 3px 10px; font-size: 13px; color: #333; } QComboBox:hover { border: 2px solid #A0825A; } QComboBox::drop-down { border: none; width: 22px; } QComboBox QAbstractItemView { background: #FFF; border: 1px solid #8B6F47; selection-background-color: #8B6F47; selection-color: white; color: #333; }"));
        btn_sort_order = new QPushButton(tab_view);
        btn_sort_order->setObjectName("btn_sort_order");
        btn_sort_order->setGeometry(QRect(350, 110, 140, 30));
        btn_sort_order->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; font-size: 13px; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        tableView = new QTableView(tab_view);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(20, 150, 1321, 480));
        tableView->setStyleSheet(QString::fromUtf8("QTableView {\n"
"  background-color: rgba(15, 12, 8, 0.85);\n"
"  border: 1px solid rgba(212, 175, 55, 0.3);\n"
"  border-radius: 20px;\n"
"  gridline-color: rgba(212, 175, 55, 0.05);\n"
"  color: #F0E6D2;\n"
"  font-family: 'Outfit', 'Segoe UI';\n"
"  font-size: 13px;\n"
"  selection-background-color: rgba(212, 175, 55, 0.25);\n"
"  selection-color: #FFFFFF;\n"
"}\n"
"QHeaderView::section {\n"
"  background-color: rgba(40, 32, 20, 0.9);\n"
"  color: #D4AF37;\n"
"  padding: 15px;\n"
"  border-bottom: 2px solid #D4AF37;\n"
"  border-right: 1px solid rgba(212, 175, 55, 0.1);\n"
"  font-weight: 800;\n"
"  text-transform: uppercase;\n"
"  letter-spacing: 1px;\n"
"}\n"
"QTableView::item {\n"
"  padding: 12px;\n"
"  border-bottom: 1px solid rgba(212, 175, 0, 0.03);\n"
"}\n"
"QScrollBar:vertical {\n"
"  background: rgba(15, 12, 8, 0.85);\n"
"  width: 12px;\n"
"  border-radius: 6px;\n"
"}\n"
"QScrollBar::handle:vertical {\n"
"  background: #D4AF37;\n"
"  border-radius: 6px;\n"
"  min-height: 20px;\n"
"}\n"
"QScrollBar"
                        "::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }\n"
"QScrollBar:horizontal {\n"
"  background: rgba(15, 12, 8, 0.85);\n"
"  height: 12px;\n"
"  border-radius: 6px;\n"
"}\n"
"QScrollBar::handle:horizontal {\n"
"  background: #D4AF37;\n"
"  border-radius: 6px;\n"
"  min-width: 20px;\n"
"}\n"
"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { border: none; background: none; }\n"
""));
        tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        btn_pdf = new QPushButton(tab_view);
        btn_pdf->setObjectName("btn_pdf");
        btn_pdf->setGeometry(QRect(800, 70, 100, 30));
        btn_pdf->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
""));
        btn_delete = new QPushButton(tab_view);
        btn_delete->setObjectName("btn_delete");
        btn_delete->setGeometry(QRect(920, 70, 120, 30));
        btn_delete->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #5A1A1A; border: 1px solid #FF3333; border-radius: 10px; color: #FF9999; font-weight: bold; } QPushButton:hover { background-color: #FF3333; color: white; }\n"
""));
        btn_edit_view = new QPushButton(tab_view);
        btn_edit_view->setObjectName("btn_edit_view");
        btn_edit_view->setGeometry(QRect(1060, 70, 120, 30));
        btn_edit_view->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
""));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/assets/modify.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_edit_view->setIcon(icon2);
        btn_edit_view->setIconSize(QSize(24, 24));
        tabWidget->addTab(tab_view, QString());
        tab_modify = new QWidget();
        tab_modify->setObjectName("tab_modify");
        group_modify = new QGroupBox(tab_modify);
        group_modify->setObjectName("group_modify");
        group_modify->setGeometry(QRect(20, 20, 1321, 721));
        group_modify->setStyleSheet(QString::fromUtf8("\n"
"        QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"        QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: transparent; border-bottom: 2px solid #8B6F47; font-family: 'Consolas'; color: #D4AF37; font-weight: bold; font-size: 16px; border-radius: 0px; }\n"
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
"            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"        }\n"
"        QLineEdit:focus {\n"
" "
                        "           border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"       "));
        label_id_mod = new QLabel(group_modify);
        label_id_mod->setObjectName("label_id_mod");
        label_id_mod->setGeometry(QRect(200, 60, 150, 30));
        label_id_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_nom_mod = new QLabel(group_modify);
        label_nom_mod->setObjectName("label_nom_mod");
        label_nom_mod->setGeometry(QRect(200, 130, 150, 30));
        label_nom_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_prenom_mod = new QLabel(group_modify);
        label_prenom_mod->setObjectName("label_prenom_mod");
        label_prenom_mod->setGeometry(QRect(200, 200, 150, 30));
        label_prenom_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_adresse_mod = new QLabel(group_modify);
        label_adresse_mod->setObjectName("label_adresse_mod");
        label_adresse_mod->setGeometry(QRect(200, 270, 150, 30));
        label_adresse_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_tel_mod = new QLabel(group_modify);
        label_tel_mod->setObjectName("label_tel_mod");
        label_tel_mod->setGeometry(QRect(200, 340, 150, 30));
        label_tel_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_email_mod = new QLabel(group_modify);
        label_email_mod->setObjectName("label_email_mod");
        label_email_mod->setGeometry(QRect(200, 410, 150, 30));
        label_email_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_sexe_mod = new QLabel(group_modify);
        label_sexe_mod->setObjectName("label_sexe_mod");
        label_sexe_mod->setGeometry(QRect(200, 480, 150, 30));
        label_sexe_mod->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        le_id_mod = new QLineEdit(group_modify);
        le_id_mod->setObjectName("le_id_mod");
        le_id_mod->setGeometry(QRect(400, 60, 250, 30));
        le_id_mod->setStyleSheet(QString::fromUtf8(""));
        le_nom_mod = new QLineEdit(group_modify);
        le_nom_mod->setObjectName("le_nom_mod");
        le_nom_mod->setGeometry(QRect(400, 130, 250, 30));
        le_nom_mod->setStyleSheet(QString::fromUtf8(""));
        le_prenom_mod = new QLineEdit(group_modify);
        le_prenom_mod->setObjectName("le_prenom_mod");
        le_prenom_mod->setGeometry(QRect(400, 200, 250, 30));
        le_prenom_mod->setStyleSheet(QString::fromUtf8(""));
        le_adresse_mod = new QLineEdit(group_modify);
        le_adresse_mod->setObjectName("le_adresse_mod");
        le_adresse_mod->setGeometry(QRect(400, 270, 250, 30));
        le_adresse_mod->setStyleSheet(QString::fromUtf8(""));
        le_tel_mod = new QLineEdit(group_modify);
        le_tel_mod->setObjectName("le_tel_mod");
        le_tel_mod->setGeometry(QRect(400, 340, 250, 30));
        le_tel_mod->setStyleSheet(QString::fromUtf8(""));
        le_email_mod = new QLineEdit(group_modify);
        le_email_mod->setObjectName("le_email_mod");
        le_email_mod->setGeometry(QRect(400, 410, 250, 30));
        le_email_mod->setStyleSheet(QString::fromUtf8(""));
        rb_homme_mod = new QRadioButton(group_modify);
        bg_client_gender_mod = new QButtonGroup(ClientManagement);
        bg_client_gender_mod->setObjectName("bg_client_gender_mod");
        bg_client_gender_mod->addButton(rb_homme_mod);
        rb_homme_mod->setObjectName("rb_homme_mod");
        rb_homme_mod->setGeometry(QRect(400, 480, 100, 30));
        rb_femme_mod = new QRadioButton(group_modify);
        bg_client_gender_mod->addButton(rb_femme_mod);
        rb_femme_mod->setObjectName("rb_femme_mod");
        rb_femme_mod->setGeometry(QRect(510, 480, 100, 30));
        btn_modify = new QPushButton(group_modify);
        btn_modify->setObjectName("btn_modify");
        btn_modify->setGeometry(QRect(750, 200, 150, 40));
        btn_modify->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"             "));
        btn_modify->setIcon(icon2);
        btn_modify->setIconSize(QSize(24, 24));
        btn_clear_mod = new QPushButton(group_modify);
        btn_clear_mod->setObjectName("btn_clear_mod");
        btn_clear_mod->setGeometry(QRect(750, 410, 150, 40));
        btn_clear_mod->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        btn_clear_mod->setIcon(icon1);
        btn_clear_mod->setIconSize(QSize(24, 24));
        tabWidget->addTab(tab_modify, QString());
        tab_stats = new QWidget();
        tab_stats->setObjectName("tab_stats");
        label_stat_title = new QLabel(tab_stats);
        label_stat_title->setObjectName("label_stat_title");
        label_stat_title->setGeometry(QRect(760, 20, 540, 40));
        label_stat_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: white;\n"
""));
        label_stat_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        widget_chart = new QFrame(tab_stats);
        widget_chart->setObjectName("widget_chart");
        widget_chart->setGeometry(QRect(100, 80, 1100, 600));
        widget_chart->setStyleSheet(QString::fromUtf8("background-color: white; border: 2px solid #333;\n"
""));
        widget_chart->setFrameShape(QFrame::Shape::StyledPanel);
        widget_chart->setFrameShadow(QFrame::Shadow::Raised);
        tabWidget->addTab(tab_stats, QString());
        tab_mail = new QWidget();
        tab_mail->setObjectName("tab_mail");
        tab_mail->setStyleSheet(QString::fromUtf8("\n"
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
"    "));
        label_mail_title = new QLabel(tab_mail);
        label_mail_title->setObjectName("label_mail_title");
        label_mail_title->setGeometry(QRect(760, 20, 540, 30));
        label_mail_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: white;\n"
""));
        l_smtp = new QLabel(tab_mail);
        l_smtp->setObjectName("l_smtp");
        l_smtp->setGeometry(QRect(200, 80, 150, 25));
        l_smtp->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_smtp = new QLineEdit(tab_mail);
        le_smtp->setObjectName("le_smtp");
        le_smtp->setGeometry(QRect(400, 80, 600, 25));
        l_port = new QLabel(tab_mail);
        l_port->setObjectName("l_port");
        l_port->setGeometry(QRect(200, 120, 150, 25));
        l_port->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_port = new QLineEdit(tab_mail);
        le_port->setObjectName("le_port");
        le_port->setGeometry(QRect(400, 120, 600, 25));
        l_user = new QLabel(tab_mail);
        l_user->setObjectName("l_user");
        l_user->setGeometry(QRect(200, 160, 150, 25));
        l_user->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_user = new QLineEdit(tab_mail);
        le_user->setObjectName("le_user");
        le_user->setGeometry(QRect(400, 160, 600, 25));
        l_pass = new QLabel(tab_mail);
        l_pass->setObjectName("l_pass");
        l_pass->setGeometry(QRect(200, 200, 150, 25));
        l_pass->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_pass = new QLineEdit(tab_mail);
        le_pass->setObjectName("le_pass");
        le_pass->setGeometry(QRect(400, 200, 600, 25));
        le_pass->setEchoMode(QLineEdit::EchoMode::Password);
        l_to = new QLabel(tab_mail);
        l_to->setObjectName("l_to");
        l_to->setGeometry(QRect(200, 240, 150, 25));
        l_to->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_to = new QLineEdit(tab_mail);
        le_to->setObjectName("le_to");
        le_to->setGeometry(QRect(400, 240, 600, 25));
        l_subj = new QLabel(tab_mail);
        l_subj->setObjectName("l_subj");
        l_subj->setGeometry(QRect(200, 280, 150, 25));
        l_subj->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_subject = new QLineEdit(tab_mail);
        le_subject->setObjectName("le_subject");
        le_subject->setGeometry(QRect(400, 280, 600, 25));
        l_att = new QLabel(tab_mail);
        l_att->setObjectName("l_att");
        l_att->setGeometry(QRect(200, 320, 150, 25));
        l_att->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        le_attachment = new QLineEdit(tab_mail);
        le_attachment->setObjectName("le_attachment");
        le_attachment->setGeometry(QRect(400, 320, 500, 25));
        btn_browse = new QPushButton(tab_mail);
        btn_browse->setObjectName("btn_browse");
        btn_browse->setGeometry(QRect(910, 320, 90, 25));
        btn_browse->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        l_msg = new QLabel(tab_mail);
        l_msg->setObjectName("l_msg");
        l_msg->setGeometry(QRect(200, 360, 150, 25));
        l_msg->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        te_message = new QTextEdit(tab_mail);
        te_message->setObjectName("te_message");
        te_message->setGeometry(QRect(400, 360, 600, 250));
        te_message->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px;\n"
""));
        btn_send = new QPushButton(tab_mail);
        btn_send->setObjectName("btn_send");
        btn_send->setGeometry(QRect(900, 650, 100, 40));
        btn_send->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        tabWidget->addTab(tab_mail, QString());
        tab_calendar = new QWidget();
        tab_calendar->setObjectName("tab_calendar");
        label_calendar_title = new QLabel(tab_calendar);
        label_calendar_title->setObjectName("label_calendar_title");
        label_calendar_title->setGeometry(QRect(760, 20, 540, 30));
        label_calendar_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: white;"));
        calendarWidget = new QCalendarWidget(tab_calendar);
        calendarWidget->setObjectName("calendarWidget");
        calendarWidget->setGeometry(QRect(100, 80, 1100, 600));
        calendarWidget->setStyleSheet(QString::fromUtf8("background-color: white; color: #333;"));
        tabWidget->addTab(tab_calendar, QString());
        tab_cyber_trace = new QWidget();
        tab_cyber_trace->setObjectName("tab_cyber_trace");
        tableView_cyber = new QTableView(tab_cyber_trace);
        tableView_cyber->setObjectName("tableView_cyber");
        tableView_cyber->setGeometry(QRect(20, 20, 1200, 660));
        tableView_cyber->setStyleSheet(QString::fromUtf8("QTableView { background: rgba(0,0,0,0.6); gridline-color: #5A4A32; border: 1px solid #8B6F47; color: #D4AF37; font-family: 'Consolas'; } QHeaderView::section { background: rgba(139,111,71,0.3); border: 1px solid #8B6F47; color: #D4AF37; font-weight: bold; } QTableView::item:selected { background: rgba(139,111,71,0.5); border: 1px solid #D4AF37; }"));
        btn_refresh_trace = new QPushButton(tab_cyber_trace);
        btn_refresh_trace->setObjectName("btn_refresh_trace");
        btn_refresh_trace->setGeometry(QRect(1070, 700, 150, 40));
        btn_refresh_trace->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139, 111, 71, 0.4); border: 1px solid #8B6F47; border-radius: 5px; color: #D4AF37; font-weight: bold; font-family: 'Consolas'; } QPushButton:hover { background: rgba(139, 111, 71, 0.8); border: 1px solid #D4AF37; }"));
        tabWidget->addTab(tab_cyber_trace, QString());
        tab_data_matrix = new QWidget();
        tab_data_matrix->setObjectName("tab_data_matrix");
        frame_matrix = new QFrame(tab_data_matrix);
        frame_matrix->setObjectName("frame_matrix");
        frame_matrix->setGeometry(QRect(100, 100, 1040, 550));
        frame_matrix->setStyleSheet(QString::fromUtf8("background: rgba(10, 10, 10, 0.7); border: 2px solid #8B6F47; border-radius: 10px;"));
        label_matrix_title = new QLabel(tab_data_matrix);
        label_matrix_title->setObjectName("label_matrix_title");
        label_matrix_title->setGeometry(QRect(370, 30, 500, 40));
        label_matrix_title->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-size: 24px; font-weight: bold; font-family: 'Consolas';"));
        tabWidget->addTab(tab_data_matrix, QString());
        btn_return_home = new QPushButton(ClientManagement);
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

        retranslateUi(ClientManagement);
        QObject::connect(btn_help_add, &QToolButton::toggled, lbl_hint_add, &QLabel::setVisible);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ClientManagement);
    } // setupUi

    void retranslateUi(QWidget *ClientManagement)
    {
        ClientManagement->setWindowTitle(QCoreApplication::translate("ClientManagement", "Client Management", nullptr));
        group_add->setTitle(QCoreApplication::translate("ClientManagement", "Add Client", nullptr));
        label_nom->setText(QCoreApplication::translate("ClientManagement", "Last Name:", nullptr));
        label_prenom->setText(QCoreApplication::translate("ClientManagement", "First Name:", nullptr));
        label_adresse->setText(QCoreApplication::translate("ClientManagement", "Address:", nullptr));
        label_tel->setText(QCoreApplication::translate("ClientManagement", "Phone Number:", nullptr));
        label_email->setText(QCoreApplication::translate("ClientManagement", "Email Address:", nullptr));
        label_sexe->setText(QCoreApplication::translate("ClientManagement", "Gender:", nullptr));
        rb_homme->setText(QCoreApplication::translate("ClientManagement", "Male", nullptr));
        rb_femme->setText(QCoreApplication::translate("ClientManagement", "Female", nullptr));
        btn_add->setText(QCoreApplication::translate("ClientManagement", "Add", nullptr));
        btn_clear->setText(QCoreApplication::translate("ClientManagement", "Clear Fields", nullptr));
        btn_help_add->setText(QCoreApplication::translate("ClientManagement", "?", nullptr));
        lbl_hint_add->setText(QCoreApplication::translate("ClientManagement", "Help: Enter client details here. Use 'Add' to save. In the 'View Clients' tab, you can search and export lists to PDF.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_add), QCoreApplication::translate("ClientManagement", "Add Client", nullptr));
        label_titre_liste->setText(QCoreApplication::translate("ClientManagement", "Client List", nullptr));
        btn_search->setText(QCoreApplication::translate("ClientManagement", "Search", nullptr));
        label_sort->setText(QCoreApplication::translate("ClientManagement", "Sort:", nullptr));
        cb_sort_field->setItemText(0, QCoreApplication::translate("ClientManagement", "Default (ID)", nullptr));
        cb_sort_field->setItemText(1, QCoreApplication::translate("ClientManagement", "First Name", nullptr));
        cb_sort_field->setItemText(2, QCoreApplication::translate("ClientManagement", "Last Name", nullptr));
        cb_sort_field->setItemText(3, QCoreApplication::translate("ClientManagement", "Gender", nullptr));

        btn_sort_order->setText(QCoreApplication::translate("ClientManagement", "Ascending \342\226\262", nullptr));
        btn_pdf->setText(QCoreApplication::translate("ClientManagement", "PDF", nullptr));
        btn_delete->setText(QCoreApplication::translate("ClientManagement", "\342\235\214 Delete", nullptr));
        btn_edit_view->setText(QCoreApplication::translate("ClientManagement", "Edit", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_view), QCoreApplication::translate("ClientManagement", "View Clients", nullptr));
        group_modify->setTitle(QCoreApplication::translate("ClientManagement", "Modify Client", nullptr));
        label_id_mod->setText(QCoreApplication::translate("ClientManagement", "ID:", nullptr));
        label_nom_mod->setText(QCoreApplication::translate("ClientManagement", "Last Name:", nullptr));
        label_prenom_mod->setText(QCoreApplication::translate("ClientManagement", "First Name:", nullptr));
        label_adresse_mod->setText(QCoreApplication::translate("ClientManagement", "Address:", nullptr));
        label_tel_mod->setText(QCoreApplication::translate("ClientManagement", "Phone Number:", nullptr));
        label_email_mod->setText(QCoreApplication::translate("ClientManagement", "Email Account:", nullptr));
        label_sexe_mod->setText(QCoreApplication::translate("ClientManagement", "Gender:", nullptr));
        rb_homme_mod->setText(QCoreApplication::translate("ClientManagement", "Male", nullptr));
        rb_femme_mod->setText(QCoreApplication::translate("ClientManagement", "Female", nullptr));
        btn_modify->setText(QCoreApplication::translate("ClientManagement", "Modify", nullptr));
        btn_clear_mod->setText(QCoreApplication::translate("ClientManagement", "Clear Fields", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_modify), QCoreApplication::translate("ClientManagement", "Modify Client", nullptr));
        label_stat_title->setText(QCoreApplication::translate("ClientManagement", "Client Statistics by Gender", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_stats), QCoreApplication::translate("ClientManagement", "Statistics", nullptr));
        label_mail_title->setText(QCoreApplication::translate("ClientManagement", "Mailing", nullptr));
        l_smtp->setText(QCoreApplication::translate("ClientManagement", "Smtp-server:", nullptr));
        l_port->setText(QCoreApplication::translate("ClientManagement", "Server port:", nullptr));
        l_user->setText(QCoreApplication::translate("ClientManagement", "Username:", nullptr));
        l_pass->setText(QCoreApplication::translate("ClientManagement", "Password:", nullptr));
        l_to->setText(QCoreApplication::translate("ClientManagement", "Recipant to:", nullptr));
        l_subj->setText(QCoreApplication::translate("ClientManagement", "Subject:", nullptr));
        l_att->setText(QCoreApplication::translate("ClientManagement", "Attachment:", nullptr));
        btn_browse->setText(QCoreApplication::translate("ClientManagement", "Browse", nullptr));
        l_msg->setText(QCoreApplication::translate("ClientManagement", "Message:", nullptr));
        btn_send->setText(QCoreApplication::translate("ClientManagement", "Send", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_mail), QCoreApplication::translate("ClientManagement", "Send Email", nullptr));
        label_calendar_title->setText(QCoreApplication::translate("ClientManagement", "Client Calendar", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_calendar), QCoreApplication::translate("ClientManagement", "Calendar", nullptr));
        btn_refresh_trace->setText(QCoreApplication::translate("ClientManagement", "UPDATE LOG", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_cyber_trace), QCoreApplication::translate("ClientManagement", "Cyber Trace", nullptr));
        label_matrix_title->setText(QCoreApplication::translate("ClientManagement", "-- DATA MATRIX PROTOCOL ACTIVATED --", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_data_matrix), QCoreApplication::translate("ClientManagement", "Data Matrix", nullptr));
        btn_return_home->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ClientManagement: public Ui_ClientManagement {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENT_MANAGEMENT_H
