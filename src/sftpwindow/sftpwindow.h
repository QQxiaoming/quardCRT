#ifndef SFTPWINDOW_H
#define SFTPWINDOW_H

#include <QDialog>
#include <QAbstractItemModel>

#include "qcustomfilesystemmodel.h"
#include "sshsftp.h"
#include "qfonticon.h"

namespace Ui {
class SftpWindow;
}

class QSshFileSystemModel : public QCustomFileSystemModel
{
    Q_OBJECT
public:
    explicit QSshFileSystemModel(QObject *parent = 0) 
        : QCustomFileSystemModel(parent) {
    }
    ~QSshFileSystemModel() {
    }

    QStringList pathEntryList(const QString &path) override {
        if(sftp) {
            QStringList list = sftp->readdir(path);
            list.removeOne(".");
            list.removeOne("..");
            return list;
        } else {
            return QStringList();
        }
    }

    bool isDir(const QString &path) override {
        if(sftp) {
            return sftp->isDir(path);
        } else {
            return false;
        }
    }

    QString separator() override {
        return "/";
    }

    QVariant pathInfo(QString path, int type) const override {
        if(sftp) {
            LIBSSH2_SFTP_ATTRIBUTES fileinfo = sftp->getFileInfo(path);
            switch (type) {
                case 0:
                    return QFileInfo(path).fileName();
                case 1:
                    if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                        if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                            return tr("Directory");
                        } else {
                            return tr("File");
                        }
                    }
                    return QVariant();
                case 2:
                    if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                        if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                            //QStringList list = sftp->readdir(path);
                            //return list.count()-2;
                            return QVariant();
                        } else {
                            if( fileinfo.filesize <= 1024) {
                                return QString("%1 B").arg(fileinfo.filesize);
                            } else if ( fileinfo.filesize <= 1024 * 1024 ) {
                                return QString::number(fileinfo.filesize / 1024.0, 'f', 2) + QString(" KB");
                            } else if ( fileinfo.filesize <= 1024 * 1024 * 1024 ) {
                                return QString::number(fileinfo.filesize / (1024.0 * 1024.0), 'f', 2) + QString(" MB");
                            } else {
                                return QString::number(fileinfo.filesize / (1024.0 * 1024.0 * 1024.0), 'f', 2) + QString(" GB");
                            }
                        }
                    }
                    return QVariant();
                case 3:
                    return QDateTime::fromSecsSinceEpoch(fileinfo.mtime);
                case 4:
                    if (fileinfo.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
                        if (fileinfo.permissions & LIBSSH2_SFTP_S_IFDIR) {
                            return QFontIcon::icon(QChar(0xf07b));
                        } else {
                            return QFontIcon::icon(QChar(0xf15b));
                        }
                    }
                    break;
                default:
                    break;
            }
            return QVariant();
        } else {
            return QVariant();
        }
    }

    void setSftpChannel(SshSFtp *sftp) { 
        this->sftp = sftp; 
    }

private:
    SshSFtp *sftp = nullptr;
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
    QSshFileSystemModel *sshFileSystemModel;
    QNativeFileSystemModel *fileSystemModel;
};

#endif // SFTPWINDOW_H
