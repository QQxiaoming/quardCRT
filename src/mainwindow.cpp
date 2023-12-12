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
#include <QUrl>
#include <QProcess>
#include <QClipboard>
#include <QInputDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPluginLoader>
#include <QMap>

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
#include "sshsftp.h"

#include "ui_mainwindow.h"

CentralWidget::CentralWidget(QString dir, StartupUIMode mode, QLocale lang, bool isDark, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CentralWidget)
    , windowTransparency(1.0)
    , windowTransparencyEnabled(false)
    , language(lang)
    , isDarkTheme(isDark)
    , mainWindow(static_cast<MainWindow *>(parent)) {
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

    sftpWindow = new SftpWindow(this);

    netScanWindow = new NetScanWindow(this);

    oneStepWindow = new OneStepWindow(this);
    GlobalSetting settings;
    int size = settings.beginReadArray("Global/OneStep");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString stepName = settings.value("stepName").toString();
        QString userName = settings.value("userName").toString();
        QString password = settings.value("password").toString();
        int port = settings.value("port").toInt();
        OneStepWindow::Config config(stepName,userName,password,port);
        oneStepList.append(config);
    }
    settings.endArray();

    startTftpSeverWindow = new StartTftpSeverWindow(this);
    tftpServer = new QTftp;

    keyMapManagerWindow = new keyMapManager(this);
    keyMapManagerWindow->setAvailableKeyBindings(QTermWidget::availableKeyBindings());

    globalOptionsWindow = new GlobalOptionsWindow(this);
    globalOptionsWindow->setAvailableColorSchemes(QTermWidget::availableColorSchemes());

    sessionOptionsWindow = new SessionOptionsWindow(this);

    hexViewWindow = new HexViewWindow(this);
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
    sessionManagerPushButton->setFixedSize(250,26);
    ui->graphicsView->setFixedSize(30, 250);
    ui->sidewidget->setFixedWidth(30);

    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        mainWidgetGroup->sessionTab->setScrollTitleMode((SessionTab::TitleScrollMode)globalOptionsWindow->getTabTitleMode());
        mainWidgetGroup->sessionTab->setTitleWidth(globalOptionsWindow->getTabTitleWidth());
        mainWidgetGroup->sessionTab->setPreviewEnabled(globalOptionsWindow->getEnableTabPreview());
        mainWidgetGroup->sessionTab->setPreviewWidth(globalOptionsWindow->getTabPreviewWidth());
    }

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
                QWidget *widget = (QWidget *)mainWidgetGroup->sessionTab->widget(index);
                foreach(SessionsWindow *sessionsWindow, sessionList) {
                    sessionsWindow->setScrollBarPosition(verticalScrollBarAction->isChecked()?QTermWidget::ScrollBarRight:QTermWidget::NoScrollBar);
                    // use new style disconnect
                    disconnect(sessionsWindow,&SessionsWindow::hexDataDup,
                                hexViewWindow,&HexViewWindow::recvData);
                    if(sessionsWindow->getMainWidget() == widget) {
                        logSessionAction->setChecked(sessionsWindow->isLog());
                        rawLogSessionAction->setChecked(sessionsWindow->isRawLog());
                        lockSessionAction->setChecked(sessionsWindow->isLocked());
                        if(hexViewAction->isChecked()) {
                            connect(sessionsWindow,&SessionsWindow::hexDataDup,
                                        hexViewWindow,&HexViewWindow::recvData);
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
                QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                if(sessionsWindow->isLocked()) {
                    QAction *unlockAction = new QAction(tr("Unlock Session"),this);
                    unlockAction->setStatusTip(tr("Unlock current session"));
                    menu->addAction(unlockAction);
                    connect(unlockAction,&QAction::triggered,this,[=](){
                        lockSessionWindow->showUnlock();
                    });
                } else {
                    QAction *moveToAnotherTabAction = new QAction(tr("Move to another Tab"),this);
                    moveToAnotherTabAction->setStatusTip(tr("Move to current session to another tab group"));
                    menu->addAction(moveToAnotherTabAction);
                    connect(moveToAnotherTabAction,&QAction::triggered,this,[=](){
                        if(mainWidgetGroup == mainWidgetGroupList.at(0)) {
                            moveToAnotherTab(0,1,index);
                        } else {
                            moveToAnotherTab(1,0,index);
                        }
                    });
                    QAction *floatAction = new QAction(tr("Floating Window"),this);
                    floatAction->setStatusTip(tr("Floating current session to a new window"));
                    menu->addAction(floatAction);
                    menu->addSeparator();
                    connect(floatAction,&QAction::triggered,this,[=](){
                        floatingWindow(mainWidgetGroup,index);
                    });
                    if(sessionsWindow->getSessionType() == SessionsWindow::LocalShell) {
                        QAction *copyPathAction = new QAction(tr("Copy Path"),this);
                        copyPathAction->setStatusTip(tr("Copy current session working folder path"));
                        menu->addAction(copyPathAction);
                        connect(copyPathAction,&QAction::triggered,this,[=](){
                            QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                            QString dir = sessionsWindow->getWorkingDirectory();
                            if(!dir.isEmpty()) {
                                QFileInfo fileInfo(dir);
                                if(fileInfo.isDir()) {
                                    QApplication::clipboard()->setText(dir);
                                    return;
                                }
                            }
                            QMessageBox::warning(this, tr("Warning"), tr("No working folder!"));
                        });
                        QAction *addPathToBookmarkAction = new QAction(tr("Add Path to Bookmark"),this);
                        addPathToBookmarkAction->setStatusTip(tr("Add current session working folder path to bookmark"));
                        menu->addAction(addPathToBookmarkAction);
                        connect(addPathToBookmarkAction,&QAction::triggered,this,[=](){
                            QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                            QString dir = sessionsWindow->getWorkingDirectory();
                            if(!dir.isEmpty()) {
                                QFileInfo fileInfo(dir);
                                if(fileInfo.isDir()) {
                                    addBookmark(dir);
                                    return;
                                }
                            }
                            QMessageBox::warning(this, tr("Warning"), tr("No working folder!"));
                        });
                        QAction *openWorkingFolderAction = new QAction(tr("Open Working Folder"),this);
                        openWorkingFolderAction->setStatusTip(tr("Open current session working folder in system file manager"));
                        menu->addAction(openWorkingFolderAction);
                        menu->addSeparator();
                        connect(openWorkingFolderAction,&QAction::triggered,this,[=](){
                            QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                            QString dir = sessionsWindow->getWorkingDirectory();
                            if(!dir.isEmpty()) {
                                QFileInfo fileInfo(dir);
                                if(fileInfo.isDir()) {
                                    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
                                    return;
                                }
                            }
                            QMessageBox::warning(this, tr("Warning"), tr("No working folder!"));
                        });
                    }
                    if(sessionsWindow->getSessionType() == SessionsWindow::SSH2) {
                        QAction *openSFtpAction = new QAction(tr("Open SFTP"),this);
                        openSFtpAction->setStatusTip(tr("Open SFTP in a new window"));
                        menu->addAction(openSFtpAction);
                        menu->addSeparator();
                        connect(openSFtpAction,&QAction::triggered,this,[=](){
                            QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                            SshSFtp *sftp = sessionsWindow->getSshSFtpChannel();
                            if(sftp == nullptr) {
                                QMessageBox::warning(this, tr("Warning"), tr("No SFTP channel!"));
                                return;
                            }
                            sftpWindow->setSftpChannel(sftp);
                            sftpWindow->show();
                        });
                    }
                    QAction *saveSessionAction = new QAction(tr("Save Session"),this);
                    saveSessionAction->setStatusTip(tr("Save current session to session manager"));
                    menu->addAction(saveSessionAction);
                    connect(saveSessionAction,&QAction::triggered,this,[=](){
                        QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
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
                    });
                    QAction *propertiesAction = new QAction(tr("Properties"),this);
                    propertiesAction->setStatusTip(tr("Show current session properties"));
                    menu->addAction(propertiesAction);
                    menu->addSeparator();
                    connect(propertiesAction,&QAction::triggered,this,[=](){
                        QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                        if(sessionsWindow->isLocked()) {
                            return;
                        }
                        QuickConnectWindow::QuickConnectData data;
                        QString name;
                        sessionWindow2InfoData(sessionsWindow,data,name);
                        sessionOptionsWindow->setSessionProperties(name,data);
                        sessionOptionsWindow->setSessionState(sessionsWindow->getStateInfo());
                        sessionOptionsWindow->show();
                    });
                    if(sessionsWindow->getState() == SessionsWindow::Disconnected ||
                          sessionsWindow->getState() == SessionsWindow::Error) {
                        menu->addAction(reconnectAction);
                    }
                    QAction *closeAction = new QAction(tr("Close"),this);
                    closeAction->setStatusTip(tr("Close current session"));
                    menu->addAction(closeAction);
                    connect(closeAction,&QAction::triggered,this,[=](){
                        stopSession(mainWidgetGroup,index);
                    });
                    QAction *closeOthersAction = new QAction(tr("Close Others"),this);
                    closeOthersAction->setStatusTip(tr("Close other sessions"));
                    menu->addAction(closeOthersAction);
                    connect(closeOthersAction,&QAction::triggered,this,[=](){
                        int count = mainWidgetGroup->sessionTab->count();
                        for(int i=count;i>0;i--) {
                            if(i != index) {
                                stopSession(mainWidgetGroup,i);
                            }
                        }
                    });
                    QAction *closeToTheRightAction = new QAction(tr("Close to the Right"),this);
                    closeToTheRightAction->setStatusTip(tr("Close sessions to the right"));
                    menu->addAction(closeToTheRightAction);
                    connect(closeToTheRightAction,&QAction::triggered,this,[=](){
                        int count = mainWidgetGroup->sessionTab->count();
                        for(int i=count;i>index;i--) {
                            stopSession(mainWidgetGroup,i);
                        }
                    });
                    QAction *closeAllAction = new QAction(tr("Close All"),this);
                    closeAllAction->setStatusTip(tr("Close all sessions"));
                    menu->addAction(closeAllAction);
                    connect(closeAllAction,&QAction::triggered,this,[=](){
                        while(mainWidgetGroup->sessionTab->count() > 0) {
                            stopSession(mainWidgetGroup,mainWidgetGroup->sessionTab->count());
                        }
                    });
                }
            } else if(index == -1) {
                if(mainWidgetGroup->sessionTab->count() != 0) {
                    QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                    if(sessionsWindow->isLocked()) {
                        delete menu;
                        return;
                    }
                    if(sessionsWindow->isTerminal())
                        terminalWidgetContextMenuBase(menu,sessionsWindow,position);
                    if(fullScreenAction->isChecked()) {
                        menu->addSeparator();
                        menu->addAction(fullScreenAction);
                    } else if(!ui->menuBar->isVisible()) {
                        menu->addSeparator();
                        menu->addAction(menuBarAction);
                    }
                } else {
                    if(fullScreenAction->isChecked()) {
                        menu->addSeparator();
                        menu->addAction(fullScreenAction);
                    } else if(!ui->menuBar->isVisible()) {
                        menu->addSeparator();
                        menu->addAction(menuBarAction);
                    }
                }
            } else if(index == -2) {
                menu->addAction(menuBarAction);
                menu->addAction(toolBarAction);
                menu->addAction(cmdWindowAction);
                menu->addAction(fullScreenAction);
            }
            if(menu->isEmpty()) {
                delete menu;
                return;
            }
            menu->move(cursor().pos()+QPoint(5,5));
            menu->show();
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::dragTabMoved,this,[=](int from, int to, SessionTab *toTab){
            if(from <= 0) return;
            if(toTab == mainWidgetGroup->sessionTab) {
                if(from == to) return;
                if(from > 0) {
                    if(to == -1) {
                        floatingWindow(mainWidgetGroup,from);
                    }
                }
            } else {
                if(mainWidgetGroupList[0]->sessionTab == toTab) {
                    moveToAnotherTab(1,0,from);    
                } else if(mainWidgetGroupList[1]->sessionTab == toTab) {
                    moveToAnotherTab(0,1,from);
                }
            }
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabBarDoubleClicked,this,[=](int index){
            QWidget *widget = mainWidgetGroup->sessionTab->widget(index);
            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            if(sessionsWindow->isLocked()) return;
            sessionsWindow->setShowShortTitle(!sessionsWindow->getShowShortTitle());
            mainWidgetGroup->sessionTab->setTabText(index,sessionsWindow->getTitle());
        });
    #if defined(Q_OS_MACOS)
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabPreviewShow,this,[=](int index){
            if(mainWindow) {
                mainWindow->fixWhenShowQuardCRTTabPreviewIssue();
            }
        });
    #endif
        connect(mainWidgetGroup->commandWidget, &CommandWidget::sendData, this, [=](const QByteArray &data) {
            if(mainWidgetGroup->sessionTab->count() != 0) {
                QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            if(sessionsWindow->isLocked()) return;
                if(sessionsWindow->isLocked()) return;
                sessionsWindow->proxySendData(data);
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
        GlobalSetting settings;
        int size = settings.beginReadArray("Global/Session");
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            QString current_name = settings.value("name").toString();
            if(current_name == str) {
                QuickConnectWindow::QuickConnectData data;
                if(setting2InfoData(&settings, data, current_name) != 0) {
                    QMessageBox::warning(this, tr("Warning"), tr("Session properties error!"));
                    return;
                }
                sessionOptionsWindow->setSessionProperties(str,data);
                sessionOptionsWindow->show();
                break;
            }
        }
    });
    connect(globalOptionsWindow, &QDialog::accepted, this,[&](){
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            mainWidgetGroup->sessionTab->setScrollTitleMode((SessionTab::TitleScrollMode)globalOptionsWindow->getTabTitleMode());
            mainWidgetGroup->sessionTab->setTitleWidth(globalOptionsWindow->getTabTitleWidth());
            mainWidgetGroup->sessionTab->setPreviewEnabled(globalOptionsWindow->getEnableTabPreview());
            mainWidgetGroup->sessionTab->setPreviewWidth(globalOptionsWindow->getTabPreviewWidth());
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
        if(ui->menuBar->isVisible() == true) if(!mainWindow) menuBarAction->trigger();
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
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        MainWidgetGroup *group = findCurrentFocusGroup();
        int index = group->sessionTab->indexOf(widget);
        if(index < group->sessionTab->count()) {
            group->sessionTab->setCurrentIndex(index);
        } else {
            group->sessionTab->setCurrentIndex(0);
        }
        group->sessionTab->currentWidget()->setFocus();
    });
    shortcutTabMinusSwitch = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_Minus),this);
    connect(shortcutTabMinusSwitch,&QShortcut::activated,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        MainWidgetGroup *group = findCurrentFocusGroup();
        int index = group->sessionTab->indexOf(widget) - 2;
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
    shortcutConnectAddressEdit = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_R),this);
    connect(shortcutConnectAddressEdit,&QShortcut::activated,this,[=](){
        if(connectAddressEdit->hasFocus() == true) {
            QString str = connectAddressEdit->text();
            static QRegularExpression connectAddressFormat("^(\\w+):\\/\\/([\\w\\.]+)(\\:(\\d+))?$");
            if(connectAddressFormat.match(str).hasMatch()) {
                QString type = connectAddressFormat.match(str).captured(1);
                QString hostname = connectAddressFormat.match(str).captured(2);
                QString port = connectAddressFormat.match(str).captured(4);
                if(type == "telnet") {
                    startTelnetSession(findCurrentFocusGroup(),hostname,port.toInt(),QTelnet::TCP);
                } else if(type == "localshell") {
                    startLocalShellSession(findCurrentFocusGroup());
                } else if(type == "raw") {
                    startRawSocketSession(findCurrentFocusGroup(),hostname,port.toInt());
                } else if(type == "namepipe") {
                    startNamePipeSession(findCurrentFocusGroup(),hostname);
                } else if(type == "ssh") {
                    //TODO:
                } else if(type == "serial") {
                    //TODO:
                } else if(type == "vnc") {
                    //TODO:
                }
            }
        }
    });

    ui->statusBar->showMessage(tr("Ready"));

    if(!dir.isEmpty()) {
        QFileInfo fileInfo(dir);
        if(!fileInfo.isDir()) {
            dir = "";
        }
    }
    if(mode == MINIUI_MODE) {
        if(!mainWindow) menuBarAction->trigger();
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

CentralWidget::~CentralWidget() {
    stopAllSession(true);
    saveSettings();
    if(tftpServer->isRunning()) {
        tftpServer->stopServer();
    }
    delete tftpServer;
    delete sessionManagerPushButton;
    delete hexViewWindow;
    delete ui;
}

void CentralWidget::moveToAnotherTab(int src,int dst, int index) {
    QIcon icon = mainWidgetGroupList.at(src)->sessionTab->tabIcon(index);
    QString text = mainWidgetGroupList.at(src)->sessionTab->tabTitle(index);
    QWidget *widget = mainWidgetGroupList.at(src)->sessionTab->widget(index);
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    //FIXME: this is a workaround for ResizeNotification label QStyleSheetStyle::polish crash bug
    //       But this not a good solution, need to find a better way to solve this problem
    sessionsWindow->setShowResizeNotificationEnabled(false); 
    mainWidgetGroupList.at(src)->sessionTab->removeTab(index);
    int newIndex = mainWidgetGroupList.at(dst)->sessionTab->addTab(widget, text);
    mainWidgetGroupList.at(dst)->sessionTab->setTabIcon(newIndex,icon);
    mainWidgetGroupList.at(dst)->sessionTab->setCurrentIndex(
        mainWidgetGroupList.at(dst)->sessionTab->count()-1);
    mainWidgetGroupList.at(src)->sessionTab->setCurrentIndex(
        mainWidgetGroupList.at(src)->sessionTab->count()-1);
    QTimer::singleShot(1000, this, [=](){
        sessionsWindow->setShowResizeNotificationEnabled(true);
    });
};

void CentralWidget::terminalWidgetContextMenuBase(QMenu *menu,SessionsWindow *term,const QPoint& position)
{
    QList<QAction*> ftActions = term->filterActions(position);
    if(!ftActions.isEmpty()) {
        menu->addActions(ftActions);
        menu->addSeparator();
    }
    menu->addAction(copyAction);
    menu->addAction(pasteAction);
    menu->addAction(selectAllAction);
    menu->addAction(findAction);
    menu->addSeparator();
    QAction *highlightAction = new QAction(tr("Highlight/Unhighlight"),this);
    highlightAction->setStatusTip(tr("Highlight/Unhighlight selected text"));
    menu->addAction(highlightAction);
    connect(highlightAction,&QAction::triggered,this,[=](){
        QString text = term->selectedText();
        if(text.isEmpty()) {
            return;
        }
        if(term->isContainHighLightText(text)) {
            term->removeHighLightText(text);
        } else {
            term->addHighLightText(text, Qt::white);
        }
    });
    int translateService  = globalOptionsWindow->getTranslateService();
    QString targetLanguage[3];
    switch (language.language()) {
    case QLocale::Chinese:
        if(language.script() == QLocale::SimplifiedChineseScript) {
            targetLanguage[0] = "zh-CN";
            targetLanguage[1] = "zh";
            targetLanguage[2] = "zh-Hans";
        } else if (language.script() == QLocale::TraditionalHanScript) {
            targetLanguage[0] = "zh-TW";
            targetLanguage[1] = "cht";
            targetLanguage[2] = "zh-Hant";
        }
        break;
    case QLocale::Russian:
        targetLanguage[0] = "ru";
        targetLanguage[1] = "ru";
        targetLanguage[2] = "ru";
        break;
    case QLocale::Korean:
        targetLanguage[0] = "ko";
        targetLanguage[1] = "kor";
        targetLanguage[2] = "ko";
        break;
    case QLocale::Japanese:
        targetLanguage[0] = "ja";
        targetLanguage[1] = "jp";
        targetLanguage[2] = "ja";
        break;
    case QLocale::French:
        targetLanguage[0] = "fr";
        targetLanguage[1] = "fra";
        targetLanguage[2] = "fr";
        break;
    case QLocale::Spanish:
        targetLanguage[0] = "es";
        targetLanguage[1] = "spa";
        targetLanguage[2] = "es";
        break;
    default:
    case QLocale::English:
        targetLanguage[0] = "en";
        targetLanguage[1] = "en";
        targetLanguage[2] = "en";
        break;
    }
    if(0 == translateService) {
        QAction *translateAction = new QAction(tr("Google Translate"),this);
        translateAction->setStatusTip(tr("Translate selected text"));
        menu->addAction(translateAction);
        connect(translateAction,&QAction::triggered,this,[=](){
            QString text = term->selectedText();
            if(text.isEmpty()) {
                return;
            }
            
            QUrl url("https://translate.google.com/?sl=auto&tl="+targetLanguage[0]+"&text="+text+"&op=translate");
            QDesktopServices::openUrl(url);
        });
    } else if(1 == translateService) {
        QAction *translateAction = new QAction(tr("Baidu Translate"),this);
        translateAction->setStatusTip(tr("Translate selected text"));
        menu->addAction(translateAction);
        connect(translateAction,&QAction::triggered,this,[=](){
            QString text = term->selectedText();
            if(text.isEmpty()) {
                return;
            }
            QUrl url("https://fanyi.baidu.com/#auto/"+targetLanguage[1]+"/"+text);
            QDesktopServices::openUrl(url);
        });
    } else if(2 == translateService) {
        QAction *translateAction = new QAction(tr("Microsoft Translate"),this);
        translateAction->setStatusTip(tr("Translate selected text"));
        menu->addAction(translateAction);
        connect(translateAction,&QAction::triggered,this,[=](){
            QString text = term->selectedText();
            if(text.isEmpty()) {
                return;
            }
            QUrl url("https://www.bing.com/translator/?from=auto&to="+targetLanguage[2]+"&text="+text);
            QDesktopServices::openUrl(url);
        });
    }
    if(!oneStepList.isEmpty()) {
        menu->addSeparator();
    }
    foreach(OneStepWindow::Config data, oneStepList) {
        QAction *action = new QAction(data.stepName,this);
        action->setStatusTip(tr("One step to \"%1\"").arg(data.stepName));
        menu->addAction(action);
        connect(action,&QAction::triggered,this,[=](){
            QString text = term->selectedText();
            if(text.isEmpty()) {
                return;
            }
            startSSH2Session(findCurrentFocusGroup(),text,data.port,data.userName,data.password);
        });
    }
}

void CentralWidget::floatingWindow(MainWidgetGroup *g, int index) {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowFlags(Qt::Window);
    dialog->resize(800,480);
    dialog->setLayout(new QVBoxLayout);
    MainWidgetGroup *group = new MainWidgetGroup(dialog);
    mainWidgetGroupList.append(group);
    int newGroup = mainWidgetGroupList.count()-1;
    if(g == mainWidgetGroupList.at(0)) {
        moveToAnotherTab(0,newGroup,index);
    } else {
        moveToAnotherTab(1,newGroup,index);
    }
    dialog->layout()->addWidget(group->splitter);
    group->sessionTab->setTabBarHidden(true);
    group->sessionTab->setAddTabButtonHidden(true);
    dialog->setWindowTitle(group->sessionTab->tabTitle(1));
    connect(group->sessionTab,&SessionTab::tabTextSet,this,[=](int index, const QString &text){
        if(index) dialog->setWindowTitle(text);
    });
    connect(group->sessionTab,&SessionTab::showContextMenu,this,[=](int index, const QPoint& position){
        if(index != -1) return;
        QMenu *menu = new QMenu(this);
        QWidget *widget = group->sessionTab->currentWidget();
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isTerminal())
            terminalWidgetContextMenuBase(menu,sessionsWindow,position);
        menu->addSeparator();
        QAction *floatBackAction = new QAction(tr("Back to Main Window"),this);
        menu->addAction(floatBackAction);
        connect(floatBackAction,&QAction::triggered,this,[=](){
            moveToAnotherTab(newGroup,0,1);
            dialog->close();
        });
        if(menu->isEmpty()) {
            delete menu;
            return;
        }
        menu->move(cursor().pos()+QPoint(5,5));
        menu->show();
    });
    connect(group->commandWidget, &CommandWidget::sendData, this, [=](const QByteArray &data) {
        QWidget *widget = group->sessionTab->currentWidget();
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->proxySendData(data);
    });
    connect(dialog, &QDialog::finished, this, [=](int result){
        MainWidgetGroup *group = mainWidgetGroupList.at(newGroup);
        stopSession(group,1,true);
        mainWidgetGroupList.removeAt(newGroup);
        delete dialog;
        Q_UNUSED(result);
    });
    dialog->show();
}

