/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2025 Quard <2014500726@smail.xtu.edu.cn>
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
#ifndef QSPDQSpdLogger_H
#define QSPDQSpdLogger_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include "spdlog/spdlog.h"

class QSpdLogger: public QObject
{
    Q_OBJECT
private:
    explicit QSpdLogger(QObject *parent = nullptr);
    ~QSpdLogger();
    static QSpdLogger* self;
#if defined(QT_NO_DEBUG)
    QString logPattern = "(%Y-%m-%d %T:%f) [%10t] [%^%10l%$] %v";
#else
    QString logPattern = "(%Y-%m-%d %T:%f) [%10t] [%^%10l%$] [%s:%#:%!] %v";
#endif

public:
    static  QSpdLogger* Instance();
    void installMessageHandler();
    void uninstallMessageHandler();

    int setGlobalLogPattern(const QString &format);
    void clearGlobalLogPattern(void);
    void setLogLevel(QtMsgType level);
    void setStdLogLevel(QtMsgType level);
    void flush(void);

    int addFileSink(const QString &filename, uint32_t max_size = 0, uint32_t max_files = 0);
    void setFileSinkLogLevel(const QString &filename, QtMsgType level);
    int removeFileSink(const QString &filename);

    int addUdpSink(const QString &ip, uint16_t port);
    void setUdpSinkLogLevel(const QString &ip, uint16_t port, QtMsgType level);
    int removeUdpSink(const QString &ip, uint16_t port);

    int addTcpSink(const QString &ip, uint16_t port);
    void setTcpSinkLogLevel(const QString &ip, uint16_t port, QtMsgType level);
    int removeTcpSink(const QString &ip, uint16_t port);

    int addQTextEditSink(QTextEdit *qt_text_edit, int max_lines,
                                        bool dark_colors, bool is_utf8);
    void setQTextEditSinkLogLevel(QTextEdit *qt_text_edit, QtMsgType level);
    int removeQTextEditSink(QTextEdit *qt_text_edit);

    int addRingBufferSink(const QString &name);
    void setRingBufferSinkLogLevel(const QString &name, QtMsgType level);
    int removeRingBufferSink(const QString &name);
    QStringList getRingBufferSinkLog(const QString &name);

private:
    class GC
    {
    public:
        ~GC();
    };
};

#define QSPDLogger    QSpdLogger::Instance()

#endif // QSPDQSpdLogger_H
