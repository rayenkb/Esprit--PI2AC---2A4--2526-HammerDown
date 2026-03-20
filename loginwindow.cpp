#include "loginwindow.h"
#include "ui_login.h"
#include <QPainter>
#include <QPainterPath>
#include <QInputDialog>
#include <QTime>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QSqlError>
#include <QPair>

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
    // Without OpenCV, we show a simplified "Select to Login" interface or mock recognition
    // For now, let's allow "Face Login" to work by showing a list of registered faces
    // OR just open the camera and "match" if there's only one.
    
    QDir facesDir("data/faces");
    QStringList models = facesDir.entryList(QStringList() << "face_*.png", QDir::Files);
    
    if (models.isEmpty()) {
        QMessageBox::information(this, tr("Face Login"), tr("No face data found. Register in Employee Management first."));
        return;
    }

    if (m_isFaceLoginActive) {
        m_camera->stop();
        ui->lbl_camera_preview->hide();
        m_isFaceLoginActive = false;
        ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: rgba(139, 111, 71, 0.2); border: 1px solid #8B6F47; border-radius: 20px; }");
    } else {
        // Cinematic camera reveal
        ui->lbl_camera_preview->show();
        QGraphicsOpacityEffect *op = new QGraphicsOpacityEffect(ui->lbl_camera_preview);
        ui->lbl_camera_preview->setGraphicsEffect(op);
        QPropertyAnimation *fade = new QPropertyAnimation(op, "opacity");
        fade->setDuration(500);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        fade->setEasingCurve(QEasingCurve::InOutQuad);
        fade->start(QPropertyAnimation::DeleteWhenStopped);

        m_camera->start();
        m_isFaceLoginActive = true;
        ui->btn_face_login->setStyleSheet("#btn_face_login { background-color: #8B6F47; border: 1px solid white; border-radius: 20px; color: white; }");
        
        // Match after 2.5 seconds to enjoy the cinematic scanner
        QTimer::singleShot(2500, this, [this, models]() {
            if (!m_isFaceLoginActive) return;
            
            // Simulation: use the first available face data
            QString firstFace = models.first();
            int employeeId = firstFace.section('_', 1, 1).section('.', 0, 0).toInt();
            
            QSqlDatabase db = QSqlDatabase::database();
            QSqlQuery q(db);
            q.prepare("SELECT FIRST_NAME FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
            q.bindValue(":id", employeeId);
            QString empName = "";
            if (q.exec() && q.next()) empName = q.value(0).toString();
            
            m_camera->stop();
            m_isFaceLoginActive = false;

            // Generate "Cinematic Welcome" overlay exactly on the camera preview
            QPixmap welcomePix(ui->lbl_camera_preview->size());
            welcomePix.fill(Qt::transparent);
            QPainter p(&welcomePix);
            p.setRenderHint(QPainter::Antialiasing);
            
            // Draw luxury background circle
            p.setBrush(QColor(20, 15, 10, 240));
            p.setPen(QPen(QColor(212, 175, 55), 4));
            p.drawEllipse(welcomePix.rect().adjusted(2, 2, -2, -2));
            
            // Draw text
            p.setPen(QColor(212, 175, 55));
            QFont f = p.font();
            f.setPointSize(22);
            f.setBold(true);
            f.setLetterSpacing(QFont::AbsoluteSpacing, 2);
            p.setFont(f);
            p.drawText(welcomePix.rect(), Qt::AlignCenter, "WELCOME\n" + empName.toUpper());
            
            ui->lbl_camera_preview->setPixmap(welcomePix);
            
            // Cinematic pulse effect for the welcome
            QGraphicsOpacityEffect *glow = new QGraphicsOpacityEffect(ui->lbl_camera_preview);
            ui->lbl_camera_preview->setGraphicsEffect(glow);
            QPropertyAnimation *pulse = new QPropertyAnimation(glow, "opacity");
            pulse->setDuration(1000);
            pulse->setKeyValueAt(0, 0.0);
            pulse->setKeyValueAt(0.3, 1.0);
            pulse->setKeyValueAt(0.7, 1.0);
            pulse->setKeyValueAt(1.0, 0.0);
            pulse->start(QPropertyAnimation::DeleteWhenStopped);

            // Wait 2 seconds (time of the pulse + extra) then vanish and login
            QTimer::singleShot(2000, this, [this, employeeId]() {
                ui->lbl_camera_preview->hide();
                emit loginSuccessful(employeeId);
            });
        });
    }
}

