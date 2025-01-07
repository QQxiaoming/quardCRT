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
#include <QTextEdit>
#include "spdlog/spdlog.h"

class QSpdLogger: public QObject
{
    Q_OBJECT
private:
    explicit QSpdLogger(QObject *parent = nullptr);
    ~QSpdLogger();
    static QSpdLogger* self;
    QString logPattern = "(%Y-%m-%d %T:%f) [%10t] [%^%10l%$] [%s:%#:%!] %v";

public:
    static  QSpdLogger* Instance();
    void installMessageHandler();
    void uninstallMessageHandler();

    int setGlobalLogPattern(const QString &format);
    void clearGlobalLogPattern(void);
    void setLogLevel(QtMsgType level);
    void setStdLogLevel(QtMsgType level);

    int addFileSink(QString filename, uint32_t max_size = 0, uint32_t max_files = 0);
    void setFileSinkLogLevel(QString filename, QtMsgType level);
    int removeFileSink(QString filename);

    int addUdpSink(QString ip, uint16_t port);
    void setUdpSinkLogLevel(QString ip, uint16_t port, QtMsgType level);
    int removeUdpSink(QString ip, uint16_t port);

    int addTcpSink(QString ip, uint16_t port);
    void setTcpSinkLogLevel(QString ip, uint16_t port, QtMsgType level);
    int removeTcpSink(QString ip, uint16_t port);

    int addQTextEditSink(QTextEdit *qt_text_edit, int max_lines,
                                        bool dark_colors, bool is_utf8);
    void setQTextEditSinkLogLevel(QTextEdit *qt_text_edit, QtMsgType level);
    int removeQTextEditSink(QTextEdit *qt_text_edit);

signals:
    void sigDebugStrData(const QString &);
    void sigDebugHtmlData(const QString &);

private:
    class GC
    {
    public:
        ~GC();
    };
};

#endif // QSPDQSpdLogger_H
