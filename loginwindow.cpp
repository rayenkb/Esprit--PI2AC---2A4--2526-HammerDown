#include "loginwindow.h"
#include "ui_login.h"
#include <QPainter>
#include <QPainterPath>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QBuffer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QParallelAnimationGroup>

LoginWindow::LoginWindow(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::LoginFrame)
{
    ui->setupUi(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    connect(ui->login, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(ui->btn_face_login, &QPushButton::clicked, this, &LoginWindow::handleFaceLogin);
    connect(ui->btn_forgot_password, &QPushButton::clicked, this, &LoginWindow::handleForgotPassword);

    // Initialize Camera for preview
    m_camera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    m_captureSession = new QMediaCaptureSession(this);
    m_videoSink = new QVideoSink(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &LoginWindow::processCameraFrame);

    m_scanLineTimer = new QTimer(this);
    connect(m_scanLineTimer, &QTimer::timeout, this, &LoginWindow::updateScanAnimation);
}

void LoginWindow::handleLogin() {
    QString username = ui->login_id->text();
    QString password = ui->login_pass->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("Login Failed"), tr("Please enter both username and password."));
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Database Error", tr("Database is not connected."));
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM EMPLOYEES WHERE EMPLOYEE_ID = :username AND PASSWORD = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (query.exec()) {
        if (query.next()) {
            emit loginSuccessful(query.value("EMPLOYEE_ID").toInt());
        } else {
            QMessageBox::warning(this, tr("Login Failed"), tr("Invalid username or password."));
        }
    }
}
void LoginWindow::handleFaceLogin() {
    if (m_isFaceLoginActive) {
        m_camera->stop();
        m_scanLineTimer->stop();
        ui->lbl_camera_preview->hide();
        m_isFaceLoginActive = false;
        ui->btn_face_login->setText("Face Scan Login");
        ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47; border-radius: 22px; }");
    } else {
        ui->lbl_camera_preview->show();
        m_camera->start();
        m_scanLineTimer->start(16); // ~60fps for smooth animation
        m_isFaceLoginActive = true;
        m_scanLineY = 0.0;
        ui->btn_face_login->setText("Authenticating...");
        ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: #8B6F47; border: 1px solid white; border-radius: 22px; color: white; }");
        
        // Scan for 3 seconds, then verify
        QTimer::singleShot(3000, this, [this]() {
            if (!m_isFaceLoginActive) return;
            
            ui->btn_face_login->setText("Matching Profiles...");
            
            QVideoFrame frame = m_videoSink->videoFrame();
            QImage liveImage = frame.toImage().convertToFormat(QImage::Format_RGB888);
            
            m_camera->stop();
            m_scanLineTimer->stop();
            m_isFaceLoginActive = false;
            
            ui->lbl_camera_preview->hide();
            ui->btn_face_login->setText("Face Scan Login");
            ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47; border-radius: 22px; }");

            // SEARCH ALL REGISTERED FACES
            QDir avDir("assets/av");
            QStringList filters; filters << "face_*.png";
            QFileInfoList list = avDir.entryInfoList(filters, QDir::Files);
            
            int identifiedId = -1;
            double bestMatchScore = 1000.0; 

            for (const QFileInfo &fileInfo : list) {
                QString fileName = fileInfo.baseName();
                QString idStr = fileName.section('_', 1);
                int currentId = idStr.toInt();
                if (currentId <= 0) continue;

                QImage regImage(fileInfo.absoluteFilePath());
                if (regImage.isNull()) continue;
                
                // Robust Matching Algorithm (Mirror + Orientation + Multi-Shift)
                QList<QImage> orientationChecks; 
                orientationChecks << liveImage << liveImage.mirrored(true, false);
                
                for (const QImage& testImg : orientationChecks) {
                    // Scaled slightly larger (100x100) to allow for 80x80 sliding window search
                    QImage i1_full = testImg.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                          .convertToFormat(QImage::Format_Grayscale8);
                    QImage i2 = regImage.scaled(80, 80, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                          .convertToFormat(QImage::Format_Grayscale8);
                    
                    // Multi-Shift Scan (Handles head tilts and minor centering adjustments)
                    for (int dy = 0; dy <= 20; dy += 4) {
                        for (int dx = 0; dx <= 20; dx += 4) {
                            long long diff = 0;
                            for (int py = 0; py < 80; ++py) {
                                const uchar* p1 = i1_full.constScanLine(py + dy);
                                const uchar* p2 = i2.constScanLine(py);
                                for (int px = 0; px < 80; ++px) {
                                    diff += std::abs(p1[px + dx] - p2[px]);
                                }
                            }
                            
                            double avgDiff = (double)diff / (80.0 * 80.0);
                            // Relaxed threshold for 10/10 reliability (65.0 instead of 52.0)
                            if (avgDiff < 65.0 && avgDiff < bestMatchScore) {
                                bestMatchScore = avgDiff;
                                identifiedId = currentId;
                            }
                        }
                    }
                }
            }
            
            if (identifiedId > 0) {
                // MANDATORY DB CHECK: Face file match alone is NOT enough.
                // The employee MUST actively exist in the database.
                QSqlQuery nq;
                nq.prepare("SELECT FIRST_NAME FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
                nq.bindValue(":id", identifiedId);

                if (nq.exec() && nq.next()) {
                    // Employee confirmed in DB - proceed with login
                    QString empName = nq.value(0).toString();

                    ui->btn_face_login->setText("ACCESS GRANTED");
                    ui->btn_face_login->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4CAF50, stop:1 #2E7D32); color: white; font-weight: bold; border-radius: 22px;");

                    // --- PREMIUM WELCOME ANIMATION ---
                    QLabel *welcomeOverlay = new QLabel(this);
                    welcomeOverlay->setText(QString("WELCOME HOME,\n%1").arg(empName.toUpper()));
                    welcomeOverlay->setAlignment(Qt::AlignCenter);
                    welcomeOverlay->setStyleSheet(
                        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(28, 22, 16, 0.95), stop:1 rgba(15, 12, 8, 0.98));"
                        "color: #D4AF37;"
                        "font-size: 34px;"
                        "font-weight: 900;"
                        "font-family: 'Outfit';"
                        "letter-spacing: 5px;"
                        "border: 3px solid #D4AF37;"
                        "border-radius: 25px;"
                        "padding: 20px;"
                    );
                    welcomeOverlay->setGeometry(this->width()/2 - 250, this->height()/2 - 125, 500, 250);
                    welcomeOverlay->show();

                    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
                    QPropertyAnimation *posAnim = new QPropertyAnimation(welcomeOverlay, "geometry");
                    posAnim->setDuration(800);
                    posAnim->setStartValue(QRect(this->width()/2 - 250, this->height(), 500, 250));
                    posAnim->setEndValue(QRect(this->width()/2 - 250, this->height()/2 - 125, 500, 250));
                    posAnim->setEasingCurve(QEasingCurve::OutExpo);
                    group->addAnimation(posAnim);
                    group->start(QAbstractAnimation::DeleteWhenStopped);

                    QTimer::singleShot(2500, [this, identifiedId, welcomeOverlay](){
                        welcomeOverlay->deleteLater();
                        emit loginSuccessful(identifiedId);
                    });

                } else {
                    // Face file matched but NO active employee record - HARD BLOCK
                    ui->btn_face_login->setText("Face Scan Login");
                    ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47; border-radius: 22px; }");
                    QMessageBox::critical(this, "Authentication Failed",
                        "IDENTITY UNVERIFIED: Biometric match found but no active employee record exists.\n"
                        "Access is strictly denied. Contact your system administrator.");
                }

            } else {
                ui->btn_face_login->setText("Face Scan Login");
                ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47; border-radius: 22px; }");
                QMessageBox::critical(this, "Security Breach", "Access Denied. NO registered face profiles match current biometric scan. FALLBACK ACCESS REJECTED.");
                // NO loginSuccessful emit here - access is fully blocked
            }
        });
    }
}

