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
#include <QSerialPortInfo>

#include "mainwindow.h"
#include "qsendkermit.h"
#include "qrecvkermit.h"
#include "qxymodem.h"
#include "qsendzmodem.h"
#include "qrecvzmodem.h"
#ifdef ENABLE_SSH
#include "sshshell.h"
#endif
#include "filedialog.h"
#include "sessionswindow.h"
#include "globaloptionswindow.h"
#include "globalsetting.h"
#include "argv_split.h"
#include "misc.h"
#include "qextserialenumerator.h"

QString SessionsWindow::saveRecordingPath = QDir::homePath();

SessionsWindow::SessionsWindow(SessionType tp, QWidget *parent)
    : QObject(parent)
    , type(tp)
    , workingDirectory(QDir::homePath())
    , showTitleType(LongTitle)
    , messageParentWidget(parent)
    , term(nullptr)
    , telnet(nullptr)
    , serialPort(nullptr)
    , serialMonitor(nullptr)
    , rawSocket(nullptr)
    , localShell(nullptr)
    , namePipe(nullptr)
#ifdef ENABLE_SSH
    , ssh2Client(nullptr)
#endif
    , vncClient(nullptr)
    , enableLog(false)
    , enableRawLog(false)
    , enableRecordingScript(false)
    , enableBroadCast(false) {
    zmodemUploadPath = QDir::homePath();
    zmodemDownloadPath = QDir::homePath();
    shellType = Unknown;
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
        term = new QTermWidget(parent,parent);
        term->setProperty("session", QVariant::fromValue(this));

        term->setScrollBarPosition(QTermWidget::ScrollBarRight);
        term->setBlinkingCursor(true);
        term->setMargin(0);
        term->setDrawLineChars(false);
        term->setSelectionOpacity(0.5);

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
            showTitleType = ShortTitle;
            localShell = PtyQt::createPtyProcess();
            connect(term, &QTermWidget::termSizeChange, this, [=](int lines, int columns){
                localShell->resize(columns,lines);
            });
            break;
        }
        case Telnet: {
            telnet = new QTelnet(QTelnet::TCP, this);
            realtimespeed_timer = new QTimer(this);
            connect(telnet,&QTelnet::newData,this,[=](const char *data, int size){
                QByteArray dataBa(data,size);
                if(doRecvData(dataBa)) {
                    rx_total += size;
                    rx_realtime += size;
                    term->recvData(dataBa.data(), dataBa.size());
                }
            });
            connect(this,&SessionsWindow::modemProxySendData,this, [=](QByteArray data){
                if(modemProxyChannel) {
                    telnet->sendData(data.data(), data.size());
                }
            });
            connect(term, &QTermWidget::sendData,this,[=](const char *data, int size){
                QByteArray sendData(data, size);
                if(doSendData(sendData,telnet->isConnected())){
                    telnet->sendData(data, size);
                    tx_total += size;
                    tx_realtime += size;
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
                QMessageBox::warning(messageParentWidget, tr("Telnet Error"), getName() + "\n" + tr("Telnet error:\n%1.").arg(telnet->errorString()));
                state = Error;
                emit stateChanged(state);
                Q_UNUSED(socketError);
            });
            break;
        }
        case Serial: {
            serialPort = new QSerialPort(this);
            realtimespeed_timer = new QTimer(this);
            connect(serialPort,&QSerialPort::readyRead,this,[=](){
                QByteArray data = serialPort->readAll();
                uint64_t size = data.size();
                if(doRecvData(data)) {
                    rx_total += size;
                    rx_realtime += size;
                    term->recvData(data.data(), data.size());
                }
            });
            connect(this,&SessionsWindow::modemProxySendData,this,[=](QByteArray data){
                if(modemProxyChannel) {
                    if(state == Connected && serialPort->isOpen()) {
                        serialPort->write(data.data(), data.size());
                    }
                }
            });
            connect(term, &QTermWidget::sendData,this,[=](const char *data, int size){
                QByteArray sendData(data, size);
                if(doSendData(sendData,(state == Connected && serialPort->isOpen()))){
                    serialPort->write(data, size);
                    tx_total += size;
                    tx_realtime += size;
                }
            });
            connect(serialPort, &QSerialPort::errorOccurred, this, [=](QSerialPort::SerialPortError serialPortError){
                if(serialPort->error() == QSerialPort::NoError) return;
                if(state == Error) return;
                QMessageBox::warning(messageParentWidget, tr("Serial Error"), getName() + "\n" + tr("Serial error:\n%0\n%1.").arg(serialPort->portName()).arg(serialPort->errorString()));
                state = Error;
                emit stateChanged(state);
                serialPort->close();
                Q_UNUSED(serialPortError);
            });
            serialMonitor = new QextSerialEnumerator();
            serialMonitor->setUpNotifications();
            break;
        }
        case RawSocket: {
            rawSocket = new QTcpSocket(this);
            realtimespeed_timer = new QTimer(this);
            connect(rawSocket,&QTcpSocket::readyRead,this,[=](){
                QByteArray data = rawSocket->readAll();
                uint64_t size = data.size();
                if(doRecvData(data)) {
                    rx_total += size;
                    rx_realtime += size;
                    term->recvData(data.data(), data.size());
                }
            });
            connect(this,&SessionsWindow::modemProxySendData,this,[=](QByteArray data){
                if(modemProxyChannel) {
                    if(rawSocket->state() == QAbstractSocket::ConnectedState) {
                        rawSocket->write(data.data(), data.size());
                    }
                }
            });
            connect(term, &QTermWidget::sendData,this,[=](const char *data, int size){
                QByteArray sendData(data, size);
                if(doSendData(sendData,(rawSocket->state() == QAbstractSocket::ConnectedState))){
                    rawSocket->write(data, size);
                    tx_total += size;
                    tx_realtime += size;
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
                QMessageBox::warning(messageParentWidget, tr("Raw Socket Error"), getName() + "\n" + tr("Raw Socket error:\n%1.").arg(rawSocket->errorString()));
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
                if(doRecvData(data)) {
                    term->recvData(data.data(), data.size());
                }
            });
            connect(this,&SessionsWindow::modemProxySendData,this,[=](QByteArray data){
                if(modemProxyChannel) {
                    if(namePipe->state() == QLocalSocket::ConnectedState) {
                        namePipe->write(data.data(), data.size());
                    }
                }
            });
            connect(term, &QTermWidget::sendData,this,[=](const char *data, int size){
                QByteArray sendData(data, size);
                if(doSendData(sendData,(namePipe->state() == QLocalSocket::ConnectedState))){
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
                QMessageBox::warning(messageParentWidget, tr("Name Pipe Error"), getName() + "\n" + tr("Name Pipe error:\n%1.").arg(namePipe->errorString()));
                state = Error;
                emit stateChanged(state);
                Q_UNUSED(socketError);
            });
            break;
        }
        case SSH2: {
        #ifdef ENABLE_SSH
            ssh2Client = new SshClient("ssh2",this);
            realtimespeed_timer = new QTimer(this);
            connect(ssh2Client, &SshClient::sshReady, this, [=](){
                SshShell *shell = ssh2Client->getChannel<SshShell>("quardCRT.shell");
                if(shell == nullptr) {
                    QMessageBox::warning(messageParentWidget, tr("SSH2 Error"), getName() + "\n" + tr("SSH2 error:\n%1.").arg(ssh2Client->sshErrorString()));
                    state = Error;
                    emit stateChanged(state);
                    connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
                        QByteArray sendData(data, size);
                        if(sendData.contains("\r") || sendData.contains("\n")) {
                            if(!m_requestReconnect) {
                                m_requestReconnect = true;
                                emit requestReconnect();
                            }
                        }
                    });
                    return;
                }
                shell->initSize(term->columns(),term->lines());
                ssh2Client->startChannel<SshShell>(shell);
                connect(term, &QTermWidget::termSizeChange, this, [=](int lines, int columns){
                    shell->resize(columns,lines);
                });
                connect(shell, &SshShell::readyRead, this, [=](const char *data, int size){
                    QByteArray dataBa(data, size);
                    if(doRecvData(dataBa)) {
                        rx_total += size;
                        rx_realtime += size;
                        term->recvData(dataBa.data(), dataBa.size());
                    }
                });
                connect(this,&SessionsWindow::modemProxySendData,this,[=](QByteArray data){
                    if(modemProxyChannel) {
                        shell->sendData(data.data(), data.size());
                    }
                });
                connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
                    QByteArray sendData(data, size);
                    if(doSendData(sendData,(state == Connected))){
                        shell->sendData(data, size);
                        tx_total += size;
                        tx_realtime += size;
                    }
                });
                connect(shell, &SshShell::failed, this, [=](){
                    disconnect();
                });
                connect(shell, &SshShell::finished, this, [=](){
                    disconnect();
                });
                state = Connected;
                emit stateChanged(state);
            });
            connect(ssh2Client, &SshClient::sshDisconnected, this, [=](){
                state = Disconnected;
                emit stateChanged(state);
            });
            connect(ssh2Client, &SshClient::sshError, this, [=](){
                QMessageBox::warning(messageParentWidget, tr("SSH2 Error"), getName() + "\n" + tr("SSH2 error:\n%1.").arg(ssh2Client->sshErrorString()));
                state = Error;
                emit stateChanged(state);
                connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
                    QByteArray sendData(data, size);
                    if(sendData.contains("\r") || sendData.contains("\n")) {
                        if(!m_requestReconnect) {
                            m_requestReconnect = true;
                            emit requestReconnect();
                        }
                    }
                });
            });
        #endif
            break;
        }
        case VNC:
        default:
            break;
    }

    if(type != VNC) {
        connect(term, &QTermWidget::titleChanged, this, &SessionsWindow::titleChanged);
        connect(term, &QTermWidget::termGetFocus, this, &SessionsWindow::termGetFocus);
        connect(term, &QTermWidget::termLostFocus, this, &SessionsWindow::termLostFocus);
        connect(term, &QTermWidget::dupDisplayOutput, this, [&](const char *data, int size){
            saveLog(data, size);
            writeReceiveASCIIFile(data, size);
            if(enableRecordingScript) {
                QByteArray dataBa(data,size);
                QMutexLocker locker(&recording_script_recv_mutex);
                recordingScriptRecvBuffer.append(data);
                recordingScriptRecvBuffer.replace("\r\n","\n");
                recordingScriptRecvBuffer.replace("\r","\n");
                do {
                    int pos = recordingScriptRecvBuffer.indexOf('\n');
                    if(pos!=-1) {
                        QString line = QString::fromUtf8(recordingScriptRecvBuffer.left(pos+1));
                        recordingScriptRecvBuffer.remove(0, pos + 1);
                        addToRecordingScript(1,line);
                    } else {
                        break;
                    }
                }while(1);
            }
        });
        connect(term, &QTermWidget::urlActivated, this, [&](const QUrl& url, uint32_t opcode){
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
        #if defined(Q_OS_WIN)
            else if(path.startsWith("file://") ) {
                if(getSessionType() == LocalShell) {
                    if(getShellType() == WSL) {
                        path.remove("file://");
                        static QRegularExpression wslDirFormat("^/mnt/(\\S+)/(.*)$");
                        if(wslDirFormat.match(path).hasMatch()) {
                            QString pathFix = wslDirFormat.match(path).captured(1).toUpper()+":/"+wslDirFormat.match(path).captured(2);
                            u = QUrl::fromLocalFile(pathFix);
                        } else if(path.startsWith("/mnt/")) {
                            u = QUrl::fromLocalFile(path.remove(0, 5).toUpper()+":/");
                        }
                    }
                }
            }
        #endif
            QString target = u.toString();
            if(target.startsWith("file://")) {
                target.remove("file://");
                // check endsWith '$' '#'
                if(target.endsWith('$') || target.endsWith('#')) {
                    // check file exists
                    // FIXME: That's not a good way to fix filePath match issue
                    if(!QFile::exists(target)) {
                        target.chop(1);
                        u = QUrl::fromLocalFile(target);
                    }
                }
            }
            switch(opcode) {
                case QTermWidget::OpenFromContextMenu:
                case QTermWidget::OpenFromClick: {
                    bool ret = QDesktopServices::openUrl(u);
                    if(!ret) {
                        QMessageBox::warning(messageParentWidget, tr("Open URL"), tr("Cannot open URL %1.").arg(u.toString()));
                    }
                    break;
                }
                case QTermWidget::OpenContainingFromContextMenu: {
                    QFileInfo fileInfo(u.toLocalFile());
                    QDir dir = fileInfo.dir();
                    if(dir.exists()) {
                        bool ret = QDesktopServices::openUrl(QUrl::fromLocalFile(dir.path()));
                        if(!ret) {
                            QMessageBox::warning(messageParentWidget, tr("Open URL"), tr("Cannot open URL %1.").arg(u.toString()));
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        });
        // only windows and macos need do this, because linux support Selection Clipboard by default
        bool supportSelection = QApplication::clipboard()->supportsSelection();
        if(!supportSelection) {
            connect(term, &QTermWidget::mousePressEventForwarded, this, [&](QMouseEvent *event) {
                if(event->button() == Qt::MiddleButton) {
                    term->copyClipboard();
                    term->pasteClipboard();
                }
            });
        }
        connect(term, &QTermWidget::zmodemSendDetected, this, [&](){
            if(zmodemOnlie) {
                modemProxyChannelMutex.lock();
                if(modemProxyChannel) {
                    modemProxyChannelMutex.unlock();
                    return;
                }
                modemProxyChannel = true;
                modemProxyChannelMutex.unlock();
                QString msg = QString("\033[2K\rZModem transfer detected");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                modemProxyChannelMutex.lock();
                modemProxyChannel = false;
                modemProxyChannelMutex.unlock();
                recvFileUseZModem(zmodemDownloadPath);
            }
        });
    #if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
        connect(term, &QTermWidget::handleCtrlC, this, [&](void){
            QString text = term->selectedText();
            if(!text.isEmpty()) {
                QApplication::clipboard()->setText(text, QClipboard::Clipboard);
            }
        });
    #endif
        connect(term, &QTermWidget::zmodemRecvDetected, this, [&,parent](){
            if(zmodemOnlie) {
                modemProxyChannelMutex.lock();
                if(modemProxyChannel) {
                    modemProxyChannelMutex.unlock();
                    return;
                }
                modemProxyChannel = true;
                modemProxyChannelMutex.unlock();
                QString msg = QString("\033[2K\rZModem transfer detected");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);

                // FIXME: we should not use CentralWidget here
                CentralWidget *mainwindow = static_cast<CentralWidget*>(parent);
                QStringList files = mainwindow->requestZmodemUploadList();

                if(files.isEmpty())
                    files = FileDialog::getItemsPathsWithPickBox(term, tr("Select Files to Send using Zmodem"), zmodemUploadPath, tr("All Files (*)"));
                // if files is empty, we also start zmodem send, it will fast abort
                modemProxyChannelMutex.lock();
                modemProxyChannel = false;
                modemProxyChannelMutex.unlock();
                sendFileUseZModem(files); 
            }
        });
    }

    if(realtimespeed_timer) {
        connect(realtimespeed_timer, &QTimer::timeout, this, [=](){
            tx_speed = tx_realtime/3.0;
            rx_speed = rx_realtime/3.0;
            rx_realtime = 0;
            tx_realtime = 0;
        });
        realtimespeed_timer->start(3000);
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
        if (serialPort->isOpen()) {
            serialPort->clear();
            serialPort->waitForBytesWritten(1000);
            serialPort->close();
        }
        delete serialPort;
        delete serialMonitor;
    }
    if(rawSocket){
        if(rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
        delete rawSocket;
    }
    if(namePipe){
        if(namePipe->state() == QLocalSocket::ConnectedState) namePipe->disconnectFromServer();
        delete namePipe;
    }
#ifdef ENABLE_SSH
    if(ssh2Client) {
        ssh2Client->disconnectFromHost();
        delete ssh2Client;
    }
#endif
    if(realtimespeed_timer) {
        delete realtimespeed_timer;
    }
    if(vncClient) {
        vncClient->disconnectFromVncServer();
        delete vncClient;
    }
    if(term) {
        delete term;
    }
}

void SessionsWindow::matchString(QByteArray data) {
    if(!m_waitStringList.isEmpty()) {
        if(m_waitStringCaseInsensitive) {
            foreach(const QString &waitString, m_waitStringList) {
                QString dataStr = QString::fromUtf8(data).toLower();
                if(dataStr.contains(waitString.toLower())) {
                    if(m_waitStringMode == 0) {
                        int matchIndex = m_waitStringList.indexOf(waitString);
                        QString matchStr = waitString;
                        m_waitStringList.clear();
                        emit waitForStringFinished(matchStr, matchIndex);
                        break;
                    } else {
                        int index = dataStr.indexOf(waitString.toLower());
                        m_waitStringDate.append(data.left(index + waitString.length()));
                        int matchIndex = m_waitStringList.indexOf(waitString);
                        QString matchStr(m_waitStringDate);
                        m_waitStringList.clear();
                        m_waitStringDate.clear();
                        emit waitForStringFinished(matchStr, matchIndex);
                        break;
                    }
                }
            }
            if(m_waitStringMode == 1) {
                m_waitStringDate.append(data);
            }
        } else {
            foreach(const QString &waitString, m_waitStringList) {
                if(data.contains(waitString.toUtf8())) {
                    if(m_waitStringMode == 0) {
                        int matchIndex = m_waitStringList.indexOf(waitString);
                        QString matchStr = waitString;
                        m_waitStringList.clear();
                        emit waitForStringFinished(matchStr, matchIndex);
                        break;
                    } else {
                        int index = data.indexOf(waitString.toUtf8());
                        m_waitStringDate.append(data.left(index + waitString.length()));
                        int matchIndex = m_waitStringList.indexOf(waitString);
                        QString matchStr(m_waitStringDate);
                        m_waitStringList.clear();
                        m_waitStringDate.clear();
                        emit waitForStringFinished(matchStr, matchIndex);
                        break;
                    }
                }
            }
            if(m_waitStringMode == 1) {
                m_waitStringDate.append(data);
            }
        }
    }
}

bool SessionsWindow::doSendData(QByteArray &data, bool isConnected) {
    if(modemProxyChannel) {
        return false;
    }
    if(enableBroadCast) {
        emit broadCastSendData(data);
    }
    if(!isConnected) {
        if(data.contains("\r") || data.contains("\n")) {
            if(!m_requestReconnect) {
                m_requestReconnect = true;
                emit requestReconnect();
            }
        }
        return false;
    }
    if(enableRecordingScript) {
        QMutexLocker locker(&recording_script_send_mutex);
        recordingScriptSendBuffer.append(data);
        recordingScriptSendBuffer.replace("\r\n","\n");
        recordingScriptSendBuffer.replace("\r","\n");
        do {
            int pos = recordingScriptSendBuffer.indexOf('\n');
            if(pos!=-1) {
                QString line = QString::fromUtf8(recordingScriptSendBuffer.left(pos + 1));
                recordingScriptSendBuffer.remove(0, pos + 1);
                addToRecordingScript(0,line);
            } else {
                break;
            }
        }while(1);
    }
    return true;
}

bool SessionsWindow::doRecvData(QByteArray &data) {
    if(modemProxyChannel) {
        emit modemProxyRecvData(data);
        return false;
    }
    matchString(data);
    saveRawLog(data.data(), data.size());
    emit hexDataDup(data.data(), data.size());
    switch(endOfLineSeq) {
        case LF:
            data.replace("\r","\r\n");
            break;
        case CR:
            data.replace("\n","\r\n");
            break;
        case LFLF:
            data.replace("\r\r","\r\n");
            break;
        case CRCR:
            data.replace("\n\n","\r\n");
            break;
        default:
            break;
    }
    return true;
}

void SessionsWindow::cloneSession(SessionsWindow *src, QString profile) {
    switch(src->getSessionType()) {
        case LocalShell: {
            setWorkingDirectory(src->getWorkingDirectory());
            startLocalShellSession(src->m_command, profile, src->getShellType());
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
        #ifdef ENABLE_SSH
            startSSH2Session(src->m_hostname, src->m_port, src->m_username, src->m_password);
        #endif
            break;
        case VNC:
            startVNCSession(src->m_hostname, src->m_port, src->m_password);
            break;
        default:
            break;
        }
    }  
}

int SessionsWindow::startLocalShellSession(const QString &command,QString profile, ShellType sTp) {
    QString shellPath;
    QStringList args;
    shellType = sTp;
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
        m_wslUserName = wslUserName;
        switch (shellType) {
            case PowerShell:
            default: {
                QString defaultLocalShell = setting.value("Global/Options/DefaultLocalShell", "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe").toString();
                QFileInfo shellInfo(defaultLocalShell);
                if(shellInfo.isExecutable()) {
                    shellPath = defaultLocalShell;
                } 
                if(shellPath.isEmpty()) shellPath = "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
                args =  {
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
            case WSL:
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
    QFileInfo fileInfo(workingDirectory);
    if(!fileInfo.exists() || !fileInfo.isDir()) {
        workingDirectory = QDir::homePath();
    }
    bool ret = localShell->startProcess(shellPath, args, workingDirectory, envs, term->screenColumnsCount(), term->screenLinesCount());
    if(!ret) {
        state = Error;
        emit stateChanged(state);
        QMessageBox::warning(messageParentWidget, tr("Start Local Shell"), getName() + "\n" + tr("Cannot start local shell:\n%1.").arg(localShell->lastError()));
        return -1;
    }
    connect(localShell->notifier(), &QIODevice::readyRead, this, [=](){
        QByteArray data = localShell->readAll();
        if(doRecvData(data)) {
            term->recvData(data.data(), data.size());
        }
    });
    connect(this,&SessionsWindow::modemProxySendData,this,[=](QByteArray data){
        if(modemProxyChannel) {
            localShell->write(data);
        }
    });
    connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
        QByteArray sendData(data, size);
        if(doSendData(sendData,true)){
            localShell->write(QByteArray(data, size));
        }
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
        #if defined(Q_OS_WIN)
        QString monitorPortName = serialPort->portName();
        #else
        QSerialPortInfo sinfo(*serialPort);
        QString monitorPortName = sinfo.systemLocation();
        #endif
        connect(serialMonitor, &QextSerialEnumerator::deviceRemoved, this, 
            [&,monitorPortName](const QextPortInfo &info) {
            if(monitorPortName == info.portName) {
                if(serialPort->isOpen()) {
                    serialPort->close();
                    QMessageBox::warning(messageParentWidget, tr("Serial Error"), getName() + "\n" + tr("Serial port %1 has been removed.").arg(info.portName));
                    state = Error;
                    emit stateChanged(state);
                }
            }
        });
        serialPort->setBreakEnabled(xEnable);
    }
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

#ifdef ENABLE_SSH
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
#endif

int SessionsWindow::startVNCSession(const QString &hostname, quint16 port, const QString &password) {
    vncClient->connectToVncServer(hostname,password,port);
    m_hostname = hostname;
    m_port = port;
    m_password = password;
    return 0;
}

void SessionsWindow::disconnect(void) {
    switch (type) {
        case LocalShell:
            localShell->kill();
            state = Disconnected;
            emit stateChanged(state);
            break;
        case Telnet:
            if(telnet->isConnected()) telnet->disconnectFromHost();
            break;
        case Serial:
            if(serialPort->isOpen()) serialPort->close();
            break;
        case RawSocket:
            if(rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
            break;
        case NamePipe:
            if(namePipe->state() == QLocalSocket::ConnectedState) namePipe->disconnectFromServer();
            break;
        case SSH2:
        #ifdef ENABLE_SSH
            ssh2Client->disconnectFromHost();
        #endif
            break;
        case VNC:
            vncClient->disconnectFromVncServer();
            break;
        default:
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
            QFileInfo path(saveRecordingPath);
            if(!path.isDir()) {
                saveRecordingPath = QDir::homePath();
            }
            QString savefile_name = FileDialog::getSaveFileName(term, tr("Save log..."),
                saveRecordingPath + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".log", tr("log files (*.log)"));
            if (!savefile_name.isEmpty()) {
                saveRecordingPath = QFileInfo(savefile_name).absolutePath();
                log_file = new QFile(savefile_name);
                if (!log_file->open(QIODevice::WriteOnly|QIODevice::Text)) {
                    QMessageBox::warning(messageParentWidget, tr("Save log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
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

QString SessionsWindow::getLogFileName(void) {
    QString ret;
    log_file_mutex.lock();
    if(log_file != nullptr) {
        ret = log_file->fileName();
    }
    log_file_mutex.unlock();
    return ret;
}

int SessionsWindow::setRawLog(bool enable) {
    int ret = -1;
    raw_log_file_mutex.lock(); 
    if(enable) {
        if(raw_log_file == nullptr) {
            QFileInfo path(saveRecordingPath);
            if(!path.isDir()) {
                saveRecordingPath = QDir::homePath();
            }
            QString savefile_name = FileDialog::getSaveFileName(term, tr("Save Raw log..."),
                saveRecordingPath + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".bin", tr("binary files (*.bin)"));
            if (!savefile_name.isEmpty()) {
                saveRecordingPath = QFileInfo(savefile_name).absolutePath();
                raw_log_file = new QFile(savefile_name);
                if (!raw_log_file->open(QIODevice::WriteOnly)) {
                    QMessageBox::warning(messageParentWidget, tr("Save Raw log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
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

QString SessionsWindow::getRawLogFileName(void) {
    QString ret;
    raw_log_file_mutex.lock();
    if(raw_log_file != nullptr) {
        ret = raw_log_file->fileName();
    }
    raw_log_file_mutex.unlock();
    return ret;
}

int SessionsWindow::startRecordingScript(void) {
    int ret = 0;
    recording_script_file_mutex.lock();
    enableRecordingScript = true;
    recordingScript.clear();
    recording_script_file_mutex.unlock();
    recording_script_recv_mutex.lock();
    recordingScriptRecvBuffer.clear();
    recording_script_recv_mutex.unlock();
    recording_script_send_mutex.lock();
    recordingScriptSendBuffer.clear();
    recording_script_send_mutex.unlock();
    return ret;
}

void SessionsWindow::addToRecordingScript(int type, QString str) {
    QMutexLocker locker(&recording_script_file_mutex);
    QPair<int, QByteArray> cmd(type,str.toUtf8());
    recordingScript.append(cmd);
}

void SessionsWindow::addToRecordingScript(int type, QByteArray ba) {
    QMutexLocker locker(&recording_script_file_mutex);
    QPair<int, QByteArray> cmd(type,ba);
    recordingScript.append(cmd);
}

int SessionsWindow::stopRecordingScript(void) {
    int ret = -1;
    QFileInfo path(saveRecordingPath);
    if(!path.isDir()) {
        saveRecordingPath = QDir::homePath();
    }
    QString savefile_name = FileDialog::getSaveFileName(term, tr("Save script..."),
        saveRecordingPath + QDate::currentDate().toString("/script-yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".py", tr("Python files (*.py)"));
    if (!savefile_name.isEmpty()) {
        saveRecordingPath = QFileInfo(savefile_name).absolutePath();
        QFile scriptFile(savefile_name);
        if (!scriptFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QMessageBox::warning(messageParentWidget, tr("Save script"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(scriptFile.errorString()));
        } else {
            QTextStream out(&scriptFile);
            
            recording_script_file_mutex.lock();
            out << "#!/usr/bin/env python3\n";
            out << "# -*- coding: utf-8 -*-\n";
            out << "#\n";
            out << "# $interface = \"1.0\"\n";
            out << "# Script generated by QuardCRT\n";
            out << "# Date: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            out << "#\n";
            out << "# This automatically generated script may need to be\n";
            out << "# edited in order to work correctly.\n";
            out << "\n";
            out << "import sys\n";
            out << "from quardCRT import crt\n";
            out << "\n";
            out << "def main():\n";
            out << "    crt.Screen.Synchronous = True\n";
            out << "\n";
            foreach(auto cmd, recordingScript) {
                QByteArray cmdData = cmd.second;
                cmdData.replace("\n","\\n");
                cmdData.replace("\r","\\r");
                cmdData.replace("\"", "\\\"");
                if(cmd.first == 0) {
                    out << "    crt.Screen.Send(\"" << cmdData << "\")\n";
                } else {
                    out << "    crt.Screen.WaitForString(\"" << cmdData << "\")\n";
                }
            }
            out << "\n";
            out << "if __name__ == \'__main__\':\n";
            out << "    main()\n";
            out << "\n";

            enableRecordingScript = false;
            recordingScript.clear();
            recording_script_file_mutex.unlock();
            recording_script_recv_mutex.lock();
            recordingScriptRecvBuffer.clear();
            recording_script_recv_mutex.unlock();
            recording_script_send_mutex.lock();
            recordingScriptSendBuffer.clear();
            recording_script_send_mutex.unlock();
            ret = 0;
            scriptFile.close();
        }
    }
    return ret;
}

int SessionsWindow::canlcelRecordingScript(void) {
    int ret = 0;
    recording_script_file_mutex.lock();
    enableRecordingScript = false;
    recordingScript.clear();
    recording_script_file_mutex.unlock();
    recording_script_recv_mutex.lock();
    recordingScriptRecvBuffer.clear();
    recording_script_recv_mutex.unlock();
    recording_script_send_mutex.lock();
    recordingScriptSendBuffer.clear();
    recording_script_send_mutex.unlock();
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
        QMessageBox::warning(messageParentWidget, tr("Unlock Session"), tr("Wrong password!"));
    }
}

void SessionsWindow::setInBroadCastList(bool enable) {
    enableBroadCast = enable; 
    emit stateChanged(BroadCasted);
}

int SessionsWindow::writeReceiveASCIIFile(const char *data, int size) {
    if(receiveASCIIFile) {
        QMutexLocker locker(&receiveASCIIFileMutex);
        if(receiveASCIIFileFd == nullptr) {
            return -1;
        }
        receiveASCIIFileFd->write(data, size);
        return 0;
    }
    return -1;
}

int SessionsWindow::startReceiveASCIIFile(const QString &fileName) {
    QMutexLocker locker(&receiveASCIIFileMutex);
    if(receiveASCIIFile) {
        return -1;
    }
    receiveASCIIFile = true;
    receiveASCIIFileFd = new QFile(fileName);
    if(!receiveASCIIFileFd->open(QIODevice::WriteOnly)) {
        QMessageBox::warning(messageParentWidget, tr("Receive ASCII File"), tr("Cannot write file %1:\n%2.").arg(fileName).arg(receiveASCIIFileFd->errorString()));
        receiveASCIIFile = false;
        return -1;
    }
    return 0;
}

int SessionsWindow::stopReceiveASCIIFile(void) {
    QMutexLocker locker(&receiveASCIIFileMutex);
    if(receiveASCIIFile) {
        receiveASCIIFileFd->close();
        delete receiveASCIIFileFd;
        receiveASCIIFile = false;
        return 0;
    }
    return -1;
}

bool SessionsWindow::isReceiveASCIIFile(void) {
    return receiveASCIIFile;
}

void SessionsWindow::reverseProxySendData(QByteArray data) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            modemProxyChannel = true;
            emit modemProxySendData(data);
            modemProxyChannel = false;
        }
    }
}

void SessionsWindow::sendFileUseKermit(QStringList fileList) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QSendKermit *sk = new QSendKermit(10,this); //10s timeout
            connect(sk,&QSendKermit::sendData,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,sk,&QSendKermit::onRecvData);
            sk->setFilePathList(fileList);
            connect(term, &QTermWidget::sendData, sk, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        sk->requestStop();
                    }
                }
            });
            connect(sk,&QSendKermit::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                sk->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting Kermit transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                sk->start();
            });
        }
    }
}

void SessionsWindow::recvFileUseKermit(const QString &downloadPath) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QRecvKermit *rk = new QRecvKermit(10,this); //10s timeout
            rk->setFileDirPath(downloadPath);
            connect(rk,&QRecvKermit::sendData,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,rk,&QRecvKermit::onRecvData);
            connect(term, &QTermWidget::sendData, rk, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        rk->requestStop();
                    }
                }
            });
            connect(rk,&QRecvKermit::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                rk->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting Kermit transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                rk->start();
            });
        }
    }
}

void SessionsWindow::sendFileUseXModem(QString file, bool modem1KMode) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QXmodemFile *xs = new QXmodemFile(file,modem1KMode?1024:128,10*1000,16,false,this);
            connect(xs,&QXmodemFile::send,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,xs,&QXmodemFile::receive);
            connect(xs,&QXmodemFile::transferring,this,[=](QString filename){
                QString msg = QString("Transferring: %1...\r\n").arg(filename);
                QByteArray data = msg.toUtf8();
                term->recvData(data.data(), data.size());
            });
            connect(xs,&QXmodemFile::complete,this,[=](QString filename,int result, size_t size){
                QString msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(xs,&QXmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                QString msg;
                *ret = !stopModemProxy;
                if(stopModemProxy) {
                    msg = QString("\033[2K\rTransfer aborted\r\n");
                } else {
                    float progress = (float)bytes_sent / (float)bytes_total * 100;
                    if(progress > 100) progress = 100;
                    msg = QString("\033[2K\r%1%%").arg(progress, 0, 'f', 2);
                } 
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
            },Qt::BlockingQueuedConnection);
            connect(term, &QTermWidget::sendData, xs, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        xs->requestStop();
                    }
                }
            });
            connect(xs,&QRecvKermit::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                xs->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting XModem transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                xs->startSend();
            });
        }
    }
}

