#include "sshsftpcommand.h"

SshSFtp &SshSftpCommand::sftp() const
{
    return m_sftp;
}

void SshSftpCommand::setName(const QString &name)
{
    m_name = name;
}

QString SshSftpCommand::name() const
{
    return m_name;
}

QStringList SshSftpCommand::errMsg() const
{
    return m_errMsg;
}

SshSftpCommand::SshSftpCommand(SshSFtp &sftp)
    : QObject(qobject_cast<QObject*>(&sftp))
    , m_sftp(sftp)
{
    m_state = CommandState::Openning;
}

SshSftpCommand::CommandState SshSftpCommand::state() const
{
    return m_state;
}

void SshSftpCommand::setState(const CommandState &state)
{
    if(m_state != state)
    {
        m_state = state;
        emit stateChanged(m_state);
    }
}
