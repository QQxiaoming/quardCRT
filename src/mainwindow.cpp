/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
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
#include <QDesktopServices>
#include <QClipboard>
#include <QInputDialog>
#include <QPrinter>
#include <QPrintDialog>

#include "filedialog.h"
#include "qtftp.h"
#include "qfonticon.h"
#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptionswindow.h"
#include "commandwidget.h"
#include "mainwindow.h"
#include "globalsetting.h"
#include "sessionoptionswindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QString dir, StartupUIMode mode, QLocale::Language lang, bool isDark, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , windowTransparency(1.0)
    , windowTransparencyEnabled(false)
    , language(lang)
    , isDarkTheme(isDark) {
    ui->setupUi(this);

    restoreSettings();
    ui->toolBar->setVisible(true);

    setWindowTitle(QApplication::applicationName()+" - "+VERSION);

    /* Create the main UI */
    sessionManagerWidget = new SessionManagerWidget(this);
    ui->centralwidget->layout()->addWidget(sessionManagerWidget);
    restoreSessionToSessionManager();
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

    lockSessionWindow = new LockSessionWindow(this);

    startTftpSeverWindow = new StartTftpSeverWindow(this);
    tftpServer = new QTftp;

    keyMapManagerWindow = new keyMapManager(this);
    keyMapManagerWindow->setAvailableKeyBindings(QTermWidget::availableKeyBindings());

    globalOptionsWindow = new GlobalOptionsWindow(this);
    globalOptionsWindow->setAvailableColorSchemes(QTermWidget::availableColorSchemes());

    sessionOptionsWindow = new SessionOptionsWindow(this);

    hexViewWindow = new HexViewWindow(HexViewWindow::RECV);
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
                        lockSessionAction->setChecked(sessionsWindow->isLocked());
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
            if(index >= 0) {
                if(mainWidgetGroup->sessionTab->currentIndex() != index) {
                    delete menu;
                    return;
                }
                QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                foreach(SessionsWindow *sessionsWindow, sessionList) {
                    if(sessionsWindow->getTermWidget() == termWidget) {
                        if(sessionsWindow->isLocked()) {
                            delete menu;
                            return;
                        }
                        break;
                    }
                }
                QAction *moveToAnotherTabAction = new QAction(tr("Move to another Tab"),this);
                menu->addAction(moveToAnotherTabAction);
                connect(moveToAnotherTabAction,&QAction::triggered,this,[=](){
                    auto moveToAnotherTab = [&](int src,int dst, int index) {
                        mainWidgetGroupList.at(dst)->sessionTab->addTab(
                            mainWidgetGroupList.at(src)->sessionTab->widget(index),
                            mainWidgetGroupList.at(src)->sessionTab->tabText(index));
                        mainWidgetGroupList.at(src)->sessionTab->removeTab(index);
                        mainWidgetGroupList.at(dst)->sessionTab->setCurrentIndex(
                            mainWidgetGroupList.at(dst)->sessionTab->count()-1);
                        mainWidgetGroupList.at(src)->sessionTab->setCurrentIndex(
                            mainWidgetGroupList.at(src)->sessionTab->count()-1);
                    };
                    if(mainWidgetGroup == mainWidgetGroupList.at(0)) {
                        moveToAnotherTab(0,1,index);
                    } else {
                        moveToAnotherTab(1,0,index);
                    }
                });
                QAction *openWorkingFolderAction = new QAction(tr("Open Working Folder"),this);
                menu->addAction(openWorkingFolderAction);
                connect(openWorkingFolderAction,&QAction::triggered,this,[=](){
                    QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                    foreach(SessionsWindow *sessionsWindow, sessionList) {
                        if(sessionsWindow->getTermWidget() == termWidget) {
                            QString dir = sessionsWindow->getWorkingDirectory();
                            if(!dir.isEmpty()) {
                                QFileInfo fileInfo(dir);
                                if(fileInfo.isDir()) {
                                    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
                                    return;
                                }
                            }
                        }
                    }
                    QMessageBox::warning(this, tr("Warning"), tr("No working folder!"));
                });
                QAction *copyPathAction = new QAction(tr("Copy Path"),this);
                menu->addAction(copyPathAction);
                connect(copyPathAction,&QAction::triggered,this,[=](){
                    QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                    foreach(SessionsWindow *sessionsWindow, sessionList) {
                        if(sessionsWindow->getTermWidget() == termWidget) {
                            QString dir = sessionsWindow->getWorkingDirectory();
                            if(!dir.isEmpty()) {
                                QFileInfo fileInfo(dir);
                                if(fileInfo.isDir()) {
                                    QApplication::clipboard()->setText(dir);
                                    return;
                                }
                            }
                        }
                    }
                    QMessageBox::warning(this, tr("Warning"), tr("No working folder!"));
                });
                QAction *addPathToBookmarkAction = new QAction(tr("Add Path to Bookmark"),this);
                menu->addAction(addPathToBookmarkAction);
                connect(addPathToBookmarkAction,&QAction::triggered,this,[=](){
                    QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                    foreach(SessionsWindow *sessionsWindow, sessionList) {
                        if(sessionsWindow->getTermWidget() == termWidget) {
                            QString dir = sessionsWindow->getWorkingDirectory();
                            if(!dir.isEmpty()) {
                                QFileInfo fileInfo(dir);
                                if(fileInfo.isDir()) {
                                    addBookmark(dir);
                                    return;
                                }
                            }
                        }
                    }
                    QMessageBox::warning(this, tr("Warning"), tr("No working folder!"));
                });
                QAction *saveSessionAction = new QAction(tr("Save Session"),this);
                menu->addAction(saveSessionAction);
                connect(saveSessionAction,&QAction::triggered,this,[=](){
                    QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                    foreach(SessionsWindow *sessionsWindow, sessionList) {
                        if(sessionsWindow->getTermWidget() == termWidget) {
                            QString name = sessionsWindow->getName();
                            if(checkSessionName(name)) {
                                addSessionToSessionManager(sessionsWindow,name);
                            } else {
                                bool isOK = false;
                                QString getName = QInputDialog::getText(this, tr("Enter Session Name"),
                                     tr("The session already exists, please rename the new session or cancel saving."), QLineEdit::Normal, name, &isOK);
                                if(isOK && !getName.isEmpty()) {
                                    addSessionToSessionManager(sessionsWindow,getName);
                                }
                            }
                            break;
                        }
                    }
                });
                menu->addSeparator();
                QAction *closeAction = new QAction(QFontIcon::icon(QChar(0xf00d)),tr("Close"),this);
                menu->addAction(closeAction);
                connect(closeAction,&QAction::triggered,this,[=](){
                    stopSession(mainWidgetGroup,index);
                });
            } else if(index == -1) {
                if(mainWidgetGroup->sessionTab->count() != 0) {
                    QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                    foreach(SessionsWindow *sessionsWindow, sessionList) {
                        if(sessionsWindow->getTermWidget() == termWidget) {
                            if(sessionsWindow->isLocked()) {
                                delete menu;
                                return;
                            }
                            break;
                        }
                    }
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
            } else if(index == -2) {
                menu->addAction(menuBarAction);
                menu->addAction(toolBarAction);
                menu->addAction(cmdWindowAction);
                menu->addAction(fullScreenAction);
            } else {
                delete menu;
                return;
            }
            menu->move(cursor().pos());
            menu->show();
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabBarDoubleClicked,this,[=](int index){
            QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->widget(index);
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(sessionsWindow->isLocked()) {
                        return;
                    }
                    sessionsWindow->setShowShortTitle(!sessionsWindow->getShowShortTitle());
                    mainWidgetGroup->sessionTab->setTabText(index,sessionsWindow->getTitle());
                    break;
                }
            }
        });
        connect(mainWidgetGroup->commandWidget, &CommandWidget::sendData, this, [=](const QByteArray &data) {
            if(mainWidgetGroup->sessionTab->count() != 0) {
                QTermWidget *termWidget = (QTermWidget *)mainWidgetGroup->sessionTab->currentWidget();
                foreach(SessionsWindow *sessionsWindow, sessionList) {
                    if(sessionsWindow->getTermWidget() == termWidget) {
                        if(sessionsWindow->isLocked()) {
                            return;
                        }
                        break;
                    }
                }
                termWidget->proxySendData(data);
            }
        });
    }
    connect(sessionManagerWidget,&SessionManagerWidget::sessionConnect,this,[=](QString str){
        connectSessionFromSessionManager(str);
    });
    connect(sessionManagerWidget,&SessionManagerWidget::sessionRemove,this,[=](QString str){
        removeSessionFromSessionManager(str);
    });
    connect(sessionManagerWidget,&SessionManagerWidget::sessionShowProperties,this,[=](QString str){
        QuickConnectWindow::QuickConnectData data;
        GlobalSetting settings;
        int size = settings.beginReadArray("Global/Session");
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            QString current_name = settings.value("name").toString();
            if(current_name == str) {
                data.type = (QuickConnectWindow::QuickConnectType)(settings.value("type").toInt());
                switch(data.type) {
                case QuickConnectWindow::Telnet:
                    data.TelnetData.hostname = settings.value("hostname").toString();
                    data.TelnetData.port = settings.value("port").toInt();
                    data.TelnetData.webSocket = settings.value("socketType").toString();
                    break;
                case QuickConnectWindow::Serial:
                    data.SerialData.portName = settings.value("portName").toString();
                    data.SerialData.baudRate = settings.value("baudRate").toInt();
                    data.SerialData.dataBits = settings.value("dataBits").toInt();
                    data.SerialData.parity = settings.value("parity").toInt();
                    data.SerialData.stopBits = settings.value("stopBits").toInt();
                    data.SerialData.flowControl = settings.value("flowControl").toBool();
                    data.SerialData.xEnable = settings.value("xEnable").toBool();
                    break;
                case QuickConnectWindow::LocalShell:
                    data.LocalShellData.command = settings.value("command").toString();
                    break;
                case QuickConnectWindow::Raw:
                    data.RawData.hostname = settings.value("hostname").toString();
                    data.RawData.port = settings.value("port").toInt();
                    break;
                case QuickConnectWindow::NamePipe:
                    data.NamePipeData.pipeName = settings.value("pipeName").toString();
                    break;
                case QuickConnectWindow::SSH2:
                default:
                    break;
                }
                sessionOptionsWindow->setSessionProperties(str,data);
                sessionOptionsWindow->show();
                break;
            }
        }
    });

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
    shortcutTabPlusSwitch = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_Equal),this);
    connect(shortcutTabPlusSwitch,&QShortcut::activated,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        MainWidgetGroup *group = findCurrentFocusGroup();
        int index = group->sessionTab->indexOf(termWidget);
        if(index < group->sessionTab->count()) {
            group->sessionTab->setCurrentIndex(index);
        } else {
            group->sessionTab->setCurrentIndex(0);
        }
        group->sessionTab->currentWidget()->setFocus();
    });
    shortcutTabMinusSwitch = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_Minus),this);
    connect(shortcutTabMinusSwitch,&QShortcut::activated,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        MainWidgetGroup *group = findCurrentFocusGroup();
        int index = group->sessionTab->indexOf(termWidget) - 2;
        if(index >= 0) {
            group->sessionTab->setCurrentIndex(index);
        } else {
            group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
        }
        group->sessionTab->currentWidget()->setFocus();
    });
    for(int i=0;i<9;i++) {
        shortcutTabSwitch[i] = new QShortcut(QKeySequence(Qt::ALT|(Qt::Key_1+i)),this);
        connect(shortcutTabSwitch[i],&QShortcut::activated,this,[=](){
            MainWidgetGroup *group = findCurrentFocusGroup();
            group->sessionTab->setCurrentIndex(i);
            group->sessionTab->currentWidget()->setFocus();
        });
    }

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

    // TODO:Unimplemented functions are temporarily closed
    sendASCIIAction->setEnabled(false);
    receiveASCIIAction->setEnabled(false);
    sendBinaryAction->setEnabled(false);
    sendXmodemAction->setEnabled(false);
    receiveXmodemAction->setEnabled(false);
    sendYmodemAction->setEnabled(false);
    receiveYmodemAction->setEnabled(false);
    zmodemUploadListAction->setEnabled(false);
    startZmodemUploadAction->setEnabled(false);
    runAction->setEnabled(false);
    cancelAction->setEnabled(false);
    startRecordingScriptAction->setEnabled(false);
    stopRecordingScriptAction->setEnabled(false);
    canlcelRecordingScriptAction->setEnabled(false);
    createPublicKeyAction->setEnabled(false);
    publickeyManagerAction->setEnabled(false);
    tileAction->setEnabled(false);
    cascadeAction->setEnabled(false);
}