void SessionsWindow::recvFileUseXModem(QString file) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QXmodemFile *xr = new QXmodemFile(file,128,10*1000,16,false,this);
            connect(xr,&QXmodemFile::send,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,xr,&QXmodemFile::receive);
            connect(xr,&QRecvKermit::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                xr->deleteLater();
            });
            connect(xr,&QXmodemFile::transferring,this,[=](QString filename){
                QString msg = QString("Transferring: %1...\r\n").arg(filename);
                QByteArray data = msg.toUtf8();
                term->recvData(data.data(), data.size());
            });
            connect(xr,&QXmodemFile::complete,this,[=](QString filename,int result, size_t size){
                QString msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(xr,&QXmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                QString msg;
                *ret = !stopModemProxy;
                if(stopModemProxy) {
                    msg = QString("\033[2K\rTransfer aborted\r\n");
                } else {
                    float progress = (float)bytes_sent / (float)bytes_total * 100;
                    if(progress > 100) progress = 100;
                    msg = QString("\033[2K\r%1%%").arg(progress, 0, 'f', 2);
                } 
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
            },Qt::BlockingQueuedConnection);
            connect(term, &QTermWidget::sendData, xr, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        xr->requestStop();
                    }
                }
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting XModem transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                xr->startRecv();
            });
        }
    }
}