void CentralWidget::saveSettings(void) {
    GlobalSetting settings;
    if(mainWindow) {
        settings.setValue("MainWindow/Geometry", mainWindow->saveGeometry());
        settings.setValue("MainWindow/State", mainWindow->saveState());
    } else {
        settings.setValue("MainWindow/Geometry", saveGeometry());
        settings.setValue("MainWindow/State", saveState());
    }
}

void CentralWidget::restoreSettings(void) {
    GlobalSetting settings;
    if(mainWindow) {
        mainWindow->restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
        mainWindow->restoreState(settings.value("MainWindow/State").toByteArray());
    } else {
        restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
        restoreState(settings.value("MainWindow/State").toByteArray());
    }
}

MainWidgetGroup* CentralWidget::findCurrentFocusGroup(void) {
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

QWidget *CentralWidget::findCurrentFocusWidget(void) {
    SessionTab *sessionTab = findCurrentFocusGroup()->sessionTab;
    if(sessionTab->count() == 0) return nullptr;
    return sessionTab->currentWidget();
}

void CentralWidget::menuAndToolBarRetranslateUi(void) {
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

    newWindowAction->setText(tr("New Window"));
    newWindowAction->setIcon(QFontIcon::icon(QChar(0xf09d)));
    newWindowAction->setStatusTip(tr("Open a new window <Ctrl+Shift+N>"));
    newWindowAction->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_N));
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
    copyAction->setStatusTip(tr("Copy the selected text to the clipboard <Command+C>"));
    copyAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_C));
