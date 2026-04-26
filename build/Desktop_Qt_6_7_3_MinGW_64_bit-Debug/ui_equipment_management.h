/********************************************************************************
** Form generated from reading UI file 'equipment_management.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EQUIPMENT_MANAGEMENT_H
#define UI_EQUIPMENT_MANAGEMENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_EquipmentManagement
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_gestion;
    QGroupBox *groupBox_gestion;
    QLabel *label_id;
    QLineEdit *le_id;
    QLabel *label_type;
    QLineEdit *le_type;
    QLabel *label_date_achat;
    QDateEdit *de_date_achat;
    QLabel *label_unit_price;
    QDoubleSpinBox *dsb_unit_price;
    QLabel *label_etat;
    QComboBox *cb_status;
    QLabel *label_desc;
    QTextEdit *te_desc;
    QLabel *label_quantity;
    QSpinBox *sb_quantity;
    QPushButton *btn_add;
    QPushButton *btn_modify;
    QPushButton *btn_delete;
    QPushButton *btn_clear;
    QToolButton *btn_help_gestion;
    QLabel *lbl_hint_gestion;
    QWidget *tab_view;
    QToolButton *btn_help_view;
    QLabel *label_titre_liste_equip;
    QPushButton *btn_delete_confirm;
    QPushButton *btn_share_chat;
    QComboBox *cb_filter_status;
    QPushButton *btn_search;
    QLineEdit *le_recherche;
    QTableView *table_equipments;
    QGroupBox *group_bulk_actions;
    QLabel *label_bulk_instr;
    QComboBox *cb_bulk_status;
    QPushButton *btn_bulk_update_status;
    QWidget *tab_history;
    QLabel *label_titre_histo;
    QLineEdit *le_history_search;
    QPushButton *btn_history_search;
    QTabWidget *tabWidget_history_sections;
    QWidget *tab_history_add;
    QTableView *tableView_history_add;
    QWidget *tab_history_modify;
    QTableView *tableView_history_modify;
    QWidget *tab_history_delete;
    QTableView *tableView_historique;
    QPushButton *btn_refresh_history;
    QPushButton *btn_export_history;
    QPushButton *btn_clear_history;
    QWidget *tab_stats;
    QLabel *label_stats_title;
    QGroupBox *group_metrics;
    QLabel *lbl_total_eq;
    QLabel *val_total_eq;
    QLabel *lbl_oper_eq;
    QLabel *val_oper_eq;
    QLabel *lbl_broken_eq;
    QLabel *val_broken_eq;
    QLabel *lbl_usage_hours;
    QLabel *val_usage_hours;
    QLabel *lbl_maintenance;
    QLabel *val_maintenance;
    QGroupBox *group_chart;
    QLabel *label_chart_placeholder;
    QToolButton *btn_help_stats;
    QLabel *lbl_hint_stats;
    QPushButton *btn_export_stats;
    QWidget *tab_chat;
    QWidget *chat_outer_container;
    QHBoxLayout *horizontalLayout_chat_outer;
    QFrame *frame_employees;
    QVBoxLayout *verticalLayout_emp_list;
    QLabel *lbl_sidebar_header;
    QLineEdit *le_chat_search;
    QListWidget *list_employees;
    QFrame *frame_chat_panel;
    QVBoxLayout *verticalLayout_chat_main;
    QFrame *frame_chat_header;
    QHBoxLayout *horizontalLayout_header;
    QLabel *lbl_partner_avatar;
    QVBoxLayout *verticalLayout_header_text;
    QLabel *lbl_active_employees;
    QLabel *lbl_chat_status;
    QSpacerItem *spacer_header;
    QPushButton *btn_chat_settings;
    QPushButton *btn_chat_music;
    QPushButton *btn_weather_assistant;
    QPushButton *btn_weather_bot;
    QPushButton *btn_chat_refresh;
    QScrollArea *scrollArea_chat;
    QWidget *chatScrollAreaContents;
    QVBoxLayout *verticalLayout_chat_contents;
    QSpacerItem *verticalSpacer_chat;
    QLabel *lbl_img_preview;
    QFrame *frame_input_bar;
    QHBoxLayout *horizontalLayout_input;
    QPushButton *btn_chat_img;
    QPushButton *btn_chat_voice;
    QLineEdit *le_chat_input;
    QPushButton *btn_chat_send;
    QPushButton *btn_return_home;

    void setupUi(QWidget *EquipmentManagement)
    {
        if (EquipmentManagement->objectName().isEmpty())
            EquipmentManagement->setObjectName("EquipmentManagement");
        EquipmentManagement->resize(1322, 800);
        EquipmentManagement->setStyleSheet(QString::fromUtf8("\n"
"    #EquipmentManagement {\n"
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
        tabWidget = new QTabWidget(EquipmentManagement);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(118, 70, 1051, 681));
        QFont font;
        font.setFamilies({QString::fromUtf8("Gadugi")});
        font.setPointSize(10);
        font.setBold(true);
        tabWidget->setFont(font);
        tabWidget->setStyleSheet(QString::fromUtf8("\n"
"     QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"     QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"     QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"     QWidget#tab_gestion, QWidget#tab_view, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_chat { background: transparent; }\n"
"    "));
        tab_gestion = new QWidget();
        tab_gestion->setObjectName("tab_gestion");
        groupBox_gestion = new QGroupBox(tab_gestion);
        groupBox_gestion->setObjectName("groupBox_gestion");
        groupBox_gestion->setGeometry(QRect(30, 40, 615, 600));
        groupBox_gestion->setStyleSheet(QString::fromUtf8("\n"
"        QGroupBox#groupBox_gestion {\n"
"          background-color: rgba(60, 45, 30, 0.7);\n"
"          border: 2px solid #8B6F47;\n"
"          border-radius: 12px;\n"
"          margin-top: 18px;\n"
"          color: white;\n"
"        }\n"
"        QGroupBox#groupBox_gestion::title {\n"
"          subcontrol-origin: margin;\n"
"          subcontrol-position: top left;\n"
"          padding: 2px 12px;\n"
"          background-color: #8B6F47;\n"
"          font-weight: bold;\n"
"          color: white;\n"
"          border-radius: 4px;\n"
"        }\n"
"       "));
        label_id = new QLabel(groupBox_gestion);
        label_id->setObjectName("label_id");
        label_id->setGeometry(QRect(70, 70, 150, 40));
        label_id->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_id->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        le_id = new QLineEdit(groupBox_gestion);
        le_id->setObjectName("le_id");
        le_id->setGeometry(QRect(230, 70, 280, 40));
        label_type = new QLabel(groupBox_gestion);
        label_type->setObjectName("label_type");
        label_type->setGeometry(QRect(70, 125, 150, 40));
        label_type->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_type->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        le_type = new QLineEdit(groupBox_gestion);
        le_type->setObjectName("le_type");
        le_type->setGeometry(QRect(230, 125, 280, 40));
        label_date_achat = new QLabel(groupBox_gestion);
        label_date_achat->setObjectName("label_date_achat");
        label_date_achat->setGeometry(QRect(70, 180, 150, 40));
        label_date_achat->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_date_achat->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        de_date_achat = new QDateEdit(groupBox_gestion);
        de_date_achat->setObjectName("de_date_achat");
        de_date_achat->setGeometry(QRect(230, 180, 280, 40));
        de_date_achat->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;"));
        label_unit_price = new QLabel(groupBox_gestion);
        label_unit_price->setObjectName("label_unit_price");
        label_unit_price->setGeometry(QRect(70, 235, 150, 40));
        label_unit_price->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_unit_price->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        dsb_unit_price = new QDoubleSpinBox(groupBox_gestion);
        dsb_unit_price->setObjectName("dsb_unit_price");
        dsb_unit_price->setGeometry(QRect(230, 235, 280, 40));
        dsb_unit_price->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;"));
        dsb_unit_price->setMaximum(999999999.990000009536743);
        dsb_unit_price->setButtonSymbols(QAbstractSpinBox::NoButtons);
        label_etat = new QLabel(groupBox_gestion);
        label_etat->setObjectName("label_etat");
        label_etat->setGeometry(QRect(70, 290, 150, 40));
        label_etat->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_etat->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        cb_status = new QComboBox(groupBox_gestion);
        cb_status->addItem(QString());
        cb_status->addItem(QString());
        cb_status->addItem(QString());
        cb_status->addItem(QString());
        cb_status->setObjectName("cb_status");
        cb_status->setGeometry(QRect(230, 290, 280, 40));
        cb_status->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;"));
        label_desc = new QLabel(groupBox_gestion);
        label_desc->setObjectName("label_desc");
        label_desc->setGeometry(QRect(70, 345, 150, 40));
        label_desc->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_desc->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        te_desc = new QTextEdit(groupBox_gestion);
        te_desc->setObjectName("te_desc");
        te_desc->setGeometry(QRect(230, 345, 280, 120));
        te_desc->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;"));
        label_quantity = new QLabel(groupBox_gestion);
        label_quantity->setObjectName("label_quantity");
        label_quantity->setGeometry(QRect(70, 475, 150, 30));
        label_quantity->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold; background: transparent;"));
        label_quantity->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        sb_quantity = new QSpinBox(groupBox_gestion);
        sb_quantity->setObjectName("sb_quantity");
        sb_quantity->setGeometry(QRect(230, 475, 280, 30));
        sb_quantity->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; background-color: white;"));
        sb_quantity->setMaximum(999999);
        btn_add = new QPushButton(groupBox_gestion);
        btn_add->setObjectName("btn_add");
        btn_add->setGeometry(QRect(30, 545, 130, 45));
        btn_add->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"             "));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/add.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_add->setIcon(icon);
        btn_add->setIconSize(QSize(24, 24));
        btn_modify = new QPushButton(groupBox_gestion);
        btn_modify->setObjectName("btn_modify");
        btn_modify->setGeometry(QRect(170, 545, 130, 45));
        btn_modify->setStyleSheet(QString::fromUtf8("\n"
"                QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"                QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"             "));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/assets/modify.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_modify->setIcon(icon1);
        btn_modify->setIconSize(QSize(24, 24));
        btn_delete = new QPushButton(groupBox_gestion);
        btn_delete->setObjectName("btn_delete");
        btn_delete->setGeometry(QRect(310, 545, 130, 45));
        btn_delete->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/assets/icon_delete_order.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_delete->setIcon(icon2);
        btn_delete->setIconSize(QSize(24, 24));
        btn_clear = new QPushButton(groupBox_gestion);
        btn_clear->setObjectName("btn_clear");
        btn_clear->setGeometry(QRect(450, 545, 130, 45));
        btn_clear->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"        "));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/assets/icon_clear_fields.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_clear->setIcon(icon3);
        btn_clear->setIconSize(QSize(24, 24));
        btn_help_gestion = new QToolButton(groupBox_gestion);
        btn_help_gestion->setObjectName("btn_help_gestion");
        btn_help_gestion->setGeometry(QRect(980, 20, 32, 32));
        btn_help_gestion->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_gestion->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_gestion->setCheckable(true);
        lbl_hint_gestion = new QLabel(groupBox_gestion);
        lbl_hint_gestion->setObjectName("lbl_hint_gestion");
        lbl_hint_gestion->setGeometry(QRect(110, 560, 800, 60));
        lbl_hint_gestion->setVisible(false);
        lbl_hint_gestion->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        tabWidget->addTab(tab_gestion, QString());
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
        btn_help_view = new QToolButton(tab_view);
        btn_help_view->setObjectName("btn_help_view");
        btn_help_view->setGeometry(QRect(980, 20, 30, 30));
        btn_help_view->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_view->setStyleSheet(QString::fromUtf8("\n"
"          QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"          QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"         "));
        btn_help_view->setCheckable(true);
        label_titre_liste_equip = new QLabel(tab_view);
        label_titre_liste_equip->setObjectName("label_titre_liste_equip");
        label_titre_liste_equip->setGeometry(QRect(700, 20, 300, 30));
        label_titre_liste_equip->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;\n"
""));
        label_titre_liste_equip->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_delete_confirm = new QPushButton(tab_view);
        btn_delete_confirm->setObjectName("btn_delete_confirm");
        btn_delete_confirm->setGeometry(QRect(20, 80, 90, 30));
        btn_delete_confirm->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        btn_delete_confirm->setIcon(icon2);
        btn_delete_confirm->setIconSize(QSize(20, 20));
        btn_share_chat = new QPushButton(tab_view);
        btn_share_chat->setObjectName("btn_share_chat");
        btn_share_chat->setGeometry(QRect(120, 80, 90, 30));
        btn_share_chat->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        cb_filter_status = new QComboBox(tab_view);
        cb_filter_status->addItem(QString());
        cb_filter_status->addItem(QString());
        cb_filter_status->addItem(QString());
        cb_filter_status->addItem(QString());
        cb_filter_status->addItem(QString());
        cb_filter_status->setObjectName("cb_filter_status");
        cb_filter_status->setGeometry(QRect(220, 80, 130, 30));
        cb_filter_status->setStyleSheet(QString::fromUtf8("border: 1px solid #8B6F47; border-radius: 4px; padding: 2px; background-color: white;"));
        btn_search = new QPushButton(tab_view);
        btn_search->setObjectName("btn_search");
        btn_search->setGeometry(QRect(360, 80, 90, 30));
        btn_search->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/assets/icon_search.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_search->setIcon(icon4);
        btn_search->setIconSize(QSize(20, 20));
        le_recherche = new QLineEdit(tab_view);
        le_recherche->setObjectName("le_recherche");
        le_recherche->setGeometry(QRect(460, 80, 550, 30));
        table_equipments = new QTableView(tab_view);
        table_equipments->setObjectName("table_equipments");
        table_equipments->setGeometry(QRect(20, 120, 1000, 400));
        table_equipments->setStyleSheet(QString::fromUtf8("QTableView { background-color: white; border: 1px solid #8B6F47; border-radius: 8px; gridline-color: #D3C1A5; } QHeaderView::section { background-color: #8B6F47; color: white; padding: 4px; border: 1px solid #705A3A; font-weight: bold; }"));
        table_equipments->setSelectionMode(QAbstractItemView::MultiSelection);
        table_equipments->setSelectionBehavior(QAbstractItemView::SelectRows);
        group_bulk_actions = new QGroupBox(tab_view);
        group_bulk_actions->setObjectName("group_bulk_actions");
        group_bulk_actions->setGeometry(QRect(20, 530, 1000, 70));
        group_bulk_actions->setStyleSheet(QString::fromUtf8("QGroupBox { background-color: transparent; border: 1.5px dashed #8B6F47; border-radius: 8px; margin-top: 10px; color: white; font-weight: bold; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"));
        label_bulk_instr = new QLabel(group_bulk_actions);
        label_bulk_instr->setObjectName("label_bulk_instr");
        label_bulk_instr->setGeometry(QRect(15, 20, 320, 30));
        label_bulk_instr->setStyleSheet(QString::fromUtf8("color: #E0E0E0; font-size: 13px;"));
        cb_bulk_status = new QComboBox(group_bulk_actions);
        cb_bulk_status->addItem(QString());
        cb_bulk_status->addItem(QString());
        cb_bulk_status->addItem(QString());
        cb_bulk_status->addItem(QString());
        cb_bulk_status->setObjectName("cb_bulk_status");
        cb_bulk_status->setGeometry(QRect(340, 20, 200, 30));
        cb_bulk_status->setStyleSheet(QString::fromUtf8("QComboBox { border: 1px solid #8B6F47; border-radius: 5px; padding: 2px 10px; background: white; } QComboBox:hover { border-color: #A0825A; }"));
        btn_bulk_update_status = new QPushButton(group_bulk_actions);
        btn_bulk_update_status->setObjectName("btn_bulk_update_status");
        btn_bulk_update_status->setGeometry(QRect(560, 20, 160, 30));
        btn_bulk_update_status->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }"));
        btn_bulk_update_status->setIcon(icon1);
        tabWidget->addTab(tab_view, QString());
        tab_history = new QWidget();
        tab_history->setObjectName("tab_history");
        tab_history->setStyleSheet(QString::fromUtf8("\n"
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
        label_titre_histo = new QLabel(tab_history);
        label_titre_histo->setObjectName("label_titre_histo");
        label_titre_histo->setGeometry(QRect(700, 20, 300, 30));
        label_titre_histo->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: bold; color: white;"));
        label_titre_histo->setAlignment(Qt::AlignmentFlag::AlignCenter);
        le_history_search = new QLineEdit(tab_history);
        le_history_search->setObjectName("le_history_search");
        le_history_search->setGeometry(QRect(20, 70, 300, 30));
        btn_history_search = new QPushButton(tab_history);
        btn_history_search->setObjectName("btn_history_search");
        btn_history_search->setGeometry(QRect(330, 70, 100, 30));
        btn_history_search->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 5px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        tabWidget_history_sections = new QTabWidget(tab_history);
        tabWidget_history_sections->setObjectName("tabWidget_history_sections");
        tabWidget_history_sections->setGeometry(QRect(20, 115, 1000, 460));
        tabWidget_history_sections->setStyleSheet(QString::fromUtf8("\n"
"        QTabWidget::pane { border: 2px solid #8B6F47; background: white; }\n"
"        QTabBar::tab { background: #8B6F47; padding: 9px 24px; font-weight: bold; color: white; border-radius: 4px 4px 0 0; margin-right: 3px; font-size: 12px; } QTabBar::tab:selected { background: #6B5237; border: 1.5px solid white; }\n"
"       "));
        tab_history_add = new QWidget();
        tab_history_add->setObjectName("tab_history_add");
        tableView_history_add = new QTableView(tab_history_add);
        tableView_history_add->setObjectName("tableView_history_add");
        tableView_history_add->setGeometry(QRect(0, 0, 998, 424));
        tableView_history_add->setStyleSheet(QString::fromUtf8("\n"
"          QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 6px; font-size: 12px; }\n"
"          QTableView { border: none; selection-background-color: #F4E4C1; selection-color: #2A1E10; background-color: white; alternate-background-color: #FAF3E0; gridline-color: #ddd;  }\n"
"         "));
        tableView_history_add->setAlternatingRowColors(true);
        tabWidget_history_sections->addTab(tab_history_add, QString());
        tab_history_modify = new QWidget();
        tab_history_modify->setObjectName("tab_history_modify");
        tableView_history_modify = new QTableView(tab_history_modify);
        tableView_history_modify->setObjectName("tableView_history_modify");
        tableView_history_modify->setGeometry(QRect(0, 0, 998, 424));
        tableView_history_modify->setStyleSheet(QString::fromUtf8("\n"
"          QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 6px; font-size: 12px; }\n"
"          QTableView { border: none; selection-background-color: #ffe0b2; selection-color: #bf360c; background-color: white; alternate-background-color: #fff3e0; gridline-color: #ddd;  }\n"
"         "));
        tableView_history_modify->setAlternatingRowColors(true);
        tabWidget_history_sections->addTab(tab_history_modify, QString());
        tab_history_delete = new QWidget();
        tab_history_delete->setObjectName("tab_history_delete");
        tableView_historique = new QTableView(tab_history_delete);
        tableView_historique->setObjectName("tableView_historique");
        tableView_historique->setGeometry(QRect(0, 0, 998, 424));
        tableView_historique->setStyleSheet(QString::fromUtf8("\n"
"          QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 6px; font-size: 12px; }\n"
"          QTableView { border: none; selection-background-color: #ffcdd2; selection-color: #b71c1c; background-color: white; alternate-background-color: #fff8f8; gridline-color: #ddd;  }\n"
"         "));
        tableView_historique->setAlternatingRowColors(true);
        tabWidget_history_sections->addTab(tab_history_delete, QString());
        btn_refresh_history = new QPushButton(tab_history);
        btn_refresh_history->setObjectName("btn_refresh_history");
        btn_refresh_history->setGeometry(QRect(20, 580, 150, 40));
        btn_refresh_history->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/assets/refresh.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_refresh_history->setIcon(icon5);
        btn_refresh_history->setIconSize(QSize(24, 24));
        btn_export_history = new QPushButton(tab_history);
        btn_export_history->setObjectName("btn_export_history");
        btn_export_history->setGeometry(QRect(180, 580, 150, 40));
        btn_export_history->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/assets/pdf_export.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_export_history->setIcon(icon6);
        btn_export_history->setIconSize(QSize(24, 24));
        btn_clear_history = new QPushButton(tab_history);
        btn_clear_history->setObjectName("btn_clear_history");
        btn_clear_history->setGeometry(QRect(870, 580, 150, 40));
        btn_clear_history->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }"));
        btn_clear_history->setIcon(icon3);
        btn_clear_history->setIconSize(QSize(24, 24));
        tabWidget->addTab(tab_history, QString());
        tab_stats = new QWidget();
        tab_stats->setObjectName("tab_stats");
        label_stats_title = new QLabel(tab_stats);
        label_stats_title->setObjectName("label_stats_title");
        label_stats_title->setGeometry(QRect(700, 20, 350, 40));
        label_stats_title->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: white;"));
        label_stats_title->setAlignment(Qt::AlignmentFlag::AlignCenter);
        group_metrics = new QGroupBox(tab_stats);
        group_metrics->setObjectName("group_metrics");
        group_metrics->setGeometry(QRect(50, 80, 450, 350));
        group_metrics->setStyleSheet(QString::fromUtf8("\n"
"         QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"         QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"        "));
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
        lbl_usage_hours = new QLabel(group_metrics);
        lbl_usage_hours->setObjectName("lbl_usage_hours");
        lbl_usage_hours->setGeometry(QRect(20, 200, 250, 30));
        lbl_usage_hours->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px;"));
        val_usage_hours = new QLabel(group_metrics);
        val_usage_hours->setObjectName("val_usage_hours");
        val_usage_hours->setGeometry(QRect(300, 200, 100, 30));
        val_usage_hours->setStyleSheet(QString::fromUtf8("color: #4CAF50; font-size: 18px; font-weight: bold;"));
        lbl_maintenance = new QLabel(group_metrics);
        lbl_maintenance->setObjectName("lbl_maintenance");
        lbl_maintenance->setGeometry(QRect(20, 250, 250, 30));
        lbl_maintenance->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px;"));
        val_maintenance = new QLabel(group_metrics);
        val_maintenance->setObjectName("val_maintenance");
        val_maintenance->setGeometry(QRect(300, 250, 120, 30));
        val_maintenance->setStyleSheet(QString::fromUtf8("color: #D4AF37; font-size: 18px; font-weight: bold;"));
        group_chart = new QGroupBox(tab_stats);
        group_chart->setObjectName("group_chart");
        group_chart->setGeometry(QRect(550, 80, 450, 350));
        group_chart->setStyleSheet(QString::fromUtf8("\n"
"         QGroupBox { background-color: transparent; border: 1px solid #8B6F47; border-radius: 5px; margin-top: 20px; }\n"
"         QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }\n"
"        "));
        label_chart_placeholder = new QLabel(group_chart);
        label_chart_placeholder->setObjectName("label_chart_placeholder");
        label_chart_placeholder->setGeometry(QRect(20, 40, 410, 290));
        label_chart_placeholder->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0.1); border: 1px dashed #DDD;"));
        label_chart_placeholder->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_help_stats = new QToolButton(tab_stats);
        btn_help_stats->setObjectName("btn_help_stats");
        btn_help_stats->setGeometry(QRect(980, 20, 32, 32));
        btn_help_stats->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_stats->setStyleSheet(QString::fromUtf8("\n"
"           QToolButton { background-color: #8B6F47; border-radius: 15px; color: white; font-weight: bold; border: none; }\n"
"           QToolButton:checked { background-color: white; color: #8B6F47; border: 2px solid #8B6F47; }\n"
"          "));
        btn_help_stats->setCheckable(true);
        lbl_hint_stats = new QLabel(tab_stats);
        lbl_hint_stats->setObjectName("lbl_hint_stats");
        lbl_hint_stats->setGeometry(QRect(50, 580, 650, 60));
        lbl_hint_stats->setStyleSheet(QString::fromUtf8("color: white; font-size: 13px; font-style: italic; background: rgba(255, 255, 255, 0.1); padding: 10px; border-radius: 5px;"));
        lbl_hint_stats->setWordWrap(true);
        btn_export_stats = new QPushButton(tab_stats);
        btn_export_stats->setObjectName("btn_export_stats");
        btn_export_stats->setGeometry(QRect(750, 585, 250, 45));
        btn_export_stats->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; font-size: 15px; padding-left: 10px; }\n"
"        QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        btn_export_stats->setIcon(icon6);
        btn_export_stats->setIconSize(QSize(24, 24));
        tabWidget->addTab(tab_stats, QString());
        tab_chat = new QWidget();
        tab_chat->setObjectName("tab_chat");
        chat_outer_container = new QWidget(tab_chat);
        chat_outer_container->setObjectName("chat_outer_container");
        chat_outer_container->setGeometry(QRect(0, 125, 1051, 545));
        chat_outer_container->setStyleSheet(QString::fromUtf8("QWidget#chat_outer_container { background: transparent; }"));
        horizontalLayout_chat_outer = new QHBoxLayout(chat_outer_container);
        horizontalLayout_chat_outer->setSpacing(0);
        horizontalLayout_chat_outer->setObjectName("horizontalLayout_chat_outer");
        horizontalLayout_chat_outer->setContentsMargins(30, 8, 30, 12);
        frame_employees = new QFrame(chat_outer_container);
        frame_employees->setObjectName("frame_employees");
        frame_employees->setMinimumSize(QSize(225, 0));
        frame_employees->setMaximumSize(QSize(225, 16777215));
        frame_employees->setStyleSheet(QString::fromUtf8("QFrame#frame_employees { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1E1A14, stop:1 #2A2218); border-radius: 16px 0px 0px 16px; border: 1.5px solid #5A4A32; border-right: none; }"));
        verticalLayout_emp_list = new QVBoxLayout(frame_employees);
        verticalLayout_emp_list->setSpacing(0);
        verticalLayout_emp_list->setObjectName("verticalLayout_emp_list");
        verticalLayout_emp_list->setContentsMargins(0, 0, 0, 10);
        lbl_sidebar_header = new QLabel(frame_employees);
        lbl_sidebar_header->setObjectName("lbl_sidebar_header");
        lbl_sidebar_header->setMinimumHeight(54);
        lbl_sidebar_header->setMaximumHeight(54);
        lbl_sidebar_header->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_sidebar_header->setStyleSheet(QString::fromUtf8("QLabel { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B6F47, stop:1 #C4973A); color: white; font-size: 15px; font-weight: bold; border-radius: 14px 0px 0px 0px; padding: 0 10px; border-bottom: 1px solid #5A4A32; }"));

        verticalLayout_emp_list->addWidget(lbl_sidebar_header);

        le_chat_search = new QLineEdit(frame_employees);
        le_chat_search->setObjectName("le_chat_search");
        le_chat_search->setMaximumHeight(34);
        le_chat_search->setStyleSheet(QString::fromUtf8("QLineEdit { background: rgba(255,255,255,0.07); border: 1px solid #5A4A32; border-radius: 8px; padding: 4px 10px; color: #D4C0A0; font-size: 12px; margin: 8px 10px 4px 10px; } QLineEdit:focus { border: 1px solid #D4AF37; }"));

        verticalLayout_emp_list->addWidget(le_chat_search);

        list_employees = new QListWidget(frame_employees);
        list_employees->setObjectName("list_employees");
        list_employees->setStyleSheet(QString::fromUtf8("QListWidget { background: transparent; border: none; color: #C8B89A; font-size: 13px; padding: 0 6px; } QListWidget::item { padding: 10px 8px; border-bottom: 1px solid rgba(139,111,71,0.2); border-radius: 8px; margin: 1px 0; } QListWidget::item:hover { background: rgba(139,111,71,0.25); color: white; } QListWidget::item:selected { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8B6F47,stop:1 #B8925A); color: white; border-radius: 8px; }"));

        verticalLayout_emp_list->addWidget(list_employees);


        horizontalLayout_chat_outer->addWidget(frame_employees);

        frame_chat_panel = new QFrame(chat_outer_container);
        frame_chat_panel->setObjectName("frame_chat_panel");
        frame_chat_panel->setStyleSheet(QString::fromUtf8("QFrame#frame_chat_panel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2D2416, stop:1 #1A1208); border-radius: 0px 16px 16px 0px; border: 1.5px solid #5A4A32; border-left: none; }"));
        verticalLayout_chat_main = new QVBoxLayout(frame_chat_panel);
        verticalLayout_chat_main->setSpacing(0);
        verticalLayout_chat_main->setObjectName("verticalLayout_chat_main");
        verticalLayout_chat_main->setContentsMargins(0, 0, 0, 0);
        frame_chat_header = new QFrame(frame_chat_panel);
        frame_chat_header->setObjectName("frame_chat_header");
        frame_chat_header->setMinimumHeight(64);
        frame_chat_header->setMaximumHeight(64);
        frame_chat_header->setStyleSheet(QString::fromUtf8("QFrame#frame_chat_header { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3A2D1A,stop:1 #4A3820); border-bottom: 2px solid #8B6F47; border-radius: 0px 14px 0px 0px; }"));
        horizontalLayout_header = new QHBoxLayout(frame_chat_header);
        horizontalLayout_header->setSpacing(12);
        horizontalLayout_header->setObjectName("horizontalLayout_header");
        horizontalLayout_header->setContentsMargins(18, 10, 18, 10);
        lbl_partner_avatar = new QLabel(frame_chat_header);
        lbl_partner_avatar->setObjectName("lbl_partner_avatar");
        lbl_partner_avatar->setMinimumSize(QSize(42, 42));
        lbl_partner_avatar->setMaximumSize(QSize(42, 42));
        lbl_partner_avatar->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lbl_partner_avatar->setStyleSheet(QString::fromUtf8("QLabel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #D4AF37,stop:1 #8B6F47); color: white; font-size: 16px; font-weight: bold; border-radius: 21px; border: 2px solid #D4AF37; padding: 0; }"));

        horizontalLayout_header->addWidget(lbl_partner_avatar);

        verticalLayout_header_text = new QVBoxLayout();
        verticalLayout_header_text->setSpacing(2);
        verticalLayout_header_text->setObjectName("verticalLayout_header_text");
        lbl_active_employees = new QLabel(frame_chat_header);
        lbl_active_employees->setObjectName("lbl_active_employees");
        lbl_active_employees->setStyleSheet(QString::fromUtf8("QLabel { color: #F0E0C0; font-size: 15px; font-weight: bold; background: transparent; border: none; padding: 0; }"));

        verticalLayout_header_text->addWidget(lbl_active_employees);

        lbl_chat_status = new QLabel(frame_chat_header);
        lbl_chat_status->setObjectName("lbl_chat_status");
        lbl_chat_status->setStyleSheet(QString::fromUtf8("QLabel { color: #4CAF50; font-size: 11px; background: transparent; border: none; padding: 0; }"));

        verticalLayout_header_text->addWidget(lbl_chat_status);


        horizontalLayout_header->addLayout(verticalLayout_header_text);

        spacer_header = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_header->addItem(spacer_header);

        btn_chat_settings = new QPushButton(frame_chat_header);
        btn_chat_settings->setObjectName("btn_chat_settings");
        btn_chat_settings->setMinimumSize(QSize(36, 36));
        btn_chat_settings->setMaximumSize(QSize(36, 36));
        btn_chat_settings->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.3); border: 1.5px solid #8B6F47; border-radius: 18px; color: #D4AF37; font-size: 18px; } QPushButton:hover { background: #8B6F47; color: white; }"));

        horizontalLayout_header->addWidget(btn_chat_settings);

        btn_chat_music = new QPushButton(frame_chat_header);
        btn_chat_music->setObjectName("btn_chat_music");
        btn_chat_music->setMinimumSize(QSize(36, 36));
        btn_chat_music->setMaximumSize(QSize(36, 36));
        btn_chat_music->setCheckable(true);
        btn_chat_music->setChecked(true);
        btn_chat_music->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.3); border: 1.5px solid #8B6F47; border-radius: 18px; color: #D4AF37; font-size: 18px; } QPushButton:hover { background: #8B6F47; color: white; } QPushButton:checked { background: #D4AF37; color: #1A1208; border-color: white; }"));

        horizontalLayout_header->addWidget(btn_chat_music);

        btn_weather_assistant = new QPushButton(frame_chat_header);
        btn_weather_assistant->setObjectName("btn_weather_assistant");
        btn_weather_assistant->setMinimumSize(QSize(45, 45));
        btn_weather_assistant->setMaximumSize(QSize(45, 45));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/assets/meteorology.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_weather_assistant->setIcon(icon7);
        btn_weather_assistant->setIconSize(QSize(28, 28));
        btn_weather_assistant->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.3); border: 1.5px solid #8B6F47; border-radius: 18px; color: #D4AF37; font-size: 18px; } QPushButton:hover { background: #8B6F47; color: white; }"));

        horizontalLayout_header->addWidget(btn_weather_assistant);

        btn_weather_bot = new QPushButton(frame_chat_header);
        btn_weather_bot->setObjectName("btn_weather_bot");
        btn_weather_bot->setMinimumSize(QSize(36, 36));
        btn_weather_bot->setMaximumSize(QSize(36, 36));
        btn_weather_bot->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.3); border: 1.5px solid #8B6F47; border-radius: 18px; color: #D4AF37; font-size: 18px; } QPushButton:hover { background: #8B6F47; color: white; }"));

        horizontalLayout_header->addWidget(btn_weather_bot);

        btn_chat_refresh = new QPushButton(frame_chat_header);
        btn_chat_refresh->setObjectName("btn_chat_refresh");
        btn_chat_refresh->setMinimumSize(QSize(36, 36));
        btn_chat_refresh->setMaximumSize(QSize(36, 36));
        btn_chat_refresh->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.3); border: 1.5px solid #8B6F47; border-radius: 18px; color: #D4AF37; font-size: 18px; font-weight: bold; } QPushButton:hover { background: #8B6F47; color: white; }"));

        horizontalLayout_header->addWidget(btn_chat_refresh);


        verticalLayout_chat_main->addWidget(frame_chat_header);

        scrollArea_chat = new QScrollArea(frame_chat_panel);
        scrollArea_chat->setObjectName("scrollArea_chat");
        scrollArea_chat->setWidgetResizable(true);
        scrollArea_chat->setStyleSheet(QString::fromUtf8("QScrollArea { background: transparent; border: none; } QScrollBar:vertical { background: rgba(255,255,255,0.04); width: 6px; border-radius: 3px; } QScrollBar::handle:vertical { background: rgba(139,111,71,0.6); border-radius: 3px; min-height: 20px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"));
        chatScrollAreaContents = new QWidget();
        chatScrollAreaContents->setObjectName("chatScrollAreaContents");
        chatScrollAreaContents->setStyleSheet(QString::fromUtf8("QWidget#chatScrollAreaContents { background: transparent; }"));
        verticalLayout_chat_contents = new QVBoxLayout(chatScrollAreaContents);
        verticalLayout_chat_contents->setSpacing(6);
        verticalLayout_chat_contents->setObjectName("verticalLayout_chat_contents");
        verticalLayout_chat_contents->setContentsMargins(16, 12, 16, 8);
        verticalSpacer_chat = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_chat_contents->addItem(verticalSpacer_chat);

        scrollArea_chat->setWidget(chatScrollAreaContents);

        verticalLayout_chat_main->addWidget(scrollArea_chat);

        lbl_img_preview = new QLabel(frame_chat_panel);
        lbl_img_preview->setObjectName("lbl_img_preview");
        lbl_img_preview->setMaximumHeight(28);
        lbl_img_preview->setVisible(false);
        lbl_img_preview->setStyleSheet(QString::fromUtf8("QLabel { background: rgba(212,175,55,0.15); color: #D4AF37; font-size: 12px; padding: 4px 16px; border-top: 1px solid rgba(212,175,55,0.3); }"));

        verticalLayout_chat_main->addWidget(lbl_img_preview);

        frame_input_bar = new QFrame(frame_chat_panel);
        frame_input_bar->setObjectName("frame_input_bar");
        frame_input_bar->setMinimumHeight(68);
        frame_input_bar->setMaximumHeight(68);
        frame_input_bar->setStyleSheet(QString::fromUtf8("QFrame#frame_input_bar { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2A2010,stop:1 #1E1608); border-top: 2px solid #5A4A32; border-radius: 0 0 14px 0; }"));
        horizontalLayout_input = new QHBoxLayout(frame_input_bar);
        horizontalLayout_input->setSpacing(10);
        horizontalLayout_input->setObjectName("horizontalLayout_input");
        horizontalLayout_input->setContentsMargins(14, 12, 14, 12);
        btn_chat_img = new QPushButton(frame_input_bar);
        btn_chat_img->setObjectName("btn_chat_img");
        btn_chat_img->setMinimumSize(QSize(44, 44));
        btn_chat_img->setMaximumSize(QSize(44, 44));
        btn_chat_img->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 22px; font-weight: bold; } QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; color: #D4AF37; } QPushButton:pressed { background: #8B6F47; }"));

        horizontalLayout_input->addWidget(btn_chat_img);

        btn_chat_voice = new QPushButton(frame_input_bar);
        btn_chat_voice->setObjectName("btn_chat_voice");
        btn_chat_voice->setMinimumSize(QSize(44, 44));
        btn_chat_voice->setMaximumSize(QSize(44, 44));
        btn_chat_voice->setStyleSheet(QString::fromUtf8("QPushButton { background: rgba(139,111,71,0.15); border: 1.5px solid #5A4A32; border-radius: 22px; color: #B8925A; font-size: 20px; } QPushButton:hover { background: rgba(139,111,71,0.4); border-color: #D4AF37; color: #D4AF37; } QPushButton:pressed { background: #8B6F47; }"));

        horizontalLayout_input->addWidget(btn_chat_voice);

        le_chat_input = new QLineEdit(frame_input_bar);
        le_chat_input->setObjectName("le_chat_input");
        le_chat_input->setStyleSheet(QString::fromUtf8("QLineEdit { background: rgba(255,255,255,0.08); border: 1.5px solid #5A4A32; border-radius: 22px; padding: 8px 18px; font-size: 14px; color: #F0E0C0; selection-background-color: #8B6F47; } QLineEdit:focus { border: 1.5px solid #D4AF37; background: rgba(255,255,255,0.12); }"));

        horizontalLayout_input->addWidget(le_chat_input);

        btn_chat_send = new QPushButton(frame_input_bar);
        btn_chat_send->setObjectName("btn_chat_send");
        btn_chat_send->setMinimumSize(QSize(90, 44));
        btn_chat_send->setMaximumSize(QSize(90, 44));
        btn_chat_send->setStyleSheet(QString::fromUtf8("QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8B6F47,stop:1 #C4973A); color: white; border-radius: 22px; font-weight: bold; font-size: 13px; border: none; } QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #A0825A,stop:1 #D4AF37); } QPushButton:pressed { background: #6B5237; }"));

        horizontalLayout_input->addWidget(btn_chat_send);


        verticalLayout_chat_main->addWidget(frame_input_bar);


        horizontalLayout_chat_outer->addWidget(frame_chat_panel);

        tabWidget->addTab(tab_chat, QString());
        btn_return_home = new QPushButton(EquipmentManagement);
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

        retranslateUi(EquipmentManagement);
        QObject::connect(btn_help_gestion, &QToolButton::toggled, lbl_hint_gestion, &QLabel::setVisible);
        QObject::connect(btn_help_stats, &QToolButton::toggled, lbl_hint_stats, &QLabel::setVisible);

        tabWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(EquipmentManagement);
    } // setupUi

    void retranslateUi(QWidget *EquipmentManagement)
    {
        EquipmentManagement->setWindowTitle(QCoreApplication::translate("EquipmentManagement", "Equipment Management", nullptr));
        groupBox_gestion->setTitle(QCoreApplication::translate("EquipmentManagement", "Manage Equipment", nullptr));
        label_id->setText(QCoreApplication::translate("EquipmentManagement", "ID:", nullptr));
        label_type->setText(QCoreApplication::translate("EquipmentManagement", "Type:", nullptr));
        le_type->setPlaceholderText(QCoreApplication::translate("EquipmentManagement", "e.g. Power Tool, Hand Tool", nullptr));
        label_date_achat->setText(QCoreApplication::translate("EquipmentManagement", "Purchase Date:", nullptr));
        de_date_achat->setDisplayFormat(QCoreApplication::translate("EquipmentManagement", "dd/MM/yyyy", nullptr));
        label_unit_price->setText(QCoreApplication::translate("EquipmentManagement", "Unit Price (dt):", nullptr));
        label_etat->setText(QCoreApplication::translate("EquipmentManagement", "Status:", nullptr));
        cb_status->setItemText(0, QCoreApplication::translate("EquipmentManagement", "Available", nullptr));
        cb_status->setItemText(1, QCoreApplication::translate("EquipmentManagement", "In Use", nullptr));
        cb_status->setItemText(2, QCoreApplication::translate("EquipmentManagement", "Under Maintenance", nullptr));
        cb_status->setItemText(3, QCoreApplication::translate("EquipmentManagement", "Retired", nullptr));

        label_desc->setText(QCoreApplication::translate("EquipmentManagement", "Description:", nullptr));
        label_quantity->setText(QCoreApplication::translate("EquipmentManagement", "Quantity:", nullptr));
        btn_add->setText(QCoreApplication::translate("EquipmentManagement", "Ajouter", nullptr));
        btn_modify->setText(QCoreApplication::translate("EquipmentManagement", "Modifier", nullptr));
        btn_delete->setText(QCoreApplication::translate("EquipmentManagement", "Supprimer", nullptr));
        btn_clear->setText(QCoreApplication::translate("EquipmentManagement", "Clear", nullptr));
        btn_help_gestion->setText(QCoreApplication::translate("EquipmentManagement", "?", nullptr));
        lbl_hint_gestion->setText(QCoreApplication::translate("EquipmentManagement", "Tip: Use 'Ajouter' for new gear. Select equipment from the list to update its status to 'Broken' or 'Intact'. Descriptions help track maintenance history.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_gestion), QCoreApplication::translate("EquipmentManagement", "Gestion Equipements", nullptr));
        btn_help_view->setText(QCoreApplication::translate("EquipmentManagement", "?", nullptr));
        label_titre_liste_equip->setText(QCoreApplication::translate("EquipmentManagement", "Equipment List", nullptr));
        btn_delete_confirm->setText(QCoreApplication::translate("EquipmentManagement", "Delete", nullptr));
        btn_share_chat->setText(QCoreApplication::translate("EquipmentManagement", "Share", nullptr));
        cb_filter_status->setItemText(0, QCoreApplication::translate("EquipmentManagement", "All Statuses", nullptr));
        cb_filter_status->setItemText(1, QCoreApplication::translate("EquipmentManagement", "Available", nullptr));
        cb_filter_status->setItemText(2, QCoreApplication::translate("EquipmentManagement", "In Use", nullptr));
        cb_filter_status->setItemText(3, QCoreApplication::translate("EquipmentManagement", "Under Maintenance", nullptr));
        cb_filter_status->setItemText(4, QCoreApplication::translate("EquipmentManagement", "Retired", nullptr));

        btn_search->setText(QCoreApplication::translate("EquipmentManagement", "Filter", nullptr));
        le_recherche->setPlaceholderText(QCoreApplication::translate("EquipmentManagement", "Search by ID or description...", nullptr));
        group_bulk_actions->setTitle(QCoreApplication::translate("EquipmentManagement", "Bulk Actions", nullptr));
        label_bulk_instr->setText(QCoreApplication::translate("EquipmentManagement", "Select multiple rows to update status in bulk:", nullptr));
        cb_bulk_status->setItemText(0, QCoreApplication::translate("EquipmentManagement", "Available", nullptr));
        cb_bulk_status->setItemText(1, QCoreApplication::translate("EquipmentManagement", "In Use", nullptr));
        cb_bulk_status->setItemText(2, QCoreApplication::translate("EquipmentManagement", "Under Maintenance", nullptr));
        cb_bulk_status->setItemText(3, QCoreApplication::translate("EquipmentManagement", "Retired", nullptr));

        btn_bulk_update_status->setText(QCoreApplication::translate("EquipmentManagement", "Update Selected", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_view), QCoreApplication::translate("EquipmentManagement", "View Equipments", nullptr));
        label_titre_histo->setText(QCoreApplication::translate("EquipmentManagement", "Activity History", nullptr));
        le_history_search->setPlaceholderText(QCoreApplication::translate("EquipmentManagement", "Search in history...", nullptr));
        btn_history_search->setText(QCoreApplication::translate("EquipmentManagement", "Filter", nullptr));
        tabWidget_history_sections->setTabText(tabWidget_history_sections->indexOf(tab_history_add), QCoreApplication::translate("EquipmentManagement", "Added", nullptr));
        tabWidget_history_sections->setTabText(tabWidget_history_sections->indexOf(tab_history_modify), QCoreApplication::translate("EquipmentManagement", "Modified", nullptr));
        tabWidget_history_sections->setTabText(tabWidget_history_sections->indexOf(tab_history_delete), QCoreApplication::translate("EquipmentManagement", "Deleted", nullptr));
        btn_refresh_history->setText(QCoreApplication::translate("EquipmentManagement", "Refresh", nullptr));
        btn_export_history->setText(QCoreApplication::translate("EquipmentManagement", "Export to PDF", nullptr));
        btn_clear_history->setText(QCoreApplication::translate("EquipmentManagement", "Clear Logs", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_history), QCoreApplication::translate("EquipmentManagement", "History", nullptr));
        label_stats_title->setText(QCoreApplication::translate("EquipmentManagement", "Equipment Statistics Overview", nullptr));
        group_metrics->setTitle(QCoreApplication::translate("EquipmentManagement", "General Metrics", nullptr));
        lbl_total_eq->setText(QCoreApplication::translate("EquipmentManagement", "Total Equipment:", nullptr));
        val_total_eq->setText(QCoreApplication::translate("EquipmentManagement", "0", nullptr));
        lbl_oper_eq->setText(QCoreApplication::translate("EquipmentManagement", "Operational (Intact):", nullptr));
        val_oper_eq->setText(QCoreApplication::translate("EquipmentManagement", "0", nullptr));
        lbl_broken_eq->setText(QCoreApplication::translate("EquipmentManagement", "Under Repair (Broken):", nullptr));
        val_broken_eq->setText(QCoreApplication::translate("EquipmentManagement", "0", nullptr));
        lbl_usage_hours->setText(QCoreApplication::translate("EquipmentManagement", "Machine Usage Hours (avg):", nullptr));
        val_usage_hours->setText(QCoreApplication::translate("EquipmentManagement", "120 hrs", nullptr));
        lbl_maintenance->setText(QCoreApplication::translate("EquipmentManagement", "Maintenance Schedule (next):", nullptr));
        val_maintenance->setText(QCoreApplication::translate("EquipmentManagement", "3 Days", nullptr));
        group_chart->setTitle(QCoreApplication::translate("EquipmentManagement", "Condition Distribution", nullptr));
        label_chart_placeholder->setText(QCoreApplication::translate("EquipmentManagement", "Chart visualization goes here", nullptr));
        btn_help_stats->setText(QCoreApplication::translate("EquipmentManagement", "?", nullptr));
        lbl_hint_stats->setText(QCoreApplication::translate("EquipmentManagement", "Tip: Modern woodworking factories use software dashboards to monitor machines, track usage hours, schedule maintenance, and reduce breakdowns.", nullptr));
        btn_export_stats->setText(QCoreApplication::translate("EquipmentManagement", "Generate Report PDF", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_stats), QCoreApplication::translate("EquipmentManagement", "Statistics", nullptr));
        lbl_sidebar_header->setText(QCoreApplication::translate("EquipmentManagement", "Team Chat", nullptr));
        le_chat_search->setPlaceholderText(QCoreApplication::translate("EquipmentManagement", "Search employee...", nullptr));
        lbl_partner_avatar->setText(QCoreApplication::translate("EquipmentManagement", "?", nullptr));
        lbl_active_employees->setText(QCoreApplication::translate("EquipmentManagement", "Select an employee to chat", nullptr));
        lbl_chat_status->setText(QCoreApplication::translate("EquipmentManagement", "Online", nullptr));
        btn_chat_settings->setText(QCoreApplication::translate("EquipmentManagement", "?", nullptr));
#if QT_CONFIG(tooltip)
        btn_chat_settings->setToolTip(QCoreApplication::translate("EquipmentManagement", "Chat Settings", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_chat_music->setText(QCoreApplication::translate("EquipmentManagement", "?", nullptr));
#if QT_CONFIG(tooltip)
        btn_chat_music->setToolTip(QCoreApplication::translate("EquipmentManagement", "Toggle Music", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_weather_assistant->setText(QString());
#if QT_CONFIG(tooltip)
        btn_weather_assistant->setToolTip(QCoreApplication::translate("EquipmentManagement", "Weather Assistant", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_weather_bot->setText(QCoreApplication::translate("EquipmentManagement", "??", nullptr));
#if QT_CONFIG(tooltip)
        btn_weather_bot->setToolTip(QCoreApplication::translate("EquipmentManagement", "Weather AI Expert", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_chat_refresh->setText(QCoreApplication::translate("EquipmentManagement", "\342\206\273", nullptr));
#if QT_CONFIG(tooltip)
        btn_chat_refresh->setToolTip(QCoreApplication::translate("EquipmentManagement", "Refresh Messages", nullptr));
#endif // QT_CONFIG(tooltip)
        lbl_img_preview->setText(QCoreApplication::translate("EquipmentManagement", "Image attached - ready to send", nullptr));
        btn_chat_img->setText(QCoreApplication::translate("EquipmentManagement", "+", nullptr));
#if QT_CONFIG(tooltip)
        btn_chat_img->setToolTip(QCoreApplication::translate("EquipmentManagement", "Attach Image", nullptr));
#endif // QT_CONFIG(tooltip)
        btn_chat_voice->setText(QCoreApplication::translate("EquipmentManagement", "??", nullptr));
#if QT_CONFIG(tooltip)
        btn_chat_voice->setToolTip(QCoreApplication::translate("EquipmentManagement", "Voice Input (AssemblyAI)", nullptr));
#endif // QT_CONFIG(tooltip)
        le_chat_input->setPlaceholderText(QCoreApplication::translate("EquipmentManagement", "Type a message...", nullptr));
        btn_chat_send->setText(QCoreApplication::translate("EquipmentManagement", "Send", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_chat), QCoreApplication::translate("EquipmentManagement", "Employee Chat", nullptr));
        btn_return_home->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class EquipmentManagement: public Ui_EquipmentManagement {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EQUIPMENT_MANAGEMENT_H
