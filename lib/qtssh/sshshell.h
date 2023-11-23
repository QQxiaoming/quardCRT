#pragma once

#include "sshchannel.h"
#include <QSemaphore>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logsshprocess)

class SshShell : public SshChannel
{
    Q_OBJECT

protected:
    explicit SshShell(const QString &name, SshClient *client);
    friend class SshClient;

public:
    virtual ~SshShell() override;
    void close() override;
    QStringList errMsg();
    bool isError();
    int sendData(const char *data, int size);
    void resize(int cols, int rows);

public slots:
    void sshDataReceived() override;

private:
    LIBSSH2_CHANNEL *m_sshChannel {nullptr};
    QStringList m_errMsg;
    bool m_error {false};
    bool m_pty {false};

signals:
    void readyRead(const char *data, int size); 
    void finished();
    void failed();
};
