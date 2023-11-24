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

    QString separator() const override {
        return "/";
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
