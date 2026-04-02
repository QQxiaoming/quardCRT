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
#include "sessionprotocol.h"

QString SessionsWindow::saveRecordingTempDirPath = QDir::homePath();

SessionsWindow::SessionsWindow(SessionType tp, QWidget *parent)
    : QObject(parent)
    , type(tp)
    , workingDirectory(QDir::homePath())
    , showTitleType(LongTitle)
    , messageParentWidget(parent)
    , term(nullptr)
    , enableLog(false)
    , enableRawLog(false)
    , enableRecordingScript(false)
    , enableBroadCast(false) {
    zmodemUploadPath = QDir::homePath();
    zmodemDownloadPath = QDir::homePath();
    shellType = Unknown;
    protocol = SessionProtocolRegistry::instance().create(type);
    if(!protocol) {
        QMessageBox::critical(messageParentWidget, "Error", "Failed to create session protocol");
        return;
    }

    if(isConsoleSession()) {
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

    protocol->initialize(this);

    if(isConsoleSession()) {
        setupTerminalStateConnections();
        setupTerminalOutputConnections();
        setupTerminalUrlActivationConnection();
        setupTerminalClipboardAndCtrlCConnections();
        setupTerminalZModemConnections(parent);
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
    if(protocol) {
        protocol->cleanup(this);
    }
    if(realtimespeed_timer) {
        delete realtimespeed_timer;
    }
    if(term) {
        delete term;
    }
}

void SessionsWindow::cloneSession(SessionsWindow *src, QString profile) {
    if(src && src->protocol) {
        const QVariantMap commonMeta = {
            {"hostname", src->m_hostname},
            {"port", src->m_port},
            {"shellType", static_cast<int>(src->shellType)},
            {"workingDirectory", src->workingDirectory}
        };
        src->protocol->cloneTo(this, commonMeta, src->protocol->exportMeta(), profile);
    }
}

void SessionsWindow::disconnect(void) {
    if(protocol) {
        protocol->disconnect(this);
    }
}

bool SessionsWindow::hasChildProcess() {
    if(protocol) {
        return protocol->hasChildProcess(this);
    }
    return false;
}

SessionsWindow::StateInfo SessionsWindow::getStateInfo(void) {
    StateInfo info;
    info.type = type;
    info.state = state;
    info.endOfLine = endOfLineSeq;
    if(protocol) {
        protocol->fillStateInfo(this, info);
    }
    return info;
}

void SessionsWindow::refeshTermSize(void) {
    if(protocol) {
        protocol->refreshTermSize(this);
    }
}

bool SessionsWindow::isConsoleSession() const {
    return protocol->category() == ConsoleSession;
}

bool SessionsWindow::isDesktopSession() const {
    return protocol->category() == DesktopSession;
}

QWidget *SessionsWindow::getMainWidget() const {
    if(isDesktopSession()) {
        QWidget *widget = protocol->mainWidget(const_cast<SessionsWindow *>(this));
        if(widget) {
            return widget;
        }
    }
    return static_cast<QWidget *>(term);
}

bool SessionsWindow::isTerminal() const {
    return isConsoleSession();
}

void SessionsWindow::setupTerminalStateConnections() {
    connect(term, &QTermWidget::titleChanged, this, &SessionsWindow::titleChanged);
    connect(term, &QTermWidget::termGetFocus, this, &SessionsWindow::termGetFocus);
    connect(term, &QTermWidget::termLostFocus, this, &SessionsWindow::termLostFocus);
}

void SessionsWindow::setupTerminalOutputConnections() {
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
            Q_UNUSED(dataBa);
        }
    });
}

