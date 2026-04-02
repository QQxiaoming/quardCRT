/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"
#include "sessionswindow.h"

namespace sessionprotocol {
class NamePipeProtocol final : public SessionProtocolBase {
public:
    struct StartArgs {
        QString pipeName;
    };

    SessionsWindow::SessionType type() const override { return SessionsWindow::NamePipe; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::ConsoleSession; }
    void initialize(SessionsWindow *session) override {
        namePipe = new QLocalSocket(session);

        QObject::connect(namePipe, &QLocalSocket::readyRead, session, [=](){
            session->forwardReceivedData(namePipe->readAll(), false);
        });
        session->setupModemProxyForward([=]() {
            return namePipe->state() == QLocalSocket::ConnectedState;
        }, [=](const QByteArray &data) {
            namePipe->write(data.data(), data.size());
        });
        session->setupTerminalSendForward([=]() {
            return namePipe->state() == QLocalSocket::ConnectedState;
        }, [=](const char *data, int size) {
            namePipe->write(data, size);
        }, false);
        QObject::connect(namePipe, &QLocalSocket::stateChanged, session, [=](QLocalSocket::LocalSocketState socketState){
            if(socketState == QLocalSocket::ConnectedState) {
                session->updateConnectionState(true);
            } else if(socketState == QLocalSocket::UnconnectedState) {
                session->updateConnectionState(false);
            }
        });
        QObject::connect(namePipe, &QLocalSocket::errorOccurred, session, [=](QLocalSocket::LocalSocketError socketError){
            session->reportSessionError(session->tr("Name Pipe Error"), session->tr("Name Pipe error:\n%1.").arg(namePipe->errorString()));
            Q_UNUSED(socketError);
        });
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        Q_UNUSED(profile);
        Q_UNUSED(commonMeta);
        const StartArgs args = parseStartArgs(protocolMeta);
        target->startNamePipeSession(args.pipeName);
    }
    void disconnect(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(namePipe && namePipe->state() == QLocalSocket::ConnectedState) namePipe->disconnectFromServer();
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
    void cleanup(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(namePipe) {
            if(namePipe->state() == QLocalSocket::ConnectedState) namePipe->disconnectFromServer();
            delete namePipe;
            namePipe = nullptr;
        }
    }
    int startNamePipeSession(const QString &name) {
        if(!namePipe) {
            return -1;
        }
        pipeNameValue = name;
        namePipe->connectToServer(name);
        return 0;
    }
    QVariantMap exportMeta() const override {
        return {
            {"pipeName", pipeNameValue}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        pipeNameValue = meta.value("pipeName").toString();
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        Q_UNUSED(session);
        Q_UNUSED(commonMeta);
        const StartArgs args = parseStartArgs(protocolMeta);
        return startNamePipeSession(args.pipeName);
    }

private:
    static StartArgs parseStartArgs(const QVariantMap &protocolMeta) {
        StartArgs args;
        args.pipeName = protocolMeta.value("pipeName").toString();
        return args;
    }

    QLocalSocket *namePipe = nullptr;
    QString pipeNameValue;
};

SessionProtocolRegistrar kNamePipeProtocolRegistrar(
    SessionsWindow::NamePipe,
    []() { return std::make_unique<NamePipeProtocol>(); });
}

int SessionsWindow::startNamePipeSession(const QString &name) {
    if(!protocol) {
        return -1;
    }
    const QVariantMap protocolMeta = {
        {"pipeName", name}
    };
    int ret = protocol->startSession(this, QVariantMap(), protocolMeta);
    if(ret != 0) {
        return ret;
    }
    return 0;
}
