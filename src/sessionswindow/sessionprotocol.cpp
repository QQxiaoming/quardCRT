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

std::unique_ptr<SessionProtocolBase> SessionProtocolRegistry::create(SessionsWindow::SessionType type) const {
    auto it = factories.find(static_cast<int>(type));
    if(it == factories.end()) {
        return nullptr;
    }
    return it->second();
}

SessionProtocolRegistrar::SessionProtocolRegistrar(SessionsWindow::SessionType type,
                                                   SessionProtocolRegistry::Factory factory) {
    SessionProtocolRegistry::instance().registerFactory(type, std::move(factory));
}
