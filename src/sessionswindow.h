#ifndef SESSIONSWINDOW_H
#define SESSIONSWINDOW_H

#include <QObject>
#include <QWidget>
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
        Telnet,
        Serial,
        LocalShell,
        RawSocket
    };
    SessionsWindow(SessionType tp, QWidget *parent = nullptr);
    ~SessionsWindow();

    void cloneSession(SessionsWindow *src);
    int startLocalShellSession(const QString &command);
    int startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type);
    int startSerialSession(const QString &portName, uint32_t baudRate,
                    int dataBits, int parity, int stopBits, bool flowControl, bool xEnable );
    int startRawSocketSession(const QString &hostname, quint16 port);

    void setWorkingDirectory(const QString &dir);
    const QString getWorkingDirectory(void) { return workingDirectory; }
    
    int setLog(bool enable);
    bool isLog(void) { return enableLog; }
    int setRawLog(bool enable);
    bool isRawLog(void) { return enableRawLog; }

    QTermWidget *getTermWidget() const { return term; }
    SessionType getSessionType() const { return type; }
    QString getTitle() const { return showShortTitle ? shortTitle : longTitle; }
    QString getLongTitle() const { return longTitle; }
    QString getShortTitle() const { return shortTitle; }
    void setShowShortTitle(bool show) { showShortTitle = show; }
    bool getShowShortTitle() const { return showShortTitle; }
    void setLongTitle(const QString &title) { longTitle = title; }
    void setShortTitle(const QString &title) { shortTitle = title; }
    void setName(const QString &name) { this->name = name; }
    QString getName() const { return name; }

signals:
    void hexDataDup(const char *data, int size);

private:
    int saveLog(const char *data, int size);
    int saveRawLog(const char *data, int size);

public:
    QString m_hostname;
    quint16 m_port;
    QTelnet::SocketType m_type;
    QString m_portName;
    uint32_t m_baudRate;
    int m_dataBits;
    int m_parity;
    int m_stopBits;
    bool m_flowControl;
    bool m_xEnable;
    QString m_command;

private:
    SessionType type;
    QString workingDirectory;
    QString longTitle;
    QString shortTitle;
    QString name;
    bool showShortTitle;
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
