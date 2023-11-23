#include "sshclient.h"
#include <QTemporaryFile>
#include <QDir>
#include <QEventLoop>
#include <QDateTime>
#include <QCoreApplication>
#include <QNetworkProxy>
#include "sshtunnelin.h"
#include "sshtunnelout.h"
#include "sshprocess.h"
#include "sshscpsend.h"
#include "sshscpget.h"
#include "sshsftp.h"
#include "cerrno"

Q_LOGGING_CATEGORY(sshclient, "ssh.client", QtWarningMsg)

#if !defined(MAX_LOST_KEEP_ALIVE)

/*
 * Maximum keep alive cycle (generally 5s) before connection is
 * considered as lost
 * Default: 6 (30s)
 */
#define MAX_LOST_KEEP_ALIVE 6
#endif

int SshClient::s_nbInstance = 0;

static ssize_t qt_callback_libssh_recv(int socket,void *buffer, size_t length,int flags, void **abstract)
{
    Q_UNUSED(socket)
    Q_UNUSED(flags)

    QTcpSocket * c = reinterpret_cast<QTcpSocket *>(* abstract);
    qint64 r = c->read(reinterpret_cast<char *>(buffer), static_cast<qint64>(length));
    if (r == 0)
    {
        return -EAGAIN;
    }
    return static_cast<ssize_t>(r);
}

static ssize_t qt_callback_libssh_send(int socket,const void * buffer, size_t length,int flags, void ** abstract)
{
    Q_UNUSED(socket)
    Q_UNUSED(flags)

    QTcpSocket * c = reinterpret_cast<QTcpSocket *>(* abstract);
    qint64 r = c->write(reinterpret_cast<const char *>(buffer), static_cast<qint64>(length));
    if (r == 0)
    {
        return -EAGAIN;
    }
    return static_cast<ssize_t>(r);
}

void SshClient::setProxy(QNetworkProxy *proxy)
{
    m_proxy = proxy;
}

void SshClient::setConnectTimeout(int timeoutMsec)
{
    m_connTimeoutCnt = timeoutMsec;
}

SshClient::SshClient(const QString &name, QObject * parent):
    QObject(parent),
    m_name(name),
    m_socket(this)
{
    /* New implementation */
    QObject::connect(this, &SshClient::sshEvent, this, &SshClient::_ssh_processEvent, Qt::QueuedConnection);
    QObject::connect(&m_socket, &QTcpSocket::connected,      this, &SshClient::_connection_socketConnected);
    QObject::connect(&m_socket, &QTcpSocket::disconnected,   this, &SshClient::_connection_socketDisconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    QObject::connect(&m_socket, &QAbstractSocket::errorOccurred,
                                                             this, &SshClient::_connection_socketError);
#else
    QObject::connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                                                             this, &SshClient::_connection_socketError);
#endif
    QObject::connect(&m_socket, &QTcpSocket::readyRead,      this, &SshClient::_ssh_processEvent, Qt::QueuedConnection);
    QObject::connect(&m_connectionTimeout, &QTimer::timeout, this, &SshClient::_connection_socketTimeout);
    QObject::connect(&m_keepalive,&QTimer::timeout,          this, &SshClient::_sendKeepAlive);

    if(s_nbInstance == 0)
    {
        qCDebug(sshclient) << m_name << ": libssh2_init()";
        Q_ASSERT(libssh2_init(0) == 0);
    }
    ++s_nbInstance;

    qCDebug(sshclient) << m_name << ": created " << this;
}

SshClient::~SshClient()
{
    qCDebug(sshclient) << m_name << ": SshClient::~SshClient() " << this;
    disconnectFromHost();
    waitForState(SshClient::SshState::Unconnected);
    --s_nbInstance;
    if(s_nbInstance == 0)
    {
        qCDebug(sshclient) << m_name << ": libssh2_exit()";
        libssh2_exit();
    }
    qCDebug(sshclient) << m_name << ": destroyed";
}

QString SshClient::getName() const
{
    return m_name;
}

