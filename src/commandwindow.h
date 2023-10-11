#ifndef COMMANDWINDOW_H
#define COMMANDWINDOW_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class CommandWindow;
}

class CommandWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CommandWindow(QWidget *parent = nullptr);
    ~CommandWindow();
    void setCmd(QString cmd);
    void retranslateUi(void);

signals:
    void sendData(QByteArray data);

private:
    Ui::CommandWindow *ui;
    QTimer *autoSendTimer;
};

#endif // COMMANDWINDOW_H
