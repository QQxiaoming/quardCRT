#include "sshtunneldataconnector.h"
#include <QTcpSocket>
#include <QEventLoop>
#include "sshclient.h"

Q_LOGGING_CATEGORY(logxfer, "ssh.tunnel.transfer", QtWarningMsg)
#define DEBUGCH qCDebug(logxfer) << m_name

SshTunnelDataConnector::SshTunnelDataConnector(SshClient *client, const QString &name, QObject *parent)
    : QObject(parent)
    , m_sshClient(client)
    , m_name(name)
{
    DEBUGCH << "SshTunnelDataConnector constructor";
    m_tx_data_on_sock = true;
}

SshTunnelDataConnector::~SshTunnelDataConnector()
{
    emit processed();
    if(m_sock) QObject::disconnect(m_sock);
    DEBUGCH << "TOTAL TRANSFERED: Tx:" << m_total_TxToSsh << " | Rx:" << m_total_RxToSock;
}

void SshTunnelDataConnector::setChannel(LIBSSH2_CHANNEL *channel)
{
    m_sshChannel = channel;
}

void SshTunnelDataConnector::setSock(QTcpSocket *sock)
{
    m_sock = sock;
    QObject::connect(m_sock, &QTcpSocket::disconnected,
                     this,   &SshTunnelDataConnector::_socketDisconnected);

    QObject::connect(m_sock, &QTcpSocket::destroyed, [this](){m_sock = nullptr;});

    QObject::connect(m_sock, &QTcpSocket::readyRead,
                     this,   &SshTunnelDataConnector::_socketDataRecived);


#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    QObject::connect(m_sock, &QAbstractSocket::errorOccurred,
                     this,   &SshTunnelDataConnector::_socketError);
#else
    QObject::connect(m_sock, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                     this,   &SshTunnelDataConnector::_socketError);
#endif

    QObject::connect(m_sock, &QTcpSocket::bytesWritten, this, [this](qint64 len){ m_total_RxToSock += len; });
}

void SshTunnelDataConnector::_socketDisconnected()
{
    DEBUGCH << "_socketDisconnected: Socket disconnected";
    m_tx_eof = true;
    emit processed();
    emit sendEvent();
}

void SshTunnelDataConnector::_socketDataRecived()
{
    DEBUGCH << "_socketDataRecived: Socket data received";
    m_tx_data_on_sock = true;
    emit sendEvent();
}

void SshTunnelDataConnector::_socketError()
{
    DEBUGCH << "_socketError";
    emit processed();
    auto error = m_sock->error();
    switch(error)
    {
        case QAbstractSocket::RemoteHostClosedError:
            DEBUGCH << "socket RemoteHostClosedError, data available:" << m_sock->bytesAvailable();
            // Socket will be closed just after this, nothing to care about
            break;
        default:
            qCWarning(logxfer) << m_name << "socket error=" << error << m_sock->errorString();
            // setChannelState(ChannelState::Close);
    }
}

size_t SshTunnelDataConnector::_txBufferLen() const
{
    if(m_tx_stop_ptr == nullptr || m_tx_start_ptr == nullptr)
        return 0;

    if(m_tx_stop_ptr < m_tx_start_ptr)
    {
        qCWarning(logxfer) << m_name << "TX Buffer error";
        return 0;
    }

    return static_cast<size_t>(m_tx_stop_ptr - m_tx_start_ptr);
}

ssize_t SshTunnelDataConnector::_transferSockToTx()
{
    if(!m_sock) return 0;
    qint64 len = 0;
    if(_txBufferLen() != 0)
    {
        qCDebug(logxfer) << m_name << "Asking transfer sock to tx when buffer not empty (" << _txBufferLen() << " bytes)";
        return -1;
    }

    if(m_sock == nullptr)
    {
        qCCritical(logxfer) << m_name << "_transferSockToTx on invalid socket";
        return -1;
    }

    len = m_sock->read(m_tx_buffer, BUFFER_SIZE);
    m_tx_data_on_sock = (m_sock->bytesAvailable() > 0);
    if(len > 0)
    {
        m_tx_start_ptr = m_tx_buffer;
        m_tx_stop_ptr = m_tx_buffer + len;
        m_total_sockToTx += len;

        DEBUGCH << "_transferSockToTx: " << len << "bytes (available:" << m_sock->bytesAvailable() << ", state:" << m_sock->state() << ")";
        if(m_tx_data_on_sock)
        {
            DEBUGCH << "_transferSockToTx: There is other data in socket, re-arm read";
            emit sendEvent();
        }
    }
    else if(len < 0)
    {
        qCWarning(logxfer) << m_name << "_transferSockToTx: error: " << len << " Bytes available " << m_sock->bytesAvailable();
        m_tx_stop_ptr = nullptr;
        m_tx_start_ptr = nullptr;
    }
    else
    {
        DEBUGCH << m_name << "_transferSockToTx: 0 lenght";
        m_tx_stop_ptr = nullptr;
        m_tx_start_ptr = nullptr;
    }

    emit processed();
    if(len > 0)
        emit sendEvent();
    return len;
}

