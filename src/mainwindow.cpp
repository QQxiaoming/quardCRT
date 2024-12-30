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
#include <QColorDialog>
#include <QRandomGenerator>

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
#ifdef ENABLE_SSH
#include "sshsftp.h"
#endif
#include "misc.h"

#include "ui_mainwindow.h"

CentralWidget::CentralWidget(QString dir, StartupUIMode mode, QLocale lang, bool isDark,
     QString start_know_session, bool disable_plugin, QWidget *parent)
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

#ifdef ENABLE_PYTHON
    pyRun = new PyRun(this);
#endif

    m_mediaCaptureSession = new QMediaCaptureSession(this);
    m_windowCapture = new QWindowCapture(this);
    m_mediaRecorder = new QMediaRecorder(this);
    m_mediaCaptureSession->setWindowCapture(m_windowCapture);
    m_mediaCaptureSession->setRecorder(m_mediaRecorder);
    m_mediaRecorder->setQuality(QMediaRecorder::HighQuality);

    splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setHandleWidth(1);
    ui->centralwidget->layout()->addWidget(splitter);
    stackedWidget = new QStackedWidget(this);
    splitter->addWidget(stackedWidget);
    sessionManagerWidget = new SessionManagerWidget(this);
    stackedWidget->addWidget(sessionManagerWidget);
    pluginViewerWidget = new PluginViewerWidget(this);
    stackedWidget->addWidget(pluginViewerWidget);
    stackedWidget->hide();

    restoreSessionToSessionManager();

    splitterV1 = new QSplitter(Qt::Vertical,this);
    splitterV1->setHandleWidth(1);

    splitter->addWidget(splitterV1);
    splitterV2 = new QSplitter(Qt::Vertical,this);
    splitterV2->setHandleWidth(1);
    splitter->addWidget(splitterV2);
    splitterV11 = new QSplitter(Qt::Horizontal,this);
    splitterV11->setHandleWidth(1);
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitterV1->addWidget(mainWidgetGroupList.at(0));
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitterV2->addWidget(mainWidgetGroupList.at(1));
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitterV1->addWidget(mainWidgetGroupList.at(2));
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitterV2->addWidget(mainWidgetGroupList.at(3));
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitterV11->addWidget(mainWidgetGroupList.at(4));
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitterV11->addWidget(mainWidgetGroupList.at(5));
    mainWidgetGroupList.append(new MainWidgetGroup(MainWidgetGroup::EMBEDDED,this));
    splitter->addWidget(mainWidgetGroupList.at(6));
    splitterV1->addWidget(splitterV11);
    splitterV1->setVisible(true);
    splitterV2->setVisible(false);
    splitterV11->setVisible(false);
    mainWidgetGroupList.at(0)->setVisible(true);
    mainWidgetGroupList.at(1)->setVisible(false);
    mainWidgetGroupList.at(2)->setVisible(false);
    mainWidgetGroupList.at(3)->setVisible(false);
    mainWidgetGroupList.at(4)->setVisible(false);
    mainWidgetGroupList.at(5)->setVisible(false);
    mainWidgetGroupList.at(6)->setVisible(false);
    currentLayout = 0;

    notifictionWidget = new NotifictionWidget(this);
    splitter->addWidget(notifictionWidget);
    notifictionWidget->hide();

    splitter->setSizes(QList<int>() << 1 << 100000 << 100000 << 100000 << 1);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);
    splitter->setCollapsible(2,false);
    splitter->setCollapsible(3,false);
    splitter->setCollapsible(4,false);
    splitterV1->setCollapsible(0,false);
    splitterV1->setCollapsible(1,false);
    splitterV1->setCollapsible(2,false);
    splitterV2->setCollapsible(0,false);
    splitterV2->setCollapsible(1,false);
    splitterV11->setCollapsible(0,false);
    splitterV11->setCollapsible(1,false);
    auto syncSplitterMoved = [&](int index, int position) {
        Q_UNUSED(index);
        Q_UNUSED(position);
        if(!enabledSyncSplitterMoved) {
            return;
        }
        QSplitter *senderSplitter = qobject_cast<QSplitter*>(sender());
        if (!senderSplitter) {
            return;
        }
        QSplitter *otherSplitter = (senderSplitter == splitterV1) ? splitterV2 : splitterV1;
        QList<int> currentSizes;
        currentSizes.append(senderSplitter->sizes().at(0));
        currentSizes.append(senderSplitter->sizes().at(1));
        otherSplitter->setSizes(currentSizes);
    };
    connect(splitterV1, &QSplitter::splitterMoved, this, syncSplitterMoved);
    connect(splitterV2, &QSplitter::splitterMoved, this, syncSplitterMoved);

    quickConnectWindow = new QuickConnectWindow(this);
    quickConnectMainWidgetGroup = mainWidgetGroupList.at(0);

    lockSessionWindow = new LockSessionWindow(this);

#ifdef ENABLE_SSH
    sftpWindow = new SftpWindow(this);
#endif

    netScanWindow = new NetScanWindow(this);

    pluginInfoWindow = new PluginInfoWindow(this);

    startTftpSeverWindow = new StartTftpSeverWindow(this);
    tftpServer = new QTftp;

    keyMapManagerWindow = new keyMapManager(this);
    keyMapManagerWindow->setAvailableKeyBindings(QTermWidget::availableKeyBindings());

    internalCommandWindow = new InternalCommandWindow(this);

    globalOptionsWindow = new GlobalOptionsWindow(this);

    sessionOptionsWindow = new SessionOptionsWindow(this);

    hexViewWindow = new HexViewWindow(this);

    sessionManagerPushButton = new QPushButton();
    sessionManagerPushButton->setFixedSize(250,26);
    pluginViewerPushButton = new QPushButton();
    pluginViewerPushButton->setFixedSize(250,26);
    sideHboxLayout = new QHBoxLayout();
    sideHboxLayout->setContentsMargins(0,0,0,0);
    sideHboxLayout->setSpacing(2);
    sideProxyWidget = new QWidget();
    sideProxyWidget->setLayout(sideHboxLayout);
    sideHboxLayout->addWidget(pluginViewerPushButton);
    sideHboxLayout->addWidget(sessionManagerPushButton);
    QGraphicsScene *scene = new QGraphicsScene(this);
    QGraphicsProxyWidget *w = scene->addWidget(sideProxyWidget);
    w->setPos(0,0);
    w->setRotation(-90);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setFrameStyle(0);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setFixedSize(30, 502);
    ui->sidewidget->setFixedWidth(30);
    swapSideHboxLayout();

    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        mainWidgetGroup->sessionTab->setScrollTitleMode((SessionTab::TitleScrollMode)globalOptionsWindow->getTabTitleMode());
        mainWidgetGroup->sessionTab->setTitleWidth(globalOptionsWindow->getTabTitleWidth());
        mainWidgetGroup->sessionTab->setPreviewEnabled(globalOptionsWindow->getEnableTabPreview());
        mainWidgetGroup->sessionTab->setPreviewWidth(globalOptionsWindow->getTabPreviewWidth());
    }

    menuAndToolBarInit(disable_plugin);

    /* connect signals */
    menuAndToolBarConnectSignals();

    connect(sessionManagerPushButton,&QPushButton::clicked,this,[&](){
        if(stackedWidget->isHidden()) {
            stackedWidget->setCurrentWidget(sessionManagerWidget);
            stackedWidget->show();
        } else {
            if(stackedWidget->currentWidget() == sessionManagerWidget) {
                stackedWidget->hide();
            } else {
                stackedWidget->setCurrentWidget(sessionManagerWidget);
            }
        }
    });
    connect(pluginViewerPushButton,&QPushButton::clicked,this,[&](){
        if(stackedWidget->isHidden()) {
            stackedWidget->setCurrentWidget(pluginViewerWidget);
            stackedWidget->show();
        } else {
            if(stackedWidget->currentWidget() == pluginViewerWidget) {
                stackedWidget->hide();
            } else {
                stackedWidget->setCurrentWidget(pluginViewerWidget);
            }
        }
    });
    connect(startTftpSeverWindow,&StartTftpSeverWindow::setTftpInfo,this,[&](int port, const QString &upDir, const QString &downDir){
        if(tftpServer->isRunning()) {
            tftpServer->stopServer();
        }
        tftpServer->setUpDir(upDir);
        tftpServer->setDownDir(downDir);
        tftpServer->setPort(port);
        tftpServer->startServer();
    });
    connect(tftpServer,&QTftp::serverRuning,this,[&](bool runing){
        startTFTPServerAction->setChecked(runing);
    });
    connect(tftpServer,&QTftp::error,this,[&](int error){
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
    connect(pluginInfoWindow, &PluginInfoWindow::pluginEnableStateChanged, this, [=](QString name, bool enable){
        for(int i=0;i<pluginList.size();i++) {
            pluginState_t *pluginStruct = &pluginList[i];
            PluginInterface *iface = pluginStruct->iface;
            if(iface->name() == name) {
                pluginStruct->state = enable;
                GlobalSetting settings;
                settings.setValue("Global/Plugin/"+name+"/State",enable);
                QMap<QString,void *> map = iface->metaObject();
                foreach (QString key, map.keys()) {
                    if(key == "MainMenuAction" || key == "QAction") {
                        QAction *action = (QAction *)map.value(key);
                        action->setVisible(enable);
                    } else if(key == "MainMenuMenu" || key == "QMenu") {
                        QMenu *menu = (QMenu *)map.value(key);
                        menu->menuAction()->setVisible(enable);
                    } else if(key == "MainWidget" || key == "QWidget") {
                        pluginViewerWidget->setPluginVisible(iface->name(),enable);
                    } 
                }
                break;
            }
        }
    });
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        connect(mainWidgetGroup->sessionTab,&FancyTabWidget::tabAddRequested,this,[=](){
            QWidget *widget = findCurrentFocusWidget();
            if(widget != nullptr) {
                if(mainWidgetGroup->sessionTab->indexOf(widget)<0) {
                    int index = mainWidgetGroup->sessionTab->count();
                    QWidget *widget = mainWidgetGroup->sessionTab->widget(index);
                    widget->setFocus();
                }
            } else {
                int index = mainWidgetGroup->sessionTab->count();
                QWidget *widget = mainWidgetGroup->sessionTab->widget(index);
                widget->setFocus();
            }

            int mode = globalOptionsWindow->getNewTabMode();
            switch(mode) {
                case 0:
                    quickConnectWindow->show();
                    break;
                default:
                case 1:
                    cloneSessionAction->trigger();
                    break;
                case 2:
                    widget = findCurrentFocusWidget();
                    if(widget != nullptr) {
                        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
                        if(SessionsWindow::LocalShell == sessionsWindow->getSessionType()) {
                            cloneSessionAction->trigger();
                            return;
                        }
                    }
                    connectLocalShellAction->trigger();
                    break;
            }
        });
        connect(mainWidgetGroup->sessionTab,&FancyTabWidget::tabCloseRequested,this,[=](int index){
            stopSession(mainWidgetGroup,index);
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::currentChanged,this,[=](int index){
            int currentTabNum = 0;
            foreach(MainWidgetGroup *group, mainWidgetGroupList) {
                int num = group->sessionTab->count();
                if(num == 0) {
                    group->setActive(false);
                }
                currentTabNum += num;
            }
            setSessionClassActionEnable(currentTabNum != 0);
            if(index > 0) {
                QWidget *widget = (QWidget *)mainWidgetGroup->sessionTab->widget(index);
                foreach(SessionsWindow *sessionsWindow, sessionList) {
                    sessionsWindow->setScrollBarPosition(verticalScrollBarAction->isChecked()?QTermWidget::ScrollBarRight:QTermWidget::NoScrollBar);
                    disconnect(sessionsWindow,&SessionsWindow::hexDataDup,
                                hexViewWindow,&HexViewWindow::recvData);
                    if(sessionsWindow->getMainWidget() == widget) {
                        logSessionAction->setChecked(sessionsWindow->isLog());
                        rawLogSessionAction->setChecked(sessionsWindow->isRawLog());
                    #ifdef ENABLE_PYTHON
                        bool isRS = sessionsWindow->isRecordingScript();
                        startRecordingScriptAction->setEnabled(!isRS);
                        stopRecordingScriptAction->setEnabled(isRS);
                        canlcelRecordingScriptAction->setEnabled(isRS);
                    #endif
                        if(hexViewAction->isChecked()) {
                            connect(sessionsWindow,&SessionsWindow::hexDataDup,
                                        hexViewWindow,&HexViewWindow::recvData);
                        }
                        receiveASCIIAction->setChecked(sessionsWindow->isReceiveASCIIFile());
                    }
                }
            } else {
            #ifdef ENABLE_PYTHON
                startRecordingScriptAction->setEnabled(false);
                stopRecordingScriptAction->setEnabled(false);
                canlcelRecordingScriptAction->setEnabled(false);
            #endif
            }
            refreshStatusBar();
            refreshTagColor();
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::showContextMenu,this,[=](int index, const QPoint& position){
            QMenu *menu = new QMenu(this);
            if(index >= 0) {
                if(mainWidgetGroup->sessionTab->currentIndex() != index) {
                    delete menu;
                    mainWidgetGroup->sessionTab->setCurrentIndex(index-1);
                    return;
                }
                QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                if(sessionsWindow->isLocked()) {
                    QAction *unlockAction = new QAction(tr("Unlock Session"),this);
                    unlockAction->setStatusTip(tr("Unlock current session"));
                    menu->addAction(unlockAction);
                    connect(unlockAction,&QAction::triggered,this,[=](){
                        willLockUnLockSessions = sessionsWindow;
                        lockSessionWindow->showUnlock();
                    });
                } else {
                    QAction *moveToAnotherTabAction = new QAction(tr("Move to another Tab"),this);
                    moveToAnotherTabAction->setStatusTip(tr("Move to current session to another tab group"));
                    menu->addAction(moveToAnotherTabAction);
                    connect(moveToAnotherTabAction,&QAction::triggered,this,[=](){
                        int currentGroupID = mainWidgetGroupList.indexOf(mainWidgetGroup);
                        moveToAnotherTab(currentGroupID,nextGroupID(currentGroupID),index);
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
                    #ifdef ENABLE_SSH
                    if(sessionsWindow->getSessionType() == SessionsWindow::SSH2) {
                        QAction *openSFtpAction = new QAction(tr("Open SFTP"),this);
                        openSFtpAction->setStatusTip(tr("Open SFTP in a new window"));
                        menu->addAction(openSFtpAction);
                        menu->addSeparator();
                        connect(openSFtpAction,&QAction::triggered,this,[=](){
                            SshSFtp *sftp = sessionsWindow->getSshSFtpChannel();
                            if(sftp == nullptr) {
                                QMessageBox::warning(this, tr("Warning"), tr("No SFTP channel!"));
                                return;
                            }
                            sftpWindow->setSftpChannel(sftp);
                            sftpWindow->show();
                        });
                    }
                    #endif
                    QAction *addToBroadCastSessionListAction = new QAction(tr("Add to BroadCast List"),this);
                    addToBroadCastSessionListAction->setStatusTip(tr("Add current session to broadcast list"));
                    addToBroadCastSessionListAction->setCheckable(true);
                    addToBroadCastSessionListAction->setChecked(sessionsWindow->isInBroadCastList());
                    if(sessionsWindow->isInBroadCastList()) {
                        addToBroadCastSessionListAction->setText(tr("Remove from BroadCast List"));
                        addToBroadCastSessionListAction->setStatusTip(tr("Remove current session from broadcast list"));
                    }
                    menu->addAction(addToBroadCastSessionListAction);
                    connect(addToBroadCastSessionListAction,&QAction::triggered,this,[=](){
                        sessionsWindow->setInBroadCastList(!sessionsWindow->isInBroadCastList());
                        if(sessionsWindow->isInBroadCastList()) {
                            if(!broadCastSessionList.contains(sessionsWindow)){
                                broadCastSessionList.append(sessionsWindow);
                            }
                        } else {
                            broadCastSessionList.removeOne(sessionsWindow);
                        }
                    });
                    QAction *lockAction = new QAction(tr("Lock Session"),this);
                    lockAction->setStatusTip(tr("Lock current session"));
                    menu->addAction(lockAction);
                    connect(lockAction,&QAction::triggered,this,[=](){
                        willLockUnLockSessions = sessionsWindow;
                        lockSessionWindow->showLock();
                    });
                    QAction *saveSessionAction = new QAction(tr("Save Session"),this);
                    saveSessionAction->setStatusTip(tr("Save current session to session manager"));
                    menu->addAction(saveSessionAction);
                    connect(saveSessionAction,&QAction::triggered,this,[=](){
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

                    QMenu *tagMenu = new QMenu(tr("Tag"),this);
                    menu->addMenu(tagMenu);
                    QActionGroup *tagGroup = new QActionGroup(this);

                    QAction *whiteTagAction = new QAction("#FFFFFF",this);
                    whiteTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::white));
                    whiteTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(whiteTagAction);
                    QAction *redTagAction = new QAction("#FF0000",this);
                    redTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::red));
                    redTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(redTagAction);
                    QAction *greenTagAction = new QAction("#00FF00",this);
                    greenTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::green));
                    greenTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(greenTagAction);
                    QAction *blueTagAction = new QAction("#0000FF",this);
                    blueTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::blue));
                    blueTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(blueTagAction);
                    QAction *yellowTagAction = new QAction("#FFFF00",this);
                    yellowTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::yellow));
                    yellowTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(yellowTagAction);
                    QAction *magentaTagAction = new QAction("#FF00FF",this);
                    magentaTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::magenta));
                    magentaTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(magentaTagAction);
                    QAction *cyanTagAction = new QAction("#00FFFF",this);
                    cyanTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::cyan));
                    cyanTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(cyanTagAction);
                    QAction *grayTagAction = new QAction("#808080",this);
                    grayTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::gray));
                    grayTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(grayTagAction);
                    QAction *blackTagAction = new QAction("#000000",this);
                    blackTagAction->setIcon(QFontIcon::icon(QChar(0xf02e), Qt::black));
                    blackTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(blackTagAction);
                    QAction *customizeTagAction = new QAction(tr("Customize"),this);
                    customizeTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(customizeTagAction);
                    QAction *cancelTagAction = new QAction(tr("Cancel"),this);
                    cancelTagAction->setActionGroup(tagGroup);
                    tagMenu->addAction(cancelTagAction);

                    connect(tagGroup,&QActionGroup::triggered,this,[=](QAction *action){
                        QString colorStr = action->text();
                        if(action == cancelTagAction) {
                            sessionsWindow->setTagColor(false);
                        } else if(action == customizeTagAction) {
                            QColor color = QColorDialog::getColor(sessionsWindow->getTagColorValue(),this);
                            if(color.isValid()) {
                                sessionsWindow->setTagColor(true,color);
                            }
                        } else {
                            uint8_t r = QStringView{colorStr}.mid(1,2).toUInt(nullptr,16);
                            uint8_t g = QStringView{colorStr}.mid(3,2).toUInt(nullptr,16);
                            uint8_t b = QStringView{colorStr}.mid(5,2).toUInt(nullptr,16);
                            sessionsWindow->setTagColor(true,QColor(r,g,b));
                        }
                        refreshTagColor();
                    });

                    QAction *propertiesAction = new QAction(tr("Properties"),this);
                    propertiesAction->setStatusTip(tr("Show current session properties"));
                    menu->addAction(propertiesAction);
                    menu->addSeparator();
                    connect(propertiesAction,&QAction::triggered,this,[=](){
                        if(sessionsWindow->isLocked()) {
                            return;
                        }
                        QuickConnectWindow::QuickConnectData data;
                        QString name;
                        sessionWindow2InfoData(sessionsWindow,data,name);
                        sessionOptionsWindow->setSessionProperties(name,data);
                        sessionOptionsWindow->setSessionState(sessionsWindow->getStateInfo());
                        sessionOptionsWindow->setReadOnly(true);
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
                                if(stopSession(mainWidgetGroup,i) == -1){
                                    return;
                                }
                            }
                        }
                    });
                    QAction *closeToTheRightAction = new QAction(tr("Close to the Right"),this);
                    closeToTheRightAction->setStatusTip(tr("Close sessions to the right"));
                    menu->addAction(closeToTheRightAction);
                    connect(closeToTheRightAction,&QAction::triggered,this,[=](){
                        int count = mainWidgetGroup->sessionTab->count();
                        for(int i=count;i>index;i--) {
                            if(stopSession(mainWidgetGroup,i) == -1){
                                return;
                            }
                        }
                    });
                    QAction *closeAllAction = new QAction(tr("Close All"),this);
                    closeAllAction->setStatusTip(tr("Close all sessions"));
                    menu->addAction(closeAllAction);
                    connect(closeAllAction,&QAction::triggered,this,[=](){
                        while(mainWidgetGroup->sessionTab->count() > 0) {
                            if(stopSession(mainWidgetGroup,mainWidgetGroup->sessionTab->count()) == -1){
                                return;
                            }
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
            QRect screenGeometry = QGuiApplication::screenAt(cursor().pos())->geometry();
            QPoint pos = cursor().pos() + QPoint(5,5);
            if (pos.x() + menu->width() > screenGeometry.right()) {
                pos.setX(screenGeometry.right() - menu->width());
            }
            if (pos.y() + menu->height() > screenGeometry.bottom()) {
                pos.setY(screenGeometry.bottom() - menu->height());
            }
            menu->popup(pos);
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::dragTabMoved,this,[=](int from, int to, SessionTab *toTab){
            if(from <= 0) return;
            if(toTab == mainWidgetGroup->sessionTab) {
                if(from == to) return;
                if(to == -1) {
                    floatingWindow(mainWidgetGroup,from);
                }
            } else {
                int currentGroupID = mainWidgetGroupList.indexOf(mainWidgetGroup);
                for(int i=0;i<mainWidgetGroupList.size();i++) {
                    if(mainWidgetGroupList[i]->sessionTab == toTab) {
                        moveToAnotherTab(currentGroupID,i,from);    
                        break;
                    }
                }
            }
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabMoved,this,[=](int from, int to){
            // Tab move in Group internal, we just need to update TagColor
            refreshTagColor();
            Q_UNUSED(from);
            Q_UNUSED(to);
        });
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabBarDoubleClicked,this,[=](int index){
            QWidget *widget = mainWidgetGroup->sessionTab->widget(index);
            SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
            if(sessionsWindow->isLocked()) return;
            SessionsWindow::ShowTitleType type = sessionsWindow->getShowTitleType();
            type = static_cast<SessionsWindow::ShowTitleType>(type+1);
            if(type == SessionsWindow::TitleTypeMax) {
                type = static_cast<SessionsWindow::ShowTitleType>(0);
            }
            sessionsWindow->setShowTitleType(type);
            mainWidgetGroup->sessionTab->setTabText(index,sessionsWindow->getTitle());
        });
    #if defined(Q_OS_MACOS)
        connect(mainWidgetGroup->sessionTab,&SessionTab::tabPreviewShow,this,[&](int index){
            Q_UNUSED(index);
            if(mainWindow) {
                mainWindow->fixWhenShowQuardCRTTabPreviewIssue();
            }
        });
    #endif
        connect(mainWidgetGroup->commandWidget, &CommandWidget::sendData, this, [=](const QByteArray &data, int sendMode) {
            if(mainWidgetGroup->sessionTab->count() != 0) {
                if(sendMode == 0) {
                    QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
                    if(sessionsWindow->isLocked()) return;
                    sessionsWindow->proxySendData(data);
                } else if(sendMode == 1)  {
                    int count = mainWidgetGroup->sessionTab->count();
                    for(int i=0;i<count;i++) {
                        QWidget *widget = mainWidgetGroup->sessionTab->widget(i+1);
                        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
                        if(sessionsWindow->isLocked()) continue;
                        sessionsWindow->proxySendData(data);
                    }
                } else if(sendMode == 2)  {
                    foreach(SessionsWindow *sessionsWindow, sessionList) {
                        if(sessionsWindow->isLocked()) continue;
                        sessionsWindow->proxySendData(data);
                    }
                }
            }
        });
        connect(mainWidgetGroup,&MainWidgetGroup::getFocus,this,[=](){
            mainWidgetGroup->setActive(true && (currentLayout!=0));
        });
        connect(mainWidgetGroup,&MainWidgetGroup::lostFocus,this,[=](){
            mainWidgetGroup->setActive(false);
        });
    }
    connect(sessionManagerWidget,&SessionManagerWidget::sessionManagerHide,this,[&](){
        stackedWidget->hide();
    });
    connect(sessionManagerWidget,&SessionManagerWidget::sessionConnect,this,[&](QString str){
        connectSessionFromSessionManager(str);
    });
    connect(sessionManagerWidget,&SessionManagerWidget::sessionRemove,this,[&](QString str){
        removeSessionFromSessionManager(str);
    });
    connect(sessionManagerWidget,&SessionManagerWidget::sessionShowProperties,this,[=](QString str){
        GlobalSetting settings;
        int size = settings.beginReadArray("Global/Session");
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            QString current_name = settings.value("name").toString();
            QString current_group = settings.value("group", "/").toString();
            if(current_name == str) {
                QuickConnectWindow::QuickConnectData data;
                if(setting2InfoData(&settings, data, current_name,current_group) != 0) {
                    QMessageBox::warning(this, tr("Warning"), tr("Session properties error!"));
                    return;
                }
                sessionOptionsWindow->setSessionProperties(str,data);
                bool readOnly = false;
                foreach(SessionsWindow *session, sessionList){
                    if(session->getName() == str) {
                        readOnly = true;
                        break;
                    }
                }
                sessionOptionsWindow->setReadOnly(readOnly);
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
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->setZmodemOnlie(!globalOptionsWindow->getDisableZmodemOnline());
            sessionsWindow->setZmodemUploadPath(globalOptionsWindow->getModemUploadPath());
            sessionsWindow->setZmodemDownloadPath(globalOptionsWindow->getModemDownloadPath());
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
    connect(shortcutMenuBarView,&QShortcut::activated,this,[&](){
        menuBarAction->trigger();
    });
    shortcutMenuBarView->setEnabled(false);
    shortcutConnectLocalShell = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_T),this);
    connect(shortcutConnectLocalShell,&QShortcut::activated,this,[&](){
        connectLocalShellAction->trigger();
    });
    shortcutConnectLocalShell->setEnabled(false);
    shortcutCloneSession = new QShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_T),this);
    connect(shortcutCloneSession,&QShortcut::activated,this,[&](){
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
                    startTelnetSession(findCurrentFocusGroup(),-1,hostname,port.toInt(),QTelnet::TCP);
                } else if(type == "localshell") {
                    startLocalShellSession(findCurrentFocusGroup(),-1);
                } else if(type == "raw") {
                    startRawSocketSession(findCurrentFocusGroup(),-1,hostname,port.toInt());
                } else if(type == "namepipe") {
                    startNamePipeSession(findCurrentFocusGroup(),-1,hostname);
                } else if(type == "ssh") {
                    //TODO:start in connectAddress cmd
                } else if(type == "serial") {
                    //TODO:start in connectAddress cmd
                } else if(type == "vnc") {
                    //TODO:start in connectAddress cmd
                }
            }
        }
    });
    shortcutExitFullScreen = new QShortcut(QKeySequence(Qt::Key_Escape),this);
    connect(shortcutExitFullScreen,&QShortcut::activated,this,[=](){
        if(fullScreenAction->isChecked()) {
            fullScreenAction->trigger();
        }
    });
