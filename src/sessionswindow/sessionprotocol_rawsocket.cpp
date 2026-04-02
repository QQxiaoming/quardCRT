/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"
#include "sessionswindow.h"
#include "sessionprotocol_rawsocket.h"

namespace sessionprotocol {
class RawSocketProtocol final : public SessionProtocolBase {
public:
    SessionsWindow::SessionType type() const override { return SessionsWindow::RawSocket; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::ConsoleSession; }
    void initialize(SessionsWindow *session) override {
        rawSocket = new QRawSocket(session);
        session->realtimespeed_timer = new QTimer(session);

        QObject::connect(rawSocket, &QRawSocket::readyRead, session, [=](){
            session->forwardReceivedData(rawSocket->readAll(), true);
        });
        session->setupModemProxyForward([=]() {
            return rawSocket->state() == QAbstractSocket::ConnectedState;
        }, [=](const QByteArray &data) {
            rawSocket->write(data.data(), data.size());
        });
        session->setupTerminalSendForward([=]() {
            return rawSocket->state() == QAbstractSocket::ConnectedState;
        }, [=](const char *data, int size) {
            rawSocket->write(data, size);
        }, true);
        QObject::connect(rawSocket, &QRawSocket::stateChanged, session, [=](QAbstractSocket::SocketState socketState){
            if(socketState == QAbstractSocket::ConnectedState) {
                session->updateConnectionState(true);
            } else if(socketState == QAbstractSocket::UnconnectedState) {
                session->updateConnectionState(false);
            }
        });
        QObject::connect(rawSocket, &QRawSocket::errorOccurred, session, [=](QAbstractSocket::SocketError socketError){
            session->reportSessionError(session->tr("Raw Socket Error"), session->tr("Raw Socket error:\n%1.").arg(rawSocket->errorString()));
            Q_UNUSED(socketError);
        });
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        Q_UNUSED(profile);
        target->startRawSocketSession(commonMeta.value("hostname").toString(),
                                      static_cast<quint16>(commonMeta.value("port").toUInt()),
                                      protocolMeta.value("rawMode", 0).toInt());
    }
    void disconnect(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(rawSocket && rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
    }
    void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) override {
        info.rawSocket.tx_total = session->tx_total;
        info.rawSocket.rx_total = session->rx_total;
        info.rawSocket.tx_speed = session->tx_speed;
        info.rawSocket.rx_speed = session->rx_speed;
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
        if(rawSocket) {
            if(rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
            delete rawSocket;
            rawSocket = nullptr;
        }
    }    
    int getRawMode() const override {
        return rawSocket->getRawMode();
    }
    QVariantMap exportMeta() const override {
        return {
            {"rawMode", getRawMode()}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        Q_UNUSED(meta);
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        Q_UNUSED(session);
        return startRawSocketSession(commonMeta.value("hostname").toString(),
                                     static_cast<quint16>(commonMeta.value("port").toUInt()),
                                     protocolMeta.value("rawMode", 0).toInt());
    }
    int startRawSocketSession(const QString &hostname, quint16 port, int mode) {
        if(!rawSocket) {
            return -1;
        }
        rawSocket->setRawMode(mode);
        rawSocket->connectToHost(hostname, port);
        return 0;
    }

private:
    QRawSocket *rawSocket = nullptr;
};

SessionProtocolRegistrar kRawSocketProtocolRegistrar(
    SessionsWindow::RawSocket,
    []() { return std::make_unique<RawSocketProtocol>(); });
}

int SessionsWindow::startRawSocketSession(const QString &hostname, quint16 port, int mode) {
    if(!protocol) {
        return -1;
    }
    const QVariantMap commonMeta = {
        {"hostname", hostname},
        {"port", port}
    };
    const QVariantMap protocolMeta = {
        {"rawMode", mode}
    };
    int ret = protocol->startSession(this, commonMeta, protocolMeta);
    if(ret != 0) {
        return ret;
    }
    m_hostname = hostname;
    m_port = port;
    return 0;
}
