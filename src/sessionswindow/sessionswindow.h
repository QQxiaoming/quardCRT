/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <qiaoqm@aliyun.com>
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
#ifndef SESSIONSWINDOW_H
#define SESSIONSWINDOW_H

#include <QObject>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QSerialPort>
#include <QLocalSocket>
#include <QMutex>
#include <QVariant>
#include <QMutexLocker>
#include <functional>
#include <memory>

#ifdef ENABLE_SSH
#include "sshclient.h"
#include "sshtunnelout.h"
#include "sshsftp.h"
#endif
#include "qtermwidget.h"
#include "QTelnet.h"
#include "ptyqt.h"
#include "qextserialenumerator.h"

class SessionProtocolBase;
namespace sessionprotocol {
class LocalShellProtocol;
class TelnetProtocol;
class SerialProtocol;
class RawSocketProtocol;
class NamePipeProtocol;
class SSH2Protocol;
class VNCProtocol;
}


class SessionsWindow : public QObject
{
    Q_OBJECT
    friend class sessionprotocol::LocalShellProtocol;
    friend class sessionprotocol::TelnetProtocol;
    friend class sessionprotocol::SerialProtocol;
    friend class sessionprotocol::RawSocketProtocol;
    friend class sessionprotocol::NamePipeProtocol;
    friend class sessionprotocol::SSH2Protocol;
    friend class sessionprotocol::VNCProtocol;
public:
    enum SessionType {
        Telnet = 0,
        Serial,
        LocalShell,
        RawSocket,
        NamePipe,
        SSH2,
        VNC,

        SessionTypeMax,
    };
    enum SessionCategory {
        ConsoleSession = 0,
        DesktopSession,
    };
    enum ShellType {
        UnixShell = 0,
        PowerShell,
        WSL,
        Unknown,

        ShellTypeMax,
    };
    enum SessionsState {
        Connected = 0,
        Disconnected,
        Locked,
        BroadCasted,
        Error,
    };
    enum EndOfLineSeq {
        AUTO = 0,
        LF,
        CR,
        LFLF,
        CRCR,

        EndOfLineSeqMax,
    };
    enum ShowTitleType {
        LongTitle = 0,
        ShortTitle,
        NameTitle,

        TitleTypeMax,
    };
    enum SshAuthMethod {
        SshAuthPassword = 0,
        SshAuthPublicKey,
    };
    struct StateInfo {
        SessionType type;
        SessionsState state;
        EndOfLineSeq endOfLine;
        struct TelnetState {
            uint64_t tx_total;
            uint64_t rx_total;
            qreal tx_speed;
            qreal rx_speed;
        } telnet;
        struct SerialState {
            uint64_t tx_total;
            uint64_t rx_total;
            qreal tx_speed;
            qreal rx_speed;
        } serial;
        struct LocalShellState {
            IPtyProcess::pidTree_t tree;
        } localShell;
        struct RawSocketState {
            uint64_t tx_total;
            uint64_t rx_total;
            qreal tx_speed;
            qreal rx_speed;
        } rawSocket;
        struct NamePipeState {
            uint64_t dummy;
        } namePipe;
        struct SSH2State {
            uint64_t tx_total;
            uint64_t rx_total;
            qreal tx_speed;
            qreal rx_speed;
            QString encryption_method;
        } ssh2;
    };
    SessionsWindow(SessionType tp, QWidget *parent = nullptr);
    ~SessionsWindow();

    int startSession(const QVariantMap &commonMeta, const QVariantMap &protocolMeta);
    void cloneSession(SessionsWindow *src, QString profile = QString());
    void disconnect(void);

    void setWorkingDirectory(const QString &dir);
    const QString getWorkingDirectory(void) { return workingDirectory; }
    
    void sendFileUseKermit(QStringList fileList);
    void recvFileUseKermit(const QString &downloadPath);
    void sendFileUseXModem(QString file, bool modem1KMode);
    void recvFileUseXModem(QString file);
    void sendFileUseYModem(QStringList fileList, bool modem1KMode);
    void recvFileUseYModem(const QString &downloadPath);
    void sendFileUseZModem(QStringList fileList);
    void recvFileUseZModem(const QString &downloadPath);

