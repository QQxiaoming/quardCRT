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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include <QLocale>
#include <QSplitter>
#include <QLineEdit>
#include <QSocketNotifier>
#include <QActionGroup>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QDir>
#include <QStatusTipEvent>
#include <QFontMetrics>
#include <QDateTime>
#include <QMediaCaptureSession>
#include <QWindowCapture>
#include <QMediaRecorder>

#include "mainwidgetgroup.h"
#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "internalcommandwindow.h"
#include "globaloptionswindow.h"
#include "hexviewwindow.h"
#include "sessionmanagerwidget.h"
#include "notifictionwidget.h"
#include "pluginviewerwidget.h"
#include "commandwidget.h"
#include "statusbarwidget.h"
#include "starttftpseverwindow.h"
#include "locksessionwindow.h"
#include "sessionoptionswindow.h"
#include "globalsetting.h"
#include "qtftp.h"
#ifdef ENABLE_SSH
#include "sftpwindow.h"
#endif
#include "netscanwindow.h"
#include "plugininfowindow.h"
#include "keychainclass.h"
#include "plugininterface.h"
#ifdef ENABLE_PYTHON
#include "pyrun.h"
#endif
#include "QGoodWindow"
#include "QGoodCentralWidget"

extern QString VERSION;
extern QString GIT_TAG;
extern QString DATE_TAG;
extern QString DATE_TIMESTAMPS_TAG;
extern QString HASH_TAG;
extern QString SHORT_HASH_TAG;
extern QDateTime START_TIME;

class FloatingTab : public QDialog {
    Q_OBJECT

public:
    explicit FloatingTab(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::Window);
        resize(800,480);
        setLayout(new QVBoxLayout);
    }
    void forceClose(void) {
        doNotAskClose = true;
        close();
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        if(doNotAskClose) {
            QDialog::closeEvent(event);
        } else {
            QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Close"),tr("Do you want to close this window?"),QMessageBox::Yes|QMessageBox::No);
            if(ret == QMessageBox::Yes) {
                doNotAskClose = true;
                event->accept();
            } else {
                event->ignore();
            }
        }
    }

private:
    bool doNotAskClose = false;
};

QT_BEGIN_NAMESPACE
namespace Ui { class CentralWidget; }
QT_END_NAMESPACE

class MainWindow;
class CentralWidget : public QMainWindow
{
    Q_OBJECT

public:
    enum StartupUIMode {
        STDUI_MODE = 0,
        MINIUI_MODE,
    };
    CentralWidget(QString dir = QString(), 
                  StartupUIMode mode = STDUI_MODE, 
                  QLocale lang = QLocale(QLocale::English), 
                  bool isDark = true, 
                  QString start_know_session = QString(), 
                  bool disable_plugin = false,
                  QWidget *parent = nullptr);
    ~CentralWidget();
    static void appPrivacyStatement(QWidget *parent = nullptr);
    static void appAbout(QWidget *parent = nullptr);
#ifdef ENABLE_PYTHON
    void aboutPython(void);
#endif
    static void appKeyboradShortcutsReference(QWidget *parent = nullptr);
    static void setAppLangeuage(QLocale lang);
    void checkCloseEvent(QCloseEvent *event);
    void checkStatusTipEvent(QStatusTipEvent *event);
    QStringList requestZmodemUploadList(void);

#ifdef ENABLE_PYTHON
    //script engine need
    int se_getActiveTabId(void);
    int se_getActiveSessionId(void);
    QString se_getActivePrinter(void);
    void se_setActivePrinter(const QString &name);
    QString se_getScriptFullName(void);
    void se_activateWindow(void);
    void se_windowShow(int type);
    int se_getWindowShowType(void);
    bool se_getWindowActive(void);
    QString se_getCommandWindowText(void);
    void se_setCommandWindowText(const QString &text);
    void se_commandWindowSend(void);
    bool se_getCommandWindowVisibled(void);
    void se_getCommandWindowVisibled(bool enable);
    QString se_getDownloadFolder(void);
    void se_addToUploadList(const QString &file);
    void se_clearUploadList(void);
    int se_receiveKermit(void);
    int se_sendKermit(const QStringList &files);
    int se_receiveXmodem(const QString &file);
    int se_sendXmodem(const QString &file);
    int se_receiveYmodem(void);
    int se_sendYmodem(const QStringList &files);
    int se_sendZmodem(void);
    void se_messageNotifications(const QString &message);