#else
    copyAction->setStatusTip(tr("Copy the selected text to the clipboard <Ctrl+Ins>"));
    copyAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_Insert));
#endif
    pasteAction->setText(tr("Paste"));
    pasteAction->setIcon(QFontIcon::icon(QChar(0xf0ea)));
#if defined(Q_OS_MACOS)
    pasteAction->setStatusTip(tr("Paste the clipboard text to the current session <Command+V>"));
    pasteAction->setShortcut(QKeySequence(Qt::CTRL|Qt::Key_V));
#else
    pasteAction->setStatusTip(tr("Paste the clipboard text to the current session <Shift+Ins>"));
    pasteAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::Key_Insert));
#endif
    copyAndPasteAction->setText(tr("Copy and Paste"));
    copyAndPasteAction->setStatusTip(tr("Copy the selected text to the clipboard and paste to the current session"));
    selectAllAction->setText(tr("Select All"));
    selectAllAction->setIcon(QFontIcon::icon(QChar(0xf046)));
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
    screenShotAction->setIcon(QFontIcon::icon(QChar(0xf03e)));
    screenShotAction->setStatusTip(tr("Screen shot current screen <Alt+P>"));
    screenShotAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_P));
    sessionExportAction->setText(tr("Session Export"));
    sessionExportAction->setIcon(QFontIcon::icon(QChar(0xf093)));
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

    chineseAction->setText(tr("Simplified Chinese"));
    chineseAction->setStatusTip(tr("Switch to Simplified Chinese"));
    chineseHKAction->setText(tr("Traditional Chinese"));
    chineseHKAction->setStatusTip(tr("Switch to Traditional Chinese"));
    russianAction->setText(tr("Russian"));
    russianAction->setStatusTip(tr("Switch to Russian"));
    koreanAction->setText(tr("Korean"));
    koreanAction->setStatusTip(tr("Switch to Korean"));
    japaneseAction->setText(tr("Japanese"));
    japaneseAction->setStatusTip(tr("Switch to Japanese"));
    frenchAction->setText(tr("French"));
    frenchAction->setStatusTip(tr("Switch to French"));
    spanishAction->setText(tr("Spanish"));
    spanishAction->setStatusTip(tr("Switch to Spanish"));
    englishAction->setText(tr("English"));
    englishAction->setStatusTip(tr("Switch to English"));

    lightThemeAction->setText(tr("Light"));
    lightThemeAction->setStatusTip(tr("Switch to light theme"));
    darkThemeAction->setText(tr("Dark"));
    darkThemeAction->setStatusTip(tr("Switch to dark theme"));

    helpAction->setText(tr("Help"));
    helpAction->setIcon(QFontIcon::icon(QChar(0xf128)));
    helpAction->setStatusTip(tr("Display help"));
    checkUpdateAction->setText(tr("Check Update"));
    checkUpdateAction->setIcon(QFontIcon::icon(QChar(0xf09b)));
    checkUpdateAction->setStatusTip(tr("Check for updates"));
    aboutAction->setText(tr("About"));
    aboutAction->setIcon(QIcon(":/icons/icons/about.png"));
    aboutAction->setStatusTip(tr("Display about dialog"));
    aboutQtAction->setText(tr("About Qt"));
    aboutQtAction->setIcon(QIcon(":/icons/icons/aboutqt.png"));
    aboutQtAction->setStatusTip(tr("Display about Qt dialog"));

    laboratoryButton->setToolTip(tr("Laboratory"));
    laboratoryButton->setIcon(QFontIcon::icon(QChar(0xf0c3)));

    sshScanningAction->setText(tr("SSH Scanning"));
    sshScanningAction->setStatusTip(tr("Display SSH scanning dialog"));
    oneStepMenu->setTitle(tr("One Step"));
    addOneStepAction->setText(tr("Add One Step"));
    addOneStepAction->setStatusTip(tr("Add a one step"));
    editOneStepAction->setText(tr("Edit One Step"));
    editOneStepAction->setStatusTip(tr("Edit a one step"));
    removeOneStepAction->setText(tr("Remove One Step"));
    removeOneStepAction->setStatusTip(tr("Remove a one step"));
}

