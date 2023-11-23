#pragma once

#include <QObject>
#include <QList>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include "sshchannel.h"
#include "sshkey.h"
#include <QSharedPointer>

#ifndef FALLTHROUGH
#if __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif
#endif

Q_DECLARE_LOGGING_CATEGORY(sshclient)
class SshScpGet;
class SshScpSend;
class SshSFtp;
class SshTunnelIn;
class SshTunnelOut;
class QNetworkProxy;

class  SshClient : public QObject {
    Q_OBJECT

public:
    enum SshState {
        Unconnected,
        SocketConnection,
        WaitingSocketConnection,
        Initialize,
        HandShake,
        GetAuthenticationMethodes,
        Authentication,
        Ready,
        DisconnectingChannel,
        DisconnectingSession,
        FreeSession,
        Error,
    };
    Q_ENUM(SshState)

private:
    static int s_nbInstance;
    LIBSSH2_SESSION    * m_session {nullptr};
    LIBSSH2_KNOWNHOSTS * m_knownHosts {nullptr};
    QList<SshChannel*> m_channels;

    QString m_name;
    QTcpSocket m_socket;
    QNetworkProxy *m_proxy {nullptr};
    qint64 m_lastProofOfLive {0};

    quint16 m_port {0};
    int m_errorcode {0};
    bool m_sshConnected {false};
    int m_connTimeoutCnt;

    QString m_hostname;
    QString m_username;
    QString m_passphrase;
    QString m_privateKey;
    QString m_publicKey;
    QString m_errorMessage;
    QString m_knowhostFiles;
    SshKey  m_hostKey;
    QTimer m_keepalive;
    QTimer m_connectionTimeout;
    QMutex channelCreationInProgress;
    void *currentLockerForChannelCreation {nullptr};

public:
    SshClient(const QString &name = "noname", QObject * parent = nullptr);
    virtual ~SshClient();

    QString getName() const;
    bool takeChannelCreationMutex(void *identifier);
    void releaseChannelCreationMutex(void *identifier);



public slots:
    int connectToHost(const QString &username, const QString &hostname, quint16 port = 22, QByteArrayList methodes = QByteArrayList(), int connTimeoutMsec = 60000);
    bool waitForState(SshClient::SshState state);
    void disconnectFromHost();
    void resetError();

public:
    template<typename T>
    T *getChannel(const QString &name)
    {
        for(SshChannel *ch: m_channels)
        {
            if(ch->name() == name)
            {
                T *proc = qobject_cast<T*>(ch);
                if(proc)
                {
                    return proc;
                }
            }
        }

        T *res = new T(name, this);
        m_channels.append(res);
        QObject::connect(res, &SshChannel::stateChanged, this, &SshClient::_channel_free);
        emit channelsChanged(m_channels.count());
        return res;
    }

    void setKeys(const QString &publicKey, const QString &privateKey);
    void setPassphrase(const QString & pass);
    bool saveKnownHosts(const QString &file);
    void setKownHostFile(const QString &file);
    bool addKnownHost  (const QString &hostname, const SshKey &key);
    QString banner();


    LIBSSH2_SESSION *session();



private slots:
    void _sendKeepAlive();


public: /* New function implementation with state machine */
    SshState sshState() const;

    void setName(const QString &name);

    void setProxy(QNetworkProxy *proxy);

    void setConnectTimeout(int timeoutMsec);

private: /* New function implementation with state machine */
    SshState m_sshState {SshState::Unconnected};
    QByteArrayList m_authenticationMethodes;
    void setSshState(const SshState &sshState);


private slots: /* New function implementation with state machine */
    void _connection_socketTimeout();
    void _connection_socketError();
    void _connection_socketConnected();
    void _connection_socketDisconnected();
    void _ssh_processEvent();
    void _channel_free();

signals:
    void sshStateChanged(SshState sshState);
    void sshReady();
    void sshDisconnected();
    void sshError();

    void sshDataReceived();
    void sshEvent();
    void channelsChanged(int);
};

