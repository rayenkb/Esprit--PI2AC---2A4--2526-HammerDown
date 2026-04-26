/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *page_login;
    QLineEdit *login_id;
    QLineEdit *login_pass;
    QPushButton *login;
    QPushButton *oublie;
    QWidget *page_home;
    QGridLayout *homeGridLayout;
    QPushButton *gs_equipment;
    QPushButton *gs_order;
    QSpacerItem *homeRightSpacer;
    QPushButton *gs_fournisseur;
    QPushButton *gs_client;
    QPushButton *gs_employes;
    QWidget *page_employee;
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
    QSpinBox *sb_age;
    QLineEdit *le_mdp;
    QDoubleSpinBox *dsb_salaire;
    QLineEdit *le_email;
    QLineEdit *le_num;
    QPushButton *btn_add;
    QPushButton *btn_modify;
    QPushButton *btn_cancel;
    QToolButton *btn_help_add;
    QLabel *lbl_hint_add;
    QWidget *tab_view;
    QLabel *label_titre_liste_emp;
    QPushButton *on_emp_chercher;
    QWidget *page_client;
    QTabWidget *tabWidget1;
    QWidget *tab_add1;
    QGroupBox *group_add1;
    QLabel *label_id1;
    QLabel *label_nom1;
    QLabel *label_prenom1;
    QLabel *label_adresse;
    QLabel *label_tel;
    QLabel *label_email1;
    QLabel *label_sexe;
    QLineEdit *le_id1;
    QLineEdit *le_nom1;
    QLineEdit *le_prenom1;
    QLineEdit *le_adresse;
    QLineEdit *le_tel;
    QLineEdit *le_email1;
    QRadioButton *rb_homme;
    QRadioButton *rb_femme;
    QPushButton *btn_add1;
    QPushButton *btn_cancel1;
    QToolButton *btn_help_add_client;
    QLabel *lbl_hint_add_client;
    QPushButton *btn_modify_2;
    QWidget *tab_view1;
    QLabel *label_titre_liste;
    QPushButton *btn_search;
    QLineEdit *le_recherche;
    QTableView *tableView;
    QPushButton *btn_pdf;
    QPushButton *btn_delete;
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
    QWidget *page_supplier;
    QTabWidget *tabWidget2;
    QWidget *tab_gestion;
    QGroupBox *groupBox_gestion;
    QLabel *label_id2;
    QLineEdit *le_id2;
    QLabel *label_nom2;
    QLineEdit *le_nom2;
    QLabel *label_adresse1;
    QLineEdit *le_adresse1;
    QLabel *label_email2;
    QLineEdit *le_email2;
    QLabel *label_product_type;
    QLineEdit *le_product_type;
    QLabel *label_type;
    QLineEdit *le_type;
    QLabel *label_cp;
    QSpinBox *sb_cp;
    QLabel *label_tel1;
    QLineEdit *le_tel1;
    QLabel *lbl_image_preview;
    QPushButton *btn_upload_image;
    QPushButton *btn_add2;
    QPushButton *btn_modify1;
    QPushButton *btn_delete1;
    QPushButton *btn_cancel_gestion;
    QLabel *label_sms;
    QTextEdit *txt_sms;
    QPushButton *btn_send_sms;
    QToolButton *btn_help_gestion_supplier;
    QLabel *lbl_hint_gestion_supplier;
    QWidget *tab_stats1;
    QLabel *lbl_stats_title;
    QToolButton *btn_help_stats_supplier;
    QLabel *lbl_hint_stats_supplier;
    QFrame *frame_circle_retention;
    QLabel *lbl_percent_retention;
    QLabel *lbl_text_retention;
    QFrame *frame_circle_accuracy;
    QLabel *lbl_percent_accuracy;
    QLabel *lbl_text_accuracy;
    QGroupBox *group_performance_bars;
    QLabel *lbl_bar_quality;
    QProgressBar *pb_quality;
    QLabel *lbl_bar_speed;
    QProgressBar *pb_speed;
    QFrame *frame_stat_summary;
    QLabel *lbl_summary_title;
    QLabel *lbl_summary_val;
    QFrame *frame_chart_types;
    QLabel *lbl_chart_types;
    QGraphicsView *chart_types_view;
    QFrame *frame_chart_reviews;
    QLabel *lbl_chart_reviews;
    QGraphicsView *chart_reviews_view;
    QGroupBox *group_regional_stats;
    QLabel *lbl_reg_1;
    QProgressBar *pb_reg_1;
    QLabel *lbl_reg_2;
    QProgressBar *pb_reg_2;
    QLabel *lbl_top_performer;
    QWidget *tab_view2;
    QLabel *lbl_title_view;
    QPushButton *btn_chercher;
    QLineEdit *le_recherche1;
    QTableView *tableView1;
    QWidget *tab_reviews;
    QLabel *lbl_reviews_title;
    QToolButton *btn_help_reviews_supplier;
    QLabel *lbl_hint_reviews_supplier;
    QLabel *lbl_select_supplier;
    QComboBox *cb_supplier_reviews;
    QFrame *frame_rating_header;
    QLabel *lbl_avg_score;
    QLabel *lbl_stars_row;
    QLabel *lbl_review_count;
    QGroupBox *group_feedback_breakdown;
    QLabel *lbl_cat_quality;
    QProgressBar *pb_review_quality;
    QLabel *lbl_cat_response;
    QProgressBar *pb_review_response;
    QLabel *lbl_cat_price;
    QProgressBar *pb_review_price;
    QFrame *frame_distribution;
    QLabel *lbl_dist_title;
    QLabel *lbl_row_5;
    QProgressBar *pb_dist_5;
    QLabel *lbl_row_4;
    QProgressBar *pb_dist_4;
    QLabel *lbl_row_3;
    QProgressBar *pb_dist_3;
    QLabel *lbl_dist_insight;
    QTableView *table_reviews;
    QWidget *page_equipment;
    QTabWidget *tabWidget3;
    QWidget *tab_gestion1;
    QGroupBox *groupBox_gestion1;
    QLabel *label_id3;
    QLineEdit *le_id3;
    QLabel *label_date_achat;
    QDateEdit *de_date_achat;
    QLabel *label_etat;
    QRadioButton *rb_intact;
    QRadioButton *rb_broken;
    QLabel *label_desc;
    QTextEdit *te_desc;
    QPushButton *btn_add3;
    QPushButton *btn_modify2;
    QPushButton *btn_delete2;
    QPushButton *btn_cancel_gestion1;
    QToolButton *btn_help_gestion_equipment;
    QLabel *lbl_hint_gestion_equipment;
    QWidget *tab_view3;
    QLabel *label_titre_liste_equip;
    QPushButton *btn_search_top;
    QLineEdit *le_recherche2;
    QTableView *table_equipments;
    QPushButton *btn_search1;
    QPushButton *btn_delete_confirm;
    QWidget *tab_history;
    QLabel *label_titre_histo;
    QLineEdit *le_history_search;
    QPushButton *btn_history_search;
    QTableView *tableView_historique;
    QPushButton *btn_refresh_history;
    QPushButton *btn_export_history;
    QPushButton *btn_clear_history;
    QWidget *tab_stats2;
    QLabel *label_stats_title;
    QGroupBox *group_metrics;
    QLabel *lbl_total_eq;
    QLabel *val_total_eq;
    QLabel *lbl_oper_eq;
    QLabel *val_oper_eq;
    QLabel *lbl_broken_eq;
    QLabel *val_broken_eq;
    QGroupBox *group_chart;
    QLabel *label_chart_placeholder;
    QToolButton *btn_help_stats_equipment;
    QLabel *lbl_hint_stats_equipment;
    QWidget *page_order;
    QTabWidget *tabWidget4;
    QWidget *tab_manage;
    QGroupBox *group_manage;
    QLabel *label_id4;
    QLineEdit *le_id4;
    QLabel *label_type1;
    QLineEdit *le_type1;
    QLabel *label_stock;
    QLineEdit *le_stock;
    QLabel *label_prix;
    QLineEdit *le_prix;
    QLabel *label_buyer;
    QLineEdit *le_buyer;
    QPushButton *btn_add4;
    QPushButton *btn_modify3;
    QPushButton *btn_delete3;
    QPushButton *btn_clear;
    QPushButton *btn_load;
    QWidget *tab_qrcode;
    QGroupBox *group_qr;
    QLabel *label_qr_order_id;
    QLineEdit *le_qr_order_id;
    QPushButton *btn_generate_qr;
    QLabel *label_qr_display;
    QPushButton *btn_save_qr;
    QPushButton *btn_print_qr;
    QWidget *tab_catalog;
    QLabel *label_catalog_title;
    QTableWidget *table_catalog;
    QPushButton *btn_refresh_catalog;
    QPushButton *btn_export_catalog;
    QPushButton *btn_print_catalog;
    QLineEdit *le_catalog_search;
    QToolButton *btn_help_gestion_supplier_2;
    QButtonGroup *bg_equipment_status_main;
    QButtonGroup *bg_client_gender_main;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1305, 800);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        stackedWidget = new QStackedWidget(centralWidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setEnabled(true);
        page_login = new QWidget();
        page_login->setObjectName("page_login");
        page_login->setMouseTracking(false);
        page_login->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
        page_login->setStyleSheet(QString::fromUtf8("#page_login {\n"
" border-image: url(:/assets/login.png) 0 0 0 0 stretch stretch;\n"
" }\n"
""));
        login_id = new QLineEdit(page_login);
        login_id->setObjectName("login_id");
        login_id->setGeometry(QRect(490, 290, 301, 51));
        QFont font;
        font.setFamilies({QString::fromUtf8("MS Shell Dlg 2")});
        font.setPointSize(16);
        login_id->setFont(font);
        login_id->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        login_id->setStyleSheet(QString::fromUtf8("background: transparent; border: none;"));
        login_id->setFrame(false);
        login_id->setEchoMode(QLineEdit::EchoMode::Normal);
        login_id->setCursorPosition(0);
        login_id->setDragEnabled(false);
        login_id->setReadOnly(false);
        login_id->setClearButtonEnabled(false);
        login_pass = new QLineEdit(page_login);
        login_pass->setObjectName("login_pass");
        login_pass->setGeometry(QRect(490, 370, 291, 41));
        QFont font1;
        font1.setPointSize(16);
        login_pass->setFont(font1);
        login_pass->setFrame(false);
        login_pass->setEchoMode(QLineEdit::EchoMode::Password);
        login = new QPushButton(page_login);
        login->setObjectName("login");
        login->setEnabled(true);
        login->setGeometry(QRect(480, 470, 241, 31));
        login->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        login->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        login->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
        login->setStyleSheet(QString::fromUtf8("#login {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); border-radius: 5px; }\n"
""));
        login->setFlat(true);
        oublie = new QPushButton(page_login);
        oublie->setObjectName("oublie");
        oublie->setGeometry(QRect(520, 430, 171, 21));
        oublie->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        oublie->setAutoFillBackground(false);
        oublie->setStyleSheet(QString::fromUtf8("#oublie {\n"
"    background: transparent;\n"
"    border: none;\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"}"));
        oublie->setCheckable(false);
        oublie->setAutoRepeat(false);
        oublie->setAutoExclusive(false);
        oublie->setAutoDefault(false);
        oublie->setFlat(true);
        stackedWidget->addWidget(page_login);
        oublie->raise();
        login_id->raise();
        login_pass->raise();
        login->raise();
        page_home = new QWidget();
        page_home->setObjectName("page_home");
        page_home->setStyleSheet(QString::fromUtf8("#page_home {\n"
" border-image: url(:/assets/home.png) 0 0 0 0 stretch stretch;\n"
"}"));
        homeGridLayout = new QGridLayout(page_home);
        homeGridLayout->setSpacing(0);
        homeGridLayout->setObjectName("homeGridLayout");
        homeGridLayout->setContentsMargins(0, 0, 0, 0);
        gs_equipment = new QPushButton(page_home);
        gs_equipment->setObjectName("gs_equipment");
        gs_equipment->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_equipment->setFlat(true);

        homeGridLayout->addWidget(gs_equipment, 2, 3, 1, 2);

        gs_order = new QPushButton(page_home);
        gs_order->setObjectName("gs_order");
        gs_order->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_order->setFlat(true);

        homeGridLayout->addWidget(gs_order, 1, 2, 1, 2);

        homeRightSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        homeGridLayout->addItem(homeRightSpacer, 0, 6, 4, 1);

        gs_fournisseur = new QPushButton(page_home);
        gs_fournisseur->setObjectName("gs_fournisseur");
        gs_fournisseur->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_fournisseur->setFlat(true);

        homeGridLayout->addWidget(gs_fournisseur, 2, 1, 1, 2);

        gs_client = new QPushButton(page_home);
        gs_client->setObjectName("gs_client");
        gs_client->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_client->setFlat(true);

        homeGridLayout->addWidget(gs_client, 1, 4, 1, 2);

        gs_employes = new QPushButton(page_home);
        gs_employes->setObjectName("gs_employes");
        gs_employes->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        gs_employes->setFlat(true);

        homeGridLayout->addWidget(gs_employes, 1, 0, 1, 2);

        homeGridLayout->setRowStretch(0, 250);
        homeGridLayout->setColumnStretch(0, 100);
        stackedWidget->addWidget(page_home);
        page_employee = new QWidget();
        page_employee->setObjectName("page_employee");
        page_employee->setStyleSheet(QString::fromUtf8("\n"
"    #page_employee {\n"
"        border-image: url(:/assets/background.png) 0 0 0 0 stretch stretch;\n"
"    }\n"
"    QPushButton {\n"
"        border-image: url(:/assets/button_bg.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"    }\n"
"    QTabWidget::pane {\n"
"        border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch;\n"
"    }\n"
"   "));
        tabWidget = new QTabWidget(page_employee);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(110, 70, 1051, 681));
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Gadugi")});
        font2.setPointSize(10);
        font2.setBold(true);
        tabWidget->setFont(font2);
        tabWidget->setStyleSheet(QString::fromUtf8("\n"
"       QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"       QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"       QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"       QWidget#tab_add, QWidget#tab_view, QWidget#tab_delete, QWidget#tab_modify, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_calendar { background: transparent; }\n"
"      "));
        tab_add = new QWidget();
        tab_add->setObjectName("tab_add");
        group_add = new QGroupBox(tab_add);
        group_add->setObjectName("group_add");
        group_add->setGeometry(QRect(20, 20, 1321, 721));
        group_add->setStyleSheet(QString::fromUtf8("\n"
"          QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            back"
                        "ground: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"         "));
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
        sb_age = new QSpinBox(group_add);
        sb_age->setObjectName("sb_age");
        sb_age->setGeometry(QRect(400, 260, 250, 30));
        le_mdp = new QLineEdit(group_add);
        le_mdp->setObjectName("le_mdp");
        le_mdp->setGeometry(QRect(400, 310, 250, 30));
        dsb_salaire = new QDoubleSpinBox(group_add);
        dsb_salaire->setObjectName("dsb_salaire");
        dsb_salaire->setGeometry(QRect(400, 360, 250, 30));
        le_email = new QLineEdit(group_add);
        le_email->setObjectName("le_email");
        le_email->setGeometry(QRect(400, 410, 250, 30));
        le_num = new QLineEdit(group_add);
        le_num->setObjectName("le_num");
        le_num->setGeometry(QRect(400, 460, 250, 30));
        btn_add = new QPushButton(group_add);
        btn_add->setObjectName("btn_add");
        btn_add->setGeometry(QRect(750, 200, 150, 40));
        btn_add->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/add.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_add->setIcon(icon);
        btn_add->setIconSize(QSize(24, 24));
        btn_modify = new QPushButton(group_add);
        btn_modify->setObjectName("btn_modify");
        btn_modify->setGeometry(QRect(750, 270, 150, 40));
        btn_modify->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/assets/modify.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_modify->setIcon(icon1);
        btn_modify->setIconSize(QSize(24, 24));
        btn_cancel = new QPushButton(group_add);
        btn_cancel->setObjectName("btn_cancel");
        btn_cancel->setGeometry(QRect(750, 340, 150, 40));
        btn_cancel->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
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
        tab_view->setStyleSheet(QString::fromUtf8("\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"     "));
        label_titre_liste_emp = new QLabel(tab_view);
        label_titre_liste_emp->setObjectName("label_titre_liste_emp");
        label_titre_liste_emp->setGeometry(QRect(600, 20, 300, 30));
        label_titre_liste_emp->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;\n"
""));
        label_titre_liste_emp->setAlignment(Qt::AlignmentFlag::AlignCenter);
        on_emp_chercher = new QPushButton(tab_view);
        on_emp_chercher->setObjectName("on_emp_chercher");
        on_emp_chercher->setGeometry(QRect(700, 640, 150, 40));
        on_emp_chercher->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
""));
        tabWidget->addTab(tab_view, QString());
        stackedWidget->addWidget(page_employee);
        page_client = new QWidget();
        page_client->setObjectName("page_client");
        page_client->setStyleSheet(QString::fromUtf8("\n"
"    #page_client {\n"
"        border-image: url(:/assets/background.png) 0 0 0 0 stretch stretch;\n"
"    }\n"
"    QPushButton {\n"
"        border-image: url(:/assets/button_bg.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"    }\n"
"    QTabWidget::pane {\n"
"        border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch;\n"
"    }\n"
"   "));
        tabWidget1 = new QTabWidget(page_client);
        tabWidget1->setObjectName("tabWidget1");
        tabWidget1->setGeometry(QRect(19, 20, 1241, 760));
        tabWidget1->setFont(font2);
        tabWidget1->setStyleSheet(QString::fromUtf8("\n"
"       QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"       QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"       QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"       QWidget#tab_add, QWidget#tab_view, QWidget#tab_delete, QWidget#tab_modify, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_calendar, QWidget#tab_mail_search { background: transparent; }\n"
"      "));
        tab_add1 = new QWidget();
        tab_add1->setObjectName("tab_add1");
        group_add1 = new QGroupBox(tab_add1);
        group_add1->setObjectName("group_add1");
        group_add1->setGeometry(QRect(20, 20, 1321, 721));
        group_add1->setStyleSheet(QString::fromUtf8("\n"
"          QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            back"
                        "ground: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"         "));
        label_id1 = new QLabel(group_add1);
        label_id1->setObjectName("label_id1");
        label_id1->setGeometry(QRect(200, 60, 150, 30));
        label_id1->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        label_nom1 = new QLabel(group_add1);
        label_nom1->setObjectName("label_nom1");
        label_nom1->setGeometry(QRect(200, 130, 150, 30));
        label_nom1->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        label_prenom1 = new QLabel(group_add1);
        label_prenom1->setObjectName("label_prenom1");
        label_prenom1->setGeometry(QRect(200, 200, 150, 30));
        label_prenom1->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        label_adresse = new QLabel(group_add1);
        label_adresse->setObjectName("label_adresse");
        label_adresse->setGeometry(QRect(200, 270, 150, 30));
        label_adresse->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        label_tel = new QLabel(group_add1);
        label_tel->setObjectName("label_tel");
        label_tel->setGeometry(QRect(200, 340, 150, 30));
        label_tel->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        label_email1 = new QLabel(group_add1);
        label_email1->setObjectName("label_email1");
        label_email1->setGeometry(QRect(200, 410, 150, 30));
        label_email1->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        label_sexe = new QLabel(group_add1);
        label_sexe->setObjectName("label_sexe");
        label_sexe->setGeometry(QRect(200, 480, 150, 30));
        label_sexe->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_id1 = new QLineEdit(group_add1);
        le_id1->setObjectName("le_id1");
        le_id1->setGeometry(QRect(400, 60, 250, 30));
        le_id1->setStyleSheet(QString::fromUtf8(""));
        le_nom1 = new QLineEdit(group_add1);
        le_nom1->setObjectName("le_nom1");
        le_nom1->setGeometry(QRect(400, 130, 250, 30));
        le_nom1->setStyleSheet(QString::fromUtf8(""));
        le_prenom1 = new QLineEdit(group_add1);
        le_prenom1->setObjectName("le_prenom1");
        le_prenom1->setGeometry(QRect(400, 200, 250, 30));
        le_prenom1->setStyleSheet(QString::fromUtf8(""));
        le_adresse = new QLineEdit(group_add1);
        le_adresse->setObjectName("le_adresse");
        le_adresse->setGeometry(QRect(400, 270, 250, 30));
        le_adresse->setStyleSheet(QString::fromUtf8(""));
        le_tel = new QLineEdit(group_add1);
        le_tel->setObjectName("le_tel");
        le_tel->setGeometry(QRect(400, 340, 250, 30));
        le_tel->setStyleSheet(QString::fromUtf8(""));
        le_email1 = new QLineEdit(group_add1);
        le_email1->setObjectName("le_email1");
        le_email1->setGeometry(QRect(400, 410, 250, 30));
        le_email1->setStyleSheet(QString::fromUtf8(""));
        rb_homme = new QRadioButton(group_add1);
        bg_client_gender_main = new QButtonGroup(MainWindow);
        bg_client_gender_main->setObjectName("bg_client_gender_main");
        bg_client_gender_main->addButton(rb_homme);
        rb_homme->setObjectName("rb_homme");
        rb_homme->setGeometry(QRect(400, 480, 100, 30));
        rb_femme = new QRadioButton(group_add1);
        bg_client_gender_main->addButton(rb_femme);
        rb_femme->setObjectName("rb_femme");
        rb_femme->setGeometry(QRect(510, 480, 100, 30));
        btn_add1 = new QPushButton(group_add1);
        btn_add1->setObjectName("btn_add1");
        btn_add1->setGeometry(QRect(870, 200, 150, 40));
        btn_add1->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        btn_add1->setIcon(icon);
        btn_add1->setIconSize(QSize(24, 24));
        btn_cancel1 = new QPushButton(group_add1);
        btn_cancel1->setObjectName("btn_cancel1");
        btn_cancel1->setGeometry(QRect(870, 340, 150, 40));
        btn_cancel1->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        btn_help_add_client = new QToolButton(group_add1);
        btn_help_add_client->setObjectName("btn_help_add_client");
        btn_help_add_client->setGeometry(QRect(1180, 20, 30, 30));
        btn_help_add_client->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"            QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"));
        btn_help_add_client->setCheckable(true);
        lbl_hint_add_client = new QLabel(group_add1);
        lbl_hint_add_client->setObjectName("lbl_hint_add_client");
        lbl_hint_add_client->setGeometry(QRect(200, 550, 800, 60));
        lbl_hint_add_client->setVisible(false);
        lbl_hint_add_client->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        btn_modify_2 = new QPushButton(group_add1);
        btn_modify_2->setObjectName("btn_modify_2");
        btn_modify_2->setGeometry(QRect(870, 270, 150, 40));
        btn_modify_2->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        btn_modify_2->setIcon(icon1);
        btn_modify_2->setIconSize(QSize(24, 24));
        tabWidget1->addTab(tab_add1, QString());
        tab_view1 = new QWidget();
        tab_view1->setObjectName("tab_view1");
        tab_view1->setStyleSheet(QString::fromUtf8("\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"     "));
        label_titre_liste = new QLabel(tab_view1);
        label_titre_liste->setObjectName("label_titre_liste");
        label_titre_liste->setGeometry(QRect(500, 20, 300, 30));
        label_titre_liste->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;"));
        label_titre_liste->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_search = new QPushButton(tab_view1);
        btn_search->setObjectName("btn_search");
        btn_search->setGeometry(QRect(100, 70, 150, 30));
        btn_search->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        le_recherche = new QLineEdit(tab_view1);
        le_recherche->setObjectName("le_recherche");
        le_recherche->setGeometry(QRect(280, 70, 500, 30));
        le_recherche->setStyleSheet(QString::fromUtf8(""));
        tableView = new QTableView(tab_view1);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(20, 130, 1321, 500));
        tableView->setStyleSheet(QString::fromUtf8("\n"
"       QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 5px; }\n"
"       QTableView { border: 1px solid #8B6F47; selection-background-color: #E0E0E0; selection-color: white; }\n"
"      "));
        btn_pdf = new QPushButton(tab_view1);
        btn_pdf->setObjectName("btn_pdf");
        btn_pdf->setGeometry(QRect(800, 70, 100, 30));
        btn_pdf->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        btn_delete = new QPushButton(tab_view1);
        btn_delete->setObjectName("btn_delete");
        btn_delete->setGeometry(QRect(920, 70, 120, 30));
        btn_delete->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/assets/delete.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_delete->setIcon(icon2);
        btn_delete->setIconSize(QSize(24, 24));
        tabWidget1->addTab(tab_view1, QString());
        tab_stats = new QWidget();
        tab_stats->setObjectName("tab_stats");
        label_stat_title = new QLabel(tab_stats);
        label_stat_title->setObjectName("label_stat_title");
        label_stat_title->setGeometry(QRect(200, 20, 800, 40));
        label_stat_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: white;"));
        label_stat_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        widget_chart = new QFrame(tab_stats);
        widget_chart->setObjectName("widget_chart");
        widget_chart->setGeometry(QRect(100, 80, 1100, 600));
        widget_chart->setStyleSheet(QString::fromUtf8("background-color: white; border: 2px solid #333;"));
        widget_chart->setFrameShape(QFrame::Shape::StyledPanel);
        widget_chart->setFrameShadow(QFrame::Shadow::Raised);
        tabWidget1->addTab(tab_stats, QString());
        tab_mail = new QWidget();
        tab_mail->setObjectName("tab_mail");
        tab_mail->setStyleSheet(QString::fromUtf8("\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"     "));
        label_mail_title = new QLabel(tab_mail);
        label_mail_title->setObjectName("label_mail_title");
        label_mail_title->setGeometry(QRect(600, 20, 200, 30));
        label_mail_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: white;"));
        l_smtp = new QLabel(tab_mail);
        l_smtp->setObjectName("l_smtp");
        l_smtp->setGeometry(QRect(200, 80, 150, 25));
        l_smtp->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_smtp = new QLineEdit(tab_mail);
        le_smtp->setObjectName("le_smtp");
        le_smtp->setGeometry(QRect(400, 80, 600, 25));
        le_smtp->setStyleSheet(QString::fromUtf8(""));
        l_port = new QLabel(tab_mail);
        l_port->setObjectName("l_port");
        l_port->setGeometry(QRect(200, 120, 150, 25));
        l_port->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_port = new QLineEdit(tab_mail);
        le_port->setObjectName("le_port");
        le_port->setGeometry(QRect(400, 120, 600, 25));
        le_port->setStyleSheet(QString::fromUtf8(""));
        l_user = new QLabel(tab_mail);
        l_user->setObjectName("l_user");
        l_user->setGeometry(QRect(200, 160, 150, 25));
        l_user->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_user = new QLineEdit(tab_mail);
        le_user->setObjectName("le_user");
        le_user->setGeometry(QRect(400, 160, 600, 25));
        le_user->setStyleSheet(QString::fromUtf8(""));
        l_pass = new QLabel(tab_mail);
        l_pass->setObjectName("l_pass");
        l_pass->setGeometry(QRect(200, 200, 150, 25));
        l_pass->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_pass = new QLineEdit(tab_mail);
        le_pass->setObjectName("le_pass");
        le_pass->setGeometry(QRect(400, 200, 600, 25));
        le_pass->setStyleSheet(QString::fromUtf8(""));
        le_pass->setEchoMode(QLineEdit::EchoMode::Password);
        l_to = new QLabel(tab_mail);
        l_to->setObjectName("l_to");
        l_to->setGeometry(QRect(200, 240, 150, 25));
        l_to->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_to = new QLineEdit(tab_mail);
        le_to->setObjectName("le_to");
        le_to->setGeometry(QRect(400, 240, 600, 25));
        le_to->setStyleSheet(QString::fromUtf8(""));
        l_subj = new QLabel(tab_mail);
        l_subj->setObjectName("l_subj");
        l_subj->setGeometry(QRect(200, 280, 150, 25));
        l_subj->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_subject = new QLineEdit(tab_mail);
        le_subject->setObjectName("le_subject");
        le_subject->setGeometry(QRect(400, 280, 600, 25));
        le_subject->setStyleSheet(QString::fromUtf8(""));
        l_att = new QLabel(tab_mail);
        l_att->setObjectName("l_att");
        l_att->setGeometry(QRect(200, 320, 150, 25));
        l_att->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        le_attachment = new QLineEdit(tab_mail);
        le_attachment->setObjectName("le_attachment");
        le_attachment->setGeometry(QRect(400, 320, 500, 25));
        le_attachment->setStyleSheet(QString::fromUtf8(""));
        btn_browse = new QPushButton(tab_mail);
        btn_browse->setObjectName("btn_browse");
        btn_browse->setGeometry(QRect(910, 320, 90, 25));
        btn_browse->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 5px; color: white; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        l_msg = new QLabel(tab_mail);
        l_msg->setObjectName("l_msg");
        l_msg->setGeometry(QRect(200, 360, 150, 25));
        l_msg->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        te_message = new QTextEdit(tab_mail);
        te_message->setObjectName("te_message");
        te_message->setGeometry(QRect(400, 360, 600, 250));
        te_message->setStyleSheet(QString::fromUtf8(""));
        btn_send = new QPushButton(tab_mail);
        btn_send->setObjectName("btn_send");
        btn_send->setGeometry(QRect(900, 650, 100, 40));
        btn_send->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        tabWidget1->addTab(tab_mail, QString());
        stackedWidget->addWidget(page_client);
        page_supplier = new QWidget();
        page_supplier->setObjectName("page_supplier");
        page_supplier->setStyleSheet(QString::fromUtf8("\n"
"    #page_supplier {\n"
"        border-image: url(:/assets/background.png) 0 0 0 0 stretch stretch;\n"
"    }\n"
"    QPushButton {\n"
"        border-image: url(:/assets/button_bg.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"    }\n"
"    QTabWidget::pane {\n"
"        border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch;\n"
"    }\n"
"   "));
        tabWidget2 = new QTabWidget(page_supplier);
        tabWidget2->setObjectName("tabWidget2");
        tabWidget2->setGeometry(QRect(20, 10, 1191, 781));
        tabWidget2->setFont(font2);
        tabWidget2->setStyleSheet(QString::fromUtf8("\n"
"       QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"       QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"       QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"       QWidget#tab_gestion, QWidget#tab_view, QWidget#tab_stats, QWidget#tab_reviews { background: transparent; }\n"
"      "));
        tab_gestion = new QWidget();
        tab_gestion->setObjectName("tab_gestion");
        groupBox_gestion = new QGroupBox(tab_gestion);
        groupBox_gestion->setObjectName("groupBox_gestion");
        groupBox_gestion->setGeometry(QRect(20, 20, 1200, 800));
        groupBox_gestion->setStyleSheet(QString::fromUtf8("\n"
"         QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"         QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            backgr"
                        "ound: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"         QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"         QLineEdit {\n"
"             background: qlineargradi"
                        "ent(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
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
"        "));
        label_id2 = new QLabel(groupBox_gestion);
        label_id2->setObjectName("label_id2");
        label_id2->setGeometry(QRect(50, 100, 150, 30));
        label_id2->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_id2 = new QLineEdit(groupBox_gestion);
        le_id2->setObjectName("le_id2");
        le_id2->setGeometry(QRect(200, 100, 200, 30));
        le_id2->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_nom2 = new QLabel(groupBox_gestion);
        label_nom2->setObjectName("label_nom2");
        label_nom2->setGeometry(QRect(50, 170, 150, 30));
        label_nom2->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_nom2 = new QLineEdit(groupBox_gestion);
        le_nom2->setObjectName("le_nom2");
        le_nom2->setGeometry(QRect(200, 170, 200, 30));
        le_nom2->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_adresse1 = new QLabel(groupBox_gestion);
        label_adresse1->setObjectName("label_adresse1");
        label_adresse1->setGeometry(QRect(50, 240, 150, 30));
        label_adresse1->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_adresse1 = new QLineEdit(groupBox_gestion);
        le_adresse1->setObjectName("le_adresse1");
        le_adresse1->setGeometry(QRect(200, 240, 200, 30));
        le_adresse1->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_email2 = new QLabel(groupBox_gestion);
        label_email2->setObjectName("label_email2");
        label_email2->setGeometry(QRect(50, 310, 150, 30));
        label_email2->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_email2 = new QLineEdit(groupBox_gestion);
        le_email2->setObjectName("le_email2");
        le_email2->setGeometry(QRect(200, 310, 200, 30));
        le_email2->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_product_type = new QLabel(groupBox_gestion);
        label_product_type->setObjectName("label_product_type");
        label_product_type->setGeometry(QRect(450, 310, 150, 30));
        label_product_type->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_product_type = new QLineEdit(groupBox_gestion);
        le_product_type->setObjectName("le_product_type");
        le_product_type->setGeometry(QRect(600, 310, 200, 30));
        le_product_type->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_type = new QLabel(groupBox_gestion);
        label_type->setObjectName("label_type");
        label_type->setGeometry(QRect(450, 100, 150, 30));
        label_type->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_type = new QLineEdit(groupBox_gestion);
        le_type->setObjectName("le_type");
        le_type->setGeometry(QRect(600, 100, 200, 30));
        le_type->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_cp = new QLabel(groupBox_gestion);
        label_cp->setObjectName("label_cp");
        label_cp->setGeometry(QRect(450, 170, 150, 30));
        label_cp->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        sb_cp = new QSpinBox(groupBox_gestion);
        sb_cp->setObjectName("sb_cp");
        sb_cp->setGeometry(QRect(600, 170, 200, 30));
        sb_cp->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_tel1 = new QLabel(groupBox_gestion);
        label_tel1->setObjectName("label_tel1");
        label_tel1->setGeometry(QRect(450, 240, 160, 30));
        label_tel1->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_tel1 = new QLineEdit(groupBox_gestion);
        le_tel1->setObjectName("le_tel1");
        le_tel1->setGeometry(QRect(600, 240, 200, 30));
        le_tel1->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        lbl_image_preview = new QLabel(groupBox_gestion);
        lbl_image_preview->setObjectName("lbl_image_preview");
        lbl_image_preview->setGeometry(QRect(870, 80, 241, 201));
        lbl_image_preview->setStyleSheet(QString::fromUtf8("border: 2px dashed #8B6F47; border-radius: 10px; background: transparent;\n"
""));
        lbl_image_preview->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_upload_image = new QPushButton(groupBox_gestion);
        btn_upload_image->setObjectName("btn_upload_image");
        btn_upload_image->setGeometry(QRect(920, 300, 150, 30));
        btn_upload_image->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        btn_add2 = new QPushButton(groupBox_gestion);
        btn_add2->setObjectName("btn_add2");
        btn_add2->setGeometry(QRect(160, 500, 130, 40));
        btn_add2->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"        "));
        btn_add2->setIcon(icon);
        btn_modify1 = new QPushButton(groupBox_gestion);
        btn_modify1->setObjectName("btn_modify1");
        btn_modify1->setGeometry(QRect(400, 500, 130, 40));
        btn_modify1->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"        "));
        btn_modify1->setIcon(icon1);
        btn_delete1 = new QPushButton(groupBox_gestion);
        btn_delete1->setObjectName("btn_delete1");
        btn_delete1->setGeometry(QRect(640, 500, 130, 40));
        btn_delete1->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"        "));
        btn_delete1->setIcon(icon2);
        btn_delete1->setIconSize(QSize(24, 24));
        btn_cancel_gestion = new QPushButton(groupBox_gestion);
        btn_cancel_gestion->setObjectName("btn_cancel_gestion");
        btn_cancel_gestion->setGeometry(QRect(890, 500, 130, 40));
        btn_cancel_gestion->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"        "));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/assets/icon_cancel.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_cancel_gestion->setIcon(icon3);
        label_sms = new QLabel(groupBox_gestion);
        label_sms->setObjectName("label_sms");
        label_sms->setGeometry(QRect(50, 380, 150, 30));
        label_sms->setStyleSheet(QString::fromUtf8("color: #333; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        txt_sms = new QTextEdit(groupBox_gestion);
        txt_sms->setObjectName("txt_sms");
        txt_sms->setGeometry(QRect(200, 380, 500, 80));
        txt_sms->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        btn_send_sms = new QPushButton(groupBox_gestion);
        btn_send_sms->setObjectName("btn_send_sms");
        btn_send_sms->setGeometry(QRect(720, 400, 150, 40));
        btn_send_sms->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"       "));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/assets/icon_sms.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_send_sms->setIcon(icon4);
        btn_help_gestion_supplier = new QToolButton(groupBox_gestion);
        btn_help_gestion_supplier->setObjectName("btn_help_gestion_supplier");
        btn_help_gestion_supplier->setGeometry(QRect(1120, 20, 30, 30));
        btn_help_gestion_supplier->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_gestion_supplier->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_gestion_supplier->setCheckable(true);
        lbl_hint_gestion_supplier = new QLabel(groupBox_gestion);
        lbl_hint_gestion_supplier->setObjectName("lbl_hint_gestion_supplier");
        lbl_hint_gestion_supplier->setGeometry(QRect(160, 550, 800, 60));
        lbl_hint_gestion_supplier->setVisible(false);
        lbl_hint_gestion_supplier->setStyleSheet(QString::fromUtf8("color: #555; font-size: 13px; font-style: italic; background: rgba(139, 111, 71, 0.1); padding: 10px; border-radius: 5px;"));
        lbl_hint_gestion_supplier->setWordWrap(true);
        tabWidget2->addTab(tab_gestion, QString());
        tab_stats1 = new QWidget();
        tab_stats1->setObjectName("tab_stats1");
        lbl_stats_title = new QLabel(tab_stats1);
        lbl_stats_title->setObjectName("lbl_stats_title");
        lbl_stats_title->setGeometry(QRect(20, 10, 400, 40));
        lbl_stats_title->setStyleSheet(QString::fromUtf8("font-size: 26px; font-weight: bold; color: #333; background: transparent;"));
        btn_help_stats_supplier = new QToolButton(tab_stats1);
        btn_help_stats_supplier->setObjectName("btn_help_stats_supplier");
        btn_help_stats_supplier->setGeometry(QRect(1120, 15, 30, 30));
        btn_help_stats_supplier->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }"));
        btn_help_stats_supplier->setCheckable(true);
        lbl_hint_stats_supplier = new QLabel(tab_stats1);
        lbl_hint_stats_supplier->setObjectName("lbl_hint_stats_supplier");
        lbl_hint_stats_supplier->setGeometry(QRect(450, 10, 600, 40));
        lbl_hint_stats_supplier->setVisible(false);
        lbl_hint_stats_supplier->setStyleSheet(QString::fromUtf8("color: #8B6F47; font-weight: bold; font-size: 11px;"));
        frame_circle_retention = new QFrame(tab_stats1);
        frame_circle_retention->setObjectName("frame_circle_retention");
        frame_circle_retention->setGeometry(QRect(30, 60, 120, 120));
        frame_circle_retention->setStyleSheet(QString::fromUtf8("QFrame { border: 4px solid #4CAF50; border-radius: 60px; background-color: white; }"));
        lbl_percent_retention = new QLabel(frame_circle_retention);
        lbl_percent_retention->setObjectName("lbl_percent_retention");
        lbl_percent_retention->setGeometry(QRect(10, 30, 100, 40));
        lbl_percent_retention->setStyleSheet(QString::fromUtf8("border: none; font-size: 24px; font-weight: bold; color: #4CAF50;"));
        lbl_percent_retention->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_text_retention = new QLabel(frame_circle_retention);
        lbl_text_retention->setObjectName("lbl_text_retention");
        lbl_text_retention->setGeometry(QRect(10, 70, 100, 20));
        lbl_text_retention->setStyleSheet(QString::fromUtf8("border: none; font-size: 10px; color: #555; font-weight: bold;"));
        lbl_text_retention->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frame_circle_accuracy = new QFrame(tab_stats1);
        frame_circle_accuracy->setObjectName("frame_circle_accuracy");
        frame_circle_accuracy->setGeometry(QRect(180, 60, 120, 120));
        frame_circle_accuracy->setStyleSheet(QString::fromUtf8("QFrame { border: 4px solid #2196F3; border-radius: 60px; background-color: white; }"));
        lbl_percent_accuracy = new QLabel(frame_circle_accuracy);
        lbl_percent_accuracy->setObjectName("lbl_percent_accuracy");
        lbl_percent_accuracy->setGeometry(QRect(10, 30, 100, 40));
        lbl_percent_accuracy->setStyleSheet(QString::fromUtf8("border: none; font-size: 24px; font-weight: bold; color: #2196F3;"));
        lbl_percent_accuracy->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_text_accuracy = new QLabel(frame_circle_accuracy);
        lbl_text_accuracy->setObjectName("lbl_text_accuracy");
        lbl_text_accuracy->setGeometry(QRect(10, 70, 100, 20));
        lbl_text_accuracy->setStyleSheet(QString::fromUtf8("border: none; font-size: 10px; color: #555; font-weight: bold;"));
        lbl_text_accuracy->setAlignment(Qt::AlignmentFlag::AlignCenter);
        group_performance_bars = new QGroupBox(tab_stats1);
        group_performance_bars->setObjectName("group_performance_bars");
        group_performance_bars->setGeometry(QRect(330, 50, 400, 140));
        group_performance_bars->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"      QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"   "
                        "     }\n"
