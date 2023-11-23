#pragma once

#include "sshchannel.h"
#include <QFile>

Q_DECLARE_LOGGING_CATEGORY(logscpget)

class SshScpGet : public SshChannel
{
    Q_OBJECT

protected:
    SshScpGet(const QString &name, SshClient *client);
    friend class SshClient;

public:
    virtual ~SshScpGet() override;
    void close() override;



public slots:
    void get(const QString &source, const QString &dest);
    void sshDataReceived() override;

private:
    QString m_source;
    QString m_dest;
    libssh2_struct_stat m_fileinfo = {};
    libssh2_struct_stat_size m_got = 0;
    LIBSSH2_CHANNEL *m_sshChannel {nullptr};
    bool m_error {false};
    QFile m_file;

signals:
    void finished();
    void failed();
    void progress(qint64 tx, qint64 total);
};