    int setLog(bool enable);
    bool isLog(void) { return enableLog; }
    void setLogOnEachLine(QString onEachLine) {add_time_on_each_line = onEachLine;};
    void setLogFilePath(QString logFilePath) {logPath = logFilePath;};
    void setRawLogFilePath(QString logFilePath) {rawLogPath = logFilePath;};
    void setScriptLogFilePath(QString logFilePath) {scriptLogPath = logFilePath;};
    QString getLogFileName(void);
    int setRawLog(bool enable);
    bool isRawLog(void) { return enableRawLog; }
    int startRecordingScript(void);
    int stopRecordingScript(void);
    int canlcelRecordingScript(void);
    bool isRecordingScript(void) { return enableRecordingScript; }
    QString getRawLogFileName(void);
    void setInBroadCastList(bool enable);
    bool isInBroadCastList() { return enableBroadCast; }
    void setTagColor(bool enable, QColor color = QColor()) { 
        tagColor = enable;
        tagColorValue = color;
    }
    bool isTagColor() { return tagColor; }
    QColor getTagColorValue() { return tagColorValue; }

    int writeReceiveASCIIFile(const char *data, int size);
    int startReceiveASCIIFile(const QString &fileName);
    int stopReceiveASCIIFile(void);
    bool isReceiveASCIIFile(void);

    int installWaitString(const QStringList &strList, int timeout, bool bcaseInsensitive, int mode) {
        m_waitStringList = strList;
        m_waitStringTimeout = timeout;
        m_waitStringCaseInsensitive = bcaseInsensitive;
        m_waitStringMode = mode;
        return 0;
    }

    bool isConsoleSession() const;
    bool isDesktopSession() const;
    QWidget *getMainWidget() const;
    bool isTerminal() const;
    SessionType getSessionType() const { return type; }
    ShellType getShellType() const { return shellType; }
    void setShowTitleType(ShowTitleType type) { showTitleType = type; }
    ShowTitleType getShowTitleType() const { return showTitleType; }
    QString getTitle() const { 
        switch(showTitleType) {
        default:
        case LongTitle:
            return longTitle;
        case ShortTitle:
            return shortTitle;
        case NameTitle:
            return name;
        }
    }
    QString getLongTitle() const { return longTitle; }
    QString getShortTitle() const { return shortTitle; }
    void setLongTitle(const QString &title) { longTitle = title; }
    void setShortTitle(const QString &title) { shortTitle = title; }
    void setName(const QString &name) { this->name = name; }
    QString getName() const { return name; }
    void lockSession(QString password);
    void unlockSession(QString password);
    bool isLocked() const { return locked; }
    SessionsState getState() const { return state; }
    bool hasChildProcess();
    StateInfo getStateInfo(void);
    void refeshTermSize(void);
#ifdef ENABLE_SSH
    SshSFtp *getSshSFtpChannel(void);
#endif
    QVariant protocolMetaValue(const char *key, const QVariant &defaultValue = QVariant()) const;

    QString getHostname() const { return m_hostname; }
    quint16 getPort() const { return m_port; }

