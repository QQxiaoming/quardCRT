#ifndef SSHSFTPCOMMANDGET_H
#define SSHSFTPCOMMANDGET_H

#include <QObject>
#include <QFile>
#include <sshsftpcommand.h>

class SshSftpCommandGet : public SshSftpCommand
{
    Q_OBJECT

    QFile &m_fout;
    const QString &m_src;
    LIBSSH2_SFTP_HANDLE *m_sftpfile;
    bool m_error {false};
    char m_buffer[SFTP_BUFFER_SIZE];

public:
    SshSftpCommandGet(QFile &fout, const QString &source, SshSFtp &parent);
    void process() override;
};

#endif // SSHSFTPCOMMANDGET_H