MainWindow::~MainWindow() {
    saveSettings();
    if(tftpServer->isRunning()) {
        tftpServer->stopServer();
    }
    delete tftpServer;
    delete sessionManagerPushButton;
    delete hexViewWindow;
    delete ui;
}

void MainWindow::saveSettings(void) {
    GlobalSetting settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
}

void MainWindow::restoreSettings(void) {
    GlobalSetting settings;
    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    restoreState(settings.value("MainWindow/State").toByteArray());
}

MainWidgetGroup* MainWindow::findCurrentFocusGroup(void) {
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        if(mainWidgetGroup->sessionTab->currentWidget()->hasFocus()) {
            return mainWidgetGroup;
        }
    }
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        if(mainWidgetGroup->sessionTab->count() != 0) {
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
    bookmarkMenu->setTitle(tr("Bookmark"));
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
    screenShotAction->setText(tr("Screen Shot"));
    screenShotAction->setStatusTip(tr("Screen shot current screen <Alt+P>"));
    screenShotAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_P));
    sessionExportAction->setText(tr("Session Export"));
    sessionExportAction->setStatusTip(tr("Export current session to a file <Alt+O>"));
    sessionExportAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_O));
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
    windwosTransparencyAction->setText(tr("Windows Transparency"));
    windwosTransparencyAction->setStatusTip(tr("Enable/Disable alpha transparency"));
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
    realTimeSaveOptionsAction->setText(tr("Real-time Save Options"));
    realTimeSaveOptionsAction->setStatusTip(tr("Real-time save session options and global options"));
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

    addBookmarkAction->setText(tr("Add Bookmark"));
    addBookmarkAction->setStatusTip(tr("Add a bookmark"));
    removeBookmarkAction->setText(tr("Remove Bookmark"));
    removeBookmarkAction->setStatusTip(tr("Remove a bookmark"));
    cleanAllBookmarkAction->setText(tr("Clean All Bookmark"));
    cleanAllBookmarkAction->setStatusTip(tr("Clean all bookmark"));

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
    GlobalSetting settings;

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
    bookmarkMenu = new QMenu(this);
    ui->menuBar->addMenu(bookmarkMenu);
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
    lockSessionAction->setCheckable(true);
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
    ui->toolBar->addAction(printScreenAction);
    screenShotAction = new QAction(this);
    editMenu->addAction(screenShotAction);
    sessionExportAction = new QAction(this);
    editMenu->addAction(sessionExportAction);
    editMenu->addSeparator();
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
    windwosTransparencyAction = new QAction(this);
    windwosTransparencyAction->setCheckable(true);
    viewMenu->addAction(windwosTransparencyAction);
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
    realTimeSaveOptionsAction = new QAction(this);
    realTimeSaveOptionsAction->setCheckable(true);
    bool checked = settings.value("Global/Options/AutoSaveOptions",false).toBool();
    realTimeSaveOptionsAction->setChecked(checked);
    optionsMenu->addAction(realTimeSaveOptionsAction);
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

    addBookmarkAction = new QAction(this);
    bookmarkMenu->addAction(addBookmarkAction);
    removeBookmarkAction = new QAction(this);
    bookmarkMenu->addAction(removeBookmarkAction);
    cleanAllBookmarkAction = new QAction(this);
    bookmarkMenu->addAction(cleanAllBookmarkAction);
    bookmarkMenu->addSeparator();
    int size = settings.beginReadArray("Global/Bookmark");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString path = settings.value("path").toString();
        QAction *action = new QAction(path,bookmarkMenu);
        action->setStatusTip(path);
        bookmarkMenu->addAction(action);
        connect(action,&QAction::triggered,this,[=](){
            startLocalShellSession(findCurrentFocusGroup(),QString(),path);
        });
    }
    settings.endArray();

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

    copyAction->setEnabled(enable);
    pasteAction->setEnabled(enable);
    copyAndPasteAction->setEnabled(enable);
    selectAllAction->setEnabled(enable);
    findAction->setEnabled(enable);
    printScreenAction->setEnabled(enable);
    screenShotAction->setEnabled(enable);
    sessionExportAction->setEnabled(enable);
    clearScrollbackAction->setEnabled(enable);
    clearScreenAction->setEnabled(enable);
    clearScreenAndScrollbackAction->setEnabled(enable);
    resetAction->setEnabled(enable);

    zoomInAction->setEnabled(enable);
    zoomOutAction->setEnabled(enable);
    zoomResetAction->setEnabled(enable);

    sessionOptionsAction->setEnabled(enable);

    // TODO: these actions are not implemented yet
    //sendASCIIAction->setEnabled(enable);
    //receiveASCIIAction->setEnabled(enable);
    //sendBinaryAction->setEnabled(enable);
    //sendXmodemAction->setEnabled(enable);
    //receiveXmodemAction->setEnabled(enable);
    //sendYmodemAction->setEnabled(enable);
    //receiveYmodemAction->setEnabled(enable);
    //zmodemUploadListAction->setEnabled(enable);
    //startZmodemUploadAction->setEnabled(enable);
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
            QString name = data.TelnetData.hostname;
            if(data.openInTab) {
                name = startTelnetSession(quickConnectMainWidgetGroup,name,data.TelnetData.port,type);
            }
            if(data.saveSession) {
                addSessionToSessionManager(data,name, !data.openInTab);
            }
        } else if(data.type == QuickConnectWindow::Serial) {
            QString name = data.SerialData.portName;
            if(data.openInTab) {
                name = startSerialSession(quickConnectMainWidgetGroup,
                            name,data.SerialData.baudRate,
                            data.SerialData.dataBits,data.SerialData.parity,
                            data.SerialData.stopBits,data.SerialData.flowControl,
                            data.SerialData.xEnable);
            }
            if(data.saveSession) {
                addSessionToSessionManager(data,name, !data.openInTab);
            }
        } else if(data.type == QuickConnectWindow::LocalShell) {
            QString name = "Local Shell";
            if(data.openInTab) {
                name = startLocalShellSession(quickConnectMainWidgetGroup,data.LocalShellData.command,globalOptionsWindow->getNewTabWorkPath());
            }
            if(data.saveSession) {
                addSessionToSessionManager(data,name, !data.openInTab);
            }
        } else if(data.type == QuickConnectWindow::Raw) {
            QString name = data.RawData.hostname;
            if(data.openInTab) {
                name = startRawSocketSession(quickConnectMainWidgetGroup,name,data.RawData.port);
            }
            if(data.saveSession) {
                addSessionToSessionManager(data,name, !data.openInTab);
            }
        } else if(data.type == QuickConnectWindow::NamePipe) {
            QString name = data.NamePipeData.pipeName;
            if(data.openInTab) {
                name = startNamePipeSession(quickConnectMainWidgetGroup,name);
            }
            if(data.saveSession) {
                addSessionToSessionManager(data,name, !data.openInTab);
            } 
        } else if(data.type == QuickConnectWindow::SSH2) {
            QString name = data.SSH2Data.hostname;
            if(data.openInTab) {
                name = startSSH2Session(quickConnectMainWidgetGroup,name,data.SSH2Data.port,data.SSH2Data.username,data.SSH2Data.password);
            }
            if(data.saveSession) {
                addSessionToSessionManager(data,name, !data.openInTab);
            }
        }
    });
    connect(lockSessionWindow,&LockSessionWindow::sendLockSessionData,this,
        [=](QString password, bool lockAllSession, bool lockAllSessionGroup){
        if(lockAllSession) {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
            }
        } else if(lockAllSessionGroup) {
            SessionTab *sessionTab = findCurrentFocusGroup()->sessionTab;
            if(sessionTab->count() == 0) return;
            for(int i=0;i<sessionTab->count();i++) {
                QTermWidget *termWidget = (QTermWidget *)sessionTab->widget(i);
                foreach(SessionsWindow *sessionsWindow, sessionList) {
                    if(sessionsWindow->getTermWidget() == termWidget) {
                        if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
                    }
                }
            }
        } else {
            QTermWidget *termWidget = findCurrentFocusTermWidget();
            if(termWidget == nullptr) return;
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
                    break;
                }
            }
        }
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(sessionsWindow->getTermWidget() == termWidget) {
                if(sessionsWindow->isLocked()) {
                    lockSessionAction->setChecked(true);
                } else {
                    lockSessionAction->setChecked(false);
                }
                break;
            }
        }
    });
    connect(lockSessionWindow,&LockSessionWindow::sendUnLockSessionData,this,
        [=](QString password, bool unlockAllSession){
        if(unlockAllSession) {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->isLocked()) sessionsWindow->unlockSession(password);
            }
        } else {
            QTermWidget *termWidget = findCurrentFocusTermWidget();
            if(termWidget == nullptr) return;
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->getTermWidget() == termWidget) {
                    if(sessionsWindow->isLocked()) sessionsWindow->unlockSession(password);
                    break;
                }
            }
        }
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(sessionsWindow->getTermWidget() == termWidget) {
                if(sessionsWindow->isLocked()) {
                    lockSessionAction->setChecked(true);
                } else {
                    lockSessionAction->setChecked(false);
                }
                break;
            }
        }
    });

    connect(connectInTabAction,&QAction::triggered,this,[=](){
        sessionManagerWidget->setVisible(true);
    });
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        startLocalShellSession(findCurrentFocusGroup(),QString(),globalOptionsWindow->getNewTabWorkPath());
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
    connect(lockSessionAction,&QAction::triggered,this,[=](){
        if(lockSessionAction->isChecked()) {
            lockSessionWindow->showLock();
            lockSessionAction->setChecked(false);
        } else {
            lockSessionWindow->showUnlock();
            lockSessionAction->setChecked(true);
        }
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
        if(hexViewAction->isChecked()) {
            hexViewWindow->show();
            QTermWidget *termWidget = findCurrentFocusTermWidget();
            if(termWidget == nullptr) return;
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                disconnect(sessionsWindow,SIGNAL(hexDataDup(const char*,int)),
                            hexViewWindow,SLOT(recvData(const char*,int)));
                if(sessionsWindow->getTermWidget() == termWidget) {
                    connect(sessionsWindow,SIGNAL(hexDataDup(const char*,int)),
                            hexViewWindow,SLOT(recvData(const char*,int)));
                }
            }
        }
        else
            hexViewWindow->hide();
    });
    connect(hexViewWindow,&HexViewWindow::hideOrClose,this,[=](){
        hexViewAction->setChecked(false);
    });
    connect(globalOptionsWindow,&GlobalOptionsWindow::colorSchemeChanged,this,[=](QString colorScheme){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->getTermWidget()->setColorScheme(colorScheme);
        }
    });
    connect(globalOptionsWindow,&GlobalOptionsWindow::transparencyChanged,this,[=](int transparency){
        windowTransparency = (100-transparency)/100.0;
        setWindowOpacity(windowTransparencyEnabled?windowTransparency:1.0);
    });
    connect(realTimeSaveOptionsAction,&QAction::triggered,this,[=](bool checked){
        GlobalSetting settings;
        settings.setValue("Global/Options/RealtimeSaveOptions",checked);
        settings.setRealtimeSave(checked);
    });
    connect(saveSettingsNowAction,&QAction::triggered,this,[=](){
        GlobalSetting settings;
        saveSettings();
        settings.sync();
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
    connect(printScreenAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;

        QPrinter printer;
    #if defined(Q_OS_LINUX)
        GlobalSetting settings;
        QString printerPDFPath = settings.value("Global/Options/PrinterPDFPath",QDir::homePath()).toString();
        QString willsaveName = printerPDFPath + "/quartCRT-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".pdf";
        printer.setPrinterName("QuartCRT");
        printer.setPageSize(QPageSize::A4);
        printer.setOutputFileName(willsaveName);
    #endif

        QPrintDialog dlg(&printer, this);
        if (dlg.exec() == QDialog::Accepted) {
            QString buffer;
            QTextStream out(&buffer);
            termWidget->saveHistory(&out,1);
            QTextDocument doc;
            doc.setHtml(buffer);
            doc.print(&printer);
        #if defined(Q_OS_LINUX)
            settings.setValue("Global/Options/PrinterPDFPath",QFileInfo(willsaveName).absolutePath());
            ui->statusBar->showMessage(tr("PrintScreen saved to %1").arg(willsaveName),3000);
        #endif
        }
    });
    connect(screenShotAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        GlobalSetting settings;
        QString screenShotPath = settings.value("Global/Options/ScreenShotPath",QDir::homePath()).toString();
        QString willsaveName = screenShotPath + "/quartCRT-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".jpg";
        QString fileName = FileDialog::getSaveFileName(this,tr("Save Screenshot"),willsaveName,tr("Image Files (*.jpg)"));
        if(fileName.isEmpty()) return;
        if(!fileName.endsWith(".jpg")) fileName.append(".jpg");
        termWidget->screenShot(fileName);
        settings.setValue("Global/Options/ScreenShotPath",QFileInfo(fileName).absolutePath());
        ui->statusBar->showMessage(tr("Screenshot saved to %1").arg(fileName),3000);
    });
    connect(sessionExportAction,&QAction::triggered,this,[=](){
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        GlobalSetting settings;
        QString screenShotPath = settings.value("Global/Options/SessionExportPath",QDir::homePath()).toString();
        QString willsaveName = screenShotPath + "/quartCRT-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        QString selectedFilter;
        QString fileName = FileDialog::getSaveFileName(this,tr("Save Session Export"),willsaveName,tr("Text Files (*.txt);;HTML Files (*.html)"),&selectedFilter);
        if(fileName.isEmpty()) return;
        int format = 0;
        if(selectedFilter == tr("Text Files (*.txt)")) {
            format = 0;
            if(!fileName.endsWith(".txt")) fileName.append(".txt");
        } else if(selectedFilter == tr("HTML Files (*.html)")) {
            format = 1;
            if(!fileName.endsWith(".html")) fileName.append(".html");
        }
        QFile file(fileName);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            termWidget->saveHistory(&file,format);
            file.close();
            settings.setValue("Global/Options/SessionExportPath",QFileInfo(fileName).absolutePath());
            ui->statusBar->showMessage(tr("Session Export saved to %1").arg(fileName),3000);
        } else {
            ui->statusBar->showMessage(tr("Session Export failed to save to %1").arg(fileName),3000);
        }
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
            mainWidgetGroup->commandWidget->setVisible(checked);
        }
    });
    connect(connectBarAction,&QAction::triggered,this,[=](bool checked){
        connectAddressEditAction->setVisible(checked);
    });
    connect(sideWindowAction,&QAction::triggered,this,[=](bool checked){
        ui->sidewidget->setVisible(checked);
    });
    connect(windwosTransparencyAction,&QAction::triggered,this,[=](bool checked){
        windowTransparencyEnabled = checked;
        setWindowOpacity(windowTransparencyEnabled?windowTransparency:1.0);
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
    connect(startTFTPServerAction,&QAction::triggered,this,[=](bool checked){
        if(checked) {
            startTftpSeverWindow->show();
        } else {
            if(tftpServer->isRunning()) {
                tftpServer->stopServer();
            }
        }
    });
    connect(addBookmarkAction, &QAction::triggered, this, [&]() {
        QString path = FileDialog::getExistingDirectory(this,tr("Select a directory"),QDir::homePath());
        if(path.isEmpty()) return;
        GlobalSetting settings;
        int size = settings.beginReadArray("Global/Bookmark");
        settings.endArray();
        settings.beginWriteArray("Global/Bookmark");
        settings.setArrayIndex(size);
        settings.setValue("path",path);
        settings.endArray();
        QAction *action = new QAction(path,bookmarkMenu);
        action->setStatusTip(path);
        bookmarkMenu->addAction(action);
        connect(action,&QAction::triggered,this,[=](){
            startLocalShellSession(findCurrentFocusGroup(),QString(),path);
        });
    });
    connect(removeBookmarkAction, &QAction::triggered, this, [&]() {
        QStringList bookmarkList;
        GlobalSetting settings;
        int size = settings.beginReadArray("Global/Bookmark");
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            bookmarkList.append(settings.value("path").toString());
        }
        settings.endArray();

        bool isOk = false;
        QString removePath = QInputDialog::getItem(this,tr("Remove Bookmark"),tr("Select a bookmark"),bookmarkList,0,false,&isOk);
        if(!isOk || removePath.isEmpty()) return;
        bookmarkList.removeOne(removePath);

        settings.beginWriteArray("Global/Bookmark");
        settings.remove("");
        settings.endArray();
        bookmarkMenu->clear();
        bookmarkMenu->addAction(addBookmarkAction);
        bookmarkMenu->addAction(removeBookmarkAction);
        bookmarkMenu->addAction(cleanAllBookmarkAction);
        bookmarkMenu->addSeparator();

        foreach(QString path, bookmarkList) {
            settings.beginWriteArray("Global/Bookmark");
            settings.setArrayIndex(bookmarkList.indexOf(path));
            settings.setValue("path",path);
            settings.endArray();
            QAction *action = new QAction(path,bookmarkMenu);
            action->setStatusTip(path);
            bookmarkMenu->addAction(action);
            connect(action,&QAction::triggered,this,[=](){
                startLocalShellSession(findCurrentFocusGroup(),QString(),path);
            });
        }
    });
    connect(cleanAllBookmarkAction, &QAction::triggered, this, [&]() {
        if(QMessageBox::question(this,tr("Clean All Bookmark"),tr("Are you sure to clean all bookmark?"),QMessageBox::Yes|QMessageBox::No) == QMessageBox::No) 
            return;
        
        GlobalSetting settings;
        settings.beginWriteArray("Global/Bookmark");
        settings.remove("");
        settings.endArray();
        bookmarkMenu->clear();
        bookmarkMenu->addAction(addBookmarkAction);
        bookmarkMenu->addAction(removeBookmarkAction);
        bookmarkMenu->addAction(cleanAllBookmarkAction);
        bookmarkMenu->addSeparator();
    });
    connect(keymapManagerAction,&QAction::triggered,this,[=](){
        keyMapManagerWindow->show();
    });
    connect(sessionOptionsAction,&QAction::triggered,this,[=](){
        QString name;
        QuickConnectWindow::QuickConnectData data;
        QTermWidget *termWidget = findCurrentFocusTermWidget();
        if(termWidget == nullptr) return;
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(sessionsWindow->getTermWidget() == termWidget) {
                name = sessionsWindow->getName();
                data.type = (QuickConnectWindow::QuickConnectType)sessionsWindow->getSessionType();
                switch(data.type) {
                    case QuickConnectWindow::QuickConnectType::Telnet:
                        data.TelnetData.hostname = sessionsWindow->m_hostname;
                        data.TelnetData.port = sessionsWindow->m_port;
                        if(sessionsWindow->m_type == QTelnet::TCP) {
                            data.TelnetData.webSocket = "None";
                        } else if(sessionsWindow->m_type == QTelnet::WEBSOCKET) {
                            data.TelnetData.webSocket = "Insecure";
                        } else if(sessionsWindow->m_type == QTelnet::SECUREWEBSOCKET) {
                            data.TelnetData.webSocket = "Secure";
                        }
                        break;
                    case QuickConnectWindow::QuickConnectType::Serial:
                        data.SerialData.portName = sessionsWindow->m_portName;
                        data.SerialData.baudRate = sessionsWindow->m_baudRate;
                        data.SerialData.dataBits = sessionsWindow->m_dataBits;
                        data.SerialData.parity = sessionsWindow->m_parity;
                        data.SerialData.stopBits = sessionsWindow->m_stopBits;
                        data.SerialData.flowControl = sessionsWindow->m_flowControl;
                        data.SerialData.xEnable = sessionsWindow->m_xEnable;
                        break;
                    case QuickConnectWindow::QuickConnectType::LocalShell:
                        data.LocalShellData.command = sessionsWindow->m_command;
                        break;
                    case QuickConnectWindow::QuickConnectType::Raw:
                        data.RawData.hostname = sessionsWindow->m_hostname;
                        data.RawData.port = sessionsWindow->m_port;
                        break;
                    case QuickConnectWindow::QuickConnectType::NamePipe:
                        data.NamePipeData.pipeName = sessionsWindow->m_pipeName;
                        break;
                }
                sessionOptionsWindow->setSessionProperties(name,data);
                sessionOptionsWindow->show();
                break;
            }
        }
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
            mainWidgetGroup->commandWidget->retranslateUi();
        }
        menuAndToolBarRetranslateUi();
        ui->statusBar->showMessage(tr("Ready"));
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
        GlobalSetting settings;
        settings.setValue("Global/Startup/dark_theme","false");
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
        GlobalSetting settings;
        settings.setValue("Global/Startup/dark_theme","true");
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

void MainWindow::setGlobalOptions(SessionsWindow *window) {
    window->getTermWidget()->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    window->getTermWidget()->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    window->getTermWidget()->setTerminalFont(globalOptionsWindow->getCurrentFont());
    window->getTermWidget()->setTerminalBackgroundImage(globalOptionsWindow->getBackgroundImage());
    window->getTermWidget()->setTerminalBackgroundMode(globalOptionsWindow->getBackgroundImageMode());
    window->getTermWidget()->setTerminalOpacity(globalOptionsWindow->getBackgroundImageOpacity());
    window->getTermWidget()->setHistorySize(globalOptionsWindow->getScrollbackLines());
}

void MainWindow::restoreSessionToSessionManager(void)
{
    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        int type = settings.value("type").toInt();
        sessionManagerWidget->addSession(name,type);
    }
    settings.endArray();
}