void CentralWidget::menuAndToolBarInit(void) {
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

    newWindowAction = new QAction(this);
    fileMenu->addAction(newWindowAction);
    fileMenu->addSeparator();
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
#ifdef Q_OS_WIN
    windwosTransparencyAction->setEnabled(false); //windows platform not support
#endif
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
    globalOptionsAction->setMenuRole(QAction::PreferencesRole);
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
    chineseAction->setChecked(language == QLocale(QLocale::Chinese, QLocale::SimplifiedChineseScript));
    languageMenu->addAction(chineseAction);
    chineseHKAction = new QAction(this);
    chineseHKAction->setActionGroup(languageActionGroup);
    chineseHKAction->setCheckable(true);
    chineseHKAction->setChecked(language == QLocale(QLocale::Chinese, QLocale::TraditionalChineseScript));
    languageMenu->addAction(chineseHKAction);
    russianAction = new QAction(this);
    russianAction->setActionGroup(languageActionGroup);
    russianAction->setCheckable(true);
    russianAction->setChecked(language == QLocale::Russian);
    languageMenu->addAction(russianAction);
    koreanAction = new QAction(this);
    koreanAction->setActionGroup(languageActionGroup);
    koreanAction->setCheckable(true);
    koreanAction->setChecked(language == QLocale::Korean);
    languageMenu->addAction(koreanAction);
    japaneseAction = new QAction(this);
    japaneseAction->setActionGroup(languageActionGroup);
    japaneseAction->setCheckable(true);
    japaneseAction->setChecked(language == QLocale::Japanese);
    languageMenu->addAction(japaneseAction);
    frenchAction = new QAction(this);
    frenchAction->setActionGroup(languageActionGroup);
    frenchAction->setCheckable(true);
    frenchAction->setChecked(language == QLocale::French);
    languageMenu->addAction(frenchAction);
    spanishAction = new QAction(this);
    spanishAction->setActionGroup(languageActionGroup);
    spanishAction->setCheckable(true);
    spanishAction->setChecked(language == QLocale::Spanish);
    languageMenu->addAction(spanishAction);
    englishAction = new QAction(this);
    englishAction->setActionGroup(languageActionGroup);
    englishAction->setCheckable(true);
    englishAction->setChecked(language == QLocale::English);
    languageMenu->addAction(englishAction);

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
    checkUpdateAction = new QAction(this);
    helpMenu->addAction(checkUpdateAction);
    helpMenu->addSeparator();
    aboutAction = new QAction(this);
    helpMenu->addAction(aboutAction);
    aboutQtAction = new QAction(this);
    helpMenu->addAction(aboutQtAction);

    //laboratory feature
    laboratoryButton = new QToolButton(this);
    laboratoryButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    laboratoryButton->setPopupMode(QToolButton::InstantPopup);
    laboratoryButton->setAutoRaise(true);
    laboratoryMenu = new QMenu(this);
    connect(laboratoryButton,&QToolButton::clicked,this,[=](){
        if(laboratoryMenu->isEmpty()) {
            return;
        }
        laboratoryMenu->move(laboratoryButton->mapToGlobal(QPoint(0,laboratoryButton->height())));
        laboratoryMenu->show();
    });
    if(mainWindow) {
        mainWindow->setLaboratoryButton(laboratoryButton);
    } 
    
    sshScanningAction = new QAction(this);
    laboratoryMenu->addAction(sshScanningAction);
    oneStepMenu = new QMenu(this);
    laboratoryMenu->addMenu(oneStepMenu);
    addOneStepAction = new QAction(this);
    oneStepMenu->addAction(addOneStepAction);
    editOneStepAction = new QAction(this);
    oneStepMenu->addAction(editOneStepAction);
    removeOneStepAction = new QAction(this);
    oneStepMenu->addAction(removeOneStepAction);
    laboratoryMenu->addSeparator();

    QTimer::singleShot(200, this, [&](){
        QDir pluginsDir(QCoreApplication::applicationDirPath());
        pluginsDir.cd("plugins");
        pluginsDir.cd("QuardCRT");
        foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName),this);
            QObject *plugin = loader.instance();
            if(plugin) {
                PluginInterface *iface = qobject_cast<PluginInterface *>(plugin);
                if(iface) {
                    QMap<QString, QString> params;
                    params.insert("version",VERSION);
                    params.insert("git_tag",GIT_TAG);
                    qDebug() << "we will load plugin:" << iface->name();
                    if(iface->init(params, this) == 0) {
                        QAction *action = iface->action();
                        if(action) {
                            laboratoryMenu->addAction(action);
                        } else {
                            QMenu *menu = iface->menu();
                            if(menu) {
                                laboratoryMenu->addMenu(menu);
                            }
                        }
                        pluginList.append(iface);
                        iface->setLanguage(language,qApp);
                        iface->retranslateUi();
                        connect(iface,SIGNAL(sendCommand(QString)),this,SLOT(onPluginSendCommand(QString)));
                    }
                }
            }
        }
    });

    menuAndToolBarRetranslateUi();

    setSessionClassActionEnable(false);
}

void CentralWidget::onPluginSendCommand(QString cmd){
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
    if(sessionsWindow->isLocked()) return;
    cmd += "\n";
    sessionsWindow->proxySendData(cmd.toLatin1());
}

void CentralWidget::setSessionClassActionEnable(bool enable)
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

