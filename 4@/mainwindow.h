#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChart>



#include "loginwindow.h"
#include "homewindow.h"

QT_BEGIN_NAMESPACE
QT_BEGIN_NAMESPACE
namespace Ui { 
    class MainWindow; 
    class ClientManagement;
    class EmployeeManagement;
    class EquipmentManagement;
    class OrderManagement;
    class SupplierManagement;
}
QT_END_NAMESPACE
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupClientStats();
    void setupClientManagement();
    void setupEquipmentStats();
    void setupSupplierStats();
    void setupClientCalendar();
    void setupEmployeeModes();
    void setupSupplierModes();
    void setupEquipmentModes();
    void setupOrderModes();
    void setupGlobalStyles();
    void setupTabNavigation(QWidget* parentWidget, QTabWidget* tabWidget, const QStringList& tabNames, int startX, int yPos, const QList<int>& targetIndices = {});

private slots:
    void on_login_clicked();
    
    // --- Home Screen Navigation ---
    void on_gs_employes_clicked();
    void on_gs_client_clicked();
    void on_gs_fournisseur_clicked();
    void on_gs_equipment_clicked();
    void on_gs_order_clicked();

    // --- Sidebar Navigation ---
    void on_nav_employees_clicked();
    void on_nav_clients_clicked();
    void on_nav_suppliers_clicked();
    void on_nav_equipments_clicked();
    void on_nav_orders_clicked();

    // --- System Navigation ---
    void on_btn_logout_clicked();
    void on_btn_home_clicked();
    
    // --- Order Management ---
    void onOrderClearFields();
    
    // --- Client Management ---
    void onClientClearFields();
    void onClientModClearFields();
    
    // --- Employee Management ---
    void onEmployeeClearFields();
    
    // --- Supplier Management ---
    void onSupplierClearFields();
    
    // --- Equipment Management ---
    void onEquipmentClearFields();

private:
    Ui::MainWindow *ui;
    
    // UI Pointers for Modules
    Ui::ClientManagement *ui_client;
    Ui::EmployeeManagement *ui_employee;
    Ui::EquipmentManagement *ui_equipment;
    Ui::OrderManagement *ui_order;
    Ui::SupplierManagement *ui_supplier;

    // Widget containers for Modules
    QWidget *clientPage;
    QWidget *employeePage;
    QWidget *equipmentPage;
    QWidget *orderPage;
    QWidget *supplierPage;

    LoginWindow *loginWindow;
    HomeWindow *homeWindow;
};
#endif // MAINWINDOW_H
