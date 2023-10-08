#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>
#include <QProcessEnvironment>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

#include "sessionswindow.h"
#include "argv_split.h"

SessionsWindow::SessionsWindow(SessionType tp, QObject *parent)
    : QObject(parent)
    , type(tp)
    , workingDirectory(QDir::homePath())
    , showShortTitle(false)
    , term(nullptr)
    , telnet(nullptr)
    , serialPort(nullptr)
    , rawSocket(nullptr)
    , localShell(nullptr)
    , enableLog(false)
    , enableRawLog(false) {

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
    term->setMargin(0);

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
            connect(term, &QTermWidget::termSizeChange, this, [=](int lines, int columns){
                localShell->resize(columns,lines);
            });
            break;
        }
        case Telnet: {
            telnet = new QTelnet(QTelnet::TCP, this);
            connect(telnet,&QTelnet::newData,this,
                [=](const char *data, int size){
                term->recvData(data, size);
                saveRawLog(data, size);
                emit hexDataDup(data, size);
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
                saveRawLog(data.data(), data.size());
                emit hexDataDup(data.data(), data.size());
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
                saveRawLog(data.data(), data.size());
                emit hexDataDup(data.data(), data.size());
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

    connect(term, &QTermWidget::dupDisplayOutput, this, [=](const char *data, int size){
        saveLog(data, size);
    });
    connect(term, &QTermWidget::urlActivated, this, [=](const QUrl& url, bool fromContextMenu){
        QDesktopServices::openUrl(url);
    });
}

SessionsWindow::~SessionsWindow() {
    enableLog = false;
    enableRawLog = false;
    log_file_mutex.lock();
    if(log_file != nullptr) {
        log_file->close();
        delete log_file;
        log_file = nullptr;
    }
    log_file_mutex.unlock();
    raw_log_file_mutex.lock();
    if(raw_log_file != nullptr) {
        raw_log_file->close();
        delete raw_log_file;
        raw_log_file = nullptr;
    }
    raw_log_file_mutex.unlock();
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

void SessionsWindow::cloneSession(SessionsWindow *src) {
    switch(src->getSessionType()) {
        case LocalShell: {
            setWorkingDirectory(src->getWorkingDirectory());
            startLocalShellSession(src->m_command);
            break;
        case Telnet:
            startTelnetSession(src->m_hostname, src->m_port, src->m_type);
            break;
        case Serial:
            startSerialSession(src->m_portName, src->m_baudRate, src->m_dataBits, src->m_parity, src->m_stopBits, src->m_flowControl, src->m_xEnable);
            break;
        case RawSocket:
            startRawSocketSession(src->m_hostname, src->m_port);
            break;
        }
    }  
}

int SessionsWindow::startLocalShellSession(const QString &command) {
    QString shellPath;
    QStringList args;
    if(command.isEmpty()) {
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        shellPath = qEnvironmentVariable("SHELL");
        if(shellPath.isEmpty()) shellPath = "/bin/sh";
    #elif defined(Q_OS_WIN)
        shellPath = "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
        args =  {
            "powershell.exe",
            "-ExecutionPolicy",
            "Bypass",
            "-NoLogo",
            "-NoProfile",
            "-NoExit",
            "-File",
            "\"" + QApplication::applicationDirPath() + "/Profile.ps1\""
        };
    #endif
    } else {
        argv_split parser;
        parser.parse(command.toStdString());
        for(auto &it : parser.getArguments()) {
            args.append(QString::fromStdString(it));
        }
        shellPath = args.first();
        args.removeFirst();
    #if defined(Q_OS_WIN)
        QFileInfo fi(shellPath);
        args.insert(0, fi.fileName());
    #endif
    }
    bool ret = localShell->startProcess(shellPath, args, QProcessEnvironment::systemEnvironment().toStringList(), workingDirectory, term->screenColumnsCount(), term->screenLinesCount());
    if(!ret) {
        QMessageBox::warning(term, tr("Start Local Shell"), tr("Cannot start local shell:\n%1.").arg(localShell->lastError()));
        return -1;
    }
    connect(localShell->notifier(), &QIODevice::readyRead, this, [=](){
        QByteArray data = localShell->readAll();
        term->recvData(data.data(), data.size());
        saveRawLog(data.data(), data.size());
        emit hexDataDup(data.data(), data.size());
    });
    connect(term, &QTermWidget::sendData, this, [=](const char *data, int size){
        localShell->write(QByteArray(data, size));
    });
    m_command = command;
    return 0;
}

int SessionsWindow::startTelnetSession(const QString &hostname, quint16 port, QTelnet::SocketType type) {
    telnet->setType(type);
    telnet->connectToHost(hostname, port);
    m_hostname = hostname;
    m_port = port;
    m_type = type;
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
    serialPort->open(QIODevice::ReadWrite);
    serialPort->setBreakEnabled(xEnable);
    m_portName = portName;
    m_baudRate = baudRate;
    m_dataBits = dataBits;
    m_parity = parity;
    m_stopBits = stopBits;
    m_flowControl = flowControl;
    m_xEnable = xEnable;
    return 0;
}

int SessionsWindow::startRawSocketSession(const QString &hostname, quint16 port) {
    rawSocket->connectToHost(hostname, port);
    m_hostname = hostname;
    m_port = port;
    return 0;
}

void SessionsWindow::setWorkingDirectory(const QString &dir)
{
    workingDirectory = dir;
}

int SessionsWindow::setLog(bool enable) {
    int ret = -1;
    log_file_mutex.lock(); 
    if(enable) {
        if(log_file == nullptr) {
            QString savefile_name = QFileDialog::getSaveFileName(term, tr("Save log..."),
                QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".log", tr("log files (*.log)"));
            if (!savefile_name.isEmpty()) {
                log_file = new QFile(savefile_name);
                if (!log_file->open(QIODevice::WriteOnly|QIODevice::Text)) {
                    QMessageBox::warning(term, tr("Save log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
                    delete log_file;
                    log_file = nullptr;
                    enableLog = false;
                } else {
                    enableLog = true;
                    ret = 0;
                }
            } else {
                enableLog = false;
            }
        }
    } else {
        if(log_file != nullptr) {
            log_file->close();
            delete log_file;
            log_file = nullptr;
        }
        enableLog = false;
        ret = 0;
    }
    log_file_mutex.unlock();
    return ret;
}

int SessionsWindow::setRawLog(bool enable) {
    int ret = -1;
    raw_log_file_mutex.lock(); 
    if(enable) {
        if(raw_log_file == nullptr) {
            QString savefile_name = QFileDialog::getSaveFileName(term, tr("Save Raw log..."),
                QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".bin", tr("binary files (*.bin)"));
            if (!savefile_name.isEmpty()) {
                raw_log_file = new QFile(savefile_name);
                if (!raw_log_file->open(QIODevice::WriteOnly)) {
                    QMessageBox::warning(term, tr("Save Raw log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
                    delete raw_log_file;
                    raw_log_file = nullptr;
                    enableRawLog = false;
                } else {
                    enableRawLog = true;
                    ret = 0;
                }
            } else {
                enableRawLog = false;
            }
        }
    } else {
        if(raw_log_file != nullptr) {
            raw_log_file->close();
            delete raw_log_file;
            raw_log_file = nullptr;
        }
        enableRawLog = false;
        ret = 0;
    }
    raw_log_file_mutex.unlock();
    return ret;
}

int SessionsWindow::saveLog(const char *data, int size) {
    int ret = 0;
    if(enableLog) {
        if(log_file_mutex.tryLock()) {
            if(log_file != nullptr) {
                ret = log_file->write(data, size);
                if(fflush_file) {
                    log_file->flush();
                }
            }
            log_file_mutex.unlock();
        }
    }
    return ret;
}

int SessionsWindow::saveRawLog(const char *data, int size) {
    int ret = 0;
    if(enableRawLog) {
        if(raw_log_file_mutex.tryLock()) {
            if(raw_log_file != nullptr) {
                ret = raw_log_file->write(data, size);
                if(fflush_file) {
                    raw_log_file->flush();
                }
            }
            raw_log_file_mutex.unlock();
        }
    }
    return ret;
}