#ifdef ENABLE_PYTHON
    connect(pyRun,&PyRun::runScriptStarted,this,[&](void){
        bool runing = pyRun->isRunning();
        runAction->setEnabled(!runing);
        cancelAction->setEnabled(runing);
    });
    connect(pyRun,&PyRun::runScriptFinished,this,[&](void){
        bool runing = pyRun->isRunning();
        runAction->setEnabled(!runing);
        cancelAction->setEnabled(runing);
    });
    internalCommandWindow->setPyRun(pyRun);
#else
    runAction->setEnabled(false);
    cancelAction->setEnabled(false);
#endif
    #if defined(Q_OS_MACOS)
    if(mainWindow) {
        connect(mainWindow,&MainWindow::macosWindowWillEnterFullScreen,this,[=](){
            fullScreenAction->setChecked(true);
        });
        connect(mainWindow,&MainWindow::macosWindowDidEnterFullScreen,this,[=](){
            fullScreenAction->setChecked(false);
        });
    }
    #endif

    statusBarMessage = new QLabel(tr("Ready"),ui->statusBar);
    ui->statusBar->addPermanentWidget(statusBarMessage,100);
    connect(ui->statusBar,&QStatusBar::messageChanged,this,[&](const QString &message){
        statusBarMessage->setText(message);
    });
    
    ui->statusBar->setFixedHeight(28);
    statusBarWidget = new StatusBarWidget(ui->statusBar);
    ui->statusBar->addPermanentWidget(statusBarWidget,0);
    statusBarWidgetRefreshTimer = new QTimer(this);
    statusBarWidgetRefreshTimer->setInterval(1000);
    connect(statusBarWidgetRefreshTimer,&QTimer::timeout,this,[&](){
        refreshStatusBar();
    });
    statusBarWidgetRefreshTimer->start();
    ui->statusBar->setSizeGripEnabled(false);

    connect(notifictionWidget,&NotifictionWidget::notifictionChanged,this,[&](uint32_t count){
        statusBarWidget->setNotifiction(count?true:false);
    });
    connect(statusBarWidget,&StatusBarWidget::notifictionTriggered,this,[&](){
        if(notifictionWidget->isHidden()) {
            notifictionWidget->show();
        } else {
            notifictionWidget->hide();
        }
    });
    connect(statusBarWidget,&StatusBarWidget::endOfLineTriggered,this,[&](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(!sessionsWindow->isLocked()) {
            QMenu *endOfLineSeqMenu = new QMenu(tr("End of line sequence"),this);
            QActionGroup *endOfLineSeqGroup = new QActionGroup(this);
            QAction *endOfLineSeqAutoAction = new QAction(tr("Auto"),this);
            endOfLineSeqAutoAction->setToolTip(tr("Auto detect end of line sequence"));
            endOfLineSeqAutoAction->setActionGroup(endOfLineSeqGroup);
            endOfLineSeqMenu->addAction(endOfLineSeqAutoAction);
            QAction *endOfLineSeqLFAction = new QAction("LF",this);
            endOfLineSeqLFAction->setToolTip(tr("Line Feed"));
            endOfLineSeqLFAction->setActionGroup(endOfLineSeqGroup);
            endOfLineSeqMenu->addAction(endOfLineSeqLFAction);
            QAction *endOfLineSeqCRAction = new QAction("CR",this);
            endOfLineSeqCRAction->setToolTip(tr("Carriage Return"));
            endOfLineSeqCRAction->setActionGroup(endOfLineSeqGroup);
            endOfLineSeqMenu->addAction(endOfLineSeqCRAction);
            QAction *endOfLineSeqLFLFAction = new QAction("LFLF",this);
            endOfLineSeqLFLFAction->setToolTip(tr("Double Line Feed"));
            endOfLineSeqLFLFAction->setActionGroup(endOfLineSeqGroup);
            endOfLineSeqMenu->addAction(endOfLineSeqLFLFAction);
            QAction *endOfLineSeqCRCRAction = new QAction("CRCR",this);
            endOfLineSeqCRCRAction->setToolTip(tr("Double Carriage Return"));
            endOfLineSeqCRCRAction->setActionGroup(endOfLineSeqGroup);
            endOfLineSeqMenu->addAction(endOfLineSeqCRCRAction);
            connect(endOfLineSeqGroup,&QActionGroup::triggered,this,[=](QAction *action){
                int index = endOfLineSeqGroup->actions().indexOf(action);
                sessionsWindow->setEndOfLineSeq(static_cast<SessionsWindow::EndOfLineSeq>(index));
            });
            endOfLineSeqMenu->popup(QCursor::pos());
        }
    });
    connect(statusBarWidget,&StatusBarWidget::logsTriggered,this,[&](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(!sessionsWindow->isLocked()) {
            QMenu *logsMenu = new QMenu(tr("Logs"),this);
            if(sessionsWindow->isLog()) {
                QAction *openLogAction = new QAction(tr("Open Log"),this);
                openLogAction->setToolTip(tr("Open log file"));
                logsMenu->addAction(openLogAction);
                connect(openLogAction,&QAction::triggered,this,[=](){
                    QString logFile = sessionsWindow->getLogFileName();
                    if(!logFile.isEmpty()) {
                        QDesktopServices::openUrl(QUrl::fromLocalFile(logFile));
                    }
                });
                QAction *stopLogAction = new QAction(tr("Stop Log"),this);
                stopLogAction->setToolTip(tr("Stop log to file"));
                logsMenu->addAction(stopLogAction);
                connect(stopLogAction,&QAction::triggered,this,[=](){
                    logSessionAction->setChecked(sessionsWindow->setLog(false) != 0);
                });
            } else {
                QAction *startLogAction = new QAction(tr("Start Log"),this);
                startLogAction->setToolTip(tr("Start log to file"));
                logsMenu->addAction(startLogAction);
                connect(startLogAction,&QAction::triggered,this,[=](){
                    logSessionAction->setChecked(sessionsWindow->setLog(true) == 0);
                });
            }
            if(sessionsWindow->isRawLog()) {
                QAction *openRawLogAction = new QAction(tr("Open Raw Log"),this);
                openRawLogAction->setToolTip(tr("Open raw log file"));
                logsMenu->addAction(openRawLogAction);
                connect(openRawLogAction,&QAction::triggered,this,[=](){
                    QString logFile = sessionsWindow->getRawLogFileName();
                    if(!logFile.isEmpty()) {
                        QDesktopServices::openUrl(QUrl::fromLocalFile(logFile));
                    }
                });
                QAction *stopRawLogAction = new QAction(tr("Stop Raw Log"),this);
                stopRawLogAction->setToolTip(tr("Stop raw log to file"));
                logsMenu->addAction(stopRawLogAction);
                connect(stopRawLogAction,&QAction::triggered,this,[=](){
                    rawLogSessionAction->setChecked(sessionsWindow->setRawLog(false) != 0);
                });
            } else {
                QAction *startRawLogAction = new QAction(tr("Start Raw Log"),this);
                startRawLogAction->setToolTip(tr("Start raw log to file"));
                logsMenu->addAction(startRawLogAction);
                connect(startRawLogAction,&QAction::triggered,this,[=](){
                    rawLogSessionAction->setChecked(sessionsWindow->setRawLog(true) == 0);
                });
            }

            if(logsMenu->isEmpty()) {
                delete logsMenu;
                return;
            }
            logsMenu->popup(QCursor::pos());
        }
    });

    initSysEnv();

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
        if(dir.isEmpty() && start_know_session.isEmpty()) {
            connectLocalShellAction->trigger();
        }
    }
    QTimer::singleShot(500, this, [=](){
        if(!start_know_session.isEmpty()) {
            connectSessionFromSessionManager(start_know_session);
        } else if(!dir.isEmpty()) {
            startLocalShellSession(mainWidgetGroupList.at(0),-1,QString(),dir);
        }
    });

    QTimer::singleShot(0, this, [&](){
        if(mainWindow) {
            mainWindow->fixMenuBarWidth();
        #if defined(Q_OS_LINUX)
            // FIXME: only for linux, this is a bad hack, but it works
            connect(mainWindow,&QGoodWindow::fixIssueWindowEvent,this,[&](QEvent::Type type){
                if(type == QEvent::Resize) {
                    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                        if(mainWidgetGroup->type() == MainWidgetGroup::EMBEDDED) {
                            QWidget *widget = mainWidgetGroup->sessionTab->currentWidget();
                            if(widget != nullptr) {
                                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                                if(sessionsWindow != nullptr) {
                                    sessionsWindow->repaintDisplay();
                                }
                            }
                        }
                    }
                }
            });
        #endif
        }
    });

    // TODO:Unimplemented functions are temporarily closed
    createPublicKeyAction->setEnabled(false);
    publickeyManagerAction->setEnabled(false);
    tileAction->setEnabled(false);
    cascadeAction->setEnabled(false);
}

