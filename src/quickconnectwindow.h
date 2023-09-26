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

    enum QuickConnectType {
        Telnet,
        Serial,
        LocalShell,
        Raw,
    };
    struct QuickConnectData {
        QuickConnectType type;
        struct {
            QString hostname;
            int port;
            QString webSocket;
        }TelnetData;
        struct {
            QString portName;
            uint32_t baudRate;
            int dataBits;
            int parity;
            int stopBits;
            bool flowControl;
            bool xEnable;
        }SerialData;
        struct {
            QString command;
        }LocalShellData;
        struct {
            QString hostname;
            int port;
        }RawData;
    };
    
signals:
    void sendQuickConnectData(QuickConnectData);

private slots:
    void comboBoxProtocolChanged(int index);
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

private:
    Ui::QuickConnectWindow *ui;
};

#endif // QUICKCONNECTWINDOW_H
