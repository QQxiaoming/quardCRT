#include "sshsftpcommandsend.h"
#include "sshclient.h"

SshSftpCommandSend::SshSftpCommandSend(const QString &source, QString dest, SshSFtp &parent)
    : SshSftpCommand(parent)
    , m_dest(dest)
    , m_localfile(source)
{
    setName(QString("send(%1, %2)").arg(source, dest));
}

void SshSftpCommandSend::process()
{
    switch(m_state)
    {
    case Openning:
        m_sftpfile = libssh2_sftp_open_ex(
                    sftp().getSftpSession(),
                    qPrintable(m_dest),
                    static_cast<unsigned int>(m_dest.size()),
                    LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,
                    LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR| LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH,
                    LIBSSH2_SFTP_OPENFILE
                    );

        if(!m_sftpfile)
        {
            char *emsg;
            int size;
            int ret = libssh2_session_last_error(sftp().sshClient()->session(), &emsg, &size, 0);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            qCDebug(logsshsftp) << "Can't open SFTP file " << m_dest << ", " << QString(emsg);
            m_error = true;
            m_errMsg << "Can't open SFTP file " + m_dest + ", " + QString(emsg);
            setState(CommandState::Error);
            return;
        }

        if(!m_localfile.open(QIODevice::ReadOnly))
        {
            qCWarning(logsshsftp) << "Can't open local file " << m_localfile.fileName();
            m_error = true;
            m_errMsg << "Can't open local file " + m_localfile.fileName();
            setState(CommandState::Closing);
            break;
        }
        setState(CommandState::Exec);
        FALLTHROUGH;
    case Exec:
        while(1)
        {
            if(m_nread == 0 && m_localfile.isOpen())
            {
                m_nread = m_localfile.read(m_buffer, SFTP_BUFFER_SIZE);
                if (m_nread <= 0) {
                    /* end of file */
                    setState(CommandState::Closing);
                    break;
                }
                m_begin = m_buffer;
            }
            while(m_nread != 0)
            {
                ssize_t rc = libssh2_sftp_write(m_sftpfile, m_begin, m_nread);
                if(rc < 0)
                {
                    if(rc == LIBSSH2_ERROR_EAGAIN)
                    {
                        return;
                    }
                    qCWarning(logsshsftp) << "SFTP Write error " << rc;
                }
                m_nread -= rc;
                m_begin += rc;
            }
        }
        break;

    case Closing:
    {
        if(m_localfile.isOpen())
        {
            m_localfile.close();
        }
        int rc = libssh2_sftp_close_handle(m_sftpfile);
        if(rc < 0)
        {
            if(rc == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            qCWarning(logsshsftp) << "SFTP close error " << rc;
            m_errMsg << QString("SFTP close error: %1").arg(rc);
            setState(CommandState::Error);
        }
        if(m_error)
        {
            setState(CommandState::Error);
            break;
        }
        else setState(CommandState::Terminate);
        FALLTHROUGH;
    }
    case Terminate:
        break;

    case Error:
        break;
    }
}
