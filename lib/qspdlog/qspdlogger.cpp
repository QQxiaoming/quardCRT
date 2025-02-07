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
#include "qspdlogger.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/udp_sink.h"
#include "spdlog/sinks/tcp_sink.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "spdlog/sinks/qt_sinks.h"
#include "spdlog/sinks/dist_sink.h"
#include <QMutex>
#include <QCoreApplication>
#include <QMap>
#include <QMessageLogContext>

static const QMap<QtMsgType, spdlog::level::level_enum> level_map = {
    {QtDebugMsg, spdlog::level::debug},
    {QtInfoMsg, spdlog::level::info},
    {QtWarningMsg, spdlog::level::warn},
    {QtCriticalMsg, spdlog::level::critical},
    {QtFatalMsg, spdlog::level::err},
};

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(type)) {
        spd_level = level_map[type];
    }
    if(context.file == nullptr) {
        spdlog::source_loc loc = {"UnKnow", 0, "UnKnow"};
        spdlog::log(loc, spd_level, msg.toStdString());
    } else {
        spdlog::source_loc loc = {context.file, context.line, context.function};
        spdlog::log(loc, spd_level, msg.toStdString());
    }
}

QSpdLogger* QSpdLogger::self = nullptr;

QSpdLogger* QSpdLogger::Instance() {
    if(!self) {
        QMutex muter;
        QMutexLocker clocker(&muter);

        if(!self) {
            self = new QSpdLogger();
        }
    }
    return self;
}

void QSpdLogger::installMessageHandler() {
    try {
        spdlog::init_thread_pool(8192, 1);
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
        std::vector<spdlog::sink_ptr> sinks = {dist_sink};
        std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::async_logger>("QSpdLogger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        dist_sink->add_sink(console_sink);
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::trace);
        setGlobalLogPattern(logPattern);
        qInstallMessageHandler(outputMessage);
    } catch (const spdlog::spdlog_ex &ex) {
        setGlobalLogPattern(logPattern);
        spdlog::log(spdlog::level::err, "TySpdLog initialization failed\n");
        qInstallMessageHandler(outputMessage);
    }
}

void QSpdLogger::uninstallMessageHandler() {
    qInstallMessageHandler(nullptr);
    spdlog::drop_all();
    spdlog::shutdown();
}

void QSpdLogger::flush(void) {
    spdlog::flush_all();
}

int QSpdLogger::setGlobalLogPattern(const QString &format) {
    logPattern = format;
    spdlog::set_pattern(logPattern.toStdString());
    return 0;
}

void QSpdLogger::clearGlobalLogPattern(void) {
    spdlog::set_pattern("%v");
}

void QSpdLogger::setLogLevel(QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        (*it)->set_level(spd_level);
    }
}

void QSpdLogger::setStdLogLevel(QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto console_sink = std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(*it)) {
            console_sink->set_level(spd_level);
        }
    }
}

int QSpdLogger::addFileSink(const QString &filename, uint32_t max_size, uint32_t max_files) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(*it)) {
                if (file_sink->filename() == filename.toStdString()) {
                    return -1;
                }
            }
            if (auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(*it)) {
                if (file_sink->filename() == filename.toStdString()) {
                    return -1;
                }
            }
        }
        if(max_size == 0 || max_files == 0) {
            std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename.toStdString());
            file_sink->set_pattern(logPattern.toStdString());
            dist_sink->add_sink(file_sink);
        } else {
            std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename.toStdString(), max_size, max_files);
            file_sink->set_pattern(logPattern.toStdString());
            dist_sink->add_sink(file_sink);
        }
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return 0;
}

int QSpdLogger::removeFileSink(const QString &filename) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(*it)) {
                if (file_sink->filename() == filename.toStdString()) {
                    dist_sink->remove_sink(file_sink);
                    return 0;
                }
            }
            if(auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(*it)) {
                if (file_sink->filename() == filename.toStdString()) {
                    dist_sink->remove_sink(file_sink);
                    return 0;
                }
            }
        }
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return -1;
}

