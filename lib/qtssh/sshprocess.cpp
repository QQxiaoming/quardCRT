#include "sshprocess.h"
#include "sshclient.h"
#include <QTimer>
#include <QEventLoop>

Q_LOGGING_CATEGORY(logsshprocess, "ssh.process", QtWarningMsg)

SshProcess::SshProcess(const QString &name, SshClient *client)
    : SshChannel(name, client)
{
}

SshProcess::~SshProcess()
{
    qCDebug(sshchannel) << "free Channel:" << m_name;
}

void SshProcess::close()
{
    setChannelState(ChannelState::Close);
    sshDataReceived();
}

QByteArray SshProcess::result()
{
    return m_result;
}

QStringList SshProcess::errMsg()
{
    return m_errMsg;
}

bool SshProcess::isError()
{
    return m_error;
}

void SshProcess::runCommand(const QString &cmd)
{
    m_cmd = cmd;
    sshDataReceived();
}

void SshProcess::sshDataReceived()
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

        FALLTHROUGH; case Exec:
        {
            if(m_cmd.size() == 0)
            {
                /* Nothing to process */
                return;
            }
            qCDebug(logsshprocess) << "runCommand(" << m_cmd << ")";
            int ret = libssh2_channel_process_startup(m_sshChannel, "exec", sizeof("exec") - 1, m_cmd.toStdString().c_str(), static_cast<unsigned int>(m_cmd.size()));
            if (ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret != 0)
            {
                if(!m_error)
                {
                    m_error = true;
                    m_errMsg << QString("Failed to run command: %1").arg(ret);
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to run command" << ret;
                }
                setChannelState(ChannelState::Close);
                sshDataReceived();
                return;
            }
            setChannelState(ChannelState::Ready);
            /* OK, next step */
        }

        FALLTHROUGH; case Ready:
        {
            ssize_t retsz;
            char buffer[16*1024];

            retsz = libssh2_channel_read_ex(m_sshChannel, 0, buffer, 16 * 1024);
            if(retsz == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }

            if(retsz < 0)
            {
                if(!m_error)
                {
                    m_error = true;
                    m_errMsg << QString("Can't read result (%1)").arg(sshErrorToString(static_cast<int>(retsz)));
                    emit failed();
                    qCWarning(logsshprocess) << "Can't read result (" << sshErrorToString(static_cast<int>(retsz)) << ")";
                }
                setChannelState(ChannelState::Close);
                sshDataReceived();
                return;
            }

            m_result.append(buffer, static_cast<int>(retsz));

            retsz = libssh2_channel_read_stderr(m_sshChannel, buffer, 16 * 1024);
            if(retsz == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if (retsz < 0)
            {
                if(!m_error)
                {
                    m_error = true;
                    m_errMsg << QString("Can't read stderr msg (%1)").arg(sshErrorToString(static_cast<int>(retsz)));
                    emit failed();
                    qCWarning(logsshprocess) << "Can't read stderr msg (" << sshErrorToString(static_cast<int>(retsz)) << ")";
                }
                setChannelState(ChannelState::Close);
                sshDataReceived();
                return;
            }
            else if (retsz > 0)
            {
                if (!m_error)
                {
                    m_error = true;
                    emit failed();
                }
                qCWarning(logsshprocess) << "Run command error";
                m_errMsg << QString("Run command error: (%1)").arg(buffer);
            }

            if (libssh2_channel_eof(m_sshChannel) == 1)
            {
                qCDebug(logsshprocess) << "runCommand(" << m_cmd << ") RESULT: " << m_result;
                setChannelState(ChannelState::Close);
                emit finished();
            }
        }

        FALLTHROUGH; case Close:
        {
            qCDebug(logsshprocess) << "closeChannel:" << m_name;
            int ret = libssh2_channel_close(m_sshChannel);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret < 0)
            {
                if(!m_error)
                {
                    m_error = true;
                    m_errMsg << QString("Failed to channel_close: %1").arg(sshErrorToString(ret));
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to channel_close: " << sshErrorToString(ret);
                }
            }
            setChannelState(ChannelState::WaitClose);
        }

        FALLTHROUGH; case WaitClose:
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
                    m_error = true;
                    m_errMsg << QString("Failed to channel_wait_close: %1").arg(sshErrorToString(ret));
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to channel_wait_close: " << sshErrorToString(ret);
                }
            }
            setChannelState(ChannelState::Freeing);
        }

        FALLTHROUGH; case Freeing:
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
                    m_error = true;
                    m_errMsg << QString("Failed to free channel: %1").arg(sshErrorToString(ret));
                    emit failed();
                    qCWarning(logsshprocess) << "Failed to free channel: " << sshErrorToString(ret);
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
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshProcess::sshDataReceived);
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