void CentralWidget::menuAndToolBarConnectSignals(void) {
    connect(newWindowAction,&QAction::triggered,this,[=](){
        QProcess::startDetached(QApplication::applicationFilePath(),QApplication::arguments().mid(1));
    });
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
        } else if(data.type == QuickConnectWindow::VNC) {
            QString name = data.VNCData.hostname;
            if(data.openInTab) {
                name = startVNCSession(quickConnectMainWidgetGroup,name,data.VNCData.port,data.VNCData.password);
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
                QWidget *widget = sessionTab->widget(i+1);
                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
            }
        } else {
            QWidget *widget = findCurrentFocusWidget();
            if(widget == nullptr) return;
            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
            if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
        }
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isLocked()) {
            lockSessionAction->setChecked(true);
        } else {
            lockSessionAction->setChecked(false);
        }
    });
    connect(lockSessionWindow,&LockSessionWindow::sendUnLockSessionData,this,
        [=](QString password, bool unlockAllSession){
        if(unlockAllSession) {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->isLocked()) sessionsWindow->unlockSession(password);
            }
        } else {
            QWidget *widget = findCurrentFocusWidget();
            if(widget == nullptr) return;
            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
            if(sessionsWindow->isLocked()) sessionsWindow->unlockSession(password);
        }
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isLocked()) {
            lockSessionAction->setChecked(true);
        } else {
            lockSessionAction->setChecked(false);
        }
    });

    connect(connectInTabAction,&QAction::triggered,this,[=](){
        sessionManagerWidget->setVisible(true);
    });
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        startLocalShellSession(findCurrentFocusGroup(),QString(),globalOptionsWindow->getNewTabWorkPath());
    });
    connect(disconnectAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        stopSession(findCurrentFocusGroup(),findCurrentFocusGroup()->sessionTab->indexOf(widget));
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
            QWidget *widget = findCurrentFocusWidget();
            if(widget == nullptr) {
                logSessionAction->setChecked(false);
                return;
            }
            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
            if(!sessionsWindow->isLog()) {
                logSessionAction->setChecked(sessionsWindow->setLog(true) == 0);
            } else {
                logSessionAction->setChecked(sessionsWindow->setLog(false) != 0);
            }
        }
    );
    connect(rawLogSessionAction,&QAction::triggered,this,
        [&](void) {
            QWidget *widget = findCurrentFocusWidget();
            if(widget == nullptr) {
                rawLogSessionAction->setChecked(false);
                return;
            }
            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
            if(!sessionsWindow->isRawLog()) {
                rawLogSessionAction->setChecked(sessionsWindow->setRawLog(true) == 0);
            } else {
                rawLogSessionAction->setChecked(sessionsWindow->setRawLog(false) != 0);
            }
        }
    );
    connect(hexViewAction,&QAction::triggered,this,[=](){
        if(hexViewAction->isChecked()) {
            hexViewWindow->show();
            QWidget *widget = findCurrentFocusWidget();
            if(widget == nullptr) return;
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                disconnect(sessionsWindow,&SessionsWindow::hexDataDup,hexViewWindow,&HexViewWindow::recvData);
                if(sessionsWindow->getMainWidget() == widget) {
                    connect(sessionsWindow,&SessionsWindow::hexDataDup,hexViewWindow,&HexViewWindow::recvData);
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
            sessionsWindow->setColorScheme(colorScheme);
        }
    });
    connect(globalOptionsWindow,&GlobalOptionsWindow::transparencyChanged,this,[=](int transparency){
        windowTransparency = (100-transparency)/100.0;
        if(mainWindow) {
            mainWindow->setWindowOpacity(windowTransparencyEnabled?windowTransparency:1.0);
        } else {
            setWindowOpacity(windowTransparencyEnabled?windowTransparency:1.0);
        }
    });
    connect(sessionOptionsWindow,&SessionOptionsWindow::sessionPropertiesChanged,this,
        [=](QString name, QuickConnectWindow::QuickConnectData data, QString newName) {
        int64_t index = removeSessionFromSessionManager(name);
        if(index >= 0) {
            addSessionToSessionManager(data,newName, true, index);
        }
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
            sessionsWindow->setKeyBindings(keyBinding);
        }
    });

    connect(copyAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->copyClipboard();
    });
    connect(pasteAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->pasteClipboard();
    });
    connect(copyAndPasteAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->copyClipboard();
        sessionsWindow->pasteClipboard();
    });
    connect(selectAllAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->selectAll();
    });
    connect(findAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->toggleShowSearchBar();
    });
    connect(printScreenAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();

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
            sessionsWindow->saveHistory(&out,1);
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
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();

        GlobalSetting settings;
        QString screenShotPath = settings.value("Global/Options/ScreenShotPath",QDir::homePath()).toString();
        QString willsaveName = screenShotPath + "/quartCRT-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".jpg";
        QString fileName = FileDialog::getSaveFileName(this,tr("Save Screenshot"),willsaveName,tr("Image Files (*.jpg)"));
        if(fileName.isEmpty()) return;
        if(!fileName.endsWith(".jpg")) fileName.append(".jpg");
        sessionsWindow->screenShot(fileName);
        settings.setValue("Global/Options/ScreenShotPath",QFileInfo(fileName).absolutePath());
        ui->statusBar->showMessage(tr("Screenshot saved to %1").arg(fileName),3000);
    });
    connect(sessionExportAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();

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
            sessionsWindow->saveHistory(&file,format);
            file.close();
            settings.setValue("Global/Options/SessionExportPath",QFileInfo(fileName).absolutePath());
            ui->statusBar->showMessage(tr("Session Export saved to %1").arg(fileName),3000);
        } else {
            ui->statusBar->showMessage(tr("Session Export failed to save to %1").arg(fileName),3000);
        }
    });
    connect(clearScrollbackAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->clearScrollback();
    });
    connect(clearScreenAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->clearScreen();
    });
    connect(clearScreenAndScrollbackAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->clear();
    });
    connect(resetAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->clear();
    });
    connect(zoomInAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->zoomIn();
    });
    connect(zoomOutAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->zoomOut();
    });
    connect(zoomResetAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->setTerminalFont(globalOptionsWindow->getCurrentFont());
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
        // FIXME: if use QGoodWindow, we must call hide() and show() to make title bar refresh
        //       but this will cause the window flash, we will improve this in the future
        if(mainWindow) {
            mainWindow->hide();
            QTimer::singleShot(100,this,[=](){
                mainWindow->show();
            });
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
        if(mainWindow) {
            mainWindow->setWindowOpacity(windowTransparencyEnabled?windowTransparency:1.0);
        } else {
            setWindowOpacity(windowTransparencyEnabled?windowTransparency:1.0);
        }
    });
    connect(verticalScrollBarAction,&QAction::triggered,this,[=](bool checked){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->setScrollBarPosition(checked?QTermWidget::ScrollBarRight:QTermWidget::NoScrollBar);
        }
    });
    connect(allwaysOnTopAction,&QAction::triggered,this,[=](bool checked){
        if(mainWindow) {
            mainWindow->setWindowStaysOnTop(checked);
            mainWindow->hide();
            QTimer::singleShot(100,this,[=](){
                mainWindow->show();
            });
        } else {
            if(checked) {
                setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
            } else {
                setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            }
            show();
        }
    });
    connect(fullScreenAction,&QAction::triggered,this,[=](bool checked){
        if(mainWindow) {
            if(checked) {
                mainWindow->showFullScreen();
            } else {
                mainWindow->showNormal();
            }
        } else {
            if(checked) {
                showFullScreen();
            } else {
                showNormal();
            }
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
    connect(sshScanningAction,&QAction::triggered,this,[=](){
        bool isOk = false;
        int port = QInputDialog::getInt(this,tr("SSH Scanning"),tr("Port"),22,1,65535,1,&isOk);
        if(!isOk) return;
        netScanWindow->setScanPort(port);
        netScanWindow->show();
    });
    connect(addOneStepAction,&QAction::triggered,this,[=](){
        oneStepWindow->setConfig("","","",22);
        oneStepWindow->show();
    });
    connect(editOneStepAction,&QAction::triggered,this,[=](){
        QStringList stepNameList;
        foreach(OneStepWindow::Config data, oneStepList) {
            stepNameList.append(data.stepName);
        }
        if(stepNameList.isEmpty()) return;
        bool isOk = false;
        QString stepName = QInputDialog::getItem(this,tr("Edit One Step"),tr("Select a step"),stepNameList,0,false,&isOk);
        if(!isOk || stepName.isEmpty()) return;
        foreach(OneStepWindow::Config data, oneStepList) {
            if(data.stepName == stepName) {
                oneStepWindow->setConfig(data);
                oneStepWindow->show();
                break;
            }
        }
    });
    connect(removeOneStepAction,&QAction::triggered,this,[=](){
        QStringList stepNameList;
        foreach(OneStepWindow::Config data, oneStepList) {
            stepNameList.append(data.stepName);
        }
        if(stepNameList.isEmpty()) return;
        bool isOk = false;
        QString stepName = QInputDialog::getItem(this,tr("Remove One Step"),tr("Select a step"),stepNameList,0,false,&isOk);
        if(!isOk || stepName.isEmpty()) return;
        foreach(OneStepWindow::Config data, oneStepList) {
            if(data.stepName == stepName) {
                oneStepList.removeOne(data);
                break;
            }
        }
        GlobalSetting settings;
        settings.beginWriteArray("Global/OneStep");
        settings.remove("");
        settings.endArray();
        foreach(OneStepWindow::Config data, oneStepList) {
            settings.beginWriteArray("Global/OneStep");
            settings.setArrayIndex(oneStepList.indexOf(data));
            settings.setValue("stepName",data.stepName);
            settings.setValue("userName",data.userName);
            settings.setValue("password",data.password);
            settings.setValue("port",data.port);
            settings.endArray();
        }
    });
    connect(oneStepWindow,&OneStepWindow::accepted,this,[=](){
        OneStepWindow::Config newData = oneStepWindow->getConfig();
        if(newData.stepName.isEmpty()) {
            QMessageBox::warning(this,tr("One Step"),tr("Step name can not be empty!"));
            return;
        }
        QString oldStepName = oneStepWindow->getStepInitName();
        if(!oldStepName.isEmpty()) {
            foreach(OneStepWindow::Config data, oneStepList) {
                if(oldStepName == data.stepName) {
                    oneStepList.removeOne(data);
                    break;
                }
            }
        }
        oneStepList.append(newData);
        GlobalSetting settings;
        settings.beginWriteArray("Global/OneStep");
        settings.remove("");
        settings.endArray();
        foreach(OneStepWindow::Config data, oneStepList) {
            settings.beginWriteArray("Global/OneStep");
            settings.setArrayIndex(oneStepList.indexOf(data));
            settings.setValue("stepName",data.stepName);
            settings.setValue("userName",data.userName);
            settings.setValue("password",data.password);
            settings.setValue("port",data.port);
            settings.endArray();
        }
    });

    connect(sessionOptionsAction,&QAction::triggered,this,[=](){
        QString name;
        QuickConnectWindow::QuickConnectData data;
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionWindow2InfoData(sessionsWindow, data, name);
        sessionOptionsWindow->setSessionProperties(name,data);
        sessionOptionsWindow->setSessionState(sessionsWindow->getStateInfo());
        sessionOptionsWindow->show();
    });
    connect(globalOptionsAction,&QAction::triggered,this,[=](){
        globalOptionsWindow->show();
    });
    connect(languageActionGroup,&QActionGroup::triggered,this,[=](QAction *action){
        if(action == chineseAction) {
            this->language = QLocale(QLocale::Chinese, QLocale::SimplifiedChineseScript);
        } else if(action == chineseHKAction) {
            this->language = QLocale(QLocale::Chinese, QLocale::TraditionalChineseScript);
        } else if(action == russianAction) {
            this->language = QLocale::Russian;
        } else if(action == koreanAction) {
            this->language = QLocale::Korean;
        } else if(action == japaneseAction) {
            this->language = QLocale::Japanese;
        } else if(action == frenchAction) {
            this->language = QLocale::French;
        } else if(action == spanishAction) {
            this->language = QLocale::Spanish;
        } else if(action == englishAction) {
            this->language = QLocale::English;
        }

        setAppLangeuage(this->language);
        foreach(PluginInterface *iface, pluginList) {
            iface->setLanguage(this->language,qApp);
            iface->retranslateUi();
        }
        ui->retranslateUi(this);
        sessionManagerWidget->retranslateUi();
        startTftpSeverWindow->retranslateUi();
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            mainWidgetGroup->sessionTab->retranslateUi();
            mainWidgetGroup->commandWidget->retranslateUi();
        }
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->reTranslateUi();
        }
        menuAndToolBarRetranslateUi();
        ui->statusBar->showMessage(tr("Ready"));
        // FIXME: if use QGoodWindow, we must call hide() and show() to make title bar refresh
        //       but this will cause the window flash, we will improve this in the future
        if(mainWindow) {
            mainWindow->hide();
            QTimer::singleShot(100,this,[=](){
                mainWindow->show();
            });
        }
    });
    connect(lightThemeAction,&QAction::triggered,this,[=](){
        isDarkTheme = false;
        qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
        QFontIcon::instance()->setColor(Qt::black);
        menuAndToolBarRetranslateUi();
        GlobalSetting settings;
        settings.setValue("Global/Startup/dark_theme","false");
    });
    connect(darkThemeAction,&QAction::triggered,this,[=](){
        isDarkTheme = true;
        qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
        QFontIcon::instance()->setColor(Qt::white);
        menuAndToolBarRetranslateUi();
        GlobalSetting settings;
        settings.setValue("Global/Startup/dark_theme","true");
    });
    connect(exitAction, &QAction::triggered, this, [&](){
        qApp->quit();
    });
    connect(helpAction, &QAction::triggered, this, [&]() {
        if(mainWindow) {
            CentralWidget::appHelp(mainWindow);
        } else {
            CentralWidget::appHelp(this);
        }
    });
    connect(checkUpdateAction, &QAction::triggered, this, [&]() {
        QLocale locale;
        QLocale::Language lang = locale.language();
        if(lang == QLocale::Chinese && language == QLocale::Chinese) {
            QDesktopServices::openUrl(QUrl("https://gitee.com/QQxiaoming/quardCRT/releases"));
        } else {
            QDesktopServices::openUrl(QUrl("https://github.com/QQxiaoming/quardCRT/releases"));
        }
    });
    connect(aboutAction, &QAction::triggered, this, [&]() {
        if (mainWindow) {
            CentralWidget::appAbout(mainWindow);
        } else {
            CentralWidget::appAbout(this);
        }
    });
    connect(aboutQtAction, &QAction::triggered, this, [&]() {
        if (mainWindow) {
            QMessageBox::aboutQt(mainWindow);
        } else {
            QMessageBox::aboutQt(this);
        }
    });
}

