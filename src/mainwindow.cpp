#include <QLibraryInfo>
#include <QTranslator>
#include <QFontDatabase>
#include <QSplitter>
#include <QLabel>
#include <QToolBar>
#include <QLineEdit>
#include <QActionGroup>
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

MainWindow::MainWindow(QLocale::Language lang, QWidget *parent)
    : QMainWindow(parent) 
    , ui(new Ui::MainWindow)
    , language(lang) {

    ui->setupUi(this);

    splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    QWidget *label = new QWidget(this);
    label->setFixedWidth(20);
    splitter->addWidget(label);

    sessionTab = new SessionTab(this);
    sessionTab->setTabsClosable(true);
    splitter->addWidget(sessionTab);

    quickConnectWindow = new QuickConnectWindow(this);

    menuAndToolBarInit();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::menuAndToolBarRetranslateUi(void) {
    fileMenu->setTitle(tr("File"));
    editMenu->setTitle(tr("Edit"));
    viewMenu->setTitle(tr("View"));
    optionsMenu->setTitle(tr("Options"));
    transferMenu->setTitle(tr("Transfer"));
    scriptMenu->setTitle(tr("Script"));
    toolsMenu->setTitle(tr("Tools"));
    windowMenu->setTitle(tr("Window"));
    languageMenu->setTitle(tr("Language"));
    helpMenu->setTitle(tr("Help"));

    connectAction->setText(tr("Connect..."));
    sessionManagerAction->setText(tr("Session Manager"));
    quickConnectAction->setText(tr("Quick Connect..."));
    connectInTabAction->setText(tr("Connect in Tab/Tile..."));
    connectLocalShellAction->setText(tr("Connect Local Shell"));
    reconnectAction->setText(tr("Reconnect"));
    reconnectAllAction->setText(tr("Reconnect All"));
    disconnectAction->setText(tr("Disconnect"));
    connectAddressEdit->setPlaceholderText(tr("Enter host <Alt+R> to connect"));
    disconnectAllAction->setText(tr("Disconnect All"));
    cloneSessionAction->setText(tr("Clone Session"));
    lockSessionAction->setText(tr("Lock Session"));
    exitAction->setText(tr("Exit"));

    copyAction->setText(tr("Copy"));
    pasteAction->setText(tr("Paste"));
    selectAllAction->setText(tr("Select All"));
    findAction->setText(tr("Find..."));
    printScreenAction->setText(tr("Print Screen"));
    resetAction->setText(tr("Reset"));

    zoomInAction->setText(tr("Zoom In"));
    zoomOutAction->setText(tr("Zoom Out"));
    fullScreenAction->setText(tr("Full Screen"));

    sessionOptionsAction->setText(tr("Session Options..."));
    globalOptionsAction->setText(tr("Global Options..."));
    autoSaveOptionsAction->setText(tr("Auto Save Options"));
    saveSettingsNowAction->setText(tr("Save Settings Now"));

    sendASCIIAction->setText(tr("Send ASCII..."));
    receiveASCIIAction->setText(tr("Receive ASCII..."));
    sendBinaryAction->setText(tr("Send Binary..."));
    sendXmodemAction->setText(tr("Send Xmodem..."));
    receiveXmodemAction->setText(tr("Receive Xmodem..."));
    sendYmodemAction->setText(tr("Send Ymodem..."));
    receiveYmodemAction->setText(tr("Receive Ymodem..."));
    zmodemUploadListAction->setText(tr("Zmodem Upload List..."));
    startZmodemUploadAction->setText(tr("Start Zmodem Upload"));
    startTFTPServerAction->setText(tr("Start TFTP Server"));

    runAction->setText(tr("Run..."));
    cancelAction->setText(tr("Cancel"));
    startRecordingScriptAction->setText(tr("Start Recording Script"));
    stopRecordingScriptAction->setText(tr("Stop Recording Script..."));
    canlcelRecordingScriptAction->setText(tr("Cancel Recording Script"));

    keymapManagerAction->setText(tr("Keymap Manager"));
    createPublicKeyAction->setText(tr("Create Public Key..."));
    publickeyManagerAction->setText(tr("Publickey Manager"));

    tabAction->setText(tr("Tab"));
    tileAction->setText(tr("Tile"));
    cascadeAction->setText(tr("Cascade"));

    chineseAction->setText(tr("Chinese"));
    englishAction->setText(tr("English"));
    japaneseAction->setText(tr("Japanese"));

    helpAction->setText(tr("Help"));
    aboutAction->setText(tr("About"));
    aboutQtAction->setText(tr("About Qt"));
}

void MainWindow::menuAndToolBarInit(void) {
    ui->toolBar->setIconSize(QSize(16,16));
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    fileMenu = new QMenu(this);
    ui->menuBar->addMenu(fileMenu);
    editMenu = new QMenu(this);
    ui->menuBar->addMenu(editMenu);
    viewMenu = new QMenu(this);
    ui->menuBar->addMenu(viewMenu);
    optionsMenu = new QMenu(this);
    ui->menuBar->addMenu(optionsMenu);
    transferMenu = new QMenu(this);
    ui->menuBar->addMenu(transferMenu);
    scriptMenu = new QMenu(this);
    ui->menuBar->addMenu(scriptMenu);
    toolsMenu = new QMenu(this);
    ui->menuBar->addMenu(toolsMenu);
    windowMenu = new QMenu(this);
    ui->menuBar->addMenu(windowMenu);
    languageMenu = new QMenu(this);
    ui->menuBar->addMenu(languageMenu);
    helpMenu = new QMenu(this);
    ui->menuBar->addMenu(helpMenu);

    connectAction = new QAction(QFontIcon::icon(QChar(0xf0c1))," ",this);
    fileMenu->addAction(connectAction);
    sessionManagerAction = new QAction(QFontIcon::icon(QChar(0xf0e8))," ",this);
    ui->toolBar->addAction(sessionManagerAction);
    quickConnectAction = new QAction(QFontIcon::icon(QChar(0xf0e7))," ",this);
    fileMenu->addAction(quickConnectAction);
    ui->toolBar->addAction(quickConnectAction);
    connectInTabAction = new QAction(this);
    fileMenu->addAction(connectInTabAction);
    connectLocalShellAction = new QAction(QFontIcon::icon(QChar(0xf120))," ",this);
    fileMenu->addAction(connectLocalShellAction);
    ui->toolBar->addAction(connectLocalShellAction);
    fileMenu->addSeparator();
    reconnectAction = new QAction(QFontIcon::icon(QChar(0xf021))," ",this);
    fileMenu->addAction(reconnectAction);
    ui->toolBar->addAction(reconnectAction);
    reconnectAllAction = new QAction(this);
    fileMenu->addAction(reconnectAllAction);
    disconnectAction = new QAction(QFontIcon::icon(QChar(0xf127))," ",this);
    fileMenu->addAction(disconnectAction);
    ui->toolBar->addAction(disconnectAction);
    connectAddressEdit = new QLineEdit(this);
    connectAddressEdit->setFixedWidth(180);
    ui->toolBar->addWidget(connectAddressEdit);
    ui->toolBar->addSeparator();
    disconnectAllAction = new QAction(this);
    fileMenu->addAction(disconnectAllAction);
    fileMenu->addSeparator();
    cloneSessionAction = new QAction(this);
    fileMenu->addAction(cloneSessionAction);
    fileMenu->addSeparator();
    lockSessionAction = new QAction(QFontIcon::icon(QChar(0xf023))," ",this);
    fileMenu->addAction(lockSessionAction);
    fileMenu->addSeparator();
    exitAction = new QAction(this);
    fileMenu->addAction(exitAction);

    copyAction = new QAction(QFontIcon::icon(QChar(0xf0c5))," ",this);
    editMenu->addAction(copyAction);
    ui->toolBar->addAction(copyAction);
    pasteAction = new QAction(QFontIcon::icon(QChar(0xf0ea))," ",this);
    editMenu->addAction(pasteAction);
    ui->toolBar->addAction(pasteAction);
    selectAllAction = new QAction(this);
    editMenu->addAction(selectAllAction);
    findAction = new QAction(QFontIcon::icon(QChar(0xf002))," ",this);
    editMenu->addAction(findAction);
    editMenu->addSeparator();
    ui->toolBar->addAction(findAction);
    ui->toolBar->addSeparator();
    printScreenAction = new QAction(QFontIcon::icon(QChar(0xf02f))," ",this);
    editMenu->addAction(printScreenAction);
    editMenu->addSeparator();
    ui->toolBar->addAction(printScreenAction);
    ui->toolBar->addSeparator();
    resetAction = new QAction(this);
    editMenu->addAction(resetAction);

    zoomInAction = new QAction(QFontIcon::icon(QChar(0xf00e))," ",this);
    viewMenu->addAction(zoomInAction);
    zoomOutAction = new QAction(QFontIcon::icon(QChar(0xf010))," ",this);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addSeparator();
    fullScreenAction = new QAction(this);
    fullScreenAction->setCheckable(true);
    viewMenu->addAction(fullScreenAction);

    sessionOptionsAction = new QAction(QFontIcon::icon(QChar(0xf1de)),tr("Session Options..."),this);
    optionsMenu->addAction(sessionOptionsAction);
    ui->toolBar->addAction(sessionOptionsAction);
    globalOptionsAction = new QAction(QFontIcon::icon(QChar(0xf013)),tr("Global Options..."),this);
    optionsMenu->addAction(globalOptionsAction);
    optionsMenu->addSeparator();
    ui->toolBar->addAction(globalOptionsAction);
    ui->toolBar->addSeparator();
    autoSaveOptionsAction = new QAction(this);
    autoSaveOptionsAction->setCheckable(true);
    optionsMenu->addAction(autoSaveOptionsAction);
    saveSettingsNowAction = new QAction(this);
    optionsMenu->addAction(saveSettingsNowAction);

    sendASCIIAction = new QAction(this);
    transferMenu->addAction(sendASCIIAction);
    receiveASCIIAction = new QAction(this);
    receiveASCIIAction->setCheckable(true);
    transferMenu->addAction(receiveASCIIAction);
    transferMenu->addSeparator();
    sendBinaryAction = new QAction(this);
    transferMenu->addAction(sendBinaryAction);
    transferMenu->addSeparator();
    sendXmodemAction = new QAction(this);
    transferMenu->addAction(sendXmodemAction);
    receiveXmodemAction = new QAction(this);
    transferMenu->addAction(receiveXmodemAction);
    transferMenu->addSeparator();
    sendYmodemAction = new QAction(this);
    transferMenu->addAction(sendYmodemAction);
    receiveYmodemAction = new QAction(this);
    transferMenu->addAction(receiveYmodemAction);
    transferMenu->addSeparator();
    zmodemUploadListAction = new QAction(this);
    transferMenu->addAction(zmodemUploadListAction);
    startZmodemUploadAction = new QAction(this);
    transferMenu->addAction(startZmodemUploadAction);
    transferMenu->addSeparator();
    startTFTPServerAction = new QAction(this);
    transferMenu->addAction(startTFTPServerAction);

    runAction = new QAction(this);
    scriptMenu->addAction(runAction);
    cancelAction = new QAction(this);
    scriptMenu->addAction(cancelAction);
    scriptMenu->addSeparator();
    startRecordingScriptAction = new QAction(this);
    scriptMenu->addAction(startRecordingScriptAction);
    stopRecordingScriptAction = new QAction(this);
    scriptMenu->addAction(stopRecordingScriptAction);
    canlcelRecordingScriptAction = new QAction(this);
    scriptMenu->addAction(canlcelRecordingScriptAction);

    keymapManagerAction = new QAction(this);
    toolsMenu->addAction(keymapManagerAction);
    toolsMenu->addSeparator();
    createPublicKeyAction = new QAction(this);
    toolsMenu->addAction(createPublicKeyAction);
    publickeyManagerAction = new QAction(this);
    toolsMenu->addAction(publickeyManagerAction);

    windowActionGroup = new QActionGroup(this);
    tabAction = new QAction(this);
    tabAction->setActionGroup(windowActionGroup);
    tabAction->setCheckable(true);
    tabAction->setChecked(true);
    windowMenu->addAction(tabAction);
    tileAction = new QAction(this);
    tileAction->setActionGroup(windowActionGroup);
    tileAction->setCheckable(true);
    windowMenu->addAction(tileAction);
    cascadeAction = new QAction(this);
    cascadeAction->setActionGroup(windowActionGroup);
    cascadeAction->setCheckable(true);
    windowMenu->addAction(cascadeAction);

    languageActionGroup = new QActionGroup(this);
    chineseAction = new QAction(this);
    chineseAction->setActionGroup(languageActionGroup);
    chineseAction->setCheckable(true);
    chineseAction->setChecked(language == QLocale::Chinese);
    languageMenu->addAction(chineseAction);
    englishAction = new QAction(this);
    englishAction->setActionGroup(languageActionGroup);
    englishAction->setCheckable(true);
    englishAction->setChecked(language == QLocale::English);
    languageMenu->addAction(englishAction);
    japaneseAction = new QAction(this);
    japaneseAction->setActionGroup(languageActionGroup);
    japaneseAction->setCheckable(true);
    japaneseAction->setChecked(language == QLocale::Japanese);
    languageMenu->addAction(japaneseAction);

    helpAction = new QAction(QFontIcon::icon(QChar(0xf128))," ",this);
    helpMenu->addAction(helpAction);
    ui->toolBar->addAction(helpAction);
    helpMenu->addSeparator();
    aboutAction = new QAction(this);
    helpMenu->addAction(aboutAction);
    aboutQtAction = new QAction(this);
    helpMenu->addAction(aboutQtAction);

    connect(quickConnectAction,&QAction::triggered,this,[=](){
        quickConnectWindow->show();
    });
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
        sessionTab->addTab(sessionsWindow->getTermWidget(), tr("Local Shell"));
        sessionsWindow->startLocalShellSession("");
        sessionTab->setCurrentIndex(sessionTab->count()-1);
    });
    connect(quickConnectWindow,&QuickConnectWindow::sendQuickConnectData,this,
            [=](QuickConnectWindow::QuickConnectData data){
        if(data.type == QuickConnectWindow::Telnet) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
            sessionTab->addTab(sessionsWindow->getTermWidget(), tr("Telnet - ")+data.TelnetData.hostname+":"+QString::number(data.TelnetData.port));
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
            sessionTab->addTab(sessionsWindow->getTermWidget(), tr("Serial - ")+data.SerialData.portName);
            sessionsWindow->startSerialSession(
                data.SerialData.portName, data.SerialData.baudRate,
                data.SerialData.dataBits, data.SerialData.parity,
                data.SerialData.stopBits, data.SerialData.flowControl,
                data.SerialData.xEnable);
        } else if(data.type == QuickConnectWindow::LocalShell) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
            if(data.LocalShellData.command.isEmpty()) {
                sessionTab->addTab(sessionsWindow->getTermWidget(), tr("Local Shell"));
            } else {
                sessionTab->addTab(sessionsWindow->getTermWidget(), tr("Local Shell - ")+data.LocalShellData.command);
            }
            sessionsWindow->startLocalShellSession(data.LocalShellData.command);
        } else if(data.type == QuickConnectWindow::Raw) {
            SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
            sessionTab->addTab(sessionsWindow->getTermWidget(), tr("Raw - ")+data.RawData.hostname+":"+QString::number(data.RawData.port));
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
        QAction *closeAction = new QAction(QFontIcon::icon(QChar(0xf00d)),tr("Close"),this);
        menu->addAction(closeAction);
        connect(closeAction,&QAction::triggered,this,[=](){
            SessionsWindow *sessionsWindow = (SessionsWindow *)sessionTab->widget(index);
            delete sessionsWindow;
        });
        menu->move(cursor().pos());
        menu->show();
    });
    connect(languageActionGroup,&QActionGroup::triggered,this,[=](QAction *action){
        if(action == chineseAction) {
            this->language = QLocale::Chinese;
        } else if(action == englishAction) {
            this->language = QLocale::English;
        } else if(action == japaneseAction) {
            this->language = QLocale::Japanese;
        }
        setAppLangeuage(this->language);
        ui->retranslateUi(this);
        menuAndToolBarRetranslateUi();
    });
    connect(exitAction, &QAction::triggered, this, [&](){
        qApp->quit();
    });
    connect(helpAction, &QAction::triggered, this, [&]() {
            MainWindow::appHelp(this);
    });
    connect(aboutAction, &QAction::triggered, this, [&]() {
            MainWindow::appAbout(this);
    });
    connect(aboutQtAction, &QAction::triggered, this, [&]() {
            QMessageBox::aboutQt(this);
    });
    menuAndToolBarRetranslateUi();
}

void MainWindow::appAbout(QWidget *parent)
{
    QMessageBox::about(parent, tr("About"),
                       tr(
                           "<p>Version</p>"
                           "<p>&nbsp;%1</p>"
                           "<p>Commit</p>"
                           "<p>&nbsp;%2</p>"
                           "<p>Author</p>"
                           "<p>&nbsp;qiaoqm@aliyun.com</p>"
                           "<p>Website</p>"
                           "<p>&nbsp;<a href='https://github.com/QQxiaoming/quardCRT'>https://github.com/QQxiaoming</p>"
                           "<p>&nbsp;<a href='https://gitee.com/QQxiaoming/quardCRT'>https://gitee.com/QQxiaoming</a></p>"
                           ).arg(VERSION,GIT_TAG)
                       );
}

void MainWindow::appHelp(QWidget *parent)
{
    QMessageBox::about(parent, tr("Help"), "TODO"); 
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
