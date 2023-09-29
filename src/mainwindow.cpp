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
#include <QShortcut>

#include "qfonticon.h"

#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptions.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QLocale::Language lang, bool isDark, QWidget *parent)
    : QMainWindow(parent) 
    , ui(new Ui::MainWindow)
    , language(lang)
    , isDarkTheme(isDark) {

    ui->setupUi(this);

    /* Create the main UI */
    splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);
    setCentralWidget(splitter);

    QWidget *label = new QWidget(this);
    label->setFixedWidth(20);
    splitter->addWidget(label);

    sessionTab = new SessionTab(this);
    splitter->addWidget(sessionTab);

    quickConnectWindow = new QuickConnectWindow(this);
    
    keyMapManagerWindow = new keyMapManager(this);
    keyMapManagerWindow->setAvailableKeyBindings(QTermWidget::availableKeyBindings());

    globalOptionsWindow = new GlobalOptions(this);
    globalOptionsWindow->setAvailableColorSchemes(QTermWidget::availableColorSchemes());

    hexViewWindow = new HexViewWindow(HexViewWindow::RECV,this);
    hexViewWindow->setFont(globalOptionsWindow->getCurrentFont());

    menuAndToolBarInit();

    newLocalShellShortCut = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_T), this);
    cloneSessionShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_T), this);

    /* connect signals */
    menuAndToolBarConnectSignals();
    
    connect(sessionTab,&QTabWidget::tabCloseRequested,this,[=](int index){
        stopSession(index);
    });
    connect(sessionTab,&SessionTab::currentChanged,this,[=](int index){
        if(sessionTab->count() == 0) {
            reconnectAction->setEnabled(false);
            reconnectAllAction->setEnabled(false);
            disconnectAction->setEnabled(false);
            disconnectAllAction->setEnabled(false);
            cloneSessionAction->setEnabled(false);
            lockSessionAction->setEnabled(false);
            logSessionAction->setEnabled(false);
            rawLogSessionAction->setEnabled(false);
        } else {
            reconnectAction->setEnabled(true);
            reconnectAllAction->setEnabled(true);
            disconnectAction->setEnabled(true);
            disconnectAllAction->setEnabled(true);
            cloneSessionAction->setEnabled(true);
            lockSessionAction->setEnabled(true);
            logSessionAction->setEnabled(true);
            rawLogSessionAction->setEnabled(true);
        }
        if(index > 0) {
            QTermWidget *termWidget = (QTermWidget *)sessionTab->widget(index);
            foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
                disconnect(sessionsWindow,SIGNAL(hexDataDup(const char*,int)),
                            hexViewWindow,SLOT(recvData(const char*,int)));
                if(sessionsWindow->getTermWidget() == termWidget) {
                    logSessionAction->setChecked(sessionsWindow->isLog());
                    rawLogSessionAction->setChecked(sessionsWindow->isRawLog());
                    if(hexViewAction->isChecked()) {
                        connect(sessionsWindow,SIGNAL(hexDataDup(const char*,int)),
                                hexViewWindow,SLOT(recvData(const char*,int)));
                    }
                }
            }
        }
    });
    connect(sessionTab,&SessionTab::showContextMenu,this,[=](int index){
        QMenu *menu = new QMenu(this);
        if(index != -1) {
            QAction *closeAction = new QAction(QFontIcon::icon(QChar(0xf00d)),tr("Close"),this);
            menu->addAction(closeAction);
            connect(closeAction,&QAction::triggered,this,[=](){
                stopSession(index);
            });
        } else {
            if(sessionTab->count() != 0) {
                menu->addAction(copyAction);
                menu->addAction(pasteAction);
                menu->addSeparator();
                menu->addAction(selectAllAction);
                menu->addAction(findAction);
            } else {
                return;
            }
        }
        menu->move(cursor().pos());
        menu->show();
    });
    connect(sessionTab,&SessionTab::tabBarDoubleClicked,this,[=](int index){
        QTermWidget *termWidget = (QTermWidget *)sessionTab->widget(index);
        foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
            if(sessionsWindow->getTermWidget() == termWidget) {
                sessionsWindow->setShowShortTitle(!sessionsWindow->getShowShortTitle());
                sessionTab->setTabText(index,sessionsWindow->getTitle());
                break;
            }
        }
    });

    connect(newLocalShellShortCut,&QShortcut::activated,this,[=](){
        startLocalShellSession();
    });
    connect(cloneSessionShortCut,&QShortcut::activated,this,[=](){
        cloneCurrentSession();
    });
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
    themeMenu->setTitle(tr("Theme"));
    helpMenu->setTitle(tr("Help"));

    connectAction->setText(tr("Connect..."));
    connectAction->setIcon(QFontIcon::icon(QChar(0xf0c1)));
    sessionManagerAction->setText(tr("Session Manager"));
    sessionManagerAction->setIcon(QFontIcon::icon(QChar(0xf0e8)));
    quickConnectAction->setText(tr("Quick Connect..."));
    quickConnectAction->setIcon(QFontIcon::icon(QChar(0xf0e7)));
    connectInTabAction->setText(tr("Connect in Tab/Tile..."));
    connectLocalShellAction->setText(tr("Connect Local Shell"));
    connectLocalShellAction->setIcon(QFontIcon::icon(QChar(0xf120)));
    reconnectAction->setText(tr("Reconnect"));
    reconnectAction->setIcon(QFontIcon::icon(QChar(0xf021)));
    reconnectAllAction->setText(tr("Reconnect All"));
    disconnectAction->setText(tr("Disconnect"));
    disconnectAction->setIcon(QFontIcon::icon(QChar(0xf127)));
    connectAddressEdit->setPlaceholderText(tr("Enter host <Alt+R> to connect"));
    disconnectAllAction->setText(tr("Disconnect All"));
    cloneSessionAction->setText(tr("Clone Session"));
    lockSessionAction->setText(tr("Lock Session"));
    lockSessionAction->setIcon(QFontIcon::icon(QChar(0xf023)));
    logSessionAction->setText(tr("Log Session"));
    rawLogSessionAction->setText(tr("Raw Log Session"));
    hexViewAction->setText(tr("Hex View"));
    exitAction->setText(tr("Exit"));

    copyAction->setText(tr("Copy"));
    copyAction->setIcon(QFontIcon::icon(QChar(0xf0c5)));
    pasteAction->setText(tr("Paste"));
    pasteAction->setIcon(QFontIcon::icon(QChar(0xf0ea)));
    copyAndPasteAction->setText(tr("Copy and Paste"));
    selectAllAction->setText(tr("Select All"));
    findAction->setText(tr("Find..."));
    findAction->setIcon(QFontIcon::icon(QChar(0xf002)));
    printScreenAction->setText(tr("Print Screen"));
    printScreenAction->setIcon(QFontIcon::icon(QChar(0xf02f)));
    resetAction->setText(tr("Reset"));

    zoomInAction->setText(tr("Zoom In"));
    zoomInAction->setIcon(QFontIcon::icon(QChar(0xf00e)));
    zoomOutAction->setText(tr("Zoom Out"));
    zoomOutAction->setIcon(QFontIcon::icon(QChar(0xf010)));
    zoomResetAction->setText(tr("Zoom Reset"));
    zoomResetAction->setIcon(QFontIcon::icon(QChar(0xf057)));
    fullScreenAction->setText(tr("Full Screen"));

    sessionOptionsAction->setText(tr("Session Options..."));
    sessionOptionsAction->setIcon(QFontIcon::icon(QChar(0xf1de)));
    globalOptionsAction->setText(tr("Global Options..."));
    globalOptionsAction->setIcon(QFontIcon::icon(QChar(0xf013)));
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

    lightThemeAction->setText(tr("Light"));
    darkThemeAction->setText(tr("Dark"));

    helpAction->setText(tr("Help"));
    helpAction->setIcon(QFontIcon::icon(QChar(0xf128)));
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
    themeMenu = new QMenu(this);
    ui->menuBar->addMenu(themeMenu);
    helpMenu = new QMenu(this);
    ui->menuBar->addMenu(helpMenu);

    connectAction = new QAction(this);
    fileMenu->addAction(connectAction);
    sessionManagerAction = new QAction(this);
    ui->toolBar->addAction(sessionManagerAction);
    quickConnectAction = new QAction(this);
    fileMenu->addAction(quickConnectAction);
    ui->toolBar->addAction(quickConnectAction);
    connectInTabAction = new QAction(this);
    fileMenu->addAction(connectInTabAction);
    connectLocalShellAction = new QAction(this);
    fileMenu->addAction(connectLocalShellAction);
    ui->toolBar->addAction(connectLocalShellAction);
    fileMenu->addSeparator();
    reconnectAction = new QAction(this);
    fileMenu->addAction(reconnectAction);
    ui->toolBar->addAction(reconnectAction);
    reconnectAllAction = new QAction(this);
    fileMenu->addAction(reconnectAllAction);
    disconnectAction = new QAction(this);
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
    lockSessionAction = new QAction(this);
    fileMenu->addAction(lockSessionAction);
    fileMenu->addSeparator();
    logSessionAction = new QAction(this);
    logSessionAction->setCheckable(true);
    logSessionAction->setChecked(false);
    fileMenu->addAction(logSessionAction);
    rawLogSessionAction = new QAction(this);
    rawLogSessionAction->setCheckable(true);
    rawLogSessionAction->setChecked(false);
    fileMenu->addAction(rawLogSessionAction);
    hexViewAction = new QAction(this);
    hexViewAction->setCheckable(true);
    fileMenu->addAction(hexViewAction);
    fileMenu->addSeparator();
    exitAction = new QAction(this);
    fileMenu->addAction(exitAction);

    copyAction = new QAction(this);
    editMenu->addAction(copyAction);
    ui->toolBar->addAction(copyAction);
    pasteAction = new QAction(this);
    editMenu->addAction(pasteAction);
    ui->toolBar->addAction(pasteAction);
    copyAndPasteAction = new QAction(this);
    editMenu->addAction(copyAndPasteAction);
    selectAllAction = new QAction(this);
    editMenu->addAction(selectAllAction);
    findAction = new QAction(this);
    editMenu->addAction(findAction);
    editMenu->addSeparator();
    ui->toolBar->addAction(findAction);
    ui->toolBar->addSeparator();
    printScreenAction = new QAction(this);
    editMenu->addAction(printScreenAction);
    editMenu->addSeparator();
    ui->toolBar->addAction(printScreenAction);
    ui->toolBar->addSeparator();
    resetAction = new QAction(this);
    editMenu->addAction(resetAction);

    zoomInAction = new QAction(this);
    viewMenu->addAction(zoomInAction);
    zoomOutAction = new QAction(this);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addSeparator();
    zoomResetAction = new QAction(this);
    viewMenu->addAction(zoomResetAction);
    viewMenu->addSeparator();
    fullScreenAction = new QAction(this);
    fullScreenAction->setCheckable(true);
    viewMenu->addAction(fullScreenAction);

    sessionOptionsAction = new QAction(this);
    optionsMenu->addAction(sessionOptionsAction);
    ui->toolBar->addAction(sessionOptionsAction);
    globalOptionsAction = new QAction(this);
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

    themeActionGroup = new QActionGroup(this);
    lightThemeAction = new QAction(this);
    lightThemeAction->setActionGroup(themeActionGroup);
    lightThemeAction->setCheckable(true);
    lightThemeAction->setChecked(!isDarkTheme);
    themeMenu->addAction(lightThemeAction);
    darkThemeAction = new QAction(this);
    darkThemeAction->setActionGroup(themeActionGroup);
    darkThemeAction->setCheckable(true);
    darkThemeAction->setChecked(isDarkTheme);
    themeMenu->addAction(darkThemeAction);

    helpAction = new QAction(this);
    helpMenu->addAction(helpAction);
    ui->toolBar->addAction(helpAction);
    helpMenu->addSeparator();
    aboutAction = new QAction(this);
    helpMenu->addAction(aboutAction);
    aboutQtAction = new QAction(this);
    helpMenu->addAction(aboutQtAction);

    menuAndToolBarRetranslateUi();

    reconnectAction->setEnabled(false);
    reconnectAllAction->setEnabled(false);
    disconnectAction->setEnabled(false);
    disconnectAllAction->setEnabled(false);
    cloneSessionAction->setEnabled(false);
    lockSessionAction->setEnabled(false);
    logSessionAction->setEnabled(false);
    rawLogSessionAction->setEnabled(false);
}

