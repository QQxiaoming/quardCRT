#ifndef SESSIONSWINDOW_H
#define SESSIONSWINDOW_H

#include <QObject>
#include <QTcpSocket>
#include <QSerialPort>
#include <QMutex>

#include "qtermwidget.h"
#include "QTelnet.h"
#include "ptyqt.h"

class SessionsWindow : public QObject
{
    Q_OBJECT
public:
    enum SessionType {
        LocalShell,
        Telnet,
        Serial,
        RawSocket
    };
    SessionsWindow(SessionType tp, QObject *parent = nullptr);
    ~SessionsWindow();

    int startLocalShellSession(const QString &command);
    int startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type);
    int startSerialSession(const QString &portName, uint32_t baudRate,
                    int dataBits, int parity, int stopBits, bool flowControl, bool xEnable );
    int startRawSocketSession(const QString &hostname, quint16 port);

    int setLog(bool enable);
    bool isLog(void) { return enableLog; }
    int setRawLog(bool enable);
    bool isRawLog(void) { return enableRawLog; }

    QTermWidget *getTermWidget() const { return term; }

private:
    int saveLog(const char *data, int size);
    int saveRawLog(const char *data, int size);

private:
    SessionType type;
    QTermWidget *term;
    QTelnet *telnet;
    QSerialPort *serialPort;
    QTcpSocket *rawSocket;
    IPtyProcess *localShell;
    bool enableLog;
    bool enableRawLog;
    QMutex log_file_mutex;
    QMutex raw_log_file_mutex;
    QFile *log_file = nullptr;
    QFile *raw_log_file = nullptr;
    bool fflush_file = true;
};

#endif // SESSIONSWINDOW_H
