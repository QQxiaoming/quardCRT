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
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QStatusBar>
#include <QPlainTextEdit>
#include <QHostInfo>

#include "qfonticon.h"
#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptions.h"
#include "commandwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QLocale::Language lang, bool isDark, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , language(lang)
    , isDarkTheme(isDark) {

    ui->setupUi(this);

    /* Create the main UI */
    sessionManagerWidget = new SessionManagerWidget(this);
    ui->centralwidget->layout()->addWidget(sessionManagerWidget);
    sessionManagerWidget->setVisible(false);

    QSplitter *splitter = new QSplitter(Qt::Vertical,this);
    splitter->setHandleWidth(1);
    ui->centralwidget->layout()->addWidget(splitter);

    sessionTab = new SessionTab(this);
    splitter->addWidget(sessionTab);
    splitter->setCollapsible(0,false);

    cmdWindow = new CommandWindow(this);
    splitter->addWidget(cmdWindow);
    splitter->setCollapsible(1,true);
    splitter->setSizes(QList<int>() << 1 << 0);

    quickConnectWindow = new QuickConnectWindow(this);

    keyMapManagerWindow = new keyMapManager(this);
    keyMapManagerWindow->setAvailableKeyBindings(QTermWidget::availableKeyBindings());

    globalOptionsWindow = new GlobalOptions(this);
    globalOptionsWindow->setAvailableColorSchemes(QTermWidget::availableColorSchemes());

    hexViewWindow = new HexViewWindow(HexViewWindow::RECV,this);
    hexViewWindow->setFont(globalOptionsWindow->getCurrentFont());

    QGraphicsScene *scene = new QGraphicsScene(this);
    sessionManagerPushButton = new QPushButton();
    QGraphicsProxyWidget *w = scene->addWidget(sessionManagerPushButton);
    w->setPos(0,0);
    w->setRotation(-90);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setFrameStyle(0);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sessionManagerPushButton->setFixedSize(200,26);
    ui->graphicsView->setFixedSize(30, 200);
    ui->sidewidget->setFixedWidth(30);

    menuAndToolBarInit();

    /* connect signals */
    menuAndToolBarConnectSignals();

    connect(sessionManagerPushButton,&QPushButton::clicked,this,[=](){
        if(sessionManagerWidget->isVisible() == false) {
            sessionManagerWidget->setVisible(true);
        } else {
            sessionManagerWidget->setVisible(false);
        }
    });
    connect(sessionTab,&FancyTabWidget::tabAddRequested,this,[=](){
        cloneCurrentSession();
    });
    connect(sessionTab,&FancyTabWidget::tabCloseRequested,this,[=](int index){
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
    connect(sessionTab,&SessionTab::showContextMenu,this,[=](int index, const QPoint& position){
        QMenu *menu = new QMenu(this);
        if(index != -1) {
            QAction *closeAction = new QAction(QFontIcon::icon(QChar(0xf00d)),tr("Close"),this);
            menu->addAction(closeAction);
            connect(closeAction,&QAction::triggered,this,[=](){
                stopSession(index);
            });
        } else {
            if(sessionTab->count() != 0) {
                QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
                QPoint maptermWidgetPos = termWidget->mapFromGlobal(position);
                QList<QAction*> ftActions = termWidget->filterActions(maptermWidgetPos);
                if(!ftActions.isEmpty()) {
                    menu->addActions(ftActions);
                    menu->addSeparator();
                }
                menu->addAction(copyAction);
                menu->addAction(pasteAction);
                menu->addSeparator();
                menu->addAction(selectAllAction);
                menu->addAction(findAction);
                if(!ui->menuBar->isVisible()) {
                    menu->addSeparator();
                    menu->addAction(menuBarAction);
                }
            } else {
                if(!ui->menuBar->isVisible()) {
                    menu->addAction(menuBarAction);
                } else {
                    delete menu;
                    return;
                }
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
    connect(cmdWindow, &CommandWindow::sendData, this, [=](const QByteArray &data) {
        if(sessionTab->count() != 0) {
            QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
            termWidget->proxySendData(data);
        }
    });

    connect(ui->statusBar,&QStatusBar::messageChanged,this,[&](const QString &message){
        if(message.isEmpty()) {
            ui->statusBar->showMessage(tr("Ready"));
        } else {
            ui->statusBar->showMessage(message);
        }
    });

    ui->statusBar->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {
    delete sessionManagerPushButton;
    delete ui;
}

void MainWindow::menuAndToolBarRetranslateUi(void) {
    sessionManagerPushButton->setText(tr("Session Manager"));

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
    connectAction->setStatusTip(tr("Connect to a host <Alt+C>"));
    connectAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_C));
    sessionManagerAction->setText(tr("Session Manager"));
    sessionManagerAction->setIcon(QFontIcon::icon(QChar(0xf0e8)));
    sessionManagerAction->setStatusTip(tr("Go to the Session Manager <Alt+M>"));
    sessionManagerAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_M));
    quickConnectAction->setText(tr("Quick Connect..."));
    quickConnectAction->setIcon(QFontIcon::icon(QChar(0xf0e7)));
    quickConnectAction->setStatusTip(tr("Quick Connect to a host <Alt+Q>"));
    quickConnectAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_Q));
    connectInTabAction->setText(tr("Connect in Tab/Tile..."));
    connectInTabAction->setStatusTip(tr("Connect to a host in a new tab <Alt+B>"));
    connectInTabAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_B));
    connectLocalShellAction->setText(tr("Connect Local Shell"));
    connectLocalShellAction->setIcon(QFontIcon::icon(QChar(0xf120)));
    connectLocalShellAction->setStatusTip(tr("Connect to a local shell <Alt+T>"));
    connectLocalShellAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_T));
    reconnectAction->setText(tr("Reconnect"));
    reconnectAction->setIcon(QFontIcon::icon(QChar(0xf021)));
    reconnectAction->setStatusTip(tr("Reconnect current session"));
    reconnectAllAction->setText(tr("Reconnect All"));
    reconnectAllAction->setStatusTip(tr("Reconnect all sessions <Alt+A>"));
    reconnectAllAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_A));
    disconnectAction->setText(tr("Disconnect"));
    disconnectAction->setIcon(QFontIcon::icon(QChar(0xf127)));
    disconnectAction->setStatusTip(tr("Disconnect current session"));
    connectAddressEdit->setPlaceholderText(tr("Enter host <Alt+R> to connect"));
    connectAddressEdit->setStatusTip(tr("Enter host <Alt+R> to connect"));
    disconnectAllAction->setText(tr("Disconnect All"));
    disconnectAllAction->setStatusTip(tr("Disconnect all sessions"));
    cloneSessionAction->setText(tr("Clone Session"));
    cloneSessionAction->setStatusTip(tr("Clone current session <Ctrl+Shift+T>"));
    cloneSessionAction->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_T));
    lockSessionAction->setText(tr("Lock Session"));
    lockSessionAction->setIcon(QFontIcon::icon(QChar(0xf023)));
    lockSessionAction->setStatusTip(tr("Lock/Unlock current session"));
    logSessionAction->setText(tr("Log Session"));
    logSessionAction->setStatusTip(tr("Create a log file for current session"));
    rawLogSessionAction->setText(tr("Raw Log Session"));
    logSessionAction->setStatusTip(tr("Create a raw log file for current session"));
    hexViewAction->setText(tr("Hex View"));
    hexViewAction->setStatusTip(tr("Show/Hide Hex View for current session"));
    exitAction->setText(tr("Exit"));
    exitAction->setStatusTip(tr("Quit the application"));

    copyAction->setText(tr("Copy"));
    copyAction->setIcon(QFontIcon::icon(QChar(0xf0c5)));