inline const char* sshErrorToString(int err)
{
    switch(err)
    {
    case LIBSSH2_ERROR_SOCKET_NONE:
        return "LIBSSH2_ERROR_SOCKET_NONE";
    case LIBSSH2_ERROR_BANNER_RECV:
        return "LIBSSH2_ERROR_BANNER_RECV";
    case LIBSSH2_ERROR_BANNER_SEND:
        return "LIBSSH2_ERROR_BANNER_SEND";
    case LIBSSH2_ERROR_INVALID_MAC:
        return "LIBSSH2_ERROR_INVALID_MAC";
    case LIBSSH2_ERROR_KEX_FAILURE:
        return "LIBSSH2_ERROR_KEX_FAILURE";
    case LIBSSH2_ERROR_ALLOC:
        return "LIBSSH2_ERROR_ALLOC";
    case LIBSSH2_ERROR_SOCKET_SEND:
        return "LIBSSH2_ERROR_SOCKET_SEND";
    case LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE:
        return "LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE";
    case LIBSSH2_ERROR_TIMEOUT:
        return "LIBSSH2_ERROR_TIMEOUT";
    case LIBSSH2_ERROR_HOSTKEY_INIT:
        return "LIBSSH2_ERROR_HOSTKEY_INIT";
    case LIBSSH2_ERROR_HOSTKEY_SIGN:
        return "LIBSSH2_ERROR_HOSTKEY_SIGN";
    case LIBSSH2_ERROR_DECRYPT:
        return "LIBSSH2_ERROR_DECRYPT";
    case LIBSSH2_ERROR_SOCKET_DISCONNECT:
        return "LIBSSH2_ERROR_SOCKET_DISCONNECT";
    case LIBSSH2_ERROR_PROTO:
        return "LIBSSH2_ERROR_PROTO";
    case LIBSSH2_ERROR_PASSWORD_EXPIRED:
        return "LIBSSH2_ERROR_PASSWORD_EXPIRED";
    case LIBSSH2_ERROR_FILE:
        return "LIBSSH2_ERROR_FILE";
    case LIBSSH2_ERROR_METHOD_NONE:
        return "LIBSSH2_ERROR_METHOD_NONE";
    case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
        return "LIBSSH2_ERROR_AUTHENTICATION_FAILED";
    case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED:
        return "LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED";
    case LIBSSH2_ERROR_CHANNEL_OUTOFORDER:
        return "LIBSSH2_ERROR_CHANNEL_OUTOFORDER";
    case LIBSSH2_ERROR_CHANNEL_FAILURE:
        return "LIBSSH2_ERROR_CHANNEL_FAILURE";
    case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
        return "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED";
    case LIBSSH2_ERROR_CHANNEL_UNKNOWN:
        return "LIBSSH2_ERROR_CHANNEL_UNKNOWN";
    case LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED:
        return "LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED";
    case LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED:
        return "LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED";
    case LIBSSH2_ERROR_CHANNEL_CLOSED:
        return "LIBSSH2_ERROR_CHANNEL_CLOSED";
    case LIBSSH2_ERROR_CHANNEL_EOF_SENT:
        return "LIBSSH2_ERROR_CHANNEL_EOF_SENT";
    case LIBSSH2_ERROR_SCP_PROTOCOL:
        return "LIBSSH2_ERROR_SCP_PROTOCOL";
    case LIBSSH2_ERROR_ZLIB:
        return "LIBSSH2_ERROR_ZLIB";
    case LIBSSH2_ERROR_SOCKET_TIMEOUT:
        return "LIBSSH2_ERROR_SOCKET_TIMEOUT";
    case LIBSSH2_ERROR_SFTP_PROTOCOL:
        return "LIBSSH2_ERROR_SFTP_PROTOCOL";
    case LIBSSH2_ERROR_REQUEST_DENIED:
        return "LIBSSH2_ERROR_REQUEST_DENIED";
    case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED:
        return "LIBSSH2_ERROR_METHOD_NOT_SUPPORTED";
    case LIBSSH2_ERROR_INVAL:
        return "LIBSSH2_ERROR_INVAL";
    case LIBSSH2_ERROR_INVALID_POLL_TYPE:
        return "LIBSSH2_ERROR_INVALID_POLL_TYPE";
    case LIBSSH2_ERROR_PUBLICKEY_PROTOCOL:
        return "LIBSSH2_ERROR_PUBLICKEY_PROTOCOL";
    case LIBSSH2_ERROR_EAGAIN:
        return "LIBSSH2_ERROR_EAGAIN";
    case LIBSSH2_ERROR_BUFFER_TOO_SMALL:
        return "LIBSSH2_ERROR_BUFFER_TOO_SMALL";
    case LIBSSH2_ERROR_BAD_USE:
        return "LIBSSH2_ERROR_BAD_USE";
    case LIBSSH2_ERROR_COMPRESS:
        return "LIBSSH2_ERROR_COMPRESS";
    case LIBSSH2_ERROR_OUT_OF_BOUNDARY:
        return "LIBSSH2_ERROR_OUT_OF_BOUNDARY";
    case LIBSSH2_ERROR_AGENT_PROTOCOL:
        return "LIBSSH2_ERROR_AGENT_PROTOCOL";
    case LIBSSH2_ERROR_SOCKET_RECV:
        return "LIBSSH2_ERROR_SOCKET_RECV";
    case LIBSSH2_ERROR_ENCRYPT:
        return "LIBSSH2_ERROR_ENCRYPT";
    case LIBSSH2_ERROR_BAD_SOCKET:
        return "LIBSSH2_ERROR_BAD_SOCKET";
    case LIBSSH2_ERROR_KNOWN_HOSTS:
        return "LIBSSH2_ERROR_KNOWN_HOSTS";
    default:
        return "Unknown LIBSSH2 ERROR";
    }
}