void CentralWidget::setGlobalOptions(SessionsWindow *window) {
    window->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    window->setColorScheme(globalOptionsWindow->getCurrentColorScheme());
    window->setTerminalFont(globalOptionsWindow->getCurrentFont());
    window->setTerminalBackgroundMode(globalOptionsWindow->getBackgroundImageMode());
    window->setTerminalOpacity(globalOptionsWindow->getBackgroundImageOpacity());
    window->setHistorySize(globalOptionsWindow->getScrollbackLines());
    window->setKeyboardCursorShape(globalOptionsWindow->getCursorShape());
    window->setBlinkingCursor(globalOptionsWindow->getCursorBlink());
    window->setWordCharacters(globalOptionsWindow->getWordCharacters());
    window->setTerminalBackgroundImage("");
    window->setTerminalBackgroundMovie("");
    window->setTerminalBackgroundVideo("");
    QString backgroundImage = globalOptionsWindow->getBackgroundImage();
    if(backgroundImage.endsWith(".png") || backgroundImage.endsWith(".jpg") ||
        backgroundImage.endsWith(".jpeg") || backgroundImage.endsWith(".bmp")
        ) {
        window->setTerminalBackgroundImage(backgroundImage);
    } else if (backgroundImage.endsWith(".gif")) {
        if(globalOptionsWindow->getEnableTerminalBackgroundAnimation()) {
            window->setTerminalBackgroundMovie(backgroundImage);
        } else {
            window->setTerminalBackgroundImage(backgroundImage);
        }
    } else if (backgroundImage.endsWith(".mp4") || backgroundImage.endsWith(".avi") ||
        backgroundImage.endsWith(".mkv") || backgroundImage.endsWith(".mov")
        ) {
        if(globalOptionsWindow->getEnableTerminalBackgroundAnimation()) {
            window->setTerminalBackgroundVideo(backgroundImage);
        } else {
            QMessageBox::warning(this,tr("Warning"),tr("Video background is enabled, please enable animation in global options (more system resources) or change background image."),QMessageBox::Ok);
        }
    }
}

void CentralWidget::restoreSessionToSessionManager(void)
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

bool CentralWidget::checkSessionName(QString &name)
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

int CentralWidget::addSessionToSessionManager(SessionsWindow *sessionsWindow, QString &name)
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
        settings.setValue("hostname",sessionsWindow->getHostname());
        settings.setValue("port",sessionsWindow->getPort());
        settings.setValue("socketType",[&]() {
            if(sessionsWindow->getSocketType() == QTelnet::TCP) {
                return QString("None");
            } else if(sessionsWindow->getSocketType() == QTelnet::WEBSOCKET) {
                return QString("Insecure");
            } else if(sessionsWindow->getSocketType() == QTelnet::SECUREWEBSOCKET) {
                return QString("Secure");
            }
            return QString("None");
        }());
        break;
    case SessionsWindow::Serial:
        settings.setValue("portName",sessionsWindow->getPortName());
        settings.setValue("baudRate",sessionsWindow->getBaudRate());
        settings.setValue("dataBits",sessionsWindow->getDataBits());
        settings.setValue("parity",sessionsWindow->getParity());
        settings.setValue("stopBits",sessionsWindow->getStopBits());
        settings.setValue("flowControl",sessionsWindow->getFlowControl());
        settings.setValue("xEnable",sessionsWindow->getXEnable());
        break;
    case SessionsWindow::LocalShell:
        settings.setValue("command",sessionsWindow->getCommand());
        break;
    case SessionsWindow::RawSocket:
        settings.setValue("hostname",sessionsWindow->getHostname());
        settings.setValue("port",sessionsWindow->getPort());
        break;
    case SessionsWindow::NamePipe:
        settings.setValue("pipeName",sessionsWindow->getPipeName());
        break;
    case SessionsWindow::SSH2:
        settings.setValue("hostname",sessionsWindow->getHostname());
        settings.setValue("port",sessionsWindow->getPort());
        settings.setValue("username",sessionsWindow->getUserName());
        keyChainClass.writeKey(name,sessionsWindow->getPassWord());
        break;
    default:
        break;
    }
    settings.endArray();

    return 0;
}

int CentralWidget::addSessionToSessionManager(const QuickConnectWindow::QuickConnectData &data, QString &name, bool checkname, int64_t id)
{
    if(checkname) checkSessionName(name);

    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    if(id == -1 || id >= size) {
        settings.endArray();
        settings.beginWriteArray("Global/Session");
        settings.setArrayIndex(size);
        infoData2Setting(&settings, data, name);
        settings.endArray();
        sessionManagerWidget->addSession(name,data.type);
    } else {
        QList<QPair<QString,QuickConnectWindow::QuickConnectData>> infoList;
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            QuickConnectWindow::QuickConnectData dataInfo;
            QString current_name;
            setting2InfoData(&settings, dataInfo, current_name, true);
            if(i == id) {
                infoList.append(QPair<QString,QuickConnectWindow::QuickConnectData>(name,data));
            }
            infoList.append(QPair<QString,QuickConnectWindow::QuickConnectData>(current_name,dataInfo));
            sessionManagerWidget->removeSession(current_name);
        }
        settings.endArray();
        settings.beginWriteArray("Global/Session");
        settings.remove("");
        settings.endArray();
        settings.beginWriteArray("Global/Session");
        // write infoMap to settings
        for(int i=0;i<infoList.size();i++) {
            QPair<QString,QuickConnectWindow::QuickConnectData> info = infoList.at(i);
            QuickConnectWindow::QuickConnectData dataR = info.second;
            settings.setArrayIndex(i);
            sessionManagerWidget->addSession(info.first,dataR.type);
            if(i == id) {
                infoData2Setting(&settings, dataR, info.first);
            } else {
                infoData2Setting(&settings, dataR, info.first, true);
            }
        }
        settings.endArray();
    }

    return 0;
}

int64_t CentralWidget::removeSessionFromSessionManager(QString name)
{
    int64_t matched = -1;
    sessionManagerWidget->removeSession(name);
    GlobalSetting settings;
    QList<QPair<QString,QuickConnectWindow::QuickConnectData>> infoList;
    int size = settings.beginReadArray("Global/Session");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QuickConnectWindow::QuickConnectData data;
        QString current_name;
        setting2InfoData(&settings, data, current_name, true);
        if(current_name == name) {
            if(data.type == QuickConnectWindow::SSH2) {
                keyChainClass.deleteKey(name);
            }
            matched = i;
            continue;
        } else {
            infoList.append(QPair<QString,QuickConnectWindow::QuickConnectData>(current_name,data));
        }
    }
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    settings.remove("");
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    // write infoMap to settings
    for(int i=0;i<infoList.size();i++) {
        QPair<QString,QuickConnectWindow::QuickConnectData> info = infoList.at(i);
        QuickConnectWindow::QuickConnectData dataR = info.second;
        settings.setArrayIndex(i);
        infoData2Setting(&settings, dataR, info.first, true);
    }
    settings.endArray();

    return matched;
}

void CentralWidget::connectSessionFromSessionManager(QString name)
{
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getName() == name) {
            sessionsWindow->getMainWidget()->setFocus();
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
            if(setting2InfoData(&settings,data,current_name) != 0) {
                settings.endArray();
                QMessageBox::warning(this,tr("Warning"),tr("Session information get failed."),QMessageBox::Ok);
                return;
            }
            switch(data.type) {
            case QuickConnectWindow::Telnet:
                startTelnetSession(findCurrentFocusGroup(),data.TelnetData.hostname,data.TelnetData.port,
                                   data.TelnetData.webSocket == "None"?QTelnet::TCP:
                                   data.TelnetData.webSocket == "Insecure"?QTelnet::WEBSOCKET:
                                   QTelnet::SECUREWEBSOCKET, current_name);
                break;
            case QuickConnectWindow::Serial:
                startSerialSession(findCurrentFocusGroup(),data.SerialData.portName,data.SerialData.baudRate,
                                   data.SerialData.dataBits,data.SerialData.parity,
                                   data.SerialData.stopBits,data.SerialData.flowControl,
                                   data.SerialData.xEnable, current_name);
                break;
            case QuickConnectWindow::LocalShell:
                startLocalShellSession(findCurrentFocusGroup(),data.LocalShellData.command,globalOptionsWindow->getNewTabWorkPath(),current_name);
                break;
            case QuickConnectWindow::Raw:
                startRawSocketSession(findCurrentFocusGroup(),data.RawData.hostname,data.RawData.port, current_name);
                break;
            case QuickConnectWindow::NamePipe:
                startNamePipeSession(findCurrentFocusGroup(),data.NamePipeData.pipeName, current_name);
                break;
            case QuickConnectWindow::SSH2:
                startSSH2Session(findCurrentFocusGroup(),data.SSH2Data.hostname,data.SSH2Data.port,
                                 data.SSH2Data.username,data.SSH2Data.password, current_name);
                break;
            case QuickConnectWindow::VNC:
                startVNCSession(findCurrentFocusGroup(),data.VNCData.hostname,data.VNCData.port,
                                data.VNCData.password, current_name);
            default:
                break;
            }
            break;
        }
    }
}

void CentralWidget::connectSessionStateChange(SessionTab *tab, int index, SessionsWindow *sessionsWindow)
{
    tab->setTabIcon(index,QFontIcon::icon(QChar(0xf09e), Qt::gray));
    connect(sessionsWindow, &SessionsWindow::stateChanged, this, [=](SessionsWindow::SessionsState state){
        SessionTab *targetTab = nullptr;
        int targetIndex = -1;
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            int findIndex = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
            if(findIndex >= 0) {
                targetTab = mainWidgetGroup->sessionTab;
                targetIndex = findIndex;
                break;
            }
        }
        if(targetTab && targetIndex != -1) {
            switch(state) {
                case SessionsWindow::Connected:
                    targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf0c1), Qt::green));
                    break;
                case SessionsWindow::Error:
                case SessionsWindow::Disconnected:
                    targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf127), Qt::red));
                    break;
                case SessionsWindow::Locked:
                    if(sessionsWindow->isLocked())
                        targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf084), Qt::yellow));
                    else if(sessionsWindow->getState() == SessionsWindow::Connected) {
                        targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf0c1), Qt::green));
                    } else {
                        targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf127), Qt::red));
                    }
                    break;
                default:
                    break;
            }
        }
    });
}

