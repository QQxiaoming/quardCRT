#include <QSplitter>
#include <QDir>
#include <QMenu>

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
                    // TODO: Upload file to remote
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
                // download
                menu->addAction(tr("Download"), [=](){
                    QString path = sshFileSystemModel->filePath(index);
                    if (!path.isEmpty()) {
                        // TODO: Download file from remote
                    }
                });
                // open
                QString path = sshFileSystemModel->filePath(index);
                if (!path.isEmpty()) {
                    if(sftp->isDir(path)) {
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
