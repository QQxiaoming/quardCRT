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

#include "sessiontab.h"
#include "quickconnectwindow.h"

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

private:
    Ui::MainWindow *ui;
    QSplitter *splitter;
    SessionTab *sessionTab;
    QuickConnectWindow *quickConnectWindow;

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
    QAction *exitAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *selectAllAction;
    QAction *findAction;
    QAction *printScreenAction;
    QAction *resetAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
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
};
#endif // MAINWINDOW_H