bool MainWindow::checkSessionName(QString &name)
{
    QString oldNmae = name;
    for(uint32_t i=0;i<UINT_MAX;i++) {
        if(sessionManagerWidget->checkSession(name) == false) {
            break;
        }
        name = oldNmae+" ("+QString::number(i)+")";
    }
    return oldNmae == name;
}

int MainWindow::addSessionToSessionManager(SessionsWindow *sessionsWindow, QString &name)
{
    checkSessionName(name);
    sessionManagerWidget->addSession(name,sessionsWindow->getSessionType());

    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    settings.setArrayIndex(size);
    settings.setValue("name",name);
    settings.setValue("type",sessionsWindow->getSessionType());
    switch(sessionsWindow->getSessionType()) {
    case SessionsWindow::Telnet:
        settings.setValue("hostname",sessionsWindow->m_hostname);
        settings.setValue("port",sessionsWindow->m_port);
        settings.setValue("socketType",[&]() {
            if(sessionsWindow->m_type == QTelnet::TCP) {
                return QString("None");
            } else if(sessionsWindow->m_type == QTelnet::WEBSOCKET) {
                return QString("Insecure");
            } else if(sessionsWindow->m_type == QTelnet::SECUREWEBSOCKET) {
                return QString("Secure");
            }
            return QString("None");
        }());
        break;
    case SessionsWindow::Serial:
        settings.setValue("portName",sessionsWindow->m_portName);
        settings.setValue("baudRate",sessionsWindow->m_baudRate);
        settings.setValue("dataBits",sessionsWindow->m_dataBits);
        settings.setValue("parity",sessionsWindow->m_parity);
        settings.setValue("stopBits",sessionsWindow->m_stopBits);
        settings.setValue("flowControl",sessionsWindow->m_flowControl);
        settings.setValue("xEnable",sessionsWindow->m_xEnable);
        break;
    case SessionsWindow::LocalShell:
        settings.setValue("command",sessionsWindow->m_command);
        break;
    case SessionsWindow::RawSocket:
        settings.setValue("hostname",sessionsWindow->m_hostname);
        settings.setValue("port",sessionsWindow->m_port);
        break;
    default:
        break;
    }
    settings.endArray();

    return 0;
}

