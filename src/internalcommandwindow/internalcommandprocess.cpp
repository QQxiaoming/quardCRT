/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <QMutexLocker>
#include <QMap>
#include <QSysInfo>
#include "internalcommandprocess.h"
#include "mainwindow.h"
#include "globalsetting.h"
#include "qfonticon.h"
#include "qrcodegen.h"
#include "qspdlogger.h"

InternalCommandProcess::InternalCommandProcess(CentralWidget *mainWidget, QObject *parent)
    : QThread(parent)
    , m_mainWidget(mainWidget) {
    exit = false;
}

InternalCommandProcess::~InternalCommandProcess() {
    exit = true;
    condition.wakeOne();
    wait();
}

#ifdef ENABLE_PYTHON
void InternalCommandProcess::setPyRun(PyRun *pyRun) {
    m_pyRun = pyRun;
}
#endif

void InternalCommandProcess::run(void) {
    sendWelcome();
    sendPrompt();
    while (!exit) {
        mutex.lock();
        while(!buffer.contains('\n')) {
            condition.wait(&mutex);
            if(exit) {
                mutex.unlock();
                return;
            }
        }
        if(exit) {
            mutex.unlock();
            break;
        }
        int pos = buffer.indexOf('\n');
        QString line = QString::fromUtf8(buffer.left(pos));
        buffer.remove(0, pos + 1);
        mutex.unlock();
        processLine(line);
        sendPrompt();
    }
}

