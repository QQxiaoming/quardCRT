#include "sshsftpcommandget.h"
#include "sshclient.h"

SshSftpCommandGet::SshSftpCommandGet(QFile &fout, const QString &source, SshSFtp &parent)
    : SshSftpCommand(parent)
    , m_fout(fout)
    , m_src(source)
{
    setName(QString("get(%1, %2)").arg(source).arg(fout.fileName()));
}

void SshSftpCommandGet::process()
{
    switch(m_state)
    {
    case Openning:
        m_sftpfile = libssh2_sftp_open_ex(
                    sftp().getSftpSession(),
                    qPrintable(m_src),
                    static_cast<unsigned int>(m_src.size()),
                    LIBSSH2_FXF_READ,
                    0,
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
            qCDebug(logsshsftp) << "Can't open SFTP file " << m_src << ", " << QString(emsg);
            m_error = true;
            m_errMsg << "Can't open SFTP file " + m_src + ", " + QString(emsg);
            setState(CommandState::Error);
            return;
        }

        if(!m_fout.open(QIODevice::WriteOnly))
        {
            m_error = true;
            setState(CommandState::Closing);
        }
        setState(CommandState::Exec);
        FALLTHROUGH;
    case Exec:
        while(1)
        {
            ssize_t rc = libssh2_sftp_read(m_sftpfile, m_buffer, SFTP_BUFFER_SIZE);
            if(rc < 0)
            {
                if(rc == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                qCWarning(logsshsftp) << "SFTP read error " << rc;
                m_error = true;
                m_errMsg << QString("SFTP read error: %1").arg(rc);
                setState(CommandState::Error);
                break;
            }
            else if(rc == 0)
            {
                // EOF
                setState(CommandState::Closing);
                break;
            }
            else
            {
                char *begin = m_buffer;
                while(rc)
                {
                    ssize_t wrc = m_fout.write(begin, rc);
                    rc -= wrc;
                    begin += wrc;
                }
            }
        }

    case Closing:
    {
        if(m_fout.isOpen())
        {
            m_fout.close();
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
