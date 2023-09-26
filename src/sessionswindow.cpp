#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>
#include <QProcessEnvironment>

#include "sessionswindow.h"

SessionsWindow::SessionsWindow(SessionType tp, QObject *parent)
    : QObject(parent)
    , type(tp)
    , term(nullptr)
    , telnet(nullptr)
    , serialPort(nullptr)
    , rawSocket(nullptr)
    , localShell(nullptr) {

    term = new QTermWidget(0,static_cast<QWidget *>(parent));

    term->setScrollBarPosition(QTermWidget::NoScrollBar);

    QStringList availableColorSchemes = term->availableColorSchemes();
    availableColorSchemes.sort();
    QString currentColorScheme = availableColorSchemes.first();
    foreach(QString colorScheme, availableColorSchemes) {
        if(colorScheme == "WhiteOnBlack") {
            term->setColorScheme("WhiteOnBlack");
            currentColorScheme = "WhiteOnBlack";
        }
    }

    QStringList availableKeyBindings = term->availableKeyBindings();
    availableKeyBindings.sort();
    QString currentAvailableKeyBindings = availableKeyBindings.first();
    foreach(QString keyBinding, availableKeyBindings) {
        if(keyBinding == "linux") {
            term->setKeyBindings("linux");
            currentAvailableKeyBindings = "linux";
        }
    }
    term->startTerminalTeletype();

    switch (type) {
        case LocalShell: {
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            IPtyProcess::PtyType ptyType = IPtyProcess::UnixPty;
        #elif defined(Q_OS_WIN)
            IPtyProcess::PtyType ptyType = IPtyProcess::WinPty;
            //qint32 buildNumber = QSysInfo::kernelVersion().split(".").last().toInt();
            //if (buildNumber >= CONPTY_MINIMAL_WINDOWS_VERSION) {
            //    ptyType = IPtyProcess::ConPty;
            //}
        #endif
            localShell = PtyQt::createPtyProcess(ptyType);
            break;
        }
        case Telnet: {
            telnet = new QTelnet(QTelnet::TCP, this);
            connect(telnet,&QTelnet::newData,this,
                [=](const char *data, int size){
                term->recvData(data, size);
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(telnet->isConnected())
                    telnet->sendData(data, size);
            });
            break;
        }
        case Serial: {
            serialPort = new QSerialPort(this);
            connect(serialPort,&QSerialPort::readyRead,this,
                [=](){
                QByteArray data = serialPort->readAll();
                term->recvData(data.data(), data.size());
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(serialPort->isOpen()) {
                    serialPort->write(data, size);
                }
            });
            break;
        }
        case RawSocket: {
            rawSocket = new QTcpSocket(this);
            connect(rawSocket,&QTcpSocket::readyRead,this,
                [=](){
                QByteArray data = rawSocket->readAll();
                term->recvData(data.data(), data.size());
            });
            connect(term, &QTermWidget::sendData,this,
                [=](const char *data, int size){
                if(rawSocket->state() == QAbstractSocket::ConnectedState) {
                    rawSocket->write(data, size);
                }
            });
            break;
        }
    }
}

SessionsWindow::~SessionsWindow() {
    if(localShell) {
        localShell->kill();
        delete localShell;
    }
    if(telnet) {
        if(telnet->isConnected()) telnet->disconnectFromHost();
        delete telnet;
    }
    if(serialPort) {
        if(serialPort->isOpen()) serialPort->close();
        delete serialPort;
    }
    if(rawSocket){
        if(rawSocket->state() == QAbstractSocket::ConnectedState) rawSocket->disconnectFromHost();
        delete rawSocket;
    }
}

int SessionsWindow::startLocalShellSession(const QString &command) {
    QString shellPath;
    if(command.isEmpty()) {
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        shellPath = qEnvironmentVariable("SHELL");
        if(shellPath.isEmpty()) shellPath = "/bin/sh";
    #elif defined(Q_OS_WIN)
        //shellPath = "c:\\Windows\\system32\\cmd.exe";
        shellPath = "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
    #endif
    } else {
        shellPath = command;
    }
    localShell->startProcess(shellPath, QProcessEnvironment::systemEnvironment().toStringList(), 87, 26);
    connect(localShell->notifier(), &QIODevice::readyRead, this, [=](){
        QByteArray data = localShell->readAll();
        term->recvData(data.data(), data.size());
    });
    connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
        localShell->write(QByteArray(data, size));
    });
    return 0;
}

int SessionsWindow::startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type) {
    telnet->setType(type);
    telnet->connectToHost(hostname, port);
    return 0;
}

int SessionsWindow::startSerialSession(const QString &portName, uint32_t baudRate
    , int dataBits, int parity, int stopBits, bool flowControl, bool xEnable ) {
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    switch (parity)
    {
    case 0:
    default:
        serialPort->setParity(QSerialPort::NoParity);
        break;
    case 1:
        serialPort->setParity(QSerialPort::OddParity);
        break;
    case 2:
        serialPort->setParity(QSerialPort::EvenParity);
        break;
    }
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));
    serialPort->setFlowControl(flowControl?QSerialPort::HardwareControl:QSerialPort::NoFlowControl);
    serialPort->setBreakEnabled(xEnable);
    serialPort->open(QIODevice::ReadWrite);
    return 0;
}

int SessionsWindow::startRawSocketSession(const QString &hostname, quint16 port) {
    rawSocket->connectToHost(hostname, port);
    return 0;
}
