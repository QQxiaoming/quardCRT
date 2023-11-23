#include <QSplitter>
#include <QDir>
#include <QMenu>

#include "filedialog.h"
#include "qfonticon.h"
#include "sftpwindow.h"
#include "sshsftpcommandmkdir.h"
#include "sshsftpcommandget.h"
#include "sshsftpcommandsend.h"
#include "sshsftpcommandreaddir.h"
#include "sshsftpcommandfileinfo.h"
#include "ui_sftpwindow.h"

QSshFileSystemModel::QSshFileSystemModel(QObject *parent) 
    : QAbstractItemModel(parent) {
}

QSshFileSystemModel::~QSshFileSystemModel() {
}

QModelIndex QSshFileSystemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        return createIndex(row, column, (void *)m_rootPath.toUtf8().data());
    }

    QString parentPath = (const char *)parent.internalPointer();
    QString path = QDir(parentPath).entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).at(row);
    return createIndex(row, column, (void *)path.toUtf8().data());
}

QModelIndex QSshFileSystemModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return QModelIndex();

    QString childPath = (const char *)child.internalPointer();
    QString parentPath = QDir(childPath).absolutePath();
    if (parentPath == m_rootPath) {
        return QModelIndex();
    }
    return createIndex(0, 0, (void *)parentPath.toUtf8().data());
}

int QSshFileSystemModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;

    QString parentPath = (const char *)parent.internalPointer();
    QDir parentDir(parentPath);
    return parentDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).count();
}

int QSshFileSystemModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4;
}

QVariant QSshFileSystemModel::data(const QModelIndex &index, int role) const {
    if (!sftp) {
        return QVariant();
    }
    if (!index.isValid())
        return QVariant();

    QString path = (const char *)index.internalPointer();
    SshSftpCommandFileInfo fileInfo(path,*sftp);
    fileInfo.process();
    if (fileInfo.error()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return QFileInfo(path).fileName();
        case 1:
            if (fileInfo.fileinfo().flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                if (fileInfo.fileinfo().permissions & LIBSSH2_SFTP_S_IFDIR) {
                    return tr("Directory");
                } else {
                    return tr("File");
                }
            }
        case 2:
            return fileInfo.fileinfo().filesize;
        case 3:
            return qint64(fileInfo.fileinfo().mtime);
        }
        break;
    case Qt::DecorationRole:
        switch (index.column()) {
        case 0:
            if (fileInfo.fileinfo().flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                if (fileInfo.fileinfo().permissions & LIBSSH2_SFTP_S_IFDIR) {
                    return QFontIcon::icon(QChar(0xf07b));
                } else {
                    return QFontIcon::icon(QChar(0xf15b));
                }
            }
            break;
        }
    }
    return QVariant();
}

QModelIndex QSshFileSystemModel::setRootPath(const QString &path) {
    m_rootPath = path;
    return createIndex(0, 0, (void *)m_rootPath.toUtf8().data());
}

QString QSshFileSystemModel::rootPath() {
    return m_rootPath;
}

QString QSshFileSystemModel::filePath(const QModelIndex &index) {
    if (!index.isValid())
        return QString();

    QString path = (const char *)index.internalPointer();
    return path;
}

SftpWindow::SftpWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SftpWindow)
{
    ui->setupUi(this);
    fileSystemModel = new QFileSystemModel(this);
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
                    SshSftpCommandFileInfo fileInfo(path,*sftp);
                    fileInfo.process();
                    if (fileInfo.error()) {
                        return;
                    }
                    if (fileInfo.fileinfo().flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                        if (fileInfo.fileinfo().permissions & LIBSSH2_SFTP_S_IFDIR) {
                            menu->addAction(tr("Open"), [=](){
                                ui->lineEditPathRemote->setText(path);
                                ui->treeViewRemote->setRootIndex(sshFileSystemModel->setRootPath(path));
                            });
                        }
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