void QSpdLogger::setFileSinkLogLevel(const QString &filename, QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(*it)) {
            if (file_sink->filename() == filename.toStdString()) {
                file_sink->set_level(spd_level);
            }
        }
        if (auto file_sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(*it)) {
            if (file_sink->filename() == filename.toStdString()) {
                file_sink->set_level(spd_level);
            }
        }
    }
}

int QSpdLogger::addUdpSink(const QString &ip, uint16_t port) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto udp_sink = std::dynamic_pointer_cast<spdlog::sinks::udp_sink_mt>(*it)) {
                const spdlog::sinks::udp_sink_config config = udp_sink->config();
                if (config.server_host == ip.toStdString() && config.server_port == port) {
                    return -1;
                }
            }
        }
        std::shared_ptr<spdlog::sinks::udp_sink_mt> udp_sink = std::make_shared<spdlog::sinks::udp_sink_mt>(spdlog::sinks::udp_sink_config(ip.toStdString(), port));
        udp_sink->set_pattern(logPattern.toStdString());
        dist_sink->add_sink(udp_sink);
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return 0;
}

int QSpdLogger::removeUdpSink(const QString &ip, uint16_t port) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto udp_sink = std::dynamic_pointer_cast<spdlog::sinks::udp_sink_mt>(*it)) {
                const spdlog::sinks::udp_sink_config config = udp_sink->config();
                if (config.server_host == ip.toStdString() && config.server_port == port) {
                    dist_sink->remove_sink(udp_sink);
                    return 0;
                }
            }
        }
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return -1;
}

void QSpdLogger::setUdpSinkLogLevel(const QString &ip, uint16_t port, QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto udp_sink = std::dynamic_pointer_cast<spdlog::sinks::udp_sink_mt>(*it)) {
            const spdlog::sinks::udp_sink_config config = udp_sink->config();
            if (config.server_host == ip.toStdString() && config.server_port == port) {
                udp_sink->set_level(spd_level);
            }
        }
    }
}

int QSpdLogger::addTcpSink(const QString &ip, uint16_t port) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto tcp_sink = std::dynamic_pointer_cast<spdlog::sinks::tcp_sink_mt>(*it)) {
                const spdlog::sinks::tcp_sink_config config = tcp_sink->config();
                if (config.server_host == ip.toStdString() && config.server_port == port) {
                    return -1;
                }
            }
        }
        std::shared_ptr<spdlog::sinks::tcp_sink_mt> tcp_sink = std::make_shared<spdlog::sinks::tcp_sink_mt>(spdlog::sinks::tcp_sink_config(ip.toStdString(), port));
        tcp_sink->set_pattern(logPattern.toStdString());
        dist_sink->add_sink(tcp_sink);
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return 0;
}

int QSpdLogger::removeTcpSink(const QString &ip, uint16_t port) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto tcp_sink = std::dynamic_pointer_cast<spdlog::sinks::tcp_sink_mt>(*it)) {
                const spdlog::sinks::tcp_sink_config config = tcp_sink->config();
                if (config.server_host == ip.toStdString() && config.server_port == port) {
                    dist_sink->remove_sink(tcp_sink);
                    return 0;
                }
            }
        }
    } catch (const spdlog::spdlog_ex &ex) {    
        return -1;
    }
    return -1;
}

void QSpdLogger::setTcpSinkLogLevel(const QString &ip, uint16_t port, QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto tcp_sink = std::dynamic_pointer_cast<spdlog::sinks::tcp_sink_mt>(*it)) {
            const spdlog::sinks::tcp_sink_config config = tcp_sink->config();
            if (config.server_host == ip.toStdString() && config.server_port == port) {
                tcp_sink->set_level(spd_level);
            }
        }
    }
}

