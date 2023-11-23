#ifndef SSHSFTPCOMMANDFILEINFO_H
#define SSHSFTPCOMMANDFILEINFO_H

#include <QObject>
#include <sshsftpcommand.h>

class SshSftpCommandFileInfo : public SshSftpCommand
{
    Q_OBJECT
    const QString &m_path;
    bool m_error {false};
    LIBSSH2_SFTP_ATTRIBUTES m_fileinfo;

public:
    SshSftpCommandFileInfo(const QString &path, SshSFtp &parent);
    void process() override;
    bool error() const;
    LIBSSH2_SFTP_ATTRIBUTES fileinfo() const;
};

#endif // SSHSFTPCOMMANDFILEINFO_H