int MainWindow::addSessionToSessionManager(const QuickConnectWindow::QuickConnectData &data, QString &name, bool checkname)
{
    if(checkname) checkSessionName(name);
    sessionManagerWidget->addSession(name,data.type);

    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    settings.setArrayIndex(size);
    settings.setValue("name",name);
    settings.setValue("type",data.type);
    switch(data.type) {
        case QuickConnectWindow::Telnet:
            settings.setValue("hostname",data.TelnetData.hostname);
            settings.setValue("port",data.TelnetData.port);
            settings.setValue("socketType",data.TelnetData.webSocket);
            break;
        case QuickConnectWindow::Serial:
            settings.setValue("portName",data.SerialData.portName);
            settings.setValue("baudRate",data.SerialData.baudRate);
            settings.setValue("dataBits",data.SerialData.dataBits);
            settings.setValue("parity",data.SerialData.parity);
            settings.setValue("stopBits",data.SerialData.stopBits);
            settings.setValue("flowControl",data.SerialData.flowControl);
            settings.setValue("xEnable",data.SerialData.xEnable);
            break;
        case QuickConnectWindow::LocalShell:
            settings.setValue("command",data.LocalShellData.command);
            break;
        case QuickConnectWindow::Raw:
            settings.setValue("hostname",data.RawData.hostname);
            settings.setValue("port",data.RawData.port);
            break;
        default:
            break;
    }
    settings.endArray();

    return 0;
}