#if defined(Q_OS_MACOS)
    copyAction->setStatusTip(tr("Copy the selected text to the clipboard <Ctrl+C>"));
    copyAction->setShortcut(QKeySequence(Qt::META|Qt::Key_C));
#else
    copyAction->setStatusTip(tr("Copy the selected text to the clipboard <Ctrl+Ins>"));
    copyAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Insert));
#endif
    pasteAction->setText(tr("Paste"));
    pasteAction->setIcon(QFontIcon::icon(QChar(0xf0ea)));
#if defined(Q_OS_MACOS)
    pasteAction->setStatusTip(tr("Paste the clipboard text to the current session <Ctrl+V>"));
    pasteAction->setShortcut(QKeySequence(Qt::META|Qt::Key_V));
#else
    pasteAction->setStatusTip(tr("Paste the clipboard text to the current session <Shift+Ins>"));
    pasteAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_Insert));
#endif
    copyAndPasteAction->setText(tr("Copy and Paste"));
    copyAndPasteAction->setStatusTip(tr("Copy the selected text to the clipboard and paste to the current session"));
    selectAllAction->setText(tr("Select All"));
    selectAllAction->setStatusTip(tr("Select all text in the current session <Ctrl+Shift+A>"));
    selectAllAction->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_A));
    findAction->setText(tr("Find..."));
    findAction->setIcon(QFontIcon::icon(QChar(0xf002)));
    findAction->setStatusTip(tr("Find text in the current session <Ctrl+F>"));
    findAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_F));
    printScreenAction->setText(tr("Print Screen"));
    printScreenAction->setIcon(QFontIcon::icon(QChar(0xf02f)));
    printScreenAction->setStatusTip(tr("Print current screen"));
    resetAction->setText(tr("Reset"));
    resetAction->setStatusTip(tr("Reset terminal emulator"));

    zoomInAction->setText(tr("Zoom In"));
    zoomInAction->setIcon(QFontIcon::icon(QChar(0xf00e)));
    zoomInAction->setStatusTip(tr("Zoom In <Ctrl+\"=\">"));
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Equal));
    zoomOutAction->setText(tr("Zoom Out"));
    zoomOutAction->setIcon(QFontIcon::icon(QChar(0xf010)));
    zoomOutAction->setStatusTip(tr("Zoom Out <Ctrl+\"-\">"));
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Minus));
    zoomResetAction->setText(tr("Zoom Reset"));
    zoomResetAction->setIcon(QFontIcon::icon(QChar(0xf057)));
    zoomResetAction->setStatusTip(tr("Zoom Reset"));
    menuBarAction->setText(tr("Menu Bar"));
    menuBarAction->setStatusTip(tr("Show/Hide Menu Bar <Alt+U>"));
    menuBarAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_U));
    toolBarAction->setText(tr("Tool Bar"));
    toolBarAction->setStatusTip(tr("Show/Hide Tool Bar"));
    statusBarAction->setText(tr("Status Bar"));
    statusBarAction->setStatusTip(tr("Show/Hide Status Bar"));
    cmdWindowAction->setText(tr("Command Window"));
    cmdWindowAction->setStatusTip(tr("Show/Hide Command Window"));
    sideWindowAction->setText(tr("Side Window"));
    sideWindowAction->setStatusTip(tr("Show/Hide Side Window"));
    fullScreenAction->setText(tr("Full Screen"));
    fullScreenAction->setStatusTip(tr("Full Screen <Alt+Enter>"));
    fullScreenAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_Enter));

    sessionOptionsAction->setText(tr("Session Options..."));
    sessionOptionsAction->setIcon(QFontIcon::icon(QChar(0xf1de)));
    sessionOptionsAction->setStatusTip(tr("Configure session options"));
    globalOptionsAction->setText(tr("Global Options..."));
    globalOptionsAction->setIcon(QFontIcon::icon(QChar(0xf013)));
    globalOptionsAction->setStatusTip(tr("Configure global options"));
    autoSaveOptionsAction->setText(tr("Auto Save Options"));
    autoSaveOptionsAction->setStatusTip(tr("Automatically save session options and global options"));
    saveSettingsNowAction->setText(tr("Save Settings Now"));
    saveSettingsNowAction->setStatusTip(tr("Save options configuration now"));

    sendASCIIAction->setText(tr("Send ASCII..."));
    sendASCIIAction->setStatusTip(tr("Send ASCII file"));
    receiveASCIIAction->setText(tr("Receive ASCII..."));
    receiveASCIIAction->setStatusTip(tr("Receive ASCII file"));
    sendBinaryAction->setText(tr("Send Binary..."));
    sendBinaryAction->setStatusTip(tr("Send Binary file"));
    sendXmodemAction->setText(tr("Send Xmodem..."));
    sendXmodemAction->setStatusTip(tr("Send a file using Xmodem"));
    receiveXmodemAction->setText(tr("Receive Xmodem..."));
    receiveXmodemAction->setStatusTip(tr("Receive a file using Xmodem"));
    sendYmodemAction->setText(tr("Send Ymodem..."));
    sendYmodemAction->setStatusTip(tr("Send a file using Ymodem"));
    receiveYmodemAction->setText(tr("Receive Ymodem..."));
    receiveYmodemAction->setStatusTip(tr("Receive a file using Ymodem"));
    zmodemUploadListAction->setText(tr("Zmodem Upload List..."));
    zmodemUploadListAction->setStatusTip(tr("Display Zmodem file upload list"));
    startZmodemUploadAction->setText(tr("Start Zmodem Upload"));
    startZmodemUploadAction->setStatusTip(tr("Start Zmodem file upload"));
    startTFTPServerAction->setText(tr("Start TFTP Server"));
    startTFTPServerAction->setStatusTip(tr("Start/Stop the TFTP server"));

    runAction->setText(tr("Run..."));
    runAction->setStatusTip(tr("Run a script"));
    cancelAction->setText(tr("Cancel"));
    cancelAction->setStatusTip(tr("Cancel script execution"));
    startRecordingScriptAction->setText(tr("Start Recording Script"));
    startRecordingScriptAction->setStatusTip(tr("Start recording script"));
    stopRecordingScriptAction->setText(tr("Stop Recording Script..."));
    stopRecordingScriptAction->setStatusTip(tr("Stop recording script"));
    canlcelRecordingScriptAction->setText(tr("Cancel Recording Script"));
    canlcelRecordingScriptAction->setStatusTip(tr("Cancel recording script"));

    keymapManagerAction->setText(tr("Keymap Manager"));
    keymapManagerAction->setStatusTip(tr("Display keymap editor"));
    createPublicKeyAction->setText(tr("Create Public Key..."));
    createPublicKeyAction->setStatusTip(tr("Create a public key"));
    publickeyManagerAction->setText(tr("Publickey Manager"));
    publickeyManagerAction->setStatusTip(tr("Display publickey manager"));

    tabAction->setText(tr("Tab"));
    tabAction->setStatusTip(tr("Arrange sessions in tabs"));
    tileAction->setText(tr("Tile"));
    tileAction->setStatusTip(tr("Arrange sessions in non-overlapping tiles"));
    cascadeAction->setText(tr("Cascade"));
    cascadeAction->setStatusTip(tr("Arrange sessions to overlap each other"));

    chineseAction->setText(tr("Chinese"));
    chineseAction->setStatusTip(tr("Switch to Chinese"));
    englishAction->setText(tr("English"));
    englishAction->setStatusTip(tr("Switch to English"));
    japaneseAction->setText(tr("Japanese"));
    japaneseAction->setStatusTip(tr("Switch to Japanese"));

    lightThemeAction->setText(tr("Light"));
    lightThemeAction->setStatusTip(tr("Switch to light theme"));
    darkThemeAction->setText(tr("Dark"));
    darkThemeAction->setStatusTip(tr("Switch to dark theme"));

    helpAction->setText(tr("Help"));
    helpAction->setIcon(QFontIcon::icon(QChar(0xf128)));
    helpAction->setStatusTip(tr("Display help"));
    aboutAction->setText(tr("About"));
    aboutAction->setIcon(QIcon(":/icons/icons/about.png"));
    aboutAction->setStatusTip(tr("Display about dialog"));
    aboutQtAction->setText(tr("About Qt"));
    aboutQtAction->setIcon(QIcon(":/icons/icons/aboutqt.png"));
    aboutQtAction->setStatusTip(tr("Display about Qt dialog"));
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
    sessionManagerWidget->addActionOnToolBar(connectAction);
    sessionManagerAction = new QAction(this);
    ui->toolBar->addAction(sessionManagerAction);
    quickConnectAction = new QAction(this);
    fileMenu->addAction(quickConnectAction);
    ui->toolBar->addAction(quickConnectAction);
    sessionManagerWidget->addActionOnToolBar(quickConnectAction);
    connectInTabAction = new QAction(this);
    fileMenu->addAction(connectInTabAction);
    connectLocalShellAction = new QAction(this);
    fileMenu->addAction(connectLocalShellAction);
    ui->toolBar->addAction(connectLocalShellAction);
    sessionManagerWidget->addActionOnToolBar(connectLocalShellAction);
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
    sessionManagerWidget->addActionOnToolBar(lockSessionAction);
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
    sessionManagerWidget->addActionOnToolBar(copyAction);
    pasteAction = new QAction(this);
    editMenu->addAction(pasteAction);
    ui->toolBar->addAction(pasteAction);
    sessionManagerWidget->addActionOnToolBar(pasteAction);
    copyAndPasteAction = new QAction(this);
    editMenu->addAction(copyAndPasteAction);
    selectAllAction = new QAction(this);
    editMenu->addAction(selectAllAction);
    findAction = new QAction(this);
    editMenu->addAction(findAction);
    sessionManagerWidget->addActionOnToolBar(findAction);
    editMenu->addSeparator();
    ui->toolBar->addAction(findAction);
    ui->toolBar->addSeparator();
    sessionManagerWidget->addActionOnToolBar(findAction);
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
    menuBarAction = new QAction(this);
    menuBarAction->setCheckable(true);
    menuBarAction->setChecked(true);
    viewMenu->addAction(menuBarAction);
    toolBarAction = new QAction(this);
    toolBarAction->setCheckable(true);
    toolBarAction->setChecked(true);
    viewMenu->addAction(toolBarAction);
    statusBarAction = new QAction(this);
    statusBarAction->setCheckable(true);
    statusBarAction->setChecked(true);
    viewMenu->addAction(statusBarAction);
    cmdWindowAction = new QAction(this);
    cmdWindowAction->setCheckable(true);
    cmdWindowAction->setChecked(true);
    viewMenu->addAction(cmdWindowAction);
    sideWindowAction = new QAction(this);
    sideWindowAction->setCheckable(true);
    sideWindowAction->setChecked(true);
    viewMenu->addAction(sideWindowAction);
    viewMenu->addSeparator();
    fullScreenAction = new QAction(this);
    fullScreenAction->setCheckable(true);
    viewMenu->addAction(fullScreenAction);

    sessionOptionsAction = new QAction(this);
    optionsMenu->addAction(sessionOptionsAction);
    ui->toolBar->addAction(sessionOptionsAction);
    sessionManagerWidget->addActionOnToolBar(sessionOptionsAction);
    globalOptionsAction = new QAction(this);
    optionsMenu->addAction(globalOptionsAction);
    optionsMenu->addSeparator();
    sessionManagerWidget->addActionOnToolBar(globalOptionsAction);
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
    sessionManagerWidget->addActionOnToolBar(helpAction);
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
    connect(connectAction,&QAction::triggered,this,[=](){
        sessionManagerWidget->setVisible(true);
    });
    connect(sessionManagerAction,&QAction::triggered,this,[=](){
        if(sessionManagerWidget->isVisible() == false) {
            sessionManagerWidget->setVisible(true);
        } else {
            sessionManagerWidget->setVisible(false);
        }
    });
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
    connect(connectInTabAction,&QAction::triggered,this,[=](){
        sessionManagerWidget->setVisible(true);
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
    connect(menuBarAction,&QAction::triggered,this,[=](bool checked){
        ui->menuBar->setVisible(checked);
    });
    connect(toolBarAction,&QAction::triggered,this,[=](bool checked){
        ui->toolBar->setVisible(checked);
    });
    connect(statusBarAction,&QAction::triggered,this,[=](bool checked){
        ui->statusBar->setVisible(checked);
    });
    connect(cmdWindowAction,&QAction::triggered,this,[=](bool checked){
        cmdWindow->setVisible(checked);
    });
    connect(sideWindowAction,&QAction::triggered,this,[=](bool checked){
        ui->sidewidget->setVisible(checked);
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
        sessionManagerWidget->retranslateUi();
        cmdWindow->retranslateUi();
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
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
    sessionsWindow->setLongTitle(tr("Telnet - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Telnet"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    QString name = hostname;
    for(uint32_t i=0;i<UINT_MAX;i++) {
        if(sessionManagerWidget->checkSession(name) == false) {
            break;
        }
        name = hostname+" ("+QString::number(i)+")";
    }
    sessionManagerWidget->addSession(name,SessionsWindow::Telnet);
    sessionsWindow->setName(name);
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
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
    sessionsWindow->setLongTitle(tr("Serial - ")+portName);
    sessionsWindow->setShortTitle(tr("Serial"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    QString name = portName;
    for(uint32_t i=0;i<UINT_MAX;i++) {
        if(sessionManagerWidget->checkSession(name) == false) {
            break;
        }
        name = portName+" ("+QString::number(i)+")";
    }
    sessionManagerWidget->addSession(name,SessionsWindow::Serial);
    sessionsWindow->setName(name);
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
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
    sessionsWindow->setLongTitle(tr("Raw - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Raw"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    QString name = hostname;
    for(uint32_t i=0;i<UINT_MAX;i++) {
        if(sessionManagerWidget->checkSession(name) == false) {
            break;
        }
        name = hostname+" ("+QString::number(i)+")";
    }
    sessionManagerWidget->addSession(name,SessionsWindow::RawSocket);
    sessionsWindow->setName(name);
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
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency()/100.0);
    if(command.isEmpty()) {
        sessionsWindow->setLongTitle(tr("Local Shell"));
    } else {
        sessionsWindow->setLongTitle(tr("Local Shell - ")+command);
    }
    sessionsWindow->setShortTitle(tr("Local Shell"));
    sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    QString name = "Local Shell";
    for(uint32_t i=0;i<UINT_MAX;i++) {
        if(sessionManagerWidget->checkSession(name) == false) {
            break;
        }
        name = "Local Shell ("+QString::number(i)+")";
    }
    sessionManagerWidget->addSession(name,SessionsWindow::LocalShell);
    sessionsWindow->setName(name);
    sessionsWindow->startLocalShellSession(command);
    sessionActionsList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            // newTitle lile username@hostname:dir
            static QRegularExpression stdTitleFormat("^(\\S+)@(\\S+):(.*)$");
            if(stdTitleFormat.match(newTitle).hasMatch()) {
                QString username = stdTitleFormat.match(newTitle).captured(1);
                QString hostname = stdTitleFormat.match(newTitle).captured(2);
                QString dir = stdTitleFormat.match(newTitle).captured(3);
                sessionsWindow->setShortTitle(dir);
            #if defined(Q_OS_WIN)
                QString sysUsername = qEnvironmentVariable("USERNAME");
                QString sysHostname = qEnvironmentVariable("COMPUTERNAME");
            #else
                QString sysUsername = qEnvironmentVariable("USER");
                QString sysHostname = QHostInfo::localHostName();
            #endif
                if((username == sysUsername) && (hostname == sysHostname)) {
                    sessionsWindow->setWorkingDirectory(dir.replace("~",QDir::homePath()));
                }
            } else {
                sessionsWindow->setShortTitle(newTitle);
            }
            sessionTab->setTabText(sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
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
            sessionManagerWidget->removeSession(sessionsWindow->getName());
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
            sessionsWindowClone->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
            sessionsWindowClone->setLongTitle(sessionsWindow->getLongTitle());
            sessionsWindowClone->setShortTitle(sessionsWindow->getShortTitle());
            sessionsWindowClone->setShowShortTitle(sessionsWindow->getShowShortTitle());
            sessionTab->addTab(sessionsWindowClone->getTermWidget(), sessionTab->tabText(sessionTab->indexOf(termWidget)));
            QString name = sessionsWindow->getName();
            for(uint32_t i=0;i<UINT_MAX;i++) {
                if(sessionManagerWidget->checkSession(name) == false) {
                    break;
                }
                name = sessionsWindow->getName()+" ("+QString::number(i)+")";
            }
            sessionManagerWidget->addSession(name,sessionsWindow->getSessionType());
            sessionsWindowClone->setName(name);
            sessionsWindowClone->cloneSession(sessionsWindow);
            sessionActionsList.push_back(sessionsWindowClone);
            connect(sessionsWindowClone->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
                if(title == 0 || title == 2) {
                    sessionsWindowClone->setLongTitle(newTitle);
                    static QRegularExpression stdTitleFormat("^(\\S+)@(\\S+):(.*)$");
                    if((sessionsWindowClone->getSessionType() == SessionsWindow::LocalShell) 
                            && (stdTitleFormat.match(newTitle).hasMatch()) ) {
                        // newTitle lile username@hostname:dir
                        QString username = stdTitleFormat.match(newTitle).captured(1);
                        QString hostname = stdTitleFormat.match(newTitle).captured(2);
                        QString dir = stdTitleFormat.match(newTitle).captured(3);
                        sessionsWindowClone->setShortTitle(dir);
                    #if defined(Q_OS_WIN)
                        QString sysUsername = qEnvironmentVariable("USERNAME");
                        QString sysHostname = qEnvironmentVariable("COMPUTERNAME");
                    #else
                        QString sysUsername = qEnvironmentVariable("USER");
                        QString sysHostname = QHostInfo::localHostName();
                    #endif
                        if((username == sysUsername) && (hostname == sysHostname)) {
                            sessionsWindowClone->setWorkingDirectory(dir.replace("~",QDir::homePath()));
                        }
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

QMenu *MainWindow::createPopupMenu()
{
    QMenu *menu = new QMenu(this);
    menu->addAction(menuBarAction);
    menu->addAction(toolBarAction);
    menu->addAction(cmdWindowAction);
    menu->addAction(fullScreenAction);
    return menu;
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
