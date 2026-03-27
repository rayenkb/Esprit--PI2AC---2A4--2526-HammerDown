#include "loginwindow.h"
#include "ui_login.h"
#include <QPainter>
#include <QPainterPath>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QBuffer>

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

static bool isSameFaceLocally(const QImage& img1, const QImage& img2) {
    if (img1.isNull() || img2.isNull()) return false;
    
    QImage i1 = img1.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_Grayscale8);
    QImage i2 = img2.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_Grayscale8);
    
    long long diff = 0;
    for (int y = 0; y < 32; ++y) {
        const uchar* p1 = i1.constScanLine(y);
        const uchar* p2 = i2.constScanLine(y);
        for (int x = 0; x < 32; ++x) {
            diff += std::abs(p1[x] - p2[x]);
        }
    }
    double avgDiff = (double)diff / (32.0 * 32.0);
    // Threshold 60.0 allows standard lighting deviations 
    return avgDiff < 60.0;
}

void LoginWindow::handleFaceLogin() {
    QString idStr = ui->login_id->text().trimmed();
    if(idStr.isEmpty()){
        QMessageBox::warning(this, tr("Identity Verification"), tr("Please enter your Employee ID in the login field before scanning your face."));
        return;
    }
    int employeeId = idStr.toInt();
    QString savedFacePath = QString("data/faces/face_%1.png").arg(employeeId);
    if (!QFile::exists(savedFacePath)) {
        QMessageBox::information(this, tr("Face Login"), tr("No face registered for this ID. Please register in Employee Management first."));
        return;
    }

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
        ui->btn_face_login->setText("Scanning...");
        ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: #8B6F47; border: 1px solid white; border-radius: 22px; color: white; }");
        
        // Scan for 3 seconds, then verify
        QTimer::singleShot(3000, this, [this, savedFacePath, employeeId]() {
            if (!m_isFaceLoginActive) return;
            
            ui->btn_face_login->setText("Verifying...");
            
            QVideoFrame frame = m_videoSink->videoFrame();
            QImage liveImage = frame.toImage().convertToFormat(QImage::Format_RGB888).mirrored(true, false);
            
            m_camera->stop();
            m_scanLineTimer->stop();
            m_isFaceLoginActive = false;
            
            QImage regImage(savedFacePath);
            
            ui->lbl_camera_preview->hide();
            ui->btn_face_login->setText("Face Scan Login");
            ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: rgba(139, 111, 71, 0.2); border: 1.5px solid #8B6F47; border-radius: 22px; }");
            
            if (isSameFaceLocally(liveImage, regImage)) {
                emit loginSuccessful(employeeId);
            } else {
                QMessageBox::critical(this, "Security Breach", "Access Denied. Biomolecular profile does not match the registered credentials.");
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

    // Mirror image for more natural preview
    image = image.mirrored(true, false);

    QPixmap pix = QPixmap::fromImage(image);
    // Use scaling that fills the label while keeping aspect ratio, then crop
    QSize labelSize = ui->lbl_camera_preview->size();
    QPixmap scaledPix = pix.scaled(labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    // Center crop to label size
    QRect cropRect( (scaledPix.width() - labelSize.width()) / 2,
                    (scaledPix.height() - labelSize.height()) / 2,
                    labelSize.width(), labelSize.height());
    QPixmap croppedPix = scaledPix.copy(cropRect);

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
