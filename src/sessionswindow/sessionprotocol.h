/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#ifndef SESSIONPROTOCOL_H
#define SESSIONPROTOCOL_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <QVariantMap>

#include "sessionswindow.h"

class SessionProtocolBase {
public:
    virtual ~SessionProtocolBase() = default;
    virtual SessionsWindow::SessionType type() const = 0;
    virtual SessionsWindow::SessionCategory category() const = 0;
    virtual void initialize(SessionsWindow *session) = 0;
    virtual void cloneTo(SessionsWindow *target,
                         const QVariantMap &commonMeta,
                         const QVariantMap &protocolMeta,
                         const QString &profile) = 0;
    virtual void disconnect(SessionsWindow *session) = 0;
    virtual void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) = 0;
    virtual bool hasChildProcess(SessionsWindow *session) = 0;
    virtual void refreshTermSize(SessionsWindow *session) = 0;
    virtual QWidget *mainWidget(SessionsWindow *session) {
        Q_UNUSED(session);
        return nullptr;
    }
    virtual bool preprocessActivatedUrl(SessionsWindow *session, QUrl &url) {
        Q_UNUSED(session);
        if(url.toString().startsWith("relative:")) {
            return false;
        }
        return true;
    }
    virtual bool supportsUrlPostProcess(SessionsWindow *session) {
        Q_UNUSED(session);
        return false;
    }
    virtual void cleanup(SessionsWindow *session) {
        Q_UNUSED(session);
    }
#ifdef ENABLE_SSH
    virtual SshSFtp *getSshSFtpChannel(SessionsWindow *session) {
        Q_UNUSED(session);
        return nullptr;
    }
#endif
    virtual QVariantMap exportMeta() const {
        return QVariantMap();
    }
    virtual void importMeta(const QVariantMap &meta) {
        Q_UNUSED(meta);
    }
    virtual int startSession(SessionsWindow *session,
                             const QVariantMap &commonMeta,
                             const QVariantMap &protocolMeta) {
        Q_UNUSED(session);
        Q_UNUSED(commonMeta);
        Q_UNUSED(protocolMeta);
        return -1;
    }
    virtual void screenShot(SessionsWindow *session, const QString &fileName) {
        Q_UNUSED(session);
        Q_UNUSED(fileName);
    }
    virtual void screenShot(SessionsWindow *session, QPixmap *pixmap) {
        Q_UNUSED(session);
        Q_UNUSED(pixmap);
    }
};

class SessionProtocolRegistry {
public:
    using Factory = std::function<std::unique_ptr<SessionProtocolBase>()>;

    static SessionProtocolRegistry &instance();

    void registerFactory(SessionsWindow::SessionType type, Factory factory);
    std::unique_ptr<SessionProtocolBase> create(SessionsWindow::SessionType type) const;

private:
    std::unordered_map<int, Factory> factories;
};

class SessionProtocolRegistrar {
public:
    SessionProtocolRegistrar(SessionsWindow::SessionType type, SessionProtocolRegistry::Factory factory);
};

#endif // SESSIONPROTOCOL_H