void MainWindow::menuAndToolBarConnectSignals(void) {
    connect(quickConnectAction,&QAction::triggered,this,[=](){
        quickConnectWindow->show();
    });
    connect(quickConnectWindow,&QuickConnectWindow::sendQuickConnectData,this,
            [=](QuickConnectWindow::QuickConnectData data){
        if(data.type == QuickConnectWindow::Telnet) {
            QTelnet::SocketType type = QTelnet::TCP;
            if(data.TelnetData.webSocket == "None") {
                type = QTelnet::TCP;
            } else if(data.TelnetData.webSocket == "Insecure") {
                type = QTelnet::WEBSOCKET;
            } else if(data.TelnetData.webSocket == "Secure") {
                type = QTelnet::SECUREWEBSOCKET;
            }
            startTelnetSession(data.TelnetData.hostname,data.TelnetData.port,type);
        } else if(data.type == QuickConnectWindow::Serial) {
            startSerialSession(
                        data.SerialData.portName,data.SerialData.baudRate,
                        data.SerialData.dataBits,data.SerialData.parity,
                        data.SerialData.stopBits,data.SerialData.flowControl,
                        data.SerialData.xEnable);
        } else if(data.type == QuickConnectWindow::LocalShell) {
            startLocalShellSession(data.LocalShellData.command);
        } else if(data.type == QuickConnectWindow::Raw) {
            startRawSocketSession(data.RawData.hostname,data.RawData.port);
        }
    });
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        startLocalShellSession();
    });
    connect(disconnectAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        stopSession(sessionTab->indexOf(termWidget));
    });
    connect(disconnectAllAction,&QAction::triggered,this,[=](){
        stopAllSession();
    });
    connect(cloneSessionAction,&QAction::triggered,this,[=](){
        cloneCurrentSession();
    });
    connect(logSessionAction,&QAction::triggered,this,
        [&](void) {
            if(sessionTab->count() == 0) {
                logSessionAction->setChecked(false);
                return;
            }
            QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
            foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(!sessionsWindow->isLog()) {
                        sessionsWindow->setLog(true);
                        logSessionAction->setChecked(true);
                    } else {
                        sessionsWindow->setLog(false);
                        logSessionAction->setChecked(false);
                    }
                    break;
                }
            }
        }
    );
    connect(rawLogSessionAction,&QAction::triggered,this,
        [&](void) {
            if(sessionTab->count() == 0) {
                rawLogSessionAction->setChecked(false);
                return;
            }
            QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
            foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(!sessionsWindow->isRawLog()) {
                        sessionsWindow->setRawLog(true);
                        rawLogSessionAction->setChecked(true);
                    } else {
                        sessionsWindow->setRawLog(false);
                        rawLogSessionAction->setChecked(false);
                    }
                    break;
                }
            }
        }
    );
    connect(hexViewAction,&QAction::triggered,this,[=](){
        if(hexViewAction->isChecked())
            hexViewWindow->show();
        else 
            hexViewWindow->hide();
    });
    connect(hexViewWindow,&HexViewWindow::hideOrClose,this,[=](){
        hexViewAction->setChecked(false);
    });
    connect(globalOptionsWindow,&GlobalOptions::colorSchemeChanged,this,[=](QString colorScheme){
        foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
            sessionsWindow->getTermWidget()->setColorScheme(colorScheme);
        }
    });
    connect(keyMapManagerWindow,&keyMapManager::keyBindingChanged,this,[=](QString keyBinding){
        foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
            sessionsWindow->getTermWidget()->setKeyBindings(keyBinding);
        }
    });

    connect(copyAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->copyClipboard();
    });
    connect(pasteAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->pasteClipboard();
    });
    connect(copyAndPasteAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->copyClipboard();
        termWidget->pasteClipboard();
    });
    connect(selectAllAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->selectAll();
    });
    connect(findAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->toggleShowSearchBar();
    });
    connect(resetAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->clear();
    });
    connect(zoomInAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->zoomIn();
    });
    connect(zoomOutAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->zoomOut();
    });
    connect(zoomResetAction,&QAction::triggered,this,[=](){
        if(sessionTab->count() == 0) return;
        QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
        termWidget->setTerminalFont(globalOptionsWindow->getCurrentFont());
    });
    connect(fullScreenAction,&QAction::triggered,this,[=](bool checked){
        if(checked) {
            this->showFullScreen();
        } else {
            this->showNormal();
        }
    });
    connect(keymapManagerAction,&QAction::triggered,this,[=](){
        keyMapManagerWindow->show();
    });
    connect(globalOptionsAction,&QAction::triggered,this,[=](){
        globalOptionsWindow->show();
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
        sessionTab->retranslateUi();
        menuAndToolBarRetranslateUi();
    });
    connect(lightThemeAction,&QAction::triggered,this,[=](){
        isDarkTheme = false;
        QFile ftheme(":/qdarkstyle/light/lightstyle.qss");
        if (!ftheme.exists())   {
            qDebug() << "Unable to set stylesheet, file not found!";
        } else {
            ftheme.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&ftheme);
            qApp->setStyleSheet(ts.readAll());
        }
        QFontIcon::instance()->setColor(Qt::black);
        menuAndToolBarRetranslateUi();
    });
    connect(darkThemeAction,&QAction::triggered,this,[=](){
        isDarkTheme = true;
        QFile ftheme(":/qdarkstyle/dark/darkstyle.qss");
        if (!ftheme.exists())   {
            qDebug() << "Unable to set stylesheet, file not found!";
        } else {
            ftheme.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&ftheme);
            qApp->setStyleSheet(ts.readAll());
        }
        QFontIcon::instance()->setColor(Qt::white);
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
}

