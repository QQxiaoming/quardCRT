#include "sshsftpcommandfileinfo.h"
#include "sshclient.h"

LIBSSH2_SFTP_ATTRIBUTES SshSftpCommandFileInfo::fileinfo() const
{
    return m_fileinfo;
}

SshSftpCommandFileInfo::SshSftpCommandFileInfo(const QString &path, SshSFtp &parent)
    : SshSftpCommand(parent)
    , m_path(path)
{
    setName(QString("unlink(%1)").arg(path));
}



bool SshSftpCommandFileInfo::error() const
{
    return m_error;
}

void SshSftpCommandFileInfo::process()
{
    int res;
    switch(m_state)
    {
    case Openning:
        res = libssh2_sftp_stat_ex(
                    sftp().getSftpSession(),
                    qPrintable(m_path),
                    static_cast<unsigned int>(m_path.size()),
                    LIBSSH2_SFTP_STAT,
                    &m_fileinfo
                    );

        if(res < 0)
        {
            if(res == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            m_error = true;
            m_errMsg << QString("SFTP unlink error: %1").arg(res);
            qCWarning(logsshsftp) << "SFTP unlink error " << res;
            if (res == LIBSSH2_ERROR_SFTP_PROTOCOL)
            {
                int err = libssh2_sftp_last_error(sftp().getSftpSession());
                if (err == LIBSSH2_FX_NO_SUCH_FILE)
                {
                    qCWarning(logsshsftp) << "No such file or directory: " << m_path;
                    m_errMsg << "No such file or directory: " + m_path;
                    setState(CommandState::Terminate);
                    return;
                }
                else
                {
                    qCWarning(logsshsftp) << "SFTP last error " << err;
                    m_errMsg << "SFTP last error " + err;
                }
            }
            setState(CommandState::Error);
            return;
        }
        else
        {
            setState(CommandState::Terminate);
            FALLTHROUGH;
        }
    case Terminate:
        break;

    case Error:
        break;

    default:
        setState(CommandState::Terminate);
        break;
    }
}
