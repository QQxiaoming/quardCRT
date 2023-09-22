#ifndef QUICKCONNECTWINDOW_H
#define QUICKCONNECTWINDOW_H

#include <QDialog>

namespace Ui {
class QuickConnectWindow;
}

class QuickConnectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit QuickConnectWindow(QWidget *parent = nullptr);
    ~QuickConnectWindow();

signals:
    void sendQuickConnectData(QString, int, QString, QString);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

private:
    Ui::QuickConnectWindow *ui;
};

#endif // QUICKCONNECTWINDOW_H
