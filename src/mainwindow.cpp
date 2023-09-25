#include <QLibraryInfo>
#include <QTranslator>
#include <QFontDatabase>
#include <QSplitter>
#include <QLabel>
#include <QToolBar>
#include <QSerialPort>
#include <QTcpSocket>
#include <QProcess>
#include <QMessageBox>
#include <QSocketNotifier>

#include "qtermwidget.h"
#include "qfonticon.h"
#include "QTelnet.h"
#include "ptyqt.h"

#include "quickconnectwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

    ui->setupUi(this);

    ui->toolBar->setIconSize(QSize(16,16));
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    QMenu *fileMenu = new QMenu("File",this);
    ui->menuBar->addMenu(fileMenu);
    QAction *connectAction = new QAction("Connect...",this);
    fileMenu->addAction(connectAction);
    QAction *sessionManagerAction = new QAction(QFontIcon::icon(QChar(0xf015)),"Session Manager",this);
    ui->toolBar->addAction(sessionManagerAction);
    QAction *quickConnectAction = new QAction(QFontIcon::icon(QChar(0xf074)),"Quick Connect...",this);
    fileMenu->addAction(quickConnectAction);
    ui->toolBar->addAction(quickConnectAction);


    QSplitter *splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    QWidget *label = new QWidget(this);
    label->setFixedWidth(20);
    splitter->addWidget(label);

    QTabWidget *tabWidget = new QTabWidget(this);
    splitter->addWidget(tabWidget);

    QTermWidget *termWidget = new QTermWidget(0,this);
    tabWidget->addTab(termWidget, "empty");

    QFont font = QApplication::font();
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    font.setPointSize(12);

    termWidget->setTerminalFont(font);
    termWidget->setScrollBarPosition(QTermWidget::NoScrollBar);

    QStringList availableColorSchemes = termWidget->availableColorSchemes();
    availableColorSchemes.sort();
    QString currentColorScheme = availableColorSchemes.first();
    foreach(QString colorScheme, availableColorSchemes) {
        if(colorScheme == "WhiteOnBlack") {
            termWidget->setColorScheme("WhiteOnBlack");
            currentColorScheme = "WhiteOnBlack";
        }
    }

    QStringList availableKeyBindings = termWidget->availableKeyBindings();
    availableKeyBindings.sort();
    QString currentAvailableKeyBindings = availableKeyBindings.first();
    foreach(QString keyBinding, availableKeyBindings) {
        if(keyBinding == "linux") {
            termWidget->setKeyBindings("linux");
            currentAvailableKeyBindings = "linux";
        }
    }
    termWidget->startTerminalTeletype();

    QuickConnectWindow *quickConnectWindow = new QuickConnectWindow(this);
    QTelnet *telnet = new QTelnet(QTelnet::TCP, this);
    QSerialPort *serialPort = new QSerialPort(this);
    QTcpSocket *rawSocket = new QTcpSocket(this);

    connect(quickConnectAction,&QAction::triggered,this,[=](){
        quickConnectWindow->show();
    });
    connect(quickConnectWindow,&QuickConnectWindow::sendQuickConnectData,this,
            [=](QuickConnectWindow::QuickConnectData data){
        termWidget->clear();
        if(telnet->isConnected()){
            telnet->disconnectFromHost();
        }
        if(serialPort->isOpen()) {
            serialPort->close();
        }
        if(rawSocket->state() == QAbstractSocket::ConnectedState) {
            rawSocket->disconnectFromHost();
        }
        if(data.type == QuickConnectWindow::Telnet) {
            tabWidget->setTabText(0, "Telnet - "+data.TelnetData.hostname+":"+QString::number(data.TelnetData.port));
            if(data.TelnetData.webSocket == "None") {
                telnet->setType(QTelnet::TCP);
            } else if(data.TelnetData.webSocket == "Insecure") {
                telnet->setType(QTelnet::WEBSOCKET);
            } else if(data.TelnetData.webSocket == "Secure") {
                telnet->setType(QTelnet::SECUREWEBSOCKET);
            }
            telnet->connectToHost(data.TelnetData.hostname,data.TelnetData.port);
        } else if(data.type == QuickConnectWindow::Serial) {
            tabWidget->setTabText(0, "Serial - "+data.SerialData.portName);
            serialPort->setPortName(data.SerialData.portName);
            serialPort->setBaudRate(data.SerialData.baudRate);
            serialPort->setDataBits(QSerialPort::Data8);
            serialPort->setParity(QSerialPort::NoParity);
            serialPort->setStopBits(QSerialPort::OneStop);
            serialPort->setFlowControl(QSerialPort::NoFlowControl);
            serialPort->open(QIODevice::ReadWrite);
        } else if(data.type == QuickConnectWindow::LocalShell) {
            if(localShell){
                localShell->kill();
                disconnect(localShell->notifier(), &QIODevice::readyRead, this, nullptr);
                disconnect(termWidget, &QTermWidget::sendData,this,nullptr);
                delete localShell;
            }
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            IPtyProcess::PtyType ptyType = IPtyProcess::UnixPty;
            QString shellPath = qEnvironmentVariable("SHELL");
            if(shellPath.isEmpty()) shellPath = "/bin/sh";
        #elif defined(Q_OS_WIN)
            IPtyProcess::PtyType ptyType = IPtyProcess::ConPty;
            QString shellPath = "c:\\Windows\\system32\\cmd.exe";
        #endif
            if(data.LocalShellData.command.isEmpty()) {
                data.LocalShellData.command = shellPath;
            }
            tabWidget->setTabText(0, "Local Shell - "+data.LocalShellData.command);
            IPtyProcess *localShell = PtyQt::createPtyProcess(ptyType);
            localShell->startProcess(data.LocalShellData.command, QProcessEnvironment::systemEnvironment().toStringList(), 87, 26);
            connect(localShell->notifier(), &QIODevice::readyRead, this, [=](){
                QByteArray data = localShell->readAll();
                termWidget->recvData(data.data(), data.size());
            });
            connect(termWidget, &QTermWidget::sendData, this, [=](const char *data, int size){
                localShell->write(QByteArray(data, size));
            });
        } else if(data.type == QuickConnectWindow::Raw) {
            tabWidget->setTabText(0, "Raw - "+data.RawData.hostname+":"+QString::number(data.RawData.port));
            rawSocket->connectToHost(data.RawData.hostname,data.RawData.port);
        }
    });
    connect(telnet,&QTelnet::newData,this,
        [=](const char *data, int size){
        termWidget->recvData(data, size);
    });
    connect(termWidget, &QTermWidget::sendData,this,
        [=](const char *data, int size){
        if(telnet->isConnected())
            telnet->sendData(data, size);
    });
    connect(serialPort,&QSerialPort::readyRead,this,
        [=](){
        QByteArray data = serialPort->readAll();
        termWidget->recvData(data.data(), data.size());
    });
    connect(termWidget, &QTermWidget::sendData,this,
        [=](const char *data, int size){
        if(serialPort->isOpen()) {
            serialPort->write(data, size);
        }
    });
    connect(rawSocket,&QTcpSocket::readyRead,this,
        [=](){
        QByteArray data = rawSocket->readAll();
        termWidget->recvData(data.data(), data.size());
    });
    connect(termWidget, &QTermWidget::sendData,this,
        [=](const char *data, int size){
        if(rawSocket->state() == QAbstractSocket::ConnectedState) {
            rawSocket->write(data, size);
        }
    });

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setAppLangeuage(QLocale::Language lang) {
    static QTranslator *qtTranslator = nullptr;
    static QTranslator *qtbaseTranslator = nullptr;
    static QTranslator *appTranslator = nullptr;
    QString qlibpath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if(qtTranslator == nullptr) {
        qtTranslator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(qtTranslator);
        delete qtTranslator;
        qtTranslator = new QTranslator(qApp);
    }
    if(qtbaseTranslator == nullptr) {
        qtbaseTranslator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(qtbaseTranslator);
        delete qtbaseTranslator;
        qtbaseTranslator = new QTranslator(qApp);
    }
    if(appTranslator == nullptr) {
        appTranslator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(appTranslator);
        delete appTranslator;
        appTranslator = new QTranslator(qApp);
    }
    switch(lang) {
    case QLocale::Chinese:
        if(qtTranslator->load("qt_zh_CN.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_zh_CN.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_zh_CN.qm"))
            qApp->installTranslator(appTranslator);
        break;
    case QLocale::Japanese:
        if(qtTranslator->load("qt_ja.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_ja.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_ja_JP.qm"))
            qApp->installTranslator(appTranslator);
        break;
    default:
    case QLocale::English:
        if(qtTranslator->load("qt_en.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_en.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_en_US.qm"))
            qApp->installTranslator(appTranslator);
        break;
    }
    QTermWidget::setLangeuage(lang);
}