void InternalCommandProcess::processLine(const QString &sline) {
    QString line = sline;
    int index = 0;
    while ((index = line.indexOf('\b', index)) != -1) {
        if (index > 0) {
            line.remove(index - 1, 2);
            index--;
        } else {
            line.remove(index, 1);
        }
    }
    QStringList pargs = line.split(' ', Qt::SkipEmptyParts);
    if(pargs.isEmpty()) {
        return;
    }
    QStringList args;
    foreach(const QString &arg, pargs) {
        args.append(arg.trimmed());
    }
    QString command = args.takeFirst();
    QList<struct Command> commands = {
        {{"help","?"}, {"help()"}, "show this help message"               ,
            [&](void) {
                static const int maxCmdNameCount = [&]() -> int {
                    int ret = 0;
                    foreach(const struct Command &cmd, commands) {
                        foreach(const QString &name, cmd.name) {
                            ret = qMax(ret, name.size());
                        }
                    }
                    return ret;
                }();
                sendLineString("Available commands:");
                foreach(const struct Command &cmd, commands) {
                    if(!cmd.description.isEmpty()) {
                        foreach(const QString &name, cmd.name) {
                            sendLineString("  " + name.leftJustified(maxCmdNameCount, ' ')  + " - " + cmd.description);
                        }
                    }
                }
            }
        },
        {{"exit","quit"}   , {"exit()","quit()"}, "exit the QuardCRT"                    , 
            [&](void) {
                qApp->exit();
            }
        },
        {{"clear","cls"},QStringList(), "clear the screen"                     , 
            [&](void) {
                sendString("\033[2J\033[1;1H");
            }
        },
        {{"version"},QStringList(), "show the version of the QuardCRT"     , 
            [&](void) {
                if(args.size() >= 1 && args.at(0) == "-c") {
                    sendLogo(true);
                } else {
                    sendLogo(false);
                }
                sendLineString(QString("QuardCRT version %0").arg(VERSION));
            }
        },
        {{"about"},QStringList(), "how the about message of the QuardCRT", 
            [&](void) {
                sendLineString("QuardCRT is a terminal emulator.");
            }
        },
        {{"license"}, {"license()"}, "show the license of the QuardCRT"     , 
            [&](void) {
                sendString(qrcodegen::generatorQrCodeASCII("https://quardcrt.readthedocs.io/en/latest/license.html"));
                sendLineString("QuardCRT is licensed under the GPLv3 License.");
            }
        },
        {{"authors"},QStringList(), "show the authors of the QuardCRT"     , 
            [&](void) {
                sendLineString("QuardCRT is authored by the Quard <2014500726@smail.xtu.edu.cn>.");
            }
        },
        {{"credits"}, {"credits()"}, "show the credits of the QuardCRT"     , 
            [&](void) {
                sendLineString("QuardCRT is the result of the hard work of the Quard.");
            }
        },
        {{"copyright"}, {"copyright()"}, "show the copyright of the QuardCRT"     , 
            [&](void) {
                sendLineString("Copyright (C) 2023-2024 Quard <2014500726@smail.xtu.edu.cn>");
            }
        },
        {{"AskQuard"},QStringList(), QString()                             , 
            [&](void) {
                sendLineString("Thank you use QuardCRT, Have a nice day!");
            }
        },
        {{"AskQuardShow"},QStringList(), QString()                         , 
            [&](void) {
                emit showString("QuardCRT", "Thank you use QuardCRT, Have a nice day!");
            }
        },
        {{"QuardSOnly"},QStringList(), QString()                           , 
            [&](void) {
                emit showEasterEggs();
            }
        },
        {{"echo"},QStringList(), "echo the arguments"                   , 
            [&](void) {
                sendLineString(args.join(' '));
            }
        },
        {{"set"},QStringList(), "set the value of the parameter"         ,
            [&](void) {
                QMap<QString,std::function<void()>> setTable = {
                     {"prompt", [&](void){
                          setPrompt(args.at(1));
                      }},
                     {"help", [&](void){
                          sendLineString("Available parameter:");
                          foreach(const QString &para, setTable.keys()){
                              sendLineString("  "+para);
                          }
                      }},
                 };
                if(args.size() >= 2) {
                    if(setTable.contains(args.at(0))) {
                        setTable[args.at(0)]();
                    } else {
                        sendLineString("Invalid parameter!",31);
                        setTable["help"]();
                    }
                } else {
                    if(!(args.size() >= 1 && args.at(0) == "help")) {
                        sendLineString("Invalid parameter!",31);
                    }
                    setTable["help"]();
                }
            }
        },
        {{"state"},QStringList(), "show states of the QuardCRT"          , 
            [&](void) {
                GlobalSetting settings;
                bool debugMode = settings.value("Debug/DebugMode",false).toBool();
                QString debugLogFile = settings.value("Debug/DebugLogFile","").toString();
                QtMsgType debugLevel = settings.value("Debug/DebugLevel",QtInfoMsg).value<QtMsgType>();
                sendLineString(QString("debugMode                : %0").arg(debugMode));
                sendLineString(QString("debugLogFile             : %0").arg(debugLogFile));
                sendLineString(QString("debugLevel               : %0").arg(debugLevel));
                sendLineString(QString("Start Application Time   : %0").arg(START_TIME.toString("yyyy-MM-dd hh:mm:ss.zzz")));
                uint64_t running_time = START_TIME.secsTo(QDateTime::currentDateTime());
                sendString("Application Running Time : ");
                if(running_time < 60) {
                    sendLineString(QString("%0s").arg(running_time));
                } else if(running_time < 60*60) {
                    sendLineString(QString("%0m %1s").arg(running_time/60).arg(running_time%60));
                } else if(running_time < 60*60*24) {
                    sendLineString(QString("%0h %1m %2s").arg(running_time/(60*60)).arg((running_time%(60*60))/60).arg((running_time%(60*60))%60));
                } else {
                    uint64_t mod = running_time%(60*60*24);
                    sendLineString(QString("%0d %1h %2m %3s").arg(running_time/(60*60*24)).arg(mod/(60*60)).arg((mod%(60*60))/60).arg((mod%(60*60))%60));
                }
            }
        },
        {{"info"},QStringList(), "show information of the QuardCRT"     , 
            [&](void) {
                sendLineString(QString("Version                : %0").arg(VERSION));
                sendLineString(QString("GitTag                 : %0").arg(GIT_TAG));
                sendLineString(QString("BuildDate              : %0").arg(DATE_TAG));
                sendLineString(QString("os                     : %0").arg(QSysInfo::prettyProductName()));
                sendLineString(QString("bootUniqueId           : %0").arg(QSysInfo::bootUniqueId()));
                sendLineString(QString("buildAbi               : %0").arg(QSysInfo::buildAbi()));
                sendLineString(QString("buildCpuArchitecture   : %0").arg(QSysInfo::buildCpuArchitecture()));
                sendLineString(QString("currentCpuArchitecture : %0").arg(QSysInfo::currentCpuArchitecture()));
                sendLineString(QString("kernelType             : %0").arg(QSysInfo::kernelType()));
                sendLineString(QString("kernelVersion          : %0").arg(QSysInfo::kernelVersion()));
                sendLineString(QString("machineHostName        : %0").arg(QSysInfo::machineHostName()));
                sendLineString(QString("machineUniqueId        : %0").arg(QSysInfo::machineUniqueId()));
                sendLineString(QString("productType            : %0").arg(QSysInfo::productType()));
                sendLineString(QString("productVersion         : %0").arg(QSysInfo::productVersion()));
                sendLineString(QString("osVersion              : %0").arg(QOperatingSystemVersion::current().name()));
            #if defined(Q_CC_MSVC)
                sendLineString(QString("compiler               : MSVC %0").arg(Q_CC_MSVC));
            #elif defined(Q_CC_CLANG)
                sendLineString(QString("compiler               : CLANG %0").arg(Q_CC_CLANG));
            #elif defined(Q_CC_GNU)
                sendLineString(QString("compiler               : GCC %0").arg(Q_CC_GNU));
            #else
                sendLineString(QString("compiler               : unknown"));
            #endif
            }
        },
        {{"buildOpt"},QStringList(),"show build options of the QuardCRT"  ,
            [&](void) {
                sendString("ENABLE_SSH              : ");
            #ifdef ENABLE_SSH
                sendLineString("YES");
            #else
                sendLineString("NO");
            #endif
                sendString("ENABLE_PYTHON           : ");
            #ifdef ENABLE_PYTHON
                sendLineString("YES");
            #else
                sendLineString("NO");
            #endif
            }
        },
        {{"bookmark"},QStringList(),"show Bookmark List"  ,
            [&](void) {
                GlobalSetting settings;
                int size = settings.beginReadArray("Global/Bookmark");
                for (int i = 0; i < size; ++i) {
                    settings.setArrayIndex(i);
                    sendLineString(settings.value("path").toString());
                }
                settings.endArray();
                sendLineString(QString("Total %0 bookmarks").arg(size));
            }
        },
        {{"iconfont"},QStringList(),"show IconFont List"  ,
            [&](void) {
                if(args.size() == 0) {
                    QStringList families = QFontIcon::currentFamilies();
                    sendLineString("Available IconFont:");
                    foreach(const QString &family, families) {
                        sendLineString("  "+family);
                    }
                } else if(args.size() == 1) {
                    bool ok = false;
                    uint16_t code = args.at(0).toUShort(&ok, 16);
                    if(ok) {
                        emit showIconFont(QChar(code));
                    } else {
                        sendLineString("Invalid IconFont code!",31);
                    }
                } else {
                    sendLineString("Invalid IconFont cmd!",31);
                }
            }
        },
        {{"qrcode"},QStringList(),"generate qrcode"  ,
            [&](void) {
                if(args.size() != 1) {
                    sendLineString("Invalid qrcode cmd!",31);
                } else {
                    QString qrCode = qrcodegen::generatorQrCodeASCII(args.at(0));
                    if(qrCode.isEmpty()) {
                        sendLineString("Generate qrcode failed!",31);
                    } else {
                        sendString(qrCode);
                    }
                }
            }
        },
        {{"logit"},QStringList(),"Test print log"  ,
            [&](void) {
                if(args.size() != 2) {
                    sendLineString("Invalid logit cmd!",31);
                } else {
                    if(args.at(0) == "debug") {
                        qDebug().noquote() << args.at(1);
                    } else if(args.at(0) == "info") {
                        qInfo().noquote() << args.at(1);
                    } else if(args.at(0) == "warning") {
                        qWarning().noquote() << args.at(1);
                    } else if(args.at(0) == "critical") {
                        qCritical().noquote() << args.at(1);
                    } else if(args.at(0) == "fatal") {
                        qFatal("%s", args.at(1).toUtf8().constData());
                    } else {
                        sendLineString("Invalid logit level!",31);
                    }
                }
            }
        },
        {{"dmesg"},QStringList(),"show dmesg log"  ,
            [&](void) {
                QStringList logs = QSPDLogger->getRingBufferSinkLog("dmesg");
                foreach(QString log, logs) {
                    log.replace("\n","\r\n");
                    sendString(log);
                }
            }
        },
    #ifdef ENABLE_PYTHON
        {{"run"},QStringList(), "run script"  ,
            [&](void) {
                if(args.size() >= 1) {
                    QFileInfo fileInfo(args.at(0));
                    if(fileInfo.isFile()) {
                        if(m_pyRun) {
                            QString result;
                            int ret = -1;
                            m_pyRun->runScriptFile(args.at(0), &result, &ret);
                            if(ret == 0) {
                                result.replace("\n","\r\n");
                                sendString(result);
                            } else {
                                sendLineString("Script run failed!",31);
                            }
                        }
                        return;
                    }
                }
                sendLineString("Script file not found!",31);
            }
        },
    #endif
    };
    if(command.isEmpty()) {
        return;
    }
    bool matched = false;
    for(const struct Command &cmd : commands) {
        foreach(const QString &name, cmd.name) {
            if(name == command) {
                cmd.action();
                matched = true;
                break;
            }
        }
        if(matched) {
            break;
        }
        foreach(const QString &name, cmd.aliasName) {
            if(name == command) {
                cmd.action();
                matched = true;
                break;
            }
        }
        if(matched) {
            break;
        }
    }
    QString fullCommand = command;
    if(!args.isEmpty()) {
        fullCommand = command + " " + args.join(' ');
    }
    if(!matched) {
        #ifdef ENABLE_PYTHON
        if(m_pyRun) {
            QString result;
            int ret = -1;
            m_pyRun->runScriptStr("from quardCRT import crt\r\nprint("+fullCommand+")", &result, &ret);
            if(ret == 0) {
                result.replace("\n","\r\n");
                sendString(result);
            } else {
                sendLineString("Invalid command!",31);
                commands.at(0).action();
            }
        } else
        #endif
        {
            sendLineString("Invalid command!",31);
            commands.at(0).action();
        }
    }
    if((!historyCmdList.isEmpty()) && historyCmdList.last() == fullCommand) {
        return;
    }
    if(historyCmdList.count() >= 100) {
        historyCmdList.removeFirst();
    }
    historyCmdList.append(fullCommand);
    historyCmdIndex = historyCmdList.count() - 1;
}