void MainWindow::removeSessionFromSessionManager(QString name)
{
    sessionManagerWidget->removeSession(name);
    GlobalSetting settings;
    QMap<QString,QuickConnectWindow::QuickConnectData> infoMap;
    int size = settings.beginReadArray("Global/Session");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString current_name = settings.value("name").toString();
        if(current_name == name) {
            continue;
        }
        QuickConnectWindow::QuickConnectData data;
        data.type = (QuickConnectWindow::QuickConnectType)(settings.value("type").toInt());
        switch(data.type) {
        case QuickConnectWindow::Telnet:
            data.TelnetData.hostname = settings.value("hostname").toString();
            data.TelnetData.port = settings.value("port").toInt();
            data.TelnetData.webSocket = settings.value("socketType").toString();
            break;
        case QuickConnectWindow::Serial:
            data.SerialData.portName = settings.value("portName").toString();
            data.SerialData.baudRate = settings.value("baudRate").toInt();
            data.SerialData.dataBits = settings.value("dataBits").toInt();
            data.SerialData.parity = settings.value("parity").toInt();
            data.SerialData.stopBits = settings.value("stopBits").toInt();
            data.SerialData.flowControl = settings.value("flowControl").toBool();
            data.SerialData.xEnable = settings.value("xEnable").toBool();
            break;
        case QuickConnectWindow::LocalShell:
            data.LocalShellData.command = settings.value("command").toString();
            break;
        case QuickConnectWindow::Raw:
            data.RawData.hostname = settings.value("hostname").toString();
            data.RawData.port = settings.value("port").toInt();
            break;
        default:
            break;
        }
        infoMap.insert(current_name,data);
    }
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    settings.remove("");
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    // write infoMap to settings
    foreach(QString name, infoMap.keys()) {
        settings.setArrayIndex(infoMap.keys().indexOf(name));
        settings.setValue("name",name);
        settings.setValue("type",infoMap[name].type);
        switch(infoMap[name].type) {
        case QuickConnectWindow::Telnet:
            settings.setValue("hostname",infoMap[name].TelnetData.hostname);
            settings.setValue("port",infoMap[name].TelnetData.port);
            settings.setValue("socketType",infoMap[name].TelnetData.webSocket);
            break;
        case QuickConnectWindow::Serial:
            settings.setValue("portName",infoMap[name].SerialData.portName);
            settings.setValue("baudRate",infoMap[name].SerialData.baudRate);
            settings.setValue("dataBits",infoMap[name].SerialData.dataBits);
            settings.setValue("parity",infoMap[name].SerialData.parity);
            settings.setValue("stopBits",infoMap[name].SerialData.stopBits);
            settings.setValue("flowControl",infoMap[name].SerialData.flowControl);
            settings.setValue("xEnable",infoMap[name].SerialData.xEnable);
            break;
        case QuickConnectWindow::LocalShell:
            settings.setValue("command",infoMap[name].LocalShellData.command);
            break;
        case QuickConnectWindow::Raw:
            settings.setValue("hostname",infoMap[name].RawData.hostname);
            settings.setValue("port",infoMap[name].RawData.port);
            break;
        default:
            break;
        }
    }
    settings.endArray();
}

