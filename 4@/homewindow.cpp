#include "homewindow.h"
#include "ui_home.h"
#include <QSlider>
#include <QPushButton>
#include <QHBoxLayout>

HomeWindow::HomeWindow(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::HomeFrame)
{
    ui->setupUi(this);
    
    // Connect buttons to their handlers
    connect(ui->gs_employes,    &QPushButton::clicked, this, &HomeWindow::handleEmployes);
    connect(ui->gs_client,      &QPushButton::clicked, this, &HomeWindow::handleClient);
    connect(ui->gs_order,        &QPushButton::clicked, this, &HomeWindow::handleOrder);
    connect(ui->gs_fournisseur, &QPushButton::clicked, this, &HomeWindow::handleFournisseur);
    connect(ui->gs_equipment,   &QPushButton::clicked, this, &HomeWindow::handleEquipment);
    
    // Apply professional styling
    setupHomeButtons();
    setupSoundAndLanguage();
}

void HomeWindow::setupSoundAndLanguage()
{
    // Create container for controls
    QWidget *controlsContainer = new QWidget(this);
    controlsContainer->setStyleSheet("background: transparent;");
    controlsContainer->setGeometry(1000, 600, 300, 50); // Bottom-Right positioning

    QHBoxLayout *layout = new QHBoxLayout(controlsContainer);
    layout->setContentsMargins(0, 0, 0, 0);

    // --- Sound Bar ---
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setFixedWidth(150);
    volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal { border: 1px solid #999999; height: 8px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); margin: 2px 0; border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #8B6F47; border: 1px solid #5c5c5c; width: 18px; margin: -2px 0; border-radius: 9px; }"
    );

    // --- Language Buttons ---
    btnEn = new QPushButton("EN");
    btnFr = new QPushButton("FR");
    
    QString btnStyle = "QPushButton { background-color: #333; color: white; border: 1px solid #555; border-radius: 5px; padding: 5px; font-weight: bold; } QPushButton:hover { background-color: #555; }";
    btnEn->setStyleSheet(btnStyle);
    btnFr->setStyleSheet(btnStyle);
    
    btnEn->setFixedSize(40, 30);
    btnFr->setFixedSize(40, 30);
    
    // Add to layout
    layout->addWidget(volumeSlider);
    layout->addWidget(btnEn);
    layout->addWidget(btnFr);

    // Connect Signals
    connect(btnEn, &QPushButton::clicked, [=](){ updateLanguage("EN"); });
    connect(btnFr, &QPushButton::clicked, [=](){ updateLanguage("FR"); });
}

void HomeWindow::updateLanguage(QString lang)
{
    if (lang == "FR") {
        // French Mode: Visible buttons with text overlay
        QString styleFr = "QPushButton { background-color: rgba(0, 0, 0, 0.85); color: #FFD700; border: 2px solid #8B6F47; border-radius: 20px; font-weight: bold; font-size: 16px; }"
                          "QPushButton:hover { background-color: rgba(50, 50, 50, 0.9); }";

        ui->gs_employes->setText("Gestion Employés");
        ui->gs_client->setText("Gestion Clients");
        ui->gs_order->setText("Gestion Commandes");
        ui->gs_fournisseur->setText("Gestion Fournisseurs");
        ui->gs_equipment->setText("Gestion Équipements");

        ui->gs_employes->setStyleSheet(styleFr);
        ui->gs_client->setStyleSheet(styleFr);
        ui->gs_order->setStyleSheet(styleFr);
        ui->gs_fournisseur->setStyleSheet(styleFr);
        ui->gs_equipment->setStyleSheet(styleFr);
    } else {
        // English Mode: Invisible buttons (default)
        QString styleEn = "QPushButton { background-color: transparent; border: none; border-radius: 20px; } QPushButton:hover { background-color: transparent; border: none; } QPushButton:pressed { background-color: transparent; border: none; }";

        ui->gs_employes->setText("");
        ui->gs_client->setText("");
        ui->gs_order->setText("");
        ui->gs_fournisseur->setText("");
        ui->gs_equipment->setText("");

        ui->gs_employes->setStyleSheet(styleEn);
        ui->gs_client->setStyleSheet(styleEn);
        ui->gs_order->setStyleSheet(styleEn);
        ui->gs_fournisseur->setStyleSheet(styleEn);
        ui->gs_equipment->setStyleSheet(styleEn);
    }
}

HomeWindow::~HomeWindow()
{
    delete ui;
}

void HomeWindow::handleEmployes()    { emit employesClicked();    }
void HomeWindow::handleClient()      { emit clientClicked();      }
void HomeWindow::handleOrder()        { emit orderClicked();        }
void HomeWindow::handleFournisseur() { emit fournisseurClicked(); }
void HomeWindow::handleEquipment()   { emit equipmentClicked();   }

void HomeWindow::setupHomeButtons()
{
    // List of standard buttons (Employee, Client, Order, Equipment)
    QList<QPushButton*> standardButtons = {
        ui->gs_employes,
        ui->gs_client,
        ui->gs_order,
        ui->gs_equipment
    };

    // Professional stylesheet:
    // - Transparent base
    // - Subtle white hover with rounded corners
    // - "Fitted" look (no border)
    QString style = R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 20px; 
        }
        QPushButton:hover {
            background-color: transparent;
            border: none;
        }
        QPushButton:pressed {
            background-color: transparent;
            border: none;
        }
    )";

    // Apply to standard buttons (approx 360x120 based on visual plaque)
    for (QPushButton* btn : standardButtons) {
        btn->setStyleSheet(style);
        btn->setFixedSize(360, 120); 
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    
    // Apply to Supplier button (Wider, approx 560x120)
    ui->gs_fournisseur->setStyleSheet(style);
    ui->gs_fournisseur->setFixedSize(560, 120);
    ui->gs_fournisseur->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
