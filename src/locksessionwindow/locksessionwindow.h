#ifndef LOCKSESSIONWINDOW_H
#define LOCKSESSIONWINDOW_H

#include <QDialog>

namespace Ui {
class LockSessionWindow;
}

class LockSessionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LockSessionWindow(QWidget *parent = nullptr);
    ~LockSessionWindow();

    void showLock(void);
    void showUnlock(void);

signals:
    void sendLockSessionData(QString password, bool lockAllSession, bool lockAllSessionGroup);
    void sendUnLockSessionData(QString password, bool lockAllSession);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

private:
    Ui::LockSessionWindow *ui;
    bool islock;
};

#endif // LOCKSESSIONWINDOW_H
