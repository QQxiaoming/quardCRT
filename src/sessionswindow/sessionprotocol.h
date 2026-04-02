/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#ifndef SESSIONPROTOCOL_H
#define SESSIONPROTOCOL_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <QString>
#include <QList>
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
    struct ProtocolMetaField {
        enum MetaSource {
            CommonMeta,
            ProtocolMeta,
        };

        MetaSource source = ProtocolMeta;
        QString metaKey;
        QString settingsKey;
    };

    struct ProtocolSpec {
        QString protocolId;
        QString defaultName;
        QList<ProtocolMetaField> fields;
    };

    using Factory = std::function<std::unique_ptr<SessionProtocolBase>()>;

    static SessionProtocolRegistry &instance();

    void registerFactory(SessionsWindow::SessionType type, Factory factory);
    void registerSpec(SessionsWindow::SessionType type, ProtocolSpec spec);
    std::unique_ptr<SessionProtocolBase> create(SessionsWindow::SessionType type) const;
    bool hasSpec(SessionsWindow::SessionType type) const;
    ProtocolSpec spec(SessionsWindow::SessionType type) const;
    QString settingsKey(SessionsWindow::SessionType type,
                       const QString &metaKey,
                       const QString &fallback = QString()) const;
    QString metaKey(SessionsWindow::SessionType type,
                    const QString &metaKey,
                    ProtocolMetaField::MetaSource source,
                    const QString &fallback = QString()) const;

private:
    std::unordered_map<int, Factory> factories;
    std::unordered_map<int, ProtocolSpec> specs;
};

class SessionProtocolRegistrar {
public:
    SessionProtocolRegistrar(SessionsWindow::SessionType type,
                            SessionProtocolRegistry::Factory factory,
                            SessionProtocolRegistry::ProtocolSpec spec = SessionProtocolRegistry::ProtocolSpec());
};

#endif // SESSIONPROTOCOL_H
