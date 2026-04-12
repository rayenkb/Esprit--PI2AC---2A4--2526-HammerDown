#include "smtpsender.h"

#include <QSslSocket>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QByteArray>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Read a complete SMTP response (may be multi-line, e.g. "250-..." then "250 ...").
// Returns the raw bytes received.
static QByteArray readSmtpResponse(QSslSocket *socket)
{
    QByteArray data;
    while (true) {
        if (!socket->waitForReadyRead(10000))
            break;
        data += socket->readAll();
        // A final SMTP response line is:  NNN<SP>text\r\n  (3 digits then space)
        // Multi-line continuation uses:   NNN-text\r\n
        const QList<QByteArray> lines = data.split('\n');
        for (const QByteArray &line : lines) {
            const QByteArray trimmed = line.trimmed();
            if (trimmed.size() >= 4 && (trimmed[3] == ' ' || trimmed[3] == '\t'))
                return data;
        }
    }
    return data;
}

static int smtpCode(const QByteArray &response)
{
    if (response.size() < 3) return 0;
    bool ok = false;
    const int code = response.left(3).toInt(&ok);
    return ok ? code : 0;
}

// Send a command and read the response. Returns false if the actual code
// does not match expectedCode.
static bool smtpCmd(QSslSocket *socket, const QByteArray &cmd,
                    QByteArray &response, int expectedCode)
{
    socket->write(cmd + "\r\n");
    socket->flush();
    response = readSmtpResponse(socket);
    return smtpCode(response) == expectedCode;
}

// ---------------------------------------------------------------------------
// Build a minimal MIME message (plain-text, with optional file attachment)
// ---------------------------------------------------------------------------
static QByteArray buildMimeMessage(const QString &from, const QString &to,
                                   const QString &subject, const QString &body,
                                   const QString &attachmentPath)
{
    QByteArray msg;
    const bool hasAttachment = !attachmentPath.isEmpty() && QFile::exists(attachmentPath);

    if (hasAttachment) {
        // multipart/mixed
        const QByteArray boundary =
            "----=_Part_" + QUuid::createUuid().toString(QUuid::WithoutBraces)
                                .remove('-').toUtf8();

        QString brandedFrom = "rayenkabar780@gmail.com";
        QString fancyFrom = "\"HammerDownAssociation\" <" + brandedFrom + ">";
        msg += "From: " + fancyFrom.toUtf8() + "\r\n";
        msg += "To: " + to.toUtf8() + "\r\n";
        msg += "Subject: " + subject.toUtf8() + "\r\n";
        msg += "MIME-Version: 1.0\r\n";
        msg += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n";
        msg += "\r\n";

        // --- text/html part ---
        msg += "--" + boundary + "\r\n";
        bool isHtml = body.contains("<html", Qt::CaseInsensitive) || body.contains("<body", Qt::CaseInsensitive);
        msg += "Content-Type: " + QByteArray(isHtml ? "text/html" : "text/plain") + "; charset=UTF-8\r\n";
        msg += "Content-Transfer-Encoding: 8bit\r\n";
        msg += "\r\n";
        msg += body.toUtf8() + "\r\n";

        // --- attachment part ---
        QFile file(attachmentPath);
        const QFileInfo fi(attachmentPath);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray encoded = file.readAll().toBase64();
            file.close();

            msg += "--" + boundary + "\r\n";
            msg += "Content-Type: application/octet-stream; name=\""
                   + fi.fileName().toUtf8() + "\"\r\n";
            msg += "Content-Transfer-Encoding: base64\r\n";
            msg += "Content-Disposition: attachment; filename=\""
                   + fi.fileName().toUtf8() + "\"\r\n";
            msg += "\r\n";
            // Wrap base64 at 76 chars per RFC 2045
            for (int i = 0; i < encoded.size(); i += 76)
                msg += encoded.mid(i, 76) + "\r\n";
        }
        msg += "--" + boundary + "--\r\n";
    } else {
        // simple text or html
        QString brandedFrom = "rayenkabar780@gmail.com";
        QString fancyFrom = "\"HammerDownAssociation\" <" + brandedFrom + ">";
        bool isHtml = body.contains("<html", Qt::CaseInsensitive) || body.contains("<body", Qt::CaseInsensitive);
        
        msg += "From: " + fancyFrom.toUtf8() + "\r\n";
        msg += "To: " + to.toUtf8() + "\r\n";
        msg += "Subject: " + subject.toUtf8() + "\r\n";
        msg += "MIME-Version: 1.0\r\n";
        msg += "Content-Type: " + QByteArray(isHtml ? "text/html" : "text/plain") + "; charset=UTF-8\r\n";
        msg += "Content-Transfer-Encoding: 8bit\r\n";
        msg += "\r\n";
        msg += body.toUtf8() + "\r\n";
    }

    return msg;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