void SessionsWindow::sendFileUseYModem(QStringList fileList, bool modem1KMode) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QYmodemFile *ys = new QYmodemFile(fileList,modem1KMode?1024:128,10*1000,16,false,this);
            connect(ys,&QYmodemFile::send,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,ys,&QYmodemFile::receive);
            connect(ys,&QYmodemFile::transferring,this,[=](QString filename){
                QString msg = QString("Transferring: %1...\r\n").arg(filename);
                QByteArray data = msg.toUtf8();
                term->recvData(data.data(), data.size());
            });
            connect(ys,&QYmodemFile::complete,this,[=](QString filename,int result, size_t size){
                QString msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(ys,&QYmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                QString msg;
                *ret = !stopModemProxy;
                if(stopModemProxy) {
                    msg = QString("\033[2K\rTransfer aborted\r\n");
                } else {
                    float progress = (float)bytes_sent / (float)bytes_total * 100;
                    if(progress > 100) progress = 100;
                    msg = QString("\033[2K\r%1%%").arg(progress, 0, 'f', 2);
                } 
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
            },Qt::BlockingQueuedConnection);
            connect(term, &QTermWidget::sendData, ys, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        ys->requestStop();
                    }
                }
            });
            connect(ys,&QRecvKermit::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                ys->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting YModem transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                ys->startSend();
            });
        }
    }
}