void MainWindow::connectSessionFromSessionManager(QString name)
{
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getName() == name) {
            sessionsWindow->getTermWidget()->setFocus();
            return;
        }
    }
    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString current_name = settings.value("name").toString();
        if(current_name == name) {
            QuickConnectWindow::QuickConnectData data;
            data.type = (QuickConnectWindow::QuickConnectType)(settings.value("type").toInt());
            switch(data.type) {
            case QuickConnectWindow::Telnet:
                data.TelnetData.hostname = settings.value("hostname").toString();
                data.TelnetData.port = settings.value("port").toInt();
                data.TelnetData.webSocket = settings.value("socketType").toString();
                startTelnetSession(findCurrentFocusGroup(),data.TelnetData.hostname,data.TelnetData.port,
                                   data.TelnetData.webSocket == "None"?QTelnet::TCP:
                                   data.TelnetData.webSocket == "Insecure"?QTelnet::WEBSOCKET:
                                   QTelnet::SECUREWEBSOCKET, current_name);
                break;
            case QuickConnectWindow::Serial:
                data.SerialData.portName = settings.value("portName").toString();
                data.SerialData.baudRate = settings.value("baudRate").toInt();
                data.SerialData.dataBits = settings.value("dataBits").toInt();
                data.SerialData.parity = settings.value("parity").toInt();
                data.SerialData.stopBits = settings.value("stopBits").toInt();
                data.SerialData.flowControl = settings.value("flowControl").toBool();
                data.SerialData.xEnable = settings.value("xEnable").toBool();
                startSerialSession(findCurrentFocusGroup(),data.SerialData.portName,data.SerialData.baudRate,
                                   data.SerialData.dataBits,data.SerialData.parity,
                                   data.SerialData.stopBits,data.SerialData.flowControl,
                                   data.SerialData.xEnable, current_name);
                break;
            case QuickConnectWindow::LocalShell:
                data.LocalShellData.command = settings.value("command").toString();
                startLocalShellSession(findCurrentFocusGroup(),data.LocalShellData.command,globalOptionsWindow->getNewTabWorkPath(),current_name);
                break;
            case QuickConnectWindow::Raw:
                data.RawData.hostname = settings.value("hostname").toString();
                data.RawData.port = settings.value("port").toInt();
                startRawSocketSession(findCurrentFocusGroup(),data.RawData.hostname,data.RawData.port, current_name);
                break;
            case QuickConnectWindow::NamePipe:
                data.NamePipeData.pipeName = settings.value("pipeName").toString();
                startNamePipeSession(findCurrentFocusGroup(),data.NamePipeData.pipeName, current_name);
                break;
            case QuickConnectWindow::SSH2:
            default:
                break;
            }
            break;
        }
    }
}

