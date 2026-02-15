#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_client_management.h"
#include "ui_employee_management.h"
#include "ui_equipment_management.h"
#include "ui_order_management.h"
#include "ui_supplier_management.h"
#include "buttonanimator.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QSplineSeries>
#include <QCalendarWidget>
#include <QListWidget>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- CRITICAL REFACTOR: CLEAR STACK AND REBUILD ---
    // Remove any hardcoded pages (e.g., page_login, page_home) created by setupUi
    while (ui->stackedWidget->count() > 0) {
        QWidget* widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();
    }

    // 1. Login Window (Index 0)
    loginWindow = new LoginWindow(this);
    ui->stackedWidget->addWidget(loginWindow);
    connect(loginWindow, &LoginWindow::loginSuccessful, this, &MainWindow::on_login_clicked);

    // 2. Home Window (Index 1)
    homeWindow = new HomeWindow(this);
    ui->stackedWidget->addWidget(homeWindow);
    
    // Connect HomeWindow signals
    connect(homeWindow, &HomeWindow::employesClicked,    this, &MainWindow::on_gs_employes_clicked);
    connect(homeWindow, &HomeWindow::clientClicked,      this, &MainWindow::on_gs_client_clicked);
    connect(homeWindow, &HomeWindow::fournisseurClicked, this, &MainWindow::on_gs_fournisseur_clicked);
    connect(homeWindow, &HomeWindow::equipmentClicked,   this, &MainWindow::on_gs_equipment_clicked);
    connect(homeWindow, &HomeWindow::orderClicked,       this, &MainWindow::on_gs_order_clicked);

    // 3. Employee Management (Index 2)
    ui_employee = new Ui::EmployeeManagement;
    employeePage = new QWidget(this);
    ui_employee->setupUi(employeePage);
    ui->stackedWidget->addWidget(employeePage);
    connect(ui_employee->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);

    // 4. Client Management (Index 3)
    ui_client = new Ui::ClientManagement;
    clientPage = new QWidget(this);
    ui_client->setupUi(clientPage);
    ui->stackedWidget->addWidget(clientPage);
    connect(ui_client->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);

    // 5. Supplier Management (Index 4)
    ui_supplier = new Ui::SupplierManagement;
    supplierPage = new QWidget(this);
    ui_supplier->setupUi(supplierPage);
    ui->stackedWidget->addWidget(supplierPage);
    connect(ui_supplier->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);

    // 6. Equipment Management (Index 5)
    ui_equipment = new Ui::EquipmentManagement;
    equipmentPage = new QWidget(this);
    ui_equipment->setupUi(equipmentPage);
    ui->stackedWidget->addWidget(equipmentPage);
    connect(ui_equipment->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);

    // 7. Order Management (Index 6)
    ui_order = new Ui::OrderManagement;
    orderPage = new QWidget(this);
    ui_order->setupUi(orderPage);
    ui->stackedWidget->addWidget(orderPage);
    connect(ui_order->btn_return_home, &QPushButton::clicked, this, &MainWindow::on_btn_home_clicked);
    connect(ui_order->btn_clear, &QPushButton::clicked, this, &MainWindow::onOrderClearFields);
    
    // Connect Clear Fields buttons for all modules
    connect(ui_client->btn_clear, &QPushButton::clicked, this, &MainWindow::onClientClearFields);
    connect(ui_client->btn_clear_mod, &QPushButton::clicked, this, &MainWindow::onClientModClearFields);
    connect(ui_employee->btn_clear, &QPushButton::clicked, this, &MainWindow::onEmployeeClearFields);
    connect(ui_supplier->btn_clear, &QPushButton::clicked, this, &MainWindow::onSupplierClearFields);
    connect(ui_equipment->btn_clear, &QPushButton::clicked, this, &MainWindow::onEquipmentClearFields);

    // --- Apply Hover Animations to Management Module Buttons ---
    // Employee Management
    for (auto* button : employeePage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Client Management
    for (auto* button : clientPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Supplier Management
    for (auto* button : supplierPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Equipment Management
    for (auto* button : equipmentPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }
    
    // Order Management
    for (auto* button : orderPage->findChildren<QPushButton*>()) {
        ButtonAnimator::applyHoverAnimation(button);
    }

    // Start at login page
    ui->stackedWidget->setCurrentIndex(0);

    // Call setup function for client stats
    setupClientStats();
    
    // Consolidate Client Management Tabs
    setupClientManagement();

    // Setup Equipment Stats
    setupEquipmentStats();

    // Setup Supplier Stats
    setupSupplierStats();

    // Setup Client Calendar
    setupClientCalendar();

    // Standardize Add/Modify Modes
    setupEmployeeModes();
    setupSupplierModes();
    setupEquipmentModes();
    setupOrderModes();

    // Hide tab bars and setup radio button navigation
    ui_client->tabWidget->tabBar()->hide();
    ui_employee->tabWidget->tabBar()->hide();
    ui_supplier->tabWidget->tabBar()->hide();
    ui_equipment->tabWidget->tabBar()->hide();
    ui_order->tabWidget->tabBar()->hide();
    // Setup radio button navigation for all UIs
    // Y set to (TabWidgetY + 25) to align with inner buttons.
    setupTabNavigation(clientPage, ui_client->tabWidget, {"Manage", "View", "Stats", "Mail", "Calendar"}, 150, 45, {0, 1, 2, 3, 4});   // Override indices: Manage->0, View->1, Stats->2, Mail->3, Calendar->4
    setupTabNavigation(employeePage, ui_employee->tabWidget, {"Manage", "View", "Stats", "History"}, 150, 95);  // 70+25
    setupTabNavigation(supplierPage, ui_supplier->tabWidget, {"Manage", "Stats", "View", "Reviews"}, 150, 45);  // 20+25 (Swapped View/Stats if that was intention, user said "change view to stats" and "stats to view")
    setupTabNavigation(equipmentPage, ui_equipment->tabWidget, {"Manage", "View", "History", "Stats"}, 150, 95); // 70+25 (Swapped Stats/History)
    setupTabNavigation(orderPage, ui_order->tabWidget, {"Manage", "QR Code", "Catalog"}, 150, 85);           // 60+25

    // Standardize UI Styling
    setupGlobalStyles();
}

void MainWindow::setupClientStats()
{
    // 1. Create Layout for the stats container (widget_chart)
    if (!ui_client->widget_chart->layout()) {
        QHBoxLayout *layout = new QHBoxLayout(ui_client->widget_chart);
        ui_client->widget_chart->setLayout(layout);
    }

    // --- CHART 1: PIE CHART (Gender Distribution) ---
    QPieSeries *series = new QPieSeries();
    series->append("Male", 60);
    series->append("Female", 40);

    // Add colors
    QPieSlice *sliceMale = series->slices().at(0);
    sliceMale->setBrush(QColor("#8B6F47")); // Gold/Brown
    sliceMale->setLabelVisible();

    QPieSlice *sliceFemale = series->slices().at(1);
    sliceFemale->setBrush(QColor("#C0C0C0")); // Silver/Grey
    sliceFemale->setLabelVisible();
    sliceFemale->setExploded(); // Highlight one slice

    QChart *chartPie = new QChart();
    chartPie->addSeries(series);
    chartPie->setTitle("Clients by Gender");
    chartPie->setAnimationOptions(QChart::SeriesAnimations);

    QChartView *chartViewPie = new QChartView(chartPie);
    chartViewPie->setRenderHint(QPainter::Antialiasing);


    // --- CHART 2: BAR CHART (Clients Activity) ---
    QBarSet *set0 = new QBarSet("Active");
    QBarSet *set1 = new QBarSet("Inactive");

    *set0 << 10 << 20 << 30 << 40 << 50 << 60;
    *set1 << 5 << 10 << 15 << 20 << 25 << 30;

    set0->setColor(QColor("#8B6F47"));
    set1->setColor(QColor("#A9A9A9"));

    QBarSeries *seriesBar = new QBarSeries();
    seriesBar->append(set0);
    seriesBar->append(set1);

    QChart *chartBar = new QChart();
    chartBar->addSeries(seriesBar);
    chartBar->setTitle("Client Activity (Last 6 Months)");
    chartBar->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    seriesBar->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 70);
    chartBar->addAxis(axisY, Qt::AlignLeft);
    seriesBar->attachAxis(axisY);

    QChartView *chartViewBar = new QChartView(chartBar);
    chartViewBar->setRenderHint(QPainter::Antialiasing);

    // Add charts to the layout
    ui_client->widget_chart->layout()->addWidget(chartViewPie);
    ui_client->widget_chart->layout()->addWidget(chartViewBar);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ui_client;
    delete ui_employee;
    delete ui_equipment;
    delete ui_order;
    delete ui_supplier;
}

// --- Navigation Slots ---

// Login -> Home (Page 0 -> Page 1)
void MainWindow::on_login_clicked()
{
    ui->stackedWidget->setCurrentIndex(1); 
}

// Home -> Modules
void MainWindow::on_gs_employes_clicked()    { ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_gs_client_clicked()      { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_gs_fournisseur_clicked() { ui->stackedWidget->setCurrentIndex(4); }
void MainWindow::on_gs_equipment_clicked()   { ui->stackedWidget->setCurrentIndex(5); }
void MainWindow::on_gs_order_clicked()       { ui->stackedWidget->setCurrentIndex(6); }

// Navigation sidebar
void MainWindow::on_nav_employees_clicked()  { ui->stackedWidget->setCurrentIndex(2); }
void MainWindow::on_nav_clients_clicked()    { ui->stackedWidget->setCurrentIndex(3); }
void MainWindow::on_nav_suppliers_clicked()  { ui->stackedWidget->setCurrentIndex(4); }
void MainWindow::on_nav_equipments_clicked() { ui->stackedWidget->setCurrentIndex(5); }
void MainWindow::on_nav_orders_clicked()     { ui->stackedWidget->setCurrentIndex(6); }

// Logout / Home
void MainWindow::on_btn_logout_clicked()     { ui->stackedWidget->setCurrentIndex(0); }
void MainWindow::on_btn_home_clicked()       { ui->stackedWidget->setCurrentIndex(1); }

void MainWindow::onOrderClearFields()
{
    if (ui_order) {
        ui_order->le_id->clear();
        ui_order->le_type->clear();
        ui_order->le_stock->clear();
        ui_order->le_prix->clear();
        ui_order->le_buyer->clear();
        ui_order->le_qr_order_id->clear();
        ui_order->le_catalog_search->clear();
    }
}

void MainWindow::onClientClearFields()
{
    if (ui_client) {
        ui_client->le_nom->clear();
        ui_client->le_prenom->clear();
        ui_client->le_adresse->clear();
        ui_client->le_tel->clear();
        ui_client->le_email->clear();
        // Reset radio buttons
        ui_client->rb_homme->setAutoExclusive(false);
        ui_client->rb_femme->setAutoExclusive(false);
        ui_client->rb_homme->setChecked(false);
        ui_client->rb_femme->setChecked(false);
        ui_client->rb_homme->setAutoExclusive(true);
        ui_client->rb_femme->setAutoExclusive(true);
    }
}

void MainWindow::onClientModClearFields()
{
    if (ui_client) {
        ui_client->le_id_mod->clear();
        ui_client->le_nom_mod->clear();
        ui_client->le_prenom_mod->clear();
        ui_client->le_adresse_mod->clear();
        ui_client->le_tel_mod->clear();
        ui_client->le_email_mod->clear();
        // Reset radio buttons
        ui_client->rb_homme_mod->setAutoExclusive(false);
        ui_client->rb_femme_mod->setAutoExclusive(false);
        ui_client->rb_homme_mod->setChecked(false);
        ui_client->rb_femme_mod->setChecked(false);
        ui_client->rb_homme_mod->setAutoExclusive(true);
        ui_client->rb_femme_mod->setAutoExclusive(true);
    }
}

void MainWindow::onEmployeeClearFields()
{
    if (ui_employee) {
        ui_employee->le_id->clear();
        ui_employee->le_nom->clear();
        ui_employee->le_prenom->clear();
        ui_employee->le_fonction->clear();
        ui_employee->sb_age->setValue(0);
        ui_employee->le_mdp->clear();
        ui_employee->dsb_salaire->setValue(0.0);
        ui_employee->le_email->clear();
        ui_employee->le_num->clear();
    }
}

void MainWindow::onSupplierClearFields()
{
    if (ui_supplier) {
        ui_supplier->le_id->clear();
        ui_supplier->le_nom->clear();
        ui_supplier->le_adresse->clear();
        ui_supplier->le_email->clear();
        ui_supplier->le_product_type->clear();
        ui_supplier->le_type->clear();
        ui_supplier->txt_sms->clear();
    }
}

void MainWindow::onEquipmentClearFields()
{
    if (ui_equipment) {
        ui_equipment->le_id->clear();
        ui_equipment->de_date_achat->setDate(QDate::currentDate());
        ui_equipment->te_desc->clear();
        // Reset radio buttons
        ui_equipment->rb_intact->setAutoExclusive(false);
        ui_equipment->rb_broken->setAutoExclusive(false);
        ui_equipment->rb_intact->setChecked(false);
        ui_equipment->rb_broken->setChecked(false);
        ui_equipment->rb_intact->setAutoExclusive(true);
        ui_equipment->rb_broken->setAutoExclusive(true);
    }
}

void MainWindow::setupClientManagement()
{
    // 1. Rename 'Add Client' tab to 'Manage Clients'
    int addTabIndex = ui_client->tabWidget->indexOf(ui_client->tab_add);
    if (addTabIndex != -1) {
        ui_client->tabWidget->setTabText(addTabIndex, "Manage Clients");
    }

    // 2. Reparent 'Modify Client' GroupBox to 'Add Client' tab
    ui_client->group_modify->setParent(ui_client->tab_add);
    
    // Position it same as group_add
    ui_client->group_add->move(20, 70);
    ui_client->group_modify->move(20, 70);

    // Hide modify group initially
    ui_client->group_modify->setVisible(false);
    ui_client->group_add->setVisible(true);

    // 3. Create Toggle Radio Buttons
    QRadioButton *rbAdd = new QRadioButton("Add Mode", ui_client->tab_add);
    QRadioButton *rbMod = new QRadioButton("Modify Mode", ui_client->tab_add);

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);
    
    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // 4. Connect Signals
    connect(rbAdd, &QRadioButton::toggled, [=](bool checked){
        if(checked) {
            ui_client->group_add->setVisible(true);
            ui_client->group_modify->setVisible(false);
        }
    });

    connect(rbMod, &QRadioButton::toggled, [=](bool checked){
        if(checked) {
            ui_client->group_add->setVisible(false);
            ui_client->group_modify->setVisible(true);
        }
    });

    // 5. Remove the empty 'Modify Client' tab
    int modifyTabIndex = ui_client->tabWidget->indexOf(ui_client->tab_modify);
    if (modifyTabIndex != -1) {
        ui_client->tabWidget->removeTab(modifyTabIndex);
    }
}

void MainWindow::setupEquipmentStats()
{
    // --- Layout Adjustment ---
    // Resize groups to fit professional dashboard
    // Tab size is approx 1051x681.
    // Metris sidebar: Left
    ui_equipment->group_metrics->setGeometry(20, 70, 300, 600);
    
    // Charts main area: Right
    ui_equipment->group_chart->setGeometry(340, 70, 680, 600);

    // --- Mock Data ---
    int intactCount = 45;
    int brokenCount = 8;
    int totalCount = intactCount + brokenCount;

    // --- Update Metrics Labels ---
    ui_equipment->val_total_eq->setText(QString::number(totalCount));
    ui_equipment->val_oper_eq->setText(QString::number(intactCount));
    ui_equipment->val_broken_eq->setText(QString::number(brokenCount));

    // --- 1. Condition Donut Chart ---
    QPieSeries *seriesDonut = new QPieSeries();
    seriesDonut->setHoleSize(0.35); // Create Donut effect
    seriesDonut->append("Intact", intactCount);
    seriesDonut->append("Broken", brokenCount);

    QPieSlice *sliceIntact = seriesDonut->slices().at(0);
    sliceIntact->setBrush(QColor("#4CAF50")); // Green
    sliceIntact->setLabelVisible();
    
    if (seriesDonut->slices().count() > 1) {
        QPieSlice *sliceBroken = seriesDonut->slices().at(1);
        sliceBroken->setBrush(QColor("#f44336")); // Red
        sliceBroken->setLabelVisible();
        sliceBroken->setExploded();
    }

    QChart *chartDonut = new QChart();
    chartDonut->addSeries(seriesDonut);
    chartDonut->setTitle("Condition Overview");
    chartDonut->setAnimationOptions(QChart::SeriesAnimations);
    chartDonut->legend()->setAlignment(Qt::AlignRight);
    
    QChartView *viewDonut = new QChartView(chartDonut);
    viewDonut->setRenderHint(QPainter::Antialiasing);

    // --- 2. Equipment by Type Bar Chart ---
    QBarSet *setLaptops = new QBarSet("Laptops");
    QBarSet *setPrinters = new QBarSet("Printers");
    QBarSet *setScreens = new QBarSet("Screens");

    *setLaptops << 12;
    *setPrinters << 5;
    *setScreens << 8;

    setLaptops->setColor(QColor("#8B6F47")); // Gold
    setPrinters->setColor(QColor("#A9A9A9")); // Grey
    setScreens->setColor(QColor("#333333")); // Dark

    QBarSeries *seriesBar = new QBarSeries();
    seriesBar->append(setLaptops);
    seriesBar->append(setPrinters);
    seriesBar->append(setScreens);

    QChart *chartBar = new QChart();
    chartBar->addSeries(seriesBar);
    chartBar->setTitle("Inventory by Type");
    chartBar->setAnimationOptions(QChart::SeriesAnimations);
    
    QStringList categories;
    categories << "Current Stock";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    seriesBar->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    chartBar->addAxis(axisY, Qt::AlignLeft);
    seriesBar->attachAxis(axisY);

    QChartView *viewBar = new QChartView(chartBar);
    viewBar->setRenderHint(QPainter::Antialiasing);

    // --- 3. Maintenance Cost Trends (Spline Chart) ---
    QSplineSeries *seriesSpline = new QSplineSeries();
    seriesSpline->setName("Cost (DT)");
    // Mock Data: Month 1-6
    seriesSpline->append(1, 150);
    seriesSpline->append(2, 230);
    seriesSpline->append(3, 180);
    seriesSpline->append(4, 450); // Spike
    seriesSpline->append(5, 320);
    seriesSpline->append(6, 200);

    QPen pen(QColor("#8B6F47"));
    pen.setWidth(3);
    seriesSpline->setPen(pen);

    QChart *chartSpline = new QChart();
    chartSpline->addSeries(seriesSpline);
    chartSpline->setTitle("Maintenance Costs (6 Months)");
    chartSpline->setAnimationOptions(QChart::SeriesAnimations);
    
    QValueAxis *axisXSpline = new QValueAxis();
    axisXSpline->setTitleText("Month");
    axisXSpline->setLabelFormat("%.0f");
    chartSpline->addAxis(axisXSpline, Qt::AlignBottom);
    seriesSpline->attachAxis(axisXSpline);

    QValueAxis *axisYSpline = new QValueAxis();
    axisYSpline->setTitleText("Cost");
    chartSpline->addAxis(axisYSpline, Qt::AlignLeft);
    seriesSpline->attachAxis(axisYSpline);
    
    QChartView *viewSpline = new QChartView(chartSpline);
    viewSpline->setRenderHint(QPainter::Antialiasing);

    // --- Layout Management ---
    // If layout doesn't exist, create GRID layout (was vertical/horizontal potentially)
    // We need to be careful if a layout already exists from previous setup.
    // Ideally we clear it or check type. For now assuming we can replace or reuse.
    
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(ui_equipment->group_chart->layout());
    if (!gridLayout) {
        // If there was another layout, we should probably delete it properly, 
        // but to be safe simply deleting the old widget wrapper or layout item is tricky in one go.
        // Let's assume we can set a new one or it's the first time. 
        // Or if it's a VBox/HBox, we might just add to it. 
        // Strategy: Create a new container widget if needed, or force set layout.
        // Qt warns if trying to set layout on widget that has one.
        
        if (ui_equipment->group_chart->layout()) {
            delete ui_equipment->group_chart->layout(); // Risky but often works if children are handled
        }
        gridLayout = new QGridLayout(ui_equipment->group_chart);
        ui_equipment->group_chart->setLayout(gridLayout);
    }

    // Hide placeholder
    if(ui_equipment->label_chart_placeholder)
        ui_equipment->label_chart_placeholder->setVisible(false);
    
    // Add Charts to Grid
    // Row 0, Col 0: Donut
    gridLayout->addWidget(viewDonut, 0, 0);
    // Row 0, Col 1: Bar
    gridLayout->addWidget(viewBar, 0, 1);
    // Row 1, Col 0, Span 1 Row, 2 Cols: Spline (Wide)
    gridLayout->addWidget(viewSpline, 1, 0, 1, 2);
}

void MainWindow::setupSupplierStats()
{
    // --- Mock Data ---
    // Metrics
    int retentionRate = 94;
    int accuracyRate = 89;
    int qualityScore = 96;
    int speedScore = 84;
    int northRegionPerf = 91;
    int southRegionPerf = 76;
    
    // --- Update Metrics UI ---
    ui_supplier->lbl_percent_retention->setText(QString::number(retentionRate) + "%");
    ui_supplier->lbl_percent_accuracy->setText(QString::number(accuracyRate) + "%");
    
    ui_supplier->pb_quality->setValue(qualityScore);
    ui_supplier->pb_speed->setValue(speedScore);
    
    ui_supplier->pb_reg_1->setValue(northRegionPerf);
    ui_supplier->pb_reg_2->setValue(southRegionPerf);
    
    // --- Chart 1: Product Categories (Donut) ---
    // Target: frame_chart_types
    QPieSeries *seriesCat = new QPieSeries();
    seriesCat->setHoleSize(0.40);
    seriesCat->append("Electronics", 40);
    seriesCat->append("Furniture", 25);
    seriesCat->append("Stationery", 20);
    seriesCat->append("Services", 15);
    
    // Colors
    seriesCat->slices().at(0)->setBrush(QColor("#8B6F47")); // Gold/Brown
    seriesCat->slices().at(1)->setBrush(QColor("#A9A9A9")); // Grey
    seriesCat->slices().at(2)->setBrush(QColor("#D2B48C")); // Tan
    seriesCat->slices().at(3)->setBrush(QColor("#333333")); // Dark
    
    for(auto slice : seriesCat->slices()) {
        slice->setLabelVisible();
    }
    
    QChart *chartCat = new QChart();
    chartCat->addSeries(seriesCat);
    chartCat->setTitle("Category Distribution");
    chartCat->setAnimationOptions(QChart::SeriesAnimations);
    chartCat->legend()->setAlignment(Qt::AlignRight);
    chartCat->setBackgroundBrush(Qt::transparent);
    
    QChartView *viewCat = new QChartView(chartCat);
    viewCat->setRenderHint(QPainter::Antialiasing);
    viewCat->setStyleSheet("background: transparent;");
    
    // Layout for Chart 1
    if (!ui_supplier->frame_chart_types->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui_supplier->frame_chart_types);
        ui_supplier->frame_chart_types->setLayout(layout);
    }
    // Remove placeholder
    if (ui_supplier->chart_types_view) ui_supplier->chart_types_view->setVisible(false);
    
    ui_supplier->frame_chart_types->layout()->addWidget(viewCat);
    
    
    // --- Chart 2: Satisfaction Trend (Bar) ---
    // Target: frame_chart_reviews
    QBarSet *setScore = new QBarSet("Score");
    *setScore << 85 << 88 << 90 << 92 << 89 << 94;
    setScore->setColor(QColor("#8B6F47"));
    
    QBarSeries *seriesTrend = new QBarSeries();
    seriesTrend->append(setScore);
    
    QChart *chartTrend = new QChart();
    chartTrend->addSeries(seriesTrend);
    chartTrend->setTitle("6-Month Satisfaction Trend");
    chartTrend->setAnimationOptions(QChart::SeriesAnimations);
    chartTrend->setBackgroundBrush(Qt::transparent);
    
    QStringList months;
    months << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    chartTrend->addAxis(axisX, Qt::AlignBottom);
    seriesTrend->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    chartTrend->addAxis(axisY, Qt::AlignLeft);
    seriesTrend->attachAxis(axisY);
    
    QChartView *viewTrend = new QChartView(chartTrend);
    viewTrend->setRenderHint(QPainter::Antialiasing);
    viewTrend->setStyleSheet("background: transparent;");

    // Layout for Chart 2
    if (!ui_supplier->frame_chart_reviews->layout()) {
        QVBoxLayout *layout = new QVBoxLayout(ui_supplier->frame_chart_reviews);
        ui_supplier->frame_chart_reviews->setLayout(layout);
    }
    // Remove placeholder
    if (ui_supplier->chart_reviews_view) ui_supplier->chart_reviews_view->setVisible(false);
    
    ui_supplier->frame_chart_reviews->layout()->addWidget(viewTrend);
}

void MainWindow::setupClientCalendar()
{
    // 1. Create the Tab Widget if it doesn't exist (it should, 'tabWidget')
    // We will add a new tab to it.
    QWidget *calendarTab = new QWidget();
    
    // Layout for the new tab
    QHBoxLayout *mainLayout = new QHBoxLayout(calendarTab);
    
    // --- Left Side: Calendar ---
    QCalendarWidget *calendar = new QCalendarWidget();
    calendar->setGridVisible(true);
    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    
    // Professional Styling
    calendar->setStyleSheet(R"(
        QCalendarWidget QToolButton {
            color: #333;
            icon-size: 24px;
            font-weight: bold;
            background-color: #E0E0E0;
            border-radius: 5px;
            margin: 5px;
        }
        QCalendarWidget QMenu {
            width: 150px;
            left: 20px;
            color: white;
            font-size: 14px;
            background-color: #8B6F47;
        }
        QCalendarWidget QSpinBox {
            width: 80px;
            font-size: 14px;
            color: #8B6F47;
            font-weight: bold;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar { 
            background-color: white; 
            border: 1px solid #C4C4C4;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            padding: 5px;
        }
        QCalendarWidget QAbstractItemView:enabled {
            font-size: 14px;
            color: #333;
            background-color: white;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
    )");

    // --- Right Side: Events Panel ---
    QGroupBox *eventGroup = new QGroupBox("Daily Agenda");
    eventGroup->setStyleSheet(R"(
        QGroupBox { 
            border: 1px solid #8B6F47; 
            border-radius: 8px; 
            margin-top: 20px; 
            background: rgba(255, 255, 255, 0.9);
            font-weight: bold; 
            color: #8B6F47; 
        }
        QGroupBox::title { 
            subcontrol-origin: margin; 
            subcontrol-position: top center; 
            padding: 0 10px;
            background-color: white;
        }
    )");
    
    QVBoxLayout *eventLayout = new QVBoxLayout(eventGroup);
    
    QLabel *lblDate = new QLabel("Select a date...");
    lblDate->setStyleSheet("font-size: 16px; font-weight: bold; color: #555; margin-bottom: 10px;");
    lblDate->setAlignment(Qt::AlignCenter);
    
    QListWidget *eventList = new QListWidget();
    eventList->setStyleSheet(R"(
        QListWidget {
            border: none;
            background: transparent;
            font-size: 14px;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #EEE;
        }
        QListWidget::item:selected {
            background-color: rgba(139, 111, 71, 0.1);
            color: #333;
        }
    )");
    
    // Initial Mock Events
    eventList->addItem(new QListWidgetItem("📅  09:00 AM - Team Sync"));
    eventList->addItem(new QListWidgetItem("💼  11:30 AM - Client Meeting (John Doe)"));
    eventList->addItem(new QListWidgetItem("📊  02:00 PM - Quarterly Review"));
    
    eventLayout->addWidget(lblDate);
    eventLayout->addWidget(eventList);
    
    // --- Connect Interaction ---
    connect(calendar, &QCalendarWidget::clicked, [lblDate, eventList](const QDate &date){
        lblDate->setText(date.toString("dddd, MMMM d, yyyy"));
        
        // Mocking dynamic events based on day logic
        eventList->clear();
        if (date.day() % 3 == 0) {
            eventList->addItem(new QListWidgetItem("✅  No events scheduled."));
        } else if (date.day() % 2 == 0) {
            eventList->addItem(new QListWidgetItem("📞  10:00 AM - Call with Supplier"));
            eventList->addItem(new QListWidgetItem("🛒  01:00 PM - Order #1234 Delivery"));
            eventList->addItem(new QListWidgetItem("📝  04:00 PM - Sign Contract"));
        } else {
            eventList->addItem(new QListWidgetItem("📅  09:00 AM - Team Sync"));
            eventList->addItem(new QListWidgetItem("💼  11:30 AM - Client Meeting"));
            eventList->addItem(new QListWidgetItem("📊  03:00 PM - Strategy Workshop"));
        }
    });

    // Add widgets to main layout
    mainLayout->addWidget(calendar, 70); // 70% width
    mainLayout->addWidget(eventGroup, 30); // 30% width
    
    // Add the new tab
    ui_client->tabWidget->addTab(calendarTab, "Smart Calendar");
    
    // Set an icon if available, or just text
    // ui_client->tabWidget->setTabIcon(..., QIcon(":/assets/icon_calendar.png"));
}

void MainWindow::setupEmployeeModes()
{
    // Rename tab_add to "Manage Employees"
    int addIdx = ui_employee->tabWidget->indexOf(ui_employee->tab_add);
    if(addIdx != -1) ui_employee->tabWidget->setTabText(addIdx, "Manage Employees");

    // tab_modify does not exist in this UI, so no need to remove it.


    // Create Radio Buttons in tab_add
    QRadioButton *rbAdd = new QRadioButton("Add Employee", ui_employee->tab_add);
    QRadioButton *rbMod = new QRadioButton("Modify Employee", ui_employee->tab_add);

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // Move group lower
    ui_employee->group_add->move(20, 70);

    // Lambda to update UI
    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_employee->group_add->setTitle("Add Employee");
            ui_employee->btn_add->setVisible(true);
            ui_employee->btn_modify->setVisible(false);
            // btn_cancel is common
        } else {
            ui_employee->group_add->setTitle("Modify Employee");
            ui_employee->btn_add->setVisible(false);
            ui_employee->btn_modify->setVisible(true);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    // Init
    updateUI(true);
}

void MainWindow::setupSupplierModes()
{
    // Rename tab_gestion to "Manage Suppliers"
    // (Assuming tab_gestion index 0)
    ui_supplier->tabWidget->setTabText(0, "Manage Suppliers");

    QRadioButton *rbAdd = new QRadioButton("Add Supplier", ui_supplier->tab_gestion);
    QRadioButton *rbMod = new QRadioButton("Manage Supplier", ui_supplier->tab_gestion);

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);
    
    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    // groupBox_gestion geometry check? It might need moving depending on layout
    // UI file says y=20. We put radios at y=10. Might overlap.
    // Move group lower
    ui_supplier->groupBox_gestion->move(20, 70);

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_supplier->groupBox_gestion->setTitle("Add New Supplier");
            ui_supplier->btn_add->setVisible(true);
            ui_supplier->btn_modify->setVisible(false);
            ui_supplier->btn_delete->setVisible(false);
        } else {
            ui_supplier->groupBox_gestion->setTitle("Manage Existing Supplier");
            ui_supplier->btn_add->setVisible(false);
            ui_supplier->btn_modify->setVisible(true);
            ui_supplier->btn_delete->setVisible(true);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);
}

void MainWindow::setupEquipmentModes()
{
    // tab_gestion
    ui_equipment->tabWidget->setTabText(0, "Manage Equipment");

    QRadioButton *rbAdd = new QRadioButton("Add Equipment", ui_equipment->tab_gestion);
    QRadioButton *rbMod = new QRadioButton("Manage Equipment", ui_equipment->tab_gestion);

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 180, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_equipment->groupBox_gestion->move(20, 70);

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_equipment->groupBox_gestion->setTitle("Add Equipment");
            ui_equipment->btn_add->setVisible(true);
            ui_equipment->btn_modify->setVisible(false);
            ui_equipment->btn_delete->setVisible(false);
        } else {
            ui_equipment->groupBox_gestion->setTitle("Manage Equipment");
            ui_equipment->btn_add->setVisible(false);
            ui_equipment->btn_modify->setVisible(true);
            ui_equipment->btn_delete->setVisible(true);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);
}

