#include "sshsftpcommandreaddir.h"
#include "sshclient.h"

QStringList SshSftpCommandReadDir::result() const
{
    return m_result;
}

SshSftpCommandReadDir::SshSftpCommandReadDir(const QString &dir, SshSFtp &parent)
    : SshSftpCommand(parent)
    , m_dir(dir)
{
    setName(QString("readdir(%1)").arg(dir));
}

void SshSftpCommandReadDir::process()
{
    switch(m_state)
    {
    case Openning:
        m_sftpdir = libssh2_sftp_open_ex(
                    sftp().getSftpSession(),
                    qPrintable(m_dir),
                    static_cast<unsigned int>(m_dir.size()),
                    0,
                    0,
                    LIBSSH2_SFTP_OPENDIR
                    );

        if(!m_sftpdir)
        {
            char *emsg;
            int size;
            int ret = libssh2_session_last_error(sftp().sshClient()->session(), &emsg, &size, 0);
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            qCDebug(logsshsftp) << "Can't open SFTP dir " << m_dir << ", " << QString(emsg);
            m_error = true;
            m_errMsg << "Can't open SFTP dir " + m_dir + ", " + QString(emsg);
            setState(CommandState::Error);
            return;
        }
        else
        {
            setState(CommandState::Exec);
            FALLTHROUGH;
        }
    case Exec:
        while(1)
        {
            ssize_t rc = libssh2_sftp_readdir_ex(m_sftpdir, m_buffer, SFTP_BUFFER_SIZE, nullptr, 0, &m_attrs);
            if(rc < 0)
            {
                if(rc == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                qCWarning(logsshsftp) << "SFTP readdir error " << rc;
                m_errMsg << QString("SFTP readdir error: %1").arg(rc);
            }
            else if(rc == 0)
            {
                // EOF
                setState(Closing);
                break;
            }
            else
            {
                m_result.append(QString(m_buffer));
            }
        }

    case Closing:
    {
        int rc = libssh2_sftp_close_handle(m_sftpdir);
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
