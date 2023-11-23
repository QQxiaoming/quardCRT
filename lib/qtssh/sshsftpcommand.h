#ifndef SSHSFTPCOMMAND_H
#define SSHSFTPCOMMAND_H

#include <QObject>
#include "sshsftp.h"


#define SFTP_BUFFER_SIZE 4096

class SshSftpCommand : public QObject
{
    Q_OBJECT

    SshSFtp &m_sftp;
    QString m_name;

public:
    enum CommandState {
        Openning,
        Exec,
        Closing,
        Terminate,
        Error
    };
    Q_ENUMS(CommandState);

    explicit SshSftpCommand(SshSFtp &sftp);
    virtual void process() = 0;

    CommandState state() const;

    void setState(const CommandState &state);

    SshSFtp &sftp() const;

    void setName(const QString &name);

    QString name() const;

    QStringList errMsg() const;

protected:
    CommandState m_state;
    QStringList m_errMsg;

signals:
    void stateChanged(CommandState state);
};

#endif // SSHSFTPCOMMAND_H
