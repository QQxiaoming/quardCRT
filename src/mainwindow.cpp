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

    QMenu *fileMenu = new QMenu(tr("File"),this);
    ui->menuBar->addMenu(fileMenu);
    QMenu *editMenu = new QMenu(tr("Edit"),this);
    ui->menuBar->addMenu(editMenu);
    QMenu *viewMenu = new QMenu(tr("View"),this);
    ui->menuBar->addMenu(viewMenu);
    QMenu *optionsMenu = new QMenu(tr("Options"),this);
    ui->menuBar->addMenu(optionsMenu);
    QMenu *transferMenu = new QMenu(tr("Transfer"),this);
    ui->menuBar->addMenu(transferMenu);
    QMenu *ScriptMenu = new QMenu(tr("Script"),this);
    ui->menuBar->addMenu(ScriptMenu);
    QMenu *toolsMenu = new QMenu(tr("Tools"),this);
    ui->menuBar->addMenu(toolsMenu);
    QMenu *windowMenu = new QMenu(tr("Window"),this);
    ui->menuBar->addMenu(windowMenu);
    QMenu *helpMenu = new QMenu(tr("Help"),this);
    ui->menuBar->addMenu(helpMenu);

    QAction *connectAction = new QAction(tr("Connect..."),this);
    fileMenu->addAction(connectAction);
    QAction *sessionManagerAction = new QAction(QFontIcon::icon(QChar(0xf015)),tr("Session Manager"),this);
    ui->toolBar->addAction(sessionManagerAction);
    QAction *quickConnectAction = new QAction(QFontIcon::icon(QChar(0xf074)),tr("Quick Connect..."),this);
    fileMenu->addAction(quickConnectAction);
    ui->toolBar->addAction(quickConnectAction);
    QAction *connectInTabAction = new QAction(tr("Connect in Tab/Tile..."),this);
    fileMenu->addAction(connectInTabAction);
    QAction *connectLocalShellAction = new QAction(QFontIcon::icon(QChar(0xf120)),tr("Connect Local Shell"),this);
    fileMenu->addAction(connectLocalShellAction);
    ui->toolBar->addAction(connectLocalShellAction);
    ui->toolBar->addSeparator();
    fileMenu->addSeparator();
    QAction *reconnectAction = new QAction(QFontIcon::icon(QChar(0xf021)),tr("Reconnect"),this);
    fileMenu->addAction(reconnectAction);
    QAction *reconnectAllAction = new QAction(tr("Reconnect All"),this);
    fileMenu->addAction(reconnectAllAction);
    QAction *disconnectAction = new QAction(tr("Disconnect"),this);
    fileMenu->addAction(disconnectAction);
    QAction *disconnectAllAction = new QAction(tr("Disconnect All"),this);
    fileMenu->addAction(disconnectAllAction);
    fileMenu->addSeparator();
    QAction *cloneSessionAction = new QAction(tr("Clone Session"),this);
    fileMenu->addAction(cloneSessionAction);
    fileMenu->addSeparator();
    QAction *lockSessionAction = new QAction(QFontIcon::icon(QChar(0xf023)),tr("Lock Session"),this);
    fileMenu->addAction(lockSessionAction);
    fileMenu->addSeparator();
    QAction *exitAction = new QAction(tr("Exit"),this);
    fileMenu->addAction(exitAction);

    QAction *copyAction = new QAction(QFontIcon::icon(QChar(0xf0c5)),tr("Copy"),this);
    editMenu->addAction(copyAction);
    QAction *pasteAction = new QAction(QFontIcon::icon(QChar(0xf0ea)),tr("Paste"),this);
    editMenu->addAction(pasteAction);
    QAction *selectAllAction = new QAction(tr("Select All"),this);
    editMenu->addAction(selectAllAction);
    QAction *findAction = new QAction(QFontIcon::icon(QChar(0xf002)),tr("Find..."),this);
    editMenu->addAction(findAction);
    editMenu->addSeparator();
    QAction *resetAction = new QAction(tr("Reset"),this);
    editMenu->addAction(resetAction);

    QAction *helpAction = new QAction(QFontIcon::icon(QChar(0xf128)),tr("Help"),this);
    helpMenu->addAction(helpAction);
    ui->toolBar->addAction(helpAction);
    helpMenu->addSeparator();
    QAction *aboutAction = new QAction(tr("About"),this);
    helpMenu->addAction(aboutAction);
    QAction *aboutQtAction = new QAction(tr("About Qt"),this);
    helpMenu->addAction(aboutQtAction);

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
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
        sessionTab->addTab(sessionsWindow->getTermWidget(), "Local Shell");
        sessionsWindow->startLocalShellSession("");
        sessionTab->setCurrentIndex(sessionTab->count()-1);
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
            sessionsWindow->startSerialSession(
                data.SerialData.portName, data.SerialData.baudRate,
                data.SerialData.dataBits, data.SerialData.parity,
                data.SerialData.stopBits, data.SerialData.flowControl,
                data.SerialData.xEnable);
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
    connect(sessionTab,&SessionTab::showContextMenu,this,[=](int index){
        QMenu *menu = new QMenu(this);
        QAction *closeAction = new QAction("Close",this);
        closeAction->setIcon(QFontIcon::icon(QChar(0xf00d)));
        menu->addAction(closeAction);
        connect(closeAction,&QAction::triggered,this,[=](){
            SessionsWindow *sessionsWindow = (SessionsWindow *)sessionTab->widget(index);
            delete sessionsWindow;
        });
        menu->move(cursor().pos());
        menu->show();
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
