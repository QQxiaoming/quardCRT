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

#include "qtftp.h"
#include "qfonticon.h"
#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptions.h"
#include "commandwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QString dir, StartupUIMode mode, QLocale::Language lang, bool isDark, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , language(lang)
    , isDarkTheme(isDark) {
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName()+" - "+VERSION);

    /* Create the main UI */
    sessionManagerWidget = new SessionManagerWidget(this);
    ui->centralwidget->layout()->addWidget(sessionManagerWidget);
    sessionManagerWidget->setVisible(false);

    QSplitter *splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setHandleWidth(1);
    ui->centralwidget->layout()->addWidget(splitter);
    mainWidgetGroupList.append(new MainWidgetGroup(this));
    splitter->addWidget(mainWidgetGroupList.at(0)->splitter);
    mainWidgetGroupList.append(new MainWidgetGroup(this));
    splitter->addWidget(mainWidgetGroupList.at(1)->splitter);
    splitter->setSizes(QList<int>() << 1 << 0);
    
    quickConnectWindow = new QuickConnectWindow(this);
    quickConnectMainWidgetGroup = mainWidgetGroupList.at(0);

    startTftpSeverWindow = new StartTftpSeverWindow(this);
    tftpServer = new QTftp;

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
    connect(startTftpSeverWindow,&StartTftpSeverWindow::setTftpInfo,this,[=](int port, const QString &upDir, const QString &downDir){
        if(tftpServer->isRunning()) {
            tftpServer->stopServer();
        }
        tftpServer->setUpDir(upDir);
        tftpServer->setDownDir(downDir);
        tftpServer->setPort(port);
        tftpServer->startServer();
    });
    connect(tftpServer,&QTftp::serverRuning,this,[=](bool runing){
        startTFTPServerAction->setChecked(runing);
    });
    connect(tftpServer,&QTftp::error,this,[=](int error){
        switch(error) {
        case QTftp::BindError:
            QMessageBox::warning(this, tr("Warning"), tr("TFTP server bind error!"));
            break;
        case QTftp::FileError:
            QMessageBox::warning(this, tr("Warning"), tr("TFTP server file error!"));
            break;
        case QTftp::NetworkError:
            QMessageBox::warning(this, tr("Warning"), tr("TFTP server network error!"));
            break;
        default:
            break;
        }
    });
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        connect(mainWidgetGroup->sessionTab,&FancyTabWidget::tabAddRequested,this,[=](){
            cloneCurrentSession(mainWidgetGroup);
        });
        connect(mainWidgetGroup->sessionTab,&FancyTabWidget::tabCloseRequested,this,[=](int index){
            stopSession(mainWidgetGroup,index);
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::currentChanged,this,[=](int index){
            int currentTabNum = 0;
            foreach(MainWidgetGroup *group, mainWidgetGroupList) {
                currentTabNum += group->sessionTab->count();
            }
            setSessionClassActionEnable(currentTabNum != 0);
            if(index > 0) {
                QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->widget(index);
                foreach(SessionsWindow *sessionsWindow, sessionList) {
                    sessionsWindow->getTermWidget()->setScrollBarPosition(verticalScrollBarAction->isChecked()?QTermWidget::ScrollBarRight:QTermWidget::NoScrollBar);
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
        connect(mainWidgetGroup->sessionTab,&SessionTab::showContextMenu,this,[=](int index, const QPoint& position){
            QMenu *menu = new QMenu(this);
            if(index != -1) {
                QAction *closeAction = new QAction(QFontIcon::icon(QChar(0xf00d)),tr("Close"),this);
                menu->addAction(closeAction);
                connect(closeAction,&QAction::triggered,this,[=](){
                    stopSession(mainWidgetGroup,index);
                });
            } else {
                if(mainWidgetGroup->sessionTab->count() != 0) {
                    QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
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
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabBarDoubleClicked,this,[=](int index){
            QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->widget(index);
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    sessionsWindow->setShowShortTitle(!sessionsWindow->getShowShortTitle());
                    mainWidgetGroup->sessionTab->setTabText(index,sessionsWindow->getTitle());
                    break;
                }
            }
        });
        connect(mainWidgetGroup->commandWindow, &CommandWindow::sendData, this, [=](const QByteArray &data) {
            if(mainWidgetGroup->sessionTab->count() != 0) {
                QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                termWidget->proxySendData(data);
            }
        });
    }

    connect(ui->statusBar,&QStatusBar::messageChanged,this,[&](const QString &message){
        if(message.isEmpty()) {
            ui->statusBar->showMessage(tr("Ready"));
        } else {
            ui->statusBar->showMessage(message);
        }
    });

    shortcutMenuBarView = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_U),this);
    connect(shortcutMenuBarView,&QShortcut::activated,this,[=](){
        menuBarAction->trigger();
    });
    shortcutMenuBarView->setEnabled(false);
    shortcutConnectLocalShell = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_T),this);
    connect(shortcutConnectLocalShell,&QShortcut::activated,this,[=](){
        connectLocalShellAction->trigger();
    });
    shortcutConnectLocalShell->setEnabled(false);
    shortcutCloneSession = new QShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_T),this);
    connect(shortcutCloneSession,&QShortcut::activated,this,[=](){
        cloneSessionAction->trigger();
    });
    shortcutCloneSession->setEnabled(false);
    shortcutMiniModeSwitch = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_J),this);
    connect(shortcutMiniModeSwitch,&QShortcut::activated,this,[=](){
        if(ui->menuBar->isVisible() == true) menuBarAction->trigger();
        if(ui->toolBar->isVisible() == true) toolBarAction->trigger();
        if(ui->statusBar->isVisible() == true) statusBarAction->trigger();
        if(ui->sidewidget->isVisible() == true) sideWindowAction->trigger();
        int currentTab = 0;
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            currentTab += mainWidgetGroup->sessionTab->count();
        }
        if(currentTab == 0) connectLocalShellAction->trigger();
    });
    shortcutStdModeSwitch = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_N),this);
    connect(shortcutStdModeSwitch,&QShortcut::activated,this,[=](){
        if(ui->menuBar->isVisible() == false) menuBarAction->trigger();
        if(ui->toolBar->isVisible() == false) toolBarAction->trigger();
        if(ui->statusBar->isVisible() == false) statusBarAction->trigger();
        if(ui->sidewidget->isVisible() == false) sideWindowAction->trigger();
    });

    ui->statusBar->showMessage(tr("Ready"));

    if(!dir.isEmpty()) {
        QFileInfo fileInfo(dir);
        if(!fileInfo.isDir()) {
            dir = "";
        }
    }
    if(mode == MINIUI_MODE) {
        menuBarAction->trigger();
        toolBarAction->trigger();
        statusBarAction->trigger();
        sideWindowAction->trigger();
        if(dir.isEmpty())
            connectLocalShellAction->trigger();
    }
    if(!dir.isEmpty()) {
        startLocalShellSession(mainWidgetGroupList.at(0),QString(),dir);
    }
}

