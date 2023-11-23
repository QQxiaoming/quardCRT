#pragma once

#include <QObject>
#include <QTcpServer>
#include <QLoggingCategory>
#include "sshchannel.h"
#include "sshtunneldataconnector.h"

Q_DECLARE_LOGGING_CATEGORY(logsshtunneloutconnection)
Q_DECLARE_LOGGING_CATEGORY(logsshtunneloutconnectiontransfer)

class SshTunnelOutConnection : public SshChannel
{
    Q_OBJECT
protected:
    explicit SshTunnelOutConnection(const QString &name, SshClient *client);
    friend class SshClient;

public:
    void configure(QTcpServer *server, quint16 remotePort, QString target = "127.0.0.1");
    virtual ~SshTunnelOutConnection() override;
    void close() override;

private:
    SshTunnelDataConnector m_connector;
    LIBSSH2_CHANNEL *m_sshChannel {nullptr};
    QTcpSocket *m_sock;
    QTcpServer *m_server;
    quint16 m_port;
    QString m_target;
    bool m_error {false};

private slots:
    void _eventLoop();


public slots:
    void sshDataReceived() override;
    void flushTx();

signals:
    void sendEvent();
};