CentralWidget::~CentralWidget() {
    saveSettings();
    stopAllSession(true);
    if(tftpServer->isRunning()) {
        tftpServer->stopServer();
    }
    delete tftpServer;
    delete sessionManagerPushButton;
    delete pluginViewerPushButton;
    delete sideProxyWidget;
    delete hexViewWindow;
#ifdef ENABLE_PYTHON
    delete pyRun;
#endif
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
    int newIndex = mainWidgetGroupList.at(dst)->sessionTab->addTab(-1, widget, text);
    mainWidgetGroupList.at(dst)->sessionTab->setTabIcon(newIndex,icon);
    mainWidgetGroupList.at(dst)->sessionTab->setCurrentIndex(
        mainWidgetGroupList.at(dst)->sessionTab->count()-1);
    mainWidgetGroupList.at(src)->sessionTab->setCurrentIndex(
        mainWidgetGroupList.at(src)->sessionTab->count()-1);
    QTimer::singleShot(1000, this, [=](){
        sessionsWindow->setShowResizeNotificationEnabled(true);
        sessionsWindow->refeshTermSize();
    });
    refreshTagColor();
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
    menu->addAction(clearScrollbackAction);
    menu->addAction(clearScreenAction);
    menu->addAction(clearScreenAndScrollbackAction);
    menu->addSeparator();
    QString text = term->selectedText();
    QMenu *highlightMenu = new QMenu(tr("Highlight"),this);
    menu->addMenu(highlightMenu);
    if(!text.isEmpty()) {
        QAction *highlightAction = new QAction(tr("Highlight"),this);
        highlightAction->setStatusTip(tr("Highlight selected text with random color"));
        highlightMenu->addAction(highlightAction);
        connect(highlightAction,&QAction::triggered,this,[=](){
            if(term->isContainHighLightText(text)) {
                return;
            }
            uint8_t r = QRandomGenerator::global()->bounded(256);
            uint8_t g = QRandomGenerator::global()->bounded(256);
            uint8_t b = QRandomGenerator::global()->bounded(256);
            lastHighlightColor = QColor(r,g,b);
            term->addHighLightText(text, lastHighlightColor);
        });
        QAction *highlightCustomAction = new QAction(tr("Highlight (Custom)"),this);
        highlightCustomAction->setStatusTip(tr("Highlight selected text with custom color"));
        highlightMenu->addAction(highlightCustomAction);
        connect(highlightCustomAction,&QAction::triggered,this,[=](){
            if(term->isContainHighLightText(text)) {
                return;
            }
            QColor color = QColorDialog::getColor(lastHighlightColor,this);
            if(color.isValid()) {
                lastHighlightColor = color;
                term->addHighLightText(text, color);
            }
        });
        QAction *unhighlightAction = new QAction(tr("Unhighlight"),this);
        unhighlightAction->setStatusTip(tr("Unhighlight selected text"));
        highlightMenu->addAction(unhighlightAction);
        connect(unhighlightAction,&QAction::triggered,this,[=](){
            term->removeHighLightText(text);
        });
    }
    QAction *clearHighlightAction = new QAction(tr("Clear Highlights"),this);
    clearHighlightAction->setStatusTip(tr("Clear all highlighted text"));
    highlightMenu->addAction(clearHighlightAction);
    connect(clearHighlightAction,&QAction::triggered,this,[=](){
        term->clearHighLightTexts();
    });
    QMap<QString, QColor> highLightTextList = term->getHighLightTexts();
    if(!highLightTextList.isEmpty()) {
        highlightMenu->addSeparator();
        foreach(QString text, highLightTextList.keys()) {
            QMenu *subMenu = new QMenu(text,highlightMenu);
            subMenu->setIcon(QFontIcon::icon(QChar(0xf0c8), highLightTextList[text]));
            highlightMenu->addMenu(subMenu);
            QAction *action = new QAction(tr("Remove"),this);
            subMenu->addAction(action);
            connect(action,&QAction::triggered,this,[=](){
                term->removeHighLightText(text);
            });
            QAction *colorAction = new QAction(tr("Change Color"),this);
            subMenu->addAction(colorAction);
            connect(colorAction,&QAction::triggered,this,[=](){
                QColor color = QColorDialog::getColor(highLightTextList[text],this);
                if(color.isValid()) {
                    lastHighlightColor = color;
                    term->removeHighLightText(text);
                    term->addHighLightText(text, color);
                }
            });
        }
    }
    if(!text.isEmpty()) {
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
    }
    foreach(pluginState_t pluginStruct, pluginList) {
        if(!pluginStruct.state) continue;
        PluginInterface *plugin = pluginStruct.iface;
        QMenu *pluginMenu = plugin->terminalContextMenu(term->selectedText(),term->getWorkingDirectory(),menu);
        if(pluginMenu != nullptr) {
            menu->addSeparator();
            menu->addMenu(pluginMenu);
        } else {
            QList<QAction *> pluginActionList = plugin->terminalContextAction(term->selectedText(),term->getWorkingDirectory(),menu);
            if(pluginActionList.isEmpty()) {
                continue;
            }
            menu->addSeparator();
            foreach(QAction *action, pluginActionList) {
                menu->addAction(action);
            }
        }
    }
}

void CentralWidget::floatingWindow(MainWidgetGroup *g, int index) {
    FloatingTab *dialog = new FloatingTab(this);
    MainWidgetGroup *group = new MainWidgetGroup(MainWidgetGroup::FLOATING,dialog);
    mainWidgetGroupList.append(group);
    int newGroup = mainWidgetGroupList.count()-1;
    moveToAnotherTab(mainWidgetGroupList.indexOf(g),newGroup,index);
    dialog->layout()->addWidget(group);
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
            dialog->forceClose();
        });
        if(menu->isEmpty()) {
            delete menu;
            return;
        }
        QRect screenGeometry = QGuiApplication::screenAt(cursor().pos())->geometry();
        QPoint pos = cursor().pos() + QPoint(5,5);
        if (pos.x() + menu->width() > screenGeometry.right()) {
            pos.setX(screenGeometry.right() - menu->width());
        }
        if (pos.y() + menu->height() > screenGeometry.bottom()) {
            pos.setY(screenGeometry.bottom() - menu->height());
        }
        menu->popup(pos);
    });
    connect(group->commandWidget, &CommandWidget::sendData, this, [=](const QByteArray &data) {
        QWidget *widget = group->sessionTab->currentWidget();
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->proxySendData(data);
    });
    connect(dialog, &FloatingTab::finished, this, [=](int result){
        MainWidgetGroup *group = mainWidgetGroupList.at(newGroup);
        stopSession(group,1,true);
        mainWidgetGroupList.removeAt(newGroup);
        delete dialog;
        Q_UNUSED(result);
    });
    dialog->show();
}

void CentralWidget::refreshTagColor(void) {
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        if(mainWidgetGroup->type() == MainWidgetGroup::EMBEDDED){
            SessionTab *sessionTab = mainWidgetGroup->sessionTab;
            sessionTab->clearAllTagColor();
            for(int i=0;i<sessionTab->count();i++) {
                QWidget *widget = sessionTab->widget(i+1);
                if(!widget) continue;
                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                if(sessionsWindow) {
                    sessionTab->setTagColor(i+1,sessionsWindow->isTagColor(),sessionsWindow->getTagColorValue());
                }
            }
        }
    }
}

void CentralWidget::refreshStatusBar(void) {
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) {
        statusBarWidget->setType(QString());
        statusBarWidget->setCursorPosition(-1,-1);
        statusBarWidget->setTransInfo(false);
        statusBarWidget->setSpeedInfo(false);
        statusBarWidget->setEndOfLine(false);
        statusBarWidget->setLogs(false,false);
        return;
    }
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    if(sessionsWindow == nullptr) {
        statusBarWidget->setType(QString());
        statusBarWidget->setCursorPosition(-1,-1);
        statusBarWidget->setTransInfo(false);
        statusBarWidget->setSpeedInfo(false);
        statusBarWidget->setEndOfLine(false);
        statusBarWidget->setSSHEncryption(false);
        statusBarWidget->setLogs(false,false);
        return;
    }
    statusBarWidget->setCursorPosition(sessionsWindow->getCursorLineCount(),sessionsWindow->getCursorColumnCount());
    statusBarWidget->setLogs(true,sessionsWindow->isLog() || sessionsWindow->isRawLog());
    SessionsWindow::StateInfo stateInfo = sessionsWindow->getStateInfo();
    switch(stateInfo.type) {
        case SessionsWindow::LocalShell:
            statusBarWidget->setType(tr("Local Shell"));
            statusBarWidget->setTransInfo(false);
            statusBarWidget->setSpeedInfo(false);
            statusBarWidget->setEndOfLine(false);
            statusBarWidget->setSSHEncryption(false);
            break;
        case SessionsWindow::Telnet:
            statusBarWidget->setType("Telnet");
            statusBarWidget->setTransInfo(true,stateInfo.telnet.tx_total,stateInfo.telnet.rx_total);
            statusBarWidget->setSpeedInfo(true,stateInfo.telnet.tx_speed,stateInfo.telnet.rx_speed);
            statusBarWidget->setEndOfLine(true,stateInfo.endOfLine);
            statusBarWidget->setSSHEncryption(false);
            break;
        case SessionsWindow::RawSocket:
            statusBarWidget->setType(tr("Raw Socket"));
            statusBarWidget->setTransInfo(true,stateInfo.rawSocket.tx_total,stateInfo.rawSocket.rx_total);
            statusBarWidget->setSpeedInfo(true,stateInfo.rawSocket.tx_speed,stateInfo.rawSocket.rx_speed);
            statusBarWidget->setEndOfLine(true,stateInfo.endOfLine);
            statusBarWidget->setSSHEncryption(false);
            break;
        case SessionsWindow::NamePipe:
            statusBarWidget->setType(tr("Name Pipe"));
            statusBarWidget->setTransInfo(false);
            statusBarWidget->setSpeedInfo(false);
            statusBarWidget->setEndOfLine(true,stateInfo.endOfLine);
            statusBarWidget->setSSHEncryption(false);
            break;
        case SessionsWindow::SSH2:
            statusBarWidget->setType("SSH2");
            statusBarWidget->setTransInfo(true,stateInfo.ssh2.tx_total,stateInfo.ssh2.rx_total);
            statusBarWidget->setSpeedInfo(true,stateInfo.ssh2.tx_speed,stateInfo.ssh2.rx_speed);
            statusBarWidget->setEndOfLine(false);
            statusBarWidget->setSSHEncryption(true,stateInfo.ssh2.encryption_method);
            break;
        case SessionsWindow::Serial:
            statusBarWidget->setType(tr("Serial"));
            statusBarWidget->setTransInfo(true,stateInfo.serial.tx_total,stateInfo.serial.rx_total);
            statusBarWidget->setSpeedInfo(true,stateInfo.serial.tx_speed,stateInfo.serial.rx_speed);
            statusBarWidget->setEndOfLine(true,stateInfo.endOfLine);
            statusBarWidget->setSSHEncryption(false);
            break;
        case SessionsWindow::VNC:
            statusBarWidget->setType("VNC");
            statusBarWidget->setTransInfo(false);
            statusBarWidget->setSpeedInfo(false);
            statusBarWidget->setEndOfLine(false);
            statusBarWidget->setSSHEncryption(false);
            break;
        default:
            statusBarWidget->setType(tr("Unknown"));
            statusBarWidget->setTransInfo(false);
            statusBarWidget->setSpeedInfo(false);
            statusBarWidget->setEndOfLine(false);
            statusBarWidget->setSSHEncryption(false);
            break;
    }
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
    themeColor = settings.value("Global/Startup/themeColor",QColor(30,30,30)).value<QColor>();
    themeColorEnable = settings.value("Global/Startup/themeColorEnable",false).toBool();
    if(themeColorEnable) {
        QTimer::singleShot(0, this, [&](){
            qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
            QGoodWindow::setAppCustomTheme(isDarkTheme,this->themeColor); // Must be >96
        });
    }
}

MainWidgetGroup* CentralWidget::findCurrentFocusGroup(bool forceFind, bool includefloating) {
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        if((!includefloating) && mainWidgetGroup->type() != MainWidgetGroup::EMBEDDED) {
            continue;
        }
        if(mainWidgetGroup->size().width() == 0) {
            continue;
        }
        if(mainWidgetGroup->sessionTab->currentWidget()->hasFocus()) {
            return mainWidgetGroup;
        }
    }
    foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
        if((!includefloating) && mainWidgetGroup->type() != MainWidgetGroup::EMBEDDED) {
            continue;
        }
        if(mainWidgetGroup->size().width() == 0) {
            continue;
        }
        if(mainWidgetGroup->sessionTab->count() != 0) {
            return mainWidgetGroup;
        }
    }
    if(forceFind) {
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            if((!includefloating) && mainWidgetGroup->type() != MainWidgetGroup::EMBEDDED) {
                continue;
            }
            if(mainWidgetGroup->size().width() != 0) {
                return mainWidgetGroup;
            }
        }
        return mainWidgetGroupList[0];
    } else {
        return nullptr;
    }
}

QWidget *CentralWidget::findCurrentFocusWidget(bool includefloating) {
    SessionTab *sessionTab = findCurrentFocusGroup(true,includefloating)->sessionTab;
    if(sessionTab->count() == 0) return nullptr;
    return sessionTab->currentWidget();
}

void CentralWidget::menuAndToolBarRetranslateUi(void) {
    sessionManagerPushButton->setText(tr("Session Manager"));
    pluginViewerPushButton->setText(tr("Plugin"));

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
#if defined(Q_OS_WIN)
    connectWslAction->setText(tr("Connect WSL"));
    connectWslAction->setIcon(QFontIcon::icon(QChar(0xf17c)));
    connectWslAction->setStatusTip(tr("Connect to a WSL shell"));
#endif
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
    connectAddressEdit->setToolTip(tr("Enter host <Alt+R> to connect"));
    disconnectAllAction->setText(tr("Disconnect All"));
    disconnectAllAction->setStatusTip(tr("Disconnect all sessions"));
    cloneSessionAction->setText(tr("Clone Session"));
    cloneSessionAction->setStatusTip(tr("Clone current session <Ctrl+Shift+T>"));
    cloneSessionAction->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_T));
    logSessionAction->setText(tr("Log Session"));
    logSessionAction->setStatusTip(tr("Create a log file for current session"));
    rawLogSessionAction->setText(tr("Raw Log Session"));
    logSessionAction->setStatusTip(tr("Create a raw log file for current session"));
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
    screenRecordingAction->setText(tr("Screen Recording"));
    screenRecordingAction->setStatusTip(tr("Screen recording current screen"));
    screenRecordingAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_G));
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
    layoutMenu->setTitle(tr("Layout"));
    singleLayoutAction->setText(tr("Single Layout"));
    singleLayoutAction->setStatusTip(tr("Single layout"));
    singleLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_1));
    twoColumnsLayoutAction->setText(tr("Two Columns Layout"));
    twoColumnsLayoutAction->setStatusTip(tr("Two columns layout"));
    twoColumnsLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_2));
    threeColumnsLayoutAction->setText(tr("Three Columns Layout"));
    threeColumnsLayoutAction->setStatusTip(tr("Three columns layout"));
    threeColumnsLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_3));
    twoRowsLayoutAction->setText(tr("Two Rows Layout"));
    twoRowsLayoutAction->setStatusTip(tr("Two rows layout"));
    twoRowsLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_4));
    threeRowsLayoutAction->setText(tr("Three Rows Layout"));
    threeRowsLayoutAction->setStatusTip(tr("Three rows layout"));
    threeRowsLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_5));
    gridLayoutAction->setText(tr("Grid Layout"));
    gridLayoutAction->setStatusTip(tr("Grid layout"));
    gridLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_6));
    twoRowsRightLayoutAction->setText(tr("Two Rows Right Layout"));
    twoRowsRightLayoutAction->setStatusTip(tr("Two rows right layout"));
    twoRowsRightLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_7));
    twoColumnsBottomLayoutAction->setText(tr("Two Columns Bottom Layout"));
    twoColumnsBottomLayoutAction->setStatusTip(tr("Two columns bottom layout"));
    twoColumnsBottomLayoutAction->setShortcut(QKeySequence(Qt::SHIFT|Qt::ALT|Qt::Key_8));
    flipLayoutAction->setText(tr("Flip Layout"));
    flipLayoutAction->setStatusTip(tr("Flip layout"));
    flipLayoutAction->setShortcut(QKeySequence(Qt::ALT|Qt::Key_F));
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
    hexViewAction->setText(tr("Hex View"));
    hexViewAction->setStatusTip(tr("Show/Hide Hex View for current session"));
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
    sendKermitAction->setText(tr("Send Kermit..."));
    sendKermitAction->setStatusTip(tr("Send a file using Kermit"));
    receiveKermitAction->setText(tr("Receive Kermit..."));
    receiveKermitAction->setStatusTip(tr("Receive a file using Kermit"));
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
    cleanAllRecentScriptAction->setText(tr("Clean all recent script"));
    cleanAllRecentScriptAction->setStatusTip(tr("Clean all recent script"));

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
    internalCommandAction->setText(tr("Internal Command"));
    internalCommandAction->setStatusTip(tr("Display Internal Command window"));

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
    portugueseAction->setText(tr("Portuguese (Brazil)"));
    portugueseAction->setStatusTip(tr("Switch to Portuguese (Brazil)"));
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
    germanAction->setText(tr("German"));
    germanAction->setStatusTip(tr("Switch to German"));
    czechAction->setText(tr("Czech"));
    czechAction->setStatusTip(tr("Switch to Czech"));
    arabicAction->setText(tr("Arabic"));
    arabicAction->setStatusTip(tr("Switch to Arabic"));

    lightThemeAction->setText(tr("Light"));
    lightThemeAction->setStatusTip(tr("Switch to light theme"));
    darkThemeAction->setText(tr("Dark"));
    darkThemeAction->setStatusTip(tr("Switch to dark theme"));
    themeColorAction->setText(tr("Theme Color"));
    themeColorAction->setStatusTip(tr("Set theme color, cancel to use default"));

    helpAction->setText(tr("Help"));
    helpAction->setIcon(QFontIcon::icon(QChar(0xf128)));
    helpAction->setStatusTip(tr("Display help"));
    keyboradShortcutsReferenceAction->setText(tr("Keyborad Shortcuts Reference"));
    keyboradShortcutsReferenceAction->setIcon(QFontIcon::icon(QChar(0xf128)));
    keyboradShortcutsReferenceAction->setStatusTip(tr("Display keyborad shortcuts reference"));
    checkUpdateAction->setText(tr("Check Update"));
    checkUpdateAction->setIcon(QFontIcon::icon(QChar(0xf09b)));
    checkUpdateAction->setStatusTip(tr("Check for updates"));
    privacyStatementAction->setText(tr("Privacy Statement"));
    privacyStatementAction->setIcon(QFontIcon::icon(QChar(0xf023)));
    privacyStatementAction->setStatusTip(tr("Display privacy statement"));
    aboutAction->setText(tr("About"));
    aboutAction->setIcon(QIcon(":/icons/icons/about.png"));
    aboutAction->setStatusTip(tr("Display about dialog"));
    aboutQtAction->setText(tr("About Qt"));
    aboutQtAction->setIcon(QIcon(":/icons/icons/aboutqt.png"));
    aboutQtAction->setStatusTip(tr("Display about Qt dialog"));
#ifdef ENABLE_PYTHON
    aboutPythonAction->setText(tr("About Python"));
    aboutPythonAction->setIcon(QIcon(":/icons/icons/aboutpython.png"));
    aboutPythonAction->setStatusTip(tr("Display about Python dialog"));
#endif

    laboratoryButton->setToolTip(tr("Laboratory"));
    laboratoryButton->setIcon(QFontIcon::icon(QChar(0xf0c3)));

    sshScanningAction->setText(tr("SSH Scanning"));
    sshScanningAction->setStatusTip(tr("Display SSH scanning dialog"));
    pluginInfoAction->setText(tr("Plugin Info"));
    pluginInfoAction->setStatusTip(tr("Display plugin information dialog"));
}

