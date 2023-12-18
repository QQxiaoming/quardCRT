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
#include <QSplitter>
#include <QDir>
#include <QDateTime>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QInputDialog>

#include "filedialog.h"
#include "sftpwindow.h"
#include "globalsetting.h"
#include "ui_sftpwindow.h"

SftpWindow::SftpWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SftpWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    bookmarkWindow = new SFTPmenuBookmarkWindow(this);
    menuBar = new QMenuBar(this);
    menuBookmarks = new QMenu(tr("Bookmarks"), menuBar);
    actionAddBookmark = new QAction(tr("Add Bookmark"), menuBookmarks);
    actionEditBookmark = new QAction(tr("Edit Bookmark"), menuBookmarks);
    actionRemoveBookmark = new QAction(tr("Remove Bookmark"), menuBookmarks);
    menuBookmarks->addAction(actionAddBookmark);
    menuBookmarks->addAction(actionEditBookmark);
    menuBookmarks->addAction(actionRemoveBookmark);
    menuBookmarks->addSeparator();
    menuBar->addMenu(menuBookmarks);
    this->layout()->setMenuBar(menuBar);
    connect(actionAddBookmark, &QAction::triggered, [=](){
        bookmarkWindow->setConfig("","","");
        bookmarkWindow->show();
    });
    connect(actionEditBookmark, &QAction::triggered, [=](){
        QStringList bookmarkNameList;
        foreach(SFTPmenuBookmarkWindow::Config data, bookmarkList) {
            bookmarkNameList.append(data.bookmarkName);
        }
        if(bookmarkNameList.isEmpty()) return;
        bool isOk = false;
        QString bookmarkName = QInputDialog::getItem(this, tr("Edit Bookmark"), tr("Bookmark Name:"), bookmarkNameList, 0, false, &isOk);
        if(!isOk || bookmarkName.isEmpty()) return;
        foreach(SFTPmenuBookmarkWindow::Config data, bookmarkList) {
            if(data.bookmarkName == bookmarkName) {
                bookmarkWindow->setConfig(data);
                bookmarkWindow->show();
                break;
            }
        }
    });
    connect(actionRemoveBookmark, &QAction::triggered, [=](){
        QStringList bookmarkNameList;
        foreach(SFTPmenuBookmarkWindow::Config data, bookmarkList) {
            bookmarkNameList.append(data.bookmarkName);
        }
        if(bookmarkNameList.isEmpty()) return;
        bool isOk = false;
        QString bookmarkName = QInputDialog::getItem(this, tr("Remove Bookmark"), tr("Bookmark Name:"), bookmarkNameList, 0, false, &isOk);
        if(!isOk || bookmarkName.isEmpty()) return;
        foreach(SFTPmenuBookmarkWindow::Config data, bookmarkList) {
            if(data.bookmarkName == bookmarkName) {
                bookmarkList.removeOne(data);
                QList<QAction*> actions = menuBookmarks->actions();
                foreach (QAction *action, actions) {
                    if(action->text() == bookmarkName) {
                        action->deleteLater();
                        break;
                    }
                }
                break;
            }
        }
        GlobalSetting settings;
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out << bookmarkList;
        settings.setValue("SFTPmenuBookmarkWindow/bookmarkList", byteArray);
    });
    connect(bookmarkWindow,&SFTPmenuBookmarkWindow::accepted,this,[&](){
        SFTPmenuBookmarkWindow::Config newData = bookmarkWindow->getConfig();
        if(newData.bookmarkName.isEmpty()) return;
        QString oldNmae = bookmarkWindow->getBookmarkInitName();
        if(!oldNmae.isEmpty()) {
            foreach(SFTPmenuBookmarkWindow::Config data, bookmarkList) {
                if(data.bookmarkName == oldNmae) {
                    bookmarkList.removeOne(data);
                    QList<QAction*> actions = menuBookmarks->actions();
                    foreach (QAction *action, actions) {
                        if(action->text() == oldNmae) {
                            action->deleteLater();
                            break;
                        }
                    }
                    break;
                }
            }
        }
        bookmarkList.append(newData);
        QAction *action = new QAction(newData.bookmarkName, menuBookmarks);
        menuBookmarks->addAction(action);
        connect(action, &QAction::triggered, [=](){
            ui->lineEditPathLocal->setText(newData.localPath);
            ui->lineEditPathRemote->setText(newData.remotePath);
            ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(newData.localPath));
            ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(newData.remotePath));
        });
        GlobalSetting settings;
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out << bookmarkList;
        settings.setValue("SFTPmenuBookmarkWindow/bookmarkList", byteArray);
    });

    QByteArray byteArray;
    GlobalSetting settings;
    byteArray = settings.value("SFTPmenuBookmarkWindow/bookmarkList").toByteArray();
    QDataStream in(&byteArray, QIODevice::ReadOnly);
    in >> bookmarkList;

    foreach(SFTPmenuBookmarkWindow::Config data, bookmarkList) {
        QAction *action = new QAction(data.bookmarkName, menuBookmarks);
        menuBookmarks->addAction(action);
        connect(action, &QAction::triggered, [=](){
            ui->lineEditPathLocal->setText(data.localPath);
            ui->lineEditPathRemote->setText(data.remotePath);
            ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(data.localPath));
            ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(data.remotePath));
        });
    }

    transferThread = new SftpTransferThread();
    connect(transferThread, &SftpTransferThread::taskFinished,this,[=](uint64_t id, bool ok){
        taskDone++;
        foreach (QListWidgetItem *item, ui->listWidgetTransfer->findItems(QString::number(id) + " : ", Qt::MatchStartsWith)) {
            if (ok) {
                item->setText(item->text() + " [OK]");
            } else {
                item->setText(item->text() + " [Failed]");
            } 
        }
    });
    transferThread->start();

    QTimer *timerRefreshProgressBar = new QTimer(this);
    connect(timerRefreshProgressBar, &QTimer::timeout, [=](){
        QString speed;
        uint64_t size = dataSize>lastDataSize?(dataSize-lastDataSize):0;
        size = size * 2;
        if(size < 1024) {
            speed = " " + QString::number(size) + "B/s";
        } else if(size < 1024*1024) {
            speed = " " + QString::number(size / 1024) + "KB/s";
        } else if(size < 1024*1024*1024) {
            speed = " " + QString::number(size / 1024 / 1024) + "MB/s";
        } else {
            speed = " " + QString::number(size / 1024 / 1024 / 1024) + "GB/s";
        }
        if (taskNum == 0) {
            ui->progressBarTransfer->setValue(100);
            ui->progressBarTransfer->setFormat(tr("No task!") + speed);
        } else {
            if(taskDone == taskNum) {
                ui->progressBarTransfer->setValue(100);
                ui->progressBarTransfer->setFormat(tr("All tasks finished!") + speed);
            } else {
                uint64_t progressSize = dataSize * 100 / needDataSize;
                uint64_t progressNum = taskDone * 100 / taskNum;
                ui->progressBarTransfer->setValue(progressSize > progressNum ? progressSize:progressNum);
                ui->progressBarTransfer->setFormat(tr("task %1/%2").arg(taskDone).arg(taskNum) + speed);
            }
        }
        lastDataSize = dataSize;
    });
    timerRefreshProgressBar->start(500);

    fileSystemModel = new QNativeFileSystemModel(this);
    ui->treeViewLocal->setModel(fileSystemModel);
    ui->treeViewLocal->setColumnWidth(0, 200);
    ui->treeViewLocal->setColumnWidth(1, 60);
    ui->treeViewLocal->setColumnWidth(2, 100);
    ui->treeViewLocal->setColumnWidth(3, 100);
    ui->treeViewLocal->setSelectionMode(QAbstractItemView::MultiSelection);

    ui->lineEditPathLocal->setText(QDir::homePath());
    ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(QDir::homePath()));
    connect(ui->toolButtonPathLocal, &QToolButton::clicked, [=](){
        QString path = FileDialog::getExistingDirectory(this, tr("Open Directory"), ui->lineEditPathLocal->text());
        if (!path.isEmpty()) {
            ui->lineEditPathLocal->setText(path);
            ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(path));
        }
    });

    ui->treeViewLocal->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeViewLocal, &QTreeView::customContextMenuRequested, [=](const QPoint &pos){
        QMenu *menu = new QMenu(this);
        QModelIndexList indexList = ui->treeViewLocal->selectionModel()->selectedIndexes();
        QStringList pathList;
        foreach (QModelIndex index, indexList) {
            QString path = fileSystemModel->filePath(index);
            if(pathList.contains(path)) 
                continue;
            pathList.append(path);
        }
        if (pathList.isEmpty() || pathList.count() == 1) {
            QModelIndex index = ui->treeViewLocal->indexAt(pos);
            ui->treeViewLocal->clearSelection();
            if (index.isValid()) {
                ui->treeViewLocal->setCurrentIndex(index);
                menu->addAction(tr("Show/Hide Files"), [=](){
                    fileSystemModel->setHideFiles(!fileSystemModel->hideFiles());
                    ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(fileSystemModel->rootPath()));
                });
                menu->addAction(tr("Upload"), [=](){
                    QString path = fileSystemModel->filePath(index);
                    if (!path.isEmpty()) {
                        QFileInfo fileInfo(path);
                        if (fileInfo.isDir()) {
                            std::function<void(const QString &, const QString &)> uploadDir =
                            [&](const QString &path, const QString &dstPathR) {
                                QString dirName = QFileInfo(path).fileName();
                                QStringList dstlist = sftp->readdir(dstPathR);
                                if (!dstlist.contains(dirName)) {
                                    sftp->mkdir(dstPathR + "/" + dirName);
                                }
                                QStringList filelist = QDir(path).entryList(QDir::NoDotAndDotDot);
                                foreach (QString file, filelist) {
                                    QString srcPath = path + "/" + file;
                                    QString dstPath = dstPathR + "/" + dirName;
                                    QFileInfo fileInfo(srcPath);
                                    if (fileInfo.isDir()) {
                                        uploadDir(srcPath, dstPath);
                                    } else {
                                        SftpTransferThread::task_t task;
                                        task.srcPath = srcPath;
                                        task.dstPath = dstPath + "/" + file;
                                        task.type = SftpTransferThread::Upload;
                                        transferThread->addTask(task);
                                        needDataSize += fileInfo.size();
                                        ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                                        ui->listWidgetTransfer->scrollToBottom();
                                        taskNum++;
                                    }
                                }
                            };
                            uploadDir(path, ui->lineEditPathRemote->text());
                        } else {
                            QString dstPath = ui->lineEditPathRemote->text() + "/" + fileInfo.fileName();
                            SftpTransferThread::task_t task;
                            task.srcPath = path;
                            task.dstPath = dstPath;
                            task.type = SftpTransferThread::Upload;
                            transferThread->addTask(task);
                            needDataSize += fileInfo.size();
                            ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                            ui->listWidgetTransfer->scrollToBottom();
                            taskNum++;
                        }
                    }
                });
                QString path = fileSystemModel->filePath(index);
                if (!path.isEmpty()) {
                    QFileInfo fileInfo(path);
                    if (fileInfo.isDir()) {
                        menu->addAction(tr("Open"), [=](){
                            ui->lineEditPathLocal->setText(path);
                            ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(path));
                        });
                        menu->addAction(tr("Open in System File Manager"), [=](){
                            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
                        });
                    }
                }
                menu->addAction(tr("refresh"), [=](){
                    ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(fileSystemModel->rootPath()));
                });
            }
        } else if(pathList.count() > 1) {
            menu->addAction(tr("Upload All"), [=](){
                std::function<void(const QStringList &, const QString &)> uploadDir =
                [&](const QStringList &pathList, const QString &dstPathR) {
                    QString dirName = QFileInfo(pathList.first()).fileName();
                    QStringList dstlist = sftp->readdir(dstPathR);
                    if (!dstlist.contains(dirName)) {
                        sftp->mkdir(dstPathR + "/" + dirName);
                    }
                    foreach (QString path, pathList) {
                        QFileInfo fileInfo(path);
                        if (fileInfo.isDir()) {
                            uploadDir(pathList, dstPathR + "/" + dirName);
                        } else {
                            SftpTransferThread::task_t task;
                            task.srcPath = path;
                            task.dstPath = dstPathR + "/" + dirName + "/" + fileInfo.fileName();
                            task.type = SftpTransferThread::Upload;
                            transferThread->addTask(task);
                            needDataSize += fileInfo.size();
                            ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                            ui->listWidgetTransfer->scrollToBottom();
                            taskNum++;
                        }
                    }
                };
                foreach (QString path, pathList) {
                    QFileInfo fileInfo(path);
                    if (fileInfo.isDir()) {
                        uploadDir(pathList, ui->lineEditPathRemote->text());
                        break;
                    } else {
                        QString dstPath = ui->lineEditPathRemote->text() + "/" + fileInfo.fileName();
                        SftpTransferThread::task_t task;
                        task.srcPath = path;
                        task.dstPath = dstPath;
                        task.type = SftpTransferThread::Upload;
                        transferThread->addTask(task);
                        needDataSize += fileInfo.size();
                        ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                        ui->listWidgetTransfer->scrollToBottom();
                        taskNum++;
                    }
                }
            });
            menu->addAction(tr("Cancel Selection"), [=](){
                ui->treeViewLocal->clearSelection();
            });
        }
        if(menu->isEmpty()) {
            delete menu;
            return;
        }
        menu->move(cursor().pos()+QPoint(5,5));
        menu->show();
    });

    sshFileSystemModel = new QSshFileSystemModel(this);
    ui->treeViewRemote->setModel(sshFileSystemModel);
    ui->treeViewRemote->setColumnWidth(0, 200);
    ui->treeViewRemote->setColumnWidth(1, 60);
    ui->treeViewRemote->setColumnWidth(2, 100);
    ui->treeViewRemote->setColumnWidth(3, 100);
    ui->treeViewRemote->setSelectionMode(QAbstractItemView::MultiSelection);

    if(sftp) {
        ui->lineEditPathRemote->setText("/");
        ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath("/"));
    }
    connect(ui->toolButtonPathRemote, &QToolButton::clicked, [=](){
        if(sftp) {
            QString path = ui->lineEditPathRemote->text();
            LIBSSH2_SFTP_ATTRIBUTES fileinfo = sftp->getFileInfo(path);
            if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                    ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(path));
                }
            }
        }
    });

    ui->treeViewRemote->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeViewRemote, &QTreeView::customContextMenuRequested, [=](const QPoint &pos){
        if(sftp) {
            QMenu *menu = new QMenu(this);
            QModelIndexList indexList = ui->treeViewRemote->selectionModel()->selectedIndexes();
            QStringList pathList;
            foreach (QModelIndex index, indexList) {
                QString path = sshFileSystemModel->filePath(index);
                if(pathList.contains(path)) 
                    continue;
                pathList.append(path);
            }
            if (pathList.isEmpty() || pathList.count() == 1) {
                QModelIndex index = ui->treeViewRemote->indexAt(pos);
                ui->treeViewRemote->clearSelection();
                if (index.isValid()) {
                    ui->treeViewRemote->setCurrentIndex(index);
                    menu->addAction(tr("Show/Hide Files"), [=](){
                        sshFileSystemModel->setHideFiles(!sshFileSystemModel->hideFiles());
                        ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(sshFileSystemModel->rootPath()));
                    });
                    menu->addAction(tr("Download"), [=](){
                        QString path = sshFileSystemModel->filePath(index);
                        if (!path.isEmpty()) {
                            if(sshFileSystemModel->isDir(index)) {
                                std::function<void(const QString &, const QString &)> downloadDir = 
                                [&](const QString &path, const QString &dstPathR) {
                                    QString dirName = QFileInfo(path).fileName();
                                    QDir dir(dstPathR);
                                    if (!dir.exists(dirName)) {
                                        dir.mkdir(dirName);
                                    }
                                    QStringList filelist = sftp->readdir(path);
                                    filelist.removeOne(".");
                                    filelist.removeOne("..");
                                    foreach (QString file, filelist) {
                                        QString srcPath = path + "/" + file;
                                        QString dstPath = dstPathR + "/" + dirName;
                                        LIBSSH2_SFTP_ATTRIBUTES fileinfo = sftp->getFileInfo(srcPath);
                                        if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                                            if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                                                downloadDir(srcPath, dstPath);
                                            } else {
                                                SftpTransferThread::task_t task;
                                                task.srcPath = srcPath;
                                                task.dstPath = dstPath + "/" + file;
                                                task.type = SftpTransferThread::Download;
                                                transferThread->addTask(task);
                                                needDataSize += fileinfo.filesize;
                                                ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                                                ui->listWidgetTransfer->scrollToBottom();
                                                taskNum++;
                                            }
                                        }
                                    }
                                };
                                downloadDir(path, ui->lineEditPathLocal->text());
                            } else {
                                QString dstPath = ui->lineEditPathLocal->text() + QDir::separator() + QFileInfo(path).fileName();
                                QFileInfo fileInfo(dstPath);
                                if (fileInfo.exists()) {
                                    bool ok = QMessageBox::question(this, tr("File exists"), tr("File %1 already exists. Do you want to overwrite it?").arg(dstPath)) == QMessageBox::Yes;
                                    if (!ok) {
                                        return;
                                    }
                                }
                                SftpTransferThread::task_t task;
                                task.srcPath = path;
                                task.dstPath = dstPath;
                                task.type = SftpTransferThread::Download;
                                transferThread->addTask(task);
                                needDataSize += sshFileSystemModel->filesize(index);
                                ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                                ui->listWidgetTransfer->scrollToBottom();
                                taskNum++;
                            }
                        }
                    });
                    QString path = sshFileSystemModel->filePath(index);
                    if (!path.isEmpty()) {
                        if(sshFileSystemModel->isDir(index)) {
                            menu->addAction(tr("Open"), [=](){
                                ui->lineEditPathRemote->setText(path);
                                ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(path));
                            });
                        }
                    }
                    menu->addAction(tr("refresh"), [=](){
                        ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(sshFileSystemModel->rootPath()));
                    });
                }
            } else if(pathList.count() > 1) {
                menu->addAction(tr("Download All"), [=](){
                    std::function<void(const QStringList &, const QString &)> downloadDir = 
                    [&](const QStringList &pathList, const QString &dstPathR) {
                        QString dirName = QFileInfo(pathList.first()).fileName();
                        QDir dir(dstPathR);
                        if (!dir.exists(dirName)) {
                            dir.mkdir(dirName);
                        }
                        foreach (QString path, pathList) {
                            LIBSSH2_SFTP_ATTRIBUTES fileinfo = sftp->getFileInfo(path);
                            if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                                if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                                    downloadDir(pathList, dstPathR + "/" + dirName);
                                } else {
                                    SftpTransferThread::task_t task;
                                    task.srcPath = path;
                                    task.dstPath = dstPathR + "/" + dirName + "/" + QFileInfo(path).fileName();
                                    task.type = SftpTransferThread::Download;
                                    transferThread->addTask(task);
                                    needDataSize += fileinfo.filesize;
                                    ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                                    ui->listWidgetTransfer->scrollToBottom();
                                    taskNum++;
                                }
                            }
                        }
                    };
                    foreach (QString path, pathList) {
                        LIBSSH2_SFTP_ATTRIBUTES fileinfo = sftp->getFileInfo(path);
                        if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                            if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                                downloadDir(pathList, ui->lineEditPathLocal->text());
                                break;
                            } else {
                                QString dstPath = ui->lineEditPathLocal->text() + QDir::separator() + QFileInfo(path).fileName();
                                SftpTransferThread::task_t task;
                                task.srcPath = path;
                                task.dstPath = dstPath;
                                task.type = SftpTransferThread::Download;
                                transferThread->addTask(task);
                                needDataSize += fileinfo.filesize;
                                ui->listWidgetTransfer->addItem(QString::number(task.id) + " : " + task.srcPath + " -> " + task.dstPath + " " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " transfering...");
                                ui->listWidgetTransfer->scrollToBottom();
                                taskNum++;
                            }
                        }
                    }
                });
                menu->addAction(tr("Cancel Selection"), [=](){
                    ui->treeViewRemote->clearSelection();
                });
            }
            if(menu->isEmpty()) {
                delete menu;
                return;
            }
            menu->move(cursor().pos()+QPoint(5,5));
            menu->show();
        }
    });

    resize(1020, 480);
}

SftpWindow::~SftpWindow()
{
    delete transferThread;
    delete ui;
}

void SftpWindow::setSftpChannel(SshSFtp *sftp)
{
    while(transferThread->isBusy()) {
        QThread::msleep(100);
        QApplication::processEvents();
    }
    if(this->sftp) {
        disconnect(this->sftp, &SshSFtp::progress, this, &SftpWindow::progress);
    }
    taskNum = 0;
    taskDone = 0;
    dataSize = 0;
    lastDataSize = 0;
    needDataSize = 0;
    ui->listWidgetTransfer->clear();
    this->sftp = sftp;
    connect(this->sftp, &SshSFtp::progress, this, &SftpWindow::progress);
    transferThread->setSftpChannel(sftp);
    sshFileSystemModel->setSftpChannel(sftp);
    ui->lineEditPathRemote->setText("/");
    ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath("/"));
}

void SftpWindow::progress(qint64 size)
{
    dataSize += size;
}
