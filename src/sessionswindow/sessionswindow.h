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

#include "sshclient.h"
#include "sshtunnelout.h"
#include "qtermwidget.h"
#include "QTelnet.h"
#include "ptyqt.h"
#include "sshsftp.h"
#include "qvncclientwidget.h"

class SessionsWindow : public QObject
{
    Q_OBJECT
public:
    enum SessionType {
        Telnet,
        Serial,
        LocalShell,
        RawSocket,
        NamePipe,
        SSH2,
        VNC,
    };
    enum SessionsState {
        Connected,
        Disconnected,
        Locked,
        Error,
    };
    struct StateInfo {
        SessionType type;
        SessionsState state;
        struct TelnetState {
            uint64_t tx_total;
            uint64_t rx_total;
        } telnet;
        struct SerialState {
            uint64_t tx_total;
            uint64_t rx_total;
        } serial;
        struct LocalShellState {
            IPtyProcess::pidTree_t tree;
        } localShell;
        struct RawSocketState {
            uint64_t tx_total;
            uint64_t rx_total;
        } rawSocket;
        struct NamePipeState {
            uint64_t dummy;
        } namePipe;
        struct SSH2State {
            uint64_t tx_total;
            uint64_t rx_total;
        } ssh2;
    };
    SessionsWindow(SessionType tp, QWidget *parent = nullptr);
    ~SessionsWindow();

    void cloneSession(SessionsWindow *src);
    int startLocalShellSession(const QString &command);
    int startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type);
    int startSerialSession(const QString &portName, uint32_t baudRate,
                    int dataBits, int parity, int stopBits, bool flowControl, bool xEnable );
    int startRawSocketSession(const QString &hostname, quint16 port);
    int startNamePipeSession(const QString &name);
    int startSSH2Session(const QString &hostname, quint16 port, const QString &username, const QString &password);
    int startVNCSession(const QString &hostname, quint16 port, const QString &password);

    void reconnect(void);

    void setWorkingDirectory(const QString &dir);
    const QString getWorkingDirectory(void) { return workingDirectory; }
    
    void sendFileUseKermit(QStringList fileList);
    void recvFileUseKermit(void);
    void sendFileUseXModem(QString file);
    void recvFileUseXModem(QString file);
    void sendFileUseYModem(QStringList fileList);
    void recvFileUseYModem(void);
    void sendFileUseZModem(QStringList fileList);

    int setLog(bool enable);
    bool isLog(void) { return enableLog; }
    int setRawLog(bool enable);
    bool isRawLog(void) { return enableRawLog; }

    int writeReceiveASCIIFile(const char *data, int size);
    int startReceiveASCIIFile(const QString &fileName);
    int stopReceiveASCIIFile(void);
    bool isReceiveASCIIFile(void);

    QWidget *getMainWidget() const { 
        if(type == VNC)
            return static_cast<QWidget *>(vncClient);
        else
            return static_cast<QWidget *>(term); 
    }
    bool isTerminal() const { return type != VNC; }
    SessionType getSessionType() const { return type; }
    QString getTitle() const { return showShortTitle ? shortTitle : longTitle; }
    QString getLongTitle() const { return longTitle; }
    QString getShortTitle() const { return shortTitle; }
    void setShowShortTitle(bool show) { showShortTitle = show; }
    bool getShowShortTitle() const { return showShortTitle; }
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
    SshSFtp *getSshSFtpChannel(void);

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
    void setTerminalFont(const QFont & font) {
        if(term) term->setTerminalFont(font);
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
    void saveHistory(QTextStream *stream, int format = 0) {
        if(term) term->saveHistory(stream,format);
    }
    void saveHistory(QIODevice *device, int format = 0) {
        if(term) term->saveHistory(device,format);
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
    void repaintDisplay(void) {
        if(term) term->repaintDisplay();
    }

signals:
    void hexDataDup(const char *data, int size);
    void stateChanged(SessionsState state);
    void titleChanged(int title,const QString& newTitle);
    void modemProxySendData(QByteArray data);
    void modemProxyRecvData(const QByteArray &data);

private:
    int saveLog(const char *data, int size);
    int saveRawLog(const char *data, int size);

private:
    SessionType type;
    QString workingDirectory;
    QString longTitle;
    QString shortTitle;
    QString name;
    bool showShortTitle;
    QTermWidget *term;
    QTelnet *telnet;
    QSerialPort *serialPort;
    QTcpSocket *rawSocket;
    IPtyProcess *localShell;
    QLocalSocket *namePipe;
    SshClient *ssh2Client;
    QVNCClientWidget *vncClient;
    bool enableLog;
    bool enableRawLog;
    QMutex log_file_mutex;
    QMutex raw_log_file_mutex;
    QFile *log_file = nullptr;
    QFile *raw_log_file = nullptr;
    bool fflush_file = true;
    QByteArray password_hash;
    bool locked = false;
    SessionsState state = Disconnected;
    uint64_t tx_total = 0;
    uint64_t rx_total = 0;
    
    QMutex modemProxyChannelMutex;
    bool modemProxyChannel = false;
    bool stopModemProxy = false;

    QMutex receiveASCIIFileMutex;
    bool receiveASCIIFile = false;
    QFile *receiveASCIIFileFd = nullptr;

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
    QString m_pipeName;
    QString m_username;
    QString m_password;
};

#endif // SESSIONSWINDOW_H