void CentralWidget::menuAndToolBarInit(bool disable_plugin) {
    GlobalSetting settings;
    int size = 0;

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
#if defined(Q_OS_WIN)
    connectWslAction = new QAction(this);
    fileMenu->addAction(connectWslAction);
    ui->toolBar->addAction(connectWslAction);
    sessionManagerWidget->addActionOnToolBar(connectWslAction);
#endif
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
    logSessionAction = new QAction(this);
    logSessionAction->setCheckable(true);
    logSessionAction->setChecked(false);
    fileMenu->addAction(logSessionAction);
    rawLogSessionAction = new QAction(this);
    rawLogSessionAction->setCheckable(true);
    rawLogSessionAction->setChecked(false);
    fileMenu->addAction(rawLogSessionAction);
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
    screenRecordingAction = new QAction(this);
    screenRecordingAction->setCheckable(true);
    editMenu->addAction(screenRecordingAction);
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
    layoutMenu = new QMenu(this);
    layoutActionGroup = new QActionGroup(this);
    singleLayoutAction = new QAction(this);
    singleLayoutAction->setActionGroup(layoutActionGroup);
    singleLayoutAction->setCheckable(true);
    singleLayoutAction->setChecked(true);
    layoutMenu->addAction(singleLayoutAction);
    twoColumnsLayoutAction = new QAction(this);
    twoColumnsLayoutAction->setActionGroup(layoutActionGroup);
    twoColumnsLayoutAction->setCheckable(true);
    layoutMenu->addAction(twoColumnsLayoutAction);
    threeColumnsLayoutAction = new QAction(this);
    threeColumnsLayoutAction->setActionGroup(layoutActionGroup);
    threeColumnsLayoutAction->setCheckable(true);
    layoutMenu->addAction(threeColumnsLayoutAction);
    twoRowsLayoutAction = new QAction(this);
    twoRowsLayoutAction->setActionGroup(layoutActionGroup);
    twoRowsLayoutAction->setCheckable(true);
    layoutMenu->addAction(twoRowsLayoutAction);
    threeRowsLayoutAction = new QAction(this);
    threeRowsLayoutAction->setActionGroup(layoutActionGroup);
    threeRowsLayoutAction->setCheckable(true);
    layoutMenu->addAction(threeRowsLayoutAction);
    gridLayoutAction = new QAction(this);
    gridLayoutAction->setActionGroup(layoutActionGroup);
    gridLayoutAction->setCheckable(true);
    layoutMenu->addAction(gridLayoutAction);
    twoRowsRightLayoutAction = new QAction(this);
    twoRowsRightLayoutAction->setActionGroup(layoutActionGroup);
    twoRowsRightLayoutAction->setCheckable(true);
    layoutMenu->addAction(twoRowsRightLayoutAction);
    twoColumnsBottomLayoutAction = new QAction(this);
    twoColumnsBottomLayoutAction->setActionGroup(layoutActionGroup);
    twoColumnsBottomLayoutAction->setCheckable(true);
    layoutMenu->addAction(twoColumnsBottomLayoutAction);
    layoutMenu->addSeparator();
    flipLayoutAction = new QAction(this);
    layoutMenu->addAction(flipLayoutAction);
    viewMenu->addMenu(layoutMenu);
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
    hexViewAction = new QAction(this);
    hexViewAction->setCheckable(true);
    viewMenu->addAction(hexViewAction);
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
    sendKermitAction = new QAction(this);
    transferMenu->addAction(sendKermitAction);
    receiveKermitAction = new QAction(this);
    transferMenu->addAction(receiveKermitAction);
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
    cancelAction->setEnabled(false);
    scriptMenu->addAction(cancelAction);
    scriptMenu->addSeparator();
    startRecordingScriptAction = new QAction(this);
    startRecordingScriptAction->setEnabled(false);
    scriptMenu->addAction(startRecordingScriptAction);
    stopRecordingScriptAction = new QAction(this);
    stopRecordingScriptAction->setEnabled(false);
    scriptMenu->addAction(stopRecordingScriptAction);
    canlcelRecordingScriptAction = new QAction(this);
    canlcelRecordingScriptAction->setEnabled(false);
    scriptMenu->addAction(canlcelRecordingScriptAction);
    scriptMenu->addSeparator();
    cleanAllRecentScriptAction = new QAction(this);
    scriptMenu->addAction(cleanAllRecentScriptAction);
#ifdef ENABLE_PYTHON
    size = settings.beginReadArray("Global/RecentScript");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString path = settings.value("path").toString();
        QAction *action = new QAction(path,scriptMenu);
        action->setStatusTip(path);
        scriptMenu->addAction(action);
        connect(action,&QAction::triggered,this,[=](){
            if(pyRun->isRunning()){
                return;
            }
            runScriptFullName = path;
            pyRun->runScriptFile(path);
        });
    }
    settings.endArray();
#endif

    addBookmarkAction = new QAction(this);
    bookmarkMenu->addAction(addBookmarkAction);
    removeBookmarkAction = new QAction(this);
    bookmarkMenu->addAction(removeBookmarkAction);
    cleanAllBookmarkAction = new QAction(this);
    bookmarkMenu->addAction(cleanAllBookmarkAction);
    bookmarkMenu->addSeparator();
    size = settings.beginReadArray("Global/Bookmark");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString path = settings.value("path").toString();
        QAction *action = new QAction(path,bookmarkMenu);
        action->setStatusTip(path);
        bookmarkMenu->addAction(action);
        connect(action,&QAction::triggered,this,[=](){
            startLocalShellSession(findCurrentFocusGroup(),-1,QString(),path);
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
    toolsMenu->addSeparator();
    internalCommandAction = new QAction(this);
    toolsMenu->addAction(internalCommandAction);

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
    portugueseAction = new QAction(this);
    portugueseAction->setActionGroup(languageActionGroup);
    portugueseAction->setCheckable(true);
    portugueseAction->setChecked(language == QLocale(QLocale::Portuguese, QLocale::Brazil));
    languageMenu->addAction(portugueseAction);
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
    germanAction = new QAction(this);
    germanAction->setActionGroup(languageActionGroup);
    germanAction->setCheckable(true);
    germanAction->setChecked(language == QLocale::German);
    languageMenu->addAction(germanAction);
    czechAction = new QAction(this);
    czechAction->setActionGroup(languageActionGroup);
    czechAction->setCheckable(true);
    czechAction->setChecked(language == QLocale::Czech);
    languageMenu->addAction(czechAction);
    arabicAction = new QAction(this);
    arabicAction->setActionGroup(languageActionGroup);
    arabicAction->setCheckable(true);
    arabicAction->setChecked(language == QLocale::Arabic);
    languageMenu->addAction(arabicAction);

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
    themeMenu->addSeparator();
    themeColorAction = new QAction(this);
    themeMenu->addAction(themeColorAction);

    helpAction = new QAction(this);
    helpMenu->addAction(helpAction);
    ui->toolBar->addAction(helpAction);
    sessionManagerWidget->addActionOnToolBar(helpAction);
    keyboradShortcutsReferenceAction = new QAction(this);
    helpMenu->addAction(keyboradShortcutsReferenceAction);
    checkUpdateAction = new QAction(this);
    helpMenu->addAction(checkUpdateAction);
    helpMenu->addSeparator();
    privacyStatementAction = new QAction(this);
    helpMenu->addAction(privacyStatementAction);
    helpMenu->addSeparator();
    aboutAction = new QAction(this);
    helpMenu->addAction(aboutAction);
    aboutQtAction = new QAction(this);
    helpMenu->addAction(aboutQtAction);
#ifdef ENABLE_PYTHON
    aboutPythonAction = new QAction(this);
    helpMenu->addAction(aboutPythonAction);
#endif

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
    laboratoryMenu->addSeparator();
    pluginInfoAction = new QAction(this);

    if(!disable_plugin) {
        QTimer::singleShot(200, this, [&](){
            QStringList pluginsDirList;
            QDir defPluginsDir(QCoreApplication::applicationDirPath());
            if(!defPluginsDir.cd("plugins")) {
                defPluginsDir = QDir(QCoreApplication::applicationDirPath()+"/..");
                if(!defPluginsDir.cd("plugins")) {
                    qInfo() << "plugins dir not exist:" << defPluginsDir.absolutePath();
                    goto loadPlugin;
                }
            }
            if(!defPluginsDir.cd("QuardCRT")) {
                qInfo() << "plugins/QuardCRT dir not exist:" << defPluginsDir.absolutePath();
                goto loadPlugin;
            }
            pluginsDirList.append(defPluginsDir.absolutePath());
        loadPlugin:
            GlobalSetting settings;
            QString userPluginsDir = settings.value("Global/Options/UserPluginsPath",QString()).toString();
            if(!userPluginsDir.isEmpty()) {
                QDir userDir(userPluginsDir);
                if(userDir.exists()) {
                    pluginsDirList.append(userDir.absolutePath());
                }
            }
        #if defined(Q_OS_WIN)
            qApp->addLibraryPath(QCoreApplication::applicationDirPath());
        #elif defined(Q_OS_MACOS)
            qApp->addLibraryPath(QCoreApplication::applicationDirPath()+"/../Frameworks");
        #elif defined(Q_OS_LINUX)
            qApp->addLibraryPath(QCoreApplication::applicationDirPath()+"/lib");
            qApp->addLibraryPath(QCoreApplication::applicationDirPath()+"../lib");
        #endif
            foreach (QString pluginsDirStr, pluginsDirList) {
                QDir pluginsDir(pluginsDirStr);
                qApp->addLibraryPath(pluginsDir.absolutePath());
                foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
                    QString absoluteFilePath = pluginsDir.absoluteFilePath(fileName);
                    QPluginLoader loader(absoluteFilePath,this);
                    QJsonObject metaData = loader.metaData();
                    if(!metaData.contains("MetaData")) {
                        pluginInfoWindow->addPluginInfo(fileName,"",tr("Plugin metaData not found!"),0,false,true);
                        continue;
                    }
                    QJsonObject metaDataObject = metaData.value("MetaData").toObject();
                    qInfo() << metaDataObject;
                    if(!metaDataObject.contains("APIVersion")) {
                        pluginInfoWindow->addPluginInfo(fileName,"",tr("Plugin api version not found!"),0,false,true);
                        continue;
                    }
                    int apiVersion = metaDataObject.value("APIVersion").toInt();
                    QList<uint32_t> supportVersion = PluginInfoWindow::supportAPIVersionList();
                    if(!supportVersion.contains((uint32_t)apiVersion)) {
                        pluginInfoWindow->addPluginInfo(fileName,"",tr("Plugin api version not match!"),apiVersion,false,true);         
                        continue;
                    }
                    QObject *plugin = loader.instance();
                    if(plugin) {
                        PluginInterface *iface = qobject_cast<PluginInterface *>(plugin);
                        if(iface) {
                            QMap<QString, QString> params;
                            params.insert("version",VERSION);
                            params.insert("git_tag",GIT_TAG);
                            params.insert("build_date",DATE_TAG);
                            qDebug() << "we will load plugin:" << iface->name();
                            if(iface->init(params, this) == 0) {
                                GlobalSetting settings;
                                bool state = settings.value("Global/Plugin/"+iface->name()+"/State",true).toBool();
                                pluginState_t pluginStruct;
                                pluginStruct.iface = iface;
                                connect(iface,SIGNAL(requestTelnetConnect(QString, int, int)),this,SLOT(onPluginRequestTelnetConnect(QString, int, int)));
                                connect(iface,SIGNAL(requestSerialConnect(QString, uint32_t, int, int, int, bool, bool)),this,SLOT(onPluginRequestSerialConnect(QString, uint32_t, int, int, int, bool, bool)));
                                connect(iface,SIGNAL(requestLocalShellConnect(QString, QString)),this,SLOT(onPluginRequestLocalShellConnect(QString, QString)));
                                connect(iface,SIGNAL(requestRawSocketConnect(QString, int)),this,SLOT(onPluginRequestRawSocketConnect(QString, int)));
                                connect(iface,SIGNAL(requestNamePipeConnect(QString)),this,SLOT(onPluginRequestNamePipeConnect(QString)));
                                connect(iface,SIGNAL(requestSSH2Connect(QString, QString, QString, int)),this,SLOT(onPluginRequestSSH2Connect(QString, QString, QString, int)));
                                connect(iface,SIGNAL(requestVNCConnect(QString, QString, int)),this,SLOT(onPluginRequestVNCConnect(QString, QString, int)));
                                connect(iface,SIGNAL(sendCommand(QString)),this,SLOT(onPluginSendCommand(QString)));
                                connect(iface,SIGNAL(writeSettings(QString, QString, QVariant)),this,SLOT(onPluginWriteSettings(QString, QString, QVariant)));
                                connect(iface,SIGNAL(readSettings(QString, QString, QVariant &)),this,SLOT(onPluginReadSettings(QString, QString, QVariant &)));
                                QString website;
                                QMap<QString,void *> map = iface->metaObject();
                                bool findMenu = false;
                                foreach (QString key, map.keys()) {
                                    if(key == "MainMenuAction" || key == "QAction") {
                                        QAction *action = (QAction *)map.value(key);
                                        laboratoryMenu->addAction(action);
                                        action->setVisible(state);
                                        findMenu = true;
                                    } else if(key == "MainMenuMenu" || key == "QMenu") {
                                        QMenu *menu = (QMenu *)map.value(key);
                                        laboratoryMenu->addMenu(menu);
                                        menu->menuAction()->setVisible(state);
                                        findMenu = true;
                                    } else if(key == "MainWidget" || key == "QWidget") {
                                        QWidget *widget = (QWidget *)map.value(key);
                                        if(pluginViewerWidget->addPlugin(widget,iface->name())) {
                                            pluginViewerWidget->setPluginVisible(iface->name(),state);
                                        }
                                    } else if(key == "website") {
                                        website = *(QString *)map.value(key);
                                    }
                                }
                                if(findMenu) {
                                    iface->setLanguage(language,qApp);
                                    iface->retranslateUi();
                                    pluginInfoWindow->addPluginInfo(iface,absoluteFilePath,apiVersion,state,false,website);
                                } else {
                                    state = false;
                                    pluginViewerWidget->setPluginVisible(iface->name(),state);
                                    pluginInfoWindow->addPluginInfo(fileName,"",tr("Plugin menu not found!"),apiVersion,false,true);
                                }
                                pluginStruct.state = state;
                                pluginList.append(pluginStruct);
                            } else {
                                pluginInfoWindow->addPluginInfo(fileName,"",tr("Plugin init failed!"),apiVersion,false,true);
                            }
                        } else {
                            pluginInfoWindow->addPluginInfo(fileName,"",loader.errorString(),apiVersion,false,true);
                        }
                    } else {
                        pluginInfoWindow->addPluginInfo(fileName,"",loader.errorString(),apiVersion,false,true);
                    }
                }
                laboratoryMenu->addSeparator();
            }
            laboratoryMenu->addAction(pluginInfoAction);
        });
    } else {
        laboratoryMenu->addAction(pluginInfoAction);
        pluginInfoAction->setDisabled(true);
    }

    menuAndToolBarRetranslateUi();

    setSessionClassActionEnable(false);
}

void CentralWidget::onPluginRequestTelnetConnect(QString host, int port, int type) {
    startTelnetSession(findCurrentFocusGroup(),-1,host,port,(QTelnet::SocketType)type);
}

void CentralWidget::onPluginRequestSerialConnect(QString portName, uint32_t baudRate, int dataBits, int parity, int stopBits, bool flowControl, bool xEnable) {
    startSerialSession(findCurrentFocusGroup(),-1,portName,baudRate,dataBits,parity,stopBits,flowControl,xEnable);
}

void CentralWidget::onPluginRequestLocalShellConnect(QString command, QString workingDirectory) {
    startLocalShellSession(findCurrentFocusGroup(),-1,command,workingDirectory);
}

void CentralWidget::onPluginRequestRawSocketConnect(QString host, int port) {
    startRawSocketSession(findCurrentFocusGroup(),-1,host,port);
}

void CentralWidget::onPluginRequestNamePipeConnect(QString namePipe) {
    startNamePipeSession(findCurrentFocusGroup(),-1,namePipe);
}

void CentralWidget::onPluginRequestSSH2Connect(QString host, QString user, QString password, int port) {
    startSSH2Session(findCurrentFocusGroup(),-1,host,port,user,password);
}

void CentralWidget::onPluginRequestVNCConnect(QString host, QString password, int port) {
    startVNCSession(findCurrentFocusGroup(),-1,host,port,password);
}

void CentralWidget::onPluginSendCommand(QString cmd){
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
    if(sessionsWindow->isLocked()) return;
    cmd += "\n";
    sessionsWindow->proxySendData(cmd.toLatin1());
}

void CentralWidget::onPluginWriteSettings(QString group, QString key, QVariant value){
    PluginInterface *iface = qobject_cast<PluginInterface *>(sender());
    GlobalSetting settings;
    settings.setValue("Plugin/"+iface->name()+"/"+group+"/"+key,value);
}

void CentralWidget::onPluginReadSettings(QString group, QString key, QVariant &value){
    PluginInterface *iface = qobject_cast<PluginInterface *>(sender());
    GlobalSetting settings;
    value = settings.value("Plugin/"+iface->name()+"/"+group+"/"+key);
}

void CentralWidget::setSessionClassActionEnable(bool enable)
{
    reconnectAction->setEnabled(enable);
    reconnectAllAction->setEnabled(enable);
    disconnectAction->setEnabled(enable);
    disconnectAllAction->setEnabled(enable);
    cloneSessionAction->setEnabled(enable);
    logSessionAction->setEnabled(enable);
    rawLogSessionAction->setEnabled(enable);

    copyAction->setEnabled(enable);
    pasteAction->setEnabled(enable);
    copyAndPasteAction->setEnabled(enable);
    selectAllAction->setEnabled(enable);
    findAction->setEnabled(enable);
    printScreenAction->setEnabled(enable);
    screenShotAction->setEnabled(enable);
    screenRecordingAction->setEnabled(enable);
    sessionExportAction->setEnabled(enable);
    clearScrollbackAction->setEnabled(enable);
    clearScreenAction->setEnabled(enable);
    clearScreenAndScrollbackAction->setEnabled(enable);
    resetAction->setEnabled(enable);

    zoomInAction->setEnabled(enable);
    zoomOutAction->setEnabled(enable);
    zoomResetAction->setEnabled(enable);

    sessionOptionsAction->setEnabled(enable);

    sendASCIIAction->setEnabled(enable);
    receiveASCIIAction->setEnabled(enable);
    sendBinaryAction->setEnabled(enable);
    sendKermitAction->setEnabled(enable);
    receiveKermitAction->setEnabled(enable);
    sendXmodemAction->setEnabled(enable);
    receiveXmodemAction->setEnabled(enable);
    sendYmodemAction->setEnabled(enable);
    receiveYmodemAction->setEnabled(enable);
    zmodemUploadListAction->setEnabled(enable);
    startZmodemUploadAction->setEnabled(enable&&(!zmodemUploadList.isEmpty()));
}

