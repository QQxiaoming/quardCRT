/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"
#include "sessionswindow.h"

namespace sessionprotocol {
class TelnetProtocol final : public SessionProtocolBase {
public:
    SessionsWindow::SessionType type() const override { return SessionsWindow::Telnet; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::ConsoleSession; }
    void initialize(SessionsWindow *session) override {
        telnet = new QTelnet(QTelnet::TCP, session);
        session->realtimespeed_timer = new QTimer(session);

        QObject::connect(telnet, &QTelnet::newData, session, [=](const char *data, int size){
            session->forwardReceivedData(QByteArray(data, size), true);
        });
        session->setupModemProxyForward([=]() {
            return true;
        }, [=](const QByteArray &data) {
            telnet->sendData(data.data(), data.size());
        });
        session->setupTerminalSendForward([=]() {
            return telnet->isConnected();
        }, [=](const char *data, int size) {
            telnet->sendData(data, size);
        }, true);
        QObject::connect(telnet, &QTelnet::stateChanged, session, [=](QAbstractSocket::SocketState socketState){
            if(socketState == QAbstractSocket::ConnectedState) {
                session->updateConnectionState(true);
            } else if(socketState == QAbstractSocket::UnconnectedState) {
                session->updateConnectionState(false);
            }
        });
        QObject::connect(telnet, &QTelnet::error, session, [=](QAbstractSocket::SocketError socketError){
            session->reportSessionError(session->tr("Telnet Error"), session->tr("Telnet error:\n%1.").arg(telnet->errorString()));
            Q_UNUSED(socketError);
        });
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        Q_UNUSED(profile);
        target->startTelnetSession(commonMeta.value("hostname").toString(),
                                   static_cast<quint16>(commonMeta.value("port").toUInt()),
                                   static_cast<QTelnet::SocketType>(protocolMeta.value("socketType", QTelnet::TCP).toInt()));
    }
    void disconnect(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(telnet && telnet->isConnected()) telnet->disconnectFromHost();
    }
    void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) override {
        info.telnet.tx_total = session->tx_total;
        info.telnet.rx_total = session->rx_total;
        info.telnet.tx_speed = session->tx_speed;
        info.telnet.rx_speed = session->rx_speed;
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
        if(telnet) {
            if(telnet->isConnected()) telnet->disconnectFromHost();
            delete telnet;
            telnet = nullptr;
        }
    }
    int startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type) {
        if(!telnet) {
            return -1;
        }
        socketTypeValue = type;
        telnet->setType(type);
        telnet->connectToHost(hostname, port);
        return 0;
    }
    QVariantMap exportMeta() const override {
        return {
            {"socketType", static_cast<int>(socketTypeValue)}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        socketTypeValue = static_cast<QTelnet::SocketType>(meta.value("socketType", QTelnet::TCP).toInt());
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        Q_UNUSED(session);
        return startTelnetSession(commonMeta.value("hostname").toString(),
                                  static_cast<quint16>(commonMeta.value("port").toUInt()),
                                  static_cast<QTelnet::SocketType>(protocolMeta.value("socketType", QTelnet::TCP).toInt()));
    }

private:
    QTelnet *telnet = nullptr;
    QTelnet::SocketType socketTypeValue = QTelnet::TCP;
};

SessionProtocolRegistrar kTelnetProtocolRegistrar(
    SessionsWindow::Telnet,
    []() { return std::make_unique<TelnetProtocol>(); });
}

int SessionsWindow::startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type) {
    if(!protocol) {
        return -1;
    }
    const QVariantMap commonMeta = {
        {"hostname", hostname},
        {"port", port}
    };
    const QVariantMap protocolMeta = {
        {"socketType", static_cast<int>(type)}
    };
    int ret = protocol->startSession(this, commonMeta, protocolMeta);
    if(ret != 0) {
        return ret;
    }
    m_hostname = hostname;
    m_port = port;
    return 0;
}