"       "));
        lbl_bar_quality = new QLabel(group_performance_bars);
        lbl_bar_quality->setObjectName("lbl_bar_quality");
        lbl_bar_quality->setGeometry(QRect(10, 35, 150, 20));
        pb_quality = new QProgressBar(group_performance_bars);
        pb_quality->setObjectName("pb_quality");
        pb_quality->setGeometry(QRect(160, 35, 220, 20));
        pb_quality->setStyleSheet(QString::fromUtf8("QProgressBar { border: 1px solid #BBB; border-radius: 5px; text-align: center; }\n"
"       QProgressBar::chunk { background-color: #8B6F47; width: 20px; }"));
        pb_quality->setValue(95);
        lbl_bar_speed = new QLabel(group_performance_bars);
        lbl_bar_speed->setObjectName("lbl_bar_speed");
        lbl_bar_speed->setGeometry(QRect(10, 75, 150, 20));
        pb_speed = new QProgressBar(group_performance_bars);
        pb_speed->setObjectName("pb_speed");
        pb_speed->setGeometry(QRect(160, 75, 220, 20));
        pb_speed->setStyleSheet(QString::fromUtf8("QProgressBar { border: 1px solid #BBB; border-radius: 5px; text-align: center; }\n"
"       QProgressBar::chunk { background-color: #FF9800; width: 20px; }"));
        pb_speed->setValue(82);
        frame_stat_summary = new QFrame(tab_stats1);
        frame_stat_summary->setObjectName("frame_stat_summary");
        frame_stat_summary->setGeometry(QRect(750, 60, 380, 120));
        frame_stat_summary->setStyleSheet(QString::fromUtf8("background-color: #8B6F47; border-radius: 10px; color: white;"));
        frame_stat_summary->setFrameShape(QFrame::Shape::StyledPanel);
        frame_stat_summary->setFrameShadow(QFrame::Shadow::Raised);
        lbl_summary_title = new QLabel(frame_stat_summary);
        lbl_summary_title->setObjectName("lbl_summary_title");
        lbl_summary_title->setGeometry(QRect(20, 15, 200, 20));
        lbl_summary_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold;"));
        lbl_summary_val = new QLabel(frame_stat_summary);
        lbl_summary_val->setObjectName("lbl_summary_val");
        lbl_summary_val->setGeometry(QRect(20, 45, 340, 50));
        lbl_summary_val->setStyleSheet(QString::fromUtf8("font-size: 13px;"));
        frame_chart_types = new QFrame(tab_stats1);
        frame_chart_types->setObjectName("frame_chart_types");
        frame_chart_types->setGeometry(QRect(20, 210, 550, 350));
        frame_chart_types->setStyleSheet(QString::fromUtf8("background-color: white; border-radius: 10px; border: 1px solid #8B6F47;"));
        lbl_chart_types = new QLabel(frame_chart_types);
        lbl_chart_types->setObjectName("lbl_chart_types");
        lbl_chart_types->setGeometry(QRect(0, 0, 550, 40));
        lbl_chart_types->setStyleSheet(QString::fromUtf8("border: none; font-size: 18px; font-weight: bold; color: #555; padding: 5px;"));
        lbl_chart_types->setAlignment(Qt::AlignmentFlag::AlignCenter);
        chart_types_view = new QGraphicsView(frame_chart_types);
        chart_types_view->setObjectName("chart_types_view");
        chart_types_view->setGeometry(QRect(10, 50, 530, 280));
        chart_types_view->setStyleSheet(QString::fromUtf8("background: rgba(139, 111, 71, 0.05); border: 1px dashed #DDD;"));
        frame_chart_reviews = new QFrame(tab_stats1);
        frame_chart_reviews->setObjectName("frame_chart_reviews");
        frame_chart_reviews->setGeometry(QRect(580, 210, 550, 350));
        frame_chart_reviews->setStyleSheet(QString::fromUtf8("background-color: white; border-radius: 10px; border: 1px solid #8B6F47;"));
        lbl_chart_reviews = new QLabel(frame_chart_reviews);
        lbl_chart_reviews->setObjectName("lbl_chart_reviews");
        lbl_chart_reviews->setGeometry(QRect(0, 0, 550, 40));
        lbl_chart_reviews->setStyleSheet(QString::fromUtf8("border: none; font-size: 18px; font-weight: bold; color: #555; padding: 5px;"));
        lbl_chart_reviews->setAlignment(Qt::AlignmentFlag::AlignCenter);
        chart_reviews_view = new QGraphicsView(frame_chart_reviews);
        chart_reviews_view->setObjectName("chart_reviews_view");
        chart_reviews_view->setGeometry(QRect(10, 50, 530, 280));
        chart_reviews_view->setStyleSheet(QString::fromUtf8("background: rgba(139, 111, 71, 0.05); border: 1px dashed #DDD;"));
        group_regional_stats = new QGroupBox(tab_stats1);
        group_regional_stats->setObjectName("group_regional_stats");
        group_regional_stats->setGeometry(QRect(20, 580, 1111, 180));
        group_regional_stats->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"      QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