    int se_screenSend(const QString &str, bool synchronous, int id = -1);
    int se_installWaitString(const QStringList &strList, int timeout, bool bcaseInsensitive, int mode, int id = -1);
    int se_screenGetCurrentRow(int id = -1);
    int se_screenGetCurrentColumn(int id = -1);
    int se_screenGetRows(int id = -1);
    int se_screenGetColumns(int id = -1);
    QString se_screenGetSelection(int id = -1);
    void se_screenClear(int id = -1);
    QString se_screenGet(int row1, int col1, int row2, int col2, int id = -1);
    QString se_screenGet2(int row1, int col1, int row2, int col2, int id = -1);
    void se_screenPrint(int id = -1);
    void se_screenShortcut(const QString &path, int id = -1);
    void se_screenSendKeys(const QList<Qt::Key> &keys, int id = -1);

    int se_sessionConnect(const QString &cmd,int id = -1);
    void se_sessionDisconnect(int id = -1);
    void se_sessionLog(int enable,int id = -1);
    bool se_sessionGetLocked(int id = -1);
    bool se_sessionGetConnected(int id = -1);
    bool se_sessionGetLogging(int id = -1);
    int se_sessionLock(const QString &password, int lockallsessions,int id = -1);
    int se_sessionUnlock(const QString &password, int lockallsessions,int id = -1);