ssize_t SshTunnelDataConnector::_transferTxToSsh()
{
    ssize_t transfered = 0;

    if(m_tx_closed) return 0;
    if(!m_sshChannel) return 0;

    while(_txBufferLen() > 0)
    {
        ssize_t len = libssh2_channel_write(m_sshChannel, m_tx_start_ptr, _txBufferLen());
        if(len == LIBSSH2_ERROR_EAGAIN)
        {
            return LIBSSH2_ERROR_EAGAIN;
        }
        if (len < 0)
        {
            char *emsg;
            int size;
            int ret = libssh2_session_last_error(m_sshClient->session(), &emsg, &size, 0);
            qCCritical(logxfer) << m_name << "Error" << ret << "libssh2_channel_write" << QString(emsg);
            return ret;
        }
        if (len == 0)
        {
            qCWarning(logxfer) << m_name << "ERROR:  libssh2_channel_write return 0";
            return 0;
        }
        /* xfer OK */

        m_total_TxToSsh += len;
        m_tx_start_ptr += len;
        transfered += len;
        DEBUGCH << "_transferTxToSsh: write on SSH return " << len << "bytes" ;

        if(m_tx_start_ptr == m_tx_stop_ptr)
        {
            DEBUGCH << "_transferTxToSsh: All buffer sent on SSH, buffer empty" ;
            m_tx_stop_ptr = nullptr;
            m_tx_start_ptr = nullptr;
        }
    }

    emit processed();
    return transfered;
}

size_t SshTunnelDataConnector::_rxBufferLen() const
{
    if(m_rx_stop_ptr == nullptr || m_rx_start_ptr == nullptr)
        return 0;

    if(m_rx_stop_ptr < m_rx_start_ptr)
    {
        qCWarning(logxfer) << m_name << "RX Buffer error";
        return 0;
    }

    return static_cast<size_t>(m_rx_stop_ptr - m_rx_start_ptr);
}

ssize_t SshTunnelDataConnector::_transferSshToRx()
{
    if(!m_sshChannel) return 0;

    if(_rxBufferLen() != 0)
    {
        qCDebug(logxfer) << "Buffer not empty, need to retry later";
        emit sendEvent();
        return 0;
    }

    ssize_t len = libssh2_channel_read(m_sshChannel, m_rx_buffer, BUFFER_SIZE);
    if(len == LIBSSH2_ERROR_EAGAIN)
        return 0;

    if (len < 0)
    {
        qCWarning(logxfer) << m_name << "_transferSshToRx: error: " << len;
        m_rx_stop_ptr = nullptr;
        m_rx_start_ptr = nullptr;

        char *emsg;
        int size;
        int ret = libssh2_session_last_error(m_sshClient->session(), &emsg, &size, 0);
        qCCritical(logxfer) << m_name << "Error" << ret << QString("libssh2_channel_read (%1 / %2)").arg(len).arg(BUFFER_SIZE) << QString(emsg);
        return 0;
    }

    m_total_SshToRx += len;

    if(len < BUFFER_SIZE)
    {
        DEBUGCH << "_transferSshToRx: Xfer " << len << "bytes";
        m_rx_data_on_ssh = false;
        if (libssh2_channel_eof(m_sshChannel))
        {
            m_rx_eof = true;
            DEBUGCH << "_transferSshToRx: Ssh channel closed";
        }
    }
    else
    {
        DEBUGCH << "_transferSshToRx: Xfer " << len << "bytes; There is probably more data to read, re-arm event";
        emit sendEvent();
    }
    m_rx_stop_ptr = m_rx_buffer + len;
    m_rx_start_ptr = m_rx_buffer;
    emit processed();
    return len;
}

