#pragma once

#include "sshchannel.h"
#include <libssh2_sftp.h>
#include <libssh2.h>
#include <QEventLoop>
#include <QTimer>
#include <QStringList>
#include <QHash>
#include <QLoggingCategory>

class SshSftpCommand;

Q_DECLARE_LOGGING_CATEGORY(logsshsftp)

class SshSFtp : public SshChannel
{
    Q_OBJECT

private:
    LIBSSH2_SFTP *m_sftpSession {nullptr};
    QString m_mkdir;
    bool m_error {false};
    QStringList m_errMsg;

    QList<SshSftpCommand *> m_cmd;
    SshSftpCommand *m_currentCmd {nullptr};

    QHash<QString,  LIBSSH2_SFTP_ATTRIBUTES> m_fileinfo;
    LIBSSH2_SFTP_ATTRIBUTES getFileInfo(const QString &path);

protected:
    SshSFtp(const QString &name, SshClient * client);
    friend class SshClient;

public:
    virtual ~SshSFtp() override;
    void close() override;

    QString send(const QString &source, QString dest);
    bool get(const QString &source, QString dest, bool override = false);
    int mkdir(const QString &dest, int mode = 0755);
    QStringList readdir(const QString &d);
    bool isDir(const QString &d);
    bool isFile(const QString &d);
    int mkpath(const QString &dest);
    bool unlink(const QString &d);
    quint64 filesize(const QString &d);

    LIBSSH2_SFTP *getSftpSession() const;
    bool processCmd(SshSftpCommand *cmd);

    bool isError();
    QStringList errMsg();

public slots:
    void sshDataReceived() override;

private slots:
    void _eventLoop();

signals:
    void sendEvent();
    void cmdEvent();
};
