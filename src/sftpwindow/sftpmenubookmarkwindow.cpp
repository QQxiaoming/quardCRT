#include "sftpmenubookmarkwindow.h"
#include "ui_sftpmenubookmarkwindow.h"

SFTPmenuBookmarkWindow::SFTPmenuBookmarkWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SFTPmenuBookmarkWindow)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
}

SFTPmenuBookmarkWindow::~SFTPmenuBookmarkWindow()
{
    delete ui;
}

void SFTPmenuBookmarkWindow::setConfig(const QString &bookmarkName,const QString &localPath, const QString &remotePath) {
    ui->lineEditBookmarkName->setText(bookmarkName);
    ui->lineEditLocalPath->setText(localPath);
    ui->lineEditRemotePath->setText(remotePath);
    bookmarkInitName = bookmarkName;
}

void SFTPmenuBookmarkWindow::getConfig(QString &bookmarkName, QString &localPath, QString &remotePath) {
    bookmarkName = ui->lineEditBookmarkName->text();
    localPath = ui->lineEditLocalPath->text();
    remotePath = ui->lineEditRemotePath->text();
}

QDataStream &operator<<(QDataStream &out, const SFTPmenuBookmarkWindow::Config &config) {
    out << config.bookmarkName << config.localPath << config.remotePath;
    return out;
}

QDataStream &operator>>(QDataStream &in, SFTPmenuBookmarkWindow::Config &config) {
    in >> config.bookmarkName >> config.localPath >> config.remotePath;
    return in;
}