void SessionsWindow::recvFileUseYModem(const QString &downloadPath) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QYmodemFile *yr = new QYmodemFile(downloadPath,128,10*1000,16,false,this);//10s
            connect(yr,&QYmodemFile::send,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,yr,&QYmodemFile::receive);
            connect(yr,&QYmodemFile::transferring,this,[=](QString filename){
                QString msg = QString("Transferring: %1...\r\n").arg(filename);
                QByteArray data = msg.toUtf8();
                term->recvData(data.data(), data.size());
            });
            connect(yr,&QYmodemFile::complete,this,[=](QString filename,int result, size_t size){
                QString msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(yr,&QYmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                QString msg;
                *ret = !stopModemProxy;
                if(stopModemProxy) {
                    msg = QString("\033[2K\rTransfer aborted\r\n");
                } else {
                    float progress = (float)bytes_sent / (float)bytes_total * 100;
                    if(progress > 100) progress = 100;
                    msg = QString("\033[2K\r%1%%").arg(progress, 0, 'f', 2);
                } 
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
            },Qt::BlockingQueuedConnection);
            connect(term, &QTermWidget::sendData, yr, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        yr->requestStop();
                    }
                }
            });
            connect(yr,&QRecvKermit::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                yr->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting YModem transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                yr->startRecv();
            });
        }
    }
}

