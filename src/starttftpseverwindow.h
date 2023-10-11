#ifndef STARTTFTPSEVERWINDOW_H
#define STARTTFTPSEVERWINDOW_H

#include <QDialog>

namespace Ui {
class StartTftpSeverWindow;
}

class StartTftpSeverWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StartTftpSeverWindow(QWidget *parent = nullptr);
    ~StartTftpSeverWindow();

    void retranslateUi(void);
    
signals:
    void setTftpDir(QString updir, QString downdir);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::StartTftpSeverWindow *ui;
};

#endif // STARTTFTPSEVERWINDOW_H
