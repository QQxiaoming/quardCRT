#ifndef NETSCANWINDOW_H
#define NETSCANWINDOW_H

#include <QDialog>

namespace Ui {
class NetScanWindow;
}

class NetScanWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NetScanWindow(QWidget *parent = nullptr);
    ~NetScanWindow();

    void setScanPort(int port);
    
private:
    void addRow(QString ip, QString port, QString status);

private:
    Ui::NetScanWindow *ui;
};

#endif // NETSCANWINDOW_H
