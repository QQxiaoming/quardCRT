#include "sshscpget.h"
#include "sshclient.h"
#include <QFileInfo>
#include <qdebug.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE (4*1024)
#endif

Q_LOGGING_CATEGORY(logscpget, "ssh.scpget", QtWarningMsg)

SshScpGet::SshScpGet(const QString &name, SshClient *client):
    SshChannel(name, client)
{
}

SshScpGet::~SshScpGet()
{
    qCDebug(logscpget) << "free Channel:" << m_name;
}

void SshScpGet::close()
{
    setChannelState(ChannelState::Close);
    sshDataReceived();
}


void SshScpGet::get(const QString &source, const QString &dest)
{
    m_source = source;
    m_dest = dest;
    setChannelState(ChannelState::Openning);
    sshDataReceived();
}


void SshScpGet::sshDataReceived()
{
    qCDebug(logscpget) << "Channel "<< m_name << "State:" << channelState();
    switch(channelState())
    {
        case Openning:
        {
            if ( ! m_sshClient->takeChannelCreationMutex(this) )
            {
                return;
            }
            m_sshChannel = libssh2_scp_recv2(m_sshClient->session(), qPrintable(m_source), &m_fileinfo);
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
                qCWarning(logscpget) << "Channel session open failed";
                return;
            }
            qCDebug(logscpget) << "Channel session opened";
            setChannelState(ChannelState::Exec);
        }

        FALLTHROUGH; case Exec:
        {
            m_file.setFileName(m_dest);
            if(!m_file.open(QIODevice::WriteOnly))
            {
                if(!m_error)
                {
                    m_error = true;
                    emit failed();
                    qCWarning(logscpget) << "Can't open destination file";
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
            while(m_got < m_fileinfo.st_size)
            {
                char mem[PAGE_SIZE];
                int amount=sizeof(mem);

                if((m_fileinfo.st_size - m_got) < amount) {
                    amount = static_cast<int>(m_fileinfo.st_size - m_got);
                }


                ssize_t retsz = libssh2_channel_read_ex(m_sshChannel, 0, mem, static_cast<size_t>(amount));
                if(retsz == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }

                if(retsz < 0)
                {
                    if(!m_error)
                    {
                        m_error = true;
                        emit failed();
                        qCWarning(logscpget) << "Can't read result (" << sshErrorToString(static_cast<int>(retsz)) << ")";
                    }
                    setChannelState(ChannelState::Close);
                    sshDataReceived();
                    return;
                }

                m_file.write(mem, retsz);
                m_got += retsz;
                emit progress(m_got, m_fileinfo.st_size);
            }
            setChannelState(ChannelState::Close);
        }

        FALLTHROUGH; case Close:
        {
            m_file.close();
            if(m_got != m_fileinfo.st_size)
            {
                qCDebug(logscpget) << m_name << "Transfer not completed";
                m_file.remove();
                emit failed();
            }
            else
            {
                emit finished();
            }

            qCDebug(logscpget) << m_name << "closeChannel";
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
                    emit failed();
                    qCWarning(logscpget) << "Failed to channel_close: " << sshErrorToString(ret);
                }
            }
            setChannelState(ChannelState::WaitClose);
        }

        FALLTHROUGH; case WaitClose:
        {
            qCDebug(logscpget) << "Wait close channel:" << m_name;
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
                    emit failed();
                    qCWarning(logscpget) << "Failed to channel_wait_close: " << sshErrorToString(ret);
                }
            }
            setChannelState(ChannelState::Freeing);
        }

        FALLTHROUGH; case Freeing:
        {
            qCDebug(logscpget) << "free Channel:" << m_name;

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
                    emit failed();
                    qCWarning(logscpget) << "Failed to free channel: " << sshErrorToString(ret);
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
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshScpGet::sshDataReceived);
            return;
        }

        case Free:
        {
            qCDebug(logscpget) << "Channel" << m_name << "is free";
            return;
        }

        case Error:
        {
            emit failed();
            qCDebug(logscpget) << "Channel" << m_name << "is in error state";
            return;
        }
    }
}
