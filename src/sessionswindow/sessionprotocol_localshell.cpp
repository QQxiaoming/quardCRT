/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QRegularExpression>

#include "sessionprotocol.h"
#include "sessionswindow.h"
#include "globalsetting.h"
#include "argv_split.h"
#include "misc.h"

namespace sessionprotocol {
class LocalShellProtocol final : public SessionProtocolBase {
public:
    SessionsWindow::SessionType type() const override { return SessionsWindow::LocalShell; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::ConsoleSession; }
    void initialize(SessionsWindow *session) override {
        session->showTitleType = SessionsWindow::ShortTitle;
        localShell = PtyQt::createPtyProcess();
        QObject::connect(session->term, &QTermWidget::termSizeChange, session, [=](int lines, int columns){
            localShell->resize(columns, lines);
        });
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        target->setWorkingDirectory(commonMeta.value("workingDirectory").toString());
        target->startLocalShellSession(
            protocolMeta.value("command").toString(),
            profile,
            static_cast<SessionsWindow::ShellType>(commonMeta.value("shellType", SessionsWindow::Unknown).toInt()));
    }
    void disconnect(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(localShell) {
            localShell->kill();
        }
        session->updateConnectionState(false);
    }
    void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) override {
        if(session->state == SessionsWindow::Connected && localShell) {
            info.localShell.tree = localShell->processInfoTree();
        }
    }
    bool hasChildProcess(SessionsWindow *session) override {
        Q_UNUSED(session);
        return localShell && localShell->hasChildProcess();
    }
    void refreshTermSize(SessionsWindow *session) override {
        if(localShell) {
            localShell->resize(session->term->columns(), session->term->lines());
        }
    }
    bool preprocessActivatedUrl(SessionsWindow *session, QUrl &url) override {
        QString path = url.toString();
        if(path.startsWith("relative:")) {
            path.remove("relative:");
            path = session->getWorkingDirectory() + "/" + path;
            url = QUrl::fromLocalFile(path);
            return true;
        }
#if defined(Q_OS_WIN)
        if(path.startsWith("file://") && session->getShellType() == SessionsWindow::WSL) {
            path.remove("file://");
            static QRegularExpression wslDirFormat("^/mnt/(\\S+)/(.*)$");
            if(wslDirFormat.match(path).hasMatch()) {
                QString pathFix = wslDirFormat.match(path).captured(1).toUpper() + ":/" + wslDirFormat.match(path).captured(2);
                url = QUrl::fromLocalFile(pathFix);
            } else if(path.startsWith("/mnt/")) {
                url = QUrl::fromLocalFile(path.remove(0, 5).toUpper() + ":/");
            }
        }
#endif
        return true;
    }
    bool supportsUrlPostProcess(SessionsWindow *session) override {
        Q_UNUSED(session);
        return true;
    }
    void cleanup(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(localShell) {
            localShell->kill();
            delete localShell;
            localShell = nullptr;
        }
    }
    int startLocalShellSession(SessionsWindow *session, const QString &command, QString profile, SessionsWindow::ShellType sTp) {
        QString shellPath;
        QStringList args;
        session->shellType = sTp;
        if(command.isEmpty()) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            Q_UNUSED(profile);
            GlobalSetting settings;
            QString defaultLocalShell = settings.value("Global/Options/DefaultLocalShell", "ENV:SHELL").toString();
            if(defaultLocalShell != "ENV:SHELL") {
                QFileInfo shellInfo(defaultLocalShell);
                if(shellInfo.isExecutable()) {
                    shellPath = defaultLocalShell;
                }
            }
            if(shellPath.isEmpty()) shellPath = qEnvironmentVariable("SHELL");
            if(shellPath.isEmpty()) shellPath = "/bin/sh";
#elif defined(Q_OS_WIN)
            GlobalSetting setting;
            QString wslUserName = setting.value("Global/Options/WSLUserName", "root").toString();
            QString wslDistro = setting.value("Global/Options/WSLDistroName", "Ubuntu").toString();
            wslUserNameValue = wslUserName;
            switch (session->shellType) {
                case SessionsWindow::PowerShell:
                default: {
                    QString defaultLocalShell = setting.value("Global/Options/DefaultLocalShell", "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe").toString();
                    QFileInfo shellInfo(defaultLocalShell);
                    if(shellInfo.isExecutable()) {
                        shellPath = defaultLocalShell;
                    }
                    if(shellPath.isEmpty()) shellPath = "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
                    args = {
                        "-ExecutionPolicy",
                        "Bypass",
                        "-NoLogo",
                        "-NoProfile",
                        "-NoExit",
                        "-File",
                    };
                    if(profile.isEmpty()) {
                        profile = QString("\"") + QApplication::applicationDirPath() + "/Profile.ps1\"";
                    } else {
                        profile = QString("\"") + profile + "\"";
                    }
                    args.append(profile);
                    break;
                }
                case SessionsWindow::WSL:
                    shellPath = "c:\\Windows\\System32\\wsl.exe";
                    args = {
                        "-u",
                        wslUserName,
                        "-d",
                        wslDistro,
                    };
                    break;
            }
#endif
        } else {
            argv_split parser;
            parser.parse(command.toStdString());
            for(auto &it : parser.getArguments()) {
                args.append(QString::fromStdString(it));
            }
            shellPath = args.first();
            args.removeFirst();
        }