MainWindow::~MainWindow() {
    delete tftpServer;
    delete sessionManagerPushButton;
    delete ui;
}

MainWidgetGroup* MainWindow::findCurrentFocusGroup(void) {
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        if(mainWidgetGroup->sessionTab->currentWidget()->hasFocus()) {
            return mainWidgetGroup;
        }
    }
    return mainWidgetGroupList[0];
}

QTermWidget * MainWindow::findCurrentFocusTermWidget(void) {
    SessionTab *sessionTab = findCurrentFocusGroup()->sessionTab;
    if(sessionTab->count() == 0) return nullptr;
    QTermWidget *termWidget = (QTermWidget *)sessionTab->currentWidget();
    return termWidget;
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
    clearScrollbackAction->setText(tr("Clear Scrollback"));
    clearScrollbackAction->setStatusTip(tr("Clear the contents of the scrollback rows"));
    clearScreenAction->setText(tr("Clear Screen"));
    clearScreenAction->setStatusTip(tr("Clear the contents of the current screen"));
    clearScreenAndScrollbackAction->setText(tr("Clear Screen and Scrollback"));
    clearScreenAndScrollbackAction->setStatusTip(tr("Clear the contents of the screen and scrollback"));
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
    connectBarAction->setText(tr("Connect Bar"));
    connectBarAction->setStatusTip(tr("Show/Hide Connect Bar"));
    sideWindowAction->setText(tr("Side Window"));
    sideWindowAction->setStatusTip(tr("Show/Hide Side Window"));
    verticalScrollBarAction->setText(tr("Vertical Scroll Bar"));
    verticalScrollBarAction->setStatusTip(tr("Show/Hide Vertical Scroll Bar"));
    allwaysOnTopAction->setText(tr("Allways On Top"));
    allwaysOnTopAction->setStatusTip(tr("Show window always on top"));
    fullScreenAction->setText(tr("Full Screen"));
    fullScreenAction->setStatusTip(tr("Toggle between full screen and normal mode <Alt+Enter>"));
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
    connectAddressEditAction = ui->toolBar->addWidget(connectAddressEdit);
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
    clearScrollbackAction = new QAction(this);
    editMenu->addAction(clearScrollbackAction);
    clearScreenAction = new QAction(this);
    editMenu->addAction(clearScreenAction);
    clearScreenAndScrollbackAction = new QAction(this);
    editMenu->addAction(clearScreenAndScrollbackAction);
    editMenu->addSeparator();
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
    connectBarAction = new QAction(this);
    connectBarAction->setCheckable(true);
    connectBarAction->setChecked(true);
    viewMenu->addAction(connectBarAction);
    sideWindowAction = new QAction(this);
    sideWindowAction->setCheckable(true);
    sideWindowAction->setChecked(true);
    viewMenu->addAction(sideWindowAction);
    viewMenu->addSeparator();
    verticalScrollBarAction = new QAction(this);
    verticalScrollBarAction->setCheckable(true);
    verticalScrollBarAction->setChecked(true);
    viewMenu->addAction(verticalScrollBarAction);
    viewMenu->addSeparator();
    allwaysOnTopAction = new QAction(this);
    allwaysOnTopAction->setCheckable(true);
    viewMenu->addAction(allwaysOnTopAction);
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
    startTFTPServerAction->setCheckable(true);
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

    setSessionClassActionEnable(false);
}