bool SshClient::takeChannelCreationMutex(void *identifier)
{
    if ( ! channelCreationInProgress.tryLock() && currentLockerForChannelCreation != identifier )
    {
        qCDebug(sshclient) << "takeChannelCreationMutex another channel is already been created, have to wait";
        return false;
    }
    currentLockerForChannelCreation = identifier;
    return true;
}

void SshClient::releaseChannelCreationMutex(void *identifier)
{
    if ( currentLockerForChannelCreation == identifier )
    {
        channelCreationInProgress.unlock();
        currentLockerForChannelCreation = nullptr;
    }
    else
    {
        qCCritical(sshclient) << "Trying to release channel mutex but it doesn't host it";
    }
}

LIBSSH2_SESSION *SshClient::session()
{
    return m_session;
}

int SshClient::connectToHost(const QString &user, const QString &host, quint16 port, QByteArrayList methodes, int connTimeoutMsec)
{
    if(sshState() != SshState::Unconnected && sshState() != SshState::Error)
    {
        qCCritical(sshclient) << m_name << "Allready connected";
        return 0;
    }
    qCDebug(sshclient) << m_name << "connectToHost:" << user << "@" << host << ":" << port;

    m_connTimeoutCnt = connTimeoutMsec;
    m_authenticationMethodes = methodes;
    m_hostname = host;
    m_port = port;
    m_username = user;

    setSshState(SshState::SocketConnection);
    emit sshEvent();
    return 0;
}

bool SshClient::waitForState(SshState state)
{
    if(sshState() == state) return true;
    QEventLoop wait(this);
    QObject::connect(this, &SshClient::sshStateChanged, &wait, &QEventLoop::quit);
    qCDebug(sshclient) << m_name << "Wait for state " << state << ", current state is " << m_sshState;
    while(sshState() != SshState::Error && sshState() != state)
    {
        wait.exec();
    }
    return sshState() == state;
}


void SshClient::disconnectFromHost()
{
    qCDebug(sshclient) << m_name << ": disconnectFromHost(): state is " << m_sshState << " and channel size is " << m_channels.size();
    if(m_sshState == SshState::Unconnected)
        return;

    if(m_sshState == SshState::Error)
        return;

    if(m_channels.size() == 0)
    {
        setSshState(DisconnectingSession);
    }
    else
    {
        setSshState(DisconnectingChannel);
    }

    emit sshEvent();
    return;
}

void SshClient::resetError()
{
    waitForState(Unconnected);
    if(sshState() == Error)
        setSshState(Unconnected);
}


void SshClient::setPassphrase(const QString & pass)
{
    m_passphrase = pass;
}

void SshClient::setKeys(const QString &publicKey, const QString &privateKey)
{
    m_publicKey  = publicKey;
    m_privateKey = privateKey;
}

bool SshClient::saveKnownHosts(const QString & file)
{
    bool res = (libssh2_knownhost_writefile(m_knownHosts, qPrintable(file), LIBSSH2_KNOWNHOST_FILE_OPENSSH) == 0);
    return res;
}

void SshClient::setKownHostFile(const QString &file)
{
    m_knowhostFiles = file;
}

bool SshClient::addKnownHost(const QString & hostname,const SshKey & key)
{
    bool ret;
    int typemask = LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW;
    switch (key.type)
    {
        case SshKey::Dss:
            typemask |= LIBSSH2_KNOWNHOST_KEY_SSHDSS;
            break;
        case SshKey::Rsa:
            typemask |= LIBSSH2_KNOWNHOST_KEY_SSHRSA;
            break;
        case SshKey::UnknownType:
            return false;
    }
    ret = (libssh2_knownhost_add(m_knownHosts, qPrintable(hostname), nullptr, key.key.data(), static_cast<size_t>(key.key.size()), typemask, nullptr));
    return ret;
}

QString SshClient::banner()
{
    return QString(libssh2_session_banner_get(m_session));
}