void CentralWidget::menuAndToolBarConnectSignals(void) {
    connect(newWindowAction,&QAction::triggered,this,[=](){
        QProcess::startDetached(QApplication::applicationFilePath(),QApplication::arguments().mid(1));
    });
    connect(connectAction,&QAction::triggered,this,[=](){
        if(stackedWidget->currentWidget() != sessionManagerWidget) {
            stackedWidget->setCurrentWidget(sessionManagerWidget);
        }
        stackedWidget->show();
    });
    connect(sessionManagerAction,&QAction::triggered,this,[=](){
        if(stackedWidget->currentWidget() != sessionManagerWidget) {
            stackedWidget->setCurrentWidget(sessionManagerWidget);
        }
        stackedWidget->show();
    });
    connect(quickConnectAction,&QAction::triggered,this,[=](){
        quickConnectMainWidgetGroup = findCurrentFocusGroup();
        quickConnectWindow->reset();
        quickConnectWindow->show();
    });
    connect(quickConnectWindow,&QuickConnectWindow::sendQuickConnectData,this,
            [=](QuickConnectWindow::QuickConnectData data){
        startSession(quickConnectMainWidgetGroup,-1,data);
    });
    connect(lockSessionWindow,&LockSessionWindow::sendLockSessionData,this,
        [=](QString password, bool lockAllSession, bool lockAllSessionGroup){
        if(lockAllSession) {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
            }
        } else if(lockAllSessionGroup) {
            if(!sessionList.contains(willLockUnLockSessions)) return;
            SessionTab *sessionTab = nullptr;
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                if(mainWidgetGroup->type() != MainWidgetGroup::EMBEDDED) {
                    continue;
                }
                if(mainWidgetGroup->size().width() == 0) {
                    continue;
                }
                if(mainWidgetGroup->sessionTab->indexOf(willLockUnLockSessions->getMainWidget()) > 0) {
                    sessionTab = mainWidgetGroup->sessionTab;
                    break;
                }
            }
            if(sessionTab) {
                if(sessionTab->count() == 0) return;
                for(int i=0;i<sessionTab->count();i++) {
                    QWidget *widget = sessionTab->widget(i+1);
                    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                    if(!sessionsWindow->isLocked()) sessionsWindow->lockSession(password);
                }
            }
        } else {
            if(!sessionList.contains(willLockUnLockSessions)) return;
            if(!willLockUnLockSessions->isLocked()) willLockUnLockSessions->lockSession(password);
        }
    });
    connect(lockSessionWindow,&LockSessionWindow::sendUnLockSessionData,this,
        [=](QString password, bool unlockAllSession, bool unlockAllSessionGroup){
        if(unlockAllSession) {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                if(sessionsWindow->isLocked()) sessionsWindow->unlockSession(password);
            }
        } else if(unlockAllSessionGroup) {
            if(!sessionList.contains(willLockUnLockSessions)) return;
            SessionTab *sessionTab = nullptr;
            foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
                if(mainWidgetGroup->type() != MainWidgetGroup::EMBEDDED) {
                    continue;
                }
                if(mainWidgetGroup->size().width() == 0) {
                    continue;
                }
                if(mainWidgetGroup->sessionTab->indexOf(willLockUnLockSessions->getMainWidget()) > 0) {
                    sessionTab = mainWidgetGroup->sessionTab;
                    break;
                }
            }
            if(sessionTab) {
                if(sessionTab->count() == 0) return;
                for(int i=0;i<sessionTab->count();i++) {
                    QWidget *widget = sessionTab->widget(i+1);
                    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                    if(sessionsWindow->isLocked()) sessionsWindow->unlockSession(password);
                }
            }
        } else {
            if(!sessionList.contains(willLockUnLockSessions)) return;
            if(willLockUnLockSessions->isLocked()) willLockUnLockSessions->unlockSession(password);
        }
    });

    connect(connectInTabAction,&QAction::triggered,this,[=](){
        if(stackedWidget->currentWidget() != sessionManagerWidget) {
            stackedWidget->setCurrentWidget(sessionManagerWidget);
        }
        stackedWidget->show();
    });
    connect(connectLocalShellAction,&QAction::triggered,this,[=](){
        startLocalShellSession(findCurrentFocusGroup(),-1,QString(),globalOptionsWindow->getNewTabWorkPath());
    });
#if defined(Q_OS_WIN)
    connect(connectWslAction,&QAction::triggered,this,[=](){
        startWslSession(findCurrentFocusGroup(),-1,QString(),globalOptionsWindow->getNewTabWorkPath());
    });
#endif
    connect(reconnectAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isLocked()) return;
        reconnectSession(sessionsWindow);
    });
    connect(reconnectAllAction,&QAction::triggered,this,[=](){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(sessionsWindow->isLocked()) continue;
            reconnectSession(sessionsWindow);
        }
    });
    connect(disconnectAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isLocked()) return;
        sessionsWindow->disconnect();
    });
    connect(disconnectAllAction,&QAction::triggered,this,[=](){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(sessionsWindow->isLocked()) continue;
            sessionsWindow->disconnect();
        }
    });
    connect(cloneSessionAction,&QAction::triggered,this,[=](){
        cloneCurrentSession(findCurrentFocusGroup());
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
    connect(globalOptionsWindow,&GlobalOptionsWindow::colorSchemeChanged,this,[=](QString colorScheme){
        if(colorScheme == "Custom") {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                sessionsWindow->setColorScheme(GlobalOptionsWindow::defaultColorScheme);
                QList<QColor> colorTable = globalOptionsWindow->getColorTable();
                int i = 0;
                foreach(const QColor &color, colorTable) {
                    sessionsWindow->setANSIColor(i,color);
                    i++;
                }
            }
        } else {
            foreach(SessionsWindow *sessionsWindow, sessionList) {
                sessionsWindow->setColorScheme(colorScheme);
            }
        }
    });
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    connect(globalOptionsWindow,&GlobalOptionsWindow::enableCtrlCChanged,this,[=](bool enable){
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->setEnableHandleCtrlC(enable);
        }
    });
#endif
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
            sessionManagerWidget->setCurrentSession(newName);
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
        QWidget *widget = findCurrentFocusWidget(true);
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->copyClipboard();
    });
    connect(pasteAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget(true);
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
        QWidget *widget = findCurrentFocusWidget(true);
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->selectAll();
    });
    connect(findAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget(true);
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
        printer.setPrinterName(printerName.isEmpty()?"QuartCRT":printerName);
        printer.setPageSize(QPageSize::A4);
        printer.setOutputFileName(willsaveName);
    #endif

        QPrintDialog dlg(&printer, this);
        if (dlg.exec() == QDialog::Accepted) {
            printerName = printer.printerName();
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
    connect(screenRecordingAction,&QAction::triggered,this,[=](){
        if(!screenRecordingAction->isChecked()) {
            m_mediaRecorder->stop();
            m_windowCapture->stop();
            ui->statusBar->showMessage(tr("ScreenRecording stop."),3000);
        } else {
            QWidget *widget = findCurrentFocusWidget();
            if(widget == nullptr) return;

            QList<QCapturableWindow> windowList = QWindowCapture::capturableWindows();
            bool find = false;
            foreach(const QCapturableWindow &w, windowList) {
                auto handle = QCapturableWindowPrivate::handle(w);
                if(handle && handle->id == this->mainWindow->winId()) {
                    m_windowCapture->setWindow(w);
                    find = true;
                    break;
                }
            }
            if(!find) {
                screenRecordingAction->setChecked(false);
                return;
            }

            GlobalSetting settings;
            QString screenRecordingPath = settings.value("Global/Options/ScreenRecordingPath",QDir::homePath()).toString();
            QString willsaveName = screenRecordingPath + "/quartCRT-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".mp4";
            QString fileName = FileDialog::getSaveFileName(this,tr("Save Screenshot"),willsaveName,tr("Video Files (*.mp4)"));
            if(fileName.isEmpty()) {
                screenRecordingAction->setChecked(false);
                return;
            }
            if(!fileName.endsWith(".mp4")) fileName.append(".mp4");

            m_mediaRecorder->setOutputLocation(QUrl::fromLocalFile(fileName));
            m_windowCapture->start();
            m_mediaRecorder->record();

            settings.setValue("Global/Options/ScreenRecordingPath",QFileInfo(fileName).absolutePath());
            ui->statusBar->showMessage(tr("ScreenRecording saved to %1").arg(fileName),3000);
        }
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
        QWidget *widget = findCurrentFocusWidget(true);
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->clearScrollback();
    });
    connect(clearScreenAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget(true);
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        sessionsWindow->clearScreen();
    });
    connect(clearScreenAndScrollbackAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget(true);
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
        sessionsWindow->set_fix_quardCRT_issue33(globalOptionsWindow->isCurrentFontBuiltIn());
    });
    connect(layoutActionGroup,&QActionGroup::triggered,this,[=](QAction *action){
        int oldLayout = currentLayout;
        if(action == singleLayoutAction) {
            currentLayout = 0;
            splitterV1->setVisible(true);
            splitterV2->setVisible(false);
            splitterV11->setVisible(false);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(false);
            mainWidgetGroupList.at(2)->setVisible(false);
            mainWidgetGroupList.at(3)->setVisible(false);
            mainWidgetGroupList.at(4)->setVisible(false);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = false;
        } else if(action == twoColumnsLayoutAction) {
            currentLayout = 1;
            splitterV1->setVisible(true);
            splitterV2->setVisible(true);
            splitterV11->setVisible(false);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(true);
            mainWidgetGroupList.at(2)->setVisible(false);
            mainWidgetGroupList.at(3)->setVisible(false);
            mainWidgetGroupList.at(4)->setVisible(false);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = false;
        } else if(action == threeColumnsLayoutAction) {
            currentLayout = 2;
            splitterV1->setVisible(true);
            splitterV2->setVisible(true);
            splitterV11->setVisible(false);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(true);
            mainWidgetGroupList.at(2)->setVisible(false);
            mainWidgetGroupList.at(3)->setVisible(false);
            mainWidgetGroupList.at(4)->setVisible(false);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(true);
            enabledSyncSplitterMoved = false;
        } else if(action == twoRowsLayoutAction) {
            currentLayout = 3;
            splitterV1->setVisible(true);
            splitterV2->setVisible(false);
            splitterV11->setVisible(false);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(false);
            mainWidgetGroupList.at(2)->setVisible(true);
            mainWidgetGroupList.at(3)->setVisible(false);
            mainWidgetGroupList.at(4)->setVisible(false);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = false;
        } else if(action == threeRowsLayoutAction) {
            currentLayout = 4;
            splitterV1->setVisible(true);
            splitterV2->setVisible(false);
            splitterV11->setVisible(true);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(false);
            mainWidgetGroupList.at(2)->setVisible(true);
            mainWidgetGroupList.at(3)->setVisible(false);
            mainWidgetGroupList.at(4)->setVisible(true);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = false;
        } else if(action == gridLayoutAction) {
            currentLayout = 5;
            splitterV1->setVisible(true);
            splitterV2->setVisible(true);
            splitterV11->setVisible(false);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(true);
            mainWidgetGroupList.at(2)->setVisible(true);
            mainWidgetGroupList.at(3)->setVisible(true);
            mainWidgetGroupList.at(4)->setVisible(false);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = true;
        } else if(action == twoRowsRightLayoutAction) {
            currentLayout = 6;
            splitterV1->setVisible(true);
            splitterV2->setVisible(true);
            splitterV11->setVisible(false);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(true);
            mainWidgetGroupList.at(2)->setVisible(false);
            mainWidgetGroupList.at(3)->setVisible(true);
            mainWidgetGroupList.at(4)->setVisible(false);
            mainWidgetGroupList.at(5)->setVisible(false);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = false;
        } else if(action == twoColumnsBottomLayoutAction)  {
            currentLayout = 7;
            splitterV1->setVisible(true);
            splitterV2->setVisible(false);
            splitterV11->setVisible(true);
            mainWidgetGroupList.at(0)->setVisible(true);
            mainWidgetGroupList.at(1)->setVisible(false);
            mainWidgetGroupList.at(2)->setVisible(false);
            mainWidgetGroupList.at(3)->setVisible(false);
            mainWidgetGroupList.at(4)->setVisible(true);
            mainWidgetGroupList.at(5)->setVisible(true);
            mainWidgetGroupList.at(6)->setVisible(false);
            enabledSyncSplitterMoved = false;
        }
        foreach(MainWidgetGroup *group, mainWidgetGroupList) {
            group->setActive(false);
        }
        movetabWhenLayoutChange(oldLayout,currentLayout);
    });
    connect(flipLayoutAction,&QAction::triggered,this,[=](){
        switch(currentLayout) {
            case 0: //singleLayoutAction
            case 5: //gridLayoutAction
                break;
            case 1: //twoColumnsLayoutAction
                twoRowsLayoutAction->trigger();
                break;
            case 2: //threeColumnsLayoutAction
                threeRowsLayoutAction->trigger();
                break;
            case 3: //twoRowsLayoutAction
                twoColumnsLayoutAction->trigger();
                break;
            case 4: //threeRowsLayoutAction
                threeColumnsLayoutAction->trigger();
                break;
            case 6: //twoRowsRightLayoutAction
                twoColumnsBottomLayoutAction->trigger();
                break;
            case 7: //twoColumnsBottomLayoutAction
                twoRowsRightLayoutAction->trigger();
                break;
        }
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
    connect(sendASCIIAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        QString file = FileDialog::getOpenFileName(this, tr("Select Files to Send as ASCII"), globalOptionsWindow->getModemUploadPath(), tr("Text Files (*.txt);;All Files (*)"));
        if(file.isEmpty()) return;
        QFile f(file);
        if(f.open(QIODevice::ReadOnly|QIODevice::Text)) {
            QByteArray data = f.readAll();
            f.close();
            sessionsWindow->reverseProxySendData(data);
        }
    });
    connect(receiveASCIIAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr)  return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        receiveASCIIAction->setChecked(!receiveASCIIAction->isChecked());
        if(sessionsWindow->isReceiveASCIIFile()) {
            sessionsWindow->stopReceiveASCIIFile();
            receiveASCIIAction->setChecked(false);
        } else {
            QString file = FileDialog::getSaveFileName(this, tr("Save Received Data as ASCII"), globalOptionsWindow->getModemDownloadPath(), tr("Text Files (*.txt);;All Files (*)"));
            if(file.isEmpty()) return;
            if(file.endsWith(".txt") == false) file.append(".txt");
            sessionsWindow->startReceiveASCIIFile(file);
            receiveASCIIAction->setChecked(true);
        }
    });
    connect(sendBinaryAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        QString file = FileDialog::getOpenFileName(this, tr("Select Files to Send as BINARY"), QDir::homePath(), tr("All Files (*)"));
        if(file.isEmpty()) return;
        QFile f(file);
        if(f.open(QIODevice::ReadOnly)) {
            QByteArray data = f.readAll();
            f.close();
            sessionsWindow->reverseProxySendData(data);
        }
    });
    connect(sendKermitAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        QStringList files = FileDialog::getItemsPathsWithPickBox(this, tr("Select Files to Send using Kermit"), globalOptionsWindow->getModemUploadPath(), tr("All Files (*)"));
        if(files.isEmpty()) return;
        sessionsWindow->sendFileUseKermit(files);
    });
    connect(receiveKermitAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        sessionsWindow->recvFileUseKermit(globalOptionsWindow->getModemDownloadPath());
    });
    connect(sendXmodemAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        QString file = FileDialog::getOpenFileName(this, tr("Select Files to Send using Xmodem"), globalOptionsWindow->getModemUploadPath(), tr("All Files (*)"));
        if(file.isEmpty()) return;
        sessionsWindow->sendFileUseXModem(file,globalOptionsWindow->getXYModem1K());
    });
    connect(receiveXmodemAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr)  return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        QString file = FileDialog::getSaveFileName(this, tr("Save Received Files using Xmodem"), globalOptionsWindow->getModemDownloadPath(), tr("All Files (*)"));
        if(file.isEmpty()) return;
        sessionsWindow->recvFileUseXModem(file);
    });
    connect(sendYmodemAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        QStringList files = FileDialog::getItemsPathsWithPickBox(this, tr("Select Files to Send using Ymodem"), globalOptionsWindow->getModemUploadPath(), tr("All Files (*)"));
        if(files.isEmpty()) return;
        sessionsWindow->sendFileUseYModem(files,globalOptionsWindow->getXYModem1K());
    });
    connect(receiveYmodemAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        sessionsWindow->recvFileUseYModem(globalOptionsWindow->getModemDownloadPath());
    });
    connect(zmodemUploadListAction,&QAction::triggered,this,[=](){
        bool accepted = false;
        QStringList files = FileDialog::getItemsPathsWithPickBox(this, tr("Select Files to Send using Zmodem"), globalOptionsWindow->getModemUploadPath(), tr("All Files (*)"), zmodemUploadList, &accepted);
        if(accepted)  
            zmodemUploadList = files;
        startZmodemUploadAction->setEnabled(!zmodemUploadList.isEmpty());
    });
    connect(startZmodemUploadAction,&QAction::triggered,this,[=](){
        if(zmodemUploadList.isEmpty()) return;
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) return;
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isInBroadCastList()) {
            QMessageBox::warning(this,tr("Warning"),tr("Broadcast session can't transfer file!"));
            return;
        }
        sessionsWindow->sendFileUseZModem(zmodemUploadList);
        zmodemUploadList.clear();
        startZmodemUploadAction->setEnabled(!zmodemUploadList.isEmpty());
    });
    connect(startTFTPServerAction,&QAction::triggered,this,[=](bool checked){
        startTFTPServerAction->setChecked(tftpServer->isRunning());
        if(checked) {
            startTftpSeverWindow->show();
        } else {
            if(tftpServer->isRunning()) {
                tftpServer->stopServer();
            }
        }
    });
