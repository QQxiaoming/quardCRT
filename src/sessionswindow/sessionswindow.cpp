/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
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
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>
#include <QProcessEnvironment>
#include <QMessageBox>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QCryptographicHash>

#include "qsendzmodem.h"
#include "qrecvzmodem.h"
#include "sshshell.h"
#include "filedialog.h"
#include "sessionswindow.h"
#include "globaloptionswindow.h"
#include "globalsetting.h"
#include "argv_split.h"

SessionsWindow::SessionsWindow(SessionType tp, QWidget *parent)
    : QObject(parent)
    , type(tp)
    , workingDirectory(QDir::homePath())
    , showShortTitle(false)
    , term(nullptr)
    , telnet(nullptr)
    , serialPort(nullptr)
    , rawSocket(nullptr)
    , localShell(nullptr)
    , namePipe(nullptr)
    , ssh2Client(nullptr)
    , vncClient(nullptr)
    , enableLog(false)
    , enableRawLog(false) {
    if(type == VNC) {
        vncClient = new QVNCClientWidget(parent);
        vncClient->setProperty("session", QVariant::fromValue(this));
        vncClient->setFullScreen(false);
        vncClient->setMouseHide(false);
        connect(vncClient, &QVNCClientWidget::connected, this, [=](bool connected){
            if(connected) {
                state = Connected;
                emit stateChanged(state);
            } else {
                state = Disconnected;
                emit stateChanged(state);
            }
        });
    } else {
        term = new QTermWidget(parent);
        term->setProperty("session", QVariant::fromValue(this));

        term->setScrollBarPosition(QTermWidget::ScrollBarRight);
        term->setBlinkingCursor(true);
        term->setMargin(0);
        term->startTerminalTeletype();
        term->setDrawLineChars(false);

        QStringList availableColorSchemes = term->availableColorSchemes();
        availableColorSchemes.sort();
        QString currentColorScheme = availableColorSchemes.first();
        foreach(QString colorScheme, availableColorSchemes) {
            if(colorScheme == GlobalOptionsWindow::defaultColorScheme) {
                term->setColorScheme(GlobalOptionsWindow::defaultColorScheme);
                currentColorScheme = GlobalOptionsWindow::defaultColorScheme;
            }
        }

        QStringList availableKeyBindings = term->availableKeyBindings();
        availableKeyBindings.sort();
        QString currentAvailableKeyBindings = availableKeyBindings.first();
        foreach(QString keyBinding, availableKeyBindings) {
            if(keyBinding == "linux") {
                term->setKeyBindings("linux");
                currentAvailableKeyBindings = "linux";
            }
        }
    }

    switch (type) {
        case LocalShell: {
            showShortTitle = true;
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            IPtyProcess::PtyType ptyType = IPtyProcess::UnixPty;
        #elif defined(Q_OS_WIN)
            IPtyProcess::PtyType ptyType = IPtyProcess::WinPty;
            //qint32 buildNumber = QSysInfo::kernelVersion().split(".").last().toInt();
            //if (buildNumber >= CONPTY_MINIMAL_WINDOWS_VERSION) {
            //    ptyType = IPtyProcess::ConPty;
            //}
        #endif
            localShell = PtyQt::createPtyProcess(ptyType);
            connect(term, &QTermWidget::termSizeChange, this, [=](int lines, int columns){
                localShell->resize(columns,lines);
            });
            break;
        }
        case Telnet: {
            telnet = new QTelnet(QTelnet::TCP, this);
            connect(telnet,&QTelnet::newData,this,
                [=](const char *data, int size){
                if(modemProxy) {
                    emit modemProxyRecvData(QByteArray(data,size));
                    return;
                }
                term->recvData(data, size);
                rx_total += size;
                saveRawLog(data, size);
                emit hexDataDup(data, size);
            });
            connect(this,&SessionsWindow::modemProxySendData,this,
                [=](QByteArray data){
                if(modemProxy) {
                    telnet->sendData(data.data(), data.size());
                }
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(modemProxy) {
                    return;
                }
                if(telnet->isConnected()) {
                    telnet->sendData(data, size);
                    tx_total += size;
                }
            });
            connect(telnet, &QTelnet::stateChanged, this, [=](QAbstractSocket::SocketState socketState){
                if(socketState == QAbstractSocket::ConnectedState) {
                    state = Connected;
                    emit stateChanged(state);
                } else if(socketState == QAbstractSocket::UnconnectedState) {
                    state = Disconnected;
                    emit stateChanged(state);
                }
            });
            connect(telnet, &QTelnet::error, this, [=](QAbstractSocket::SocketError socketError){
                QMessageBox::warning(term, tr("Telnet Error"), tr("Telnet error:\n%1.").arg(telnet->errorString()));
                state = Error;
                emit stateChanged(state);
                Q_UNUSED(socketError);
            });
            break;
        }
        case Serial: {
            serialPort = new QSerialPort(this);
            connect(serialPort,&QSerialPort::readyRead,this,
                [=](){
                QByteArray data = serialPort->readAll();
                if(modemProxy) {
                    emit modemProxyRecvData(data);
                    return;
                }
                term->recvData(data.data(), data.size());
                rx_total += data.size();
                saveRawLog(data.data(), data.size());
                emit hexDataDup(data.data(), data.size());
            });
            connect(this,&SessionsWindow::modemProxySendData,this,
                [=](QByteArray data){
                if(modemProxy) {
                    if(serialPort->isOpen()) {
                        serialPort->write(data.data(), data.size());
                    }
                }
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(modemProxy) {
                    return;
                }
                if(serialPort->isOpen()) {
                    serialPort->write(data, size);
                    tx_total += size;
                }
            });
            connect(serialPort, &QSerialPort::errorOccurred, this, [=](QSerialPort::SerialPortError serialPortError){
                if(serialPort->error() == QSerialPort::NoError) return;
                if(state == Error) return;
                QMessageBox::warning(term, tr("Serial Error"), tr("Serial error:\n%1.").arg(serialPort->errorString()));
                state = Error;
                emit stateChanged(state);
                serialPort->close();
                Q_UNUSED(serialPortError);
            });
            break;
        }
        case RawSocket: {
            rawSocket = new QTcpSocket(this);
            connect(rawSocket,&QTcpSocket::readyRead,this,
                [=](){
                QByteArray data = rawSocket->readAll();
                if(modemProxy) {
                    emit modemProxyRecvData(data);
                    return;
                }
                term->recvData(data.data(), data.size());
                rx_total += data.size();
                saveRawLog(data.data(), data.size());
                emit hexDataDup(data.data(), data.size());
            });
            connect(this,&SessionsWindow::modemProxySendData,this,
                [=](QByteArray data){
                if(modemProxy) {
                    if(rawSocket->state() == QAbstractSocket::ConnectedState) {
                        rawSocket->write(data.data(), data.size());
                    }
                }
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(modemProxy) {
                    return;
                }
                if(rawSocket->state() == QAbstractSocket::ConnectedState) {
                    rawSocket->write(data, size);
                    tx_total += size;
                }
            });
            connect(rawSocket, &QTcpSocket::stateChanged, this, [=](QAbstractSocket::SocketState socketState){
                if(socketState == QAbstractSocket::ConnectedState) {
                    state = Connected;
                    emit stateChanged(state);
                } else if(socketState == QAbstractSocket::UnconnectedState) {
                    state = Disconnected;
                    emit stateChanged(state);
                }
            });
            connect(rawSocket, &QTcpSocket::errorOccurred, this, [=](QAbstractSocket::SocketError socketError){
                QMessageBox::warning(term, tr("Raw Socket Error"), tr("Raw Socket error:\n%1.").arg(rawSocket->errorString()));
                state = Error;
                emit stateChanged(state);
                Q_UNUSED(socketError);
            });
            break;
        }
        case NamePipe: {
            namePipe = new QLocalSocket(this);
            connect(namePipe,&QLocalSocket::readyRead,this,[=](){
                QByteArray data = namePipe->readAll();
                if(modemProxy) {
                    emit modemProxyRecvData(data);
                    return;
                }
                term->recvData(data.data(), data.size());
                saveRawLog(data.data(), data.size());
                emit hexDataDup(data.data(), data.size());
            });
            connect(this,&SessionsWindow::modemProxySendData,this,
                [=](QByteArray data){
                if(modemProxy) {
                    if(namePipe->state() == QLocalSocket::ConnectedState) {
                        namePipe->write(data.data(), data.size());
                    }
                }
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(modemProxy) {
                    return;
                }
                if(namePipe->state() == QLocalSocket::ConnectedState) {
                    namePipe->write(data, size);
                }
            });
            connect(namePipe, &QLocalSocket::stateChanged, this, [=](QLocalSocket::LocalSocketState socketState){
                if(socketState == QLocalSocket::ConnectedState) {
                    state = Connected;
                    emit stateChanged(state);
                } else if(socketState == QLocalSocket::UnconnectedState) {
                    state = Disconnected;
                    emit stateChanged(state);
                }
            });
            connect(namePipe, &QLocalSocket::errorOccurred, this, [=](QLocalSocket::LocalSocketError socketError){
                QMessageBox::warning(term, tr("Name Pipe Error"), tr("Name Pipe error:\n%1.").arg(namePipe->errorString()));
                state = Error;
                emit stateChanged(state);
                Q_UNUSED(socketError);
            });
            break;
        }
        case SSH2: {
            ssh2Client = new SshClient("ssh2", this);
            connect(ssh2Client, &SshClient::sshReady, this, [=](){
                SshShell *shell = ssh2Client->getChannel<SshShell>("quardCRT.shell");
                if(shell == nullptr) {
                    QMessageBox::warning(term, tr("SSH2 Error"), tr("SSH2 error:\n%1.").arg(sshErrorToString(ssh2Client->sshState())));
                    state = Error;
                    emit stateChanged(state);
                    return;
                }
                QTimer::singleShot(100, [=](){
                    shell->resize(term->columns(),term->lines());
                });
                connect(term, &QTermWidget::termSizeChange, this, [=](int lines, int columns){
                    shell->resize(columns,lines);
                });
                connect(shell, &SshShell::readyRead, this, [=](const char *data, int size){
                    if(modemProxy) {
                        emit modemProxyRecvData(QByteArray(data, size));
                        return;
                    }
                    term->recvData(data, size);
                    rx_total += size;
                    saveRawLog(data, size);
                    emit hexDataDup(data, size);
                });
                connect(this,&SessionsWindow::modemProxySendData,this,
                    [=](QByteArray data){
                    if(modemProxy) {
                        shell->sendData(data.data(), data.size());
                    }
                });
                connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
                    if(modemProxy) {
                        return;
                    }
                    shell->sendData(data, size);
                    tx_total += size;
                });
                state = Connected;
                emit stateChanged(state);
            });
            connect(ssh2Client, &SshClient::sshDisconnected, this, [=](){
                state = Disconnected;
                emit stateChanged(state);
            });
            connect(ssh2Client, &SshClient::sshError, this, [=](){
                QMessageBox::warning(term, tr("SSH2 Error"), tr("SSH2 error:\n%1.").arg(sshErrorToString(ssh2Client->sshState())));
                state = Error;
                emit stateChanged(state);
            });
            break;
        }
        case VNC:
            break;
    }

    if(type != VNC) {
        connect(term, &QTermWidget::titleChanged, this, &SessionsWindow::titleChanged);
        connect(term, &QTermWidget::dupDisplayOutput, this, [&](const char *data, int size){
            saveLog(data, size);
        });
        connect(term, &QTermWidget::urlActivated, this, [&](const QUrl& url, bool fromContextMenu){
            QUrl u = url;
            QString path = u.toString();
            if(path.startsWith("relative:") ) {
                if(getSessionType() == LocalShell) {
                    path.remove("relative:");
                    path = getWorkingDirectory() + "/" + path;
                    u = QUrl::fromLocalFile(path);
                } else {
                    return;
                }
            }
            QDesktopServices::openUrl(u);
            Q_UNUSED(fromContextMenu);
        });
        connect(term, &QTermWidget::mousePressEventForwarded, this, [&](QMouseEvent *event){
            // only windows and macos need do this
        #if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
            if(event->button() == Qt::MiddleButton) {
                term->copyClipboard();
                term->pasteClipboard();
            }
        #else
            Q_UNUSED(event);
        #endif
        });
    }
}