    int se_tabGetnumber(int id);
    int se_tabCheckScreenId(int tabId, int screenId);
    void se_tabActivate(int tabId, int screenId);
#endif

private:
    void menuAndToolBarInit(bool disable_plugin);
    void menuAndToolBarRetranslateUi(void);
    void menuAndToolBarConnectSignals(void);
    QString startTelnetSession(MainWidgetGroup *group, int groupIndex,  QString hostname, quint16 port, QTelnet::SocketType type, QString name = QString());
    QString startSerialSession(MainWidgetGroup *group, int groupIndex,  QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable, QString name = QString());
    QString startLocalShellSession(MainWidgetGroup *group, int groupIndex,  const QString &command = QString(), const QString &workingDirectory = QDir::homePath(), QString name = QString());
#if defined(Q_OS_WIN)
    QString startWslSession(MainWidgetGroup *group, int groupIndex,  const QString &command = QString(), const QString &workingDirectory = QDir::homePath(), QString name = QString());
#endif
    QString startRawSocketSession(MainWidgetGroup *group, int groupIndex,  QString hostname, quint16 port, QString name = QString());
    QString startNamePipeSession(MainWidgetGroup *group, int groupIndex,  QString namePipe, QString name = QString());
    QString startSSH2Session(MainWidgetGroup *group, int groupIndex, 
        QString hostname, quint16 port, QString username, QString password, QString name = QString());
    QString startVNCSession(MainWidgetGroup *group, int groupIndex,  QString hostname, quint16 port, QString password, QString name = QString());
    void startSession(MainWidgetGroup *group, int groupIndex, QuickConnectWindow::QuickConnectData data);
    int reconnectSession(SessionsWindow *sessionsWindow);
    int stopSession(MainWidgetGroup *group, int index, bool force = false);
    int stopAllSession(bool force = false);
    int cloneTargetSession(MainWidgetGroup *group, QString name,SessionsWindow *sessionsWindow);
    int cloneCurrentSession(MainWidgetGroup *group, QString name = QString());
    MainWidgetGroup *findCurrentFocusGroup(bool forceFind = true, bool includefloating = false);
    QWidget *findCurrentFocusWidget(bool includefloating = false);
    QMenu *createPopupMenu(void) override;
    void setSessionClassActionEnable(bool enable);
    void setGlobalOptions(SessionsWindow *window);
#if defined(Q_OS_WIN)
    QString getDirAndcheckeSysName(const QString &title,SessionsWindow::ShellType shellType = SessionsWindow::PowerShell, QString overrideSysUsername = QString(), QString overrideSysHostname = QString());
#else
    QString getDirAndcheckeSysName(const QString &title,SessionsWindow::ShellType shellType = SessionsWindow::UnixShell, QString overrideSysUsername = QString(), QString overrideSysHostname = QString());
#endif
    void initSysEnv(void);
    void addBookmark(const QString &path);
    bool checkSessionName(QString &name);
    int addSessionToSessionManager(SessionsWindow *sessionsWindow, QString &name);
    int addSessionToSessionManager(const QuickConnectWindow::QuickConnectData &data, QString &name, bool checkname = true, int64_t id = -1);
    int64_t removeSessionFromSessionManager(const QString &name);
    void connectSessionFromSessionManager(QString name);
    void restoreSessionToSessionManager(void);
    void saveSettings(void);
    void restoreSettings(void);
    void connectSessionStateChange(SessionTab *tab, int index, SessionsWindow *sessionsWindow);
    void sessionWindow2InfoData(SessionsWindow *sessionsWindow, QuickConnectWindow::QuickConnectData &data, QString &name);
    int setting2InfoData(GlobalSetting *settings, QuickConnectWindow::QuickConnectData &data, QString &name, QString &group, bool skipPassword = false);
    void infoData2Setting(GlobalSetting *settings,const QuickConnectWindow::QuickConnectData &data,const QString &name,const QString &group, bool skipPassword = false);
    void moveToAnotherTab(int src,int dst, int index);
    void floatingWindow(MainWidgetGroup *g, int index);
    void terminalWidgetContextMenuBase(QMenu *menu,SessionsWindow *term,const QPoint& position);
    void swapSideHboxLayout(void);
    int nextGroupID(int id);
    void movetabWhenLayoutChange(int oldL, int newL);
    void refreshTagColor(void);
    void refreshStatusBar(void);

private slots:
    void onPluginRequestTelnetConnect(QString host, int port, int type);
    void onPluginRequestSerialConnect(QString portName, uint32_t baudRate, int dataBits, int parity, int stopBits, bool flowControl, bool xEnable);
    void onPluginRequestLocalShellConnect(QString command, QString workingDirectory);
    void onPluginRequestRawSocketConnect(QString host, int port);
    void onPluginRequestNamePipeConnect(QString namePipe);
    void onPluginRequestSSH2Connect(QString host, QString user, QString password, int port);
    void onPluginRequestVNCConnect(QString host, QString password, int port);
    void onPluginSendCommand(QString cmd);
    void onPluginWriteSettings(QString group, QString key, QVariant value);
    void onPluginReadSettings(QString group, QString key, QVariant &value);

protected:
    void closeEvent(QCloseEvent *event) override {
        checkCloseEvent(event);
    }

private:
    Ui::CentralWidget *ui;

