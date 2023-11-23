#include "sshtunnelout.h"
#include "sshclient.h"
#include <QDateTime>

Q_LOGGING_CATEGORY(logsshtunnelout, "ssh.tunnelout", QtWarningMsg)


SshTunnelOut::SshTunnelOut(const QString &name, SshClient *client)
    : SshChannel(name, client)
{
    QObject::connect(&m_tcpserver, &QTcpServer::newConnection, this, &SshTunnelOut::_createConnection);
    sshDataReceived();
}

SshTunnelOut::~SshTunnelOut()
{
    qCDebug(logsshtunnelout) << "delete SshTunnelOut:" << m_name;
}

void SshTunnelOut::close()
{
    qCDebug(logsshtunnelout) << m_name << "Ask to close";
    setChannelState(ChannelState::Close);
    sshDataReceived();
}

void SshTunnelOut::listen(quint16 port, QString hostTarget, QString hostListen)
{
    m_port = port;
    m_hostTarget = hostTarget;
    m_tcpserver.listen(QHostAddress(hostListen), 0);
    setChannelState(ChannelState::Ready);
}

void SshTunnelOut::sshDataReceived()
{
    switch(channelState())
    {
        case Openning:
        {
            qCDebug(logsshtunnelout) << "Channel session opened";
            setChannelState(ChannelState::Exec);
        }

        FALLTHROUGH; case Exec:
        {
            /* OK, next step */
            setChannelState(ChannelState::Ready);
            FALLTHROUGH;
        }

        case Ready:
        {
            // Nothing to do...
            return;
        }

        case Close:
        {
            qCDebug(logsshtunnelout) << m_name << "Close server";
            m_tcpserver.close();
            setChannelState(ChannelState::WaitClose);
        }

        FALLTHROUGH; case WaitClose:
        {
            qCDebug(logsshtunnelout) << "Wait close channel:" << m_name << " (connections:"<< m_connection.count() << ")";
            if(m_connection.count() == 0)
            {
                setChannelState(ChannelState::Freeing);
            }
            else
            {
                break;
            }
        }

        FALLTHROUGH; case Freeing:
        {
            qCDebug(logsshtunnelout) << "free Channel:" << m_name;
            setChannelState(ChannelState::Free);

            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshTunnelOut::sshDataReceived);
            return;
        }

        case Free:
        {
            qCDebug(logsshtunnelout) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            qCDebug(logsshtunnelout) << "Channel" << m_name << "is in error state";
            return;
        }
    }
}

int SshTunnelOut::connections()
{
    return m_connection.count();
}

void SshTunnelOut::closeAllConnections()
{
    for(SshTunnelOutConnection *connection : m_connection)
    {
        connection->close();
    }
}

void SshTunnelOut::connectionStateChanged()
{
    QObject *obj = QObject::sender();
    SshTunnelOutConnection *connection = qobject_cast<SshTunnelOutConnection*>(obj);
    if(connection)
    {
        if(connection->channelState() == SshChannel::ChannelState::Free)
        {
            m_connection.removeAll(connection);
            emit connectionChanged(m_connection.count());

            if(m_connection.count() == 0 && channelState() == SshChannel::ChannelState::WaitClose)
            {
                setChannelState(SshChannel::ChannelState::Freeing);
            }
        }
    }
}

void SshTunnelOut::flushTx() const
{
    for(auto &c: m_connection)
    {
        c->flushTx();
    }
}

quint16 SshTunnelOut::port() const
{
    return m_port;
}

void SshTunnelOut::_createConnection()
{
    qCDebug(logsshtunnelout) << "SshTunnelOut new connection";
    SshTunnelOutConnection *connection = m_sshClient->getChannel<SshTunnelOutConnection>(m_name + QString("_%1").arg(m_connectionCounter++));
    connection->configure(&m_tcpserver, m_port, m_hostTarget);
    m_connection.append(connection);
    QObject::connect(connection, &SshTunnelOutConnection::stateChanged, this, &SshTunnelOut::connectionStateChanged);
    emit connectionChanged(m_connection.count());
}

quint16 SshTunnelOut::localPort()
{
    return m_tcpserver.serverPort();
}
