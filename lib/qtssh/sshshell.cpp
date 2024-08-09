#include "sshshell.h"
#include "sshclient.h"
#include <QTimer>
#include <QEventLoop>

Q_LOGGING_CATEGORY(logsshshell, "ssh.shell", QtWarningMsg)

SshShell::SshShell(const QString &name, SshClient *client)
    : SshChannel(name, client)
{
}

SshShell::~SshShell()
{
    qCDebug(sshchannel) << "free Channel:" << m_name;
}

void SshShell::close()
{
    setChannelState(ChannelState::Close);
    sshDataReceived();
}

int SshShell::sendData(const char *data, int size) {
    if(channelState() != ChannelState::Ready) {
        return 0;
    }
    if(m_sshChannel == nullptr) {
        return 0;
    }
    ssize_t retsz = libssh2_channel_write_ex(m_sshChannel, 0, data, size);
    if(retsz == LIBSSH2_ERROR_EAGAIN)
    {
        return 0;
    }
    if(retsz < 0)
    {
        if(!m_error)
        {                        
            QString errStr(sshErrorToString(static_cast<int>(retsz)));
            sshClient()->setSSHErrorString(errStr);
            m_error = true;
            m_errMsg << QString("Can't write data (%1)").arg(errStr);
            emit failed();
            qCWarning(logsshprocess) << "Can't write data (" << errStr << ")";
        }
        setChannelState(ChannelState::Close);
        sshDataReceived();
        return 0;
    }
    return retsz;
}

void SshShell::resize(int cols, int rows)
{
    if(channelState() != ChannelState::Ready) {
        return;
    }
    if(m_sshChannel == nullptr) {
        return;
    }
    m_cols = cols;
    m_rows = rows;
    int ret = libssh2_channel_request_pty_size_ex(m_sshChannel, cols, rows, 0, 0);
    if(ret == LIBSSH2_ERROR_EAGAIN)
    {
        return;
    }
    if(ret != 0)
    {
        if(!m_error)
        {
            m_error = true;
            m_errMsg << QString("Failed to resize pty: %1").arg(ret);
            emit failed();
            qCWarning(logsshprocess) << "Failed to resize pty" << ret;
        }
        setChannelState(ChannelState::Close);
        sshDataReceived();
        return;
    }
}

QStringList SshShell::errMsg()
{
    return m_errMsg;
}

bool SshShell::isError()
{
    return m_error;
}

void SshShell::sshDataReceived()
{
    qCDebug(logsshprocess) << "Channel "<< m_name << "State:" << channelState();
    switch(channelState())
    {
        case Openning:
        {
            if ( ! m_sshClient->takeChannelCreationMutex(this) )
            {
                return;
            }
            m_sshChannel = libssh2_channel_open_ex(m_sshClient->session(), "session", sizeof("session") - 1, LIBSSH2_CHANNEL_WINDOW_DEFAULT, LIBSSH2_CHANNEL_PACKET_DEFAULT, nullptr, 0);
            m_sshClient->releaseChannelCreationMutex(this);
            if (m_sshChannel == nullptr)
            {
                int ret = libssh2_session_last_error(m_sshClient->session(), nullptr, nullptr, 0);
                if(ret == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                if(!m_error)
                {
                    m_error = true;
                    m_errMsg << QString("Channel session open failed: %1").arg(ret);
                    emit failed();
                }
                setChannelState(ChannelState::Error);
                qCWarning(logsshprocess) << "Channel session open failed";
                return;
            }
            qCDebug(logsshprocess) << "Channel session opened";
            setChannelState(ChannelState::Exec);
        }

        SSH2FALLTHROUGH(); case Exec:
        {
            if(!m_pty) {
                int ret = libssh2_channel_request_pty_ex(m_sshChannel, "xterm", sizeof("xterm") - 1, nullptr, 0, m_cols, m_rows, 0, 0);
                if (ret == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                if(ret != 0)
                {
                    if(!m_error)
                    {
                        m_error = true;
                        m_errMsg << QString("Failed to request pty: %1").arg(ret);
                        emit failed();
                        qCWarning(logsshprocess) << "Failed to request pty" << ret;
                    }
                    setChannelState(ChannelState::Close);
                    sshDataReceived();
                    return;
                } else {
                    m_pty = true;
                }
            }
            int ret = libssh2_channel_shell(m_sshChannel);
            if (ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret != 0)
            {
                if(!m_error)
                {
                    m_error = true;
                    m_errMsg << QString("Failed to run shell: %1").arg(ret);
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to run shell" << ret;
                }
                setChannelState(ChannelState::Close);
                sshDataReceived();
                return;
            }
            setChannelState(ChannelState::Ready);
            return;
        }
        case Ready:
        {
            ssize_t retsz;
            char buffer[16*1024];

            retsz = libssh2_channel_read_ex(m_sshChannel, 0, buffer, 16 * 1024);
            if(retsz == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }

            if(retsz)
                emit readyRead(buffer, static_cast<int>(retsz));

            retsz = libssh2_channel_read_stderr(m_sshChannel, buffer, 16 * 1024);
            if(retsz == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }

            if(retsz)
                emit readyRead(buffer, static_cast<int>(retsz));

            if (libssh2_channel_eof(m_sshChannel) == 1)
            {
                setChannelState(ChannelState::Close);
                emit finished();
            }
            return;
        }
        case Close:
        {
            qCDebug(logsshprocess) << "closeChannel:" << m_name;
            libssh2_channel_send_eof(m_sshChannel);
            int ret = libssh2_channel_close(m_sshChannel);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret < 0)
            {
                if(!m_error)
                {                    
                    QString errStr(sshErrorToString(ret));
                    sshClient()->setSSHErrorString(errStr);
                    m_error = true;
                    m_errMsg << QString("Failed to channel_close: %1").arg(errStr);
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to channel_close: " << errStr;
                }
            }
            setChannelState(ChannelState::WaitClose);
        }

        SSH2FALLTHROUGH(); case WaitClose:
        {
            qCDebug(logsshprocess) << "Wait close channel:" << m_name;
            int ret = libssh2_channel_wait_closed(m_sshChannel);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret < 0)
            {
                if(!m_error)
                {                    
                    QString errStr(sshErrorToString(ret));
                    sshClient()->setSSHErrorString(errStr);
                    m_error = true;
                    m_errMsg << QString("Failed to channel_wait_close: %1").arg(errStr);
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to channel_wait_close: " << errStr;
                }
            }
            setChannelState(ChannelState::Freeing);
        }

        SSH2FALLTHROUGH(); case Freeing:
        {
            qCDebug(logsshprocess) << "free Channel:" << m_name;

            int ret = libssh2_channel_free(m_sshChannel);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret < 0)
            {
                if(!m_error)
                {                    
                    QString errStr(sshErrorToString(ret));
                    sshClient()->setSSHErrorString(errStr);
                    m_error = true;
                    m_errMsg << QString("Failed to free channel: %1").arg(errStr);
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to free channel: " << errStr;
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
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshShell::sshDataReceived);
            return;
        }

        case Free:
        {
            qCDebug(logsshprocess) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            emit failed();
            qCDebug(logsshprocess) << "Channel" << m_name << "is in error state";
            setChannelState(ChannelState::Free);
            sshDataReceived();
            return;
        }
    }
}