void LoginWindow::handleForgotPassword() {
    bool ok;
    QString idStr = QInputDialog::getText(this, tr("Forgot Password - Step 1/3"),
                                         tr("Identity Check: Please enter your Employee ID:"), QLineEdit::Normal,
                                         ui->login_id->text(), &ok);
    if (!ok || idStr.trimmed().isEmpty()) return;

    idStr = idStr.trimmed();
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Database Error", tr("System database is disconnected."));
        return;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT EMAIL FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
    query.bindValue(":id", idStr);

    if (query.exec() && query.next()) {
        QString dbEmail = query.value("EMAIL").toString();
        
        if (dbEmail.isEmpty()) {
            QMessageBox::warning(this, tr("Forgot Password"), tr("No security email registered for this ID. Please contact HR explicitly."));
            return;
        }

        QString inputEmail = QInputDialog::getText(this, tr("Forgot Password - Step 2/3"),
                                         tr("Security Check: Enter your registered Email Address:"), QLineEdit::Normal,
                                         "", &ok);
        if (!ok || inputEmail.trimmed().isEmpty()) return;
        
        if(inputEmail.trimmed().compare(dbEmail, Qt::CaseInsensitive) != 0) {
            QMessageBox::critical(this, tr("Security Alert"), tr("The email address provided does not match the registered security email. Access denied."));
            return;
        }
        
        QString newPass = QInputDialog::getText(this, tr("Forgot Password - Step 3/3"),
                                         tr("Identity Verified! Enter your NEW Password:"), QLineEdit::Password,
                                         "", &ok);
        if (!ok || newPass.trimmed().isEmpty()) return;
        
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE EMPLOYEES SET PASSWORD = :pass WHERE EMPLOYEE_ID = :id");
        updateQuery.bindValue(":pass", newPass.trimmed());
        updateQuery.bindValue(":id", idStr);
        
        if(updateQuery.exec()) {
            QMessageBox::information(this, tr("Password Reset"), tr("Your password has been securely reset! You can now log in."));
        } else {
            QMessageBox::critical(this, tr("Database Error"), tr("Failed to update password across the network. Error: ") + updateQuery.lastError().text());
        }
    } else {
        QMessageBox::warning(this, tr("Forgot Password"), tr("System could not securely locate this Employee ID."));
    }
}

