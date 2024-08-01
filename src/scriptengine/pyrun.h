/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
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
    void runScriptFile(const QString &scriptFile, QString *result = nullptr, int *ret = nullptr);
    void runScriptStr(const QString &scriptStr, QString *result = nullptr, int *ret = nullptr);
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
    int getActiveSessionId(void);
    void messageNotifications(const QString &messageStr);
    void messageBox(const QString &messageStr, const QString &titleStr, int buttons);
    QString prompt(const QString &promptStr, const QString &nameStr, const QString &inputStr, bool password);
    QString fileOpenDialog(const QString &titleStr, const QString &buttonLabelStr, const QString &directoryStr, const QString &filterStr);
    QString fileSaveDialog(const QString &titleStr, const QString &buttonLabelStr, const QString &directoryStr, const QString &filterStr);
    void windowActivate(void);
    void windowShow(int type);
    int getWindowShowType(void);
    bool getWindowActive(void);
    QStringList getArguments(void);
    QString getCommandWindowText(void);
    void setCommandWindowText(const QString &text);
    void commandWindowSend(void);
    QString getDownloadFolder(void);
    bool getCommandWindowVisibled(void);
    void setCommandWindowVisibled(bool enable);
    void addToUploadList(const QString &file);
    void clearUploadList(void);
    int receiveKermit(void);
    int sendKermit(const QString &file);
    int receiveXmodem(const QString &file);
    int sendXmodem(const QString &file);
    int receiveYmodem(void);
    int sendYmodem(const QString &file);
    int sendZmodem(void);

    int screenSend(const QString &str, bool synchronous, int id = -1);
    QString screenWaitForString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex, int id = -1);
    QString screenReadString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex, int id = -1);
    int screenGetCurrentRow(int id = -1);
    int screenGetCurrentColumn(int id = -1);
    int screenGetRows(int id = -1);
    int screenGetColumns(int id = -1);
    QString screenGetSelection(int id = -1);
    void screenClear(int id = -1);
    QString screenGet(int row1, int col1, int row2, int col2, int id = -1);
    QString screenGet2(int row1, int col1, int row2, int col2, int id = -1);
    void screenPrint(int id = -1);
    void screenShortcut(const QString &path, int id = -1);
    void screenSendKeys(const QStringList &keys, int id = -1);

    bool sessionGetLocked(int id = -1);
    bool sessionGetConnected(int id = -1);
    bool sessionGetLogging(int id = -1);
    int sessionLock(const QString &prompt, const QString &password, int lockallsessions, int id = -1);
    int sessionUnlock(const QString &prompt, const QString &password, int lockallsessions, int id = -1);
    int sessionConnect(const QString &cmd, int id = -1);
    void sessionDisconnect(int id = -1);
    void sessionLog(int enable, int id = -1);

    int tabGetnumber(int id);
    int tabCheckScreenId(int tabId, int screenId);
    void tabActivate(int tabId, int screenId);

    static void setOverridePythonLib(const QString &path);

signals:
    void runScriptStarted(void);
    void runScriptFinished(void);
    void requestCmd(const QStringList &cmdList, QStringList *resultList);
    void waitForStringFinished(const QString &str);

private:
    struct ScriptInfo {
        int type;
        QString script;
        QString *result;
        int *ret;
        volatile bool *finished;
    };
    void runScript(struct ScriptInfo *scriptInfo);
    QString runScriptFileInternal(const QString &scriptFile, int *sret);
    QString runScriptStrInternal(const QString &scriptStr, int *sret);

protected:
    void run();

private:
    QMutex mutex;
    QQueue<struct ScriptInfo> runScriptList;
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
    static QString overridePythonLibPath;

    QString specialCmdResult;
    const QStringList specialSyncCmd = {
        "AskQuardShow",
        "QuardSOnly",
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
        {"Del", Qt::Key_Delete},
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
        {"Ctrl", Qt::Key_Control},
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