""
                        "        }\n"
"       "));
        lbl_reg_1 = new QLabel(group_regional_stats);
        lbl_reg_1->setObjectName("lbl_reg_1");
        lbl_reg_1->setGeometry(QRect(20, 40, 100, 20));
        pb_reg_1 = new QProgressBar(group_regional_stats);
        pb_reg_1->setObjectName("pb_reg_1");
        pb_reg_1->setGeometry(QRect(130, 40, 380, 15));
        pb_reg_1->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #4CAF50; }"));
        pb_reg_1->setValue(88);
        lbl_reg_2 = new QLabel(group_regional_stats);
        lbl_reg_2->setObjectName("lbl_reg_2");
        lbl_reg_2->setGeometry(QRect(20, 80, 100, 20));
        pb_reg_2 = new QProgressBar(group_regional_stats);
        pb_reg_2->setObjectName("pb_reg_2");
        pb_reg_2->setGeometry(QRect(130, 80, 380, 15));
        pb_reg_2->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #FFC107; }"));
        pb_reg_2->setValue(72);
        lbl_top_performer = new QLabel(group_regional_stats);
        lbl_top_performer->setObjectName("lbl_top_performer");
        lbl_top_performer->setGeometry(QRect(600, 40, 480, 80));
        lbl_top_performer->setStyleSheet(QString::fromUtf8("font-size: 16px; color: #8B6F47; font-weight: bold; border-left: 3px solid #8B6F47; padding-left: 20px;"));
        tabWidget2->addTab(tab_stats1, QString());
        tab_view2 = new QWidget();
        tab_view2->setObjectName("tab_view2");
        tab_view2->setStyleSheet(QString::fromUtf8("\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"     "));
        lbl_title_view = new QLabel(tab_view2);
        lbl_title_view->setObjectName("lbl_title_view");
        lbl_title_view->setGeometry(QRect(380, 20, 400, 40));
        lbl_title_view->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #333; background: white; border-radius: 5px; padding: 5px;\n"
""));
        lbl_title_view->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_chercher = new QPushButton(tab_view2);
        btn_chercher->setObjectName("btn_chercher");
        btn_chercher->setGeometry(QRect(150, 100, 150, 40));
        btn_chercher->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-top-left-radius: 10px; border-bottom-left-radius: 10px; color: white; font-weight: bold; border: none; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        QPushButton:hover { background-color: #C19DAF; }\n"
"       "));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/assets/icon_search.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_chercher->setIcon(icon5);
        le_recherche1 = new QLineEdit(tab_view2);
        le_recherche1->setObjectName("le_recherche1");
        le_recherche1->setGeometry(QRect(300, 100, 600, 40));
        le_recherche1->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-top-right-radius: 10px; border-bottom-right-radius: 10px; padding: 5px; background: white;\n"