#ifdef ENABLE_PYTHON
    connect(runAction,&QAction::triggered,this,[=](){
        GlobalSetting settings;
        QString scriptDir = settings.value("Global/Options/ScriptPath",QDir::homePath()).toString();
        QString scriptPath = FileDialog::getOpenFileName(this, tr("Select a script file"), scriptDir, tr("Python Files (*.py);;All Files (*)"));
        if(scriptPath.isEmpty()) return;
        settings.setValue("Global/Options/ScriptPath",QFileInfo(scriptPath).absolutePath());
        int size = settings.beginReadArray("Global/RecentScript");
        bool get = false;
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            if(settings.value("path").toString() == scriptPath){
                get = true;
                break;
            }
        }
        settings.endArray();
        if(!get) {
            settings.beginWriteArray("Global/RecentScript");
            settings.setArrayIndex(size);
            settings.setValue("path",scriptPath);
            settings.endArray();
            QAction *action = new QAction(scriptPath,scriptMenu);
            action->setStatusTip(scriptPath);
            scriptMenu->addAction(action);
            connect(action,&QAction::triggered,this,[=](){
                if(pyRun->isRunning()){
                    return;
                }
                runScriptFullName = scriptPath;
                pyRun->runScriptFile(scriptPath);
            });
        }
        runScriptFullName = scriptPath;
        pyRun->runScriptFile(scriptPath);
    });
    connect(cancelAction,&QAction::triggered,this,[=](){
        pyRun->cancelScript();
    });
    connect(startRecordingScriptAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) {
            return;
        }
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(!sessionsWindow->isRecordingScript()) {
            sessionsWindow->startRecordingScript();
        }
        bool isRS = sessionsWindow->isRecordingScript();
        startRecordingScriptAction->setEnabled(!isRS);
        stopRecordingScriptAction->setEnabled(isRS);
        canlcelRecordingScriptAction->setEnabled(isRS);
    });
    connect(stopRecordingScriptAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) {
            return;
        }
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isRecordingScript()) {
            sessionsWindow->stopRecordingScript();
        }
        bool isRS = sessionsWindow->isRecordingScript();
        startRecordingScriptAction->setEnabled(!isRS);
        stopRecordingScriptAction->setEnabled(isRS);
        canlcelRecordingScriptAction->setEnabled(isRS);
    });
    connect(canlcelRecordingScriptAction,&QAction::triggered,this,[=](){
        QWidget *widget = findCurrentFocusWidget();
        if(widget == nullptr) {
            return;
        }
        SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
        if(sessionsWindow->isRecordingScript()) {
            sessionsWindow->canlcelRecordingScript();
        }
        bool isRS = sessionsWindow->isRecordingScript();
        startRecordingScriptAction->setEnabled(!isRS);
        stopRecordingScriptAction->setEnabled(isRS);
        canlcelRecordingScriptAction->setEnabled(isRS);
    });
    connect(cleanAllRecentScriptAction, &QAction::triggered, this, [&]() {
        if(QMessageBox::question(this,tr("Clean All Recent script"),tr("Are you sure to clean all recent script?"),QMessageBox::Yes|QMessageBox::No) == QMessageBox::No) 
            return;
        
        GlobalSetting settings;
        settings.beginWriteArray("Global/RecentScript");
        settings.remove("");
        settings.endArray();
        scriptMenu->clear();
        scriptMenu->addAction(runAction);
        scriptMenu->addAction(cancelAction);
        scriptMenu->addSeparator();
        scriptMenu->addAction(startRecordingScriptAction);
        scriptMenu->addAction(stopRecordingScriptAction);
        scriptMenu->addAction(canlcelRecordingScriptAction);
        scriptMenu->addSeparator();
        scriptMenu->addAction(cleanAllRecentScriptAction);
    });