SessionsWindow::~SessionsWindow() {
    enableLog = false;
    enableRawLog = false;
    log_file_mutex.lock();
    if(log_file != nullptr) {
        log_file->close();
        delete log_file;
        log_file = nullptr;
    }
    log_file_mutex.unlock();
    raw_log_file_mutex.lock();
    if(raw_log_file != nullptr) {
        raw_log_file->close();
        delete raw_log_file;
        raw_log_file = nullptr;
    }
    raw_log_file_mutex.unlock();
    if(localShell) {
        localShell->kill();
        delete localShell;
    }
    if(telnet) {
        if(telnet->isConnected()) telnet->disconnectFromHost();
        delete telnet;
    }
    if(serialPort) {
        if(serialPort->isOpen()) serialPort->close();
        delete serialPort;
    }
    if(rawSocket){
        if(rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
        delete rawSocket;
    }
    if(namePipe){
        if(namePipe->state() == QLocalSocket::ConnectedState) namePipe->disconnectFromServer();
        delete namePipe;
    }
    if(ssh2Client) {
        ssh2Client->disconnectFromHost();
        delete ssh2Client;
    }
    if(vncClient) {
        vncClient->disconnectFromVncServer();
        delete vncClient;
    }
    if(term) {
        delete term;
    }
}

void SessionsWindow::cloneSession(SessionsWindow *src) {
    switch(src->getSessionType()) {
        case LocalShell: {
            setWorkingDirectory(src->getWorkingDirectory());
            startLocalShellSession(src->m_command);
            break;
        case Telnet:
            startTelnetSession(src->m_hostname, src->m_port, src->m_type);
            break;
        case Serial:
            startSerialSession(src->m_portName, src->m_baudRate, src->m_dataBits, src->m_parity, src->m_stopBits, src->m_flowControl, src->m_xEnable);
            break;
        case RawSocket:
            startRawSocketSession(src->m_hostname, src->m_port);
            break;
        case NamePipe:
            startNamePipeSession(src->m_pipeName);
            break;
        case SSH2:
            startSSH2Session(src->m_hostname, src->m_port, src->m_username, src->m_password);
            break;
        case VNC:
            startVNCSession(src->m_hostname, src->m_port, src->m_password);
            break;
        }
    }  
}

int SessionsWindow::startLocalShellSession(const QString &command) {
    QString shellPath;
    QStringList args;
    if(command.isEmpty()) {
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        shellPath = qEnvironmentVariable("SHELL");
        if(shellPath.isEmpty()) shellPath = "/bin/sh";
    #elif defined(Q_OS_WIN)
        shellPath = "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
        args =  {
            "powershell.exe",
            "-ExecutionPolicy",
            "Bypass",
            "-NoLogo",
            "-NoProfile",
            "-NoExit",
            "-File",
            "\"" + QApplication::applicationDirPath() + "/Profile.ps1\""
        };
    #endif
    } else {
        argv_split parser;
        parser.parse(command.toStdString());
        for(auto &it : parser.getArguments()) {
            args.append(QString::fromStdString(it));
        }
        shellPath = args.first();
        args.removeFirst();
    #if defined(Q_OS_WIN)
        QFileInfo fi(shellPath);
        args.insert(0, fi.fileName());
    #endif
    }
    QStringList envs = QProcessEnvironment::systemEnvironment().toStringList();
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
    bool ret = localShell->startProcess(shellPath, args, envs, workingDirectory, term->screenColumnsCount(), term->screenLinesCount());
    if(!ret) {
        state = Error;
        emit stateChanged(state);
        QMessageBox::warning(term, tr("Start Local Shell"), tr("Cannot start local shell:\n%1.").arg(localShell->lastError()));
        return -1;
    }
    connect(localShell->notifier(), &QIODevice::readyRead, this, [=](){
        QByteArray data = localShell->readAll();
        if(modemProxy) {
            emit modemProxyRecvData(data);
            return;
        }
        term->recvData(data.data(), data.size());
        saveRawLog(data.data(), data.size());
        emit hexDataDup(data.data(), data.size());
    });
    connect(this,&SessionsWindow::modemProxySendData,this,
        [=](QByteArray data){
        if(modemProxy) {
            localShell->write(data);
        }
    });
    connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
        if(modemProxy) {
            return;
        }
        localShell->write(QByteArray(data, size));
    });
    m_command = command;
    state = Connected;
    emit stateChanged(state);
    return 0;
}