""));
        tableView1 = new QTableView(tab_view2);
        tableView1->setObjectName("tableView1");
        tableView1->setGeometry(QRect(100, 220, 911, 411));
        tableView1->setStyleSheet(QString::fromUtf8("\n"
"       QTableView { background-color: rgba(255,255,255,0.9); border-radius: 10px; gridline-color: #8B6F47; }\n"
"       QHeaderView::section { background-color: #8B6F47; color: #333; font-weight: bold; border: none; padding: 5px; }\n"
"            "));
        tabWidget2->addTab(tab_view2, QString());
        tab_reviews = new QWidget();
        tab_reviews->setObjectName("tab_reviews");
        lbl_reviews_title = new QLabel(tab_reviews);
        lbl_reviews_title->setObjectName("lbl_reviews_title");
        lbl_reviews_title->setGeometry(QRect(20, 15, 300, 40));
        lbl_reviews_title->setStyleSheet(QString::fromUtf8("font-size: 26px; font-weight: bold; color: #333; background: transparent;"));
        btn_help_reviews_supplier = new QToolButton(tab_reviews);
        btn_help_reviews_supplier->setObjectName("btn_help_reviews_supplier");
        btn_help_reviews_supplier->setGeometry(QRect(1120, 15, 30, 30));
        btn_help_reviews_supplier->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"      QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"));
        btn_help_reviews_supplier->setCheckable(true);
        lbl_hint_reviews_supplier = new QLabel(tab_reviews);
        lbl_hint_reviews_supplier->setObjectName("lbl_hint_reviews_supplier");
        lbl_hint_reviews_supplier->setGeometry(QRect(360, 15, 700, 40));
        lbl_hint_reviews_supplier->setVisible(false);
        lbl_hint_reviews_supplier->setStyleSheet(QString::fromUtf8("color: #8B6F47; font-weight: bold; font-size: 11px;"));
        lbl_select_supplier = new QLabel(tab_reviews);
        lbl_select_supplier->setObjectName("lbl_select_supplier");
        lbl_select_supplier->setGeometry(QRect(30, 70, 200, 30));
        lbl_select_supplier->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; color: #555;"));
        cb_supplier_reviews = new QComboBox(tab_reviews);
        cb_supplier_reviews->setObjectName("cb_supplier_reviews");
        cb_supplier_reviews->setGeometry(QRect(30, 105, 300, 35));
        cb_supplier_reviews->setStyleSheet(QString::fromUtf8("border: 2px solid #8B6F47; border-radius: 8px; padding: 5px; background: white;"));
        frame_rating_header = new QFrame(tab_reviews);
        frame_rating_header->setObjectName("frame_rating_header");
        frame_rating_header->setGeometry(QRect(360, 70, 350, 120));
        frame_rating_header->setStyleSheet(QString::fromUtf8("background-color: #8B6F47; border-radius: 12px; color: white;"));
        lbl_avg_score = new QLabel(frame_rating_header);
        lbl_avg_score->setObjectName("lbl_avg_score");
        lbl_avg_score->setGeometry(QRect(20, 20, 100, 50));
        lbl_avg_score->setStyleSheet(QString::fromUtf8("font-size: 42px; font-weight: bold; border: none;"));
        lbl_stars_row = new QLabel(frame_rating_header);
        lbl_stars_row->setObjectName("lbl_stars_row");
        lbl_stars_row->setGeometry(QRect(130, 30, 200, 30));
        lbl_stars_row->setStyleSheet(QString::fromUtf8("font-size: 24px; color: #FFC107; border: none;"));
        lbl_review_count = new QLabel(frame_rating_header);
        lbl_review_count->setObjectName("lbl_review_count");
        lbl_review_count->setGeometry(QRect(20, 75, 310, 30));
        lbl_review_count->setStyleSheet(QString::fromUtf8("font-size: 14px; opacity: 0.9; border: none;"));
        group_feedback_breakdown = new QGroupBox(tab_reviews);
        group_feedback_breakdown->setObjectName("group_feedback_breakdown");
        group_feedback_breakdown->setGeometry(QRect(730, 60, 421, 140));
        group_feedback_breakdown->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"      QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
