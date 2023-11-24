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
            // upload
            menu->addAction(tr("Upload"), [=](){
                QString path = fileSystemModel->filePath(index);
                if (!path.isEmpty()) {
                    QFileInfo fileInfo(path);
                    if (fileInfo.isDir()) {
                        //TODO: Upload directory to remote
                        QMessageBox::information(this, tr("Not implemented"), tr("Upload directory to remote is not implemented yet."));
                    } else {
                        QString dstPath = ui->lineEditPathRemote->text() + "/" + fileInfo.fileName();
                        sftp->send(path, dstPath);
                    }
                }
            });
            // open
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
            QString path = FileDialog::getExistingDirectory(this, tr("Open Directory"), ui->lineEditPathRemote->text());
            if (!path.isEmpty()) {
                ui->lineEditPathRemote->setText(path);
                ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(path));
            }
        }
    });

    ui->treeViewRemote->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeViewRemote, &QTreeView::customContextMenuRequested, [=](const QPoint &pos){
        if(sftp) {
            QModelIndex index = ui->treeViewRemote->indexAt(pos);
            if (index.isValid()) {
                QMenu *menu = new QMenu(this);
                menu->addAction(tr("Download"), [=](){
                    QString path = sshFileSystemModel->filePath(index);
                    if (!path.isEmpty()) {
                        if(sshFileSystemModel->isDir(index)) {
                            //TODO: Download directory from remote
                            QMessageBox::information(this, tr("Not implemented"), tr("Download directory from remote is not implemented yet."));
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