    QSplitter *splitter;
    QSplitter *splitterV1;
    QSplitter *splitterV2;
    QSplitter *splitterV11;
    QStackedWidget *stackedWidget;
    SessionManagerWidget *sessionManagerWidget;
    NotifictionWidget *notifictionWidget;
    PluginViewerWidget *pluginViewerWidget;
    QList<MainWidgetGroup *> mainWidgetGroupList;
    QuickConnectWindow *quickConnectWindow;
    MainWidgetGroup *quickConnectMainWidgetGroup;
    keyMapManager *keyMapManagerWindow;
    InternalCommandWindow *internalCommandWindow;
    GlobalOptionsWindow *globalOptionsWindow;
    SessionOptionsWindow *sessionOptionsWindow;
    HexViewWindow *hexViewWindow;
    QWidget *sideProxyWidget;
    QPushButton *sessionManagerPushButton;
    QPushButton *pluginViewerPushButton;
    QHBoxLayout *sideHboxLayout;
    StartTftpSeverWindow *startTftpSeverWindow;
    LockSessionWindow *lockSessionWindow;
#ifdef ENABLE_SSH
    SftpWindow *sftpWindow;
#endif
    NetScanWindow *netScanWindow;
    PluginInfoWindow *pluginInfoWindow;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *optionsMenu;
    QMenu *transferMenu;
    QMenu *scriptMenu;
    QMenu *bookmarkMenu;
    QMenu *toolsMenu;
    QMenu *windowMenu;
    QMenu *languageMenu;
    QMenu *themeMenu;
    QMenu *helpMenu;
    QAction *newWindowAction;
    QAction *connectAction;
    QAction *sessionManagerAction;
    QAction *quickConnectAction;
    QAction *connectInTabAction;
    QAction *connectLocalShellAction;
#if defined(Q_OS_WIN)
    QAction *connectWslAction;
#endif
    QAction *reconnectAction;
    QAction *reconnectAllAction;
    QAction *disconnectAction;
    QLineEdit *connectAddressEdit;
    QAction *connectAddressEditAction;
    QAction *disconnectAllAction;
    QAction *cloneSessionAction;
    QAction *logSessionAction;
    QAction *rawLogSessionAction;
    QAction *exitAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *copyAndPasteAction;
    QAction *selectAllAction;
    QAction *findAction;
    QAction *printScreenAction;
    QAction *screenShotAction;
    QAction *screenRecordingAction;
    QAction *sessionExportAction;
    QAction *clearScrollbackAction;
    QAction *clearScreenAction;
    QAction *clearScreenAndScrollbackAction;
    QAction *resetAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomResetAction;
    QMenu *layoutMenu;
    QActionGroup *layoutActionGroup;
    QAction *singleLayoutAction;
    QAction *twoColumnsLayoutAction;
    QAction *threeColumnsLayoutAction;
    QAction *twoRowsLayoutAction;
    QAction *threeRowsLayoutAction;
    QAction *gridLayoutAction;
    QAction *twoRowsRightLayoutAction;
    QAction *twoColumnsBottomLayoutAction;
    QAction *flipLayoutAction;
    QAction *menuBarAction;
    QAction *toolBarAction;
    QAction *statusBarAction;
    QAction *cmdWindowAction;
    QAction *connectBarAction;
    QAction *sideWindowAction;
    QAction *hexViewAction;
    QAction *windwosTransparencyAction;
    QAction *verticalScrollBarAction;
    QAction *allwaysOnTopAction;
    QAction *fullScreenAction;
    QAction *sessionOptionsAction;
    QAction *globalOptionsAction;
    QAction *realTimeSaveOptionsAction;
    QAction *saveSettingsNowAction;
    QAction *sendASCIIAction;
    QAction *receiveASCIIAction;
    QAction *sendBinaryAction;
    QAction *sendKermitAction;
    QAction *receiveKermitAction;
    QAction *sendXmodemAction;
    QAction *receiveXmodemAction;
    QAction *sendYmodemAction;
    QAction *receiveYmodemAction;
    QAction *zmodemUploadListAction;
    QAction *startZmodemUploadAction;
    QAction *startTFTPServerAction;
    QAction *runAction;
    QAction *cancelAction;
    QAction *startRecordingScriptAction;
    QAction *stopRecordingScriptAction;
    QAction *canlcelRecordingScriptAction;
    QAction *cleanAllRecentScriptAction;
    QAction *addBookmarkAction;
    QAction *removeBookmarkAction;
    QAction *cleanAllBookmarkAction;
    QAction *keymapManagerAction;
    QAction *createPublicKeyAction;
    QAction *publickeyManagerAction;
    QAction *internalCommandAction;
    QAction *sshScanningAction;
    QAction *pluginInfoAction;
    QActionGroup *windowActionGroup;
    QAction *tabAction;
    QAction *tileAction;
    QAction *cascadeAction;
    QActionGroup *languageActionGroup;
    QAction *chineseAction;
    QAction *chineseHKAction;
    QAction *russianAction;
    QAction *portugueseAction;
    QAction *koreanAction;
    QAction *japaneseAction;
    QAction *frenchAction;
    QAction *spanishAction;
    QAction *englishAction;
    QAction *germanAction;
    QAction *czechAction;
    QAction *arabicAction;
    QActionGroup *themeActionGroup;
    QAction *lightThemeAction;
    QAction *darkThemeAction;
    QAction *themeColorAction;
    QAction *helpAction;
    QAction *keyboradShortcutsReferenceAction;
    QAction *checkUpdateAction;
    QAction *privacyStatementAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
#ifdef ENABLE_PYTHON
    QAction *aboutPythonAction;
#endif

