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

#include "mainwidgetgroup.h"
#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptions.h"
#include "hexviewwindow.h"
#include "sessionmanagerwidget.h"
#include "commandwindow.h"
#include "starttftpseverwindow.h"
#include "locksessionwindow.h"
#include "qtftp.h"

extern QString VERSION;
extern QString GIT_TAG;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum StartupUIMode {
        STDUI_MODE = 0,
        MINIUI_MODE,
    };
    MainWindow(QString dir = QString(), StartupUIMode mode = STDUI_MODE, QLocale::Language lang = QLocale::English, bool isDark = true, QWidget *parent = nullptr);
    ~MainWindow();
    static void appAbout(QWidget *parent = nullptr);
    static void appHelp(QWidget *parent = nullptr);
    static void setAppLangeuage(QLocale::Language lang);

private:
    void menuAndToolBarInit(void);
    void menuAndToolBarRetranslateUi(void);
    void menuAndToolBarConnectSignals(void);
    QString startTelnetSession(MainWidgetGroup *group, QString hostname, quint16 port, QTelnet::SocketType type, QString name = QString());
    QString startSerialSession(MainWidgetGroup *group, QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable, QString name = QString());
    QString startLocalShellSession(MainWidgetGroup *group, const QString &command = QString(), const QString &workingDirectory = QDir::homePath(), QString name = QString());
    QString startRawSocketSession(MainWidgetGroup *group, QString hostname, quint16 port, QString name = QString());
    QString startSSH2Session(MainWidgetGroup *group,
        QString hostname, quint16 port, QString username, QString password, QString name = QString());
    int stopSession(MainWidgetGroup *group, int index);
    int stopAllSession(void);
    int cloneCurrentSession(MainWidgetGroup *group, QString name = QString());
    MainWidgetGroup *findCurrentFocusGroup(void);
    QTermWidget *findCurrentFocusTermWidget(void);
    QMenu *createPopupMenu(void) override;
    void setSessionClassActionEnable(bool enable);
    void setGlobalOptions(SessionsWindow *window);
    QString getDirAndcheckeSysName(const QString &title);
    void addBookmark(const QString &path);
    bool checkSessionName(QString &name);
    int addSessionToSessionManager(SessionsWindow *sessionsWindow, QString &name);
    int addSessionToSessionManager(const QuickConnectWindow::QuickConnectData &data, QString &name, bool checkname = true);
    void removeSessionFromSessionManager(QString name);
    void connectSessionFromSessionManager(QString name);
    void restoreSessionToSessionManager(void);
    void saveSettings(void);
    void restoreSettings(void);

private:
    Ui::MainWindow *ui;

    SessionManagerWidget *sessionManagerWidget;
    QList<MainWidgetGroup *> mainWidgetGroupList;
    QuickConnectWindow *quickConnectWindow;
    MainWidgetGroup *quickConnectMainWidgetGroup;
    keyMapManager *keyMapManagerWindow;
    GlobalOptions *globalOptionsWindow;
    HexViewWindow *hexViewWindow;
    QPushButton *sessionManagerPushButton;
    StartTftpSeverWindow *startTftpSeverWindow;
    LockSessionWindow *lockSessionWindow;

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
    QActionGroup *windowActionGroup;
    QAction *tabAction;
    QAction *tileAction;
    QAction *cascadeAction;
    QActionGroup *languageActionGroup;
    QAction *chineseAction;
    QAction *englishAction;
    QAction *japaneseAction;
    QActionGroup *themeActionGroup;
    QAction *lightThemeAction;
    QAction *darkThemeAction;
    QAction *helpAction;
    QAction *aboutAction;
    QAction *aboutQtAction;

    QShortcut *shortcutMenuBarView;
    QShortcut *shortcutConnectLocalShell;
    QShortcut *shortcutCloneSession;
    QShortcut *shortcutMiniModeSwitch;
    QShortcut *shortcutStdModeSwitch;
    QShortcut *shortcutTabPlusSwitch;
    QShortcut *shortcutTabMinusSwitch;
    QShortcut *shortcutTabSwitch[9];

    QList<SessionsWindow *> sessionList;
    QTftp *tftpServer;
    qreal windowTransparency;
    bool windowTransparencyEnabled;

    QLocale::Language language;
    bool isDarkTheme;
};
#endif // MAINWINDOW_H
