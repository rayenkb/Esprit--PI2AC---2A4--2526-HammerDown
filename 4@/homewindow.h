#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QFrame>
#include <QFrame>

class QSlider;
class QPushButton;

namespace Ui {
class HomeFrame;
}

class HomeWindow : public QFrame
{
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();

signals:
    void employesClicked();
    void clientClicked();
    void orderClicked();
    void fournisseurClicked();
    void equipmentClicked();

private slots:
    void handleEmployes();
    void handleClient();
    void handleOrder();
    void handleFournisseur();
    void handleEquipment();
    
private slots:
    void updateLanguage(QString lang);

private:
    Ui::HomeFrame *ui;
    void setupHomeButtons();
    
    // UI Components for Sound and Language
    QSlider *volumeSlider;
    QPushButton *btnEn;
    QPushButton *btnFr;
    
    void setupSoundAndLanguage();
};

#endif // HOMEWINDOW_H
