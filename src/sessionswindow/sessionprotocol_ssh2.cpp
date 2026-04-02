/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"
#include <QMessageBox>
#include <QFileInfo>

#include "sessionswindow.h"

#ifdef ENABLE_SSH
#include "sshshell.h"
#endif

namespace sessionprotocol {
class SSH2Protocol final : public SessionProtocolBase {
public:
#ifdef ENABLE_SSH
    struct StartArgs {
        QString hostname;
        quint16 port = 0;
        QString userName;
        QString password;
        int sshAuthType = SessionsWindow::SshAuthPassword;
        QString privateKeyPath;
        QString publicKeyPath;
        QString passphrase;
    };
#endif

    SessionsWindow::SessionType type() const override { return SessionsWindow::SSH2; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::ConsoleSession; }
    void initialize(SessionsWindow *session) override {
#ifdef ENABLE_SSH
        ssh2Client = new SshClient("ssh2", session);
        session->realtimespeed_timer = new QTimer(session);

        QObject::connect(ssh2Client, &SshClient::sshReady, session, [=](){
            SshShell *shell = ssh2Client->getChannel<SshShell>("quardCRT.shell");
            if(shell == nullptr) {
                session->reportSessionError(session->tr("SSH2 Error"), session->tr("SSH2 error:\n%1.").arg(ssh2Client->sshErrorString()));
                this->installReconnectOnEnterRequest(session);
                return;
            }
            shell->initSize(session->term->columns(), session->term->lines());
            ssh2Client->startChannel<SshShell>(shell);
            QObject::connect(session->term, &QTermWidget::termSizeChange, session, [=](int lines, int columns){
                shell->resize(columns, lines);
            });
            QObject::connect(shell, &SshShell::readyRead, session, [=](const char *data, int size){
                session->forwardReceivedData(QByteArray(data, size), true);
            });
            session->setupModemProxyForward([=]() {
                return session->state == SessionsWindow::Connected;
            }, [=](const QByteArray &data) {
                shell->sendData(data.data(), data.size());
            });
            session->setupTerminalSendForward([=]() {
                return session->state == SessionsWindow::Connected;
            }, [=](const char *data, int size) {
                shell->sendData(data, size);
            }, true);
            QObject::connect(shell, &SshShell::failed, session, [=](){
                session->disconnect();
            });
            QObject::connect(shell, &SshShell::finished, session, [=](){
                session->disconnect();
            });
            session->updateConnectionState(true);
        });
        QObject::connect(ssh2Client, &SshClient::sshDisconnected, session, [=](){
            session->updateConnectionState(false);
        });
        QObject::connect(ssh2Client, &SshClient::sshError, session, [=](){
            session->reportSessionError(session->tr("SSH2 Error"), session->tr("SSH2 error:\n%1.").arg(ssh2Client->sshErrorString()));
            this->installReconnectOnEnterRequest(session);
        });
#else
        Q_UNUSED(session);
#endif
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        Q_UNUSED(profile);
#ifdef ENABLE_SSH
        const StartArgs args = parseStartArgs(commonMeta, protocolMeta);
        target->startSSH2Session(args.hostname,
                     args.port,
                     args.userName,
                     args.password,
                     args.sshAuthType,
                     args.privateKeyPath,
                     args.publicKeyPath,
                     args.passphrase);
#else
        Q_UNUSED(target);
        Q_UNUSED(commonMeta);
        Q_UNUSED(protocolMeta);
#endif
    }
    void disconnect(SessionsWindow *session) override {
#ifdef ENABLE_SSH
        Q_UNUSED(session);
        if(ssh2Client) {
            ssh2Client->disconnectFromHost();
        }
#else
        Q_UNUSED(session);
#endif
    }
    void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) override {
        info.ssh2.tx_total = session->tx_total;
        info.ssh2.rx_total = session->rx_total;
        info.ssh2.tx_speed = session->tx_speed;
        info.ssh2.rx_speed = session->rx_speed;
#ifdef ENABLE_SSH
    Q_UNUSED(session);
    if(ssh2Client) {
        info.ssh2.encryption_method = ssh2Client->getEncryptionMethod();
    }
#endif
    }
    bool hasChildProcess(SessionsWindow *session) override {
        Q_UNUSED(session);
        return false;
    }
    void refreshTermSize(SessionsWindow *session) override {
        Q_UNUSED(session);
    }
    void cleanup(SessionsWindow *session) override {
#ifdef ENABLE_SSH
        Q_UNUSED(session);
        if(ssh2Client) {
            ssh2Client->disconnectFromHost();
            delete ssh2Client;
            ssh2Client = nullptr;
        }
#else
        Q_UNUSED(session);
#endif
    }
#ifdef ENABLE_SSH
    SshSFtp *getSshSFtpChannel(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(!ssh2Client) {
            return nullptr;
        }
        SshSFtp *res = ssh2Client->getChannel<SshSFtp>("quardCRT.sftp");
        ssh2Client->startChannel<SshSFtp>(res);
        return res;
    }
    int startSSH2Session(SessionsWindow *session, const QString &hostname, quint16 port, const QString &username,
                         const QString &password, int authType, const QString &privateKeyPath,
                         const QString &publicKeyPath, const QString &passphrase) {
        if(!ssh2Client) {
            return -1;
        }
        userNameValue = username;
        passwordValue = password;
        passphraseValue = passphrase;
        privateKeyPathValue = privateKeyPath;
        publicKeyPathValue = publicKeyPath;
        sshAuthTypeValue = authType;
        QByteArrayList methodes;
        if(authType == SessionsWindow::SshAuthPublicKey) {
            if(privateKeyPath.isEmpty()) {
                QMessageBox::warning(session->messageParentWidget, session->tr("SSH2 Error"), session->tr("Private key is required for public key authentication."));
                return -1;
            }
            methodes.append("publickey");
            QString resolvedPublicKeyPath = publicKeyPath;
            if(resolvedPublicKeyPath.isEmpty()) {
                QString candidate = privateKeyPath + ".pub";
                if(QFileInfo::exists(candidate)) {
                    resolvedPublicKeyPath = candidate;
                }
            }
            ssh2Client->setKeyFiles(resolvedPublicKeyPath, privateKeyPath);
            ssh2Client->setPassphrase(passphrase);
        } else {
            methodes.append("password");
            ssh2Client->setKeyFiles(QString(), QString());
            ssh2Client->setPassphrase(password);
        }
        ssh2Client->connectToHost(username, hostname, port, methodes);
        return 0;
    }
    QVariantMap exportMeta() const override {
        return {
            {"userName", userNameValue},
            {"password", passwordValue},
            {"passphrase", passphraseValue},
            {"privateKeyPath", privateKeyPathValue},
            {"publicKeyPath", publicKeyPathValue},
            {"sshAuthType", sshAuthTypeValue}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        userNameValue = meta.value("userName").toString();
        passwordValue = meta.value("password").toString();
        passphraseValue = meta.value("passphrase").toString();
        privateKeyPathValue = meta.value("privateKeyPath").toString();
        publicKeyPathValue = meta.value("publicKeyPath").toString();
        sshAuthTypeValue = meta.value("sshAuthType", SessionsWindow::SshAuthPassword).toInt();
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        const StartArgs args = parseStartArgs(commonMeta, protocolMeta);
        return startSSH2Session(session,
                                args.hostname,
                                args.port,
                                args.userName,
                                args.password,
                                args.sshAuthType,
                                args.privateKeyPath,
                                args.publicKeyPath,
                                args.passphrase);
    }
#endif

private:
#ifdef ENABLE_SSH
    static StartArgs parseStartArgs(const QVariantMap &commonMeta, const QVariantMap &protocolMeta) {
        StartArgs args;
        args.hostname = commonMeta.value("hostname").toString();
        args.port = static_cast<quint16>(commonMeta.value("port").toUInt());
        args.userName = protocolMeta.value("userName").toString();
        args.password = protocolMeta.value("password").toString();
        args.sshAuthType = protocolMeta.value("sshAuthType", SessionsWindow::SshAuthPassword).toInt();
        args.privateKeyPath = protocolMeta.value("privateKeyPath").toString();
        args.publicKeyPath = protocolMeta.value("publicKeyPath").toString();
        args.passphrase = protocolMeta.value("passphrase").toString();
        return args;
    }
#endif

    void installReconnectOnEnterRequest(SessionsWindow *session) {
        QObject::connect(session->term, &QTermWidget::sendData, session, [=](const char *data, int size) {
            QByteArray sendData(data, size);
            if(sendData.contains("\r") || sendData.contains("\n")) {
                if(!session->m_requestReconnect) {
                    session->m_requestReconnect = true;
                    emit session->requestReconnect();
                }
            }
        });
    }
#ifdef ENABLE_SSH
    SshClient *ssh2Client = nullptr;
    QString userNameValue;
    QString passwordValue;
    QString passphraseValue;
    QString privateKeyPathValue;
    QString publicKeyPathValue;
    int sshAuthTypeValue = SessionsWindow::SshAuthPassword;
#endif
};

SessionProtocolRegistrar kSSH2ProtocolRegistrar(
    SessionsWindow::SSH2,
    []() { return std::make_unique<SSH2Protocol>(); });
}

#ifdef ENABLE_SSH
int SessionsWindow::startSSH2Session(const QString &hostname, quint16 port, const QString &username, const QString &password) {
    return startSSH2Session(hostname, port, username, password, SshAuthPassword, QString(), QString(), QString());
}

int SessionsWindow::startSSH2Session(const QString &hostname, quint16 port, const QString &username, const QString &password,
                                     int authType, const QString &privateKeyPath, const QString &publicKeyPath, const QString &passphrase) {
    if(!protocol) {
        return -1;
    }
    const QVariantMap commonMeta = {
        {"hostname", hostname},
        {"port", port}
    };
    const QVariantMap protocolMeta = {
        {"userName", username},
        {"password", password},
        {"sshAuthType", authType},
        {"privateKeyPath", privateKeyPath},
        {"publicKeyPath", publicKeyPath},
        {"passphrase", passphrase}
    };
    int ret = protocol->startSession(this, commonMeta, protocolMeta);
    if(ret != 0) {
        return ret;
    }
    m_hostname = hostname;
    m_port = port;
    return 0;
}
#endif
