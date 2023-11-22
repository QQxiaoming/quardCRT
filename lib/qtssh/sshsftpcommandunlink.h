#ifndef SSHSFTPCOMMANDUNLINK_H
#define SSHSFTPCOMMANDUNLINK_H

#include <QObject>
#include <sshsftpcommand.h>

class SshSftpCommandUnlink : public SshSftpCommand
{
    Q_OBJECT
    const QString &m_path;
    bool m_error {false};

public:
    SshSftpCommandUnlink(const QString &path, SshSFtp &parent);
    void process() override;
    bool error() const;
};

#endif // SSHSFTPCOMMANDUNLINK_H