int QSpdLogger::addQTextEditSink(QTextEdit *qt_text_edit, int max_lines,
                                    bool dark_colors, bool is_utf8) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto qt_sink = std::dynamic_pointer_cast<spdlog::sinks::qt_color_sink_mt>(*it)) {
                if (qt_sink->textEdit() == qt_text_edit) {
                    return -1;
                }
            }
        }
        std::shared_ptr<spdlog::sinks::qt_color_sink_mt> qt_sink = std::make_shared<spdlog::sinks::qt_color_sink_mt>(qt_text_edit, max_lines, dark_colors, is_utf8);
        qt_sink->set_pattern(logPattern.toStdString());
        dist_sink->add_sink(qt_sink);
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return 0;
}

int QSpdLogger::removeQTextEditSink(QTextEdit *qt_text_edit) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto qt_sink = std::dynamic_pointer_cast<spdlog::sinks::qt_color_sink_mt>(*it)) {
                if (qt_sink->textEdit() == qt_text_edit) {
                    dist_sink->remove_sink(qt_sink);
                    return 0;
                }
            }
        }
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return -1;
}

void QSpdLogger::setQTextEditSinkLogLevel(QTextEdit *qt_text_edit, QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto qt_sink = std::dynamic_pointer_cast<spdlog::sinks::qt_color_sink_mt>(*it)) {
            if (qt_sink->textEdit() == qt_text_edit) {
                qt_sink->set_level(spd_level);
            }
        }
    }
}

int QSpdLogger::addRingBufferSink(const QString &name) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto ringbuffer_sink = std::dynamic_pointer_cast<spdlog::sinks::ringbuffer_sink_mt>(*it)) {
                if (ringbuffer_sink->name() == name.toStdString()) {
                    return -1;
                }
            }
        }
        std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> ringbuffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(name.toStdString(),10);
        ringbuffer_sink->set_pattern(logPattern.toStdString());
        dist_sink->add_sink(ringbuffer_sink);
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return 0;
}

int QSpdLogger::removeRingBufferSink(const QString &name) {
    try {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
        std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
        for (auto it = sinks.begin(); it != sinks.end(); ++it) {
            if (auto ringbuffer_sink = std::dynamic_pointer_cast<spdlog::sinks::ringbuffer_sink_mt>(*it)) {
                if (ringbuffer_sink->name() == name.toStdString()) {
                    dist_sink->remove_sink(ringbuffer_sink);
                    return 0;
                }
            }
        }
    } catch (const spdlog::spdlog_ex &ex) {
        return -1;
    }
    return -1;
}

void QSpdLogger::setRingBufferSinkLogLevel(const QString &name, QtMsgType level) {
    spdlog::level::level_enum spd_level = spdlog::level::info;
    if(level_map.contains(level)) {
        spd_level = level_map[level];
    }
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto ringbuffer_sink = std::dynamic_pointer_cast<spdlog::sinks::ringbuffer_sink_mt>(*it)) {
            if (ringbuffer_sink->name() == name.toStdString()) {
                ringbuffer_sink->set_level(spd_level);
            }
        }
    }
}

QStringList QSpdLogger::getRingBufferSinkLog(const QString &name) {
    QStringList list;
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(spdlog::default_logger()->sinks().front());
    std::vector<spdlog::sink_ptr> sinks = dist_sink->sinks();
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        if (auto ringbuffer_sink = std::dynamic_pointer_cast<spdlog::sinks::ringbuffer_sink_mt>(*it)) {
            if (ringbuffer_sink->name() == name.toStdString()) {
                std::vector<std::string> str = ringbuffer_sink->last_formatted();
                for (auto its = str.begin(); its != str.end(); ++its) {
                    list.append(QString::fromStdString(*its));
                }
                return list;
            }
        }
    }
    return list;
}

QSpdLogger::QSpdLogger(QObject *parent) : QObject(parent) {
    static QSpdLogger::GC gc;
}

QSpdLogger::~QSpdLogger() { }

QSpdLogger::GC::~GC() {
    if (self != nullptr) {
        delete self;
        self = nullptr;
    }
}
