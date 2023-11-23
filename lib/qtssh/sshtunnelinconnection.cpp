#include "sshtunnelinconnection.h"
#include "sshclient.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QEventLoop>
#include <cerrno>
#include <QTime>

Q_LOGGING_CATEGORY(logsshtunnelinconnection, "ssh.tunnelin.connection", QtWarningMsg)

#define BUFFER_LEN (16384)
#define DEBUGCH qCDebug(logsshtunnelinconnection) << m_name




SshTunnelInConnection::SshTunnelInConnection(const QString &name, SshClient *client)
    : SshChannel(name, client)
    , m_connector(client, name)
{
    QObject::connect(&m_sock, &QTcpSocket::connected, this, &SshTunnelInConnection::_socketConnected);
    QObject::connect(this, &SshTunnelInConnection::sendEvent, this, &SshTunnelInConnection::_eventLoop, Qt::QueuedConnection);
    QObject::connect(&m_connector, &SshTunnelDataConnector::sendEvent, this, &SshTunnelInConnection::sendEvent);
}

void SshTunnelInConnection::configure(LIBSSH2_CHANNEL *channel, quint16 port, QString hostname)
{
    DEBUGCH << "configure: " << hostname << ":" << port;
    m_sshChannel = channel;
    m_port = port;
    m_hostname = hostname;
    _eventLoop();
}

SshTunnelInConnection::~SshTunnelInConnection()
{
    DEBUGCH << "SshTunnelInConnection Destroyed";
}

void SshTunnelInConnection::close()
{
}

void SshTunnelInConnection::_eventLoop()
{
    switch(channelState())
    {
        case Openning:
        {
            DEBUGCH << "Channel session opened:" << m_hostname << ":" << m_port;
            m_sock.connectToHost(m_hostname, m_port);
            setChannelState(SshChannel::Exec);
            return;
        }

        case Exec:
        {
            // Wait connected, state change by _socketConnected
            return;
        }
        case Ready:
        {
            if(!m_connector.process())
            {
                setChannelState(ChannelState::Close);
            }
            return;
        }

        case Close:
        {
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
        }

        FALLTHROUGH; case Freeing:
        {
            DEBUGCH << "Freeing Channel";

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
                    qCWarning(logsshtunnelinconnection) << "Failed to free channel: " << sshErrorToString(ret);
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
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshTunnelInConnection::sshDataReceived);
            return;
        }

        case Free:
        {
            qCDebug(logsshtunnelinconnection) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            qCDebug(logsshtunnelinconnection) << "Channel" << m_name << "is in error state";
            return;
        }
    }
}

void SshTunnelInConnection::sshDataReceived()
{
    DEBUGCH << "sshDataReceived: SSH data received";
    m_connector.sshDataReceived();
    emit sendEvent();
}

void SshTunnelInConnection::flushTx()
{
    m_connector.flushTx();
}

void SshTunnelInConnection::_socketConnected()
{
    DEBUGCH << "Socket connection established";
    m_connector.setChannel(m_sshChannel);
    m_connector.setSock(&m_sock);
    setChannelState(ChannelState::Ready);
    emit sendEvent();
}