QString CentralWidget::startTelnetSession(MainWidgetGroup *group, QString hostname, quint16 port, QTelnet::SocketType type, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Telnet - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Telnet"));
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = hostname;
        checkSessionName(name);
    } 
    sessionsWindow->setName(name);
    sessionsWindow->startTelnetSession(hostname,port,type);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindow->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString CentralWidget::startSerialSession(MainWidgetGroup *group, QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Serial,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Serial - ")+portName);
    sessionsWindow->setShortTitle(tr("Serial"));
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = portName;
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startSerialSession(portName,baudRate,dataBits,parity,stopBits,flowControl,xEnable);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindow->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString CentralWidget::startRawSocketSession(MainWidgetGroup *group, QString hostname, quint16 port, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Raw - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Raw"));
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = hostname;
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startRawSocketSession(hostname,port);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindow->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString CentralWidget::startNamePipeSession(MainWidgetGroup *group, QString pipeName, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::NamePipe,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("NamePipe - ")+pipeName);
    sessionsWindow->setShortTitle(tr("NamePipe"));
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = pipeName;
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startNamePipeSession(pipeName);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindow->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString CentralWidget::getDirAndcheckeSysName(const QString &title)
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
            return dir;
        }
    }

    return QString();
}

QString CentralWidget::startLocalShellSession(MainWidgetGroup *group, const QString &command, const QString &workingDirectory, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
    setGlobalOptions(sessionsWindow);
    if(command.isEmpty()) {
        sessionsWindow->setLongTitle(tr("Local Shell"));
    } else {
        sessionsWindow->setLongTitle(tr("Local Shell - ")+command);
    }
    sessionsWindow->setShortTitle(tr("Local Shell"));
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = "Local Shell";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    QFileInfo workingDirectoryInfo(workingDirectory);
    sessionsWindow->setWorkingDirectory(workingDirectoryInfo.isDir()?workingDirectory:QDir::homePath());
    sessionsWindow->startLocalShellSession(command);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            QString workDir = getDirAndcheckeSysName(newTitle);
            if(!workDir.isEmpty()) {
                sessionsWindow->setShortTitle(workDir);
                if(workDir.startsWith("~/")) {
                    workDir = workDir.replace(0,1,QDir::homePath());
                }
                QFileInfo fileInfo(workDir);
                if(fileInfo.isDir()) {
                    sessionsWindow->setWorkingDirectory(workDir);
                }
            } else {
                sessionsWindow->setShortTitle(newTitle);
            }
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindow->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString CentralWidget::startSSH2Session(MainWidgetGroup *group, 
        QString hostname, quint16 port, QString username, QString password, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::SSH2,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle("SSH2 - "+username+"@"+hostname);
    sessionsWindow->setShortTitle("SSH2");
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = "SSH2";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startSSH2Session(hostname,port,username,password);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            sessionsWindow->setShortTitle(newTitle);
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindow->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindow->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

QString CentralWidget::startVNCSession(MainWidgetGroup *group, QString hostname, quint16 port, QString password, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::VNC,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle("VNC - "+hostname);
    sessionsWindow->setShortTitle("VNC");
    int index = group->sessionTab->addTab(sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = "VNC";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startVNCSession(hostname,port,password);
    sessionList.push_back(sessionsWindow);
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return name;
}

int CentralWidget::stopSession(MainWidgetGroup *group, int index, bool force)
{
    if(index <= 0) return -1;
    if(group->sessionTab->count() == 0) return -1;
    QWidget *widget = group->sessionTab->widget(index);
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    if(!sessionsWindow->isLocked()) {
        if(force) {
            sessionList.removeOne(sessionsWindow);
            group->sessionTab->removeTab(index);
            delete sessionsWindow;
        } else {
            if(sessionsWindow->getState() == SessionsWindow::Connected) {
                if(sessionsWindow->getSessionType() == SessionsWindow::LocalShell) {
                    if(!sessionsWindow->hasChildProcess()) {
                        sessionList.removeOne(sessionsWindow);
                        group->sessionTab->removeTab(index);
                        delete sessionsWindow;
                        return 0;
                    }
                }
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Warning"), tr("Are you sure to disconnect this session?"),
                                            QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    sessionList.removeOne(sessionsWindow);
                    group->sessionTab->removeTab(index);
                    delete sessionsWindow;
                }
            } else {
                sessionList.removeOne(sessionsWindow);
                group->sessionTab->removeTab(index);
                delete sessionsWindow;
            }
        }
    }
    return 0;
}

int CentralWidget::stopAllSession(bool force)
{
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        while(mainWidgetGroup->sessionTab->count() > 0) {
            stopSession(mainWidgetGroup,mainWidgetGroup->sessionTab->count(), force);
        }
    }
    return 0;
}

int CentralWidget::cloneCurrentSession(MainWidgetGroup *group, QString name)
{
    if(group->sessionTab->count() == 0) return -1;
    QWidget *widget = group->sessionTab->currentWidget();
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    SessionsWindow *sessionsWindowClone = new SessionsWindow(sessionsWindow->getSessionType(),this);
    setGlobalOptions(sessionsWindowClone);
    sessionsWindowClone->setLongTitle(sessionsWindow->getLongTitle());
    sessionsWindowClone->setShortTitle(sessionsWindow->getShortTitle());
    sessionsWindowClone->setShowShortTitle(sessionsWindow->getShowShortTitle());
    int index = group->sessionTab->addTab(sessionsWindowClone->getMainWidget(), group->sessionTab->tabTitle(group->sessionTab->indexOf(widget)));
    connectSessionStateChange(group->sessionTab,index,sessionsWindowClone);
    if(name.isEmpty()) {
        name = sessionsWindow->getName();
        checkSessionName(name);
    }
    sessionsWindowClone->setName(name);
    sessionsWindowClone->cloneSession(sessionsWindow);
    sessionList.push_back(sessionsWindowClone);
    connect(sessionsWindowClone, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindowClone->setLongTitle(newTitle);
            QString workDir = getDirAndcheckeSysName(newTitle);
            if(!workDir.isEmpty()) {
                sessionsWindowClone->setShortTitle(workDir);
                if(workDir.startsWith("~/")) {
                    workDir = workDir.replace(0,1,QDir::homePath());
                }
                QFileInfo fileInfo(workDir);
                if(fileInfo.isDir()) {
                    sessionsWindowClone->setWorkingDirectory(workDir);
                }
            } else {
                sessionsWindowClone->setShortTitle(newTitle);
            }
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                int index = mainWidgetGroup->sessionTab->indexOf(sessionsWindowClone->getMainWidget());
                if(index >= 0) {
                    mainWidgetGroup->sessionTab->setTabText(index, sessionsWindowClone->getTitle());
                    break;
                }
            }
        }
    });
    group->sessionTab->setCurrentIndex(group->sessionTab->count()-1);
    return 0;
}

void CentralWidget::sessionWindow2InfoData(SessionsWindow *sessionsWindow, QuickConnectWindow::QuickConnectData &data, QString &name)
{
    name = sessionsWindow->getName();
    data.type = (QuickConnectWindow::QuickConnectType)sessionsWindow->getSessionType();
    switch(data.type) {
        case QuickConnectWindow::Telnet:
            data.TelnetData.hostname = sessionsWindow->getHostname();
            data.TelnetData.port = sessionsWindow->getPort();
            if(sessionsWindow->getSocketType() == QTelnet::TCP) {
                data.TelnetData.webSocket = "None";
            } else if(sessionsWindow->getSocketType() == QTelnet::WEBSOCKET) {
                data.TelnetData.webSocket = "Insecure";
            } else if(sessionsWindow->getSocketType() == QTelnet::SECUREWEBSOCKET) {
                data.TelnetData.webSocket = "Secure";
            }
            break;
        case QuickConnectWindow::Serial:
            data.SerialData.portName = sessionsWindow->getPortName();
            data.SerialData.baudRate = sessionsWindow->getBaudRate();
            data.SerialData.dataBits = sessionsWindow->getDataBits();
            data.SerialData.parity = sessionsWindow->getParity();
            data.SerialData.stopBits = sessionsWindow->getStopBits();
            data.SerialData.flowControl = sessionsWindow->getFlowControl();
            data.SerialData.xEnable = sessionsWindow->getXEnable();
            break;
        case QuickConnectWindow::LocalShell:
            data.LocalShellData.command = sessionsWindow->getCommand();
            break;
        case QuickConnectWindow::Raw:
            data.RawData.hostname = sessionsWindow->getHostname();
            data.RawData.port = sessionsWindow->getPort();
            break;
        case QuickConnectWindow::NamePipe:
            data.NamePipeData.pipeName = sessionsWindow->getPipeName();
            break;
        case QuickConnectWindow::SSH2:
            data.SSH2Data.hostname = sessionsWindow->getHostname();
            data.SSH2Data.port = sessionsWindow->getPort();
            data.SSH2Data.username = sessionsWindow->getUserName();
            data.SSH2Data.password = sessionsWindow->getPassWord();
            break;
        case QuickConnectWindow::VNC:
            data.VNCData.hostname = sessionsWindow->getHostname();
            data.VNCData.port = sessionsWindow->getPort();
            data.VNCData.password = sessionsWindow->getPassWord();
            break;
        default:
            break;
    }
}

int CentralWidget::setting2InfoData(GlobalSetting *settings, QuickConnectWindow::QuickConnectData &data, QString &name,bool skipPassword)
{
    name = settings->value("name").toString();
    data.type = (QuickConnectWindow::QuickConnectType)(settings->value("type").toInt());
    switch(data.type) {
    case QuickConnectWindow::Telnet:
        data.TelnetData.hostname = settings->value("hostname").toString();
        data.TelnetData.port = settings->value("port").toInt();
        data.TelnetData.webSocket = settings->value("socketType").toString();
        break;
    case QuickConnectWindow::Serial:
        data.SerialData.portName = settings->value("portName").toString();
        data.SerialData.baudRate = settings->value("baudRate").toInt();
        data.SerialData.dataBits = settings->value("dataBits").toInt();
        data.SerialData.parity = settings->value("parity").toInt();
        data.SerialData.stopBits = settings->value("stopBits").toInt();
        data.SerialData.flowControl = settings->value("flowControl").toBool();
        data.SerialData.xEnable = settings->value("xEnable").toBool();
        break;
    case QuickConnectWindow::LocalShell:
        data.LocalShellData.command = settings->value("command").toString();
        break;
    case QuickConnectWindow::Raw:
        data.RawData.hostname = settings->value("hostname").toString();
        data.RawData.port = settings->value("port").toInt();
        break;
    case QuickConnectWindow::NamePipe:
        data.NamePipeData.pipeName = settings->value("pipeName").toString();
        break;
    case QuickConnectWindow::SSH2:{
        data.SSH2Data.hostname = settings->value("hostname").toString();
        data.SSH2Data.port = settings->value("port").toInt();
        data.SSH2Data.username = settings->value("username").toString();
        if(!skipPassword){
            bool isOK = keyChainClass.readKey(name,data.SSH2Data.password);
            if(!isOK) {
                return -1;
            }
        }
        break;
    }
    case QuickConnectWindow::VNC: {
        data.VNCData.hostname = settings->value("hostname").toString();
        data.VNCData.port = settings->value("port").toInt();
        if(!skipPassword){
            bool isOK = keyChainClass.readKey(name,data.VNCData.password);
            if(!isOK) {
                return -1;
            }
        }
        break;
    }
    default:
        break;
    }
    return 0;
}