int SessionsWindow::startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type) {
    telnet->setType(type);
    telnet->connectToHost(hostname, port);
    m_hostname = hostname;
    m_port = port;
    m_type = type;
    return 0;
}

int SessionsWindow::startSerialSession(const QString &portName, uint32_t baudRate
    , int dataBits, int parity, int stopBits, bool flowControl, bool xEnable ) {
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    switch (parity)
    {
    case 0:
    default:
        serialPort->setParity(QSerialPort::NoParity);
        break;
    case 1:
        serialPort->setParity(QSerialPort::OddParity);
        break;
    case 2:
        serialPort->setParity(QSerialPort::EvenParity);
        break;
    }
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));
    serialPort->setFlowControl(flowControl?QSerialPort::HardwareControl:QSerialPort::NoFlowControl);
    if(!serialPort->open(QIODevice::ReadWrite)) {
        state = Error;
        emit stateChanged(state);
    } else {
        state = Connected;
        emit stateChanged(state);
    }
    serialPort->setBreakEnabled(xEnable);
    m_portName = portName;
    m_baudRate = baudRate;
    m_dataBits = dataBits;
    m_parity = parity;
    m_stopBits = stopBits;
    m_flowControl = flowControl;
    m_xEnable = xEnable;
    return 0;
}

int SessionsWindow::startRawSocketSession(const QString &hostname, quint16 port) {
    rawSocket->connectToHost(hostname, port);
    m_hostname = hostname;
    m_port = port;
    return 0;
}

