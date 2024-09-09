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
#ifndef SESSIONSWINDOW_H
#define SESSIONSWINDOW_H

#include <QObject>
#include <QWidget>
#include <QTcpSocket>
#include <QSerialPort>
#include <QLocalSocket>
#include <QMutex>
#include <QMutexLocker>

#ifdef ENABLE_SSH
#include "sshclient.h"
#include "sshtunnelout.h"
#include "sshsftp.h"
#endif
#include "qtermwidget.h"
#include "QTelnet.h"
#include "ptyqt.h"
#include "qvncclientwidget.h"
#include "qextserialenumerator.h"

class SessionsWindow : public QObject
{
    Q_OBJECT
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

    void cloneSession(SessionsWindow *src, QString profile = QString());
#if defined(Q_OS_WIN)
    int startLocalShellSession(const QString &command, QString profile = QString(), ShellType sTp = PowerShell);
#else
    int startLocalShellSession(const QString &command, QString profile = QString(), ShellType sTp = UnixShell);
#endif
    int startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type);
    int startSerialSession(const QString &portName, uint32_t baudRate,
                    int dataBits, int parity, int stopBits, bool flowControl, bool xEnable );
    int startRawSocketSession(const QString &hostname, quint16 port);
    int startNamePipeSession(const QString &name);
#ifdef ENABLE_SSH
    int startSSH2Session(const QString &hostname, quint16 port, const QString &username, const QString &password);