void CentralWidget::infoData2Setting(GlobalSetting *settings,const QuickConnectWindow::QuickConnectData &data,const QString &name,bool skipPassword) {
    settings->setValue("name",name);
    settings->setValue("type",data.type);
    switch(data.type) {
    case QuickConnectWindow::Telnet:
        settings->setValue("hostname",data.TelnetData.hostname);
        settings->setValue("port",data.TelnetData.port);
        settings->setValue("socketType",data.TelnetData.webSocket);
        break;
    case QuickConnectWindow::Serial:
        settings->setValue("portName",data.SerialData.portName);
        settings->setValue("baudRate",data.SerialData.baudRate);
        settings->setValue("dataBits",data.SerialData.dataBits);
        settings->setValue("parity",data.SerialData.parity);
        settings->setValue("stopBits",data.SerialData.stopBits);
        settings->setValue("flowControl",data.SerialData.flowControl);
        settings->setValue("xEnable",data.SerialData.xEnable);
        break;
    case QuickConnectWindow::LocalShell:
        settings->setValue("command",data.LocalShellData.command);
        break;
    case QuickConnectWindow::Raw:
        settings->setValue("hostname",data.RawData.hostname);
        settings->setValue("port",data.RawData.port);
        break;
    case QuickConnectWindow::NamePipe:
        settings->setValue("pipeName",data.NamePipeData.pipeName);
        break;
    case QuickConnectWindow::SSH2:
        settings->setValue("hostname",data.SSH2Data.hostname);
        settings->setValue("port",data.SSH2Data.port);
        settings->setValue("username",data.SSH2Data.username);
        if(!skipPassword)
            keyChainClass.writeKey(name,data.SSH2Data.password);
        break;
    case QuickConnectWindow::VNC:
        settings->setValue("hostname",data.VNCData.hostname);
        settings->setValue("port",data.VNCData.port);
        if(!skipPassword)
            keyChainClass.writeKey(name,data.VNCData.password);
        break;
    default:
        break;
    }
}

void CentralWidget::addBookmark(const QString &path)
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

QMenu *CentralWidget::createPopupMenu()
{
    QMenu *menu = new QMenu(this);
    menu->addAction(menuBarAction);
    menu->addAction(toolBarAction);
    menu->addAction(cmdWindowAction);
    menu->addAction(fullScreenAction);
    return menu;
}

void CentralWidget::appAbout(QWidget *parent)
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

void CentralWidget::appHelp(QWidget *parent)
{
    QMessageBox::about(parent, tr("Help"), 
        QString() + "<table border='0' width='100%'>" + 
        "<tr><td width='30%'>" + tr("Global Shortcuts:") + "</td><td width='50%'></td></tr>" + 
        "<tr><td>ALT+\"U\"</td><td>" + tr("show/hide menu bar") + "</td></tr>" +
        "<tr><td>ALT+\"T\"</td><td>" + tr("connect to LocalShell") + "</td></tr>" +
        "<tr><td>CTRL+SHIFT+\"T\"</td><td>" + tr("clone current session") + "</td></tr>" +
        "<tr><td>ALT+\"N\"</td><td>" + tr("switch ui to STD mode") + "</td></tr>" +
        "<tr><td>ALT+\"J\"</td><td>" + tr("switch ui to MINI mode") + "</td></tr>" +
        "<tr><td>ALT+\"-\"</td><td>" + tr("switch to previous session") + "</td></tr>" +
        "<tr><td>ALT+\"=\"</td><td>" + tr("switch to next session") + "</td></tr>" +
        "<tr><td>ALT+[num]</td><td>" + tr("switch to session [num]") + "</td></tr>" +
        "<tr><td>ALT+LEFT</td><td>" + tr("Go to line start") + "</td></tr>" +
        "<tr><td>ALT+RIGHT</td><td>" + tr("Go to line end") + "</td></tr>" +
        "</table>"
    );
}

void CentralWidget::checkCloseEvent(QCloseEvent *event)
{
    int activeSessionCount = 0;
    int lockedSessionCount = 0;
    foreach(SessionsWindow *sessionsWindow, sessionList) {
        if(sessionsWindow->getState() == SessionsWindow::Connected) {
            if(sessionsWindow->getSessionType() == SessionsWindow::LocalShell) {
                if(sessionsWindow->hasChildProcess()) {
                    activeSessionCount++;
                }
            } else {
                activeSessionCount++;
            }
        }
        if(sessionsWindow->isLocked()) {
            lockedSessionCount++;
        }
    }
    if(lockedSessionCount) {
        QMessageBox::warning(this,tr("Warning"),tr("There are sessions that have not yet been unlocked, please unlock them first."),QMessageBox::Ok);
        event->ignore();
        return;
    }
    if(activeSessionCount) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Warning"), tr("Are you sure to quit?"),
                                            QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes) {
            stopAllSession(true);
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void CentralWidget::checkStatusTipEvent(QStatusTipEvent *event) {
    if (!event->tip().isEmpty()) {
        ui->statusBar->showMessage(event->tip(), 2000);
    }
}

void CentralWidget::setAppLangeuage(QLocale lang) {
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
    switch(lang.language()) {
    case QLocale::Chinese:
        if(lang.script() == QLocale::SimplifiedChineseScript) {
            if(qtTranslator->load("qt_zh_CN.qm",qlibpath))
                qApp->installTranslator(qtTranslator);
            if(qtbaseTranslator->load("qtbase_zh_CN.qm",qlibpath))
                qApp->installTranslator(qtbaseTranslator);
            if(appTranslator->load(":/lang/lang/quardCRT_zh_CN.qm"))
                qApp->installTranslator(appTranslator);
            settings.setValue("Global/Startup/language","zh_CN");
        } else if (lang.script() == QLocale::TraditionalHanScript) {
            if(qtTranslator->load("qt_zh_TW.qm",qlibpath))
                qApp->installTranslator(qtTranslator);
            if(qtbaseTranslator->load("qtbase_zh_TW.qm",qlibpath))
                qApp->installTranslator(qtbaseTranslator);
            if(appTranslator->load(":/lang/lang/quardCRT_zh_HK.qm"))
                qApp->installTranslator(appTranslator);
            settings.setValue("Global/Startup/language","zh_HK");
        }
        break;
    case QLocale::Russian:
        if(qtTranslator->load("qt_ru.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_ru.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_ru_RU.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","ru_RU");
        break;
    case QLocale::Korean:
        if(qtTranslator->load("qt_ko.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_ko.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_ko_KR.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","ko_KR");
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
    case QLocale::French:
        if(qtTranslator->load("qt_fr.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_fr.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_fr_FR.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","fr_FR");
        break;
    case QLocale::Spanish:
        if(qtTranslator->load("qt_es.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_es.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_es_ES.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","es_ES");
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
}

MainWindow::MainWindow(QString dir, CentralWidget::StartupUIMode mode, QLocale lang, bool isDark, QWidget *parent) 
    : QGoodWindow(parent) {
    m_central_widget = new CentralWidget(dir,mode,lang,isDark,this);
    m_central_widget->setWindowFlags(Qt::Widget);

    m_good_central_widget = new QGoodCentralWidget(this);

#ifdef Q_OS_MAC
    //macOS uses global menu bar
    if(QApplication::testAttribute(Qt::AA_DontUseNativeMenuBar)) {
#else
    if(true) {
#endif
        m_menu_bar = m_central_widget->menuBar();

        //Set font of menu bar
        QFont font = m_menu_bar->font();
    #ifdef Q_OS_WIN
        font.setFamily("Segoe UI");
    #else
        font.setFamily(qApp->font().family());
    #endif
        m_menu_bar->setFont(font);

        QTimer::singleShot(0, this, [=]{
            const int title_bar_height = m_good_central_widget->titleBarHeight();
            m_menu_bar->setStyleSheet(QString("QMenuBar {height: %0px;}").arg(title_bar_height));
        });

        connect(m_good_central_widget,&QGoodCentralWidget::windowActiveChanged,this, [&](bool active){
            m_menu_bar->setEnabled(active);
        #ifdef Q_OS_MACOS
            fixWhenShowQuardCRTTabPreviewIssue();
        #endif
        });

        m_good_central_widget->setLeftTitleBarWidget(m_menu_bar);
        setNativeCaptionButtonsVisibleOnMac(false);
    }

    connect(qGoodStateHolder, &QGoodStateHolder::currentThemeChanged, this, [](){
        if (qGoodStateHolder->isCurrentThemeDark())
            QGoodWindow::setAppDarkTheme();
        else
            QGoodWindow::setAppLightTheme();
    });
    connect(this, &QGoodWindow::systemThemeChanged, this, [=]{
        qGoodStateHolder->setCurrentThemeDark(QGoodWindow::isSystemThemeDark());
    });
    qGoodStateHolder->setCurrentThemeDark(isDark);

    m_good_central_widget->setCentralWidget(m_central_widget);
    setCentralWidget(m_good_central_widget);

    setWindowIcon(QIcon(":/icons/icons/about.png"));
    setWindowTitle(m_central_widget->windowTitle());

    m_good_central_widget->setTitleAlignment(Qt::AlignCenter);
}

MainWindow::~MainWindow() {
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_central_widget->checkCloseEvent(event);
}

bool MainWindow::event(QEvent * event) {
    if(event->type() == QEvent::StatusTip) {
        m_central_widget->checkStatusTipEvent(static_cast<QStatusTipEvent *>(event));
        return true;
    }
    return QGoodWindow::event(event);
}
