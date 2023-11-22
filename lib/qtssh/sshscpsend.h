#pragma once

#include "sshchannel.h"
#include <QFile>

#if !defined(PAGE_SIZE)
#define PAGE_SIZE (4*1024)
#endif

Q_DECLARE_LOGGING_CATEGORY(logscpsend)

class SshScpSend : public SshChannel
{
    Q_OBJECT

protected:
    SshScpSend(const QString &name, SshClient * client);
    friend class SshClient;

public:
    virtual ~SshScpSend() override;
    void close() override;

public slots:
    void send(const QString &source, QString dest);
    void sshDataReceived() override;


private:
    QString m_source;
    QString m_dest;
    struct stat m_fileinfo = {};
    libssh2_struct_stat_size m_sent = 0;
    LIBSSH2_CHANNEL *m_sshChannel {nullptr};
    bool m_error {false};
    QFile m_file;
    char m_buffer[PAGE_SIZE];
    qint64 m_dataInBuf {0};
    qint64 m_offset {0};

signals:
    void finished();
    void failed();
    void progress(qint64 tx, qint64 total);
};