void SshClient::_sendKeepAlive()
{
    int keepalive = 0;
    if(m_session)
    {
        int ret = libssh2_keepalive_send(m_session, &keepalive);
        if(ret == LIBSSH2_ERROR_SOCKET_SEND)
        {
            qCWarning(sshclient) << m_name << ": Connection I/O error !!!";
            m_socket.disconnectFromHost();
        }
        else if(((QDateTime::currentMSecsSinceEpoch() - m_lastProofOfLive) / 1000) > (MAX_LOST_KEEP_ALIVE * keepalive))
        {
            qCWarning(sshclient) << m_name << ": Connection lost !!!";
            setSshState(SshState::Error);
            m_socket.disconnectFromHost();
        }
        else
        {
            keepalive -= 1;
            if(keepalive < 2) keepalive = 2;
            m_keepalive.start(keepalive * 1000);
        }
    }
}


SshClient::SshState SshClient::sshState() const
{
    return m_sshState;
}

void SshClient::setSshState(const SshState &sshState)
{
    if(m_sshState != sshState)
    {
        qCDebug(sshclient) << m_name << ": Change state " <<  m_sshState << " to " << sshState;
        m_sshState = sshState;
        emit sshStateChanged(m_sshState);
    }
}


void SshClient::setName(const QString &name)
{
    m_name = name;
}

/* New implementation */
void SshClient::_connection_socketTimeout()
{
    m_connectionTimeout.stop();
    m_socket.disconnectFromHost();
    qCWarning(sshclient) << m_name << ": ssh socket connection timeout";
    setSshState(SshState::Error);
    emit sshEvent();
}

void SshClient::_connection_socketError()
{
    qCWarning(sshclient) << m_name << ": ssh socket connection error:" << m_sshState;
    setSshState(SshState::Error);
    emit sshEvent();
}

void SshClient::_connection_socketConnected()
{
    qCDebug(sshclient) << m_name << ": ssh socket connected";

    if(m_sshState == SshState::WaitingSocketConnection)
    {
        /* Normal process; socket is connected */
        setSshState(SshState::Initialize);
        emit sshEvent();
    }
    else
    {
        qCWarning(sshclient) << m_name << ": Unknown conenction on socket";
        setSshState(SshState::Error);
        emit sshEvent();
    }
}

void SshClient::_connection_socketDisconnected()
{
    qCDebug(sshclient) << m_name << ": ssh socket disconnected";
    if(sshState() != Error)
    {
        setSshState(FreeSession);
    }
    emit sshEvent();
}

