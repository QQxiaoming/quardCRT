#ifndef SSHSFTPCOMMANDREADDIR_H
#define SSHSFTPCOMMANDREADDIR_H

#include <QObject>
#include <sshsftpcommand.h>

class SshSftpCommandReadDir : public SshSftpCommand
{
    Q_OBJECT
    const QString &m_dir;

    QStringList m_result;
    LIBSSH2_SFTP_HANDLE *m_sftpdir;
    char m_buffer[SFTP_BUFFER_SIZE];
    bool m_error {false};
    LIBSSH2_SFTP_ATTRIBUTES m_attrs;

public:
    SshSftpCommandReadDir(const QString &dir, SshSFtp &parent);
    void process() override;
    QStringList result() const;
};

#endif // SSHSFTPCOMMANDREADDIR_H
