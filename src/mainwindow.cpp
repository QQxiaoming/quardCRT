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
#include <QTabBar>

#include "qfonticon.h"

#include "sessiontab.h"
#include "sessionswindow.h"
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

    SessionTab *sessionTab = new SessionTab(this);
    sessionTab->setTabsClosable(true);
    splitter->addWidget(sessionTab);

    QuickConnectWindow *quickConnectWindow = new QuickConnectWindow(this);

    connect(quickConnectAction,&QAction::triggered,this,[=](){
        quickConnectWindow->show();
    });
    connect(quickConnectWindow,&QuickConnectWindow::sendQuickConnectData,this,
            [=](QuickConnectWindow::QuickConnectData data){
        if(data.type == QuickConnectWindow::Telnet) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
            sessionTab->addTab(sessionsWindow->getTermWidget(), "Telnet - "+data.TelnetData.hostname+":"+QString::number(data.TelnetData.port));
            QTelnet::SocketType type = QTelnet::TCP;
            if(data.TelnetData.webSocket == "None") {
                type = QTelnet::TCP;
            } else if(data.TelnetData.webSocket == "Insecure") {
                type = QTelnet::WEBSOCKET;
            } else if(data.TelnetData.webSocket == "Secure") {
                type = QTelnet::SECUREWEBSOCKET;
            }
            sessionsWindow->startTelnetSession(data.TelnetData.hostname,data.TelnetData.port,type);
        } else if(data.type == QuickConnectWindow::Serial) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Serial,this);
            sessionTab->addTab(sessionsWindow->getTermWidget(), "Serial - "+data.SerialData.portName);
            sessionsWindow->startSerialSession(data.SerialData.portName,data.SerialData.baudRate);
        } else if(data.type == QuickConnectWindow::LocalShell) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
            if(data.LocalShellData.command.isEmpty()) {
                sessionTab->addTab(sessionsWindow->getTermWidget(), "Local Shell");
            } else {
                sessionTab->addTab(sessionsWindow->getTermWidget(), "Local Shell - "+data.LocalShellData.command);
            }
            sessionsWindow->startLocalShellSession(data.LocalShellData.command);
        } else if(data.type == QuickConnectWindow::Raw) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
            sessionTab->addTab(sessionsWindow->getTermWidget(), "Raw - "+data.RawData.hostname+":"+QString::number(data.RawData.port));
            sessionsWindow->startRawSocketSession(data.RawData.hostname,data.RawData.port);
        }
        sessionTab->setCurrentIndex(sessionTab->count()-1);
    });
    connect(sessionTab,&QTabWidget::tabCloseRequested,this,[=](int index){
        SessionsWindow *sessionsWindow = (SessionsWindow *)sessionTab->widget(index);
        delete sessionsWindow;
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