void MainWindow::setSessionClassActionEnable(bool enable)
{
    reconnectAction->setEnabled(enable);
    reconnectAllAction->setEnabled(enable);
    disconnectAction->setEnabled(enable);
    disconnectAllAction->setEnabled(enable);
    cloneSessionAction->setEnabled(enable);
    lockSessionAction->setEnabled(enable);
    logSessionAction->setEnabled(enable);
    rawLogSessionAction->setEnabled(enable);
    sendASCIIAction->setEnabled(enable);
    receiveASCIIAction->setEnabled(enable);
    sendBinaryAction->setEnabled(enable);
    sendXmodemAction->setEnabled(enable);
    receiveXmodemAction->setEnabled(enable);
    sendYmodemAction->setEnabled(enable);
    receiveYmodemAction->setEnabled(enable);
    zmodemUploadListAction->setEnabled(enable);
    startZmodemUploadAction->setEnabled(enable);
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
        quickConnectMainWidgetGroup = findCurrentFocusGroup();
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
            startTelnetSession(quickConnectMainWidgetGroup,data.TelnetData.hostname,data.TelnetData.port,type);
        } else if(data.type == QuickConnectWindow::Serial) {
            startSerialSession(quickConnectMainWidgetGroup,
                        data.SerialData.portName,data.SerialData.baudRate,
                        data.SerialData.dataBits,data.SerialData.parity,
                        data.SerialData.stopBits,data.SerialData.flowControl,
                        data.SerialData.xEnable);
        } else if(data.type == QuickConnectWindow::LocalShell) {
            startLocalShellSession(quickConnectMainWidgetGroup,data.LocalShellData.command);
        } else if(data.type == QuickConnectWindow::Raw) {
            startRawSocketSession(quickConnectMainWidgetGroup,data.RawData.hostname,data.RawData.port);
        }
    });
    connect(connectInTabAction,&QAction::triggered,this,[=](){
        sessionManagerWidget->setVisible(true);
    });
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        startLocalShellSession(findCurrentFocusGroup());
    });
    connect(disconnectAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        stopSession(findCurrentFocusGroup(),findCurrentFocusGroup()->sessionTab->indexOf(termWidget));
    });
    connect(disconnectAllAction,&QAction::triggered,this,[=](){
        stopAllSession();
    });
    connect(cloneSessionAction,&QAction::triggered,this,[=](){
        cloneCurrentSession(findCurrentFocusGroup());
    });
    connect(logSessionAction,&QAction::triggered,this,
        [&](void) {
            QTermWidget *termWidget = findCurrentFocusTermWidget();
            if(termWidget == nullptr) {
                logSessionAction->setChecked(false);
                return;
            }
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(!sessionsWindow->isLog()) {
                        logSessionAction->setChecked(sessionsWindow->setLog(true) == 0);
                    } else {
                        logSessionAction->setChecked(sessionsWindow->setLog(false) != 0);
                    }
                    break;
                }
            }
        }
    );
    connect(rawLogSessionAction,&QAction::triggered,this,
        [&](void) {
            QTermWidget *termWidget = findCurrentFocusTermWidget();
            if(termWidget == nullptr) {
                rawLogSessionAction->setChecked(false);
                return;
            }
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(!sessionsWindow->isRawLog()) {
                        rawLogSessionAction->setChecked(sessionsWindow->setRawLog(true) == 0);
                    } else {
                        rawLogSessionAction->setChecked(sessionsWindow->setRawLog(false) != 0);
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
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->getTermWidget()->setColorScheme(colorScheme);
        }
    });
    connect(keyMapManagerWindow,&keyMapManager::keyBindingChanged,this,[=](QString keyBinding){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->getTermWidget()->setKeyBindings(keyBinding);
        }
    });

    connect(copyAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->copyClipboard();
    });
    connect(pasteAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->pasteClipboard();
    });
    connect(copyAndPasteAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->copyClipboard();
        termWidget->pasteClipboard();
    });
    connect(selectAllAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->selectAll();
    });
    connect(findAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->toggleShowSearchBar();
    });
    connect(clearScrollbackAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->clearScrollback();
    });
    connect(clearScreenAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->clearScreen();
    });
    connect(clearScreenAndScrollbackAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->clear();
    });
    connect(resetAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->clear();
    });
    connect(zoomInAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->zoomIn();
    });
    connect(zoomOutAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->zoomOut();
    });
    connect(zoomResetAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        termWidget->setTerminalFont(globalOptionsWindow->getCurrentFont());
    });
    connect(menuBarAction,&QAction::triggered,this,[=](bool checked){
        ui->menuBar->setVisible(checked);
        if(ui->menuBar->isVisible() == false) {
            shortcutMenuBarView->setEnabled(true);
            shortcutCloneSession->setEnabled(true);
        } else {
            shortcutMenuBarView->setEnabled(false);
            shortcutCloneSession->setEnabled(false);
        }
        if(ui->menuBar->isVisible() || ui->toolBar->isVisible()) {
            shortcutConnectLocalShell->setEnabled(false);
        } else {
            shortcutConnectLocalShell->setEnabled(true);
        }
    });
    connect(toolBarAction,&QAction::triggered,this,[=](bool checked){
        ui->toolBar->setVisible(checked);
        if(ui->menuBar->isVisible() || ui->toolBar->isVisible()) {
            shortcutConnectLocalShell->setEnabled(false);
        } else {
            shortcutConnectLocalShell->setEnabled(true);
        }
    });
    connect(statusBarAction,&QAction::triggered,this,[=](bool checked){
        ui->statusBar->setVisible(checked);
    });
    connect(cmdWindowAction,&QAction::triggered,this,[=](bool checked){
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            mainWidgetGroup->commandWindow->setVisible(checked);
        }
    });
    connect(connectBarAction,&QAction::triggered,this,[=](bool checked){
        connectAddressEditAction->setVisible(checked);
    });
    connect(sideWindowAction,&QAction::triggered,this,[=](bool checked){
        ui->sidewidget->setVisible(checked);
    });
    connect(verticalScrollBarAction,&QAction::triggered,this,[=](bool checked){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->getTermWidget()->setScrollBarPosition(checked?QTermWidget::ScrollBarRight:QTermWidget::NoScrollBar);
        }
    });
    connect(allwaysOnTopAction,&QAction::triggered,this,[=](bool checked){
        if(checked) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        } else {
            setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        }
        show();
    });
    connect(fullScreenAction,&QAction::triggered,this,[=](bool checked){
        if(checked) {
            this->showFullScreen();
        } else {
            this->showNormal();
        }
    });
    connect(startTFTPServerAction,&QAction::triggered,this,[=](){
        startTftpSeverWindow->show();
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
        sessionManagerWidget->retranslateUi();
        startTftpSeverWindow->retranslateUi();
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            mainWidgetGroup->sessionTab->retranslateUi();
            mainWidgetGroup->commandWindow->retranslateUi();
        }
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

SessionsWindow *MainWindow::startTelnetSession(MainWidgetGroup *group, QString hostname, quint16 port, QTelnet::SocketType type)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
    sessionsWindow->setLongTitle(tr("Telnet - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Telnet"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
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
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return sessionsWindow;
}

SessionsWindow *MainWindow::startSerialSession(MainWidgetGroup *group, QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Serial,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
    sessionsWindow->setLongTitle(tr("Serial - ")+portName);
    sessionsWindow->setShortTitle(tr("Serial"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
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
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return sessionsWindow;
}

SessionsWindow *MainWindow::startRawSocketSession(MainWidgetGroup *group, QString hostname, quint16 port)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
    sessionsWindow->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    sessionsWindow->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    sessionsWindow->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    sessionsWindow->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
    sessionsWindow->setLongTitle(tr("Raw - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Raw"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
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
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return sessionsWindow;
}

SessionsWindow *MainWindow::startLocalShellSession(MainWidgetGroup *group, const QString &command, const QString &workingDirectory)
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
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    QString name = "Local Shell";
    for(uint32_t i=0;i<UINT_MAX;i++) {
        if(sessionManagerWidget->checkSession(name) == false) {
            break;
        }
        name = "Local Shell ("+QString::number(i)+")";
    }
    sessionManagerWidget->addSession(name,SessionsWindow::LocalShell);
    sessionsWindow->setName(name);
    sessionsWindow->setWorkingDirectory(workingDirectory);
    sessionsWindow->startLocalShellSession(command);
    sessionList.push_back(sessionsWindow);
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
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return sessionsWindow;
}

int MainWindow::stopSession(MainWidgetGroup *group, int index)
{
    QTermWidget *termWidget = (QTermWidget *)group->sessionTab->widget(index);
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getTermWidget() == termWidget) {
            sessionManagerWidget->removeSession(sessionsWindow->getName());
            sessionList.removeOne(sessionsWindow);
            group->sessionTab->removeTab(index);
            delete sessionsWindow;
            return 0;
        }
    }
    return -1;
}

int MainWindow::stopAllSession(void)
{
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        while(mainWidgetGroup->sessionTab->count() > 0) {
            stopSession(mainWidgetGroup,mainWidgetGroup->sessionTab->count());
        }
    }
    return 0;
}

int MainWindow::cloneCurrentSession(MainWidgetGroup *group)
{
    if(group->sessionTab->count() == 0) return -1;
    QTermWidget *termWidget = (QTermWidget *)group->sessionTab->currentWidget();
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getTermWidget() == termWidget) {
            SessionsWindow *sessionsWindowClone = new SessionsWindow(sessionsWindow->getSessionType(),this);
            sessionsWindowClone->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
            sessionsWindowClone->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
            sessionsWindowClone->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
            sessionsWindowClone->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getTransparency());
            sessionsWindowClone->setLongTitle(sessionsWindow->getLongTitle());
            sessionsWindowClone->setShortTitle(sessionsWindow->getShortTitle());
            sessionsWindowClone->setShowShortTitle(sessionsWindow->getShowShortTitle());
            group->sessionTab->addTab(sessionsWindowClone->getTermWidget(), group->sessionTab->tabText(group->sessionTab->indexOf(termWidget)));
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
            sessionList.push_back(sessionsWindowClone);
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
                    group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindowClone->getTermWidget()), sessionsWindowClone->getTitle());
                }
            });
            group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
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
