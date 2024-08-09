#include "sshscpsend.h"
#include "sshclient.h"
#include <QFileInfo>
#include <qdebug.h>


Q_LOGGING_CATEGORY(logscpsend, "ssh.scpsend", QtWarningMsg)

SshScpSend::SshScpSend(const QString &name, SshClient *client):
    SshChannel(name, client)
{
}

SshScpSend::~SshScpSend()
{
    qCDebug(logscpsend) << "free Channel:" << m_name;
}

void SshScpSend::close()
{
    setChannelState(ChannelState::Close);
    sshDataReceived();
}


void SshScpSend::send(const QString &source, QString dest)
{
    m_source = source;
    m_dest = dest;
    setChannelState(ChannelState::Openning);
    sshDataReceived();
}

void SshScpSend::sshDataReceived()
{
    qCDebug(logscpsend) << "Channel "<< m_name << "State:" << channelState();
    switch(channelState())
    {
        case Openning:
        {
            stat(m_source.toStdString().c_str(), &m_fileinfo);
            if ( ! m_sshClient->takeChannelCreationMutex(this) )
            {
                return;
            }
            m_sshChannel = libssh2_scp_send64(m_sshClient->session(), m_dest.toStdString().c_str(), m_fileinfo.st_mode & 0777, m_fileinfo.st_size, 0, 0);
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
                    emit failed();
                }
                setChannelState(ChannelState::Error);
                qCWarning(logscpsend) << "Channel session open failed";
                return;
            }
            qCDebug(logscpsend) << "Channel session opened";
            setChannelState(ChannelState::Exec);
        }

        SSH2FALLTHROUGH(); case Exec:
        {
            m_file.setFileName(m_source);
            if(!m_file.open(QIODevice::ReadOnly))
            {
                if(!m_error)
                {
                    m_error = true;
                    emit failed();
                    qCWarning(logscpsend) << "Can't open source file";
                }
                setChannelState(ChannelState::Close);
                sshDataReceived();
                return;
            }

            setChannelState(ChannelState::Ready);
            /* OK, next step */
        }

        SSH2FALLTHROUGH(); case Ready:
        {
            while(!m_file.atEnd())
            {
                if(m_dataInBuf == 0)
                {
                    m_dataInBuf = m_file.read(m_buffer, PAGE_SIZE);
                }

                ssize_t retsz = libssh2_channel_write_ex(m_sshChannel, 0, m_buffer + m_offset, static_cast<size_t>(m_dataInBuf - m_offset));
                if(retsz == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }

                if(retsz < 0)
                {
                    if(!m_error)
                    {                    
                        QString errStr(sshErrorToString(static_cast<int>(retsz)));
                        sshClient()->setSSHErrorString(errStr);
                        m_error = true;
                        emit failed();
                        qCWarning(logscpsend) << "Can't write result (" << errStr << ")";
                    }
                    setChannelState(ChannelState::Close);
                    sshDataReceived();
                    return;
                }

                m_sent += retsz;
                m_offset += retsz;
                if(m_offset == m_dataInBuf)
                {
                    m_dataInBuf = 0;
                    m_offset = 0;
                }
                emit progress(m_sent, m_file.size());
            }
            setChannelState(ChannelState::Close);
        }

        SSH2FALLTHROUGH(); case Close:
        {
            m_file.close();
            if(m_sent != m_file.size())
            {
                qCDebug(logscpsend) << m_name << "Transfer not completed";
                emit failed();
            }
            else
            {
                emit finished();
            }

            qCDebug(logscpsend) << m_name << "closeChannel";
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
                    emit failed();
                    qCWarning(logscpsend) << "Failed to channel_close: " << errStr;
                }
            }
            setChannelState(ChannelState::WaitClose);
        }

        SSH2FALLTHROUGH(); case WaitClose:
        {
            qCDebug(logscpsend) << "Wait close channel:" << m_name;
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
                    emit failed();
                    qCWarning(logscpsend) << "Failed to channel_wait_close: " << errStr;
                }
            }
            setChannelState(ChannelState::Freeing);
        }

        SSH2FALLTHROUGH(); case Freeing:
        {
            qCDebug(logscpsend) << "free Channel:" << m_name;

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
                    emit failed();
                    qCWarning(logscpsend) << "Failed to free channel: " << errStr;
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
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshScpSend::sshDataReceived);
            return;
        }

        case Free:
        {
            qCDebug(logscpsend) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            emit failed();
            qCDebug(logscpsend) << "Channel" << m_name << "is in error state";
            return;
        }
    }
}