    QToolButton *laboratoryButton;
    QMenu *laboratoryMenu;

    QShortcut *shortcutMenuBarView;
    QShortcut *shortcutConnectLocalShell;
    QShortcut *shortcutCloneSession;
    QShortcut *shortcutMiniModeSwitch;
    QShortcut *shortcutStdModeSwitch;
    QShortcut *shortcutTabPlusSwitch;
    QShortcut *shortcutTabMinusSwitch;
    QShortcut *shortcutTabSwitch[9];
    QShortcut *shortcutConnectAddressEdit;
    QShortcut *shortcutExitFullScreen;

    QMediaCaptureSession *m_mediaCaptureSession;
    QWindowCapture *m_windowCapture;
    QMediaRecorder *m_mediaRecorder;

    QLabel *statusBarMessage;
    StatusBarWidget *statusBarWidget;
    QTimer *statusBarWidgetRefreshTimer;

    QString sysUsername;
    QString sysHostname;

    QStringList zmodemUploadList;

    KeyChainClass keyChainClass;
    QList<SessionsWindow *> sessionList;
    QList<SessionsWindow *> broadCastSessionList;
    SessionsWindow *willLockUnLockSessions = nullptr;
    struct pluginState_t {
        PluginInterface *iface;
        bool state;
    };
    QList<pluginState_t> pluginList;
    QTftp *tftpServer;
    qreal windowTransparency;
    bool windowTransparencyEnabled;

#ifdef ENABLE_PYTHON
    //script engine need
    PyRun *pyRun;
    QString runScriptFullName;
#endif
    QString printerName;
    int currentLayout;
    bool enabledSyncSplitterMoved = false;

    QColor lastHighlightColor = Qt::white;

    QLocale language;
    bool isDarkTheme;
    QColor themeColor;
    bool themeColorEnable = false;
    class MainWindow *mainWindow = nullptr;
};

class MainWindow : public QGoodWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QString dir = QString(), 
                        CentralWidget::StartupUIMode mode = CentralWidget::STDUI_MODE, 
                        QLocale lang = QLocale(QLocale::English), 
                        bool isDark = true, 
                        QString start_know_session = QString(), 
                        bool disable_plugin = false,
                        QWidget *parent = nullptr);
    ~MainWindow();
    void setLaboratoryButton(QToolButton *laboratoryButton) {
        QTimer::singleShot(0, this, [this, laboratoryButton](){
            laboratoryButton->setFixedSize(m_good_central_widget->titleBarHeight(),m_good_central_widget->titleBarHeight());
            m_good_central_widget->setRightTitleBarWidget(laboratoryButton, false);
            connect(m_good_central_widget,&QGoodCentralWidget::windowActiveChanged,this, [laboratoryButton](bool active){
                laboratoryButton->setEnabled(active);
            });
        });
    }
    void fixMenuBarWidth(void) {
        if (m_menu_bar) {
            /* FIXME: Fix the width of the menu bar 
             * please optimize this code */
            int width = 0;
            int itemSpacingPx = m_menu_bar->style()->pixelMetric(QStyle::PM_MenuBarItemSpacing);
            for (int i = 0; i < m_menu_bar->actions().size(); i++) {
                QString text = m_menu_bar->actions().at(i)->text();
                QFontMetrics fm(m_menu_bar->font());
                width += fm.size(0, text).width() + itemSpacingPx*1.5;
            }
            m_good_central_widget->setLeftTitleBarWidth(width);
        }
    }

protected:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent * event) override;

private:
    QGoodCentralWidget *m_good_central_widget;
    QMenuBar *m_menu_bar = nullptr;
    CentralWidget *m_central_widget;
};

class QCapturableWindowPrivate : public QSharedData {
public:
    using Id = size_t;

    QString description;
    Id id = 0;

    static const QCapturableWindowPrivate *handle(const QCapturableWindow &window)
    {
        return window.d.get();
    }

    QCapturableWindow create() { return QCapturableWindow(this); }
};

#endif // MAINWINDOW_H
