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
    ~SshShell() override;
    void close() override;
    QStringList errMsg();
    bool isError();
    int sendData(const char *data, int size);
    void resize(int cols, int rows);
    void initSize(int cols, int rows) {
        m_cols = cols;
        m_rows = rows;
    
    }

public slots:
    void sshDataReceived() override;

private:
    LIBSSH2_CHANNEL *m_sshChannel {nullptr};
    QStringList m_errMsg;
    bool m_error {false};
    bool m_pty {false};
    int m_cols = 80;
    int m_rows = 24;

signals:
    void readyRead(const char *data, int size); 
    void finished();
    void failed();
};
