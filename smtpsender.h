#pragma once
#include <QString>

struct SmtpResult {
    bool success;
    QString errorMessage;
};

class SmtpSender {
public:
    // Sends an email via SMTP with STARTTLS (port 587).
    // attachmentPath is optional — pass empty string to skip.
    static SmtpResult send(
        const QString &host, quint16 port,
        const QString &username, const QString &password,
        const QString &to,
        const QString &subject,
        const QString &body,
        const QString &attachmentPath = QString()
    );
};