SessionsWindow *MainWindow::startTelnetSession(QString hostname, quint16 port, QTelnet::SocketType type)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    sessionsWindow->setLongTitle(tr("Telnet - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Telnet"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    sessionsWindow->startTelnetSession(hostname,port,type);
    sessionActionsList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            sessionTab->setTabText(sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    sessionTab->setCurrentIndex(sessionTab->count()-1);
    return sessionsWindow;
}

SessionsWindow *MainWindow::startSerialSession(QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Serial,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    sessionsWindow->setLongTitle(tr("Serial - ")+portName);
    sessionsWindow->setShortTitle(tr("Serial"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    sessionsWindow->startSerialSession(portName,baudRate,dataBits,parity,stopBits,flowControl,xEnable);
    sessionActionsList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            sessionTab->setTabText(sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    sessionTab->setCurrentIndex(sessionTab->count()-1);
    return sessionsWindow;
}

SessionsWindow *MainWindow::startRawSocketSession(QString hostname, quint16 port)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    sessionsWindow->setLongTitle(tr("Raw - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Raw"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    sessionsWindow->startRawSocketSession(hostname,port);
    sessionActionsList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            sessionTab->setTabText(sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    sessionTab->setCurrentIndex(sessionTab->count()-1);
    return sessionsWindow;
}

SessionsWindow *MainWindow::startLocalShellSession(const QString &command)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    if(command.isEmpty()) {
        sessionsWindow->setLongTitle(tr("Local Shell"));
    } else {
        sessionsWindow->setLongTitle(tr("Local Shell - ")+command);
    }
    sessionsWindow->setShortTitle(tr("Local Shell"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    sessionsWindow->startLocalShellSession(command);
    sessionActionsList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            QString dir = newTitle.right(newTitle.length()-newTitle.indexOf(":")-1);
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(dir);
            sessionTab->setTabText(sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
            // newTitle lile [hostname:dir]
            sessionsWindow->setWorkingDirectory(dir.replace("~",QDir::homePath()));
        }
    });
    sessionTab->setCurrentIndex(sessionTab->count()-1);
    return sessionsWindow;
}

int MainWindow::stopSession(int index)
{
    QTermWidget *termWidget = (QTermWidget *)sessionTab->widget(index);
    foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
        if(sessionsWindow->getTermWidget() == termWidget) {
            sessionActionsList.removeOne(sessionsWindow);
            sessionTab->removeTab(index);
            delete sessionsWindow;
            return 0;
        }
    }
    return -1;
}

int MainWindow::stopAllSession(void)
{
    while(sessionTab->count() > 0) {
        stopSession(sessionTab->count());
    }
    return 0;
}

int MainWindow::cloneCurrentSession(void)
{
    if(sessionTab->count() == 0) return -1;
    QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
    foreach(SessionsWindow *sessionsWindow, sessionActionsList) {
        if(sessionsWindow->getTermWidget() == termWidget) {
            SessionsWindow *sessionsWindowClone = new SessionsWindow(sessionsWindow->getSessionType(),this);
            sessionsWindowClone->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
            sessionsWindowClone->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
            sessionsWindowClone->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
            sessionsWindowClone->setLongTitle(sessionsWindow->getLongTitle());
            sessionsWindowClone->setShortTitle(sessionsWindow->getShortTitle());
            sessionsWindowClone->setShowShortTitle(sessionsWindow->getShowShortTitle());
            sessionTab->addTab(sessionsWindowClone->getTermWidget(), sessionTab->tabText(sessionTab->indexOf(termWidget)));
            sessionsWindowClone->cloneSession(sessionsWindow);
            sessionActionsList.push_back(sessionsWindowClone);
            connect(sessionsWindowClone->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
                if(title == 0 || title == 2) {
                    sessionsWindowClone->setLongTitle(newTitle);
                    if(sessionsWindowClone->getSessionType() == SessionsWindow::LocalShell) {
                        // newTitle lile [hostname:dir]
                        QString dir = newTitle.right(newTitle.length()-newTitle.indexOf(":")-1);
                        sessionsWindowClone->setShortTitle(dir);
                        sessionsWindowClone->setWorkingDirectory(dir.replace("~",QDir::homePath()));
                    } else {
                        sessionsWindowClone->setShortTitle(newTitle);
                    }
                    sessionTab->setTabText(sessionTab->indexOf(sessionsWindowClone->getTermWidget()), sessionsWindowClone->getTitle());
                }
            });
            sessionTab->setCurrentIndex(sessionTab->count()-1);
            return 0;
        }
    }
    return -1;
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
