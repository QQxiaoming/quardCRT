#ifndef SFTPMENUBOOKMARKWIDGET_H
#define SFTPMENUBOOKMARKWIDGET_H

#include <QWidget>

namespace Ui {
class SFTPmenuBookmarkWidget;
}

class SFTPmenuBookmarkWidget : public QWidget
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
        friend QDataStream &operator<<(QDataStream &out, const SFTPmenuBookmarkWidget::Config &config);
        friend QDataStream &operator>>(QDataStream &in, SFTPmenuBookmarkWidget::Config &config);
    };
    explicit SFTPmenuBookmarkWidget(QWidget *parent = nullptr);
    ~SFTPmenuBookmarkWidget();

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

signals:
    void accepted();
    void rejected();

private:
    Ui::SFTPmenuBookmarkWidget *ui;
    QString bookmarkInitName;
};

#endif // SFTPMENUBOOKMARKWIDGET_H