    void setScrollBarPosition(QTermWidget::ScrollBarPosition position) {
        if(isConsoleSession()) term->setScrollBarPosition(position);
    }
    void reTranslateUi(void) {
        if(isConsoleSession()) term->reTranslateUi();
    }
    void setKeyBindings(const QString & kb) {
        if(isConsoleSession()) term->setKeyBindings(kb);
    }
    void setColorScheme(const QString & name) {
        if(isConsoleSession()) term->setColorScheme(name);
    }
    void setANSIColor(int index, const QColor & color) {
        if(isConsoleSession()) term->setANSIColor(index,color);
    }
    void setTerminalFont(const QFont & font) {
        if(isConsoleSession()) term->setTerminalFont(font);
    }
    void set_fix_quardCRT_issue33(bool fix) {
        if(isConsoleSession()) term->set_fix_quardCRT_issue33(fix);
    }
    void setTerminalBackgroundMode(int mode) {
        if(isConsoleSession()) term->setTerminalBackgroundMode(mode);
    }
    void setTerminalOpacity(qreal level) {
        if(isConsoleSession()) term->setTerminalOpacity(level);
    }
    void setHistorySize(int lines) {
        if(isConsoleSession()) term->setHistorySize(lines);
    }
    void setKeyboardCursorShape(uint32_t shape) {
        if(isConsoleSession()) term->setKeyboardCursorShape(shape);
    }
    void setBlinkingCursor(bool blink) {
        if(isConsoleSession()) term->setBlinkingCursor(blink);
    }
    void setWordCharacters(const QString &wordCharacters) {
        if(isConsoleSession()) term->setWordCharacters(wordCharacters);
    }
    void setAutoHideMouseAfter(int delay) {
        if(isConsoleSession()) term->autoHideMouseAfter(delay);
    }
    void setPreeditColorIndex(int index) {
        if(isConsoleSession()) term->setPreeditColorIndex(index);
    }
    void setSelectedTextAccentColorTransparency(int transparency) {
        if(isConsoleSession()) {
            if(transparency >= 100)
                term->setSelectionOpacity(1.0);
            else if(transparency <= 10)
                term->setSelectionOpacity(0.1);
            else
                term->setSelectionOpacity(transparency / 100.0);
        }
    }
    void setTerminalBackgroundImage(const QString& backgroundImage) {
        if(isConsoleSession()) term->setTerminalBackgroundImage(backgroundImage);
    }
    void setTerminalBackgroundMovie(const QString& backgroundMovie) {
        if(isConsoleSession()) term->setTerminalBackgroundMovie(backgroundMovie);
    }
    void setTerminalBackgroundVideo(const QString& backgroundVideo) {
        if(isConsoleSession()) term->setTerminalBackgroundVideo(backgroundVideo);
    }
    QString selectedText(bool preserveLineBreaks = true) {
        if(isConsoleSession()) return term->selectedText(preserveLineBreaks);
        return QString();
    }
    void setShowResizeNotificationEnabled(bool enabled) {
        if(isConsoleSession()) term->setShowResizeNotificationEnabled(enabled);
    }
    void copyClipboard() {
        if(isConsoleSession()) term->copyClipboard();
    }
    void pasteClipboard() {
        if(isConsoleSession()) term->pasteClipboard();
    }
    void selectAll() {
        if(isConsoleSession()) term->selectAll();
    }
    void toggleShowSearchBar() {
        if(isConsoleSession()) term->toggleShowSearchBar();
    }
    void saveHistory(QTextStream *stream, int format = 0, int start = -1, int end = -1) {
        if(isConsoleSession()) term->saveHistory(stream,format,start,end);
    }
    void saveHistory(QIODevice *device, int format = 0, int start = -1, int end = -1) {
        if(isConsoleSession()) term->saveHistory(device,format,start,end);
    }
    void screenShot(const QString &fileName);
    void screenShot(QPixmap *pixmap);
    void clearScrollback() {
        if(isConsoleSession()) term->clearScrollback();
    }
    void clearScreen() {
        if(isConsoleSession()) term->clearScreen();
    }
    QString screenGet(int row1, int col1, int row2, int col2, int mode) {
        if(isConsoleSession()) return term->screenGet(row1, col1, row2, col2, mode);
        return QString();
    }
    void clear() {
        if(isConsoleSession()) term->clear();
    }
    void zoomIn() {
        if(isConsoleSession()) term->zoomIn();
    }
    void zoomOut() {
        if(isConsoleSession()) term->zoomOut();
    }
    void proxySendData(QByteArray data) {
        if(isConsoleSession()) term->proxySendData(data);
    }
    void proxyRecvData(QByteArray data) {
        if(isConsoleSession()) term->recvData(data.data(),data.size());
    }
    void reverseProxySendData(QByteArray data);
    QList<QAction*> filterActions(const QPoint& position) {
        if(isConsoleSession()) {
            QPoint maptermWidgetPos = term->mapFromGlobal(position);
            return term->filterActions(maptermWidgetPos);
        }
        return QList<QAction*>();
    }
    void addHighLightText(const QString &text, const QColor &color) {
        if(isConsoleSession()) term->addHighLightText(text,color);
    }
    bool isContainHighLightText(const QString &text) {
        if(isConsoleSession()) return term->isContainHighLightText(text);
        return false;
    }
    void removeHighLightText(const QString &text) {
        if(isConsoleSession()) term->removeHighLightText(text);
    }
    void clearHighLightTexts(void) {
        if(isConsoleSession()) term->clearHighLightTexts();
    }
    QMap<QString, QColor> getHighLightTexts(void) {
        if(isConsoleSession()) return term->getHighLightTexts();
        return QMap<QString, QColor>();
    }
    void repaintDisplay(void) {
        if(isConsoleSession()) term->repaintDisplay();
    }
    int getLineCount(void) {
        if(isConsoleSession()) return term->lines();
        return -1;
    }
    int getColumnCount(void) {
        if(isConsoleSession()) return term->columns();
        return -1;
    }
    int getCursorLineCount(void) {
        if(isConsoleSession()) return term->getCursorY();
        return -1;
    }
    int getCursorColumnCount(void){
        if(isConsoleSession()) return term->getCursorX();
        return -1;
    }
    void setZmodemUploadPath(const QString &path) {
        zmodemUploadPath = path;
    }
    void setZmodemDownloadPath(const QString &path) {
        zmodemDownloadPath = path;
    }
    void setZmodemOnlie(bool enable) {
        zmodemOnlie = enable;
    }
    void setConfirmMultilinePaste(bool enable) {
        if(isConsoleSession()) term->setConfirmMultilinePaste(enable);
    }
    void setTrimPastedTrailingNewlines(bool enable) {
        if(isConsoleSession()) term->setTrimPastedTrailingNewlines(enable);
    }
    void setEcho(bool enable) {
        if(isConsoleSession()) term->setEcho(enable);
    }
    void setCursorColor(const QColor &color) {
        if(isConsoleSession()) term->setKeyboardCursorColor(false,color);
    }
    void setEnableHandleCtrlC(bool enable) {
        if(isConsoleSession()) term->setEnableHandleCtrlC(enable);
    }
    int getEndOfLineSeq(void) {
        return endOfLineSeq;
    }
    void setEndOfLineSeq(EndOfLineSeq mode) {
        endOfLineSeq = mode;
    }

signals:
    void hexDataDup(const char *data, int size);
    void stateChanged(SessionsState state);
    void titleChanged(int title,const QString& newTitle);
    void modemProxySendData(QByteArray data);
    void modemProxyRecvData(const QByteArray &data);
    void waitForStringFinished(const QString &str, int matchIndex);
    void broadCastSendData(const QByteArray &data);
    void requestReconnect(void);
    void termGetFocus();
    void termLostFocus();
    
private:
    int saveLog(const char *data, int size);
    int saveRawLog(const char *data, int size);
    void matchString(QByteArray data);
    bool doSendData(QByteArray &data, bool isConnected);
    bool doRecvData(QByteArray &data);
    void beginModemTransfer(const QString &protocolName);
    void finishModemTransfer(QObject *worker);
    void showTransferingFile(const QString &filename, bool useBaseName = false);
    void showTransferComplete(int result);
    void showSimpleTransferProgress(long bytesSent, long bytesTotal, bool *ret);
    void showZmodemTransferProgress(long bytesSent, long bytesTotal, long lastBps,
                                    int minLeft, int secLeft, bool *ret);
    void setupTerminalStateConnections();
    void setupTerminalOutputConnections();
    void setupTerminalUrlActivationConnection();
    void setupTerminalClipboardAndCtrlCConnections();
    void setupTerminalZModemConnections(QWidget *parent);
    void updateConnectionState(bool connected);
    void reportSessionError(const QString &title, const QString &message);
    void forwardReceivedData(QByteArray data, bool countTraffic);
    void setupModemProxyForward(const std::function<bool()> &canWrite,
                                const std::function<void(const QByteArray &)> &writer);
    void setupTerminalSendForward(const std::function<bool()> &isConnected,
                                  const std::function<void(const char *, int)> &writer,
                                  bool countTraffic);
    void addToRecordingScript(int type, QString str);
    void addToRecordingScript(int type, QByteArray ba);
    void prepareString(QString &str);

private:
    SessionType type;
    ShellType shellType;
    QString workingDirectory;
    QString longTitle;
    QString shortTitle;
    QString name;
    ShowTitleType showTitleType;
    QWidget *messageParentWidget;
    QTermWidget *term;
    bool enableLog;
    bool enableRawLog;
    bool enableRecordingScript;
    QList<QPair<int, QByteArray>> recordingScript;
    QByteArray recordingScriptRecvBuffer;
    QByteArray recordingScriptSendBuffer;
    bool enableBroadCast;
    QMutex log_file_mutex;
    QMutex raw_log_file_mutex;
    QMutex recording_script_file_mutex;
    QMutex recording_script_recv_mutex;
    QMutex recording_script_send_mutex;
    QFile *log_file = nullptr;
    QFile *raw_log_file = nullptr;
    QString add_time_on_each_line;
    bool fflush_file = true;
    QByteArray password_hash;
    bool locked = false;
    SessionsState state = Disconnected;
    uint64_t tx_total = 0;
    uint64_t rx_total = 0;
    uint64_t tx_realtime = 0;
    uint64_t rx_realtime = 0;
    qreal tx_speed = 0;
    qreal rx_speed = 0;
    QTimer *realtimespeed_timer = nullptr;
    bool tagColor = false;
    QColor tagColorValue;
    EndOfLineSeq endOfLineSeq = AUTO;
    bool m_requestReconnect = false;
    
    QMutex modemProxyChannelMutex;
    bool modemProxyChannel = false;
    bool stopModemProxy = false;
    bool zmodemOnlie = true;
    QString zmodemUploadPath;
    QString zmodemDownloadPath;

    QMutex receiveASCIIFileMutex;
    bool receiveASCIIFile = false;
    QFile *receiveASCIIFileFd = nullptr;

    QStringList m_waitStringList;
    int m_waitStringTimeout;
    bool m_waitStringCaseInsensitive;
    int m_waitStringMode;
    QByteArray m_waitStringDate;

    QString m_hostname;
    quint16 m_port;

    QString logPath;
    QString rawLogPath;
    QString scriptLogPath;
    static QString saveRecordingTempDirPath;
    std::unique_ptr<SessionProtocolBase> protocol;
};

#endif // SESSIONSWINDOW_H
