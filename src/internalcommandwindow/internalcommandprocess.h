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
#ifndef INTERNALCOMMANDPROCESS_H
#define INTERNALCOMMANDPROCESS_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#ifdef ENABLE_PYTHON
#include "pyrun.h"
#endif
class CentralWidget;
class InternalCommandProcess : public QThread
{
    Q_OBJECT
public:
    explicit InternalCommandProcess(CentralWidget *mainWidget, QObject *parent = nullptr);
    ~InternalCommandProcess();
#ifdef ENABLE_PYTHON
    void setPyRun(PyRun *pyRun);
#endif
    void setPrompt(const QString &prompt);
    void recvData(const QByteArray &data);
    void sendLogo(bool color = false);

    struct Command {
        QStringList name;
        QStringList aliasName;
        QString description;
        std::function<void()> action;
    };

signals:
    void sendData(const QByteArray &data);
    void showString(const QString &name, const QString &str);
    void showEasterEggs(void);

protected:
    void run(void);

private:
    void sendString(const QString &str);
    void sendLineString(const QString &str, int asciiColor = 0);
    void sendPrompt(void);
    void sendWelcome(void);
    void processLine(const QString &line);
    QString promptLine = ">>> ";
    QStringList historyCmdList;
    int32_t historyCmdIndex = 0;

    QByteArray buffer;
    QMutex mutex;
    QWaitCondition condition;
    bool exit;
    CentralWidget *m_mainWidget;
#ifdef ENABLE_PYTHON
    PyRun *m_pyRun = nullptr;
#endif
};

#endif // INTERNALCOMMANDPROCESS_H
