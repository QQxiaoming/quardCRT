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
#include "logger.h"
#include <QMutex>
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <iostream>
#include <QMessageLogContext>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    Logger *instance = Logger::Instance();

    if(instance->logPath().isEmpty()) {
        /* debug时，若不输出日志，则输出打印，release则什么都不输出*/
        #if !defined(QT_NO_DEBUG)
        //if(type == QtDebugMsg) {return;}     //根据需要选择，屏蔽debug信息
        //if(type == QtInfoMsg) {return;}      //根据需要选择，屏蔽info信息
        //if(type == QtWarningMsg) {return;}   //根据需要选择，屏蔽warning信息
        //if(type == QtCriticalMsg) {return;}  //根据需要选择，屏蔽critical信息
        //if(type == QtFatalMsg) {return;}     //根据需要选择，屏蔽fatal信息
        std::cout << msg.toStdString() << std::endl;
        #endif
    } else {
        QString text;
        switch(type)
        {
            case QtDebugMsg: {
                if(instance->outPutLv == QtDebugMsg) {
                    text = QString("TYPE: debug");
                } else {
                    return;
                }
                break;
            }
            case QtInfoMsg: {
                text = QString("TYPE: info");
                break;
            }
            case QtWarningMsg: {
                text = QString("TYPE: Warning");
                break;
            }
            case QtCriticalMsg: {
                text = QString("TYPE: Critical");
                break;
            }
            case QtFatalMsg: {
                text = QString("TYPE:Fatal");
                break;
            }
        }

        QFile file(instance->logPath() + instance->logName());
        file.open(QIODevice::WriteOnly | QIODevice::Append);

        if(instance->verboseLog) {
            //消息输出位置
            QString context_info = QString("File: %1\r\nFunc: %2\r\nLine: %3")
                    .arg(QString(context.file))
                    .arg(QString(context.function))
                    .arg(context.line);
            //消息打印时间
            QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ddd");
            QString current_date = QString("Time: %1")
                    .arg(current_date_time);
            //调试信息
            QString message = QString("%1\r\n%2\r\n%3\r\n%4")
                    .arg(text)
                    .arg(current_date)
                    .arg(context_info)
                    .arg("MSG : "+msg);
            //log 位置
            QString logPahtHtml = "<a href = \"%1\">Click here to open the log directory.</a>\r\n";
            logPahtHtml = logPahtHtml.arg(QDir::currentPath()+"/"+instance->logPath());
            //将调试信息格式化成 html 格式
            QString msgHtml = msg;
            msgHtml.replace("  ", "&nbsp;");
            msgHtml.remove("\r");
            msgHtml.replace("\n", "<br>");
            msgHtml = QString("<font color=red>" + msgHtml + "</font>");

            QString contextInfoHtml = context_info;
            contextInfoHtml.remove("\r");
            contextInfoHtml.replace("\n", "<br>");
            //格式化后的调试信息
            QString messageHtml = QString("%1<br>%2<br>%3<br>%4<br>%5")
                    .arg(text)
                    .arg(logPahtHtml)
                    .arg(current_date)
                    .arg(contextInfoHtml)
                    .arg("MSG : "+msgHtml);

            //将调试信息写入文件
            QTextStream text_stream(&file);
            text_stream << message << "\r\n\r\n\r\n";
            file.flush();
            file.close();

            //将处理好的调试信息发送出去
            instance->sigDebugStrData(message);
            //将处理成 html 的调试信息发送出去
            instance->sigDebugHtmlData(messageHtml);
        } else {
            QString current_date_time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
            QTextStream text_stream(&file);
            text_stream << current_date_time << " : " << msg << "\r\n";
            file.flush();
            file.close();
        }

        //检查文件是否达到了指定大小
        if(file.size() < 1024*1024) {
            return ;
        }
        //log达到了限制值则将名字更改，防止文件越来越大
        for(int loop = 1; loop < 1000; ++loop) {
            QString fileName = QString("%1/log_%2.txt").arg(instance->logPath()).arg(loop);
            QFile file_1(fileName);
            if(file_1.size() < 4) {
                file.rename(fileName);
                return ;
            }
        }
    }
}

Logger* Logger::self = nullptr;

Logger* Logger::Instance() {
    if(!self) {
        QMutex muter;
        QMutexLocker clocker(&muter);

        if(!self) {
            self = new Logger();
        }
    }
    return self;
}

void Logger::installMessageHandler(QString logPath, QtMsgType lv, bool verboseLog) {
    this->existsLog = false;
    this->outPutLv = QtInfoMsg;
    this->verboseLog = false;
    if(!logPath.isEmpty()) {
        QDir log_dir(logPath);
        if(!log_dir.exists()) {
            log_dir.mkpath(".");
        }
        if(log_dir.exists()) {
            this->m_logPath = logPath;
            this->existsLog = true;
            this->outPutLv = lv;
            this->verboseLog = verboseLog;
        }
    }

    qInstallMessageHandler(outputMessage);
}

void Logger::uninstallMessageHandler() {
    qInstallMessageHandler(nullptr);
}

QString Logger::logPath() const {
    if(!this->existsLog) {
        return "";
    }

    QString current_date_file_name = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QDir dir(QString(m_logPath+"/%1").arg(current_date_file_name));
    if(!dir.exists()) {
        dir.mkpath("./");
    }
    return dir.path() + "/" ;
}

QString Logger::logName() const {
    return "log.txt";
}

Logger::Logger(QObject *parent) : QObject(parent) {
    static Logger::GC gc;
}

Logger::~Logger() { }

Logger::GC::~GC() {
    if (self != nullptr) {
        delete self;
        self = nullptr;
    }
}