""
                        "        }\n"
"       "));
        lbl_cat_quality = new QLabel(group_feedback_breakdown);
        lbl_cat_quality->setObjectName("lbl_cat_quality");
        lbl_cat_quality->setGeometry(QRect(10, 30, 120, 20));
        pb_review_quality = new QProgressBar(group_feedback_breakdown);
        pb_review_quality->setObjectName("pb_review_quality");
        pb_review_quality->setGeometry(QRect(140, 30, 260, 15));
        pb_review_quality->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #4CAF50; }"));
        pb_review_quality->setValue(98);
        lbl_cat_response = new QLabel(group_feedback_breakdown);
        lbl_cat_response->setObjectName("lbl_cat_response");
        lbl_cat_response->setGeometry(QRect(10, 65, 120, 20));
        pb_review_response = new QProgressBar(group_feedback_breakdown);
        pb_review_response->setObjectName("pb_review_response");
        pb_review_response->setGeometry(QRect(140, 65, 260, 15));
        pb_review_response->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #2196F3; }"));
        pb_review_response->setValue(85);
        lbl_cat_price = new QLabel(group_feedback_breakdown);
        lbl_cat_price->setObjectName("lbl_cat_price");
        lbl_cat_price->setGeometry(QRect(10, 100, 120, 20));
        pb_review_price = new QProgressBar(group_feedback_breakdown);
        pb_review_price->setObjectName("pb_review_price");
        pb_review_price->setGeometry(QRect(140, 100, 260, 15));
        pb_review_price->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #9C27B0; }"));
        pb_review_price->setValue(90);
        frame_distribution = new QFrame(tab_reviews);
        frame_distribution->setObjectName("frame_distribution");
        frame_distribution->setGeometry(QRect(30, 210, 300, 521));
        frame_distribution->setStyleSheet(QString::fromUtf8("background: rgba(255, 255, 255, 0.8); border: 1px solid #DDD; border-radius: 8px;"));
        lbl_dist_title = new QLabel(frame_distribution);
        lbl_dist_title->setObjectName("lbl_dist_title");
        lbl_dist_title->setGeometry(QRect(0, 10, 300, 30));
        lbl_dist_title->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #555; border: none;"));
        lbl_dist_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_row_5 = new QLabel(frame_distribution);
        lbl_row_5->setObjectName("lbl_row_5");
        lbl_row_5->setGeometry(QRect(10, 50, 50, 20));
        pb_dist_5 = new QProgressBar(frame_distribution);
        pb_dist_5->setObjectName("pb_dist_5");
        pb_dist_5->setGeometry(QRect(60, 50, 220, 20));
        pb_dist_5->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #FFC107; }"));
        pb_dist_5->setValue(80);
        lbl_row_4 = new QLabel(frame_distribution);
        lbl_row_4->setObjectName("lbl_row_4");
        lbl_row_4->setGeometry(QRect(10, 80, 50, 20));
        pb_dist_4 = new QProgressBar(frame_distribution);
        pb_dist_4->setObjectName("pb_dist_4");
        pb_dist_4->setGeometry(QRect(60, 80, 220, 20));
        pb_dist_4->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #FFC107; }"));
        pb_dist_4->setValue(15);
        lbl_row_3 = new QLabel(frame_distribution);
        lbl_row_3->setObjectName("lbl_row_3");
        lbl_row_3->setGeometry(QRect(10, 110, 50, 20));
        pb_dist_3 = new QProgressBar(frame_distribution);
        pb_dist_3->setObjectName("pb_dist_3");
        pb_dist_3->setGeometry(QRect(60, 110, 220, 20));
        pb_dist_3->setStyleSheet(QString::fromUtf8("QProgressBar::chunk { background-color: #FFC107; }"));
        pb_dist_3->setValue(4);
        lbl_dist_insight = new QLabel(frame_distribution);
        lbl_dist_insight->setObjectName("lbl_dist_insight");
        lbl_dist_insight->setGeometry(QRect(10, 450, 280, 60));
        lbl_dist_insight->setStyleSheet(QString::fromUtf8("color: #4CAF50; font-style: italic; border: none;"));
        lbl_dist_insight->setWordWrap(true);
        table_reviews = new QTableView(tab_reviews);
        table_reviews->setObjectName("table_reviews");
        table_reviews->setGeometry(QRect(350, 210, 801, 521));
        table_reviews->setStyleSheet(QString::fromUtf8("\n"
"       QTableView { background-color: white; border: 1px solid #DDD; border-radius: 8px; gridline-color: #EEE; }\n"
"       QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 10px; }\n"
"      "));
        tabWidget2->addTab(tab_reviews, QString());
        stackedWidget->addWidget(page_supplier);
        page_equipment = new QWidget();
        page_equipment->setObjectName("page_equipment");
        page_equipment->setStyleSheet(QString::fromUtf8("\n"
"    #page_equipment {\n"
"        border-image: url(:/assets/background.png) 0 0 0 0 stretch stretch;\n"
"    }\n"
"    QPushButton {\n"
"        border-image: url(:/assets/button_bg.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"    }\n"
"    QTabWidget::pane {\n"
"        border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch;\n"
"    }\n"
"   "));
        tabWidget3 = new QTabWidget(page_equipment);
        tabWidget3->setObjectName("tabWidget3");
        tabWidget3->setGeometry(QRect(10, 20, 1261, 691));
        tabWidget3->setFont(font2);
        tabWidget3->setStyleSheet(QString::fromUtf8("\n"
"     QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"     QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"     QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"     QWidget#tab_gestion, QWidget#tab_view, QWidget#tab_stats, QWidget#tab_history { background: transparent; }\n"
"    "));
        tab_gestion1 = new QWidget();
        tab_gestion1->setObjectName("tab_gestion1");
        groupBox_gestion1 = new QGroupBox(tab_gestion1);
        groupBox_gestion1->setObjectName("groupBox_gestion1");
        groupBox_gestion1->setGeometry(QRect(20, 20, 1200, 800));
        groupBox_gestion1->setStyleSheet(QString::fromUtf8("\n"
"       QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"       QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background"
                        ": #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"       QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"      "));
        label_id3 = new QLabel(groupBox_gestion1);
        label_id3->setObjectName("label_id3");
        label_id3->setGeometry(QRect(50, 100, 150, 30));
        label_id3->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_id3 = new QLineEdit(groupBox_gestion1);
        le_id3->setObjectName("le_id3");
        le_id3->setGeometry(QRect(200, 100, 200, 30));
        le_id3->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_date_achat = new QLabel(groupBox_gestion1);
        label_date_achat->setObjectName("label_date_achat");
        label_date_achat->setGeometry(QRect(50, 170, 150, 30));
        label_date_achat->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        de_date_achat = new QDateEdit(groupBox_gestion1);
        de_date_achat->setObjectName("de_date_achat");
        de_date_achat->setGeometry(QRect(200, 170, 200, 30));
        de_date_achat->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        label_etat = new QLabel(groupBox_gestion1);
        label_etat->setObjectName("label_etat");
        label_etat->setGeometry(QRect(450, 100, 150, 30));
        label_etat->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        rb_intact = new QRadioButton(groupBox_gestion1);
        bg_equipment_status_main = new QButtonGroup(MainWindow);
        bg_equipment_status_main->setObjectName("bg_equipment_status_main");
        bg_equipment_status_main->addButton(rb_intact);
        rb_intact->setObjectName("rb_intact");
        rb_intact->setGeometry(QRect(600, 100, 100, 30));
        rb_intact->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        rb_broken = new QRadioButton(groupBox_gestion1);
        bg_equipment_status_main->addButton(rb_broken);
        rb_broken->setObjectName("rb_broken");
        rb_broken->setGeometry(QRect(760, 100, 100, 30));
        rb_broken->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;\n"
""));
        label_desc = new QLabel(groupBox_gestion1);
        label_desc->setObjectName("label_desc");
        label_desc->setGeometry(QRect(450, 210, 150, 30));
        label_desc->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        te_desc = new QTextEdit(groupBox_gestion1);
        te_desc->setObjectName("te_desc");
        te_desc->setGeometry(QRect(590, 220, 400, 250));
        te_desc->setStyleSheet(QString::fromUtf8(" background-color: white;\n"
""));
        btn_add3 = new QPushButton(groupBox_gestion1);
        btn_add3->setObjectName("btn_add3");
        btn_add3->setGeometry(QRect(110, 500, 130, 40));
        btn_add3->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; padding-left: 10px; text-align: left; }\n"
"                    QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"       "));
        btn_add3->setIcon(icon);
        btn_add3->setIconSize(QSize(24, 24));
        btn_modify2 = new QPushButton(groupBox_gestion1);
        btn_modify2->setObjectName("btn_modify2");
        btn_modify2->setGeometry(QRect(340, 500, 130, 40));
        btn_modify2->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; padding-left: 10px; text-align: left; }\n"
"                    QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"       "));
        btn_modify2->setIcon(icon1);
        btn_modify2->setIconSize(QSize(24, 24));
        btn_delete2 = new QPushButton(groupBox_gestion1);
        btn_delete2->setObjectName("btn_delete2");
        btn_delete2->setGeometry(QRect(580, 500, 130, 40));
        btn_delete2->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"       "));
        btn_delete2->setIcon(icon2);
        btn_delete2->setIconSize(QSize(24, 24));
        btn_cancel_gestion1 = new QPushButton(groupBox_gestion1);
        btn_cancel_gestion1->setObjectName("btn_cancel_gestion1");
        btn_cancel_gestion1->setGeometry(QRect(830, 500, 130, 40));
        btn_cancel_gestion1->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: transparent; border-radius: 10px; color: #333; font-weight: bold; border: none; }\n"
"          QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
"       "));
        btn_help_gestion_equipment = new QToolButton(groupBox_gestion1);
        btn_help_gestion_equipment->setObjectName("btn_help_gestion_equipment");
        btn_help_gestion_equipment->setGeometry(QRect(1140, 20, 30, 30));
        btn_help_gestion_equipment->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_gestion_equipment->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_gestion_equipment->setCheckable(true);
        lbl_hint_gestion_equipment = new QLabel(groupBox_gestion1);
        lbl_hint_gestion_equipment->setObjectName("lbl_hint_gestion_equipment");
        lbl_hint_gestion_equipment->setGeometry(QRect(110, 560, 800, 60));
        lbl_hint_gestion_equipment->setVisible(false);
        lbl_hint_gestion_equipment->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        tabWidget3->addTab(tab_gestion1, QString());
        tab_view3 = new QWidget();
        tab_view3->setObjectName("tab_view3");
        tab_view3->setStyleSheet(QString::fromUtf8("\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"     "));
        label_titre_liste_equip = new QLabel(tab_view3);
        label_titre_liste_equip->setObjectName("label_titre_liste_equip");
        label_titre_liste_equip->setGeometry(QRect(450, 20, 300, 30));
        label_titre_liste_equip->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;\n"
""));
        label_titre_liste_equip->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_search_top = new QPushButton(tab_view3);
        btn_search_top->setObjectName("btn_search_top");
        btn_search_top->setGeometry(QRect(200, 80, 150, 30));
        btn_search_top->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
