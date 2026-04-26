/********************************************************************************
** Form generated from reading UI file 'supplier_management.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUPPLIER_MANAGEMENT_H
#define UI_SUPPLIER_MANAGEMENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SupplierManagement
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_gestion;
    QGroupBox *groupBox_gestion;
    QLabel *label_id;
    QLineEdit *le_id;
    QLabel *label_nom;
    QLineEdit *le_nom;
    QLabel *label_adresse;
    QLineEdit *le_adresse;
    QLabel *label_email;
    QLineEdit *le_email;
    QLabel *label_product_type;
    QLineEdit *le_product_type;
    QLabel *label_type;
    QLineEdit *le_type;
    QLabel *label_cp;
    QSpinBox *sb_cp;
    QLabel *label_tel;
    QLineEdit *le_tel;
    QLabel *lbl_image_preview;
    QPushButton *btn_upload_image;
    QPushButton *btn_add;
    QPushButton *btn_modify;
    QPushButton *btn_delete;
    QPushButton *btn_cancel_gestion;
    QPushButton *btn_clear;
    QLabel *label_sms;
    QTextEdit *txt_sms;
    QPushButton *btn_send_sms;
    QToolButton *btn_help_gestion;
    QLabel *lbl_hint_gestion;
    QWidget *tab_stats;
    QLabel *lbl_stats_title;
    QToolButton *btn_help_stats;
    QLabel *lbl_hint_stats;
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
    QWidget *tab_view;
    QLabel *lbl_title_view;
    QPushButton *btn_chercher;
    QLineEdit *le_recherche;
    QTableView *tableView;
    QWidget *tab_reviews;
    QLabel *lbl_reviews_title;
    QToolButton *btn_help_reviews;
    QLabel *lbl_hint_reviews;
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
    QGroupBox *group_submit_review;
    QLabel *lbl_employee_label;
    QComboBox *cb_employee_rating;
    QLabel *lbl_equipment_label;
    QComboBox *cb_equipment_rating;
    QLabel *lbl_rating_label;
    QSpinBox *sb_rating;
    QLabel *lbl_rating_stars;
    QLabel *lbl_comment_label;
    QLineEdit *le_comment;
    QPushButton *btn_submit_review;
    QPushButton *btn_refresh_reviews;
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
    QPushButton *btn_return_home;

    void setupUi(QWidget *SupplierManagement)
    {
        if (SupplierManagement->objectName().isEmpty())
            SupplierManagement->setObjectName("SupplierManagement");
        SupplierManagement->resize(1322, 800);
        SupplierManagement->setStyleSheet(QString::fromUtf8("\n"
"    #SupplierManagement {\n"
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
        tabWidget = new QTabWidget(SupplierManagement);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(100, 20, 1181, 800));
        QFont font;
        font.setFamilies({QString::fromUtf8("Gadugi")});
        font.setPointSize(10);
        font.setBold(true);
        tabWidget->setFont(font);
        tabWidget->setStyleSheet(QString::fromUtf8("\n"
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
"          QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"          QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0; background: transparent; color: transparent; border: none; }\n"
"          QLineEdit {\n"
"              background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"              border: 2px solid #8B6F47;\n"
"              border-radius: 8px;\n"
"              padding: 3px 12px;\n"
"              font-size: 14px;\n"
"              color: #333;\n"
"              selection-background-color: #8B6F47;\n"
"              selection-color: white;\n"
"          }\n"
"          QLineEdit:hover {\n"
"              border: 2px solid #A0825A;\n"
"              background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"          }\n"
"          QLineEdit:focus {\n"
"              border: 2px solid #8B4513; \n"
"              background: #F"
                        "FFAF0;\n"
"          }\n"
"         "));
        label_id = new QLabel(groupBox_gestion);
        label_id->setObjectName("label_id");
        label_id->setGeometry(QRect(50, 100, 150, 30));
        label_id->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_id = new QLineEdit(groupBox_gestion);
        le_id->setObjectName("le_id");
        le_id->setGeometry(QRect(200, 100, 200, 30));
        label_nom = new QLabel(groupBox_gestion);
        label_nom->setObjectName("label_nom");
        label_nom->setGeometry(QRect(50, 170, 150, 30));
        label_nom->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_nom = new QLineEdit(groupBox_gestion);
        le_nom->setObjectName("le_nom");
        le_nom->setGeometry(QRect(200, 170, 200, 30));
        label_adresse = new QLabel(groupBox_gestion);
        label_adresse->setObjectName("label_adresse");
        label_adresse->setGeometry(QRect(50, 240, 150, 30));
        label_adresse->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_adresse = new QLineEdit(groupBox_gestion);
        le_adresse->setObjectName("le_adresse");
        le_adresse->setGeometry(QRect(200, 240, 200, 30));
        label_email = new QLabel(groupBox_gestion);
        label_email->setObjectName("label_email");
        label_email->setGeometry(QRect(50, 310, 150, 30));
        label_email->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_email = new QLineEdit(groupBox_gestion);
        le_email->setObjectName("le_email");
        le_email->setGeometry(QRect(200, 310, 200, 30));
        label_product_type = new QLabel(groupBox_gestion);
        label_product_type->setObjectName("label_product_type");
        label_product_type->setGeometry(QRect(450, 310, 150, 30));
        label_product_type->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_product_type = new QLineEdit(groupBox_gestion);
        le_product_type->setObjectName("le_product_type");
        le_product_type->setGeometry(QRect(600, 310, 200, 30));
        label_type = new QLabel(groupBox_gestion);
        label_type->setObjectName("label_type");
        label_type->setGeometry(QRect(450, 100, 150, 30));
        label_type->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_type = new QLineEdit(groupBox_gestion);
        le_type->setObjectName("le_type");
        le_type->setGeometry(QRect(600, 100, 200, 30));
        label_cp = new QLabel(groupBox_gestion);
        label_cp->setObjectName("label_cp");
        label_cp->setGeometry(QRect(450, 170, 150, 30));
        label_cp->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        sb_cp = new QSpinBox(groupBox_gestion);
        sb_cp->setObjectName("sb_cp");
        sb_cp->setGeometry(QRect(600, 170, 200, 30));
        sb_cp->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;\n"
""));
        label_tel = new QLabel(groupBox_gestion);
        label_tel->setObjectName("label_tel");
        label_tel->setGeometry(QRect(450, 240, 160, 30));
        label_tel->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        le_tel = new QLineEdit(groupBox_gestion);
        le_tel->setObjectName("le_tel");
        le_tel->setGeometry(QRect(600, 240, 200, 30));
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
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        btn_add = new QPushButton(groupBox_gestion);
        btn_add->setObjectName("btn_add");
        btn_add->setGeometry(QRect(160, 500, 130, 40));
        btn_add->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/add.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_add->setIcon(icon);
        btn_add->setIconSize(QSize(24, 24));
        btn_modify = new QPushButton(groupBox_gestion);
        btn_modify->setObjectName("btn_modify");
        btn_modify->setGeometry(QRect(400, 500, 130, 40));
        btn_modify->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/assets/modify.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_modify->setIcon(icon1);
        btn_modify->setIconSize(QSize(24, 24));
        btn_delete = new QPushButton(groupBox_gestion);
        btn_delete->setObjectName("btn_delete");
        btn_delete->setGeometry(QRect(640, 500, 130, 40));
        btn_delete->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/assets/delete.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_delete->setIcon(icon2);
        btn_delete->setIconSize(QSize(24, 24));
        btn_cancel_gestion = new QPushButton(groupBox_gestion);
        btn_cancel_gestion->setObjectName("btn_cancel_gestion");
        btn_cancel_gestion->setGeometry(QRect(890, 500, 130, 40));
        btn_cancel_gestion->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/assets/icon_cancel.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_cancel_gestion->setIcon(icon3);
        btn_clear = new QPushButton(groupBox_gestion);
        btn_clear->setObjectName("btn_clear");
        btn_clear->setGeometry(QRect(1040, 500, 130, 40));
        btn_clear->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/assets/clear.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_clear->setIcon(icon4);
        btn_clear->setIconSize(QSize(24, 24));
        label_sms = new QLabel(groupBox_gestion);
        label_sms->setObjectName("label_sms");
        label_sms->setGeometry(QRect(50, 380, 150, 30));
        label_sms->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;\n"
""));
        txt_sms = new QTextEdit(groupBox_gestion);
        txt_sms->setObjectName("txt_sms");
        txt_sms->setGeometry(QRect(200, 380, 500, 80));
        txt_sms->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;\n"
""));
        btn_send_sms = new QPushButton(groupBox_gestion);
        btn_send_sms->setObjectName("btn_send_sms");
        btn_send_sms->setGeometry(QRect(720, 400, 150, 40));
        btn_send_sms->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/assets/icon_sms.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_send_sms->setIcon(icon5);
        btn_help_gestion = new QToolButton(groupBox_gestion);
        btn_help_gestion->setObjectName("btn_help_gestion");
        btn_help_gestion->setGeometry(QRect(1120, 20, 30, 30));
        btn_help_gestion->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_gestion->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_gestion->setCheckable(true);
        lbl_hint_gestion = new QLabel(groupBox_gestion);
        lbl_hint_gestion->setObjectName("lbl_hint_gestion");
        lbl_hint_gestion->setGeometry(QRect(160, 550, 800, 60));
        lbl_hint_gestion->setVisible(false);
        lbl_hint_gestion->setStyleSheet(QString::fromUtf8("color: #555; font-size: 13px; font-style: italic; background: rgba(139, 111, 71, 0.1); padding: 10px; border-radius: 5px;"));
        lbl_hint_gestion->setWordWrap(true);
        tabWidget->addTab(tab_gestion, QString());
        tab_stats = new QWidget();
        tab_stats->setObjectName("tab_stats");
        lbl_stats_title = new QLabel(tab_stats);
        lbl_stats_title->setObjectName("lbl_stats_title");
        lbl_stats_title->setGeometry(QRect(700, 10, 400, 40));
        lbl_stats_title->setStyleSheet(QString::fromUtf8("font-size: 26px; font-weight: bold; color: #333; background: transparent;"));
        btn_help_stats = new QToolButton(tab_stats);
        btn_help_stats->setObjectName("btn_help_stats");
        btn_help_stats->setGeometry(QRect(1120, 15, 30, 30));
        btn_help_stats->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }"));
        btn_help_stats->setCheckable(true);
        lbl_hint_stats = new QLabel(tab_stats);
        lbl_hint_stats->setObjectName("lbl_hint_stats");
        lbl_hint_stats->setGeometry(QRect(450, 10, 600, 40));
        lbl_hint_stats->setVisible(false);
        lbl_hint_stats->setStyleSheet(QString::fromUtf8("color: #8B6F47; font-weight: bold; font-size: 11px;"));
        frame_circle_retention = new QFrame(tab_stats);
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
        frame_circle_accuracy = new QFrame(tab_stats);
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
        group_performance_bars = new QGroupBox(tab_stats);
        group_performance_bars->setObjectName("group_performance_bars");
        group_performance_bars->setGeometry(QRect(330, 50, 400, 140));
        group_performance_bars->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: rgba(60, 45, 30, 0.7); border: 2px solid #8B6F47; border-radius: 12px; margin-top: 18px; color: white; }\n"
"    QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 2px 12px; background-color: #8B6F47; font-weight: bold; color: white; border-radius: 4px; }"));
        lbl_bar_quality = new QLabel(group_performance_bars);
        lbl_bar_quality->setObjectName("lbl_bar_quality");
        lbl_bar_quality->setGeometry(QRect(10, 35, 150, 20));
        lbl_bar_quality->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_quality = new QProgressBar(group_performance_bars);
        pb_quality->setObjectName("pb_quality");
        pb_quality->setGeometry(QRect(160, 35, 220, 20));
        pb_quality->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 5px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #D4AF37; border-radius: 4px; }"));
        pb_quality->setValue(95);
        lbl_bar_speed = new QLabel(group_performance_bars);
        lbl_bar_speed->setObjectName("lbl_bar_speed");
        lbl_bar_speed->setGeometry(QRect(10, 75, 150, 20));
        lbl_bar_speed->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_speed = new QProgressBar(group_performance_bars);
        pb_speed->setObjectName("pb_speed");
        pb_speed->setGeometry(QRect(160, 75, 220, 20));
        pb_speed->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 5px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #D4AF37; border-radius: 4px; }"));
        pb_speed->setValue(82);
        frame_stat_summary = new QFrame(tab_stats);
        frame_stat_summary->setObjectName("frame_stat_summary");
        frame_stat_summary->setGeometry(QRect(750, 60, 380, 120));
        frame_stat_summary->setStyleSheet(QString::fromUtf8("background-color: rgba(60, 45, 30, 0.7); border: 2px solid #8B6F47; border-radius: 12px; color: white;"));
        frame_stat_summary->setFrameShape(QFrame::Shape::StyledPanel);
        frame_stat_summary->setFrameShadow(QFrame::Shadow::Raised);
        lbl_summary_title = new QLabel(frame_stat_summary);
        lbl_summary_title->setObjectName("lbl_summary_title");
        lbl_summary_title->setGeometry(QRect(20, 15, 200, 20));
        lbl_summary_title->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #D4AF37;"));
        lbl_summary_val = new QLabel(frame_stat_summary);
        lbl_summary_val->setObjectName("lbl_summary_val");
        lbl_summary_val->setGeometry(QRect(20, 45, 340, 50));
        lbl_summary_val->setStyleSheet(QString::fromUtf8("font-size: 13px;"));
        frame_chart_types = new QFrame(tab_stats);
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
        frame_chart_reviews = new QFrame(tab_stats);
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
        group_regional_stats = new QGroupBox(tab_stats);
        group_regional_stats->setObjectName("group_regional_stats");
        group_regional_stats->setGeometry(QRect(20, 580, 1111, 180));
        group_regional_stats->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"    QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }"));
        lbl_reg_1 = new QLabel(group_regional_stats);
        lbl_reg_1->setObjectName("lbl_reg_1");
        lbl_reg_1->setGeometry(QRect(20, 40, 100, 20));
        lbl_reg_1->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_reg_1 = new QProgressBar(group_regional_stats);
        pb_reg_1->setObjectName("pb_reg_1");
        pb_reg_1->setGeometry(QRect(130, 40, 380, 15));
        pb_reg_1->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 5px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #4CAF50; border-radius: 4px; }"));
        pb_reg_1->setValue(88);
        lbl_reg_2 = new QLabel(group_regional_stats);
        lbl_reg_2->setObjectName("lbl_reg_2");
        lbl_reg_2->setGeometry(QRect(20, 80, 100, 20));
        lbl_reg_2->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_reg_2 = new QProgressBar(group_regional_stats);
        pb_reg_2->setObjectName("pb_reg_2");
        pb_reg_2->setGeometry(QRect(130, 80, 380, 15));
        pb_reg_2->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 10px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #D4AF37; border-radius: 8px; }"));
        pb_reg_2->setValue(72);
        lbl_top_performer = new QLabel(group_regional_stats);
        lbl_top_performer->setObjectName("lbl_top_performer");
        lbl_top_performer->setGeometry(QRect(600, 40, 480, 80));
        lbl_top_performer->setStyleSheet(QString::fromUtf8("font-size: 16px; color: #D4AF37; font-weight: bold; border-left: 3px solid #8B6F47; padding-left: 20px;"));
        tabWidget->addTab(tab_stats, QString());
        tab_view = new QWidget();
        tab_view->setObjectName("tab_view");
        tab_view->setStyleSheet(QString::fromUtf8("\n"
"          QLineEdit {\n"
"              background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);\n"
"              border: 2px solid #8B6F47;\n"
"              border-radius: 8px;\n"
"              padding: 3px 12px;\n"
"              font-size: 14px;\n"
"              color: #333;\n"
"              selection-background-color: #8B6F47;\n"
"              selection-color: white;\n"
"          }\n"
"          QLineEdit:hover {\n"
"              border: 2px solid #A0825A;\n"
"              background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);\n"
"          }\n"
"          QLineEdit:focus {\n"
"              border: 2px solid #8B4513; \n"
"              background: #FFFAF0;\n"
"          }\n"
"       "));
        lbl_title_view = new QLabel(tab_view);
        lbl_title_view->setObjectName("lbl_title_view");
        lbl_title_view->setGeometry(QRect(700, 20, 400, 40));
        lbl_title_view->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #333; background: white; border-radius: 5px; padding: 5px;\n"
""));
        lbl_title_view->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_chercher = new QPushButton(tab_view);
        btn_chercher->setObjectName("btn_chercher");
        btn_chercher->setGeometry(QRect(150, 100, 150, 40));
        btn_chercher->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-top-left-radius: 10px; border-bottom-left-radius: 10px; color: white; font-weight: bold; border: none; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        QPushButton:hover { background-color: #C19DAF; }\n"
"       "));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/assets/icon_search.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_chercher->setIcon(icon6);
        le_recherche = new QLineEdit(tab_view);
        le_recherche->setObjectName("le_recherche");
        le_recherche->setGeometry(QRect(300, 100, 600, 40));
        le_recherche->setStyleSheet(QString::fromUtf8(""));
        tableView = new QTableView(tab_view);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(100, 220, 911, 411));
        tableView->setStyleSheet(QString::fromUtf8("\n"
"        QTableView { background-color: rgba(44, 30, 18, 0.92); color: #F5E6D3; border-radius: 10px; gridline-color: #8B6F47; selection-background-color: rgba(193,127,62,0.35); selection-color: #F5E6D3; }\n"
"        QTableView::item { background-color: rgba(44, 30, 18, 0.96); color: #F5E6D3; border: none; }\n"
"        QTableView::item:selected { background-color: rgba(193,127,62,0.45); color: #FFFFFF; }\n"
"        QHeaderView::section { background-color: #8B6F47; color: #F5E6D3; font-weight: bold; border: none; padding: 5px; }\n"
"       "));
        tabWidget->addTab(tab_view, QString());
        tab_reviews = new QWidget();
        tab_reviews->setObjectName("tab_reviews");
        lbl_reviews_title = new QLabel(tab_reviews);
        lbl_reviews_title->setObjectName("lbl_reviews_title");
        lbl_reviews_title->setGeometry(QRect(640, 15, 520, 40));
        lbl_reviews_title->setStyleSheet(QString::fromUtf8("font-size: 26px; font-weight: bold; color: #333; background: transparent;"));
        btn_help_reviews = new QToolButton(tab_reviews);
        btn_help_reviews->setObjectName("btn_help_reviews");
        btn_help_reviews->setGeometry(QRect(1120, 15, 30, 30));
        btn_help_reviews->setStyleSheet(QString::fromUtf8("QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"      QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"));
        btn_help_reviews->setCheckable(true);
        lbl_hint_reviews = new QLabel(tab_reviews);
        lbl_hint_reviews->setObjectName("lbl_hint_reviews");
        lbl_hint_reviews->setGeometry(QRect(330, 15, 780, 50));
        lbl_hint_reviews->setVisible(false);
        lbl_hint_reviews->setStyleSheet(QString::fromUtf8("color: #8B6F47; font-weight: bold; font-size: 11px;"));
        lbl_hint_reviews->setWordWrap(true);
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
        lbl_stars_row->setStyleSheet(QString::fromUtf8("font-size: 24px; color: #FFD700; border: none; font-family: 'Segoe UI Symbol', 'Arial';"));
        lbl_review_count = new QLabel(frame_rating_header);
        lbl_review_count->setObjectName("lbl_review_count");
        lbl_review_count->setGeometry(QRect(20, 75, 310, 30));
        lbl_review_count->setStyleSheet(QString::fromUtf8("font-size: 14px; opacity: 0.9; border: none;"));
        group_feedback_breakdown = new QGroupBox(tab_reviews);
        group_feedback_breakdown->setObjectName("group_feedback_breakdown");
        group_feedback_breakdown->setGeometry(QRect(730, 60, 421, 140));
        group_feedback_breakdown->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"    QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }"));
        lbl_cat_quality = new QLabel(group_feedback_breakdown);
        lbl_cat_quality->setObjectName("lbl_cat_quality");
        lbl_cat_quality->setGeometry(QRect(10, 30, 120, 20));
        lbl_cat_quality->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_review_quality = new QProgressBar(group_feedback_breakdown);
        pb_review_quality->setObjectName("pb_review_quality");
        pb_review_quality->setGeometry(QRect(140, 30, 260, 15));
        pb_review_quality->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 5px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #4CAF50; border-radius: 4px; }"));
        pb_review_quality->setValue(98);
        lbl_cat_response = new QLabel(group_feedback_breakdown);
        lbl_cat_response->setObjectName("lbl_cat_response");
        lbl_cat_response->setGeometry(QRect(10, 65, 120, 20));
        lbl_cat_response->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_review_response = new QProgressBar(group_feedback_breakdown);
        pb_review_response->setObjectName("pb_review_response");
        pb_review_response->setGeometry(QRect(140, 65, 260, 15));
        pb_review_response->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 5px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #2196F3; border-radius: 4px; }"));
        pb_review_response->setValue(85);
        lbl_cat_price = new QLabel(group_feedback_breakdown);
        lbl_cat_price->setObjectName("lbl_cat_price");
        lbl_cat_price->setGeometry(QRect(10, 100, 120, 20));
        lbl_cat_price->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-weight: bold;"));
        pb_review_price = new QProgressBar(group_feedback_breakdown);
        pb_review_price->setObjectName("pb_review_price");
        pb_review_price->setGeometry(QRect(140, 100, 260, 15));
        pb_review_price->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 5px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #9C27B0; border-radius: 4px; }"));
        pb_review_price->setValue(90);
        group_submit_review = new QGroupBox(tab_reviews);
        group_submit_review->setObjectName("group_submit_review");
        group_submit_review->setGeometry(QRect(30, 205, 1121, 200));
        group_submit_review->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: rgba(60, 45, 30, 0.7); border: 2px solid #8B6F47; border-radius: 12px; margin-top: 18px; color: white; }\n"
"    QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 2px 12px; background-color: #8B6F47; font-weight: bold; color: white; border-radius: 4px; }"));
        lbl_employee_label = new QLabel(group_submit_review);
        lbl_employee_label->setObjectName("lbl_employee_label");
        lbl_employee_label->setGeometry(QRect(20, 30, 100, 28));
        lbl_employee_label->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; color: #D4AF37;"));
        cb_employee_rating = new QComboBox(group_submit_review);
        cb_employee_rating->setObjectName("cb_employee_rating");
        cb_employee_rating->setGeometry(QRect(120, 30, 250, 28));
        cb_employee_rating->setStyleSheet(QString::fromUtf8("QComboBox { border: 2px solid #8B6F47; border-radius: 6px; padding: 2px 5px; background: rgba(0,0,0,0.4); color: white; font-size: 13px; }\n"
"QComboBox::drop-down { border: none; width: 25px; }\n"
"QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #8B6F47; margin-top: 2px; }\n"
"QComboBox QAbstractItemView { background-color: #332211; color: white; selection-background-color: #8B6F47; outline: none; }"));
        lbl_equipment_label = new QLabel(group_submit_review);
        lbl_equipment_label->setObjectName("lbl_equipment_label");
        lbl_equipment_label->setGeometry(QRect(390, 30, 100, 28));
        lbl_equipment_label->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; color: #D4AF37;"));
        cb_equipment_rating = new QComboBox(group_submit_review);
        cb_equipment_rating->setObjectName("cb_equipment_rating");
        cb_equipment_rating->setGeometry(QRect(490, 30, 250, 28));
        cb_equipment_rating->setStyleSheet(QString::fromUtf8("QComboBox { border: 2px solid #8B6F47; border-radius: 6px; padding: 2px 5px; background: rgba(0,0,0,0.4); color: white; font-size: 13px; }\n"
"QComboBox::drop-down { border: none; width: 25px; }\n"
"QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #8B6F47; margin-top: 2px; }\n"
"QComboBox QAbstractItemView { background-color: #332211; color: white; selection-background-color: #8B6F47; outline: none; }"));
        lbl_rating_label = new QLabel(group_submit_review);
        lbl_rating_label->setObjectName("lbl_rating_label");
        lbl_rating_label->setGeometry(QRect(760, 30, 120, 28));
        lbl_rating_label->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; color: #D4AF37;"));
        sb_rating = new QSpinBox(group_submit_review);
        sb_rating->setObjectName("sb_rating");
        sb_rating->setGeometry(QRect(885, 30, 60, 28));
        sb_rating->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; border: 2px solid #8B6F47; border-radius: 6px; padding: 2px 5px; background: rgba(0,0,0,0.3); color: white;"));
        sb_rating->setMinimum(1);
        sb_rating->setMaximum(5);
        sb_rating->setValue(5);
        lbl_rating_stars = new QLabel(group_submit_review);
        lbl_rating_stars->setObjectName("lbl_rating_stars");
        lbl_rating_stars->setGeometry(QRect(955, 28, 150, 32));
        lbl_rating_stars->setStyleSheet(QString::fromUtf8("font-size: 22px; color: #FFD700; font-family: 'Segoe UI Symbol', 'Arial';"));
        lbl_comment_label = new QLabel(group_submit_review);
        lbl_comment_label->setObjectName("lbl_comment_label");
        lbl_comment_label->setGeometry(QRect(20, 80, 120, 28));
        lbl_comment_label->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; color: #D4AF37;"));
        le_comment = new QLineEdit(group_submit_review);
        le_comment->setObjectName("le_comment");
        le_comment->setGeometry(QRect(150, 80, 700, 34));
        le_comment->setStyleSheet(QString::fromUtf8("border: 2px solid #8B6F47; border-radius: 8px; padding: 4px 10px; font-size: 14px; background: rgba(0,0,0,0.3); color: white;"));
        btn_submit_review = new QPushButton(group_submit_review);
        btn_submit_review->setObjectName("btn_submit_review");
        btn_submit_review->setGeometry(QRect(870, 130, 170, 40));
        btn_submit_review->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; color: white; font-weight: bold; font-size: 14px; border-radius: 8px; border: none; }\n"
"        QPushButton:hover { background-color: #A0825A; }\n"
"        QPushButton:pressed { background-color: #6B5030; }"));
        btn_refresh_reviews = new QPushButton(group_submit_review);
        btn_refresh_reviews->setObjectName("btn_refresh_reviews");
        btn_refresh_reviews->setGeometry(QRect(1050, 130, 55, 40));
        btn_refresh_reviews->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #DDD; color: #555; font-weight: bold; font-size: 18px; border-radius: 8px; border: none; }\n"
"        QPushButton:hover { background-color: #CCC; }"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/assets/refresh.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_refresh_reviews->setIcon(icon7);
        btn_refresh_reviews->setIconSize(QSize(24, 24));
        frame_distribution = new QFrame(tab_reviews);
        frame_distribution->setObjectName("frame_distribution");
        frame_distribution->setGeometry(QRect(30, 425, 300, 310));
        frame_distribution->setStyleSheet(QString::fromUtf8("background: rgba(60, 45, 30, 0.7); border: 2px solid #8B6F47; border-radius: 12px;"));
        lbl_dist_title = new QLabel(frame_distribution);
        lbl_dist_title->setObjectName("lbl_dist_title");
        lbl_dist_title->setGeometry(QRect(0, 10, 300, 30));
        lbl_dist_title->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #D4AF37; border: none; font-size: 15px;"));
        lbl_dist_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_row_5 = new QLabel(frame_distribution);
        lbl_row_5->setObjectName("lbl_row_5");
        lbl_row_5->setGeometry(QRect(10, 50, 50, 20));
        lbl_row_5->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        pb_dist_5 = new QProgressBar(frame_distribution);
        pb_dist_5->setObjectName("pb_dist_5");
        pb_dist_5->setGeometry(QRect(60, 50, 220, 20));
        pb_dist_5->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 10px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #D4AF37; border-radius: 8px; }"));
        pb_dist_5->setValue(80);
        lbl_row_4 = new QLabel(frame_distribution);
        lbl_row_4->setObjectName("lbl_row_4");
        lbl_row_4->setGeometry(QRect(10, 80, 50, 20));
        lbl_row_4->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        pb_dist_4 = new QProgressBar(frame_distribution);
        pb_dist_4->setObjectName("pb_dist_4");
        pb_dist_4->setGeometry(QRect(60, 80, 220, 20));
        pb_dist_4->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 10px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #D4AF37; border-radius: 8px; }"));
        pb_dist_4->setValue(15);
        lbl_row_3 = new QLabel(frame_distribution);
        lbl_row_3->setObjectName("lbl_row_3");
        lbl_row_3->setGeometry(QRect(10, 110, 50, 20));
        lbl_row_3->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        pb_dist_3 = new QProgressBar(frame_distribution);
        pb_dist_3->setObjectName("pb_dist_3");
        pb_dist_3->setGeometry(QRect(60, 110, 220, 20));
        pb_dist_3->setStyleSheet(QString::fromUtf8("QProgressBar { background-color: rgba(0,0,0,0.5); border: 1px solid #8B6F47; border-radius: 10px; text-align: right; color: transparent; }\n"
"QProgressBar::chunk { background-color: #D4AF37; border-radius: 8px; }"));
        pb_dist_3->setValue(4);
        lbl_dist_insight = new QLabel(frame_distribution);
        lbl_dist_insight->setObjectName("lbl_dist_insight");
        lbl_dist_insight->setGeometry(QRect(10, 450, 280, 60));
        lbl_dist_insight->setStyleSheet(QString::fromUtf8("color: #4CAF50; font-style: italic; border: none; font-weight: bold;"));
        lbl_dist_insight->setWordWrap(true);
        table_reviews = new QTableView(tab_reviews);
        table_reviews->setObjectName("table_reviews");
        table_reviews->setGeometry(QRect(350, 425, 801, 310));
        table_reviews->setStyleSheet(QString::fromUtf8("\n"
"       QTableView { background-color: rgba(30, 20, 10, 0.8); border: 2px solid #8B6F47; border-radius: 12px; gridline-color: #444; color: white; selection-background-color: #8B6F47; }\n"
"       QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 10px; }\n"
"      "));
        tabWidget->addTab(tab_reviews, QString());
        btn_return_home = new QPushButton(SupplierManagement);
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

        retranslateUi(SupplierManagement);
        QObject::connect(btn_help_gestion, &QToolButton::toggled, lbl_hint_gestion, &QLabel::setVisible);
        QObject::connect(btn_help_stats, &QToolButton::toggled, lbl_hint_stats, &QLabel::setVisible);
        QObject::connect(btn_help_reviews, &QToolButton::toggled, lbl_hint_reviews, &QLabel::setVisible);

        tabWidget->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(SupplierManagement);
    } // setupUi

    void retranslateUi(QWidget *SupplierManagement)
    {
        SupplierManagement->setWindowTitle(QCoreApplication::translate("SupplierManagement", "Supplier Management", nullptr));
        groupBox_gestion->setTitle(QCoreApplication::translate("SupplierManagement", "Supplier Management", nullptr));
        label_id->setText(QCoreApplication::translate("SupplierManagement", "Supplier ID:", nullptr));
        label_nom->setText(QCoreApplication::translate("SupplierManagement", "Company Name:", nullptr));
        label_adresse->setText(QCoreApplication::translate("SupplierManagement", "Address:", nullptr));
        label_email->setText(QCoreApplication::translate("SupplierManagement", "Email Address:", nullptr));
        label_product_type->setText(QCoreApplication::translate("SupplierManagement", "Product Type:", nullptr));
        label_type->setText(QCoreApplication::translate("SupplierManagement", "Type:", nullptr));
        label_cp->setText(QCoreApplication::translate("SupplierManagement", "Postal Code:", nullptr));
        label_tel->setText(QCoreApplication::translate("SupplierManagement", "Phone Number:", nullptr));
        lbl_image_preview->setText(QCoreApplication::translate("SupplierManagement", "No image", nullptr));
        btn_upload_image->setText(QCoreApplication::translate("SupplierManagement", "Choose Image", nullptr));
        btn_add->setText(QCoreApplication::translate("SupplierManagement", "Add", nullptr));
        btn_modify->setText(QCoreApplication::translate("SupplierManagement", "Modify", nullptr));
        btn_delete->setText(QCoreApplication::translate("SupplierManagement", "Delete", nullptr));
        btn_cancel_gestion->setText(QCoreApplication::translate("SupplierManagement", "Cancel", nullptr));
        btn_clear->setText(QCoreApplication::translate("SupplierManagement", "Clear Fields", nullptr));
        label_sms->setText(QCoreApplication::translate("SupplierManagement", "SMS Message:", nullptr));
        btn_send_sms->setText(QCoreApplication::translate("SupplierManagement", "Send SMS", nullptr));
        btn_help_gestion->setText(QCoreApplication::translate("SupplierManagement", "?", nullptr));
        lbl_hint_gestion->setText(QCoreApplication::translate("SupplierManagement", "Quick Help: Use 'Add' to create new suppliers. Select a row in the 'View' tab to 'Modify' or 'Delete' here. Email and Phone are required for SMS/Communication features.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_gestion), QCoreApplication::translate("SupplierManagement", "Supplier Management", nullptr));
        lbl_stats_title->setText(QCoreApplication::translate("SupplierManagement", "Supplier Insights Dashboard", nullptr));
        btn_help_stats->setText(QCoreApplication::translate("SupplierManagement", "?", nullptr));
        lbl_hint_stats->setText(QCoreApplication::translate("SupplierManagement", "Dashboard Tip: Circular charts show general health. Bar charts below show regional performance. Hover over icons for detailed metrics.", nullptr));
        lbl_percent_retention->setText(QCoreApplication::translate("SupplierManagement", "92%", nullptr));
        lbl_text_retention->setText(QCoreApplication::translate("SupplierManagement", "RETENTION", nullptr));
        lbl_percent_accuracy->setText(QCoreApplication::translate("SupplierManagement", "87%", nullptr));
        lbl_text_accuracy->setText(QCoreApplication::translate("SupplierManagement", "ACCURACY", nullptr));
        group_performance_bars->setTitle(QCoreApplication::translate("SupplierManagement", "Performance Metrics", nullptr));
        lbl_bar_quality->setText(QCoreApplication::translate("SupplierManagement", "Product Quality:", nullptr));
        lbl_bar_speed->setText(QCoreApplication::translate("SupplierManagement", "Delivery Speed:", nullptr));
        lbl_summary_title->setText(QCoreApplication::translate("SupplierManagement", "Network Status", nullptr));
        lbl_summary_val->setText(QCoreApplication::translate("SupplierManagement", "Active: 12 | Inactive: 2 | Performance: optimal\n"
"Last updated: Today 01:40 AM", nullptr));
        lbl_chart_types->setText(QCoreApplication::translate("SupplierManagement", "Product Categories (%)", nullptr));
        lbl_chart_reviews->setText(QCoreApplication::translate("SupplierManagement", "Monthly Satisfaction Trend", nullptr));
        group_regional_stats->setTitle(QCoreApplication::translate("SupplierManagement", "Regional Performance Comparison", nullptr));
        lbl_reg_1->setText(QCoreApplication::translate("SupplierManagement", "North Region:", nullptr));
        lbl_reg_2->setText(QCoreApplication::translate("SupplierManagement", "South Region:", nullptr));
        lbl_top_performer->setText(QCoreApplication::translate("SupplierManagement", "?? Top Performer: Global Logistics Corp\n"
"Compliance Score: 99.8%\n"
"Consistency Index: high", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_stats), QCoreApplication::translate("SupplierManagement", "Statistics", nullptr));
        lbl_title_view->setText(QCoreApplication::translate("SupplierManagement", "Supplier List", nullptr));
        btn_chercher->setText(QCoreApplication::translate("SupplierManagement", "Search by Name", nullptr));
        le_recherche->setPlaceholderText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_view), QCoreApplication::translate("SupplierManagement", "View Supplier", nullptr));
        lbl_reviews_title->setText(QCoreApplication::translate("SupplierManagement", "Supplier Reliability & Feedback", nullptr));
        btn_help_reviews->setText(QCoreApplication::translate("SupplierManagement", "?", nullptr));
        lbl_hint_reviews->setText(QCoreApplication::translate("SupplierManagement", "Review Tip: Selection impacts both the 'Satisfaction Breakdown' and 'Rating Distribution' charts. The table below lists all individual feedback comments.", nullptr));
        lbl_select_supplier->setText(QCoreApplication::translate("SupplierManagement", "Select Supplier:", nullptr));
        lbl_avg_score->setText(QCoreApplication::translate("SupplierManagement", "4.8", nullptr));
        lbl_stars_row->setText(QCoreApplication::translate("SupplierManagement", "?????", nullptr));
        lbl_review_count->setText(QCoreApplication::translate("SupplierManagement", "Based on 24 customer reviews", nullptr));
        group_feedback_breakdown->setTitle(QCoreApplication::translate("SupplierManagement", "Satisfaction Breakdown", nullptr));
        lbl_cat_quality->setText(QCoreApplication::translate("SupplierManagement", "Quality:", nullptr));
        lbl_cat_response->setText(QCoreApplication::translate("SupplierManagement", "Response:", nullptr));
        lbl_cat_price->setText(QCoreApplication::translate("SupplierManagement", "Price Value:", nullptr));
        group_submit_review->setTitle(QCoreApplication::translate("SupplierManagement", "Log Delivery Rating", nullptr));
        lbl_employee_label->setText(QCoreApplication::translate("SupplierManagement", "Employee:", nullptr));
        lbl_equipment_label->setText(QCoreApplication::translate("SupplierManagement", "Equipment:", nullptr));
        lbl_rating_label->setText(QCoreApplication::translate("SupplierManagement", "Rating (1-5):", nullptr));
        lbl_rating_stars->setText(QCoreApplication::translate("SupplierManagement", "\342\230\205\342\230\205\342\230\205\342\230\205\342\230\205", nullptr));
        lbl_comment_label->setText(QCoreApplication::translate("SupplierManagement", "Delivery Note:", nullptr));
        le_comment->setPlaceholderText(QCoreApplication::translate("SupplierManagement", "e.g. On time, good packaging, correct items...", nullptr));
        btn_submit_review->setText(QCoreApplication::translate("SupplierManagement", "\342\234\223 Log Rating", nullptr));
        btn_refresh_reviews->setText(QString());
#if QT_CONFIG(tooltip)
        btn_refresh_reviews->setToolTip(QCoreApplication::translate("SupplierManagement", "Refresh ratings", nullptr));
#endif // QT_CONFIG(tooltip)
        lbl_dist_title->setText(QCoreApplication::translate("SupplierManagement", "Rating Distribution", nullptr));
        lbl_row_5->setText(QCoreApplication::translate("SupplierManagement", "5 Star", nullptr));
        lbl_row_4->setText(QCoreApplication::translate("SupplierManagement", "4 Star", nullptr));
        lbl_row_3->setText(QCoreApplication::translate("SupplierManagement", "3 Star", nullptr));
        lbl_dist_insight->setText(QCoreApplication::translate("SupplierManagement", "Trend: Positive feedback has increased by 12% in the last 30 days.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_reviews), QCoreApplication::translate("SupplierManagement", "Supplier Reviews", nullptr));
        btn_return_home->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class SupplierManagement: public Ui_SupplierManagement {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUPPLIER_MANAGEMENT_H
