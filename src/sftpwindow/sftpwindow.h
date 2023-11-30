/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2021 Quard <2014500726@smail.xtu.edu.cn>
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
#ifndef SFTPWINDOW_H
#define SFTPWINDOW_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QThread>
#include <QMutex>

#include "qcustomfilesystemmodel.h"
#include "sshsftp.h"
#include "qfonticon.h"

namespace Ui {
class SftpWindow;
}

class SftpTransferThread : public QThread
{
    Q_OBJECT
public:
    enum TaskType {
        Upload,
        Download
    };
    struct task_t {
        uint64_t id;
        QString srcPath;
        QString dstPath;
        TaskType type;
    };
    SftpTransferThread(QObject *parent = nullptr) :
        QThread(parent), sftp(nullptr) {
        exit = false;
    }
    ~SftpTransferThread() {
        exit = true;
        wait();
    }

    void setSftpChannel(SshSFtp *sftp) {
        mutex.lock();
        this->sftp = sftp;
        mutex.unlock();
    }
    void addTask(task_t &task) {
        mutex.lock();
        task.id = taskId++;
        taskList.append(task);
        mutex.unlock();
    }
    bool isBusy() {
        return !taskList.isEmpty();
    }

signals:
    void taskProgress(uint64_t id, uint64_t done, uint64_t total);
    void taskFinished(uint64_t id, bool ok);

protected:
    void run() override {
        while (!exit) {
            mutex.lock();
            if(sftp) {
                if(taskList.isEmpty()) {
                    mutex.unlock();
                    msleep(100);
                    continue;
                }
                task_t task = taskList.takeFirst();
                mutex.unlock();
                if (task.type == Upload) {
                    sftp->send(task.srcPath, task.dstPath);
                } else if (task.type == Download) {
                    sftp->get(task.srcPath, task.dstPath, true);
                }
                emit taskFinished(task.id, true);
            } else {
                mutex.unlock();
            }
        }
    }
private:
    SshSFtp *sftp;
    QMutex mutex;
    bool exit;
    QList<task_t> taskList;
    uint64_t taskId = 0;
};

class QSshFileSystemModel : public QCustomFileSystemModel
{
    Q_OBJECT
public:
    explicit QSshFileSystemModel(QObject *parent = 0) 
        : QCustomFileSystemModel(parent) {
    }
    ~QSshFileSystemModel() {
    }

    QString separator() const override {
        return "/";
    }

    QStringList pathEntryList(const QString &path) override {
        if(sftp) {
            QStringList list = sftp->readdir(path);
            list.removeOne(".");
            list.removeOne("..");
            if (!m_hideFiles) {
                QStringList result;
                foreach (QString name, list) {
                    if (name.startsWith(".")) {
                        continue;
                    }
                    result.append(name);
                }
                return result;
            } else {
                return list;
            }
        } else {
            return QStringList();
        }
    }

    void pathInfo(QString path, bool &isDir, uint64_t &size, QDateTime &lastModified) override {
        if(sftp) {
            LIBSSH2_SFTP_ATTRIBUTES fileinfo = sftp->getFileInfo(path);
            if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                    isDir = true;
                } else {
                    isDir = false;
                }
                size = fileinfo.filesize;
                lastModified = QDateTime::fromSecsSinceEpoch(fileinfo.mtime);
            }
        }
    }

    bool isDir(const QModelIndex &index) {
        if (!index.isValid())
            return false;
        if (!indexValid(index))
            return false;
        QCustomFileSystemItem *item = static_cast<QCustomFileSystemItem*>(index.internalPointer());
        return item->isDir();
    }

    void setSftpChannel(SshSFtp *sftp) { 
        this->sftp = sftp; 
    }

    void setHideFiles(bool hideFiles) { m_hideFiles = hideFiles; }
    bool hideFiles() const { return m_hideFiles; }

private:
    SshSFtp *sftp = nullptr;
    bool m_hideFiles = false;
};

class SftpWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SftpWindow(QWidget *parent = nullptr);
    ~SftpWindow();
    void setSftpChannel(SshSFtp *sftp);

private:
    Ui::SftpWindow *ui;
    SshSFtp *sftp = nullptr;
    SftpTransferThread *transferThread = nullptr;
    QSshFileSystemModel *sshFileSystemModel;
    QNativeFileSystemModel *fileSystemModel;
    uint64_t taskNum = 0;
    uint64_t taskDone = 0;
};

#endif // SFTPWINDOW_H