""));
        le_recherche2 = new QLineEdit(tab_view3);
        le_recherche2->setObjectName("le_recherche2");
        le_recherche2->setGeometry(QRect(370, 80, 500, 30));
        le_recherche2->setStyleSheet(QString::fromUtf8(""));
        table_equipments = new QTableView(tab_view3);
        table_equipments->setObjectName("table_equipments");
        table_equipments->setGeometry(QRect(20, 130, 1000, 400));
        btn_search1 = new QPushButton(tab_view3);
        btn_search1->setObjectName("btn_search1");
        btn_search1->setGeometry(QRect(220, 560, 120, 40));
        btn_search1->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
""));
        btn_search1->setIcon(icon5);
        btn_delete_confirm = new QPushButton(tab_view3);
        btn_delete_confirm->setObjectName("btn_delete_confirm");
        btn_delete_confirm->setGeometry(QRect(600, 560, 150, 40));
        btn_delete_confirm->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }\n"
""));
        btn_delete_confirm->setIcon(icon2);
        btn_delete_confirm->setIconSize(QSize(24, 24));
        tabWidget3->addTab(tab_view3, QString());
        tab_history = new QWidget();
        tab_history->setObjectName("tab_history");
        tab_history->setStyleSheet(QString::fromUtf8("\n"
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
        label_titre_histo = new QLabel(tab_history);
        label_titre_histo->setObjectName("label_titre_histo");
        label_titre_histo->setGeometry(QRect(350, 20, 300, 30));
        label_titre_histo->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;"));
        label_titre_histo->setAlignment(Qt::AlignmentFlag::AlignCenter);
        le_history_search = new QLineEdit(tab_history);
        le_history_search->setObjectName("le_history_search");
        le_history_search->setGeometry(QRect(20, 70, 300, 30));
        btn_history_search = new QPushButton(tab_history);
        btn_history_search->setObjectName("btn_history_search");
        btn_history_search->setGeometry(QRect(330, 70, 100, 30));
        btn_history_search->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 5px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        tableView_historique = new QTableView(tab_history);
        tableView_historique->setObjectName("tableView_historique");
        tableView_historique->setGeometry(QRect(20, 120, 1000, 450));
        tableView_historique->setStyleSheet(QString::fromUtf8("\n"
"       QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 5px; }\n"
"       QTableView { border: 1px solid #8B6F47; selection-background-color: #E0E0E0; selection-color: #333; background-color: white; }\n"
"      "));
        btn_refresh_history = new QPushButton(tab_history);
        btn_refresh_history->setObjectName("btn_refresh_history");
        btn_refresh_history->setGeometry(QRect(20, 580, 150, 40));
        btn_refresh_history->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        btn_export_history = new QPushButton(tab_history);
        btn_export_history->setObjectName("btn_export_history");
        btn_export_history->setGeometry(QRect(180, 580, 150, 40));
        btn_export_history->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        btn_clear_history = new QPushButton(tab_history);
        btn_clear_history->setObjectName("btn_clear_history");
        btn_clear_history->setGeometry(QRect(870, 580, 150, 40));
        btn_clear_history->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #f44336; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); }"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/assets/clear.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_clear_history->setIcon(icon6);
        btn_clear_history->setIconSize(QSize(24, 24));
        tabWidget3->addTab(tab_history, QString());
        tab_stats2 = new QWidget();
        tab_stats2->setObjectName("tab_stats2");
        label_stats_title = new QLabel(tab_stats2);
        label_stats_title->setObjectName("label_stats_title");
        label_stats_title->setGeometry(QRect(350, 20, 350, 40));
        label_stats_title->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: white;"));
        label_stats_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        group_metrics = new QGroupBox(tab_stats2);
        group_metrics->setObjectName("group_metrics");
        group_metrics->setGeometry(QRect(50, 80, 450, 250));
        group_metrics->setStyleSheet(QString::fromUtf8("\n"
"                QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"                QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"    "
                        "        background: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"                QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"            "));
        lbl_total_eq = new QLabel(group_metrics);
        lbl_total_eq->setObjectName("lbl_total_eq");
        lbl_total_eq->setGeometry(QRect(20, 50, 250, 30));
        lbl_total_eq->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px;"));
        val_total_eq = new QLabel(group_metrics);
        val_total_eq->setObjectName("val_total_eq");
        val_total_eq->setGeometry(QRect(300, 50, 100, 30));
        val_total_eq->setStyleSheet(QString::fromUtf8("color: #8B6F47; font-size: 18px; font-weight: bold;"));
        lbl_oper_eq = new QLabel(group_metrics);
        lbl_oper_eq->setObjectName("lbl_oper_eq");
        lbl_oper_eq->setGeometry(QRect(20, 100, 250, 30));
        lbl_oper_eq->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px;"));
        val_oper_eq = new QLabel(group_metrics);
        val_oper_eq->setObjectName("val_oper_eq");
        val_oper_eq->setGeometry(QRect(300, 100, 100, 30));
        val_oper_eq->setStyleSheet(QString::fromUtf8("color: #4CAF50; font-size: 18px; font-weight: bold;"));
        lbl_broken_eq = new QLabel(group_metrics);
        lbl_broken_eq->setObjectName("lbl_broken_eq");
        lbl_broken_eq->setGeometry(QRect(20, 150, 250, 30));
        lbl_broken_eq->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px;"));
        val_broken_eq = new QLabel(group_metrics);
        val_broken_eq->setObjectName("val_broken_eq");
        val_broken_eq->setGeometry(QRect(300, 150, 100, 30));
        val_broken_eq->setStyleSheet(QString::fromUtf8("color: #f44336; font-size: 18px; font-weight: bold;"));
        group_chart = new QGroupBox(tab_stats2);
        group_chart->setObjectName("group_chart");
        group_chart->setGeometry(QRect(550, 80, 450, 250));
        group_chart->setStyleSheet(QString::fromUtf8("\n"
"                QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"                QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"    "
                        "        background: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"                QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"            "));
        label_chart_placeholder = new QLabel(group_chart);
        label_chart_placeholder->setObjectName("label_chart_placeholder");
        label_chart_placeholder->setGeometry(QRect(20, 40, 410, 190));
        label_chart_placeholder->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0.1); border: 1px dashed #DDD;"));
        label_chart_placeholder->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_help_stats_equipment = new QToolButton(tab_stats2);
        btn_help_stats_equipment->setObjectName("btn_help_stats_equipment");
        btn_help_stats_equipment->setGeometry(QRect(1140, 20, 30, 30));
        btn_help_stats_equipment->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_stats_equipment->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_stats_equipment->setCheckable(true);
        lbl_hint_stats_equipment = new QLabel(tab_stats2);
        lbl_hint_stats_equipment->setObjectName("lbl_hint_stats_equipment");
        lbl_hint_stats_equipment->setGeometry(QRect(50, 350, 950, 60));
        lbl_hint_stats_equipment->setVisible(false);
        lbl_hint_stats_equipment->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        lbl_hint_stats_equipment->setWordWrap(true);
        tabWidget3->addTab(tab_stats2, QString());
        stackedWidget->addWidget(page_equipment);
        page_order = new QWidget();
        page_order->setObjectName("page_order");
        page_order->setStyleSheet(QString::fromUtf8("\n"
"    #page_order {\n"
"        border-image: url(:/assets/background.png) 0 0 0 0 stretch stretch;\n"
"    }\n"
"    QPushButton {\n"
"        border-image: url(:/assets/button_bg.png) 0 0 0 0 stretch stretch;\n"
"        border: none;\n"
"    }\n"
"    QTabWidget::pane {\n"
"        border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch;\n"
"    }\n"
"   "));
        tabWidget4 = new QTabWidget(page_order);
        tabWidget4->setObjectName("tabWidget4");
        tabWidget4->setGeometry(QRect(9, 20, 1161, 760));
        tabWidget4->setFont(font2);
        tabWidget4->setStyleSheet(QString::fromUtf8("\n"
"    QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"    QTabWidget::tab-bar { left: 250px; }\n"
"+    QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"      QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"      QWidget#tab_manage, QWidget#tab_view, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_calendar, QWidget#tab_qrcode, QWidget#tab_catalog { background: transparent; }\n"
"     "));
        tab_manage = new QWidget();
        tab_manage->setObjectName("tab_manage");
        group_manage = new QGroupBox(tab_manage);
        group_manage->setObjectName("group_manage");
        group_manage->setGeometry(QRect(20, 19, 1121, 701));
        group_manage->setStyleSheet(QString::fromUtf8("\n"
"          QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            back"
                        "ground: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"         "));
        label_id4 = new QLabel(group_manage);
        label_id4->setObjectName("label_id4");
        label_id4->setGeometry(QRect(50, 60, 150, 30));
        label_id4->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_id4 = new QLineEdit(group_manage);
        le_id4->setObjectName("le_id4");
        le_id4->setGeometry(QRect(220, 60, 250, 30));
        le_id4->setStyleSheet(QString::fromUtf8(""));
        label_type1 = new QLabel(group_manage);
        label_type1->setObjectName("label_type1");
        label_type1->setGeometry(QRect(50, 110, 150, 30));
        label_type1->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_type1 = new QLineEdit(group_manage);
        le_type1->setObjectName("le_type1");
        le_type1->setGeometry(QRect(220, 110, 250, 30));
        le_type1->setStyleSheet(QString::fromUtf8(""));
        label_stock = new QLabel(group_manage);
        label_stock->setObjectName("label_stock");
        label_stock->setGeometry(QRect(50, 160, 150, 30));
        label_stock->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_stock = new QLineEdit(group_manage);
        le_stock->setObjectName("le_stock");
        le_stock->setGeometry(QRect(220, 160, 250, 30));
        le_stock->setStyleSheet(QString::fromUtf8(""));
        label_prix = new QLabel(group_manage);
        label_prix->setObjectName("label_prix");
        label_prix->setGeometry(QRect(50, 210, 150, 30));
        label_prix->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_prix = new QLineEdit(group_manage);
        le_prix->setObjectName("le_prix");
        le_prix->setGeometry(QRect(220, 210, 250, 30));
        le_prix->setStyleSheet(QString::fromUtf8(""));
        label_buyer = new QLabel(group_manage);
        label_buyer->setObjectName("label_buyer");
        label_buyer->setGeometry(QRect(50, 260, 150, 30));
        label_buyer->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_buyer = new QLineEdit(group_manage);
        le_buyer->setObjectName("le_buyer");
        le_buyer->setGeometry(QRect(220, 260, 250, 30));
        le_buyer->setStyleSheet(QString::fromUtf8(""));
        btn_add4 = new QPushButton(group_manage);
        btn_add4->setObjectName("btn_add4");
        btn_add4->setGeometry(QRect(150, 350, 140, 45));
        btn_add4->setStyleSheet(QString::fromUtf8("\n"
"          QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"          QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        btn_add4->setIcon(icon);
        btn_add4->setIconSize(QSize(24, 24));
        btn_modify3 = new QPushButton(group_manage);
        btn_modify3->setObjectName("btn_modify3");
        btn_modify3->setGeometry(QRect(420, 350, 140, 45));
        btn_modify3->setStyleSheet(QString::fromUtf8("\n"
"          QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"          QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        btn_modify3->setIcon(icon1);
        btn_modify3->setIconSize(QSize(24, 24));
        btn_delete3 = new QPushButton(group_manage);
        btn_delete3->setObjectName("btn_delete3");
        btn_delete3->setGeometry(QRect(690, 350, 140, 45));
        btn_delete3->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        btn_delete3->setIcon(icon2);
        btn_delete3->setIconSize(QSize(24, 24));
        btn_clear = new QPushButton(group_manage);
        btn_clear->setObjectName("btn_clear");
        btn_clear->setGeometry(QRect(280, 420, 140, 45));
        btn_clear->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        btn_clear->setIcon(icon6);
        btn_clear->setIconSize(QSize(24, 24));
        btn_load = new QPushButton(group_manage);
        btn_load->setObjectName("btn_load");
        btn_load->setGeometry(QRect(550, 420, 140, 45));
        btn_load->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        tabWidget4->addTab(tab_manage, QString());
        tab_qrcode = new QWidget();
        tab_qrcode->setObjectName("tab_qrcode");
        group_qr = new QGroupBox(tab_qrcode);
        group_qr->setObjectName("group_qr");
        group_qr->setGeometry(QRect(20, -1, 1121, 721));
        group_qr->setStyleSheet(QString::fromUtf8("\n"
"          QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            back"
                        "ground: #FFFAF0;\n"
"        }\n"
"\n"
"           QLineEdit {\n"
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
"            border: 2px solid #8B4513; \n"
"            background: #FFFAF0;\n"
"        }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"         "));
        label_qr_order_id = new QLabel(group_qr);
        label_qr_order_id->setObjectName("label_qr_order_id");
        label_qr_order_id->setGeometry(QRect(50, 60, 150, 30));
        label_qr_order_id->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;"));
        le_qr_order_id = new QLineEdit(group_qr);
        le_qr_order_id->setObjectName("le_qr_order_id");
        le_qr_order_id->setGeometry(QRect(220, 60, 300, 35));
        le_qr_order_id->setStyleSheet(QString::fromUtf8(" padding: 5px; font-size: 14px;"));
        btn_generate_qr = new QPushButton(group_qr);
        btn_generate_qr->setObjectName("btn_generate_qr");
        btn_generate_qr->setGeometry(QRect(540, 60, 140, 35));
        btn_generate_qr->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        label_qr_display = new QLabel(group_qr);
        label_qr_display->setObjectName("label_qr_display");
        label_qr_display->setGeometry(QRect(200, 130, 300, 300));
        label_qr_display->setStyleSheet(QString::fromUtf8("\n"
"         border: 2px solid #8B6F47; \n"
"         border-radius: 10px; \n"
"         background-color: white;\n"
"         qproperty-alignment: AlignCenter;\n"
"        "));
        label_qr_display->setScaledContents(true);
        label_qr_display->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_save_qr = new QPushButton(group_qr);
        btn_save_qr->setObjectName("btn_save_qr");
        btn_save_qr->setGeometry(QRect(200, 460, 140, 40));
        btn_save_qr->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 10px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/assets/save.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_save_qr->setIcon(icon7);
        btn_save_qr->setIconSize(QSize(24, 24));
        btn_print_qr = new QPushButton(group_qr);
        btn_print_qr->setObjectName("btn_print_qr");
        btn_print_qr->setGeometry(QRect(360, 460, 140, 40));
        btn_print_qr->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 10px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/assets/printer.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_print_qr->setIcon(icon8);
        btn_print_qr->setIconSize(QSize(24, 24));
        tabWidget4->addTab(tab_qrcode, QString());
        tab_catalog = new QWidget();
        tab_catalog->setObjectName("tab_catalog");
        tab_catalog->setStyleSheet(QString::fromUtf8("\n"
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
        label_catalog_title = new QLabel(tab_catalog);
        label_catalog_title->setObjectName("label_catalog_title");
        label_catalog_title->setGeometry(QRect(450, 20, 300, 40));
        label_catalog_title->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: white;"));
        label_catalog_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        table_catalog = new QTableWidget(tab_catalog);
        if (table_catalog->columnCount() < 6)
            table_catalog->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        table_catalog->setObjectName("table_catalog");
        table_catalog->setGeometry(QRect(20, 80, 1100, 550));
        table_catalog->setStyleSheet(QString::fromUtf8("\n"
"        QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 8px; }\n"
"        QTableWidget { border: 1px solid #8B6F47; selection-background-color: #E0E0E0; selection-color: white; gridline-color: #8B6F47; }\n"
"        QTableWidget::item { padding: 5px; }\n"
"       "));
        table_catalog->setRowCount(0);
        table_catalog->setColumnCount(6);
        btn_refresh_catalog = new QPushButton(tab_catalog);
        btn_refresh_catalog->setObjectName("btn_refresh_catalog");
        btn_refresh_catalog->setGeometry(QRect(20, 650, 150, 40));
        btn_refresh_catalog->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/assets/refresh.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_refresh_catalog->setIcon(icon9);
        btn_refresh_catalog->setIconSize(QSize(24, 24));
        btn_export_catalog = new QPushButton(tab_catalog);
        btn_export_catalog->setObjectName("btn_export_catalog");
        btn_export_catalog->setGeometry(QRect(190, 650, 150, 40));
        btn_export_catalog->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/assets/pdf_export.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_export_catalog->setIcon(icon10);
        btn_export_catalog->setIconSize(QSize(24, 24));
        btn_print_catalog = new QPushButton(tab_catalog);
        btn_print_catalog->setObjectName("btn_print_catalog");
        btn_print_catalog->setGeometry(QRect(360, 650, 150, 40));
        btn_print_catalog->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: #333; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        btn_print_catalog->setIcon(icon8);
        btn_print_catalog->setIconSize(QSize(24, 24));
        le_catalog_search = new QLineEdit(tab_catalog);
        le_catalog_search->setObjectName("le_catalog_search");
        le_catalog_search->setGeometry(QRect(800, 650, 250, 40));
        tabWidget4->addTab(tab_catalog, QString());
        btn_help_gestion_supplier_2 = new QToolButton(page_order);
        btn_help_gestion_supplier_2->setObjectName("btn_help_gestion_supplier_2");
        btn_help_gestion_supplier_2->setGeometry(QRect(990, 110, 30, 30));
        btn_help_gestion_supplier_2->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_gestion_supplier_2->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_gestion_supplier_2->setCheckable(true);
        stackedWidget->addWidget(page_order);

        verticalLayout->addWidget(stackedWidget);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);
        QObject::connect(btn_help_gestion_supplier, &QToolButton::toggled, lbl_hint_gestion_supplier, &QLabel::setVisible);
        QObject::connect(btn_help_stats_supplier, &QToolButton::toggled, lbl_hint_stats_supplier, &QLabel::setVisible);
        QObject::connect(btn_help_reviews_supplier, &QToolButton::toggled, lbl_hint_reviews_supplier, &QLabel::setVisible);
        QObject::connect(btn_help_gestion_equipment, &QToolButton::toggled, lbl_hint_gestion_equipment, &QLabel::setVisible);
        QObject::connect(btn_help_stats_equipment, &QToolButton::toggled, lbl_hint_stats_equipment, &QLabel::setVisible);

        stackedWidget->setCurrentIndex(6);
        oublie->setDefault(false);
        tabWidget->setCurrentIndex(1);
        tabWidget1->setCurrentIndex(1);
        tabWidget2->setCurrentIndex(0);
        tabWidget3->setCurrentIndex(2);
        tabWidget4->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Hammer Down", nullptr));
#if QT_CONFIG(tooltip)
        page_login->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        page_login->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(accessibility)
        page_login->setAccessibleName(QString());
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        login_id->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        login_id->setInputMask(QString());
        login_id->setText(QString());
        login_id->setPlaceholderText(QString());
        login->setText(QString());
        oublie->setText(QString());
        gs_equipment->setText(QString());
        gs_order->setText(QString());
        gs_fournisseur->setText(QString());
        gs_client->setText(QString());
        gs_employes->setText(QString());
        group_add->setTitle(QCoreApplication::translate("MainWindow", "Add Employee", nullptr));
        label_id->setText(QCoreApplication::translate("MainWindow", "Employee ID:", nullptr));
        label_nom->setText(QCoreApplication::translate("MainWindow", "Last Name:", nullptr));
        label_prenom->setText(QCoreApplication::translate("MainWindow", "First Name:", nullptr));
        label_fonction->setText(QCoreApplication::translate("MainWindow", "Job Title:", nullptr));
        label_age->setText(QCoreApplication::translate("MainWindow", "Age:", nullptr));
        label_mdp->setText(QCoreApplication::translate("MainWindow", "Password:", nullptr));
        label_salaire->setText(QCoreApplication::translate("MainWindow", "Salary:", nullptr));
        label_email->setText(QCoreApplication::translate("MainWindow", "Email Address:", nullptr));
        label_num->setText(QCoreApplication::translate("MainWindow", "Number:", nullptr));
        btn_add->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        btn_modify->setText(QCoreApplication::translate("MainWindow", "Modify", nullptr));
        btn_cancel->setText(QCoreApplication::translate("MainWindow", "Cancel", nullptr));
        btn_help_add->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_add->setText(QCoreApplication::translate("MainWindow", "Quick Info: Use this form to add or modify employee records. Passwords and Salary are sensitive data. Job Title helps in team assignment.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_add), QCoreApplication::translate("MainWindow", "Add Employee", nullptr));
        label_titre_liste_emp->setText(QCoreApplication::translate("MainWindow", "Employee List", nullptr));
        on_emp_chercher->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_view), QCoreApplication::translate("MainWindow", "View Employees", nullptr));
        group_add1->setTitle(QCoreApplication::translate("MainWindow", "Add Client", nullptr));
        label_id1->setText(QCoreApplication::translate("MainWindow", "Client ID:", nullptr));
        label_nom1->setText(QCoreApplication::translate("MainWindow", "Last Name:", nullptr));
        label_prenom1->setText(QCoreApplication::translate("MainWindow", "First Name:", nullptr));
        label_adresse->setText(QCoreApplication::translate("MainWindow", "Address:", nullptr));
        label_tel->setText(QCoreApplication::translate("MainWindow", "Phone Number:", nullptr));
        label_email1->setText(QCoreApplication::translate("MainWindow", "Email Address:", nullptr));
        label_sexe->setText(QCoreApplication::translate("MainWindow", "Gender:", nullptr));
        rb_homme->setText(QCoreApplication::translate("MainWindow", "Male", nullptr));
        rb_femme->setText(QCoreApplication::translate("MainWindow", "Female", nullptr));
        btn_add1->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        btn_cancel1->setText(QCoreApplication::translate("MainWindow", "Cancel", nullptr));
        btn_help_add_client->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_add_client->setText(QCoreApplication::translate("MainWindow", "Help: Enter client details here. Use 'Add' to save. In the 'View Clients' tab, you can search and export lists to PDF.", nullptr));
        btn_modify_2->setText(QCoreApplication::translate("MainWindow", "Modify", nullptr));
        tabWidget1->setTabText(tabWidget1->indexOf(tab_add1), QCoreApplication::translate("MainWindow", "Add Client", nullptr));
        label_titre_liste->setText(QCoreApplication::translate("MainWindow", "Client List", nullptr));
        btn_search->setText(QCoreApplication::translate("MainWindow", "Search", nullptr));
        btn_pdf->setText(QCoreApplication::translate("MainWindow", "PDF", nullptr));
        btn_delete->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        tabWidget1->setTabText(tabWidget1->indexOf(tab_view1), QCoreApplication::translate("MainWindow", "View Clients", nullptr));
        label_stat_title->setText(QCoreApplication::translate("MainWindow", "Client Statistics by Gender", nullptr));
        tabWidget1->setTabText(tabWidget1->indexOf(tab_stats), QCoreApplication::translate("MainWindow", "Statistics", nullptr));
        label_mail_title->setText(QCoreApplication::translate("MainWindow", "Mailing", nullptr));
        l_smtp->setText(QCoreApplication::translate("MainWindow", "Smtp-server:", nullptr));
        l_port->setText(QCoreApplication::translate("MainWindow", "Server port:", nullptr));
        l_user->setText(QCoreApplication::translate("MainWindow", "Username:", nullptr));
        l_pass->setText(QCoreApplication::translate("MainWindow", "Password:", nullptr));
        l_to->setText(QCoreApplication::translate("MainWindow", "Recipant to:", nullptr));
        l_subj->setText(QCoreApplication::translate("MainWindow", "Subject:", nullptr));
        l_att->setText(QCoreApplication::translate("MainWindow", "Attachment:", nullptr));
        btn_browse->setText(QCoreApplication::translate("MainWindow", "Browse", nullptr));
        l_msg->setText(QCoreApplication::translate("MainWindow", "Message:", nullptr));
        btn_send->setText(QCoreApplication::translate("MainWindow", "Send", nullptr));
        tabWidget1->setTabText(tabWidget1->indexOf(tab_mail), QCoreApplication::translate("MainWindow", "Send Email", nullptr));
        groupBox_gestion->setTitle(QCoreApplication::translate("MainWindow", "Supplier Management", nullptr));
        label_id2->setText(QCoreApplication::translate("MainWindow", "Supplier ID:", nullptr));
        label_nom2->setText(QCoreApplication::translate("MainWindow", "Company Name:", nullptr));
        label_adresse1->setText(QCoreApplication::translate("MainWindow", "Adresse:", nullptr));
        label_email2->setText(QCoreApplication::translate("MainWindow", "Email Address:", nullptr));
        label_product_type->setText(QCoreApplication::translate("MainWindow", "Product Type:", nullptr));
        label_type->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        label_cp->setText(QCoreApplication::translate("MainWindow", "Postal Code:", nullptr));
        label_tel1->setText(QCoreApplication::translate("MainWindow", "Phone Number:", nullptr));
        lbl_image_preview->setText(QCoreApplication::translate("MainWindow", "No image", nullptr));
        btn_upload_image->setText(QCoreApplication::translate("MainWindow", "Choose Image", nullptr));
        btn_add2->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        btn_modify1->setText(QCoreApplication::translate("MainWindow", "Modify", nullptr));
        btn_delete1->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        btn_cancel_gestion->setText(QCoreApplication::translate("MainWindow", "Cancel", nullptr));
        label_sms->setText(QCoreApplication::translate("MainWindow", "SMS Message:", nullptr));
        btn_send_sms->setText(QCoreApplication::translate("MainWindow", "Send SMS", nullptr));
        btn_help_gestion_supplier->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_gestion_supplier->setText(QCoreApplication::translate("MainWindow", "Quick Help: Use 'Add' to create new suppliers. Select a row in the 'View' tab to 'Modify' or 'Delete' here. Email and Phone are required for SMS/Communication features.", nullptr));
        tabWidget2->setTabText(tabWidget2->indexOf(tab_gestion), QCoreApplication::translate("MainWindow", "Supplier Management", nullptr));
        lbl_stats_title->setText(QCoreApplication::translate("MainWindow", "Supplier Insights Dashboard", nullptr));
        btn_help_stats_supplier->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_stats_supplier->setText(QCoreApplication::translate("MainWindow", "Dashboard Tip: Circular charts show general health. Bar charts below show regional performance. Hover over icons for detailed metrics.", nullptr));
        lbl_percent_retention->setText(QCoreApplication::translate("MainWindow", "92%", nullptr));
        lbl_text_retention->setText(QCoreApplication::translate("MainWindow", "RETENTION", nullptr));
        lbl_percent_accuracy->setText(QCoreApplication::translate("MainWindow", "87%", nullptr));
        lbl_text_accuracy->setText(QCoreApplication::translate("MainWindow", "ACCURACY", nullptr));
        group_performance_bars->setTitle(QCoreApplication::translate("MainWindow", "Performance Metrics", nullptr));
        lbl_bar_quality->setText(QCoreApplication::translate("MainWindow", "Product Quality:", nullptr));
        lbl_bar_speed->setText(QCoreApplication::translate("MainWindow", "Delivery Speed:", nullptr));
        lbl_summary_title->setText(QCoreApplication::translate("MainWindow", "Network Status", nullptr));
        lbl_summary_val->setText(QCoreApplication::translate("MainWindow", "Active: 12 | Inactive: 2 | Performance: optimal\n"
"Last updated: Today 01:40 AM", nullptr));
        lbl_chart_types->setText(QCoreApplication::translate("MainWindow", "Product Categories (%)", nullptr));
        lbl_chart_reviews->setText(QCoreApplication::translate("MainWindow", "Monthly Satisfaction Trend", nullptr));
        group_regional_stats->setTitle(QCoreApplication::translate("MainWindow", "Regional Performance Comparison", nullptr));
        lbl_reg_1->setText(QCoreApplication::translate("MainWindow", "North Region:", nullptr));
        lbl_reg_2->setText(QCoreApplication::translate("MainWindow", "South Region:", nullptr));
        lbl_top_performer->setText(QCoreApplication::translate("MainWindow", "\360\237\217\206 Top Performer: Global Logistics Corp\n"
"Compliance Score: 99.8%\n"
"Consistency Index: high", nullptr));
        tabWidget2->setTabText(tabWidget2->indexOf(tab_stats1), QCoreApplication::translate("MainWindow", "Statistics", nullptr));
        lbl_title_view->setText(QCoreApplication::translate("MainWindow", "Supplier List", nullptr));
        btn_chercher->setText(QCoreApplication::translate("MainWindow", "Chercher par nom", nullptr));
        le_recherche1->setPlaceholderText(QString());
        tabWidget2->setTabText(tabWidget2->indexOf(tab_view2), QCoreApplication::translate("MainWindow", "View Supplier", nullptr));
        lbl_reviews_title->setText(QCoreApplication::translate("MainWindow", "Supplier Reliability & Feedback", nullptr));
        btn_help_reviews_supplier->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_reviews_supplier->setText(QCoreApplication::translate("MainWindow", "Review Tip: Selection impacts both the 'Satisfaction Breakdown' and 'Rating Distribution' charts. The table below lists all individual feedback comments.", nullptr));
        lbl_select_supplier->setText(QCoreApplication::translate("MainWindow", "Select Supplier:", nullptr));
        lbl_avg_score->setText(QCoreApplication::translate("MainWindow", "4.8", nullptr));
        lbl_stars_row->setText(QCoreApplication::translate("MainWindow", "\342\230\205\342\230\205\342\230\205\342\230\205\342\230\205", nullptr));
        lbl_review_count->setText(QCoreApplication::translate("MainWindow", "Based on 24 customer reviews", nullptr));
        group_feedback_breakdown->setTitle(QCoreApplication::translate("MainWindow", "Satisfaction Breakdown", nullptr));
        lbl_cat_quality->setText(QCoreApplication::translate("MainWindow", "Quality:", nullptr));
        lbl_cat_response->setText(QCoreApplication::translate("MainWindow", "Response:", nullptr));
        lbl_cat_price->setText(QCoreApplication::translate("MainWindow", "Price Value:", nullptr));
        lbl_dist_title->setText(QCoreApplication::translate("MainWindow", "Rating Distribution", nullptr));
        lbl_row_5->setText(QCoreApplication::translate("MainWindow", "5 Star", nullptr));
        lbl_row_4->setText(QCoreApplication::translate("MainWindow", "4 Star", nullptr));
        lbl_row_3->setText(QCoreApplication::translate("MainWindow", "3 Star", nullptr));
        lbl_dist_insight->setText(QCoreApplication::translate("MainWindow", "Trend: Positive feedback has increased by 12% in the last 30 days.", nullptr));
        tabWidget2->setTabText(tabWidget2->indexOf(tab_reviews), QCoreApplication::translate("MainWindow", "Supplier Reviews", nullptr));
        groupBox_gestion1->setTitle(QCoreApplication::translate("MainWindow", "Equipment Management", nullptr));
        label_id3->setText(QCoreApplication::translate("MainWindow", "ID:", nullptr));
        label_date_achat->setText(QCoreApplication::translate("MainWindow", "Purchase Date:", nullptr));
        de_date_achat->setDisplayFormat(QCoreApplication::translate("MainWindow", "dd/MM/yyyy", nullptr));
        label_etat->setText(QCoreApplication::translate("MainWindow", "Status:", nullptr));
        rb_intact->setText(QCoreApplication::translate("MainWindow", "Intact", nullptr));
        rb_broken->setText(QCoreApplication::translate("MainWindow", "Broken", nullptr));
        label_desc->setText(QCoreApplication::translate("MainWindow", "Description:", nullptr));
        btn_add3->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        btn_modify2->setText(QCoreApplication::translate("MainWindow", "Modify", nullptr));
        btn_delete2->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        btn_cancel_gestion1->setText(QCoreApplication::translate("MainWindow", "Cancel", nullptr));
        btn_help_gestion_equipment->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_gestion_equipment->setText(QCoreApplication::translate("MainWindow", "Tip: Use 'Ajouter' for new gear. Select equipment from the list to update its status to 'Broken' or 'Intact'. Descriptions help track maintenance history.", nullptr));
        tabWidget3->setTabText(tabWidget3->indexOf(tab_gestion1), QCoreApplication::translate("MainWindow", "Equipment Management", nullptr));
        label_titre_liste_equip->setText(QCoreApplication::translate("MainWindow", "Equipment List", nullptr));
        btn_search_top->setText(QCoreApplication::translate("MainWindow", "search", nullptr));
        table_equipments->setProperty("placeholderText", QVariant(QCoreApplication::translate("MainWindow", "Search...", nullptr)));
        btn_search1->setText(QCoreApplication::translate("MainWindow", "Search", nullptr));
        btn_delete_confirm->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        tabWidget3->setTabText(tabWidget3->indexOf(tab_view3), QCoreApplication::translate("MainWindow", "View Equipments", nullptr));
        label_titre_histo->setText(QCoreApplication::translate("MainWindow", "Activity History", nullptr));
        le_history_search->setPlaceholderText(QCoreApplication::translate("MainWindow", "Search in history...", nullptr));
        btn_history_search->setText(QCoreApplication::translate("MainWindow", "Search", nullptr));
        btn_refresh_history->setText(QCoreApplication::translate("MainWindow", "Refresh", nullptr));
        btn_export_history->setText(QCoreApplication::translate("MainWindow", "Export to PDF", nullptr));
        btn_clear_history->setText(QCoreApplication::translate("MainWindow", "Clear Logs", nullptr));
        tabWidget3->setTabText(tabWidget3->indexOf(tab_history), QCoreApplication::translate("MainWindow", "History", nullptr));
        label_stats_title->setText(QCoreApplication::translate("MainWindow", "Equipment Statistics Overview", nullptr));
        group_metrics->setTitle(QCoreApplication::translate("MainWindow", "General Metrics", nullptr));
        lbl_total_eq->setText(QCoreApplication::translate("MainWindow", "Total Equipment:", nullptr));
        val_total_eq->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lbl_oper_eq->setText(QCoreApplication::translate("MainWindow", "Operational (Intact):", nullptr));
        val_oper_eq->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lbl_broken_eq->setText(QCoreApplication::translate("MainWindow", "Under Repair (Broken):", nullptr));
        val_broken_eq->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        group_chart->setTitle(QCoreApplication::translate("MainWindow", "Condition Distribution", nullptr));
        label_chart_placeholder->setText(QCoreApplication::translate("MainWindow", "Chart visualization goes here", nullptr));
        btn_help_stats_equipment->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
        lbl_hint_stats_equipment->setText(QCoreApplication::translate("MainWindow", "Tip: This tab provides an overview of your equipment. 'General Metrics' shows total, intact, and broken items. 'Condition Distribution' will display a chart (e.g., pie chart) visualizing the proportion of intact vs. broken equipment.", nullptr));
        tabWidget3->setTabText(tabWidget3->indexOf(tab_stats2), QCoreApplication::translate("MainWindow", "Statistics", nullptr));
        group_manage->setTitle(QCoreApplication::translate("MainWindow", "Order Management", nullptr));
        label_id4->setText(QCoreApplication::translate("MainWindow", "Order ID:", nullptr));
        label_type1->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        label_stock->setText(QCoreApplication::translate("MainWindow", "Quantity:", nullptr));
        label_prix->setText(QCoreApplication::translate("MainWindow", "Price:", nullptr));
        label_buyer->setText(QCoreApplication::translate("MainWindow", "Buyer ID:", nullptr));
        btn_add4->setText(QCoreApplication::translate("MainWindow", "Add Order", nullptr));
        btn_modify3->setText(QCoreApplication::translate("MainWindow", "Modify Order", nullptr));
        btn_delete3->setText(QCoreApplication::translate("MainWindow", "Delete Order", nullptr));
        btn_clear->setText(QCoreApplication::translate("MainWindow", "Clear Fields", nullptr));
        btn_load->setText(QCoreApplication::translate("MainWindow", "Load Order", nullptr));
        tabWidget4->setTabText(tabWidget4->indexOf(tab_manage), QCoreApplication::translate("MainWindow", "Manage Orders", nullptr));
        group_qr->setTitle(QCoreApplication::translate("MainWindow", "Generate QR Code", nullptr));
        label_qr_order_id->setText(QCoreApplication::translate("MainWindow", "Order ID:", nullptr));
        le_qr_order_id->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter Order ID to generate QR code...", nullptr));
        btn_generate_qr->setText(QCoreApplication::translate("MainWindow", "Generate QR", nullptr));
        label_qr_display->setText(QCoreApplication::translate("MainWindow", "QR Code will appear here", nullptr));
        btn_save_qr->setText(QCoreApplication::translate("MainWindow", "Save QR Code", nullptr));
        btn_print_qr->setText(QCoreApplication::translate("MainWindow", "Print QR Code", nullptr));
        tabWidget4->setTabText(tabWidget4->indexOf(tab_qrcode), QCoreApplication::translate("MainWindow", "QR Code", nullptr));
        label_catalog_title->setText(QCoreApplication::translate("MainWindow", "Order Catalog with QR Codes", nullptr));
        QTableWidgetItem *___qtablewidgetitem = table_catalog->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Order ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = table_catalog->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = table_catalog->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Quantity", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = table_catalog->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Price", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = table_catalog->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Buyer ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = table_catalog->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "QR Code", nullptr));
        btn_refresh_catalog->setText(QCoreApplication::translate("MainWindow", "Refresh Catalog", nullptr));
        btn_export_catalog->setText(QCoreApplication::translate("MainWindow", "Export to PDF", nullptr));
        btn_print_catalog->setText(QCoreApplication::translate("MainWindow", "Print Catalog", nullptr));
        le_catalog_search->setPlaceholderText(QCoreApplication::translate("MainWindow", "Search catalog...", nullptr));
        tabWidget4->setTabText(tabWidget4->indexOf(tab_catalog), QCoreApplication::translate("MainWindow", "Catalog", nullptr));
        btn_help_gestion_supplier_2->setText(QCoreApplication::translate("MainWindow", "?", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
