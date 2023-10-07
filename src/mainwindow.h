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

#include "sessiontab.h"
#include "sessionswindow.h"
#include "quickconnectwindow.h"
#include "keymapmanager.h"
#include "globaloptions.h"
#include "hexviewwindow.h"
#include "sessionmanagerwidget.h"
#include "commandwindow.h"

extern QString VERSION;
extern QString GIT_TAG;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QLocale::Language lang = QLocale::English, bool isDark = true, QWidget *parent = nullptr);
    ~MainWindow();
    static void appAbout(QWidget *parent = nullptr);
    static void appHelp(QWidget *parent = nullptr);
    static void setAppLangeuage(QLocale::Language lang);

private:
    void menuAndToolBarInit(void);
    void menuAndToolBarRetranslateUi(void);
    void menuAndToolBarConnectSignals(void);
    SessionsWindow *startTelnetSession(QString hostname, quint16 port, QTelnet::SocketType type);
    SessionsWindow *startSerialSession(QString portName, uint32_t baudRate,
                int dataBits, int parity, int stopBits, bool flowControl, bool xEnable);
    SessionsWindow *startLocalShellSession(const QString &command = QString());
    SessionsWindow *startRawSocketSession(QString hostname, quint16 port);
    int stopSession(int index);
    int stopAllSession(void);
    int cloneCurrentSession(void);
    QMenu *createPopupMenu(void) override;

private:
    Ui::MainWindow *ui;

    SessionManagerWidget *sessionManagerWidget;
    SessionTab *sessionTab;
    QuickConnectWindow *quickConnectWindow;
    keyMapManager *keyMapManagerWindow;
    GlobalOptions *globalOptionsWindow;
    HexViewWindow *hexViewWindow;
    CommandWindow *cmdWindow;
    QPushButton *sessionManagerPushButton;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *optionsMenu;
    QMenu *transferMenu;
    QMenu *scriptMenu;
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
    QAction *resetAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomResetAction;
    QAction *menuBarAction;
    QAction *toolBarAction;
    QAction *statusBarAction;
    QAction *cmdWindowAction;
    QAction *sideWindowAction;
    QAction *fullScreenAction;
    QAction *sessionOptionsAction;
    QAction *globalOptionsAction;
    QAction *autoSaveOptionsAction;
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

    QLocale::Language language;
    bool isDarkTheme;
    QList<SessionsWindow *> sessionActionsList;
};
#endif // MAINWINDOW_H
