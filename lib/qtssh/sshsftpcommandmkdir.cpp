#include "sshsftpcommandmkdir.h"
#include "sshclient.h"

SshSftpCommandMkdir::SshSftpCommandMkdir(const QString &dir, int mode, SshSFtp &parent)
    : SshSftpCommand(parent)
    , m_dir(dir)
    , m_mode(mode)
{
    setName(QString("mkdir(%1)").arg(dir));
}

bool SshSftpCommandMkdir::error() const
{
    return m_error;
}


void SshSftpCommandMkdir::process()
{
    int res;
    switch(m_state)
    {
    case Openning:
        res = libssh2_sftp_mkdir_ex(
                    sftp().getSftpSession(),
                    qPrintable(m_dir),
                    static_cast<unsigned int>(m_dir.size()),
                    m_mode
                    );

        if(res < 0)
        {
            if(res == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            m_error = true;
            m_errMsg << "SFTP mkdir error " + res;
            qCWarning(logsshsftp) << "SFTP mkdir error " << res;
            setState(CommandState::Error);
            break;
        }
        setState(CommandState::Terminate);
        FALLTHROUGH;
    case Terminate:
        break;

    case Error:
        break;

    default:
        setState(CommandState::Terminate);
        break;
    }
}
