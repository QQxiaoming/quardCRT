#ifndef SFTPMENUBOOKMARKWINDOW_H
#define SFTPMENUBOOKMARKWINDOW_H

#include <QDialog>

namespace Ui {
class SFTPmenuBookmarkWindow;
}

class SFTPmenuBookmarkWindow : public QDialog
{
    Q_OBJECT

public:
 class Config {
    public:
        QString bookmarkName;
        QString localPath;
        QString remotePath;
        Config() {
            bookmarkName = "";
            localPath = "";
            remotePath = "";
        }
        Config(const QString &bookmarkName, const QString &localPath, const QString &remotePath) {
            this->bookmarkName = bookmarkName;
            this->localPath = localPath;
            this->remotePath = remotePath;
        }
        bool operator==(const Config &config) const{
            return (this->bookmarkName == config.bookmarkName &&
                    this->localPath == config.localPath &&
                    this->localPath == config.localPath);
        }
        bool operator!=(const Config &config) const{
            return !(*this == config);
        }
        friend QDataStream &operator<<(QDataStream &out, const SFTPmenuBookmarkWindow::Config &config);
        friend QDataStream &operator>>(QDataStream &in, SFTPmenuBookmarkWindow::Config &config);
    };
    explicit SFTPmenuBookmarkWindow(QWidget *parent = nullptr);
    ~SFTPmenuBookmarkWindow();

    void setConfig(const QString &bookmarkName,const QString &localPath, const QString &remotePath);
    void setConfig(const Config &config) {
        setConfig(config.bookmarkName, config.localPath, config.remotePath);
    }
    void getConfig(QString &bookmarkName, QString &localPath, QString &remotePath);
    Config getConfig(void) {
        Config config;
        getConfig(config.bookmarkName, config.localPath, config.remotePath);
        return config;
    }
    QString getBookmarkInitName(void) {
        return bookmarkInitName;
    }

private:
    Ui::SFTPmenuBookmarkWindow *ui;
    QString bookmarkInitName;
};

#endif // SFTPMENUBOOKMARKWINDOW_H