void MainWindow::setupOrderModes()
{
    // tab_manage
    int idx = ui_order->tabWidget->indexOf(ui_order->tab_manage);
    if(idx != -1) ui_order->tabWidget->setTabText(idx, "Manage Orders");

    QRadioButton *rbAdd = new QRadioButton("Add Order", ui_order->tab_manage);
    QRadioButton *rbMod = new QRadioButton("Manage Order", ui_order->tab_manage);

    rbAdd->setGeometry(700, 25, 150, 30);
    rbMod->setGeometry(850, 25, 150, 30);

    QString rbStyle = "font-weight: bold; font-size: 14px; color: white;";
    rbAdd->setStyleSheet(rbStyle);
    rbMod->setStyleSheet(rbStyle);

    rbAdd->setChecked(true);
    rbAdd->show();
    rbMod->show();

    ui_order->group_manage->move(20, 70);

    auto updateUI = [=](bool isAdd) {
        if(isAdd) {
            ui_order->group_manage->setTitle("Add New Order");
            ui_order->btn_add->setVisible(true);
            ui_order->btn_modify->setVisible(false);
            ui_order->btn_delete->setVisible(false);
            ui_order->btn_load->setVisible(false);
        } else {
            ui_order->group_manage->setTitle("Manage Existing Order");
            ui_order->btn_add->setVisible(false);
            ui_order->btn_modify->setVisible(true);
            ui_order->btn_delete->setVisible(true);
            ui_order->btn_load->setVisible(true);
        }
    };

    connect(rbAdd, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(true); });
    connect(rbMod, &QRadioButton::toggled, [=](bool c){ if(c) updateUI(false); });

    updateUI(true);
}