SmtpResult SmtpSender::send(
    const QString &host, quint16 port,
    const QString &username, const QString &password,
    const QString &to,
    const QString &subject,
    const QString &body,
    const QString &attachmentPath)
{
    QSslSocket socket;
    socket.setProtocol(QSsl::TlsV1_2OrLater);

    // 1. Open plain TCP connection (STARTTLS — not direct SSL)
    socket.connectToHost(host, port);
    if (!socket.waitForConnected(10000))
        return {false, "Connection failed: " + socket.errorString()};

    // 2. Server greeting  220
    QByteArray resp = readSmtpResponse(&socket);
    if (smtpCode(resp) != 220)
        return {false, "Unexpected greeting: " + resp};

    // 3. EHLO
    if (!smtpCmd(&socket, "EHLO localhost", resp, 250))
        return {false, "EHLO failed: " + resp};

    // 4. STARTTLS
    if (!smtpCmd(&socket, "STARTTLS", resp, 220))
        return {false, "STARTTLS failed: " + resp};

    // 5. Upgrade to TLS
    socket.startClientEncryption();
    if (!socket.waitForEncrypted(10000))
        return {false, "TLS upgrade failed: " + socket.errorString()};

    // 6. EHLO again over TLS
    if (!smtpCmd(&socket, "EHLO localhost", resp, 250))
        return {false, "EHLO (TLS) failed: " + resp};

    // 7. AUTH LOGIN
    if (!smtpCmd(&socket, "AUTH LOGIN", resp, 334))
        return {false, "AUTH LOGIN failed: " + resp};

    if (!smtpCmd(&socket, username.toUtf8().toBase64(), resp, 334))
        return {false, "Username rejected: " + resp};

    if (!smtpCmd(&socket, password.toUtf8().toBase64(), resp, 235))
        return {false, "Authentication failed: " + resp};

    // 8. Envelope
    if (!smtpCmd(&socket, "MAIL FROM:<" + username.toUtf8() + ">", resp, 250))
        return {false, "MAIL FROM failed: " + resp};

    if (!smtpCmd(&socket, "RCPT TO:<" + to.toUtf8() + ">", resp, 250))
        return {false, "RCPT TO failed: " + resp};

    // 9. DATA
    if (!smtpCmd(&socket, "DATA", resp, 354))
        return {false, "DATA command failed: " + resp};

    // 10. Send MIME message, terminated by <CRLF>.<CRLF>
    QByteArray mime = buildMimeMessage(username, to, subject, body, attachmentPath);
    mime += "\r\n.\r\n";
    socket.write(mime);
    socket.flush();

    resp = readSmtpResponse(&socket);
    if (smtpCode(resp) != 250)
        return {false, "Message rejected: " + resp};

    // 11. QUIT
    socket.write("QUIT\r\n");
    socket.flush();
    socket.waitForReadyRead(5000);
    socket.disconnectFromHost();

    return {true, {}};
}
