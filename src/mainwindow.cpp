#include <QLibraryInfo>
#include <QTranslator>
#include <QFontDatabase>
#include <QSplitter>
#include <QLabel>
#include <QToolBar>

#include "qtermwidget.h"
#include "qfonticon.h"
#include "QTelnet.h"

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
    tabWidget->addTab(termWidget, "local shell");

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

    QuickConnectWindow *quickConnectWindow = new QuickConnectWindow(this);
    QTelnet *telnet = new QTelnet(QTelnet::TCP, this);
    termWidget->startTerminalTeletype();
    
    connect(quickConnectAction,&QAction::triggered,this,[=](){
        quickConnectWindow->show();
    });
    connect(quickConnectWindow,&QuickConnectWindow::sendQuickConnectData,this,
        [=](QString hostname, int port, QString protocol, QString webSocket){
        if(protocol == "Telnet") {
            if(telnet->isConnected()){
                telnet->disconnectFromHost();
            }
            if(webSocket == "None") {
                telnet->setType(QTelnet::TCP);
            } else if(webSocket == "Insecure") {
                telnet->setType(QTelnet::WEBSOCKET);
            } else if(webSocket == "Secure") {
                telnet->setType(QTelnet::SECUREWEBSOCKET);
            }
            telnet->connectToHost(hostname,port);
        }
    });
    connect(telnet,&QTelnet::newData,this,
        [=](const char *data, int size){
        termWidget->recvData(data, size);
    });
    connect(termWidget, &QTermWidget::sendData,this,
        [=](const char *data, int size){
        telnet->sendData(data, size);
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