void SessionsWindow::sendFileUseZModem(QStringList fileList) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QSendZmodem *sz = new QSendZmodem(10,this); //10s timeout
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
                proxyRecvData(data);
                Q_UNUSED(filename);
                Q_UNUSED(size);
                Q_UNUSED(date);
            });
            connect(sz,&QSendZmodem::tick,this,[=](const char *fname, long bytes_sent, long bytes_total, long last_bps,
                                                    int min_left, int sec_left, bool *ret){
                QString msg;
                *ret = !stopModemProxy;
                if(stopModemProxy) {
                    msg = QString("\033[2K\rTransfer aborted\r\n");
                } else {
                    float progress = (float)bytes_sent / (float)bytes_total * 100;
                    if(progress > 100) progress = 100;
                    long last_bs = last_bps/8;
                    if(last_bs < 1024) {
                        msg = QString("\033[2K\r%1%% %4B/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs).arg(min_left).arg(sec_left);
                    } else if(last_bs < 1024*1024) {
                        msg = QString("\033[2K\r%1%% %4KB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024).arg(min_left).arg(sec_left);
                    } else if(last_bs < 1024*1024*1024) {
                        msg = QString("\033[2K\r%1%% %4MB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024/1024).arg(min_left).arg(sec_left);
                    } else {
                        msg = QString("\033[2K\r%1%% %4GB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024/1024/1024).arg(min_left).arg(sec_left);
                    }
                } 
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(fname);
            },Qt::BlockingQueuedConnection);
            connect(term, &QTermWidget::sendData, sz, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        sz->requestStop();
                    }
                }
            });
            sz->setFilePath(fileList,fileList);
            connect(sz,&QSendZmodem::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                sz->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting zmodem transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                sz->start();
            });
        }
    }
}

