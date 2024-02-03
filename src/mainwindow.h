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

#include "mainwidgetgroup.h"
#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptionswindow.h"
#include "hexviewwindow.h"
#include "sessionmanagerwidget.h"
#include "pluginviewerwidget.h"
#include "commandwidget.h"
#include "starttftpseverwindow.h"
#include "locksessionwindow.h"
#include "sessionoptionswindow.h"
#include "globalsetting.h"
#include "qtftp.h"
#include "sftpwindow.h"
#include "netscanwindow.h"
#include "plugininfowindow.h"
#include "keychainclass.h"
#include "plugininterface.h"
#include "QGoodWindow"
#include "QGoodCentralWidget"

extern QString VERSION;
extern QString GIT_TAG;
extern QString DATE_TAG;
extern QString HASH_TAG;
extern QString SHORT_HASH_TAG;

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
    CentralWidget(QString dir = QString(), StartupUIMode mode = STDUI_MODE, QLocale lang = QLocale(QLocale::English), 
        bool isDark = true, QString start_know_session = QString(), QWidget *parent = nullptr);
    ~CentralWidget();
    static void appAbout(QWidget *parent = nullptr);
    static void appHelp(QWidget *parent = nullptr);
    static void setAppLangeuage(QLocale lang);
    void checkCloseEvent(QCloseEvent *event);
    void checkStatusTipEvent(QStatusTipEvent *event);

private:
    void menuAndToolBarInit(void);
    void menuAndToolBarRetranslateUi(void);
    void menuAndToolBarConnectSignals(void);
    QString startTelnetSession(MainWidgetGroup *group, QString hostname, quint16 port, QTelnet::SocketType type, QString name = QString());
    QString startSerialSession(MainWidgetGroup *group, QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable, QString name = QString());
    QString startLocalShellSession(MainWidgetGroup *group, const QString &command = QString(), const QString &workingDirectory = QDir::homePath(), QString name = QString());
    QString startRawSocketSession(MainWidgetGroup *group, QString hostname, quint16 port, QString name = QString());
    QString startNamePipeSession(MainWidgetGroup *group, QString namePipe, QString name = QString());
    QString startSSH2Session(MainWidgetGroup *group,
        QString hostname, quint16 port, QString username, QString password, QString name = QString());
    QString startVNCSession(MainWidgetGroup *group, QString hostname, quint16 port, QString password, QString name = QString());
    int stopSession(MainWidgetGroup *group, int index, bool force = false);
    int stopAllSession(bool force = false);
    int cloneCurrentSession(MainWidgetGroup *group, QString name = QString());
    MainWidgetGroup *findCurrentFocusGroup(void);
    QWidget *findCurrentFocusWidget(void);
    QMenu *createPopupMenu(void) override;
    void setSessionClassActionEnable(bool enable);
    void setGlobalOptions(SessionsWindow *window);
    QString getDirAndcheckeSysName(const QString &title);
    void addBookmark(const QString &path);
    bool checkSessionName(QString &name);
    int addSessionToSessionManager(SessionsWindow *sessionsWindow, QString &name);
    int addSessionToSessionManager(const QuickConnectWindow::QuickConnectData &data, QString &name, bool checkname = true, int64_t id = -1);
    int64_t removeSessionFromSessionManager(QString name);
    void connectSessionFromSessionManager(QString name);
    void restoreSessionToSessionManager(void);
    void saveSettings(void);
    void restoreSettings(void);
    void connectSessionStateChange(SessionTab *tab, int index, SessionsWindow *sessionsWindow);
    void sessionWindow2InfoData(SessionsWindow *sessionsWindow, QuickConnectWindow::QuickConnectData &data, QString &name);
    int setting2InfoData(GlobalSetting *settings, QuickConnectWindow::QuickConnectData &data, QString &name, bool skipPassword = false);
    void infoData2Setting(GlobalSetting *settings,const QuickConnectWindow::QuickConnectData &data,const QString &name, bool skipPassword = false);
    void moveToAnotherTab(int src,int dst, int index);
    void floatingWindow(MainWidgetGroup *g, int index);
    void terminalWidgetContextMenuBase(QMenu *menu,SessionsWindow *term,const QPoint& position);
    void swapSideHboxLayout(void);

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

    SessionManagerWidget *sessionManagerWidget;
    PluginViewerWidget *pluginViewerWidget;
    QList<MainWidgetGroup *> mainWidgetGroupList;
    QuickConnectWindow *quickConnectWindow;
    MainWidgetGroup *quickConnectMainWidgetGroup;
    keyMapManager *keyMapManagerWindow;
    GlobalOptionsWindow *globalOptionsWindow;
    SessionOptionsWindow *sessionOptionsWindow;
    HexViewWindow *hexViewWindow;
    QWidget *sideProxyWidget;
    QPushButton *sessionManagerPushButton;
    QPushButton *pluginViewerPushButton;
    QHBoxLayout *sideHboxLayout;
    StartTftpSeverWindow *startTftpSeverWindow;
    LockSessionWindow *lockSessionWindow;
    SftpWindow *sftpWindow;
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
    QAction *reconnectAction;
    QAction *reconnectAllAction;
    QAction *disconnectAction;
    QLineEdit *connectAddressEdit;
    QAction *connectAddressEditAction;
    QAction *disconnectAllAction;
    QAction *cloneSessionAction;
    QAction *lockSessionAction;
    QAction *logSessionAction;
    QAction *rawLogSessionAction;
    QAction *hexViewAction;
    QAction *exitAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *copyAndPasteAction;
    QAction *selectAllAction;
    QAction *findAction;
    QAction *printScreenAction;
    QAction *screenShotAction;
    QAction *sessionExportAction;
    QAction *clearScrollbackAction;
    QAction *clearScreenAction;
    QAction *clearScreenAndScrollbackAction;
    QAction *resetAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomResetAction;
    QAction *menuBarAction;
    QAction *toolBarAction;
    QAction *statusBarAction;
    QAction *cmdWindowAction;
    QAction *connectBarAction;
    QAction *sideWindowAction;
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
    QAction *addBookmarkAction;
    QAction *removeBookmarkAction;
    QAction *cleanAllBookmarkAction;
    QAction *keymapManagerAction;
    QAction *createPublicKeyAction;
    QAction *publickeyManagerAction;
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
    QAction *helpAction;
    QAction *checkUpdateAction;
    QAction *aboutAction;
    QAction *aboutQtAction;

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

    KeyChainClass keyChainClass;
    QList<SessionsWindow *> sessionList;
    struct pluginState_t {
        PluginInterface *iface;
        bool state;
    };
    QList<pluginState_t> pluginList;
    QTftp *tftpServer;
    qreal windowTransparency;
    bool windowTransparencyEnabled;

    QLocale language;
    bool isDarkTheme;
    class MainWindow *mainWindow = nullptr;
};

class MainWindow : public QGoodWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QString dir = QString(), CentralWidget::StartupUIMode mode = CentralWidget::STDUI_MODE, 
        QLocale lang = QLocale(QLocale::English), bool isDark = true, QString start_know_session = QString(), QWidget *parent = nullptr);
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

protected:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent * event) override;

private:
    QGoodCentralWidget *m_good_central_widget;
    QMenuBar *m_menu_bar = nullptr;
    CentralWidget *m_central_widget;
};

#endif // MAINWINDOW_H
