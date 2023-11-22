#include "sshtunnelin.h"
#include "sshclient.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QEventLoop>
#include <cerrno>
#include <QTime>
#include <sshtunnelinconnection.h>

Q_LOGGING_CATEGORY(logsshtunnelin, "ssh.tunnelin", QtWarningMsg)

#define BUFFER_LEN (16384)


SshTunnelIn::SshTunnelIn(const QString &name, SshClient *client)
    : SshChannel(name, client)
{

}

SshTunnelIn::~SshTunnelIn()
{
    qCDebug(logsshtunnelin) << m_name << "SshTunnelIn Destroyed";
}

void SshTunnelIn::listen(QString host, quint16 localPort, quint16 remotePort, QString listenHost, int queueSize)
{
    qCDebug(logsshtunnelin) << m_name << "listen(" << remotePort << " -> " << host << ":" << localPort << ")";
    m_localTcpPort = localPort;
    m_remoteTcpPort = remotePort;
    m_queueSize = queueSize;
    m_targethost = host;
    m_listenhost = listenHost;
    m_retryListen = 10;
    setChannelState(ChannelState::Exec);
    sshDataReceived();
}

quint16 SshTunnelIn::localPort()
{
    return static_cast<unsigned short>(m_localTcpPort);
}

quint16 SshTunnelIn::remotePort()
{
    if(m_remoteTcpPort == 0) return static_cast<unsigned short>(m_boundPort);
    return static_cast<unsigned short>(m_remoteTcpPort);
}

void SshTunnelIn::sshDataReceived()
{
    switch(channelState())
    {
        case Openning:
        {
            qCDebug(logsshtunnelin) << "Channel session opened";
            break;
        }

        case Exec:
        {
            do
            {
                if ( ! m_sshClient->takeChannelCreationMutex(this) )
                {
                    qCWarning(logsshtunnelin) << m_name << "mutex busy";
                    return;
                }

                m_sshListener = libssh2_channel_forward_listen_ex(m_sshClient->session(), qPrintable(m_listenhost), m_remoteTcpPort, &m_boundPort, m_queueSize);
                m_sshClient->releaseChannelCreationMutex(this);

                if(m_sshListener == nullptr)
                {
                    char *emsg;
                    int size;
                    int ret = libssh2_session_last_error(m_sshClient->session(), &emsg, &size, 0);
                    if(ret == LIBSSH2_ERROR_EAGAIN)
                    {
                        return;
                    }
                    if ( ret==LIBSSH2_ERROR_REQUEST_DENIED && m_retryListen > 0 )
                    {
                        m_retryListen--;
                        return;
                    }
                    else
                    {
                        if(m_remoteTcpPort == 0)
                        {
                            m_remoteTcpPort = m_localTcpPort;
                            m_retryListen = 5;
                            qCWarning(logsshtunnelin) << "The server refuse dynamic port, try with the same port as local";
                            return;
                        }
                    }

                    setChannelState(ChannelState::Error);
                    qCWarning(logsshtunnelin) << "Channel session open failed: " << emsg;
                    return;
                }
                qCDebug(logsshtunnelin) << m_name << "Create Reverse tunnel for " << m_listenhost << m_remoteTcpPort << m_boundPort << m_queueSize;
            } while (m_sshListener == nullptr);
            /* OK, next step */
            setChannelState(ChannelState::Ready);
        }

        FALLTHROUGH; case Ready:
        {
            LIBSSH2_CHANNEL *newChannel;
            if ( ! m_sshClient->takeChannelCreationMutex(this) )
            {
                return;
            }
            newChannel = libssh2_channel_forward_accept(m_sshListener);
            m_sshClient->releaseChannelCreationMutex(this);

            if(newChannel == nullptr)
            {
                char *emsg;
                int size;
                int ret = libssh2_session_last_error(m_sshClient->session(), &emsg, &size, 0);
                if(ret == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }

                qCWarning(logsshtunnelin) << "Channel session open failed: " << emsg;
                return;
            }

            /* We have a new connection on the remote port, need to create a connection tunnel */
            qCDebug(logsshtunnelin) << "SshTunnelIn new connection";
            SshTunnelInConnection *connection = m_sshClient->getChannel<SshTunnelInConnection>(m_name + QString("_%1").arg(m_connectionCounter++));
            connection->configure(newChannel, m_localTcpPort, m_targethost);
            m_connection.append(connection);
            QObject::connect(connection, &SshTunnelInConnection::stateChanged, this, &SshTunnelIn::connectionStateChanged);
            emit connectionChanged(m_connection.size());
            break;
        }

        case Close:
        {
            if(m_sshListener)
            {
                if(libssh2_channel_forward_cancel(m_sshListener) == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                m_sshListener = nullptr;
            }
            setChannelState(ChannelState::WaitClose);
            FALLTHROUGH;
        }

        case WaitClose:
        {
            qCDebug(logsshtunnelin) << "Wait close channel:" << m_name << " (connections:"<< m_connection.count() << ")";
            if(m_connection.count() != 0)
            {
                return;
            }
            setChannelState(ChannelState::Freeing);
            FALLTHROUGH;
        }

        case Freeing:
        {
            qCDebug(logsshtunnelin) << "free Channel";
            setChannelState(ChannelState::Free);
        }

        FALLTHROUGH; case Free:
        {
            qCDebug(logsshtunnelin) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            qCDebug(logsshtunnelin) << "Channel" << m_name << "is in error state";
            return;
        }
    }
}

void SshTunnelIn::connectionStateChanged()
{
    QObject *obj = QObject::sender();
    SshTunnelInConnection *connection = qobject_cast<SshTunnelInConnection*>(obj);
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

void SshTunnelIn::flushTx() const
{
    for(auto &c: m_connection)
    {
        c->flushTx();
    }
}

void SshTunnelIn::close()
{
    setChannelState(ChannelState::Close);
    sshDataReceived();
}