void SessionsWindow::recvFileUseZModem(const QString &downloadPath) {
    if(term) {
        QMutexLocker locker(&modemProxyChannelMutex);
        if(!modemProxyChannel) {
            QRecvZmodem *rz = new QRecvZmodem(10,this); //10s timeout
            connect(rz,&QRecvZmodem::sendData,this,&SessionsWindow::modemProxySendData);
            connect(this,&SessionsWindow::modemProxyRecvData,rz,&QRecvZmodem::onRecvData);

            connect(rz,&QRecvZmodem::transferring,this,[=](QString filename){
                QFileInfo info(filename);
                QString msg = QString("Transferring: %1...\r\n").arg(info.fileName());
                QByteArray data = msg.toUtf8();
                term->recvData(data.data(), data.size());
            });
            connect(rz,&QRecvZmodem::complete,this,[=](QString filename, int result, size_t size, time_t date){
                QString msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed");
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(filename);
                Q_UNUSED(size);
                Q_UNUSED(date);
            });
            connect(rz,&QRecvZmodem::tick,this,[=](const char *fname, long bytes_sent, long bytes_total, long last_bps,
                                                    int min_left, int sec_left, bool *ret){
                QString msg;
                *ret = !stopModemProxy;
                if(stopModemProxy) {
                    msg = QString("\033[2K\rTransfer aborted\r\n");
                } else {
                    float progress = (float)bytes_sent / (float)bytes_total * 100;
                    if(progress > 100) progress = 100;
                    long last_bs = last_bps/8;
                    if(last_bs < 1024) {
                        msg = QString("\033[2K\r%1%% %4B/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs).arg(min_left).arg(sec_left);
                    } else if(last_bs < 1024*1024) {
                        msg = QString("\033[2K\r%1%% %4KB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024).arg(min_left).arg(sec_left);
                    } else if(last_bs < 1024*1024*1024) {
                        msg = QString("\033[2K\r%1%% %4MB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024/1024).arg(min_left).arg(sec_left);
                    } else {
                        msg = QString("\033[2K\r%1%% %4GB/s %5:%6").arg(progress, 0, 'f', 2).arg(last_bs/1024/1024/1024).arg(min_left).arg(sec_left);
                    }
                } 
                QByteArray data = msg.toUtf8();
                proxyRecvData(data);
                Q_UNUSED(fname);
            },Qt::BlockingQueuedConnection);
            connect(rz, &QRecvZmodem::approver, this, [=](const char *fname, size_t size, time_t date, bool *ret){
                *ret = true;
                Q_UNUSED(fname);
                Q_UNUSED(size);
                Q_UNUSED(date);
            },Qt::BlockingQueuedConnection);
            connect(term, &QTermWidget::sendData, rz, [=](const char *data, int size){
                if(modemProxyChannel) {
                    QByteArray s = QByteArray(data, size);
                    if(s.contains(3)) { //TODO: check if it is a good way to check ctrl+c
                        stopModemProxy = true;
                        rz->requestStop();
                    }
                }
            });
            rz->setFileDirPath(downloadPath);
            connect(rz,&QRecvZmodem::finished,this,[=]{
                QMutexLocker locker(&modemProxyChannelMutex);
                modemProxyChannel = false;
                stopModemProxy = false;
                rz->deleteLater();
            });
            stopModemProxy = false;
            modemProxyChannel = true;
            QByteArray test("\r\nStarting zmodem transfer... Use Ctrl+C to cancel\r\n");
            proxyRecvData(test);
            QTimer::singleShot(100, [=](){
                rz->start();
            });
        }
    }
}

#ifdef ENABLE_SSH
SshSFtp *SessionsWindow::getSshSFtpChannel(void) {
    if(type == SSH2) {
        SshSFtp *res = ssh2Client->getChannel<SshSFtp>("quardCRT.sftp");
        ssh2Client->startChannel<SshSFtp>(res);
        return res;
    }
    return nullptr;
}
#endif

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
    info.endOfLine = endOfLineSeq;
    switch (type) {
        case Telnet:
            info.telnet.tx_total = tx_total;
            info.telnet.rx_total = rx_total;
            info.telnet.tx_speed = tx_speed;
            info.telnet.rx_speed = rx_speed;
            break;
        case Serial:
            info.serial.tx_total = tx_total;
            info.serial.rx_total = rx_total;
            info.serial.tx_speed = tx_speed;
            info.serial.rx_speed = rx_speed;
            break;
        case LocalShell:
            if(state == Connected) {
                info.localShell.tree = localShell->processInfoTree();
            }
            break;
        case RawSocket:
            info.rawSocket.tx_total = tx_total;
            info.rawSocket.rx_total = rx_total;
            info.rawSocket.tx_speed = tx_speed;
            info.rawSocket.rx_speed = rx_speed;
            break;
        case NamePipe:
            break;
        case SSH2:
            info.ssh2.tx_total = tx_total;
            info.ssh2.rx_total = rx_total;
            info.ssh2.tx_speed = tx_speed;
            info.ssh2.rx_speed = rx_speed;
        #ifdef ENABLE_SSH
            info.ssh2.encryption_method = ssh2Client->getEncryptionMethod();
        #endif
            break;
        case VNC:
        default:
            break;
    }
    return info;
}

void SessionsWindow::refeshTermSize(void) {
    if(type == LocalShell) {
        localShell->resize(term->columns(),term->lines());
    }
}