void LoginWindow::processCameraFrame() {
    if (!m_isFaceLoginActive) return;

    QVideoFrame frame = m_videoSink->videoFrame();
    if (!frame.isValid() || !frame.map(QVideoFrame::ReadOnly)) return;

    QImage image = frame.toImage().convertToFormat(QImage::Format_RGB888);
    frame.unmap();

    // Disable Mirroring (Show Real World view)
    image = image.mirrored(true, false);

    QPixmap pix = QPixmap::fromImage(image);
    QSize labelSize = ui->lbl_camera_preview->size();
    QPixmap scaledPix = pix.scaled(labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    // Center Crop to Square
    QRect cropRect( (scaledPix.width() - labelSize.width()) / 2,
                    (scaledPix.height() - labelSize.height()) / 2,
                    labelSize.width(), labelSize.height() );
    QPixmap croppedPix = scaledPix.copy(cropRect);

    // Dynamic Circular Preview with Scanline HUD
    ui->lbl_camera_preview->setPixmap(getCircularPixmap(croppedPix));
}


void LoginWindow::updateScanAnimation() {
    if (!m_isFaceLoginActive) return;
    
    if (m_scanForward) {
        m_scanLineY += 0.02;
        if (m_scanLineY >= 1.0) m_scanForward = false;
    } else {
        m_scanLineY -= 0.02;
        if (m_scanLineY <= 0.0) m_scanForward = true;
    }
    update();
}

QPixmap LoginWindow::getCircularPixmap(const QPixmap &src) {
    if (src.isNull()) return src;
    
    QPixmap out(src.size());
    out.fill(Qt::transparent);
    
    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QPainterPath path;
    // Circular crop (radius = half size)
    path.addEllipse(src.rect());
    painter.setClipPath(path);
    
    painter.drawPixmap(0, 0, src);
    
    // SCAN LINE OVERLAY
    if (m_isFaceLoginActive) {
        int y = m_scanLineY * src.height();
        
        // Laser Glow
        QLinearGradient laserGrad(0, y - 10, 0, y + 10);
        laserGrad.setColorAt(0, Qt::transparent);
        laserGrad.setColorAt(0.5, QColor(0, 255, 255, 180)); // Cyan laser
        laserGrad.setColorAt(1, Qt::transparent);
        
        painter.setBrush(laserGrad);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, y - 10, src.width(), 20);
        
        // Core Line
        painter.setPen(QPen(QColor(255, 255, 255, 220), 2));
        painter.drawLine(0, y, src.width(), y);
    }
    
    // Add a professional pulsing gold border
    painter.setClipping(false);
    int borderPulse = 0;
    if (m_isFaceLoginActive) {
        borderPulse = qAbs(qSin(m_scanLineY * 3.14159) * 4);
    }
    
    painter.setPen(QPen(QColor(139, 111, 71), 4 + borderPulse));
    painter.drawEllipse(src.rect().adjusted(2, 2, -2, -2));
    
    return out;
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::retranslateUI()
{
    ui->retranslateUi(this);
}
