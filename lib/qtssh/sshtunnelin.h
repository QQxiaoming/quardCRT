#pragma once

#include "sshchannel.h"
#include <QAbstractSocket>
#include <QLoggingCategory>

class SshTunnelInConnection;

Q_DECLARE_LOGGING_CATEGORY(logsshtunnelin)

class SshTunnelIn : public SshChannel
{
    Q_OBJECT

private:
    quint16 m_localTcpPort {0};
    quint16 m_remoteTcpPort {0};
    int m_boundPort {0};
    int m_queueSize {16};
    int m_retryListen {10};
    QString m_targethost;
    QString m_listenhost;
    LIBSSH2_LISTENER *m_sshListener {nullptr};
    int  m_connectionCounter {0};
    QList<SshTunnelInConnection*> m_connection;

protected:
    explicit SshTunnelIn(const QString &name, SshClient *client);
    friend class SshClient;

public:
    virtual ~SshTunnelIn() override;
    void listen(QString host, quint16 localPort, quint16 remotePort, QString listenHost = "127.0.0.1", int queueSize = 16);
    void close() override;
    quint16 localPort();
    quint16 remotePort();

public slots:
    void sshDataReceived() override;
    void connectionStateChanged();
    void flushTx() const;

signals:
    void connectionChanged(int);
};