void SessionsWindow::setupTerminalUrlActivationConnection() {
    connect(term, &QTermWidget::urlActivated, this, [&](const QUrl& url, uint32_t opcode){
        QUrl u = url;
        if(!protocol->preprocessActivatedUrl(this, u)) {
            return;
        }
        if(protocol->supportsUrlPostProcess(this)) {
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
}

void SessionsWindow::setupTerminalClipboardAndCtrlCConnections() {
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
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    connect(term, &QTermWidget::handleCtrlC, this, [&](void){
        QString text = term->selectedText();
        if(!text.isEmpty()) {
            QApplication::clipboard()->setText(text, QClipboard::Clipboard);
        }
    });
#endif
}

void SessionsWindow::setupTerminalZModemConnections(QWidget *parent) {
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

void SessionsWindow::setWorkingDirectory(const QString &dir)
{
    workingDirectory = dir;
}

void SessionsWindow::prepareString(QString &str) {
    //%H - hostname 
    //%S - session name 
    //%Y - 4-digit year
    //%M - 2-digit month
    //%D - 2-digit day 
    //%h - 2-digit hour 
    //%m - 2-digit minute
    //%s - 2-digit second
    //%t - 3-digit millisecond
    QDateTime now = QDateTime::currentDateTime();
    str.replace("%H", getHostname());
    str.replace("%S", getName());
    str.replace("%Y", now.toString("yyyy"));
    str.replace("%M", now.toString("MM"));
    str.replace("%D", now.toString("dd"));
    str.replace("%h", now.toString("hh"));
    str.replace("%m", now.toString("mm"));
    str.replace("%s", now.toString("ss"));
    str.replace("%t", now.toString("zzz"));
}

int SessionsWindow::setLog(bool enable) {
    int ret = -1;
    log_file_mutex.lock(); 
    if(enable) {
        if(log_file == nullptr) {
            QString savefile_name = logPath;
            prepareString(savefile_name);
            if(savefile_name.isEmpty()) {
                savefile_name = FileDialog::getSaveFileName(term, tr("Save log..."),
                    saveRecordingTempDirPath + "/" + getName() + QDate::currentDate().toString("-yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".log", tr("log files (*.log)"));
            }
            if (!savefile_name.isEmpty()) {
                saveRecordingTempDirPath = QFileInfo(savefile_name).absolutePath();
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
            QString savefile_name = rawLogPath;
            prepareString(savefile_name);
            if(savefile_name.isEmpty()) {
                savefile_name = FileDialog::getSaveFileName(term, tr("Save Raw log..."),
                    saveRecordingTempDirPath + "/" + getName() + QDate::currentDate().toString("-yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".bin", tr("binary files (*.bin)"));
            }
            if (!savefile_name.isEmpty()) {
                saveRecordingTempDirPath = QFileInfo(savefile_name).absolutePath();
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
    QString savefile_name = scriptLogPath;
    prepareString(savefile_name);
    if(savefile_name.isEmpty()) {
        savefile_name = FileDialog::getSaveFileName(term, tr("Save script..."),
            saveRecordingTempDirPath + "/script-" + getName() + QDate::currentDate().toString("-yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".py", tr("Python files (*.py)"));
    }
    if (!savefile_name.isEmpty()) {
        saveRecordingTempDirPath = QFileInfo(savefile_name).absolutePath();
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
                if(!add_time_on_each_line.isEmpty()) {
                    QString lineText = add_time_on_each_line;
                    prepareString(lineText);
                    log_file->write(lineText.toUtf8());
                }
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

void SessionsWindow::updateConnectionState(bool connected) {
    state = connected ? Connected : Disconnected;
    emit stateChanged(state);
}

void SessionsWindow::reportSessionError(const QString &title, const QString &message) {
    QMessageBox::warning(messageParentWidget, title, getName() + "\n" + message);
    state = Error;
    emit stateChanged(state);
}

void SessionsWindow::forwardReceivedData(QByteArray data, bool countTraffic) {
    if(doRecvData(data)) {
        if(countTraffic) {
            const uint64_t size = data.size();
            rx_total += size;
            rx_realtime += size;
        }
        term->recvData(data.data(), data.size());
    }
}

void SessionsWindow::setupModemProxyForward(const std::function<bool()> &canWrite,
                                            const std::function<void(const QByteArray &)> &writer) {
    connect(this, &SessionsWindow::modemProxySendData, this, [=](QByteArray data) {
        if(modemProxyChannel && canWrite()) {
            writer(data);
        }
    });
}

void SessionsWindow::setupTerminalSendForward(const std::function<bool()> &isConnected,
                                              const std::function<void(const char *, int)> &writer,
                                              bool countTraffic) {
    connect(term, &QTermWidget::sendData, this, [=](const char *data, int size) {
        QByteArray sendData(data, size);
        if(doSendData(sendData, isConnected())) {
            writer(data, size);
            if(countTraffic) {
                tx_total += size;
                tx_realtime += size;
            }
        }
    });
}

void SessionsWindow::beginModemTransfer(const QString &protocolName) {
    stopModemProxy = false;
    modemProxyChannel = true;
    QByteArray msg = QString("\r\nStarting %1 transfer... Use Ctrl+C to cancel\r\n").arg(protocolName).toUtf8();
    proxyRecvData(msg);
}

void SessionsWindow::finishModemTransfer(QObject *worker) {
    QMutexLocker locker(&modemProxyChannelMutex);
    modemProxyChannel = false;
    stopModemProxy = false;
    if(worker) {
        worker->deleteLater();
    }
}

void SessionsWindow::showTransferingFile(const QString &filename, bool useBaseName) {
    const QString displayName = useBaseName ? QFileInfo(filename).fileName() : filename;
    QByteArray msg = QString("Transferring: %1...\r\n").arg(displayName).toUtf8();
    term->recvData(msg.data(), msg.size());
}

void SessionsWindow::showTransferComplete(int result) {
    QByteArray msg = QString("\r\n%1\r\n").arg(result == 0 ? "successful" : "failed").toUtf8();
    proxyRecvData(msg);
}

void SessionsWindow::showSimpleTransferProgress(long bytesSent, long bytesTotal, bool *ret) {
    QString msg;
    *ret = !stopModemProxy;
    if(stopModemProxy) {
        msg = QString("\033[2K\rTransfer aborted\r\n");
    } else {
        float progress = (float)bytesSent / (float)bytesTotal * 100;
        if(progress > 100) progress = 100;
        msg = QString("\033[2K\r%1%%").arg(progress, 0, 'f', 2);
    }
    QByteArray data = msg.toUtf8();
    proxyRecvData(data);
}

void SessionsWindow::showZmodemTransferProgress(long bytesSent, long bytesTotal, long lastBps,
                                                int minLeft, int secLeft, bool *ret) {
    QString msg;
    *ret = !stopModemProxy;
    if(stopModemProxy) {
        msg = QString("\033[2K\rTransfer aborted\r\n");
    } else {
        float progress = (float)bytesSent / (float)bytesTotal * 100;
        if(progress > 100) progress = 100;
        long lastBs = lastBps / 8;
        if(lastBs < 1024) {
            msg = QString("\033[2K\r%1%% %4B/s %5:%6").arg(progress, 0, 'f', 2).arg(lastBs).arg(minLeft).arg(secLeft);
        } else if(lastBs < 1024 * 1024) {
            msg = QString("\033[2K\r%1%% %4KB/s %5:%6").arg(progress, 0, 'f', 2).arg(lastBs / 1024).arg(minLeft).arg(secLeft);
        } else if(lastBs < 1024 * 1024 * 1024) {
            msg = QString("\033[2K\r%1%% %4MB/s %5:%6").arg(progress, 0, 'f', 2).arg(lastBs / 1024 / 1024).arg(minLeft).arg(secLeft);
        } else {
            msg = QString("\033[2K\r%1%% %4GB/s %5:%6").arg(progress, 0, 'f', 2).arg(lastBs / 1024 / 1024 / 1024).arg(minLeft).arg(secLeft);
        }
    }
    QByteArray data = msg.toUtf8();
    proxyRecvData(data);
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
                finishModemTransfer(sk);
            });
            beginModemTransfer("Kermit");
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
                finishModemTransfer(rk);
            });
            beginModemTransfer("Kermit");
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
                showTransferingFile(filename);
            });
            connect(xs,&QXmodemFile::complete,this,[=](QString filename,int result, size_t size){
                showTransferComplete(result);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(xs,&QXmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                showSimpleTransferProgress(bytes_sent, bytes_total, ret);
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
                finishModemTransfer(xs);
            });
            beginModemTransfer("XModem");
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
                finishModemTransfer(xr);
            });
            connect(xr,&QXmodemFile::transferring,this,[=](QString filename){
                showTransferingFile(filename);
            });
            connect(xr,&QXmodemFile::complete,this,[=](QString filename,int result, size_t size){
                showTransferComplete(result);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(xr,&QXmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                showSimpleTransferProgress(bytes_sent, bytes_total, ret);
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
            beginModemTransfer("XModem");
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
                showTransferingFile(filename);
            });
            connect(ys,&QYmodemFile::complete,this,[=](QString filename,int result, size_t size){
                showTransferComplete(result);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(ys,&QYmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                showSimpleTransferProgress(bytes_sent, bytes_total, ret);
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
                finishModemTransfer(ys);
            });
            beginModemTransfer("YModem");
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
                showTransferingFile(filename);
            });
            connect(yr,&QYmodemFile::complete,this,[=](QString filename,int result, size_t size){
                showTransferComplete(result);
                Q_UNUSED(filename);
                Q_UNUSED(size);
            });
            connect(yr,&QYmodemFile::tick,this,[=](long bytes_sent, long bytes_total, bool *ret){
                showSimpleTransferProgress(bytes_sent, bytes_total, ret);
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
                finishModemTransfer(yr);
            });
            beginModemTransfer("YModem");
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
                showTransferingFile(filename, true);
            });
            connect(sz,&QSendZmodem::complete,this,[=](QString filename, int result, size_t size, time_t date){
                showTransferComplete(result);
                Q_UNUSED(filename);
                Q_UNUSED(size);
                Q_UNUSED(date);
            });
            connect(sz,&QSendZmodem::tick,this,[=](const char *fname, long bytes_sent, long bytes_total, long last_bps,
                                                    int min_left, int sec_left, bool *ret){
                showZmodemTransferProgress(bytes_sent, bytes_total, last_bps, min_left, sec_left, ret);
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
                finishModemTransfer(sz);
            });
            beginModemTransfer("zmodem");
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
                showTransferingFile(filename, true);
            });
            connect(rz,&QRecvZmodem::complete,this,[=](QString filename, int result, size_t size, time_t date){
                showTransferComplete(result);
                Q_UNUSED(filename);
                Q_UNUSED(size);
                Q_UNUSED(date);
            });
            connect(rz,&QRecvZmodem::tick,this,[=](const char *fname, long bytes_sent, long bytes_total, long last_bps,
                                                    int min_left, int sec_left, bool *ret){
                showZmodemTransferProgress(bytes_sent, bytes_total, last_bps, min_left, sec_left, ret);
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
                finishModemTransfer(rz);
            });
            beginModemTransfer("zmodem");
            QTimer::singleShot(100, [=](){
                rz->start();
            });
        }
    }
}

#ifdef ENABLE_SSH
SshSFtp *SessionsWindow::getSshSFtpChannel(void) {
    return protocol->getSshSFtpChannel(this);
}
#endif

QString SessionsWindow::getWSLUserName() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("wslUserName").toString();
}

QString SessionsWindow::getPortName() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("portName").toString();
}

QTelnet::SocketType SessionsWindow::getSocketType() const {
    if(!protocol) return QTelnet::TCP;
    return static_cast<QTelnet::SocketType>(protocol->exportMeta().value("socketType", QTelnet::TCP).toInt());
}

uint32_t SessionsWindow::getBaudRate() const {
    if(!protocol) return 0;
    return protocol->exportMeta().value("baudRate").toUInt();
}

int SessionsWindow::getDataBits() const {
    if(!protocol) return 0;
    return protocol->exportMeta().value("dataBits").toInt();
}

int SessionsWindow::getParity() const {
    if(!protocol) return 0;
    return protocol->exportMeta().value("parity").toInt();
}

int SessionsWindow::getStopBits() const {
    if(!protocol) return 0;
    return protocol->exportMeta().value("stopBits").toInt();
}

bool SessionsWindow::getFlowControl() const {
    if(!protocol) return false;
    return protocol->exportMeta().value("flowControl").toBool();
}

bool SessionsWindow::getXEnable() const {
    if(!protocol) return false;
    return protocol->exportMeta().value("xEnable").toBool();
}

QString SessionsWindow::getCommand() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("command").toString();
}

QString SessionsWindow::getPipeName() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("pipeName").toString();
}

QString SessionsWindow::getUserName() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("userName").toString();
}

QString SessionsWindow::getPassWord() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("password").toString();
}

int SessionsWindow::getSshAuthType() const {
    if(!protocol) return SshAuthPassword;
    return protocol->exportMeta().value("sshAuthType", SshAuthPassword).toInt();
}

QString SessionsWindow::getPrivateKeyPath() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("privateKeyPath").toString();
}

QString SessionsWindow::getPublicKeyPath() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("publicKeyPath").toString();
}

QString SessionsWindow::getPassphrase() const {
    if(!protocol) return QString();
    return protocol->exportMeta().value("passphrase").toString();
}

int SessionsWindow::getRawMode() const {
    return protocol->getRawMode();
}

void SessionsWindow::screenShot(const QString &fileName) {
    if(isConsoleSession()) {
        term->screenShot(fileName);
    } else if(isDesktopSession()) {
        protocol->screenShot(this,fileName);
    }
}

void SessionsWindow::screenShot(QPixmap *pixmap) {
    if(isConsoleSession()) {
        term->screenShot(pixmap);
    } else if(isDesktopSession()) {
        protocol->screenShot(this,pixmap);
    }
}

