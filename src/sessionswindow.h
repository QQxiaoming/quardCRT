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

#include "qtermwidget.h"
#include "QTelnet.h"
#include "ptyqt.h"

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
    };
    enum SessionsState {
        Connected,
        Disconnected,
        Locked,
        Error,
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

    void setWorkingDirectory(const QString &dir);
    const QString getWorkingDirectory(void) { return workingDirectory; }
    
    int setLog(bool enable);
    bool isLog(void) { return enableLog; }
    int setRawLog(bool enable);
    bool isRawLog(void) { return enableRawLog; }

    QTermWidget *getTermWidget() const { return term; }
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
    QList<QPair<int, QString>> getLocalShellState(void);

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

signals:
    void hexDataDup(const char *data, int size);
    void stateChanged(SessionsState state);

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
};

#endif // SESSIONSWINDOW_H
