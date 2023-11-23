#include "sshtunneloutconnection.h"
#include "sshtunnelout.h"
#include "sshclient.h"

Q_LOGGING_CATEGORY(logsshtunneloutconnection, "ssh.tunnelout.connection")
Q_LOGGING_CATEGORY(logsshtunneloutconnectiontransfer, "ssh.tunnelout.connection.transfer")

#define DEBUGCH qCDebug(logsshtunneloutconnection) << m_name
#define DEBUGTX qCDebug(logsshtunneloutconnectiontransfer) << m_name

#define SOCKET_WRITE_ERROR (-1001)

SshTunnelOutConnection::SshTunnelOutConnection(const QString &name, SshClient *client)
    : SshChannel(name, client)
    , m_connector(client, name)
{
    QObject::connect(this, &SshTunnelOutConnection::sendEvent, this, &SshTunnelOutConnection::_eventLoop, Qt::QueuedConnection);
    QObject::connect(&m_connector, &SshTunnelDataConnector::sendEvent, this, &SshTunnelOutConnection::sendEvent);
    DEBUGCH << "Create SshTunnelOutConnection (constructor)";
    emit sendEvent();
}

void SshTunnelOutConnection::configure(QTcpServer *server, quint16 remotePort, QString target)
{
    m_server = server;
    m_port = remotePort;
    m_target = target;
}

SshTunnelOutConnection::~SshTunnelOutConnection()
{
    DEBUGCH << "Free SshTunnelOutConnection (destructor)";
    delete m_sock;
}

void SshTunnelOutConnection::close()
{
    DEBUGCH << "Close SshTunnelOutConnection asked";
    if(channelState() != ChannelState::Error)
    {
        setChannelState(ChannelState::Close);
    }
    emit sendEvent();
}


void SshTunnelOutConnection::sshDataReceived()
{
    m_connector.sshDataReceived();
    emit sendEvent();
}

void SshTunnelOutConnection::flushTx()
{
    m_connector.flushTx();
}

void SshTunnelOutConnection::_eventLoop()
{
    switch(channelState())
    {
        case Openning:
        {
            if ( ! m_sshClient->takeChannelCreationMutex(this) )
            {
                return;
            }
            m_sshChannel = libssh2_channel_direct_tcpip(m_sshClient->session(), qPrintable(m_target), m_port);
            m_sshClient->releaseChannelCreationMutex(this);
            if (m_sshChannel == nullptr)
            {
                char *emsg;
                int size;
                int ret = libssh2_session_last_error(m_sshClient->session(), &emsg, &size, 0);
                if(ret == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                if(!m_error)
                {
                    qCDebug(logsshtunneloutconnection) << "Refuse client socket connection on " << m_server->serverPort() << QString(emsg);
                    m_error = true;
                    m_sock = m_server->nextPendingConnection();
                    if(m_sock)
                    {
                        m_sock->close();
                    }
                    m_server->close();
                }
                setChannelState(ChannelState::Error);
                qCWarning(logsshtunneloutconnection) << "Channel session open failed";
                return;
            }
            DEBUGCH << "Channel session opened";
            setChannelState(ChannelState::Exec);
        }

        FALLTHROUGH; case Exec:
        {
            m_sock = m_server->nextPendingConnection();
            if(!m_sock)
            {
                m_server->close();
                setChannelState(ChannelState::Error);
                qCWarning(logsshtunneloutconnection) << "Fail to get client socket";
                setChannelState(ChannelState::Close);
                return;
            }

            QObject::connect(m_sock, &QObject::destroyed, [this](){ DEBUGCH << "Client Socket destroyed";});
            m_name = QString(m_name + ":%1").arg(m_sock->localPort());
            DEBUGCH << "createConnection: " << m_sock << m_sock->localPort();
            m_connector.setChannel(m_sshChannel);
            m_connector.setSock(m_sock);
            setChannelState(ChannelState::Ready);
            /* OK, next step */
        }

        FALLTHROUGH; case Ready:
        {
            if(!m_connector.process())
            {
                setChannelState(ChannelState::Close);
            }
            return;
        }

        case Close:
        {
            DEBUGCH << "closeChannel";
            m_connector.close();
            setChannelState(ChannelState::WaitClose);
        }

        FALLTHROUGH; case WaitClose:
        {
            DEBUGCH << "Wait close channel";
            if(m_connector.isClosed())
            {
                setChannelState(ChannelState::Freeing);
            }
            else
            {
                m_connector.process();
                return;
            }
        }

        FALLTHROUGH; case Freeing:
        {
            DEBUGCH << "free Channel";

            int ret = libssh2_channel_free(m_sshChannel);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret < 0)
            {
                if(!m_error)
                {
                    m_error = true;
                    qCWarning(logsshtunneloutconnection) << "Failed to free channel: " << sshErrorToString(ret);
                }
            }

            if(m_error)
            {
                setChannelState(ChannelState::Error);
            }
            else
            {
                setChannelState(ChannelState::Free);
            }
            m_sshChannel = nullptr;
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshTunnelOutConnection::sshDataReceived);
            return;
        }

        case Free:
        {
            qCDebug(logsshtunneloutconnection) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            qCDebug(logsshtunneloutconnection) << "Channel" << m_name << "is in error state";
            setChannelState(Free);
            return;
        }
    }
}

