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
#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QObject>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QLocale>
#include <QList>

#define PLUGIN_API_VERSION 2

class PluginInterfaceV2 : public QObject
{
    Q_OBJECT
public:
    virtual ~PluginInterfaceV2() {}
    virtual int init(QMap<QString, QString> params, QWidget *parent) = 0;
    virtual QString name() = 0;
    virtual QString version() = 0;
    virtual QMap<QString,void *> metaObject() = 0;
    virtual QMenu *terminalContextMenu(QString selectedText, QString workingDirectory, QMenu *parentMenu) = 0;
    virtual QList<QAction *> terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu) = 0;
    virtual void setLanguage(const QLocale &language,QApplication *app) = 0;
    virtual void retranslateUi() = 0;

signals:
    void requestTelnetConnect(QString host, int port, int type);
    void requestSerialConnect(QString portName, uint32_t baudRate, int dataBits, int parity, int stopBits, bool flowControl, bool xEnable);
    void requestLocalShellConnect(QString command, QString workingDirectory);
    void requestRawSocketConnect(QString host, int port);
    void requestNamePipeConnect(QString namePipe);
    void requestSSH2Connect(QString host, QString user, QString password, int port);
    void requestVNCConnect(QString host, QString password, int port);
    void sendCommand(QString cmd);
    void writeSettings(QString path, QString key, QVariant value);
    void readSettings(QString path, QString key, QVariant &value);
};

class PluginInterfaceV1 : public QObject
{
    Q_OBJECT
public:
    virtual ~PluginInterfaceV1() {}
    virtual int init(QMap<QString, QString> params, QWidget *parent) = 0;
    virtual QString name() = 0;
    virtual QString version() = 0;
    virtual QMenu *mainMenu() = 0;
    virtual QAction *mainAction() = 0;
    virtual QMenu *terminalContextMenu(QString selectedText, QString workingDirectory, QMenu *parentMenu) = 0;
    virtual QList<QAction *> terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu) = 0;
    virtual void setLanguage(const QLocale &language,QApplication *app) = 0;
    virtual void retranslateUi() = 0;

signals:
    void requestTelnetConnect(QString host, int port, int type);
    void requestSerialConnect(QString portName, uint32_t baudRate, int dataBits, int parity, int stopBits, bool flowControl, bool xEnable);
    void requestLocalShellConnect(QString command, QString workingDirectory);
    void requestRawSocketConnect(QString host, int port);
    void requestNamePipeConnect(QString namePipe);
    void requestSSH2Connect(QString host, QString user, QString password, int port);
    void requestVNCConnect(QString host, QString password, int port);
    void sendCommand(QString cmd);
    void writeSettings(QString path, QString key, QVariant value);
    void readSettings(QString path, QString key, QVariant &value);
};

#define PluginInterface PluginInterfaceV2

#define PluginInterface_iid "org.quardCRT.PluginInterface"

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