void SshClient::_ssh_processEvent()
{
    switch(m_sshState)
    {
        case SshState::Unconnected:
        {
            return;
        }

        case SshState::SocketConnection:
        {
            m_connectionTimeout.start(m_connTimeoutCnt);
            if(m_proxy)
            {
                m_socket.setProxy(*m_proxy);
            }
            else
            {
                m_socket.setProxy(QNetworkProxy::NoProxy);
            }
            m_socket.connectToHost(m_hostname, m_port);
            setSshState(SshState::WaitingSocketConnection);
        }

        FALLTHROUGH; case SshState::WaitingSocketConnection:
        {
            return;
        }

        case SshState::Initialize:
        {
            m_session = libssh2_session_init_ex(nullptr, nullptr, nullptr, reinterpret_cast<void *>(&m_socket));
            if(m_session == nullptr)
            {
                qCCritical(sshclient) << m_name << ": libssh error during session init";
                setSshState(SshState::Error);
                m_socket.disconnectFromHost();
                return;
            }

            libssh2_session_callback_set(m_session, LIBSSH2_CALLBACK_RECV,reinterpret_cast<void*>(& qt_callback_libssh_recv));
            libssh2_session_callback_set(m_session, LIBSSH2_CALLBACK_SEND,reinterpret_cast<void*>(& qt_callback_libssh_send));
            libssh2_session_set_blocking(m_session, 0);

            m_knownHosts = libssh2_knownhost_init(m_session);
            Q_ASSERT(m_knownHosts);

            if(m_knowhostFiles.size())
            {
                libssh2_knownhost_readfile(m_knownHosts, qPrintable(m_knowhostFiles), LIBSSH2_KNOWNHOST_FILE_OPENSSH);
            }

            setSshState(SshState::HandShake);
        }

        FALLTHROUGH; case SshState::HandShake:
        {
            int ret = libssh2_session_handshake(m_session, static_cast<int>(m_socket.socketDescriptor()));
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(ret != 0)
            {
                qCCritical(sshclient) << m_name << "Handshake error" << sshErrorToString(ret);
                setSshState(SshState::Error);
                m_socket.disconnectFromHost();
                return;
            }

            /* HandShake success, continue autentication */
            size_t len;
            int type;
            const char * fingerprint = libssh2_session_hostkey(m_session, &len, &type);
            if(fingerprint == nullptr)
            {
                qCCritical(sshclient) << m_name << "Fingerprint error";
                setSshState(SshState::Error);
                m_socket.disconnectFromHost();
                return;
            }

            m_hostKey.hash = QByteArray(libssh2_hostkey_hash(m_session,LIBSSH2_HOSTKEY_HASH_MD5), 16);
            switch (type)
            {
                case LIBSSH2_HOSTKEY_TYPE_RSA:
                    m_hostKey.type=SshKey::Rsa;
                    break;
                case LIBSSH2_HOSTKEY_TYPE_DSS:
                    m_hostKey.type=SshKey::Dss;
                    break;
                default:
                    m_hostKey.type=SshKey::UnknownType;
            }

            m_hostKey.key = QByteArray(fingerprint, static_cast<int>(len));
            struct libssh2_knownhost *khost;
            libssh2_knownhost_check(m_knownHosts, m_hostname.toStdString().c_str(), fingerprint, len, LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW, &khost);
            setSshState(SshState::GetAuthenticationMethodes);
        }

        FALLTHROUGH; case SshState::GetAuthenticationMethodes:
        {
            if(m_authenticationMethodes.length() == 0)
            {
                QByteArray username = m_username.toLocal8Bit();
                char * alist = nullptr;

                alist = libssh2_userauth_list(m_session, username.data(), static_cast<unsigned int>(username.length()));
                if(alist == nullptr)
                {
                    int ret = libssh2_session_last_error(m_session, nullptr, nullptr, 0);
                    if(ret == LIBSSH2_ERROR_EAGAIN)
                    {
                        return;
                    }
                    setSshState(SshState::Error);
                    m_socket.disconnectFromHost();
                    qCDebug(sshclient) << m_name << ": Failed to authenticate:" << sshErrorToString(ret);
                    return;
                }
                qCDebug(sshclient) << m_name << ": ssh start authentication userauth_list: " << alist;

                m_authenticationMethodes = QByteArray(alist).split(',');
            }
            setSshState(SshState::Authentication);
        }

        FALLTHROUGH; case SshState::Authentication:
        {
            while(m_authenticationMethodes.length() != 0)
            {
                if(m_authenticationMethodes.first() == "publickey")
                {
                    int ret = libssh2_userauth_publickey_frommemory(
                                    m_session,
                                    m_username.toStdString().c_str(),
                                    static_cast<size_t>(m_username.length()),
                                    m_publicKey.toStdString().c_str(),
                                    static_cast<size_t>(m_publicKey.length()),
                                    m_privateKey.toStdString().c_str(),
                                    static_cast<size_t>(m_privateKey.length()),
                                    m_passphrase.toStdString().c_str()
                            );
                    if(ret == LIBSSH2_ERROR_EAGAIN)
                    {
                        return;
                    }
                    if(ret < 0)
                    {
                        qCWarning(sshclient) << m_name << ": Authentication with publickey failed:" << sshErrorToString(ret);
                        m_authenticationMethodes.removeFirst();
                    }
                    if(ret == 0)
                    {
                        qCDebug(sshclient) << m_name << ": Authenticated with publickey";
                        setSshState(SshState::Ready);
                        break;
                    }
                }

                if(m_authenticationMethodes.first() == "password")
                {
                    QByteArray username = m_username.toLatin1();
                    QByteArray passphrase = m_passphrase.toLatin1();

                    int ret = libssh2_userauth_password_ex(m_session,
                                                             username.data(),
                                                             static_cast<unsigned int>(username.length()),
                                                             passphrase.data(),
                                                             static_cast<unsigned int>(passphrase.length()), nullptr);
                    if(ret == LIBSSH2_ERROR_EAGAIN)
                    {
                        return;
                    }
                    if(ret < 0)
                    {
                        qCWarning(sshclient) << m_name << ": Authentication with password failed";
                        m_authenticationMethodes.removeFirst();
                        return;
                    }
                    if(ret == 0)
                    {
                        qCDebug(sshclient) << m_name << ": Authenticated with password";
                        emit channelsChanged(m_channels.count());
                        setSshState(SshState::Ready);
                        break;
                    }
                }

                m_authenticationMethodes.pop_front();
            }
            if(libssh2_userauth_authenticated(m_session))
            {
                qCDebug(sshclient) << m_name << ": Connected and authenticated";
                m_connectionTimeout.stop();
                m_keepalive.setSingleShot(true);
                m_keepalive.start(1000);
                libssh2_keepalive_config(m_session, 1, 5);
                setSshState(SshState::Ready);
                emit sshReady();
            }
            else
            {
                qCWarning(sshclient) << m_name << ": Authentication failed";
                m_socket.disconnectFromHost();
                setSshState(SshState::Error);
                return;
            }
            FALLTHROUGH;
        }

        case SshState::Ready:
        {
            m_lastProofOfLive = QDateTime::currentMSecsSinceEpoch();
            emit sshDataReceived();
            return;
        }

        case SshState::DisconnectingChannel:
        {
            /* Close all Opened Channels */
            if(m_channels.size() > 0)
            {
                qCDebug(sshclient) << m_name << ": DisconnectingChannel, there is still " << m_channels.size() << "connections:";
                for(SshChannel* ch: m_channels)
                {
                    qCDebug(sshclient) << m_name << "\t" << ch->name();
                    ch->close();
                }
            }
            else
            {
                setSshState(DisconnectingSession);
            }
            break;
        }

        case SshState::DisconnectingSession:
        {
            int ret = libssh2_session_disconnect_ex(m_session, SSH_DISCONNECT_BY_APPLICATION, "good bye!", "");
            if(ret == LIBSSH2_ERROR_EAGAIN)
            {
                return;
            }
            if(m_socket.state() == QAbstractSocket::ConnectedState)
            {
                qCDebug(sshclient) << m_name << ": Ask for main socket disconnection";
                m_socket.disconnectFromHost();
                return;
            }
            else
            {
                qCDebug(sshclient) << m_name << ": Socket state is " << m_socket.state();
                if(m_socket.state() == QAbstractSocket::UnconnectedState)
                {
                    qCDebug(sshclient) << m_name << ": Socket state is " << m_socket.state();
                    setSshState(FreeSession);
                }
                else
                {
                    return;
                }
            }
        }

        FALLTHROUGH; case SshState::FreeSession:
        {
            m_keepalive.stop();
            if (m_knownHosts)
            {
                libssh2_knownhost_free(m_knownHosts);
                m_knownHosts = nullptr;
            }

            if(m_session)
            {
                int ret = libssh2_session_free(m_session);
                if(ret == LIBSSH2_ERROR_EAGAIN)
                {
                    emit sshEvent();
                    return;
                }
            }

            m_session = nullptr;
            setSshState(Unconnected);
            emit sshDisconnected();
            break;
        }

        case SshState::Error:
        {
            m_keepalive.stop();
            if(m_socket.state() != QAbstractSocket::UnconnectedState)
            {
                m_socket.disconnectFromHost();
                m_socket.waitForDisconnected(3000);
            }
            qCWarning(sshclient) << m_name << ": ssh socket connection error";
            emit sshError();
            return;
        }
    }
}

void SshClient::_channel_free()
{
    QObject *obj = QObject::sender();
    SshChannel *connection = qobject_cast<SshChannel*>(obj);
    if(connection)
    {
        if(connection->channelState() == SshChannel::ChannelState::Free)
        {
            qCDebug(sshclient) << "Channel " << connection->name() << " is FREE";
            m_channels.removeAll(connection);
            connection->deleteLater();
            emit channelsChanged(m_channels.count());

            if(sshState() == SshState::DisconnectingChannel && m_channels.size() == 0)
            {

                qCDebug(sshclient) << m_name << ": no more channel registered";

                /* Stop keepalive */
                m_keepalive.stop();

                setSshState(SshState::DisconnectingSession);
            }
            emit sshEvent();
        }
    }
}
