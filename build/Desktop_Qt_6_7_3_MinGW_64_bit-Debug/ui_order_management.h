/********************************************************************************
** Form generated from reading UI file 'order_management.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORDER_MANAGEMENT_H
#define UI_ORDER_MANAGEMENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OrderManagement
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_manage;
    QLabel *label_id;
    QLineEdit *le_id;
    QLabel *label_type;
    QComboBox *cb_type;
    QLabel *label_stock;
    QLineEdit *le_stock;
    QLabel *label_prix;
    QLineEdit *le_prix;
    QLabel *label_buyer;
    QLineEdit *le_buyer;
    QPushButton *btn_add;
    QPushButton *btn_modify;
    QPushButton *btn_delete;
    QPushButton *btn_clear;
    QPushButton *btn_import;
    QToolButton *btn_help;
    QWidget *tab_qrcode;
    QLabel *label_qr_order_id;
    QLineEdit *le_qr_order_id;
    QPushButton *btn_generate_qr;
    QLabel *label_qr_display;
    QPushButton *btn_save_qr;
    QPushButton *btn_print_qr;
    QToolButton *btn_help_qr;
    QWidget *tab_catalog;
    QTableWidget *table_catalog;
    QPushButton *btn_export_catalog;
    QPushButton *btn_print_catalog;
    QPushButton *btn_delete_all;
    QLineEdit *le_catalog_search;
    QWidget *tab_3d_modeling;
    QPushButton *btn_return_home;

    void setupUi(QWidget *OrderManagement)
    {
        if (OrderManagement->objectName().isEmpty())
            OrderManagement->setObjectName("OrderManagement");
        OrderManagement->resize(1322, 800);
        OrderManagement->setStyleSheet(QString::fromUtf8("\n"
"    #OrderManagement {\n"
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
        tabWidget = new QTabWidget(OrderManagement);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(160, 60, 1051, 681));
        QFont font;
        font.setFamilies({QString::fromUtf8("Gadugi")});
        font.setPointSize(10);
        font.setBold(true);
        tabWidget->setFont(font);
        tabWidget->setStyleSheet(QString::fromUtf8("\n"
"      QTabWidget::pane { border: 1px solid #C4C4C4; border-image: url(:/assets/bg2.PNG) 0 0 0 0 stretch stretch; }\n"
"      QTabWidget::tab-bar { left: 250px; }\n"
"      QTabBar::tab { background: #E0E0E0; border: 1px solid #C4C4C4; padding: 10px 20px; margin-right: 2px; }\n"
"      QTabBar::tab:selected { background: #8B6F47; color: white; }\n"
"      QWidget#tab_manage, QWidget#tab_view, QWidget#tab_stats, QWidget#tab_history, QWidget#tab_calendar, QWidget#tab_qrcode, QWidget#tab_catalog { background: transparent; }\n"
"     "));
        tab_manage = new QWidget();
        tab_manage->setObjectName("tab_manage");
        label_id = new QLabel(tab_manage);
        label_id->setObjectName("label_id");
        label_id->setGeometry(QRect(70, 90, 150, 40));
        label_id->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_id->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        le_id = new QLineEdit(tab_manage);
        le_id->setObjectName("le_id");
        le_id->setGeometry(QRect(230, 90, 280, 40));
        label_type = new QLabel(tab_manage);
        label_type->setObjectName("label_type");
        label_type->setGeometry(QRect(70, 145, 150, 40));
        label_type->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_type->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        cb_type = new QComboBox(tab_manage);
        cb_type->addItem(QString());
        cb_type->addItem(QString());
        cb_type->addItem(QString());
        cb_type->addItem(QString());
        cb_type->addItem(QString());
        cb_type->setObjectName("cb_type");
        cb_type->setGeometry(QRect(230, 145, 280, 40));
        cb_type->setStyleSheet(QString::fromUtf8("\n"
"        QComboBox { background-color: #FFFFFF; border-radius: 8px; padding: 5px 12px; font-size: 14px; color: #333; }\n"
"        QComboBox::drop-down { border: none; }\n"
"      "));
        label_stock = new QLabel(tab_manage);
        label_stock->setObjectName("label_stock");
        label_stock->setGeometry(QRect(70, 200, 150, 40));
        label_stock->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_stock->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        le_stock = new QLineEdit(tab_manage);
        le_stock->setObjectName("le_stock");
        le_stock->setGeometry(QRect(230, 200, 280, 40));
        label_prix = new QLabel(tab_manage);
        label_prix->setObjectName("label_prix");
        label_prix->setGeometry(QRect(70, 255, 150, 40));
        label_prix->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_prix->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        le_prix = new QLineEdit(tab_manage);
        le_prix->setObjectName("le_prix");
        le_prix->setGeometry(QRect(230, 255, 280, 40));
        label_buyer = new QLabel(tab_manage);
        label_buyer->setObjectName("label_buyer");
        label_buyer->setGeometry(QRect(70, 310, 150, 40));
        label_buyer->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        label_buyer->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        le_buyer = new QLineEdit(tab_manage);
        le_buyer->setObjectName("le_buyer");
        le_buyer->setGeometry(QRect(230, 310, 280, 40));
        btn_add = new QPushButton(tab_manage);
        btn_add->setObjectName("btn_add");
        btn_add->setGeometry(QRect(100, 370, 150, 45));
        btn_add->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/assets/add.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_add->setIcon(icon);
        btn_add->setIconSize(QSize(24, 24));
        btn_modify = new QPushButton(tab_manage);
        btn_modify->setObjectName("btn_modify");
        btn_modify->setGeometry(QRect(270, 370, 150, 45));
        btn_modify->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; padding-left: 12px; text-align: left; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/assets/modify.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_modify->setIcon(icon1);
        btn_modify->setIconSize(QSize(24, 24));
        btn_delete = new QPushButton(tab_manage);
        btn_delete->setObjectName("btn_delete");
        btn_delete->setGeometry(QRect(440, 370, 150, 45));
        btn_delete->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/assets/icon_delete_order.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_delete->setIcon(icon2);
        btn_delete->setIconSize(QSize(24, 24));
        btn_clear = new QPushButton(tab_manage);
        btn_clear->setObjectName("btn_clear");
        btn_clear->setGeometry(QRect(185, 435, 150, 45));
        btn_clear->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/assets/icon_clear_fields.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_clear->setIcon(icon3);
        btn_clear->setIconSize(QSize(24, 24));
        btn_import = new QPushButton(tab_manage);
        btn_import->setObjectName("btn_import");
        btn_import->setGeometry(QRect(355, 435, 150, 45));
        btn_import->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"       "));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/assets/icon_import_orders.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_import->setIcon(icon4);
        btn_import->setIconSize(QSize(24, 24));
        btn_help = new QToolButton(tab_manage);
        btn_help->setObjectName("btn_help");
        btn_help->setGeometry(QRect(1000, 30, 30, 30));
        btn_help->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help->setStyleSheet(QString::fromUtf8("\n"
"         QToolButton { background-color: #8B6F47; border-radius: 15px; color: #333; font-weight: bold; border: none; }\n"
"         QToolButton:checked { background-color: #FFF; border: 3px solid #8B6F47; }\n"
"         QToolButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        btn_help->setCheckable(true);
        tabWidget->addTab(tab_manage, QString());
        tab_qrcode = new QWidget();
        tab_qrcode->setObjectName("tab_qrcode");
        label_qr_order_id = new QLabel(tab_qrcode);
        label_qr_order_id->setObjectName("label_qr_order_id");
        label_qr_order_id->setGeometry(QRect(50, 60, 150, 30));
        label_qr_order_id->setStyleSheet(QString::fromUtf8("color: white; font-size: 16px; font-weight: bold;\n"
""));
        le_qr_order_id = new QLineEdit(tab_qrcode);
        le_qr_order_id->setObjectName("le_qr_order_id");
        le_qr_order_id->setGeometry(QRect(360, 60, 300, 40));
        btn_generate_qr = new QPushButton(tab_qrcode);
        btn_generate_qr->setObjectName("btn_generate_qr");
        btn_generate_qr->setGeometry(QRect(810, 60, 140, 35));
        btn_generate_qr->setStyleSheet(QString::fromUtf8("\n"
"         QPushButton { background-color: #8B6F47; border-radius: 10px; color: white; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; color: #8B6F47; }\n"
"         QPushButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"        "));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/assets/icon_qr.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_generate_qr->setIcon(icon5);
        label_qr_display = new QLabel(tab_qrcode);
        label_qr_display->setObjectName("label_qr_display");
        label_qr_display->setGeometry(QRect(360, 120, 300, 300));
        label_qr_display->setStyleSheet(QString::fromUtf8("\n"
"         border: 2px solid #8B6F47; \n"
"         border-radius: 10px; \n"
"         background-color: white;\n"
"         qproperty-alignment: AlignCenter;\n"
"        "));
        label_qr_display->setScaledContents(false);
        label_qr_display->setAlignment(Qt::AlignmentFlag::AlignCenter);
        btn_save_qr = new QPushButton(tab_qrcode);
        btn_save_qr->setObjectName("btn_save_qr");
        btn_save_qr->setGeometry(QRect(320, 440, 190, 54));
        btn_save_qr->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 12px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"        QPushButton:hover { background-color: #a38253; border: 2px solid #8B6F47; color: white; }\n"
"       "));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/assets/save.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_save_qr->setIcon(icon6);
        btn_save_qr->setIconSize(QSize(28, 28));
        btn_print_qr = new QPushButton(tab_qrcode);
        btn_print_qr->setObjectName("btn_print_qr");
        btn_print_qr->setGeometry(QRect(540, 440, 190, 54));
        btn_print_qr->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 12px; color: white; font-weight: bold; padding-left: 12px; text-align: left; }\n"
"        QPushButton:hover { background-color: #a38253; border: 2px solid #8B6F47; color: white; }\n"
"       "));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/assets/printer.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_print_qr->setIcon(icon7);
        btn_print_qr->setIconSize(QSize(28, 28));
        btn_help_qr = new QToolButton(tab_qrcode);
        btn_help_qr->setObjectName("btn_help_qr");
        btn_help_qr->setGeometry(QRect(1000, 30, 30, 30));
        btn_help_qr->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_help_qr->setStyleSheet(QString::fromUtf8("\n"
"        QToolButton { background-color: #8B6F47; border-radius: 15px; color: #333; font-weight: bold; border: none; }\n"
"        QToolButton:checked { background-color: #FFF; border: 3px solid #8B6F47; }\n"
"        QToolButton:hover { background-color: #FFF; border: 2px solid #8B6F47; }\n"
"       "));
        btn_help_qr->setCheckable(true);
        tabWidget->addTab(tab_qrcode, QString());
        tab_catalog = new QWidget();
        tab_catalog->setObjectName("tab_catalog");
        table_catalog = new QTableWidget(tab_catalog);
        if (table_catalog->columnCount() < 7)
            table_catalog->setColumnCount(7);
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
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        table_catalog->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        table_catalog->setObjectName("table_catalog");
        table_catalog->setGeometry(QRect(10, 50, 1030, 570));
        table_catalog->setStyleSheet(QString::fromUtf8("\n"
"        QHeaderView::section { background-color: #8B6F47; color: white; font-weight: bold; border: none; padding: 8px; }\n"
"        QTableWidget { border: 1px solid #8B6F47; selection-background-color: #E0E0E0; selection-color: white; gridline-color: #8B6F47; }\n"
"        QTableWidget::item { padding: 5px; }\n"
"       "));
        table_catalog->setRowCount(0);
        table_catalog->setColumnCount(7);
        btn_export_catalog = new QPushButton(tab_catalog);
        btn_export_catalog->setObjectName("btn_export_catalog");
        btn_export_catalog->setGeometry(QRect(10, 630, 180, 40));
        btn_export_catalog->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: #FFF; font-weight: bold; padding-left: 10px; text-align: left; }\n"
"        QPushButton:hover { background-color: #a38253; border: 2px solid #8B6F47; }\n"
"       "));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/assets/pdf_export.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btn_export_catalog->setIcon(icon8);
        btn_export_catalog->setIconSize(QSize(24, 24));
        btn_print_catalog = new QPushButton(tab_catalog);
        btn_print_catalog->setObjectName("btn_print_catalog");
        btn_print_catalog->setGeometry(QRect(210, 630, 180, 40));
        btn_print_catalog->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #8B6F47; border-radius: 10px; color: #FFF; font-weight: bold; padding-left: 10px; text-align: left; }\n"
"        QPushButton:hover { background-color: #a38253; border: 2px solid #8B6F47; }\n"
"       "));
        btn_print_catalog->setIcon(icon7);
        btn_print_catalog->setIconSize(QSize(24, 24));
        btn_delete_all = new QPushButton(tab_catalog);
        btn_delete_all->setObjectName("btn_delete_all");
        btn_delete_all->setGeometry(QRect(410, 630, 180, 40));
        btn_delete_all->setStyleSheet(QString::fromUtf8("\n"
"        QPushButton { background-color: #A03030; border-radius: 10px; color: #FFF; font-weight: bold; padding-left: 10px; text-align: left; }\n"
"        QPushButton:hover { background-color: #D04040; border: 2px solid #8B6F47; }\n"
"       "));
        btn_delete_all->setIcon(icon2);
        btn_delete_all->setIconSize(QSize(24, 24));
        le_catalog_search = new QLineEdit(tab_catalog);
        le_catalog_search->setObjectName("le_catalog_search");
        le_catalog_search->setGeometry(QRect(830, 630, 200, 40));
        tabWidget->addTab(tab_catalog, QString());
        tab_3d_modeling = new QWidget();
        tab_3d_modeling->setObjectName("tab_3d_modeling");
        tabWidget->addTab(tab_3d_modeling, QString());
        btn_return_home = new QPushButton(OrderManagement);
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

        retranslateUi(OrderManagement);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(OrderManagement);
    } // setupUi

    void retranslateUi(QWidget *OrderManagement)
    {
        OrderManagement->setWindowTitle(QCoreApplication::translate("OrderManagement", "Order Management", nullptr));
        label_id->setText(QCoreApplication::translate("OrderManagement", "Order ID:", nullptr));
        label_type->setText(QCoreApplication::translate("OrderManagement", "Type:", nullptr));
        cb_type->setItemText(0, QCoreApplication::translate("OrderManagement", "Chair", nullptr));
        cb_type->setItemText(1, QCoreApplication::translate("OrderManagement", "Table", nullptr));
        cb_type->setItemText(2, QCoreApplication::translate("OrderManagement", "Cabinet", nullptr));
        cb_type->setItemText(3, QCoreApplication::translate("OrderManagement", "Wardrobe", nullptr));
        cb_type->setItemText(4, QCoreApplication::translate("OrderManagement", "Other", nullptr));

        label_stock->setText(QCoreApplication::translate("OrderManagement", "Quantity:", nullptr));
        label_prix->setText(QCoreApplication::translate("OrderManagement", "Price:", nullptr));
        label_buyer->setText(QCoreApplication::translate("OrderManagement", "Buyer ID:", nullptr));
        btn_add->setText(QCoreApplication::translate("OrderManagement", "Add Order", nullptr));
        btn_modify->setText(QCoreApplication::translate("OrderManagement", "Modify Order", nullptr));
        btn_delete->setText(QCoreApplication::translate("OrderManagement", "Delete Order", nullptr));
        btn_clear->setText(QCoreApplication::translate("OrderManagement", "Clear Fields", nullptr));
        btn_import->setText(QCoreApplication::translate("OrderManagement", "Import Orders", nullptr));
        btn_help->setText(QCoreApplication::translate("OrderManagement", "?", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_manage), QCoreApplication::translate("OrderManagement", "Manage Orders", nullptr));
        label_qr_order_id->setText(QCoreApplication::translate("OrderManagement", "Order ID:", nullptr));
        le_qr_order_id->setPlaceholderText(QCoreApplication::translate("OrderManagement", "Enter Order ID to generate QR code...", nullptr));
        btn_generate_qr->setText(QCoreApplication::translate("OrderManagement", "Generate Status", nullptr));
        label_qr_display->setText(QCoreApplication::translate("OrderManagement", "Status will appear here", nullptr));
        btn_save_qr->setText(QCoreApplication::translate("OrderManagement", "Save Status", nullptr));
        btn_print_qr->setText(QCoreApplication::translate("OrderManagement", "Print Status", nullptr));
        btn_help_qr->setText(QCoreApplication::translate("OrderManagement", "?", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_qrcode), QCoreApplication::translate("OrderManagement", "Status", nullptr));
        QTableWidgetItem *___qtablewidgetitem = table_catalog->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("OrderManagement", "Order ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = table_catalog->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("OrderManagement", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = table_catalog->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("OrderManagement", "Quantity", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = table_catalog->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("OrderManagement", "Unit Price", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = table_catalog->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("OrderManagement", "Total Price", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = table_catalog->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("OrderManagement", "Buyer ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = table_catalog->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("OrderManagement", "Status", nullptr));
        btn_export_catalog->setText(QCoreApplication::translate("OrderManagement", "Export Order", nullptr));
        btn_print_catalog->setText(QCoreApplication::translate("OrderManagement", "Print Order", nullptr));
        btn_delete_all->setText(QCoreApplication::translate("OrderManagement", "Delete All", nullptr));
        le_catalog_search->setPlaceholderText(QCoreApplication::translate("OrderManagement", "Enter Order ID...", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_catalog), QCoreApplication::translate("OrderManagement", "Catalog", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3d_modeling), QCoreApplication::translate("OrderManagement", "3D Modeling", nullptr));
        btn_return_home->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class OrderManagement: public Ui_OrderManagement {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORDER_MANAGEMENT_H