ssize_t SshTunnelDataConnector::_transferRxToSock()
{
    if(!m_sock) return 0;
    ssize_t total = 0;

    /* If socket not ready, wait for socket connected */
    if(!m_sock || m_sock->state() != QAbstractSocket::ConnectedState)
    {
        qCDebug(logxfer) << m_name << "_transferRxToSock: Data on SSH when socket closed";
        return -1;
    }

    if(m_rx_stop_ptr == nullptr)
    {
        qCWarning(logxfer) <<  m_name << "Buffer empty";
        return 0;
    }

    DEBUGCH << "_transferRxToSock: Buffer contains " << _rxBufferLen() << "bytes";

    while (_rxBufferLen() > 0)
    {
        ssize_t slen = m_sock->write(m_rx_start_ptr, m_rx_stop_ptr - m_rx_start_ptr);
        if (slen <= 0)
        {
            qCWarning(logxfer) << "ERROR : " << m_name << " local failed to write (" << slen << ")";
            return slen;
        }

        m_rx_start_ptr += slen;
        total += slen;
        DEBUGCH << "_transferRxToSock: " << slen << "bytes written on socket";
    }

    /* Buffer is empty */
    m_rx_stop_ptr = nullptr;
    m_rx_start_ptr = nullptr;

    emit processed();
    return total;
}

void SshTunnelDataConnector::sshDataReceived()
{
    m_rx_data_on_ssh = true;
}

bool SshTunnelDataConnector::process()
{
    if(!m_rx_closed)
    {
        if(m_rx_data_on_ssh)
            _transferSshToRx();

        if(_rxBufferLen() || m_rx_eof)
            _transferRxToSock();
    }

    if(!m_tx_closed)
    {
        if(m_tx_data_on_sock)
            _transferSockToTx();

        if(_txBufferLen() || m_tx_eof)
            _transferTxToSsh();
    }



    if(!m_tx_closed && m_tx_eof && (m_sock->bytesAvailable() == 0) && (_txBufferLen() == 0))
    {
        DEBUGCH << "Send EOF to SSH";
        int ret = libssh2_channel_send_eof(m_sshChannel);
        if(ret == 0)
        {
            m_tx_closed = true;
            emit sendEvent();
        }
    }

    if(!m_rx_closed && m_rx_eof && (_rxBufferLen() == 0) && (m_sock->bytesAvailable() == 0) && (_txBufferLen() == 0))
    {
        if(m_sock->state() == QAbstractSocket::ConnectedState)
        {
            DEBUGCH << "_transferRxToSock: RX EOF, need to close ???";
            m_sock->disconnectFromHost();
        }
        else
        {
            DEBUGCH << "_transferRxToSock: RX EOF, RX closed";
            m_rx_closed = true;
            emit sendEvent();
        }
    }

    DEBUGCH << "XFer Tx: Sock->" << m_total_sockToTx << "->Tx->" <<  m_total_TxToSsh  << "->SSH" << ((m_tx_eof)?(" (EOF)"):("")) << ((m_tx_closed)?(" (CLOSED)"):(""));
    DEBUGCH << "XFer Rx: SSH->"  << m_total_SshToRx  << "->Rx->" <<  m_total_RxToSock << "->Sock(" << m_sock->state() << ")" << ((m_rx_eof)?(" (EOF)"):(""))<< ((m_rx_closed)?(" (CLOSED)"):(""));

    return (!m_rx_closed && !m_tx_closed);
}

void SshTunnelDataConnector::close()
{
    if(m_sock && m_sock->state() == QAbstractSocket::ConnectedState)
    {
        m_sock->disconnectFromHost();
    }
    else
    {
        m_rx_closed = true;
    }
}

bool SshTunnelDataConnector::isClosed()
{
    DEBUGCH << "SshTunnelDataConnector::isClosed(tx:" << m_tx_closed << ", rx:" << m_rx_closed << ")";
    return m_tx_closed && m_rx_closed && _rxBufferLen() == 0 && _txBufferLen() == 0;
}

void SshTunnelDataConnector::flushTx()
{
    DEBUGCH << "flushTx: start " << ": Sock: " << m_sock->bytesAvailable() << " | buffer:" << _txBufferLen();
    while(1)
    {
        if(m_sock->bytesAvailable() == 0 && _txBufferLen() == 0)
            break;

        if(m_tx_closed || m_rx_closed)
            break;

        if(m_sock->bytesAvailable() > 0)
        {
            _transferSockToTx();
        }

        if(_txBufferLen() > 0)
        {
            if(_transferTxToSsh() == LIBSSH2_ERROR_EAGAIN)
            {
                QEventLoop wait(this);
                QObject::connect(this, &SshTunnelDataConnector::processed, &wait, &QEventLoop::quit);
                wait.exec();
            }
        }
    }

    DEBUGCH << "flushTx: end " << ": Sock: " << m_sock->bytesAvailable() << " | buffer:" << _txBufferLen();
}