void InternalCommandProcess::recvData(const QByteArray &data) {
    QMutexLocker locker(&mutex);
    QByteArray sdata = data;
    if(sdata.contains("\x1B[A")) {
        buffer.clear();
        emit sendData(QByteArray("\x1B[2K\r"));
        sendPrompt();
        if(historyCmdList.count() > historyCmdIndex) {
            QString cmd = historyCmdList.at(historyCmdIndex--);
            if(0 > historyCmdIndex) {
                historyCmdIndex = historyCmdList.count()-1;
            }
            buffer.append(cmd.toUtf8());
            emit sendData(cmd.toUtf8());
            condition.wakeOne();
        }
        return;
    }
    if(sdata.contains("\x1B[B")) {
        buffer.clear();
        emit sendData(QByteArray("\x1B[2K\r"));
        sendPrompt();
        if(historyCmdList.count() > historyCmdIndex) {
            QString cmd = historyCmdList.at(historyCmdIndex++);
            if(historyCmdList.count() <= historyCmdIndex) {
                historyCmdIndex = 0;
            }
            buffer.append(cmd.toUtf8());
            emit sendData(cmd.toUtf8());
            condition.wakeOne();
        }
        return;
    }
    if(sdata.contains("\x1B[C")) {
        sdata.replace("\x1B[C", "");
    }
    if(sdata.contains("\x1B[D")) {
        sdata.replace("\x1B[D", "");
    }
    sdata.replace('\r', "\r\n");
    buffer.append(sdata);
    if(sdata.contains("\b")) {
        int index = 0;
        while ((index = buffer.indexOf('\b', index)) != -1) {
            if (index > 0) {
                buffer.remove(index - 1, 2);
                index--;
            } else {
                buffer.remove(index, 1);
            }
        }
        if(buffer.isEmpty()) {
            emit sendData(QByteArray("\x1B[2K\r"));
            sendPrompt();
            return;
        }
    } 
    sdata.replace("\b", "\x08\x1B[J");
    emit sendData(sdata);
    condition.wakeOne();
}