void MainWindow::setupGlobalStyles()
{
    QString style = R"(
        /* --- General Application Style --- */
        QWidget {
            font-family: 'Gadugi', 'Segoe UI', sans-serif;
            font-size: 14px;
        }

        /* --- Buttons --- */
        QPushButton {
            background-color: #8B6F47; /* Gold/Brown */
            color: white;
            border-radius: 5px;
            padding: 8px 15px;
            font-weight: bold;
            border: 1px solid #6d5638;
        }
        QPushButton:hover {
            background-color: #a38253;
            border: 1px solid #8B6F47;
        }
        QPushButton:pressed {
            background-color: #6d5638;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
            border: 1px solid #aaaaaa;
        }

        /* --- Input Fields --- */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QComboBox {
            background-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
            padding: 5px;
            color: #333333;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QComboBox:focus {
            border: 1px solid #8B6F47;
        }

        /* --- Group Boxes --- */
        QGroupBox {
            border: 1px solid #8B6F47;
            border-radius: 6px;
            margin-top: 24px; /* Leave space for title */
            background-color: rgba(255, 255, 255, 0.8); /* Slight transparency */
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 5px 10px;
            background-color: #8B6F47;
            color: white;
            border-radius: 4px;
            font-weight: bold;
        }

        /* --- Tab Widget --- */
        QTabWidget::pane {
            border: 1px solid #cccccc;
            background: rgba(255, 255, 255, 0.9);
            border-radius: 4px;
        }
        QTabWidget::tab-bar {
            left: 5px; /* move to the right by 5px */
        }
        QTabBar::tab {
            background: #e0e0e0;
            border: 1px solid #cccccc;
            border-bottom-color: #cccccc; /* same as the pane color */
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            min-width: 8ex;
            padding: 8px 15px;
            margin-right: 2px;
            color: #333;
        }
        QTabBar::tab:selected, QTabBar::tab:hover {
            background: #8B6F47;
            color: white;
            border-color: #8B6F47;
        }

        /* --- Tables & Lists --- */
        QTableView, QListWidget {
            border: 1px solid #cccccc;
            gridline-color: #eeeeee;
            background-color: white;
            selection-background-color: rgba(139, 111, 71, 0.2); /* Light Gold */
            selection-color: black;
            alternate-background-color: #f9f9f9;
        }
        QHeaderView::section {
            background-color: #8B6F47;
            color: white;
            padding: 5px;
            border: none;
            font-weight: bold;
        }
        
        /* --- Scrollbars --- */
        QScrollBar:vertical {
            border: none;
            background: #f0f0f0;
            width: 10px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #cdcdcd;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: #8B6F47;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    
    // Apply style to the entire application to ensure consistency
    qApp->setStyleSheet(style);
}

void MainWindow::setupTabNavigation(QWidget* parentWidget, QTabWidget* tabWidget, const QStringList& tabNames, int startX, int yPos, const QList<int>& targetIndices)
{
    int y = yPos;
    int spacing = 120;
    
    QString rbStyle = "QRadioButton { font-weight: bold; font-size: 14px; color: white; } QRadioButton::indicator { width: 15px; height: 15px; }";
    
    QButtonGroup *group = new QButtonGroup(parentWidget);
    group->setExclusive(true);
    
    for(int i = 0; i < tabNames.size(); i++) {
        QRadioButton *rb = new QRadioButton(tabNames[i], parentWidget);
        rb->setGeometry(startX + (spacing * i), y, 110, 30);
        rb->setStyleSheet(rbStyle);
        
        if(i == 0) rb->setChecked(true);
        
        // Connect to switch tabs
        connect(rb, &QRadioButton::toggled, [=](bool checked){
            if(checked) {
                int index = (targetIndices.size() > i) ? targetIndices[i] : i;
                tabWidget->setCurrentIndex(index);
            }
        });

        group->addButton(rb, i);
    }
}