#endif
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
            startLocalShellSession(findCurrentFocusGroup(),-1,QString(),path);
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
                startLocalShellSession(findCurrentFocusGroup(),-1,QString(),path);
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
    connect(internalCommandAction,&QAction::triggered,this,[=](){
        internalCommandWindow->show();
    });
    connect(sshScanningAction,&QAction::triggered,this,[=](){
        bool isOk = false;
        int port = QInputDialog::getInt(this,tr("SSH Scanning"),tr("Port"),22,1,65535,1,&isOk);
        if(!isOk) return;
        netScanWindow->setScanPort(port);
        netScanWindow->show();
    });
    connect(pluginInfoAction,&QAction::triggered,this,[=](){
        pluginInfoWindow->show();
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
        sessionOptionsWindow->setReadOnly(true);
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
        } else if(action == portugueseAction) {
            this->language = QLocale(QLocale::Portuguese, QLocale::Brazil);
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
        } else if(action == germanAction) {
            this->language = QLocale::German;
        } else if(action == czechAction) {
            this->language = QLocale::Czech;
        } else if(action == arabicAction) {
            this->language = QLocale::Arabic;
        }

        setAppLangeuage(this->language);
        swapSideHboxLayout();
        QTimer::singleShot(0, this, [&](){
            if(mainWindow) mainWindow->fixMenuBarWidth();
        });
        foreach(pluginState_t pluginStruct, pluginList) {
            PluginInterface *iface = pluginStruct.iface;
            iface->setLanguage(this->language,qApp);
            iface->retranslateUi();
        }
        ui->retranslateUi(this);
        quickConnectWindow->retranslateUi();
        sessionManagerWidget->retranslateUi();
        startTftpSeverWindow->retranslateUi();
        pluginInfoWindow->retranslateUi();
        pluginViewerWidget->retranslateUi();
        notifictionWidget->retranslateUi();
        statusBarWidget->retranslateUi();
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
            QTimer::singleShot(100,this,[&](){
                mainWindow->show();
            });
        }
    });
    connect(lightThemeAction,&QAction::triggered,this,[=](){
        if(!isDarkTheme) return;
        isDarkTheme = false;
        qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
        if(themeColorEnable) QGoodWindow::setAppCustomTheme(isDarkTheme,themeColor);
        QFontIcon::instance()->setColor(Qt::black);
        menuAndToolBarRetranslateUi();
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            mainWidgetGroup->sessionTab->retranslateUi();
        }
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->reTranslateUi();
        }
        statusBarWidget->retranslateUi();
        globalOptionsWindow->switchTheme();
        GlobalSetting settings;
        settings.setValue("Global/Startup/dark_theme","false");
    });
    connect(darkThemeAction,&QAction::triggered,this,[=](){
        if(isDarkTheme) return;
        isDarkTheme = true;
        qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
        if(themeColorEnable) QGoodWindow::setAppCustomTheme(isDarkTheme,themeColor);
        QFontIcon::instance()->setColor(Qt::white);
        menuAndToolBarRetranslateUi();
        foreach(MainWidgetGroup *mainWidgetGroup, mainWidgetGroupList) {
            mainWidgetGroup->sessionTab->retranslateUi();
        }
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            sessionsWindow->reTranslateUi();
        }
        statusBarWidget->retranslateUi();
        globalOptionsWindow->switchTheme();
        GlobalSetting settings;
        settings.setValue("Global/Startup/dark_theme","true");
    });
    connect(themeColorAction,&QAction::triggered,this,[=](){
        GlobalSetting settings;
        QColor color = QColorDialog::getColor(themeColor, this, tr("Select color"));
        if (color.isValid()) {
            themeColor = color;
            themeColorEnable = true;
            qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
            QGoodWindow::setAppCustomTheme(isDarkTheme,themeColor);
            settings.setValue("Global/Startup/themeColor",themeColor);
        } else {
            themeColorEnable = false;
            qGoodStateHolder->setCurrentThemeDark(isDarkTheme);
        }
        settings.setValue("Global/Startup/themeColorEnable",themeColorEnable);
    });
    connect(exitAction, &QAction::triggered, this, [&](){
        qApp->quit();
    });
    connect(helpAction, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl("https://quardcrt.rtfd.io"));
    });
    connect(keyboradShortcutsReferenceAction, &QAction::triggered, this, [&]() {
        if(mainWindow) {
            CentralWidget::appKeyboradShortcutsReference(mainWindow);
        } else {
            CentralWidget::appKeyboradShortcutsReference(this);
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
    connect(privacyStatementAction, &QAction::triggered, this, [&]() {
        if(mainWindow) {
            CentralWidget::appPrivacyStatement(mainWindow);
        } else {
            CentralWidget::appPrivacyStatement(this);
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
#ifdef ENABLE_PYTHON
    connect(aboutPythonAction, &QAction::triggered, this, [&]() {
        aboutPython();
    });
#endif
}

void CentralWidget::setGlobalOptions(SessionsWindow *window) {
    window->setKeyBindings(keyMapManagerWindow->getCurrentKeyBinding());
    QString colorScheme = globalOptionsWindow->getCurrentColorScheme();
    if(colorScheme == "Custom") {
        window->setColorScheme(GlobalOptionsWindow::defaultColorScheme);
        QList<QColor> colorTable = globalOptionsWindow->getColorTable();
        int i = 0;
        foreach(const QColor &color, colorTable) {
            window->setANSIColor(i,color);
            i++;
        }
    } else {
        window->setColorScheme(colorScheme);
    }
    window->setTerminalFont(globalOptionsWindow->getCurrentFont());
    window->set_fix_quardCRT_issue33(globalOptionsWindow->isCurrentFontBuiltIn());
    window->setTerminalBackgroundMode(globalOptionsWindow->getBackgroundImageMode());
    window->setTerminalOpacity(globalOptionsWindow->getBackgroundImageOpacity());
    window->setHistorySize(globalOptionsWindow->getScrollbackLines());
    window->setKeyboardCursorShape(globalOptionsWindow->getCursorShape());
    window->setBlinkingCursor(globalOptionsWindow->getCursorBlink());
    window->setWordCharacters(globalOptionsWindow->getWordCharacters());
    window->setPreeditColorIndex(globalOptionsWindow->getPreeditColorIndex());
    window->setSelectedTextAccentColorTransparency(globalOptionsWindow->getSelectedTextAccentColorTransparency());
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
    window->setZmodemOnlie(!globalOptionsWindow->getDisableZmodemOnline());
    window->setZmodemUploadPath(globalOptionsWindow->getModemUploadPath());
    window->setZmodemDownloadPath(globalOptionsWindow->getModemDownloadPath());
    window->setConfirmMultilinePaste(globalOptionsWindow->getConfirmMultilinePaste());
    window->setTrimPastedTrailingNewlines(globalOptionsWindow->getTrimPastedTrailingNewlines());
    window->setEcho(globalOptionsWindow->getEcho());
    window->setCursorColor(globalOptionsWindow->getCursorColor());
    window->setLogOnEachLine(globalOptionsWindow->getLogOnEachLine());
    window->setLogFilePath(globalOptionsWindow->getLogFilePath());
    window->setRawLogFilePath(globalOptionsWindow->getLogRawFilePath());
    window->setScriptLogFilePath(globalOptionsWindow->getScriptLogFilePath());
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    window->setEnableHandleCtrlC(globalOptionsWindow->getEnableCtrlC());
#endif
    connect(window,&SessionsWindow::broadCastSendData,this,[=](const QByteArray &data){
        foreach(SessionsWindow *sessionsWindow, broadCastSessionList) {
            if(sessionsWindow != window) {
                sessionsWindow->reverseProxySendData(data);
            }
        }
    });
    connect(window,&SessionsWindow::requestReconnect,this,[=](void){
        reconnectSession(window);
    });
    connect(window,&SessionsWindow::termGetFocus,this,[=](void){
        foreach(MainWidgetGroup *group, mainWidgetGroupList) {
            if(group->sessionTab->indexOf(window->getMainWidget())>=0) {
                group->setActive(true && (currentLayout!=0));
                break;
            }
        }
    });
    connect(window,&SessionsWindow::termLostFocus,this,[=](void){
        foreach(MainWidgetGroup *group, mainWidgetGroupList) {
            if(group->sessionTab->indexOf(window->getMainWidget())>=0) {
                group->setActive(false);
                break;
            }
        }
    });
}

void CentralWidget::restoreSessionToSessionManager(void)
{
    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        int type = settings.value("type").toInt();
        QString group = settings.value("group", "/").toString();
        sessionManagerWidget->addSession(name,type,group);
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
    sessionManagerWidget->addSession(name,sessionsWindow->getSessionType(),"/");

    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Session");
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    settings.setArrayIndex(size);
    settings.setValue("name",name);
    settings.setValue("type",sessionsWindow->getSessionType());
    settings.setValue("group","/");
    switch(sessionsWindow->getSessionType()) {
    case SessionsWindow::Telnet:
        settings.setValue("hostname",sessionsWindow->getHostname());
        settings.setValue("port",sessionsWindow->getPort());
        settings.setValue("socketType",(int)(sessionsWindow->getSocketType()));
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
        infoData2Setting(&settings, data, name, "/");
        settings.endArray();
        sessionManagerWidget->addSession(name,data.type,"/");
    } else {
        QList<QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData>> infoList;
        for(int i=0;i<size;i++) {
            settings.setArrayIndex(i);
            QuickConnectWindow::QuickConnectData dataInfo;
            QString current_name;
            QString current_group;
            setting2InfoData(&settings, dataInfo, current_name, current_group, true);
            if(i == id) {
                infoList.append(QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData>(QPair<QString,QString>(name,"/"),data));
            }
            infoList.append(QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData>(QPair<QString,QString>(current_name,current_group),dataInfo));
            sessionManagerWidget->removeSession(current_name);
        }
        settings.endArray();
        settings.beginWriteArray("Global/Session");
        settings.remove("");
        settings.endArray();
        settings.beginWriteArray("Global/Session");
        // write infoMap to settings
        for(int i=0;i<infoList.size();i++) {
            QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData> info = infoList.at(i);
            QuickConnectWindow::QuickConnectData dataR = info.second;
            settings.setArrayIndex(i);
            sessionManagerWidget->addSession(info.first.first,dataR.type,info.first.second);
            if(i == id) {
                infoData2Setting(&settings, dataR, info.first.first,info.first.second);
            } else {
                infoData2Setting(&settings, dataR, info.first.first,info.first.second, true);
            }
        }
        settings.endArray();
    }

    return 0;
}

int64_t CentralWidget::removeSessionFromSessionManager(const QString &name)
{
    int64_t matched = -1;
    sessionManagerWidget->removeSession(name);
    GlobalSetting settings;
    QList<QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData>> infoList;
    int size = settings.beginReadArray("Global/Session");
    for(int i=0;i<size;i++) {
        settings.setArrayIndex(i);
        QuickConnectWindow::QuickConnectData data;
        QString current_name;
        QString current_group;
        setting2InfoData(&settings, data, current_name,current_group,true);
        if(current_name == name) {
            if(data.type == QuickConnectWindow::SSH2) {
                keyChainClass.deleteKey(name);
            }
            matched = i;
            continue;
        } else {
            infoList.append(QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData>(QPair<QString,QString>(current_name,current_group),data));
        }
    }
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    settings.remove("");
    settings.endArray();
    settings.beginWriteArray("Global/Session");
    // write infoMap to settings
    for(int i=0;i<infoList.size();i++) {
        QPair<QPair<QString,QString>,QuickConnectWindow::QuickConnectData> info = infoList.at(i);
        QuickConnectWindow::QuickConnectData dataR = info.second;
        settings.setArrayIndex(i);
        infoData2Setting(&settings, dataR, info.first.first,info.first.second, true);
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
        QString current_group = settings.value("group", "/").toString();
        if(current_name == name) {
            QuickConnectWindow::QuickConnectData data;
            if(setting2InfoData(&settings,data,current_name,current_group) != 0) {
                settings.endArray();
                QMessageBox::warning(this,tr("Warning"),tr("Session information get failed."),QMessageBox::Ok);
                return;
            }
            switch(data.type) {
            case QuickConnectWindow::Telnet:
                startTelnetSession(findCurrentFocusGroup(),-1,data.TelnetData.hostname,data.TelnetData.port,
                                   data.TelnetData.webSocket == 0?QTelnet::TCP:
                                   data.TelnetData.webSocket == 1?QTelnet::WEBSOCKET:
                                   QTelnet::SECUREWEBSOCKET, current_name);
                break;
            case QuickConnectWindow::Serial:
                startSerialSession(findCurrentFocusGroup(),-1,data.SerialData.portName,data.SerialData.baudRate,
                                   data.SerialData.dataBits,data.SerialData.parity,
                                   data.SerialData.stopBits,data.SerialData.flowControl,
                                   data.SerialData.xEnable, current_name);
                break;
            case QuickConnectWindow::LocalShell:
                startLocalShellSession(findCurrentFocusGroup(),-1,data.LocalShellData.command,globalOptionsWindow->getNewTabWorkPath(),current_name);
                break;
            case QuickConnectWindow::Raw:
                startRawSocketSession(findCurrentFocusGroup(),-1,data.RawData.hostname,data.RawData.port, current_name);
                break;
            case QuickConnectWindow::NamePipe:
                startNamePipeSession(findCurrentFocusGroup(),-1,data.NamePipeData.pipeName, current_name);
                break;
            case QuickConnectWindow::SSH2:
                startSSH2Session(findCurrentFocusGroup(),-1,data.SSH2Data.hostname,data.SSH2Data.port,
                                 data.SSH2Data.username,data.SSH2Data.password, current_name);
                break;
            case QuickConnectWindow::VNC:
                startVNCSession(findCurrentFocusGroup(),-1,data.VNCData.hostname,data.VNCData.port,
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
                case SessionsWindow::BroadCasted:
                    if(sessionsWindow->isLocked()) {
                        targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf084), Qt::yellow));
                    } else if(sessionsWindow->isInBroadCastList()) {
                        targetTab->setTabIcon(targetIndex,QFontIcon::icon(QChar(0xf08e), Qt::yellow));
                    } else if(sessionsWindow->getState() == SessionsWindow::Connected) {
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

QString CentralWidget::startTelnetSession(MainWidgetGroup *group, int groupIndex, QString hostname, quint16 port, QTelnet::SocketType type, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Telnet,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Telnet - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Telnet"));
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}

QString CentralWidget::startSerialSession(MainWidgetGroup *group, int groupIndex, QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::Serial,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Serial - ")+portName);
    sessionsWindow->setShortTitle(tr("Serial"));
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}

QString CentralWidget::startRawSocketSession(MainWidgetGroup *group, int groupIndex, QString hostname, quint16 port, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::RawSocket,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("Raw - ")+hostname+":"+QString::number(port));
    sessionsWindow->setShortTitle(tr("Raw"));
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}

QString CentralWidget::startNamePipeSession(MainWidgetGroup *group, int groupIndex, QString pipeName, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::NamePipe,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle(tr("NamePipe - ")+pipeName);
    sessionsWindow->setShortTitle(tr("NamePipe"));
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}

void CentralWidget::initSysEnv(void) {
#if defined(Q_OS_WIN)
    sysUsername = MiscWin32::getUserName();
    sysHostname = MiscWin32::getComputerName();
#elif defined(Q_OS_MAC)
    sysUsername = qEnvironmentVariable("USER");
    sysHostname = QHostInfo::localHostName().replace(".local","");
#else
    sysUsername = qEnvironmentVariable("USER");
    sysHostname = QHostInfo::localHostName();
#endif
}

QString CentralWidget::getDirAndcheckeSysName(const QString &title, SessionsWindow::ShellType shellType, QString overrideSysUsername, QString overrideSysHostname)
{
    QString currSysUsername = overrideSysUsername.isEmpty()?sysUsername:overrideSysUsername;
    QString currSysHostname = overrideSysHostname.isEmpty()?sysHostname:overrideSysHostname;
#if defined(Q_OS_WIN)
    if(shellType != SessionsWindow::WSL) {
        static QRegularExpression stdTitleFormat("^(\\S+)@(\\S+):(.*):(.*)$");
        if(stdTitleFormat.match(title).hasMatch()) {
            QString username = stdTitleFormat.match(title).captured(1);
            QString hostname = stdTitleFormat.match(title).captured(2);
            QString dir = stdTitleFormat.match(title).captured(3) + ":" +
                            stdTitleFormat.match(title).captured(4);
            hostname = hostname.left(hostname.indexOf(":"));
            QByteArray usernameArray = username.toLocal8Bit();
            QByteArray hostnameArray = hostname.toLocal8Bit();
            QByteArray sysUsernameArray = currSysUsername.toLocal8Bit();
            QByteArray sysHostnameArray = currSysHostname.toLocal8Bit();
            if((usernameArray == sysUsernameArray) && (hostnameArray == sysHostnameArray)) {
                return dir;
            }
        }
    } else {
        // FIXME: Why WSL Hostname is different from Windows Hostname?
        currSysHostname = currSysHostname.replace('_',"");
#else
    {
        Q_UNUSED(shellType);
#endif
        static QRegularExpression stdTitleFormat("^(\\S+)@(\\S+):(.*)$");
        if(stdTitleFormat.match(title).hasMatch()) {
            QString username = stdTitleFormat.match(title).captured(1);
            QString hostname = stdTitleFormat.match(title).captured(2);
            QString dir = stdTitleFormat.match(title).captured(3);
            dir = dir.remove(0, dir.indexOf(QChar::fromLatin1(' ')) + 1);
#if defined(Q_OS_WIN)
            if((username.toUpper() == currSysUsername.toUpper()) && (hostname.toUpper() == currSysHostname.toUpper())) {
                return dir;
            }
#else
            if((username == currSysUsername) && (hostname == currSysHostname)) {
                return dir;
            }
#endif
        }
    }

    return QString();
}

QString CentralWidget::startLocalShellSession(MainWidgetGroup *group, int groupIndex, const QString &command, const QString &workingDirectory, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
    setGlobalOptions(sessionsWindow);
    if(command.isEmpty()) {
        sessionsWindow->setLongTitle(tr("Local Shell"));
    } else {
        sessionsWindow->setLongTitle(tr("Local Shell - ")+command);
    }
    sessionsWindow->setShortTitle(tr("Local Shell"));
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = "Local Shell";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    QFileInfo workingDirectoryInfo(workingDirectory);
    sessionsWindow->setWorkingDirectory(workingDirectoryInfo.isDir()?workingDirectory:QDir::homePath());
#if defined(Q_OS_WIN)
    sessionsWindow->startLocalShellSession(command,globalOptionsWindow->getPowerShellProfile());
#else
    sessionsWindow->startLocalShellSession(command);
#endif
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            QString workDir = getDirAndcheckeSysName(newTitle,sessionsWindow->getShellType());
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}

#if defined(Q_OS_WIN)
QString CentralWidget::startWslSession(MainWidgetGroup *group, int groupIndex, const QString &command, const QString &workingDirectory, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::LocalShell,this);
    setGlobalOptions(sessionsWindow);
    if(command.isEmpty()) {
        sessionsWindow->setLongTitle("WSL");
    } else {
        sessionsWindow->setLongTitle("WSL - "+command);
    }
    sessionsWindow->setShortTitle("WSL");
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = "WSL";
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    QFileInfo workingDirectoryInfo(workingDirectory);
    sessionsWindow->setWorkingDirectory(workingDirectoryInfo.isDir()?workingDirectory:QDir::homePath());
    sessionsWindow->startLocalShellSession(command,globalOptionsWindow->getPowerShellProfile(),SessionsWindow::WSL);
    sessionList.push_back(sessionsWindow);
    connect(sessionsWindow, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
        if(title == 0 || title == 2) {
            sessionsWindow->setLongTitle(newTitle);
            QString workDir = getDirAndcheckeSysName(newTitle,sessionsWindow->getShellType(),sessionsWindow->getWSLUserName());
            if(!workDir.isEmpty()) {
                sessionsWindow->setShortTitle(workDir);
                // replace /mnt/xxx to XXX:
                static QRegularExpression wslDirFormat("^/mnt/(\\S+)/(.*)$");
                if(wslDirFormat.match(workDir).hasMatch()) {
                    QString workDirFix = wslDirFormat.match(workDir).captured(1).toUpper()+":/"+wslDirFormat.match(workDir).captured(2);
                    workDir = workDirFix;     
                } else if(workDir.startsWith("/mnt/")) {
                    workDir = workDir.remove(0, 5).toUpper()+":/";
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}
#endif

QString CentralWidget::startSSH2Session(MainWidgetGroup *group, int groupIndex, 
        QString hostname, quint16 port, QString username, QString password, QString name)
{
#ifdef ENABLE_SSH
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::SSH2,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle("SSH2 - "+username+"@"+hostname);
    sessionsWindow->setShortTitle("SSH2");
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = hostname;
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
    group->sessionTab->setCurrentIndex(index-1);
    return name;
#else
    Q_UNUSED(group);
    Q_UNUSED(groupIndex);
    Q_UNUSED(hostname);
    Q_UNUSED(port);
    Q_UNUSED(username);
    Q_UNUSED(password);
    return name;
#endif
}

QString CentralWidget::startVNCSession(MainWidgetGroup *group, int groupIndex, QString hostname, quint16 port, QString password, QString name)
{
    SessionsWindow *sessionsWindow = new SessionsWindow(SessionsWindow::VNC,this);
    setGlobalOptions(sessionsWindow);
    sessionsWindow->setLongTitle("VNC - "+hostname);
    sessionsWindow->setShortTitle("VNC");
    int index = group->sessionTab->addTab(groupIndex,sessionsWindow->getMainWidget(), sessionsWindow->getTitle());
    connectSessionStateChange(group->sessionTab,index,sessionsWindow);
    if(name.isEmpty()) {
        name = hostname;
        checkSessionName(name);
    }
    sessionsWindow->setName(name);
    sessionsWindow->startVNCSession(hostname,port,password);
    sessionList.push_back(sessionsWindow);
    group->sessionTab->setCurrentIndex(index-1);
    return name;
}

void CentralWidget::startSession(MainWidgetGroup *group, int groupIndex, QuickConnectWindow::QuickConnectData data) {
    if(data.type == QuickConnectWindow::Telnet) {
        QTelnet::SocketType type = QTelnet::TCP;
        type = (QTelnet::SocketType)data.TelnetData.webSocket;
        QString name = data.TelnetData.hostname;
        if(data.openInTab) {
            name = startTelnetSession(group,groupIndex,name,data.TelnetData.port,type);
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        }
    } else if(data.type == QuickConnectWindow::Serial) {
        QString name = data.SerialData.portName;
        if(data.openInTab) {
            name = startSerialSession(group,groupIndex,
                        name,data.SerialData.baudRate,
                        data.SerialData.dataBits,data.SerialData.parity,
                        data.SerialData.stopBits,data.SerialData.flowControl,
                        data.SerialData.xEnable);
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        }
    } else if(data.type == QuickConnectWindow::LocalShell) {
        QString name = "Local Shell";
        if(data.openInTab) {
            name = startLocalShellSession(group,groupIndex,data.LocalShellData.command,globalOptionsWindow->getNewTabWorkPath());
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        }
    } else if(data.type == QuickConnectWindow::Raw) {
        QString name = data.RawData.hostname;
        if(data.openInTab) {
            name = startRawSocketSession(group,groupIndex,name,data.RawData.port);
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        }
    } else if(data.type == QuickConnectWindow::NamePipe) {
        QString name = data.NamePipeData.pipeName;
        if(data.openInTab) {
            name = startNamePipeSession(group,groupIndex,name);
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        } 
    } else if(data.type == QuickConnectWindow::SSH2) {
        QString name = data.SSH2Data.hostname;
        if(data.openInTab) {
            name = startSSH2Session(group,groupIndex,name,data.SSH2Data.port,data.SSH2Data.username,data.SSH2Data.password);
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        }
    } else if(data.type == QuickConnectWindow::VNC) {
        QString name = data.VNCData.hostname;
        if(data.openInTab) {
            name = startVNCSession(group,groupIndex,name,data.VNCData.port,data.VNCData.password);
        } else {
            checkSessionName(name);
        }
        if(data.saveSession) {
            addSessionToSessionManager(data,name, !data.openInTab);
        }
    }
}

int CentralWidget::reconnectSession(SessionsWindow *sessionsWindow) {
    QWidget *widget = sessionsWindow->getMainWidget();
    foreach(MainWidgetGroup *group, mainWidgetGroupList) {
        int index = group->sessionTab->indexOf(widget);
        if(index != -1) {
            QString newName;
            QuickConnectWindow::QuickConnectData data;
            sessionWindow2InfoData(sessionsWindow, data, newName);
            sessionList.removeOne(sessionsWindow);
            if(sessionsWindow->isInBroadCastList()) {
                broadCastSessionList.removeOne(sessionsWindow);
            }
            group->sessionTab->removeTab(index);
            delete sessionsWindow;
            data.saveSession = false;
            data.openInTab = true;
            startSession(group,index,data);
            return 0;
        }
    }

    return -1;
}

int CentralWidget::stopSession(MainWidgetGroup *group, int index, bool force) {
    if(index <= 0) return -1;
    if(group->sessionTab->count() == 0) return -1;
    QWidget *widget = group->sessionTab->widget(index);
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    if(!sessionsWindow->isLocked()) {
        if(force) {
            sessionList.removeOne(sessionsWindow);
            if(sessionsWindow->isInBroadCastList()) {
                broadCastSessionList.removeOne(sessionsWindow);
            }
            group->sessionTab->removeTab(index);
            delete sessionsWindow;
        } else {
            if(sessionsWindow->getState() == SessionsWindow::Connected) {
                if(sessionsWindow->getSessionType() == SessionsWindow::LocalShell) {
                    if(!sessionsWindow->hasChildProcess()) {
                        sessionList.removeOne(sessionsWindow);
                        if(sessionsWindow->isInBroadCastList()) {
                            broadCastSessionList.removeOne(sessionsWindow);
                        }
                        group->sessionTab->removeTab(index);
                        delete sessionsWindow;
                        return 0;
                    }
                }
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Warning"), tr("Are you sure to disconnect \"") + sessionsWindow->getTitle() + tr("\" session?"),
                                            QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                #ifdef ENABLE_SSH
                    if(sessionsWindow->getSessionType() == SessionsWindow::SSH2) {
                        sftpWindow->hide();
                    }
                #endif
                    sessionList.removeOne(sessionsWindow);
                    if(sessionsWindow->isInBroadCastList()) {
                        broadCastSessionList.removeOne(sessionsWindow);
                    }
                    group->sessionTab->removeTab(index);
                    delete sessionsWindow;
                } else {
                    return -1;
                }
            } else {
                sessionList.removeOne(sessionsWindow);
                if(sessionsWindow->isInBroadCastList()) {
                    broadCastSessionList.removeOne(sessionsWindow);
                }
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
            if(stopSession(mainWidgetGroup,mainWidgetGroup->sessionTab->count(), force) == -1){
                return -1;
            }
        }
    }
    return 0;
}

int CentralWidget::cloneCurrentSession(MainWidgetGroup *group, QString name)
{
    if(group->sessionTab->count() == 0) return -1;
    QWidget *widget = group->sessionTab->currentWidget();
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    return cloneTargetSession(group,name,sessionsWindow);
}

int CentralWidget::cloneTargetSession(MainWidgetGroup *group, QString name,SessionsWindow *sessionsWindow)
{
    QWidget *widget = sessionsWindow->getMainWidget();
    SessionsWindow::SessionType type = sessionsWindow->getSessionType();
    switch(type) {
        case SessionsWindow::Serial:
        case SessionsWindow::RawSocket:
        case SessionsWindow::NamePipe: {
            QString newName;
            QuickConnectWindow::QuickConnectData data;
            sessionWindow2InfoData(sessionsWindow, data, newName);
            quickConnectWindow->reset();
            quickConnectWindow->setQuickConnectData(data);
            quickConnectWindow->show();
            break;
        }
        case SessionsWindow::Telnet:
        case SessionsWindow::LocalShell:
        case SessionsWindow::SSH2:
        case SessionsWindow::VNC: {
            SessionsWindow *sessionsWindowClone = new SessionsWindow(type,this);
            setGlobalOptions(sessionsWindowClone);
            sessionsWindowClone->setLongTitle(sessionsWindow->getLongTitle());
            sessionsWindowClone->setShortTitle(sessionsWindow->getShortTitle());
            sessionsWindowClone->setShowTitleType(sessionsWindow->getShowTitleType());
            int index = group->sessionTab->addTab(-1, sessionsWindowClone->getMainWidget(), group->sessionTab->tabTitle(group->sessionTab->indexOf(widget)));
            connectSessionStateChange(group->sessionTab,index,sessionsWindowClone);
            if(name.isEmpty()) {
                name = sessionsWindow->getName();
                checkSessionName(name);
            }
            sessionsWindowClone->setName(name);
        #if defined(Q_OS_WIN)
            sessionsWindowClone->cloneSession(sessionsWindow,globalOptionsWindow->getPowerShellProfile());
        #else
            sessionsWindowClone->cloneSession(sessionsWindow);
        #endif
            sessionList.push_back(sessionsWindowClone);
            connect(sessionsWindowClone, &SessionsWindow::titleChanged, this, [=](int title,const QString& newTitle){
                if(title == 0 || title == 2) {
                    sessionsWindowClone->setLongTitle(newTitle);
                    SessionsWindow::ShellType shellType = sessionsWindowClone->getShellType();
                    QString workDir = getDirAndcheckeSysName(newTitle,shellType,shellType==SessionsWindow::WSL?sessionsWindowClone->getWSLUserName():"");
                    if(!workDir.isEmpty()) {
                        if(shellType==SessionsWindow::WSL) {
                            sessionsWindowClone->setShortTitle(workDir);
                            // replace /mnt/xxx to XXX:
                            static QRegularExpression wslDirFormat("^/mnt/(\\S+)/(.*)$");
                            if(wslDirFormat.match(workDir).hasMatch()) {
                                QString workDirFix = wslDirFormat.match(workDir).captured(1).toUpper()+":/"+wslDirFormat.match(workDir).captured(2);
                                workDir = workDirFix;     
                            } else if(workDir.startsWith("/mnt/")) {
                                workDir = workDir.remove(0, 5).toUpper()+":/";
                            }
                        } else {
                            sessionsWindowClone->setShortTitle(workDir);
                            if(workDir.startsWith("~/")) {
                                workDir = workDir.replace(0,1,QDir::homePath());
                            }
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
            break;
        }
        default:
            break;
    }
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
            data.TelnetData.webSocket = sessionsWindow->getSocketType();
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

int CentralWidget::setting2InfoData(GlobalSetting *settings, QuickConnectWindow::QuickConnectData &data, QString &name, QString &group, bool skipPassword)
{
    name = settings->value("name").toString();
    group = settings->value("group", "/").toString();
    data.type = (QuickConnectWindow::QuickConnectType)(settings->value("type").toInt());
    switch(data.type) {
    case QuickConnectWindow::Telnet:
        data.TelnetData.hostname = settings->value("hostname").toString();
        data.TelnetData.port = settings->value("port").toInt();
        data.TelnetData.webSocket = settings->value("socketType").toInt();
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

void CentralWidget::infoData2Setting(GlobalSetting *settings,const QuickConnectWindow::QuickConnectData &data,const QString &name,const QString &group,bool skipPassword) {
    settings->setValue("name",name);
    settings->setValue("type",data.type);
    settings->setValue("group",group);
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

void CentralWidget::addBookmark(const QString &path) {
    QAction *action = new QAction(path,bookmarkMenu);
    action->setStatusTip(path);
    bookmarkMenu->addAction(action);
    connect(action,&QAction::triggered,this,[&,path](){
        startLocalShellSession(findCurrentFocusGroup(),-1,QString(),path);
    });
    GlobalSetting settings;
    int size = settings.beginReadArray("Global/Bookmark");
    settings.endArray();
    settings.beginWriteArray("Global/Bookmark");
    settings.setArrayIndex(size);
    settings.setValue("path",path);
    settings.endArray();
}

QMenu *CentralWidget::createPopupMenu() {
    QMenu *menu = new QMenu(this);
    menu->addAction(menuBarAction);
    menu->addAction(toolBarAction);
    menu->addAction(cmdWindowAction);
    menu->addAction(fullScreenAction);
    return menu;
}

void CentralWidget::appPrivacyStatement(QWidget *parent)
{
    QMessageBox::about(parent, tr("Privacy Statement"),
                           QString("<p>") + 
                           tr("This software is a terminal emulator that provides a secure shell (SSH) connection, telnet connection, serial port connection, and other functions.") +
                           QString("</p><p>") + 
                           tr("This software does not collect any personal information, and does not send any information to the server.") +
                           QString("</p><p>") + 
                           tr("This software does not contain any malicious code, and does not contain any backdoors.") +
                           QString("</p><p>") + 
                           tr("This software is open source software, you can view the source code on the github website.") +
                           QString("</p><p>") + 
                           tr("This software is provided as is, without any warranty.") +
                           QString("</p><p>") + 
                           tr("If you have any questions or suggestions, please contact the author.") +
                           QString("</p><p></p><p>") + tr("Author") +
                           QString(":<a href='mailto:2014500726@smail.xtu.edu.cn'>2014500726@smail.xtu.edu.cn</a></p>") +
                           "<p>Github:<a href='https://github.com/QQxiaoming/quardCRT'>https://github.com/QQxiaoming</a></p>" +
                           "<p>Gitee:<a href='https://gitee.com/QQxiaoming/quardCRT'>https://gitee.com/QQxiaoming</a></p>"
                       );
}

void CentralWidget::appAbout(QWidget *parent)
{
    uint32_t timestamps = DATE_TIMESTAMPS_TAG.toUInt();
    uint32_t current = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
    uint32_t days = (current - timestamps) / 86400;
    QMessageBox::about(parent, tr("About"),
                           QString("<p>") + 
                           tr("Version") +
                           QString("</p><p>&nbsp;<a href='https://github.com/QQxiaoming/quardCRT/releases/tag/%0'>%0</a></p><p>").arg(VERSION) +
                           tr("Commit") +
                           QString("</p><p>&nbsp;<a href='https://github.com/QQxiaoming/quardCRT/commit/%0'>%1</a></p><p>").arg(HASH_TAG).arg(GIT_TAG) +
                           tr("Date") +
                           QString("</p><p>&nbsp;%0 (%1 days ago)</p><p>").arg(DATE_TAG).arg(days) +
                           tr("Author") +
                           "</p><p>&nbsp;<a href='mailto:2014500726@smail.xtu.edu.cn'>2014500726@smail.xtu.edu.cn</a></p><p>" +
                           tr("Website") +
                           "</p><p>&nbsp;<a href='https://github.com/QQxiaoming/quardCRT'>https://github.com/QQxiaoming</a></p>" +
                           "<p>&nbsp;<a href='https://gitee.com/QQxiaoming/quardCRT'>https://gitee.com/QQxiaoming</a></p>"
                       );
}

#ifdef ENABLE_PYTHON
void CentralWidget::aboutPython(void){
    QMessageBox msgBox(QMessageBox::Information,tr("About Python"),pyRun->getPyVersion(),QMessageBox::NoButton,nullptr);
    msgBox.setIconPixmap(QPixmap(":/icons/icons/aboutpython.png").scaled(64,64));
    msgBox.exec();
}
#endif

void CentralWidget::appKeyboradShortcutsReference(QWidget *parent)
{
    QMessageBox::about(parent, tr("Keyborad Shortcuts Reference"),
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

QStringList CentralWidget::requestZmodemUploadList(void) {
    QStringList list = zmodemUploadList;
    zmodemUploadList.clear();
    startZmodemUploadAction->setEnabled(!zmodemUploadList.isEmpty());
    return list;
}

void CentralWidget::swapSideHboxLayout(void) {
    sideHboxLayout->removeWidget(sessionManagerPushButton);
    sideHboxLayout->removeWidget(pluginViewerPushButton);
    if(qApp->isRightToLeft()) {
        sideHboxLayout->addWidget(sessionManagerPushButton);
        sideHboxLayout->addWidget(pluginViewerPushButton);
    } else {
        sideHboxLayout->addWidget(pluginViewerPushButton);
        sideHboxLayout->addWidget(sessionManagerPushButton);
    }
}

int CentralWidget::nextGroupID(int id) {
    QList<int> idList[8] = {
        {0},      //singleLayout
        {0,1},    //twoColumnsLayout
        {0,1,6},  //threeColumnsLayout
        {0,2},    //twoRowsLayout
        {0,2,4},  //threeRowsLayout
        {0,1,2,3},//gridLayout
        {0,1,3},  //twoRowsRightLayout
        {0,4,5},  //twoColumnsBottomLayout
    };
    QList<int> current = idList[currentLayout];
    int index = current.indexOf(id);
    if(index < current.size() && index >= 0) {
        if(index == (current.size()-1)) {
            return current.at(0);
        } else {
            return current.at(index+1);
        }
    }
    return 0;
}

void CentralWidget::movetabWhenLayoutChange(int oldL, int newL) {
    QList<int> idList[8] = {
        {0},      //singleLayout
        {0,1},    //twoColumnsLayout
        {0,1,6},  //threeColumnsLayout
        {0,2},    //twoRowsLayout
        {0,2,4},  //threeRowsLayout
        {0,1,2,3},//gridLayout
        {0,1,3},  //twoRowsRightLayout
        {0,4,5},  //twoColumnsBottomLayout
    };
    if(oldL == newL)
        return;
    QList<int> oldIdList = idList[oldL];
    QList<int> newIdList = idList[newL];
    if(oldIdList.size() <= newIdList.size()) {
        QList<int> movnum;
        for(int i=0;i<oldIdList.size();i++) {
            if(oldIdList[i] != newIdList[i]) {
                movnum.append(mainWidgetGroupList.at(oldIdList[i])->sessionTab->count());
            } else {
                movnum.append(0);
            }
        }
        for(int i=0;i<oldIdList.size();i++) {
            int j = movnum.at(i);
            while(j) {
                moveToAnotherTab(oldIdList[i],newIdList[i],1);
                j--;
            }
        }
    } else {
        QList<int> movnum;
        for(int i=0;i<newIdList.size();i++) {
            if(oldIdList[i] != newIdList[i]) {
                movnum.append(mainWidgetGroupList.at(oldIdList[i])->sessionTab->count());
            } else {
                movnum.append(0);
            }
        }
        for(int i=newIdList.size();i<oldIdList.size();i++) {
            if(oldIdList[i] != newIdList[newIdList.size()-1]) {
                movnum.append(mainWidgetGroupList.at(oldIdList[i])->sessionTab->count());
            } else {
                movnum.append(0);
            }
        }
        for(int i=0;i<newIdList.size();i++) {
            int j = movnum.at(i);
            while(j) {
                moveToAnotherTab(oldIdList[i],newIdList[i],1);
                j--;
            }
        }
        for(int i=newIdList.size();i<oldIdList.size();i++) {
            int j = movnum.at(i);
            while(j) {
                moveToAnotherTab(oldIdList[i],newIdList[newIdList.size()-1],1);
                j--;
            }
        }
    }
};

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
    qApp->setLayoutDirection(Qt::LeftToRight);
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
    case QLocale::Portuguese:
        if(qtTranslator->load("qt_pt_BR.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_pt_BR.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_pt_BR.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","pt_BR");
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
    case QLocale::German:
        if(qtTranslator->load("qt_de.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_de.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_de_DE.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","de_DE");
        break;
    case QLocale::Czech:
        if(qtTranslator->load("qt_cs.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_cs.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_cs_CZ.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","cs_CZ");
        break;
    case QLocale::Arabic:
        if(qtTranslator->load("qt_ar.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_ar.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quardCRT_ar_SA.qm"))
            qApp->installTranslator(appTranslator);
        settings.setValue("Global/Startup/language","ar_SA");
        qApp->setLayoutDirection(Qt::RightToLeft);
        break;
    }
}

#ifdef ENABLE_PYTHON
int CentralWidget::se_sessionConnect(const QString &cmd,int id) {
    QStringList args = cmd.split(" ",Qt::SkipEmptyParts);
    for(int i = 0; i < args.size(); i++) {
        if(args[i] == "-telnet") {
            if(i+2 < args.size()) {
                QString hostname = args[i+1];
                quint16 port = args[i+2].toUShort();
                startTelnetSession(findCurrentFocusGroup(),-1,hostname,port,QTelnet::TCP);
                return 0;
            }
        } else if(args[i] == "-serial") {
            if(i+6 < args.size()) {
                QString portName = args[i+1];
                int baudRate = args[i+2].toInt();
                int dataBits = args[i+3].toInt();
                int parity = args[i+4].toInt();
                int stopBits = args[i+5].toInt();
                bool flowControl = args[i+6].toInt();
                bool xEnable = args[i+7].toInt();
                startSerialSession(findCurrentFocusGroup(),-1,portName,baudRate,dataBits,parity,stopBits,flowControl,xEnable);
                return 0;
            }
        } else if(args[i] == "-localshell") {
            if(i+1 < args.size()) {
                QString path = args[i+1];
                startLocalShellSession(findCurrentFocusGroup(),-1,QString(),path);
                return 0;
            } 
        } else if(args[i] == "-raw") {
            if(i+2 < args.size()) {
                QString hostname = args[i+1];
                quint16 port = args[i+2].toUShort();
                startRawSocketSession(findCurrentFocusGroup(),-1,hostname,port);
                return 0;
            } 
        } else if(args[i] == "-namepipe") {
            if(i+1 < args.size()) {
                QString pipeName = args[i+1];
                startNamePipeSession(findCurrentFocusGroup(),-1,pipeName);
                return 0;
            }
        } else if(args[i] == "-ssh2") {
            if(i+4 < args.size()) {
                QString hostname = args[i+1];
                quint16 port = args[i+2].toUShort();
                QString username = args[i+3];
                QString password = args[i+4];
                startSSH2Session(findCurrentFocusGroup(),-1,hostname,port,username,password);
                return 0;
            }
        } else if(args[i] == "-vnc") {
            if(i+3 < args.size()) {
                QString hostname = args[i+1];
                quint16 port = args[i+2].toUShort();
                QString password = args[i+3];
                startVNCSession(findCurrentFocusGroup(),-1,hostname,port,password);
                return 0;
            }
        } else if(args[i] == "-s") {
            if(i+1 < args.size()) {
                QString name = args[i+1];
                connectSessionFromSessionManager(name);
                return 0;
            } 
        } else if(args[i] == "-clone") {
            if(id == -1) {
                id = se_getActiveSessionId();
                if(id == -1) return -1;
            }
            SessionsWindow *sessionsWindow = sessionList.at(id);
            cloneTargetSession(findCurrentFocusGroup(),QString(),sessionsWindow);
            return 0;
        }
    }

    return -1;
}

void CentralWidget::se_sessionDisconnect(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    if(sessionsWindow->isLocked()) return;
    if(sessionsWindow->getState() == SessionsWindow::Connected) {
        if(sessionsWindow->getSessionType() == SessionsWindow::SSH2) {
            sftpWindow->hide();
        }
        sessionsWindow->disconnect();
    }
}

void CentralWidget::se_sessionLog(int enable,int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    if(sessionsWindow->isLog() == enable) return;
    sessionsWindow->setLog(enable);
}

QString CentralWidget::se_getActivePrinter(void) {
    return printerName;
}

void CentralWidget::se_setActivePrinter(const QString &name) {
    printerName = name;
}

QString CentralWidget::se_getScriptFullName(void) {
    return runScriptFullName;
}

int CentralWidget::se_getActiveTabId(void) {
    MainWidgetGroup *group = findCurrentFocusGroup();
    return mainWidgetGroupList.indexOf(group);
}

int CentralWidget::se_getActiveSessionId(void) {
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return -1;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();                
    if(sessionsWindow) {
        return sessionList.indexOf(sessionsWindow);
    }
    return -1;
}

void CentralWidget::se_activateWindow(void) {
    if(mainWindow) {
        activateWindow();
    } else {
        mainWindow->activateWindow();
    }
}

bool CentralWidget::se_getWindowActive(void) {
    if(mainWindow) {
        return mainWindow->isActiveWindow();
    } else {
        return isActiveWindow();
    }
}

void CentralWidget::se_windowShow(int type) {
    if(mainWindow) {
        switch(type) {
        case 0:
            mainWindow->hide();
            break;
        case 1:
            mainWindow->showNormal();
            break;
        case 2:
            mainWindow->showMinimized();
            break;
        case 3:
            mainWindow->showMaximized();
            break;
        }
    } else {
        switch(type) {
        case 0:
            hide();
            break;
        case 1:
            showNormal();
            break;
        case 2:
            showMinimized();
            break;
        case 3:
            showMaximized();
            break;
        }
    }
}

int CentralWidget::se_getWindowShowType(void) {
    int ret = 1;
    if(mainWindow) {
        if(mainWindow->isHidden()) {
            ret = 0;
        } else if(mainWindow->isMaximized()) {
            ret = 3;
        } else if(mainWindow->isMinimized()) {
            ret = 2;
        } else {
            ret = 1;
        }
    } else {
        if(isHidden()) {
            ret = 0;
        } else if(isMaximized()) {
            ret = 3;
        } else if(isMinimized()) {
            ret = 2;
        } else {
            ret = 1;
        }
    }
    return ret;
}

QString CentralWidget::se_getCommandWindowText(void) {
    MainWidgetGroup *group = findCurrentFocusGroup();
    CommandWidget *cmdWidget = group->commandWidget;
    return cmdWidget->getCmd();
}

void CentralWidget::se_setCommandWindowText(const QString &text) {
    MainWidgetGroup *group = findCurrentFocusGroup();
    CommandWidget *cmdWidget = group->commandWidget;
    cmdWidget->setCmd(text);
}

void CentralWidget::se_commandWindowSend(void) {
    MainWidgetGroup *group = findCurrentFocusGroup();
    CommandWidget *cmdWidget = group->commandWidget;
    cmdWidget->sendCurrentData();
}

bool CentralWidget::se_getCommandWindowVisibled(void) {
    return cmdWindowAction->isChecked();
}

void CentralWidget::se_getCommandWindowVisibled(bool enable) {
    if(cmdWindowAction->isChecked() != enable) {
        cmdWindowAction->trigger();
    }
}

QString CentralWidget::se_getDownloadFolder(void) {
    return globalOptionsWindow->getModemDownloadPath();
}

void CentralWidget::se_addToUploadList(const QString &file) {
    zmodemUploadList.append(file);
    startZmodemUploadAction->setEnabled(!zmodemUploadList.isEmpty());
}

void CentralWidget::se_clearUploadList(void) {
    zmodemUploadList.clear();
    startZmodemUploadAction->setEnabled(!zmodemUploadList.isEmpty());
}

int CentralWidget::se_receiveKermit(void){
    receiveKermitAction->trigger();
    return 0;
}

int CentralWidget::se_sendKermit(const QStringList &files){
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return -1;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    sessionsWindow->sendFileUseKermit(files);
    return 0;
}

int CentralWidget::se_receiveXmodem(const QString &file){
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return -1;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    sessionsWindow->recvFileUseXModem(file);
    return 0;
}

int CentralWidget::se_sendXmodem(const QString &file){
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return -1;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    sessionsWindow->sendFileUseXModem(file,globalOptionsWindow->getXYModem1K());
    return 0;
}

int CentralWidget::se_receiveYmodem(void){
    receiveYmodemAction->trigger();
    return 0;
}

int CentralWidget::se_sendYmodem(const QStringList &files){
    QWidget *widget = findCurrentFocusWidget();
    if(widget == nullptr) return -1;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
    sessionsWindow->sendFileUseYModem(files,globalOptionsWindow->getXYModem1K());
    return 0;
}

int CentralWidget::se_sendZmodem(void) {
    startZmodemUploadAction->trigger();
    return 0;
}

void CentralWidget::se_messageNotifications(const QString &message) {
    ui->statusBar->showMessage(message);
}

int CentralWidget::se_screenSend(const QString &str, bool synchronous, int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    auto proxySendCommand = [&,sessionsWindow](const QString &str){
        sessionsWindow->proxySendData(str.toLatin1());
    };
    if(synchronous) {
        proxySendCommand(str);
        return 0;
    } else {
        QTimer::singleShot(0, this, [=]{
            proxySendCommand(str);
        });
        return 0;
    }
}

int CentralWidget::se_installWaitString(const QStringList &strList, int timeout, bool bcaseInsensitive, int mode, int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    if(sessionsWindow->isLocked()) return -1;
    connect(sessionsWindow,&SessionsWindow::waitForStringFinished,this,[=](const QString &s, int matchIndex){
        pyRun->emitWaitForStringFinished(s,matchIndex);
        disconnect(sessionsWindow,&SessionsWindow::waitForStringFinished,0,0);
    });
    // TODO:not implemented timeout
    return sessionsWindow->installWaitString(strList,timeout,bcaseInsensitive,mode);
}

int CentralWidget::se_screenGetCurrentRow(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->getCursorLineCount();
}

int CentralWidget::se_screenGetCurrentColumn(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->getCursorColumnCount();
}

int CentralWidget::se_screenGetRows(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->getLineCount();
}

int CentralWidget::se_screenGetColumns(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->getColumnCount();
}

QString CentralWidget::se_screenGetSelection(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return QString();
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->selectedText();
}

void CentralWidget::se_screenClear(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    sessionsWindow->clearScreen();
}

QString CentralWidget::se_screenGet(int row1, int col1, int row2, int col2, int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return QString();
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->screenGet(row1, col1, row2, col2, 1); //mode 1
}

QString CentralWidget::se_screenGet2(int row1, int col1, int row2, int col2, int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return QString();
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->screenGet(row1, col1, row2, col2, 2); //mode 2
}

void CentralWidget::se_screenPrint(int id) {
    //TODO: id
    Q_UNUSED(id);
    printScreenAction->trigger();
}

void CentralWidget::se_screenShortcut(const QString &path, int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    QString fileName = path;
    if(!fileName.endsWith(".jpg")) fileName.append(".jpg");
    sessionsWindow->screenShot(fileName);
    ui->statusBar->showMessage(tr("Screenshot saved to %1").arg(fileName),3000);
}

void CentralWidget::se_screenSendKeys(const QList<Qt::Key> &keys, int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    QWidget *widget = sessionsWindow->getMainWidget();
    if(widget == nullptr) return;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    for(int i = 0; i < keys.size(); i++) {
        if(keys[i] == Qt::Key_Alt) {
            modifiers |= Qt::AltModifier;
        } else if(keys[i] == Qt::Key_Control) {
            modifiers |= Qt::ControlModifier;
        } else if(keys[i] == Qt::Key_Shift) {
            modifiers |= Qt::ShiftModifier;
        } else if(keys[i] == Qt::Key_Meta) {
            modifiers |= Qt::MetaModifier;
        }
    }
    Qt::Key key = Qt::Key_unknown;
    for(int i = 0; i < keys.size(); i++) {
        if(keys[i] == Qt::Key_Alt || keys[i] == Qt::Key_Control || keys[i] == Qt::Key_Shift || keys[i] == Qt::Key_Meta) {
            continue;
        }
        key = keys[i];
        break;
    }
    if(key != Qt::Key_unknown) {
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress,key,modifiers);
        QCoreApplication::postEvent(widget,keyEvent);
        keyEvent = new QKeyEvent(QEvent::KeyRelease,key,modifiers);
        QCoreApplication::postEvent(widget,keyEvent);
    }
}

bool CentralWidget::se_sessionGetLocked(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return false;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->isLocked();
}

bool CentralWidget::se_sessionGetConnected(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return false;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->getState() == SessionsWindow::Connected;
}

bool CentralWidget::se_sessionGetLogging(int id) {
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return false;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    return sessionsWindow->isLog();
}

int CentralWidget::se_sessionLock(const QString &password, int lockallsessions,int id) {
    if(lockallsessions) {
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(sessionsWindow->isLocked()) continue;
            sessionsWindow->lockSession(password);
        }
        return 0;
    }
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    sessionsWindow->lockSession(password);
    return 0;
}

int CentralWidget::se_sessionUnlock(const QString &password, int lockallsessions,int id) {
    if(lockallsessions) {
        foreach(SessionsWindow *sessionsWindow, sessionList) {
            if(!sessionsWindow->isLocked()) continue;
            sessionsWindow->unlockSession(password);
        }
        return 0;
    }
    if(id == -1) {
        id = se_getActiveSessionId();
        if(id == -1) return -1;
    }
    SessionsWindow *sessionsWindow = sessionList.at(id);
    sessionsWindow->unlockSession(password);
    return 0;
}

int CentralWidget::se_tabGetnumber(int id) {
    if(id < mainWidgetGroupList.size()) {
        MainWidgetGroup *group = mainWidgetGroupList.at(id);
        return group->sessionTab->count();
    }
    return 0;
}

int CentralWidget::se_tabCheckScreenId(int tabId, int screenId) {
    if(tabId >= mainWidgetGroupList.size()) return -1;
    MainWidgetGroup *group = mainWidgetGroupList.at(tabId);
    QWidget *widget = group->sessionTab->widget(screenId+1);
    if(widget == nullptr) return -1;
    SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();            
    if(sessionsWindow == nullptr) return -1;
    return sessionList.indexOf(sessionsWindow);
}

void CentralWidget::se_tabActivate(int tabId, int screenId) {
    if(tabId >= mainWidgetGroupList.size()) return;
    if(screenId == -1) screenId = 0;
    MainWidgetGroup *group = mainWidgetGroupList.at(tabId);
    group->sessionTab->setCurrentIndex(screenId);
    group->sessionTab->currentWidget()->setFocus();
}
#endif

MainWindow::MainWindow(QString dir, CentralWidget::StartupUIMode mode, QLocale lang, bool isDark, 
    QString start_know_session, bool disable_plugin, QWidget *parent) 
    : QGoodWindow(parent) {
    m_central_widget = new CentralWidget(dir,mode,lang,isDark,start_know_session,disable_plugin,this);
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

        QTimer::singleShot(0, this, [&]{
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
    } else {
        setNativeCaptionButtonsVisibleOnMac(true);
    }

    connect(qGoodStateHolder, &QGoodStateHolder::currentThemeChanged, this, [](){
        if (qGoodStateHolder->isCurrentThemeDark())
            QGoodWindow::setAppDarkTheme();
        else
            QGoodWindow::setAppLightTheme();
    });
    connect(this, &QGoodWindow::systemThemeChanged, this, [&]{
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
    delete m_central_widget;
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