#endif
    int startVNCSession(const QString &hostname, quint16 port, const QString &password);

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

    QWidget *getMainWidget() const { 
        if(type == VNC)
            return static_cast<QWidget *>(vncClient);
        else
            return static_cast<QWidget *>(term); 
    }
    bool isTerminal() const { return type != VNC; }
    SessionType getSessionType() const { return type; }
    ShellType getShellType() const { return shellType; }
    QString getWSLUserName() const { return m_wslUserName; }
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

    QString getHostname() const { return m_hostname; }
    quint16 getPort() const { return m_port; }
    QString getPortName() const { return m_portName; }
    QTelnet::SocketType getSocketType() const { return m_type; }
    uint32_t getBaudRate() const { return m_baudRate; }
    int getDataBits() const { return m_dataBits; }
    int getParity() const { return m_parity; }
    int getStopBits() const { return m_stopBits; }
    bool getFlowControl() const { return m_flowControl; }
    bool getXEnable() const { return m_xEnable; }
    QString getCommand() const { return m_command; }
    QString getPipeName() const { return m_pipeName; }
    QString getUserName() const { return m_username; }
    QString getPassWord() const { return m_password; }

    void setScrollBarPosition(QTermWidget::ScrollBarPosition position) {
        if(term) term->setScrollBarPosition(position);
    }
    void reTranslateUi(void) {
        if(term) term->reTranslateUi();
    }
    void setKeyBindings(const QString & kb) {
        if(term) term->setKeyBindings(kb);
    }
    void setColorScheme(const QString & name) {
        if(term) term->setColorScheme(name);
    }
    void setANSIColor(int index, const QColor & color) {
        if(term) term->setANSIColor(index,color);
    }
    void setTerminalFont(const QFont & font) {
        if(term) term->setTerminalFont(font);
    }
    void set_fix_quardCRT_issue33(bool fix) {
        if(term) term->set_fix_quardCRT_issue33(fix);
    }
    void setTerminalBackgroundMode(int mode) {
        if(term) term->setTerminalBackgroundMode(mode);
    }
    void setTerminalOpacity(qreal level) {
        if(term) term->setTerminalOpacity(level);
    }
    void setHistorySize(int lines) {
        if(term) term->setHistorySize(lines);
    }
    void setKeyboardCursorShape(uint32_t shape) {
        if(term) term->setKeyboardCursorShape(shape);
    }
    void setBlinkingCursor(bool blink) {
        if(term) term->setBlinkingCursor(blink);
    }
    void setWordCharacters(const QString &wordCharacters) {
        if(term) term->setWordCharacters(wordCharacters);
    }
    void setPreeditColorIndex(int index) {
        if(term) term->setPreeditColorIndex(index);
    }
    void setSelectedTextAccentColorTransparency(int transparency) {
        if(term) {
            if(transparency >= 100)
                term->setSelectionOpacity(1.0);
            else if(transparency <= 10)
                term->setSelectionOpacity(0.1);
            else
                term->setSelectionOpacity(transparency / 100.0);
        }
    }
    void setTerminalBackgroundImage(const QString& backgroundImage) {
        if(term) term->setTerminalBackgroundImage(backgroundImage);
    }
    void setTerminalBackgroundMovie(const QString& backgroundMovie) {
        if(term) term->setTerminalBackgroundMovie(backgroundMovie);
    }
    void setTerminalBackgroundVideo(const QString& backgroundVideo) {
        if(term) term->setTerminalBackgroundVideo(backgroundVideo);
    }
    QString selectedText(bool preserveLineBreaks = true) {
        if(term) return term->selectedText(preserveLineBreaks);
        return QString();
    }
    void setShowResizeNotificationEnabled(bool enabled) {
        if(term) term->setShowResizeNotificationEnabled(enabled);
    }
    void copyClipboard() {
        if(term) term->copyClipboard();
    }
    void pasteClipboard() {
        if(term) term->pasteClipboard();
    }
    void selectAll() {
        if(term) term->selectAll();
    }
    void toggleShowSearchBar() {
        if(term) term->toggleShowSearchBar();
    }
    void saveHistory(QTextStream *stream, int format = 0, int start = -1, int end = -1) {
        if(term) term->saveHistory(stream,format,start,end);
    }
    void saveHistory(QIODevice *device, int format = 0, int start = -1, int end = -1) {
        if(term) term->saveHistory(device,format,start,end);
    }
    void screenShot(const QString &fileName) {
        if(term) term->screenShot(fileName);
        if(vncClient) vncClient->screenShot(fileName);
    }
    void screenShot(QPixmap *pixmap) {
        if(term) term->screenShot(pixmap);
        if(vncClient) vncClient->screenShot(pixmap);
    }
    void clearScrollback() {
        if(term) term->clearScrollback();
    }
    void clearScreen() {
        if(term) term->clearScreen();
    }
    QString screenGet(int row1, int col1, int row2, int col2, int mode) {
        if(term) return term->screenGet(row1, col1, row2, col2, mode);
        return QString();
    }
    void clear() {
        if(term) term->clear();
    }
    void zoomIn() {
        if(term) term->zoomIn();
    }
    void zoomOut() {
        if(term) term->zoomOut();
    }
    void proxySendData(QByteArray data) {
        if(term) term->proxySendData(data);
    }
    void proxyRecvData(QByteArray data) {
        if(term) term->recvData(data.data(),data.size());
    }
    void reverseProxySendData(QByteArray data);
    QList<QAction*> filterActions(const QPoint& position) {
        if(term) {
            QPoint maptermWidgetPos = term->mapFromGlobal(position);
            return term->filterActions(maptermWidgetPos);
        }
        return QList<QAction*>();
    }
    void addHighLightText(const QString &text, const QColor &color) {
        if(term) term->addHighLightText(text,color);
    }
    bool isContainHighLightText(const QString &text) {
        if(term) return term->isContainHighLightText(text);
        return false;
    }
    void removeHighLightText(const QString &text) {
        if(term) term->removeHighLightText(text);
    }
    void clearHighLightTexts(void) {
        if(term) term->clearHighLightTexts();
    }
    QMap<QString, QColor> getHighLightTexts(void) {
        if(term) return term->getHighLightTexts();
        return QMap<QString, QColor>();
    }
    void repaintDisplay(void) {
        if(term) term->repaintDisplay();
    }
    int getLineCount(void) {
        if(term) return term->lines();
        return -1;
    }
    int getColumnCount(void) {
        if(term) return term->columns();
        return -1;
    }
    int getCursorLineCount(void) {
        if(term) return term->getCursorY();
        return -1;
    }
    int getCursorColumnCount(void){
        if(term) return term->getCursorX();
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
        if(term) term->setConfirmMultilinePaste(enable);
    }
    void setTrimPastedTrailingNewlines(bool enable) {
        if(term) term->setTrimPastedTrailingNewlines(enable);
    }
    void setEcho(bool enable) {
        if(term) term->setEcho(enable);
    }
    void setCursorColor(const QColor &color) {
        if(term) term->setKeyboardCursorColor(false,color);
    }
    void setEnableHandleCtrlC(bool enable) {
        if(term) term->setEnableHandleCtrlC(enable);
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
    void addToRecordingScript(int type, QString str);
    void addToRecordingScript(int type, QByteArray ba);

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
    QTelnet *telnet;
    QSerialPort *serialPort;
    QextSerialEnumerator *serialMonitor;
    QTcpSocket *rawSocket;
    IPtyProcess *localShell;
    QLocalSocket *namePipe;
#ifdef ENABLE_SSH
    SshClient *ssh2Client;
#endif
    QVNCClientWidget *vncClient;
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
    QTelnet::SocketType m_type;
    QString m_portName;
    uint32_t m_baudRate;
    int m_dataBits;
    int m_parity;
    int m_stopBits;
    bool m_flowControl;
    bool m_xEnable;
    QString m_command;
    QString m_wslUserName;
    QString m_pipeName;
    QString m_username;
    QString m_password;

    static QString saveRecordingPath;
};

#endif // SESSIONSWINDOW_H