        QStringList envs = QProcessEnvironment::systemEnvironment().toStringList();
#if defined(Q_OS_WIN)
        if(envs.contains("quardcrt_computername")) {
            envs.replaceInStrings("quardcrt_computername", MiscWin32::getComputerName());
        } else {
            envs.append("quardcrt_computername=" + MiscWin32::getComputerName());
        }
        if(envs.contains("quardcrt_username")) {
            envs.replaceInStrings("quardcrt_username", MiscWin32::getUserName());
        } else {
            envs.append("quardcrt_username=" + MiscWin32::getUserName());
        }
#endif

        GlobalSetting setting;
#if defined(Q_OS_MACOS)
        bool defaultForceUTF8 = true;
#else
        bool defaultForceUTF8 = false;
#endif
        bool forceUTF8 = setting.value("Global/misc/forceUTF8", defaultForceUTF8).toBool();
        if(forceUTF8) {
            envs.append("LC_CTYPE=UTF-8");
        }

        QFileInfo fileInfo(session->workingDirectory);
        if(!fileInfo.exists() || !fileInfo.isDir()) {
            session->workingDirectory = QDir::homePath();
        }

        if(!localShell) {
            return -1;
        }
        bool ret = localShell->startProcess(shellPath, args, session->workingDirectory, envs,
                                            session->term->screenColumnsCount(), session->term->screenLinesCount());
        if(!ret) {
            session->state = SessionsWindow::Error;
            emit session->stateChanged(session->state);
            QMessageBox::warning(session->messageParentWidget, session->tr("Start Local Shell"),
                                 session->getName() + "\n" + session->tr("Cannot start local shell:\n%1.").arg(localShell->lastError()));
            return -1;
        }

        QObject::connect(localShell->notifier(), &QIODevice::readyRead, session, [=](){
            QByteArray data = localShell->readAll();
            if(!data.isEmpty()) {
                if(session->doRecvData(data)) {
                    session->term->recvData(data.data(), data.size());
                }
            }
        });
        QObject::connect(localShell->notifier(), &QIODevice::aboutToClose, session, [=] {
            if(localShell) {
                QByteArray restOfOutput = localShell->readAll();
                if(!restOfOutput.isEmpty()) {
                    if(session->doRecvData(restOfOutput)) {
                        session->term->recvData(restOfOutput.data(), restOfOutput.size());
                    }
                    localShell->notifier()->disconnect();
                }
            }
        });
        session->setupModemProxyForward([=]() {
            return true;
        }, [=](const QByteArray &data) {
            localShell->write(data);
        });
        session->setupTerminalSendForward([=]() {
            return true;
        }, [=](const char *data, int size) {
            localShell->write(QByteArray(data, size));
        }, false);

        commandValue = command;
        session->updateConnectionState(true);
        return 0;
    }
    QVariantMap exportMeta() const override {
        return {
            {"command", commandValue},
            {"wslUserName", wslUserNameValue}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        commandValue = meta.value("command").toString();
        wslUserNameValue = meta.value("wslUserName").toString();
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        return startLocalShellSession(
            session,
            protocolMeta.value("command").toString(),
            protocolMeta.value("profile").toString(),
            static_cast<SessionsWindow::ShellType>(commonMeta.value("shellType", SessionsWindow::Unknown).toInt()));
    }

private:
    IPtyProcess *localShell = nullptr;
    QString commandValue;
    QString wslUserNameValue;
};

SessionProtocolRegistrar kLocalShellProtocolRegistrar(
    SessionsWindow::LocalShell,
    []() { return std::make_unique<LocalShellProtocol>(); });
}

int SessionsWindow::startLocalShellSession(const QString &command, QString profile, ShellType sTp) {
    if(!protocol) {
        return -1;
    }
    const QVariantMap commonMeta = {
        {"shellType", static_cast<int>(sTp)}
    };
    const QVariantMap protocolMeta = {
        {"command", command},
        {"profile", profile}
    };
    return protocol->startSession(this, commonMeta, protocolMeta);
}
