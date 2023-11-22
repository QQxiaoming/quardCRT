#ifndef SSHTUNNELDATACONNECTOR_H
#define SSHTUNNELDATACONNECTOR_H

#include <QObject>
#include <QLoggingCategory>
#include "sshchannel.h"
class QTcpSocket;

#define BUFFER_SIZE (128*1024)

Q_DECLARE_LOGGING_CATEGORY(logxfer)

class SshTunnelDataConnector : public QObject
{
    Q_OBJECT

    SshClient *m_sshClient  {nullptr};
    LIBSSH2_CHANNEL *m_sshChannel {nullptr};
    QTcpSocket *m_sock  {nullptr};
    QString m_name;

    /* Transfer functions */

    /* TX Channel */
    char m_tx_buffer[BUFFER_SIZE] {0,};
    char *m_tx_start_ptr {nullptr};
    char *m_tx_stop_ptr {nullptr};
    size_t _txBufferLen() const;

    bool m_tx_data_on_sock {false};
    ssize_t _transferSockToTx();
    ssize_t m_total_sockToTx {0};
    bool m_tx_eof {false};

    ssize_t _transferTxToSsh();
    ssize_t m_total_TxToSsh {0};
    bool m_tx_closed {false};


    /* RX Channel */
    char m_rx_buffer[BUFFER_SIZE] {0,};
    char *m_rx_start_ptr {nullptr};
    char *m_rx_stop_ptr {nullptr};
    size_t _rxBufferLen() const;

    bool m_rx_data_on_ssh {false};
    ssize_t _transferSshToRx();
    ssize_t m_total_SshToRx {0};
    bool m_rx_eof {false};

    ssize_t _transferRxToSock();
    ssize_t m_total_RxToSock {0};
    bool m_rx_closed {false};


public slots:
    void sshDataReceived();
    bool process();
    void close();
    bool isClosed();
    void flushTx();

public:
    explicit SshTunnelDataConnector(SshClient *client, const QString &name, QObject *parent = nullptr);
    virtual ~SshTunnelDataConnector();
    void setChannel(LIBSSH2_CHANNEL *channel);
    void setSock(QTcpSocket *sock);

signals:
    void sendEvent();
    void processed();

private slots:
    void _socketDisconnected();
    void _socketDataRecived();
    void _socketError();
};

#endif // SSHTUNNELDATACONNECTOR_H
