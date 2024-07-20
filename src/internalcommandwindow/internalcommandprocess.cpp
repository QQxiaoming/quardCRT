#include <QMutexLocker>
#include <QMap>
#include <QSysInfo>
#include "internalcommandprocess.h"
#include "mainwindow.h"
#include "globalsetting.h"

InternalCommandProcess::InternalCommandProcess(QObject *parent)
    : QThread(parent) {
    exit = false;
}

InternalCommandProcess::~InternalCommandProcess() {
    exit = true;
    condition.wakeOne();
    wait();
}

void InternalCommandProcess::run(void) {
    sendString("Welcome to the QuardCRT command window.\r\n");
    sendString("Please type 'help' or '?' to get a list of available commands.\r\n");
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

    const QMap<QString, QString> commands = {
        {"help",    "show this help message"},
        {"?",       "show this help message"},
        {"exit",    "exit the QuardCRT"},
        {"quit",    "exit the QuardCRT"},
        {"clear",   "clear the screen"},
        {"cls",     "clear the screen"},
        {"version", "show the version of the QuardCRT"},
        {"about",   "how the about message of the QuardCRT"},
        {"license", "show the license of the QuardCRT"},
        {"authors", "show the authors of the QuardCRT"},
        {"credits", "show the credits of the QuardCRT"},
        {"copying", "show the copying of the QuardCRT"},
        {"echo",    "echo the arguments"},
        {"state",   "show states of the QuardCRT"},
        {"info",    "show information of the QuardCRT"},
    };
    if(command == "help" || command == "?") {
        sendString("Available commands:\r\n");
        foreach(const QString &key, commands.keys()) {
            sendString("  " + key + " - " + commands[key] + "\r\n");
        }
    } else if(command == "exit" || command == "quit") {
        qApp->exit();
    } else if(command == "clear" || command == "cls") {
        sendString("\033[2J\033[1;1H");
    } else if(command == "version") {
        sendString(QString("QuardCRT version %0\r\n").arg(VERSION));
    } else if(command == "about") {
        sendString("QuardCRT is a terminal emulator.\r\n");
    } else if(command == "license") {
        sendString("QuardCRT is licensed under the GPLv3 License.\r\n");
    } else if(command == "authors") {
        sendString("QuardCRT is authored by the Quard <2014500726@smail.xtu.edu.cn>.\r\n");
    } else if(command == "credits") {
        sendString("QuardCRT is credited to the Quard <2014500726@smail.xtu.edu.cn>.\r\n");
    } else if(command == "copying") {
        sendString("QuardCRT is copied by the Quard <2014500726@smail.xtu.edu.cn>.\r\n");
    } else if(command == "AskQuard") {
        sendString("Thank you use QuardCRT, Have a nice day!\r\n");
    } else if(command == "AskQuardShow") {
        emit showString("QuardCRT", "Thank you use QuardCRT, Have a nice day!\r\n");
    } else if(command == "QuardSOnly") {
        emit showEasterEggs();
    } else if(command == "echo") {
        sendString(args.join(' ') + "\r\n");
    } else if(command == "state") {
        GlobalSetting settings;
        bool debugMode = settings.value("Debug/DebugMode",false).toBool();
        QString debugLogFile = settings.value("Debug/DebugLogFile","").toString();
        QtMsgType debugLevel = settings.value("Debug/DebugLevel",QtInfoMsg).value<QtMsgType>();
        sendString(QString("debugMode : %0\r\n").arg(debugMode));
        sendString(QString("debugLogFile : %0\r\n").arg(debugLogFile));
        sendString(QString("debugLevel : %0\r\n").arg(debugLevel));
        sendString(QString("Start Application Time : %0\r\n").arg(START_TIME.toString("yyyy-MM-dd hh:mm:ss.zzz")));
        uint64_t running_time = START_TIME.secsTo(QDateTime::currentDateTime());
        sendString("Application Running Time : ");
        if(running_time < 60) {
            sendString(QString("%0s\r\n").arg(running_time));
        } else if(running_time < 60*60) {
            sendString(QString("%0m %1s\r\n").arg(running_time/60).arg(running_time%60));
        } else if(running_time < 60*60*24) {
            sendString(QString("%0h %1m %2s\r\n").arg(running_time/(60*60)).arg((running_time%(60*60))/60).arg((running_time%(60*60))%60));
        } else {
            uint64_t mod = running_time%(60*60*24);
            sendString(QString("%0d %1h %2m %3s\r\n").arg(running_time/(60*60*24)).arg(mod/(60*60)).arg((mod%(60*60))/60).arg((mod%(60*60))%60));
        } 
    } else if(command == "info") {
        sendString(QString("Version : %0\r\n").arg(VERSION));
        sendString(QString("GitTag : %0\r\n").arg(GIT_TAG));
        sendString(QString("BuildDate : %0\r\n").arg(DATE_TAG));
        sendString(QString("os : %0\r\n").arg(QSysInfo::prettyProductName()));
        sendString(QString("bootUniqueId : %0\r\n").arg(QSysInfo::bootUniqueId()));
        sendString(QString("buildAbi : %0\r\n").arg(QSysInfo::buildAbi()));
        sendString(QString("buildCpuArchitecture : %0\r\n").arg(QSysInfo::buildCpuArchitecture()));
        sendString(QString("currentCpuArchitecture : %0\r\n").arg(QSysInfo::currentCpuArchitecture()));
        sendString(QString("kernelType : %0\r\n").arg(QSysInfo::kernelType()));
        sendString(QString("kernelVersion : %0\r\n").arg(QSysInfo::kernelVersion()));
        sendString(QString("machineHostName : %0\r\n").arg(QSysInfo::machineHostName()));
        sendString(QString("machineUniqueId : %0\r\n").arg(QSysInfo::machineUniqueId()));
        sendString(QString("productType : %0\r\n").arg(QSysInfo::productType()));
        sendString(QString("productVersion : %0\r\n").arg(QSysInfo::productVersion()));
        sendString(QString("osVersion : %0\r\n").arg(QOperatingSystemVersion::current().name()));
    #if defined(Q_CC_MSVC)
        sendString(QString("compiler : MSVC %0\r\n").arg(Q_CC_MSVC));
    #elif defined(Q_CC_CLANG)
        sendString(QString("compiler : CLANG %0\r\n").arg(Q_CC_CLANG));
    #elif defined(Q_CC_GNU)
        sendString(QString("compiler : GCC %0\r\n").arg(Q_CC_GNU));
    #else
        sendString(QString("compiler : unknown\r\n"));
    #endif
    } else {
        if(command.isEmpty()) {
            return;
        }
        sendString("Unknown command: " + command + "\r\n");
    }
    if((!historyCmdList.isEmpty()) && historyCmdList.last() == command) {
        return;
    }
    if(historyCmdList.count() >= 100) {
        historyCmdList.removeFirst();
    }
    historyCmdList.append(command);
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

void InternalCommandProcess::sendPrompt(void) {
    sendString(promptLine);
}