void LoginWindow::handleForgotPassword() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Hammer Down - Security Protocol"));
    dialog.setFixedSize(500, 360);
    dialog.setStyleSheet("QDialog { background-color: #1A140A; border: 2px solid #D4AF37; border-radius: 12px; } "
                         "QLabel { color: #D4AF37; font-weight: bold; font-family: 'Segoe UI', Arial; } "
                         "QLineEdit { background-color: rgba(0,0,0,0.5); color: #F0E0C0; border: 1px solid #8B6F47; padding: 10px; border-radius: 6px; font-size: 14px; } "
                         "QLineEdit:focus { border-color: #D4AF37; } "
                         "QPushButton { background-color: #8B6F47; color: white; padding: 10px; border-radius: 6px; font-weight: bold; font-size: 14px; border: none; } "
                         "QPushButton:hover { background-color: #A0825A; } "
                         "QPushButton:pressed { background-color: #6D5535; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QStackedWidget *stack = new QStackedWidget(&dialog);
    mainLayout->addWidget(stack);

    // Common Step Header Function
    auto createHeader = [&](const QString &title, const QString &subtitle) {
        QWidget *w = new QWidget();
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        QLabel *tLbl = new QLabel(title, w);
        tLbl->setAlignment(Qt::AlignCenter);
        tLbl->setStyleSheet("font-size: 18px; letter-spacing: 2px; color: #D4AF37; margin-bottom: 5px;");
        l->addWidget(tLbl);
        QLabel *sLbl = new QLabel(subtitle, w);
        sLbl->setAlignment(Qt::AlignCenter);
        sLbl->setStyleSheet("color: #A0A0A0; font-weight: normal; margin-bottom: 20px;");
        l->addWidget(sLbl);
        return qMakePair(w, l);
    };

    // Transition Helper
    auto animateTransition = [&](int index) {
        QWidget *current = stack->currentWidget();
        QWidget *next = stack->widget(index);
        
        QGraphicsOpacityEffect *outEff = new QGraphicsOpacityEffect(current);
        current->setGraphicsEffect(outEff);
        QPropertyAnimation *fadeOut = new QPropertyAnimation(outEff, "opacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);

        connect(fadeOut, &QPropertyAnimation::finished, [stack, index, next]() {
            stack->setCurrentIndex(index);
            QGraphicsOpacityEffect *inEff = new QGraphicsOpacityEffect(next);
            next->setGraphicsEffect(inEff);
            QPropertyAnimation *fadeIn = new QPropertyAnimation(inEff, "opacity");
            fadeIn->setDuration(300);
            fadeIn->setStartValue(0.0);
            fadeIn->setEndValue(1.0);
            fadeIn->start(QPropertyAnimation::DeleteWhenStopped);
        });
        fadeOut->start(QPropertyAnimation::DeleteWhenStopped);
    };

    QString targetId;
    QString targetEmail;

    // --- STEP 1: IDENTITY DISCOVERY ---
    auto step1 = createHeader("IDENTITY DISCOVERY", "Please enter your Employee ID to initiate formula.");
    QLineEdit *idEdit = new QLineEdit(step1.first);
    idEdit->setPlaceholderText("Enter ID (e.g. 1)");
    step1.second->addWidget(idEdit);
    QPushButton *btnNext1 = new QPushButton("Verify Identity", step1.first);
    step1.second->addWidget(btnNext1);
    stack->addWidget(step1.first);

    // --- STEP 2: FORMULA CHALLENGE ---
    auto step2 = createHeader("FORMULA CHALLENGE", "Confirm your identity by entering your registered email.");
    QLineEdit *emailEdit = new QLineEdit(step2.first);
    emailEdit->setPlaceholderText("yourname@company.com");
    step2.second->addWidget(emailEdit);
    QPushButton *btnNext2 = new QPushButton("Authorize Reset", step2.first);
    step2.second->addWidget(btnNext2);
    stack->addWidget(step2.first);

    // --- STEP 3: PROTOCOL RESET ---
    auto step3 = createHeader("PROTOCOL RESET", "Specify your new secure credentials.");
    QLineEdit *pwdEdit = new QLineEdit(step3.first);
    pwdEdit->setEchoMode(QLineEdit::Password);
    pwdEdit->setPlaceholderText("New Password");
    step3.second->addWidget(pwdEdit);
    QLineEdit *confirmEdit = new QLineEdit(step3.first);
    confirmEdit->setEchoMode(QLineEdit::Password);
    confirmEdit->setPlaceholderText("Confirm Password");
    step3.second->addWidget(confirmEdit);
    QPushButton *btnFinish = new QPushButton("Finalize Formula", step3.first);
    step3.second->addWidget(btnFinish);
    stack->addWidget(step3.first);

    // --- LOGIC ---
    connect(btnNext1, &QPushButton::clicked, [&]() {
        targetId = idEdit->text().trimmed();
        if (targetId.isEmpty()) return;
        
        QSqlQuery q;
        q.prepare("SELECT EMAIL FROM EMPLOYEES WHERE EMPLOYEE_ID = :id");
        q.bindValue(":id", targetId);
        if (q.exec() && q.next()) {
            targetEmail = q.value(0).toString();
            animateTransition(1);
        } else {
            QMessageBox::warning(&dialog, "Access Denied", "System Error: Employee ID not found in database.");
        }
    });

    connect(btnNext2, &QPushButton::clicked, [&]() {
        if (emailEdit->text().trimmed().toLower() == targetEmail.trimmed().toLower()) {
            animateTransition(2);
        } else {
            QMessageBox::critical(&dialog, "Verification Failed", "Formula Mismatch: Email does not align with Identity records.");
        }
    });

    connect(btnFinish, &QPushButton::clicked, [&]() {
        if (pwdEdit->text().isEmpty()) return;
        if (pwdEdit->text() != confirmEdit->text()) {
            QMessageBox::warning(&dialog, "Mismatch", "Formula Error: Passwords do not align.");
            return;
        }

        QSqlQuery update;
        update.prepare("UPDATE EMPLOYEES SET PASSWORD = :pass WHERE EMPLOYEE_ID = :id");
        update.bindValue(":pass", pwdEdit->text());
        update.bindValue(":id", targetId);
        if (update.exec()) {
            QSqlDatabase::database().commit();
            QMessageBox::information(&dialog, "Success", "Security Protocol Updated. You may now proceed to Login.");
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "Database Error", "Failed to commit update: " + update.lastError().text());
        }
    });

    dialog.exec();
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
    
    // Add a professional gold border
    painter.setClipping(false);
    painter.setPen(QPen(QColor(139, 111, 71), 4));
    painter.drawEllipse(src.rect().adjusted(2, 2, -2, -2));
    
    // Cinematic Scanning Line Overlay
    int msec = QTime::currentTime().msecsSinceStartOfDay() % 2000;
    float scanPos = (msec < 1000) ? (msec / 1000.0) : (2.0 - (msec / 1000.0));
    int yOffset = src.height() * scanPos;
    
    QLinearGradient scanGrad(0, yOffset - 40, 0, yOffset);
    scanGrad.setColorAt(0, Qt::transparent);
    scanGrad.setColorAt(1, QColor(0, 255, 255, 120)); // Cyan scanning glow
    painter.setBrush(scanGrad);
    painter.setPen(Qt::NoPen);
    
    // Draw the gradient sweeping area (intersected with the circle for perfection)
    QPainterPath circlePath;
    circlePath.addEllipse(src.rect().adjusted(2, 2, -2, -2));
    painter.setClipPath(circlePath);
    painter.drawRect(0, yOffset - 40, src.width(), 40);
    
    // Solid glowing laser line
    painter.setPen(QPen(QColor(0, 255, 255, 220), 3));
    painter.drawLine(0, yOffset, src.width(), yOffset);
    
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
