/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"
#include "sessionswindow.h"
#include "qvncclientwidget.h"

namespace sessionprotocol {
class VNCProtocol final : public SessionProtocolBase {
public:
    struct StartArgs {
        QString hostname;
        quint16 port = 0;
        QString password;
    };

    SessionsWindow::SessionType type() const override { return SessionsWindow::VNC; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::DesktopSession; }
    void initialize(SessionsWindow *session) override {
        if(vncClient != nullptr) {
            return;
        }
        vncClient = new QVNCClientWidget(session->messageParentWidget);
        vncClient->setProperty("session", QVariant::fromValue(session));
        vncClient->setFullScreen(false);
        vncClient->setMouseHide(false);
        QObject::connect(vncClient, &QVNCClientWidget::connected, session, [=](bool connected){
            session->updateConnectionState(connected);
        });
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        Q_UNUSED(profile);
        const StartArgs args = parseStartArgs(commonMeta, protocolMeta);
        target->startVNCSession(args.hostname, args.port, args.password);
    }
    void disconnect(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(vncClient) {
            vncClient->disconnectFromVncServer();
        }
    }
    void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) override {
        Q_UNUSED(session);
        Q_UNUSED(info);
    }
    bool hasChildProcess(SessionsWindow *session) override {
        Q_UNUSED(session);
        return false;
    }
    void refreshTermSize(SessionsWindow *session) override {
        Q_UNUSED(session);
    }
    QWidget *mainWidget(SessionsWindow *session) override {
        Q_UNUSED(session);
        return static_cast<QWidget *>(vncClient);
    }
    void cleanup(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(vncClient) {
            vncClient->disconnectFromVncServer();
            delete vncClient;
            vncClient = nullptr;
        }
    }
    void screenShot(SessionsWindow *session, const QString &fileName) override {
        Q_UNUSED(session);
        if(vncClient) {
            vncClient->screenShot(fileName);
        }
    }
    void screenShot(SessionsWindow *session, QPixmap *pixmap) override {
        Q_UNUSED(session);
        if(vncClient) {
            vncClient->screenShot(pixmap);
        }
    }
    int startVNCSession(SessionsWindow *session, const QString &hostname, quint16 port, const QString &password) {
        Q_UNUSED(session);
        if(!vncClient) {
            return -1;
        }
        passwordValue = password;
        vncClient->connectToVncServer(hostname, password, port);
        return 0;
    }
    QVariantMap exportMeta() const override {
        return {
            {"password", passwordValue}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        passwordValue = meta.value("password").toString();
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        const StartArgs args = parseStartArgs(commonMeta, protocolMeta);
        return startVNCSession(session, args.hostname, args.port, args.password);
    }

private:
    static StartArgs parseStartArgs(const QVariantMap &commonMeta, const QVariantMap &protocolMeta) {
        StartArgs args;
        args.hostname = commonMeta.value("hostname").toString();
        args.port = static_cast<quint16>(commonMeta.value("port").toUInt());
        args.password = protocolMeta.value("password").toString();
        return args;
    }

    QVNCClientWidget *vncClient = nullptr;
    QString passwordValue;
};

SessionProtocolRegistrar kVNCProtocolRegistrar(
    SessionsWindow::VNC,
    []() { return std::make_unique<VNCProtocol>(); });
}

int SessionsWindow::startVNCSession(const QString &hostname, quint16 port, const QString &password) {
    if(!protocol) {
        return -1;
    }
    const QVariantMap commonMeta = {
        {"hostname", hostname},
        {"port", port}
    };
    const QVariantMap protocolMeta = {
        {"password", password}
    };
    int ret = protocol->startSession(this, commonMeta, protocolMeta);
    if(ret != 0) {
        return ret;
    }
    m_hostname = hostname;
    m_port = port;
    return 0;
}