int SessionsWindow::startNamePipeSession(const QString &name) {
    namePipe->connectToServer(name);
    m_pipeName = name;
    return 0;
}

int SessionsWindow::startSSH2Session(const QString &hostname, quint16 port, const QString &username, const QString &password) {
    QByteArrayList methodes;
    methodes.append("password");
    ssh2Client->setPassphrase(password);
    ssh2Client->connectToHost(username, hostname, port, methodes);
    m_hostname = hostname;
    m_port = port;
    m_username = username;
    m_password = password;
    return 0;
}

int SessionsWindow::startVNCSession(const QString &hostname, quint16 port, const QString &password) {
    vncClient->connectToVncServer(hostname,password,port);
    m_hostname = hostname;
    m_port = port;
    m_password = password;
    return 0;
}

void SessionsWindow::reconnect(void) {
    switch (type) {
        case LocalShell:
            //TODO: reconnect
            break;
        case Telnet:
            if(telnet->isConnected()) telnet->disconnectFromHost();
            startTelnetSession(m_hostname, m_port, m_type);
            break;
        case Serial:
            if(serialPort->isOpen()) serialPort->close();
            startSerialSession(m_portName, m_baudRate, m_dataBits, m_parity, m_stopBits, m_flowControl, m_xEnable);
            break;
        case RawSocket:
            if(rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
            startRawSocketSession(m_hostname, m_port);
            break;
        case NamePipe:
            if(namePipe->state() == QLocalSocket::ConnectedState) namePipe->disconnectFromServer();
            startNamePipeSession(m_pipeName);
            break;
        case SSH2:
            //TODO: reconnect
            break;
        case VNC:
            vncClient->disconnectFromVncServer();
            startVNCSession(m_hostname, m_port, m_password);
            break;
    }
}

void SessionsWindow::setWorkingDirectory(const QString &dir)
{
    workingDirectory = dir;
}

int SessionsWindow::setLog(bool enable) {
    int ret = -1;
    log_file_mutex.lock(); 
    if(enable) {
        if(log_file == nullptr) {
            QString savefile_name = FileDialog::getSaveFileName(term, tr("Save log..."),
                QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".log", tr("log files (*.log)"));
            if (!savefile_name.isEmpty()) {
                log_file = new QFile(savefile_name);
                if (!log_file->open(QIODevice::WriteOnly|QIODevice::Text)) {
                    QMessageBox::warning(term, tr("Save log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
                    delete log_file;
                    log_file = nullptr;
                    enableLog = false;
                } else {
                    enableLog = true;
                    ret = 0;
                }
            } else {
                enableLog = false;
            }
        }
    } else {
        if(log_file != nullptr) {
            log_file->close();
            delete log_file;
            log_file = nullptr;
        }
        enableLog = false;
        ret = 0;
    }
    log_file_mutex.unlock();
    return ret;
}

int SessionsWindow::setRawLog(bool enable) {
    int ret = -1;
    raw_log_file_mutex.lock(); 
    if(enable) {
        if(raw_log_file == nullptr) {
            QString savefile_name = FileDialog::getSaveFileName(term, tr("Save Raw log..."),
                QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".bin", tr("binary files (*.bin)"));
            if (!savefile_name.isEmpty()) {
                raw_log_file = new QFile(savefile_name);
                if (!raw_log_file->open(QIODevice::WriteOnly)) {
                    QMessageBox::warning(term, tr("Save Raw log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
                    delete raw_log_file;
                    raw_log_file = nullptr;
                    enableRawLog = false;
                } else {
                    enableRawLog = true;
                    ret = 0;
                }
            } else {
                enableRawLog = false;
            }
        }
    } else {
        if(raw_log_file != nullptr) {
            raw_log_file->close();
            delete raw_log_file;
            raw_log_file = nullptr;
        }
        enableRawLog = false;
        ret = 0;
    }
    raw_log_file_mutex.unlock();
    return ret;
}

int SessionsWindow::saveLog(const char *data, int size) {
    int ret = 0;
    if(enableLog) {
        if(log_file_mutex.tryLock()) {
            if(log_file != nullptr) {
                ret = log_file->write(data, size);
                if(fflush_file) {
                    log_file->flush();
                }
            }
            log_file_mutex.unlock();
        }
    }
    return ret;
}

int SessionsWindow::saveRawLog(const char *data, int size) {
    int ret = 0;
    if(enableRawLog) {
        if(raw_log_file_mutex.tryLock()) {
            if(raw_log_file != nullptr) {
                ret = raw_log_file->write(data, size);
                if(fflush_file) {
                    raw_log_file->flush();
                }
            }
            raw_log_file_mutex.unlock();
        }
    }
    return ret;
}

void SessionsWindow::lockSession(QString password) {
    password_hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    locked = true;
    term->setLocked(locked);
    emit stateChanged(Locked);
}

void SessionsWindow::unlockSession(QString password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    if(hash == password_hash) {
        password_hash = QByteArray();
        locked = false;
        term->setLocked(locked);
        emit stateChanged(Locked);
    } else {
        QMessageBox::warning(term, tr("Unlock Session"), tr("Wrong password!"));
    }
}

void SessionsWindow::sendFileUseZModem(QStringList fileList) {
    if(term) {
        QSendZmodem *sz = new QSendZmodem(-1,this);
        connect(sz,&QSendZmodem::sendData,this,&SessionsWindow::modemProxySendData);
        connect(this,&SessionsWindow::modemProxyRecvData,sz,&QSendZmodem::onRecvData);

        connect(sz,&QSendZmodem::transferring,this,[=](QString filename){
            QFileInfo info(filename);
            QString msg = QString("Transferring: %1...\r\n").arg(info.fileName());
            QByteArray data = msg.toUtf8();
            term->recvData(data.data(), data.size());
        });
        connect(sz,&QSendZmodem::complete,this,[=](QString filename, int result, size_t size, time_t date){
            QString msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed");
            QByteArray data = msg.toUtf8();
            term->recvData(data.data(), data.size());
        });
        connect(sz,&QSendZmodem::tick,this,[=](const char *fname, long bytes_sent, long bytes_total, long last_bps,
                                                int min_left, int sec_left, bool *ret){
            QString msg;
            float progress = (float)bytes_sent / (float)bytes_total * 100;
            if(progress > 100) progress = 100;
            long last_bs = last_bps/8;
            if(last_bs < 1024) {
                msg = QString("\033[2K\r%1\% %4B/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs).arg(min_left).arg(sec_left);
            } else if(last_bs < 1024*1024) {
                msg = QString("\033[2K\r%1\% %4KB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024).arg(min_left).arg(sec_left);                
            } else if(last_bs < 1024*1024*1024) {
                msg = QString("\033[2K\r%1\% %4MB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024/1024).arg(min_left).arg(sec_left);
            } else {
                msg = QString("\033[2K\r%1\% %4GB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024/1024/1024).arg(min_left).arg(sec_left);
            }
            QByteArray data = msg.toUtf8();
            term->recvData(data.data(), data.size());
            *ret = true;
        },Qt::BlockingQueuedConnection);

        sz->setFilePath(fileList,fileList);
        connect(sz,&QSendZmodem::finished,this,[=]{
            modemProxy = false;
            sz->deleteLater();
        });
        modemProxy = true;
        char test[] = "\r\nStarting zmodem transfer...\r\n";
        term->recvData(test,2);
        sz->start();
    }
}

SshSFtp *SessionsWindow::getSshSFtpChannel(void) {
    if(type == SSH2) {
        return ssh2Client->getChannel<SshSFtp>("quardCRT.sftp");
    }
    return nullptr;
}

bool SessionsWindow::hasChildProcess() {
    if(type == LocalShell) {
        return localShell->hasChildProcess();
    } else {
        return false;
    }
}

SessionsWindow::StateInfo SessionsWindow::getStateInfo(void) {
    StateInfo info;
    info.type = type;
    info.state = state;
    switch (type) {
        case Telnet:
            info.telnet.tx_total = tx_total;
            info.telnet.rx_total = rx_total;
            break;
        case Serial:
            info.serial.tx_total = tx_total;
            info.serial.rx_total = rx_total;
            break;
        case LocalShell:
            info.localShell.tree = localShell->processInfoTree();
            break;
        case RawSocket:
            info.rawSocket.tx_total = tx_total;
            info.rawSocket.rx_total = rx_total;
            break;
        case NamePipe:
            break;
        case SSH2:
            info.ssh2.tx_total = tx_total;
            info.ssh2.rx_total = rx_total;
            break;
        case VNC:
            break;
    }
    return info;
}

void SessionsWindow::refeshTermSize(void) {
    if(type == LocalShell) {
        localShell->resize(term->columns(),term->lines());
    }
}
