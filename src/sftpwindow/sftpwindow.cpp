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
#include <QMenu>
#include <QMessageBox>

#include "filedialog.h"
#include "sftpwindow.h"
#include "ui_sftpwindow.h"


SftpWindow::SftpWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SftpWindow)
{
    ui->setupUi(this);
    fileSystemModel = new QNativeFileSystemModel(this);
    ui->treeViewLocal->setModel(fileSystemModel);
    ui->treeViewLocal->setColumnWidth(0, 200);
    ui->treeViewLocal->setColumnWidth(1, 60);
    ui->treeViewLocal->setColumnWidth(2, 100);
    ui->treeViewLocal->setColumnWidth(3, 100);

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
        QModelIndex index = ui->treeViewLocal->indexAt(pos);
        if (index.isValid()) {
            QMenu *menu = new QMenu(this);
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
                                    sftp->send(srcPath, dstPath + "/" + file);
                                }
                            }
                        };
                        uploadDir(path, ui->lineEditPathRemote->text());
                    } else {
                        QString dstPath = ui->lineEditPathRemote->text() + "/" + fileInfo.fileName();
                        sftp->send(path, dstPath);
                    }
                    // FIXME: refresh remote file list
                    ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(sshFileSystemModel->rootPath()));
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
                }
            }
            menu->move(cursor().pos()+QPoint(5,5));
            menu->show();
        }
    });

    sshFileSystemModel = new QSshFileSystemModel(this);
    ui->treeViewRemote->setModel(sshFileSystemModel);
    ui->treeViewRemote->setColumnWidth(0, 200);
    ui->treeViewRemote->setColumnWidth(1, 60);
    ui->treeViewRemote->setColumnWidth(2, 100);
    ui->treeViewRemote->setColumnWidth(3, 100);

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
            QModelIndex index = ui->treeViewRemote->indexAt(pos);
            if (index.isValid()) {
                QMenu *menu = new QMenu(this);
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
                                            sftp->get(srcPath, dstPath + "/" + file, true);
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
                            sftp->get(path, dstPath, true);
                        }
                    }
                    // FIXME: refresh local file list
                    ui->treeViewLocal->setRootIndex(fileSystemModel->setRootPath(fileSystemModel->rootPath()));
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
                menu->move(cursor().pos()+QPoint(5,5));
                menu->show();
            }
        }
    });

    resize(920, 380);
}

SftpWindow::~SftpWindow()
{
    delete ui;
}

void SftpWindow::setSftpChannel(SshSFtp *sftp)
{
    this->sftp = sftp;
    sshFileSystemModel->setSftpChannel(sftp);
    ui->lineEditPathRemote->setText("/");
    ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath("/"));
}