QString MainWindow::startTelnetSession(MainWidgetGroup *group, QString hostname, quint16 port, QTelnet::SocketType type, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Telnet - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Telnet"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    if(name.isEmpty()) {
        name = hostname;
        checkSessionName(name);
    } 
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
    return name;
}

QString MainWindow::startSerialSession(MainWidgetGroup *group, QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Serial,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Serial - ")+portName);
    sessionsWindow->setShortTitle(tr("Serial"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    if(name.isEmpty()) {
        name = portName;
        checkSessionName(name);
    }
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
    return name;
}

QString MainWindow::startRawSocketSession(MainWidgetGroup *group, QString hostname, quint16 port, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Raw - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Raw"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    if(name.isEmpty()) {
        name = hostname;
        checkSessionName(name);
    }
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
    return name;
}

QString MainWindow::startNamePipeSession(MainWidgetGroup *group, QString pipeName, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::NamePipe,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("NamePipe - ")+pipeName);
    sessionsWindow->setShortTitle(tr("NamePipe"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    if(name.isEmpty()) {
        name = pipeName;
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startNamePipeSession(pipeName);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString MainWindow::getDirAndcheckeSysName(const QString &title)
{
    // newTitle lile username@hostname:dir
#if defined(Q_OS_WIN)
    static QRegularExpression stdTitleFormat("^(\\S+)@(\\S+):(.*):(.*)$");
    if(stdTitleFormat.match(title).hasMatch()) {
        QString username = stdTitleFormat.match(title).captured(1);
        QString hostname = stdTitleFormat.match(title).captured(2);
        QString dir = stdTitleFormat.match(title).captured(3) + ":" +
                        stdTitleFormat.match(title).captured(4);
#else
    static QRegularExpression stdTitleFormat("^(\\S+)@(\\S+):(.*)$");
    if(stdTitleFormat.match(title).hasMatch()) {
        QString username = stdTitleFormat.match(title).captured(1);
        QString hostname = stdTitleFormat.match(title).captured(2);
        QString dir = stdTitleFormat.match(title).captured(3);
#endif
    #if defined(Q_OS_WIN)
        QString sysUsername = qEnvironmentVariable("USERNAME");
        QString sysHostname = qEnvironmentVariable("COMPUTERNAME");
        hostname = hostname.left(hostname.indexOf(":"));
    #elif defined(Q_OS_MAC)
        QString sysUsername = qEnvironmentVariable("USER");
        QString sysHostname = QHostInfo::localHostName().replace(".local","");
    #else
        QString sysUsername = qEnvironmentVariable("USER");
        QString sysHostname = QHostInfo::localHostName();
    #endif
        if((username == sysUsername) && (hostname == sysHostname)) {
        #if defined(Q_OS_WIN)
            return dir;
        #else
            return dir.replace("~",QDir::homePath());
        #endif
        }
    }

    return QString();
}

QString MainWindow::startLocalShellSession(MainWidgetGroup *group, const QString &command, const QString &workingDirectory, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
    setGlobalOptions(sessionsWindow);
    if(command.isEmpty()) {
        sessionsWindow->setLongTitle(tr("Local Shell"));
    } else {
        sessionsWindow->setLongTitle(tr("Local Shell - ")+command);
    }
    sessionsWindow->setShortTitle(tr("Local Shell"));
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    if(name.isEmpty()) {
        name = "Local Shell";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    QFileInfo workingDirectoryInfo(workingDirectory);
    sessionsWindow->setWorkingDirectory(workingDirectoryInfo.isDir()?workingDirectory:QDir::homePath());
    sessionsWindow->startLocalShellSession(command);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            QString workDir = getDirAndcheckeSysName(newTitle);
            if(!workDir.isEmpty()) {
                sessionsWindow->setShortTitle(workDir);
                QFileInfo fileInfo(workDir);
                if(fileInfo.isDir()) {
                    sessionsWindow->setWorkingDirectory(workDir.replace("~",QDir::homePath()));
                }
            } else {
                sessionsWindow->setShortTitle(newTitle);
            }
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString MainWindow::startSSH2Session(MainWidgetGroup *group, 
        QString hostname, quint16 port, QString username, QString password, QString name)
{
    QString opensshCmd = "/usr/bin/sshpass -p "+ password + " /usr/bin/ssh "+username+"@"+hostname+" -p "+QString::number(port);
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle("SSH2 - "+username+"@"+hostname);
    sessionsWindow->setShortTitle("SSH2");
    group->sessionTab->addTab(sessionsWindow->getTermWidget(), sessionsWindow->getTitle());
    if(name.isEmpty()) {
        name = "SSH2";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->setWorkingDirectory(QDir::homePath());
    sessionsWindow->startLocalShellSession(opensshCmd);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            group->sessionTab->setTabText(group->sessionTab->indexOf(sessionsWindow->getTermWidget()), sessionsWindow->getTitle());
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

int MainWindow::stopSession(MainWidgetGroup *group, int index)
{
    QTermWidget *termWidget = (QTermWidget *)group->sessionTab->widget(index);
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getTermWidget() == termWidget) {
            if(!sessionsWindow->isLocked()) {
                sessionList.removeOne(sessionsWindow);
                group->sessionTab->removeTab(index);
                delete sessionsWindow;
            }
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

int MainWindow::cloneCurrentSession(MainWidgetGroup *group, QString name)
{
    if(group->sessionTab->count() == 0) return -1;
    QTermWidget *termWidget = (QTermWidget *)group->sessionTab->currentWidget();
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getTermWidget() == termWidget) {
            SessionsWindow *sessionsWindowClone = new SessionsWindow(sessionsWindow->getSessionType(),this);
            setGlobalOptions(sessionsWindowClone);
            sessionsWindowClone->setLongTitle(sessionsWindow->getLongTitle());
            sessionsWindowClone->setShortTitle(sessionsWindow->getShortTitle());
            sessionsWindowClone->setShowShortTitle(sessionsWindow->getShowShortTitle());
            group->sessionTab->addTab(sessionsWindowClone->getTermWidget(), group->sessionTab->tabText(group->sessionTab->indexOf(termWidget)));
            if(name.isEmpty()) {
                name = sessionsWindow->getName();
                checkSessionName(name);
            }
            sessionsWindowClone->setName(name);
            sessionsWindowClone->cloneSession(sessionsWindow);
            sessionList.push_back(sessionsWindowClone);
            connect(sessionsWindowClone->getTermWidget(), &QTermWidget::titleChanged, this, [=](int title,const QString& newTitle){
                if(title == 0 || title == 2) {
                    sessionsWindowClone->setLongTitle(newTitle);
                    QString workDir = getDirAndcheckeSysName(newTitle);
                    if(!workDir.isEmpty()) {
                        sessionsWindowClone->setShortTitle(workDir);
                        QFileInfo fileInfo(workDir);
                        if(fileInfo.isDir()) {
                            sessionsWindowClone->setWorkingDirectory(workDir.replace("~",QDir::homePath()));
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

void MainWindow::addBookmark(const QString &path)
{
    QAction *action = new QAction(path,bookmarkMenu);
    action->setStatusTip(path);
    bookmarkMenu->addAction(action);
    connect(action,&QAction::triggered,this,[=](){
        startLocalShellSession(findCurrentFocusGroup(),QString(),path);
    });
    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Bookmark");
    settings.endArray();
    settings.beginWriteArray("Global/Bookmark");
    settings.setArrayIndex(size);
    settings.setValue("path",path);
    settings.endArray();
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
                           "<p>&nbsp;<a href='https://github.com/QQxiaoming/quardCRT'>https://github.com/QQxiaoming</a></p>"
                           "<p>&nbsp;<a href='https://gitee.com/QQxiaoming/quardCRT'>https://gitee.com/QQxiaoming</a></p>"
                           ).arg(VERSION,GIT_TAG)
                       );
}

void MainWindow::appHelp(QWidget *parent)
{
    QMessageBox::about(parent, tr("Help"), 
        tr("Global Shortcuts:\n\n"
        "  ALT+\"U\"\t\tshow/hide menu bar\n"
        "  ALT+\"T\"\t\tconnect to LocalShell\n"
        "  CTRL+SHIFT+\"T\"\tclone current session\n"
        "  ALT+\"N\"\t\tswitch ui to STD mode\n"
        "  ALT+\"J\"\t\tswitch ui to MINI mode\n"
        "  ALT+\"-\"\t\tswitch to previous session\n"
        "  ALT+\"=\"\t\tswitch to next session\n"
        "  ALT+LEFT\tGo to line start\n"
        "  ALT+RIGHT\tGo to line end\n")
    );
}

void MainWindow::setAppLangeuage(QLocale::Language lang) {
    GlobalSetting settings;
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
        settings.setValue("Global/Startup/language","zh_CN");
        break;
    case QLocale::Japanese:
        if(qtTranslator->load("qt_ja.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_ja.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_ja_JP.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","ja_JP");
        break;
    default:
    case QLocale::English:
        if(qtTranslator->load("qt_en.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_en.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_en_US.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","en_US");
        break;
    }
    QTermWidget::setLangeuage(lang);
}
