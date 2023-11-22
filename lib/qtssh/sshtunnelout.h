#pragma once

#include <QObject>
#include "sshchannel.h"
#include "sshtunneloutconnection.h"
#include <QTcpServer>

Q_DECLARE_LOGGING_CATEGORY(logsshtunnelout)

class SshTunnelOut : public SshChannel
{
    Q_OBJECT

protected:
    explicit SshTunnelOut(const QString &name, SshClient * client);
    friend class SshClient;

public:
    virtual ~SshTunnelOut() override;
    void close() override;
    quint16 localPort();
    quint16 port() const;

public slots:
    void listen(quint16 port, QString hostTarget = "127.0.0.1", QString hostListen = "127.0.0.1");
    void sshDataReceived() override;
    int connections();
    void closeAllConnections();
    void connectionStateChanged();
    void flushTx() const;

private:
    QTcpServer              m_tcpserver;
    quint16                 m_port {0};
    int                     m_connectionCounter {0};
    QString                 m_hostTarget;
    QList<SshTunnelOutConnection*> m_connection;


private slots:
    void _createConnection();

signals:
    void connectionChanged(int);
};
