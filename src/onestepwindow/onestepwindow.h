#ifndef ONESTEPWINDOW_H
#define ONESTEPWINDOW_H

#include <QDialog>
#include "passwordedit.h"

namespace Ui {
class OneStepWindow;
}

class OneStepWindow : public QDialog
{
    Q_OBJECT

public:
    class Config {
    public:
        QString stepName;
        QString userName;
        QString password;
        int port;
        Config() {
            stepName = "";
            userName = "";
            password = "";
            port = 0;
        }
        Config(const QString &stepName, const QString &userName, const QString &password, const int &port) {
            this->stepName = stepName;
            this->userName = userName;
            this->password = password;
            this->port = port;
        }
        bool operator==(const Config &config) const{
            return (this->stepName == config.stepName &&
                    this->userName == config.userName &&
                    this->password == config.password &&
                    this->port == config.port);
        }
        bool operator!=(const Config &config) const{
            return !(*this == config);
        }
    };
    explicit OneStepWindow(QWidget *parent = nullptr);
    ~OneStepWindow();
    void setConfig(const QString &stepName, const QString &userName, const QString &password, const int &port);
    void setConfig(const Config &config) {
        setConfig(config.stepName, config.userName, config.password, config.port);
    }
    void getConfig(QString &stepName, QString &userName, QString &password, int &port);
    Config getConfig(void) {
        Config config;
        getConfig(config.stepName, config.userName, config.password, config.port);
        return config;
    }
    QString getStepInitName(void) {
        return stepInitName;
    }

private:
    Ui::OneStepWindow *ui;
    PasswordEdit *lineEditPassword;
    QString stepInitName;
};

#endif // ONESTEPWINDOW_H