void InternalCommandProcess::sendString(const QString &str) {
    emit sendData(str.toUtf8());
}

void InternalCommandProcess::sendLineString(const QString &str, int asciiColor) {
    if(asciiColor == 0){
        sendString(str+"\r\n");
    } else {
        // asciiColor
        // 31 red
        // 34 blue
        sendString("\033[0;"+QString::number(asciiColor)+"m"+str+"\033[0m\r\n");
    }
}

void InternalCommandProcess::sendPrompt(void) {
    sendString(promptLine);
}

void InternalCommandProcess::sendLogo(bool color) {
    if(color) {
        sendString(
            "\033[1;33m  ___                      _ \033[1;36m ____ ____ _____   \033[0m\r\n"
            "\033[1;33m / _ \\ _   _  __ _ _ __ __| |\033[1;36m/ ___|  _ \\_   _|  \033[0m\r\n"
            "\033[1;33m| | | | | | |/ _` | '__/ _` \033[1;36m| |   | |_) || |    \033[0m\r\n"
            "\033[1;33m| |_| | |_| | (_| | | | (_| \033[1;36m| |___|  _ < | |    \033[0m\r\n"
            "\033[1;33m \\__\\_\\\\__,_|\\__,_|_|  \\__,_|\033[1;36m\\____|_| \\_\\|_|    \033[0m\r\n"
        );
    } else {
        sendString(
            "  ___                      _  ____ ____ _____   \r\n"
            " / _ \\ _   _  __ _ _ __ __| |/ ___|  _ \\_   _|  \r\n"
            "| | | | | | |/ _` | '__/ _` | |   | |_) || |    \r\n"
            "| |_| | |_| | (_| | | | (_| | |___|  _ < | |    \r\n"
            " \\__\\_\\\\__,_|\\__,_|_|  \\__,_|\\____|_| \\_\\|_|    \r\n"
        );
    }
}

void InternalCommandProcess::sendWelcome(void) {
    sendLogo();
    sendLineString("Welcome to the QuardCRT command window.");
    sendLineString("Please type 'help' or '?' to get a list of available commands.");
}

void InternalCommandProcess::setPrompt(const QString &prompt) {
    promptLine = prompt+" ";
}
