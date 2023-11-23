#ifndef SFTPWINDOW_H
#define SFTPWINDOW_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QFileSystemModel>

#include "sshsftp.h"

namespace Ui {
class SftpWindow;
}

class QSshFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QSshFileSystemModel(QObject *parent = nullptr);
    ~QSshFileSystemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex setRootPath(const QString &path);
    QModelIndex setRootPath(const char *path) { return setRootPath(QString::fromUtf8(path)); }
    QString rootPath();

    QString filePath(const QModelIndex &index);
    void setSftpChannel(SshSFtp *sftp) { this->sftp = sftp; }

private:
    QString m_rootPath;
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
    QFileSystemModel *fileSystemModel;
};

#endif // SFTPWINDOW_H
