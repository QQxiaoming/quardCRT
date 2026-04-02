/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"

SessionProtocolRegistry &SessionProtocolRegistry::instance() {
    static SessionProtocolRegistry registry;
    return registry;
}

void SessionProtocolRegistry::registerFactory(SessionsWindow::SessionType type, Factory factory) {
    factories[static_cast<int>(type)] = std::move(factory);
}

void SessionProtocolRegistry::registerSpec(SessionsWindow::SessionType type, ProtocolSpec spec) {
    specs[static_cast<int>(type)] = std::move(spec);
}

std::unique_ptr<SessionProtocolBase> SessionProtocolRegistry::create(SessionsWindow::SessionType type) const {
    auto it = factories.find(static_cast<int>(type));
    if(it == factories.end()) {
        return nullptr;
    }
    return it->second();
}

bool SessionProtocolRegistry::hasSpec(SessionsWindow::SessionType type) const {
    return specs.find(static_cast<int>(type)) != specs.end();
}

SessionProtocolRegistry::ProtocolSpec SessionProtocolRegistry::spec(SessionsWindow::SessionType type) const {
    auto it = specs.find(static_cast<int>(type));
    if(it == specs.end()) {
        return ProtocolSpec();
    }
    return it->second;
}

QString SessionProtocolRegistry::settingsKey(SessionsWindow::SessionType type,
                                            const QString &metaKey,
                                            const QString &fallback) const {
    ProtocolSpec specData = spec(type);
    for(const auto &field : specData.fields) {
        if(field.metaKey == metaKey) {
            if(!field.settingsKey.isEmpty()) {
                return field.settingsKey;
            }
            break;
        }
    }
    return fallback;
}

QString SessionProtocolRegistry::metaKey(SessionsWindow::SessionType type,
                                         const QString &metaKey,
                                         ProtocolMetaField::MetaSource source,
                                         const QString &fallback) const {
    ProtocolSpec specData = spec(type);
    for(const auto &field : specData.fields) {
        if(field.metaKey == metaKey && field.source == source) {
            return field.metaKey;
        }
    }
    return fallback;
}

SessionProtocolRegistrar::SessionProtocolRegistrar(SessionsWindow::SessionType type,
                                                   SessionProtocolRegistry::Factory factory,
                                                   SessionProtocolRegistry::ProtocolSpec spec) {
    SessionProtocolRegistry::instance().registerFactory(type, std::move(factory));
    if(!spec.protocolId.isEmpty()) {
        SessionProtocolRegistry::instance().registerSpec(type, std::move(spec));
    }
}
