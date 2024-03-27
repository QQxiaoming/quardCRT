#ifndef PYRUN_H
#define PYRUN_H

#include <QThread>
#include <QString>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>

class PyRun : public QThread
{
    Q_OBJECT
public:
    explicit PyRun(QObject *parent = nullptr);
    ~PyRun();

    QString getPyVersion(void) { return pyVersion; }
    void runScript(QString scriptFile);
    bool isRunning(void) { return m_running; }
    void cancelScript(void);
    bool isStopScript(void);
    void emitWaitForStringFinished(const QString &str, int matchIndex) {
        QMutexLocker locker(&waitForStringMutex);
        waitForStringStrResult = str;
        waitForStringMatchIndex = matchIndex;
        waitForStringCondition.wakeOne();
        Q_UNUSED(str);
    }

    QString getActivePrinter(void);
    void setActivePrinter(const QString &name);
    QString getScriptFullName(void);
    QString getVersion(void);
    int runSpecialCmd(const QString &cmd, const QStringList &arg);
    QString getSpecialCmdResult(void);
    int getActiveTabId(void);
    void messageBox(const QString &messageStr, const QString &titleStr, int buttons);
    QString prompt(const QString &promptStr, const QString &nameStr, const QString &inputStr, bool password);
    QString fileOpenDialog(const QString &titleStr, const QString &buttonLabelStr, const QString &directoryStr, const QString &filterStr);
    QString fileSaveDialog(const QString &titleStr, const QString &buttonLabelStr, const QString &directoryStr, const QString &filterStr);
    int screenSend(const QString &str, bool synchronous);
    QString screenWaitForString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex);
    QString screenReadString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex);
    int sessionConnect(const QString &cmd);
    void sessionDisconnect(void);
    void sessionLog(int enable);
    void windowActivate(void);
    void windowShow(int type);
    int getWindowShowType(void);
    bool getWindowActive(void);
    QStringList getArguments(void);
    QString getCommandWindowText(void);
    void setCommandWindowText(const QString &text);
    void commandWindowSend(void);
    QString getDownloadFolder(void);
    void addToUploadList(const QString &file);
    int receiveKermit(void);
    int sendKermit(const QString &file);
    int receiveXmodem(const QString &file);
    int sendXmodem(const QString &file);
    int receiveYmodem(void);
    int sendYmodem(const QString &file);
    int sendZmodem(void);
    int screenGetCurrentRow(void);
    int screenGetCurrentColumn(void);
    int screenGetRows(void);
    int screenGetColumns(void);
    QString screenGetSelection(void);
    void screenClear(void);
    QString screenGet(int row1, int col1, int row2, int col2);
    QString screenGet2(int row1, int col1, int row2, int col2);
    void screenPrint(void);
    void screenShortcut(const QString &path);
    bool sessionGetLocked(void);
    bool sessionGetConnected(void);
    bool sessionGetLogging(void);
    void screenSendKeys(const QStringList &keys);
    int sessionLock(const QString &prompt, const QString &password, int lockallsessions);
    int sessionUnlock(const QString &prompt, const QString &password, int lockallsessions);

signals:
    void runScriptStarted(void);
    void runScriptFinished(void);
    void requestCmd(const QStringList &cmdList, QStringList *resultList);
    void waitForStringFinished(const QString &str);

private:
    void runScriptInternal(QString scriptFile);

protected:
    void run();

private:
    QMutex mutex;
    QQueue<QString> runScriptList;
    QWaitCondition condition;

    QMutex waitForStringMutex;
    QWaitCondition waitForStringCondition;
    QString waitForStringStrResult;
    int waitForStringMatchIndex;

    bool m_exit = false;
    bool m_running = false;
    QMutex mutexUserStop;
    bool m_userStop = false;

    QString pyVersion;

    QString specialCmdResult;
    const QStringList specialSyncCmd = {
        "AskQuardShow",
        "LoveTangtang",
    };
    const QStringList specialASyncCmd = {
        "AskQuard",
    };
    const QMap<QString, Qt::Key> keyMap = {
        {"F1", Qt::Key_F1},
        {"F2", Qt::Key_F2},
        {"F3", Qt::Key_F3},
        {"F4", Qt::Key_F4},
        {"F5", Qt::Key_F5},
        {"F6", Qt::Key_F6},
        {"F7", Qt::Key_F7},
        {"F8", Qt::Key_F8},
        {"F9", Qt::Key_F9},
        {"F10", Qt::Key_F10},
        {"F11", Qt::Key_F11},
        {"F12", Qt::Key_F12},
        {"Space", Qt::Key_Space},
        {"Return", Qt::Key_Return},
        {"Enter", Qt::Key_Enter},
        {"Tab", Qt::Key_Tab},
        {"Backtab", Qt::Key_Backtab},
        {"Backspace", Qt::Key_Backspace},
        {"Delete", Qt::Key_Delete},
        {"Insert", Qt::Key_Insert},
        {"Escape", Qt::Key_Escape},
        {"Up", Qt::Key_Up},
        {"Down", Qt::Key_Down},
        {"Left", Qt::Key_Left},
        {"Right", Qt::Key_Right},
        {"Home", Qt::Key_Home},
        {"End", Qt::Key_End},
        {"PageUp", Qt::Key_PageUp},
        {"PageDown", Qt::Key_PageDown},
        {"Shift", Qt::Key_Shift},
        {"Control", Qt::Key_Control},
        {"Alt", Qt::Key_Alt},
        {"Meta", Qt::Key_Meta},
        {"0", Qt::Key_0},
        {"1", Qt::Key_1},
        {"2", Qt::Key_2},
        {"3", Qt::Key_3},
        {"4", Qt::Key_4},
        {"5", Qt::Key_5},
        {"6", Qt::Key_6},
        {"7", Qt::Key_7},
        {"8", Qt::Key_8},
        {"9", Qt::Key_9},
        {"A", Qt::Key_A},
        {"B", Qt::Key_B},
        {"C", Qt::Key_C},
        {"D", Qt::Key_D},
        {"E", Qt::Key_E},
        {"F", Qt::Key_F},
        {"G", Qt::Key_G},
        {"H", Qt::Key_H},
        {"I", Qt::Key_I},
        {"J", Qt::Key_J},
        {"K", Qt::Key_K},
        {"L", Qt::Key_L},
        {"M", Qt::Key_M},
        {"N", Qt::Key_N},
        {"O", Qt::Key_O},
        {"P", Qt::Key_P},
        {"Q", Qt::Key_Q},
        {"R", Qt::Key_R},
        {"S", Qt::Key_S},
        {"T", Qt::Key_T},
        {"U", Qt::Key_U},
        {"V", Qt::Key_V},
        {"W", Qt::Key_W},
        {"X", Qt::Key_X},
        {"Y", Qt::Key_Y},
        {"Z", Qt::Key_Z},
    };
};

#endif // PYRUN_H
