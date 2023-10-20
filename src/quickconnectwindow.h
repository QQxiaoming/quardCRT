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
#ifndef QUICKCONNECTWINDOW_H
#define QUICKCONNECTWINDOW_H

#include <QDialog>
#include <QShowEvent>

namespace Ui {
class QuickConnectWindow;
}

class QuickConnectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit QuickConnectWindow(QWidget *parent = nullptr);
    ~QuickConnectWindow();

    enum QuickConnectType : uint32_t {
        Telnet = 0,
        Serial,
        LocalShell,
        Raw,
        SSH2,
    };
    struct QuickConnectData {
        QuickConnectType type;
        struct {
            QString hostname;
            int port;
            QString webSocket;
        }TelnetData;
        struct {
            QString portName;
            uint32_t baudRate;
            int dataBits;
            int parity;
            int stopBits;
            bool flowControl;
            bool xEnable;
        }SerialData;
        struct {
            QString command;
        }LocalShellData;
        struct {
            QString hostname;
            int port;
        }RawData;
        struct {
            QString hostname;
            int port;
            QString username;
            QString password;
            QString privateKey;
            QString passphrase;
        }SSH2Data;
        bool saveSession;
        bool openInTab;
    };
    void setProtocol(QuickConnectType index);
    
signals:
    void sendQuickConnectData(QuickConnectData);

private slots:
    void comboBoxProtocolChanged(int index);
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::QuickConnectWindow *ui;
};

#endif // QUICKCONNECTWINDOW_H
