#ifndef SSHSFTPCOMMANDMKDIR_H
#define SSHSFTPCOMMANDMKDIR_H

#include <QObject>
#include <sshsftpcommand.h>

class SshSftpCommandMkdir : public SshSftpCommand
{
    Q_OBJECT
    const QString &m_dir;
    int m_mode;
    bool m_error {false};

public:
    SshSftpCommandMkdir(const QString &dir, int mode, SshSFtp &parent);
    void process() override;
    bool error() const;
};

#endif // SSHSFTPCOMMANDMKDIR_H
