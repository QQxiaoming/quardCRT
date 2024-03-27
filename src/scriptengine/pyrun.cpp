#include "pycore.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

#include "mainwindow.h"
#include "filedialog.h"
#include "pyrun.h"

PyRun::PyRun(QObject *parent)
    : QThread{parent} {
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    QString pythonHome = QApplication::applicationDirPath() + "/pythonlib";
#elif defined(Q_OS_MACOS)
    QString pythonHome = QApplication::applicationDirPath() + "/../Frameworks/pythonlib";
#endif
    if (QDir(pythonHome).exists()) {
        std::wstring  pythonHomeWStr = pythonHome.toStdWString();
        const wchar_t* pythonHomeStr = pythonHomeWStr.c_str();
        Py_SetPythonHome(pythonHomeStr);
    } else {
        qDebug() << "Python home not overriden, Maybe in debug mode, using system python";
    }
    Py_Initialize();

    // Register the trace function
    PyEval_SetProfile((Py_tracefunc)PyCore::stop_script, NULL);

    PyObject* quardCRTModule = PyCore::PyInit_quardCRT(this);
    if (quardCRTModule == nullptr) {
        qDebug() << "Failed to import quadCRT module";
    }
    PyObject* sysModules = PyImport_GetModuleDict();
    PyDict_SetItemString(sysModules, "quardCRT", quardCRTModule);

    const char * version = Py_GetVersion();
    pyVersion = QString::fromUtf8(version);
    const char * platform = Py_GetPlatform();
    pyVersion.append("\n\nPlatform: " + QString::fromUtf8(platform));
    const char * copyright = Py_GetCopyright();
    pyVersion.append("\n\n" + QString::fromUtf8(copyright));

    connect(this, &PyRun::requestCmd, this, [&](const QStringList &cmdList, QStringList *resultList) {
        CentralWidget *centralWidget = static_cast<CentralWidget*>(this->parent());
        QString cmd = cmdList.at(0);
        if (cmd == "messageBox") {
            QString messageStr = cmdList.at(1);
            QString titleStr = cmdList.at(2);
            QMessageBox::information(centralWidget, titleStr, messageStr);
        } else if (cmd == "prompt") {
            QString promptStr = cmdList.at(1);
            QString nameStr = cmdList.at(2);
            QString inputStr = cmdList.at(3);
            bool password = cmdList.at(4) == "True";
            if (password) {
                QString resultStr = QInputDialog::getText(centralWidget, promptStr, nameStr, QLineEdit::Password, inputStr);
                if(resultList != nullptr)
                    resultList->append(resultStr);
            } else {
                QString resultStr = QInputDialog::getText(centralWidget, promptStr, nameStr, QLineEdit::Normal, inputStr);
                if(resultList != nullptr)
                    resultList->append(resultStr);
            }
        } else if(cmd == "fileOpenDialog") {
            QString titleStr = cmdList.at(1);
            //QString buttonLabelStr = cmdList.at(2);
            QString directoryStr = cmdList.at(3);
            QString filterStr = cmdList.at(4);
            QString resultStr = FileDialog::getOpenFileName(centralWidget, titleStr, directoryStr, filterStr);
            if(resultList != nullptr)
                resultList->append(resultStr);
        } else if(cmd == "fileSaveDialog") {
            QString titleStr = cmdList.at(1);
            //QString buttonLabelStr = cmdList.at(2);
            QString directoryStr = cmdList.at(3);
            QString filterStr = cmdList.at(4);
            QString resultStr = FileDialog::getSaveFileName(centralWidget, titleStr, directoryStr, filterStr);
            if(resultList != nullptr)
                resultList->append(resultStr);
        } else if (cmd == "screenSend") {
            QString str = cmdList.at(1);
            bool synchronous = cmdList.at(2) == "True";
            centralWidget->se_screenSend(str,synchronous);
        } else if (cmd == "installWaitString") {
            QString timeout = cmdList.at(1);
            bool bcaseInsensitive = cmdList.at(2) == "True";
            int size = cmdList.at(3).toInt();
            QStringList strList;
            for (int i = 0; i < size; i++) {
                strList.append(cmdList.at(4 + i));
            }
            centralWidget->se_installWaitString(strList,timeout.toInt(),bcaseInsensitive);
        } else if (cmd == "sessionConnect") {
            QString cmd = cmdList.at(1);
            centralWidget->se_sessionConnect(cmd);
        } else if (cmd == "windowActivate") {
            centralWidget->se_activateWindow();
        } else if (cmd == "windowShow") {
            int type = cmdList.at(1).toInt();
            centralWidget->se_windowShow(type);
        } else if (cmd == "getWindowShowType") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_getWindowShowType()));
        } else if (cmd == "getWindowActive") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_getWindowActive()));
        } else if (cmd == "getCommandWindowText") {
            if(resultList != nullptr)
                resultList->append(centralWidget->se_getCommandWindowText());
        } else if (cmd == "setCommandWindowText") {
            QString text = cmdList.at(1);
            centralWidget->se_setCommandWindowText(text);
        } else if (cmd == "commandWindowSend") {
            centralWidget->se_commandWindowSend();
        } else if (cmd == "getDownloadFolder") {
            if(resultList != nullptr)
                resultList->append(centralWidget->se_getDownloadFolder());
        } else if (cmd == "addToUploadList") {
            QString file = cmdList.at(1);
            centralWidget->se_addToUploadList(file);
        } else if (cmd == "receiveKermit") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_receiveKermit()));
        } else if (cmd == "sendKermit") {
            QStringList files = {cmdList.at(1)};
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sendKermit(files)));
        } else if (cmd == "receiveXmodem") {
            QString file = cmdList.at(1);
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_receiveXmodem(file)));
        } else if (cmd == "sendXmodem") {
            QString file = cmdList.at(1);
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sendXmodem(file)));
        } else if (cmd == "receiveYmodem") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_receiveYmodem()));
        } else if (cmd == "sendYmodem") {
            QStringList files = {cmdList.at(1)};
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sendYmodem(files)));
        } else if (cmd == "sendZmodem") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sendZmodem()));
        } else if (cmd == "screenGetCurrentRow") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetCurrentRow()));
        } else if (cmd == "screenGetCurrentColumn") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetCurrentColumn()));
        } else if (cmd == "screenGetRows") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetRows()));
        } else if (cmd == "screenGetColumns") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetColumns()));
        } else if (cmd == "screenGetSelection") {
            if(resultList != nullptr)
                resultList->append(centralWidget->se_screenGetSelection());
        } else if (cmd == "screenClear") {
            centralWidget->se_screenClear();
        } else if (cmd == "screenGet") {
            int row1 = cmdList.at(1).toInt();
            int col1 = cmdList.at(2).toInt();
            int row2 = cmdList.at(3).toInt();
            int col2 = cmdList.at(4).toInt();
            if(resultList != nullptr)
                resultList->append(centralWidget->se_screenGet(row1,col1,row2,col2));
        } else if (cmd == "screenGet2") {
            int row1 = cmdList.at(1).toInt();
            int col1 = cmdList.at(2).toInt();
            int row2 = cmdList.at(3).toInt();
            int col2 = cmdList.at(4).toInt();
            if(resultList != nullptr)
                resultList->append(centralWidget->se_screenGet2(row1,col1,row2,col2));
        } else if (cmd == "screenPrint") {
            centralWidget->se_screenPrint();
        } else if (cmd == "screenShortcut") {
            QString path = cmdList.at(1);
            centralWidget->se_screenShortcut(path);
        } else if (cmd == "sessionGetLocked") {
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sessionGetLocked()));
        } else if (cmd == "screenSendKeys") {
            QStringList keys;
            int size = cmdList.at(1).toInt();
            for (int i = 0; i < size; i++) {
                keys.append(cmdList.at(2 + i));
            }
            QList<Qt::Key> keyList;
            for (int i = 0; i < keys.size(); i++) {
                keyList.append(keyMap.value(keys.at(i), Qt::Key_unknown));
            }
            centralWidget->se_screenSendKeys(keyList);
        } else if (cmd == "specialCmd") {
            int ret = -1;
            QString specialCmd = cmdList.at(1);
            if(specialCmd == "AskQuardShow") {
                specialCmdResult = "Thank you use QuardCRT, Have a nice day!";
                QMessageBox::information(centralWidget, "QuardCRT", specialCmdResult);
                ret = 0;
            } else if(specialCmd == "LoveTangtang") {
                specialCmdResult = "I love Tangtang!";
                QMessageBox::information(centralWidget, "QuardCRT", specialCmdResult);
                ret = 0;
            }
            if(resultList != nullptr)
                resultList->append(QString::number(ret));
        }
    }, Qt::BlockingQueuedConnection);

    start();
}

PyRun::~PyRun() {
    m_exit = true;
    for(int i=0;i<30;i++) {
        if(isRunning()) {
            cancelScript();
        }
        condition.wakeOne();
        if(wait(100)){
            break;
        }
    }
    condition.wakeOne();
    wait();
    Py_Finalize();
}

void PyRun::cancelScript(void) {
    QMutexLocker locker(&mutexUserStop);
    m_userStop = true;
    emitWaitForStringFinished("",-1);
}

bool PyRun::isStopScript(void) { 
    QMutexLocker locker(&mutexUserStop);
    if(m_userStop){
        if(!m_exit)
            m_userStop = false;
        return true;
    }
    return false;
}

void PyRun::runScript(QString scriptFile) {
    QMutexLocker locker(&mutex);
    runScriptList.enqueue(scriptFile);
    condition.wakeOne();
}

void PyRun::run() {
    while(m_exit == false) {
        mutex.lock();
        while(runScriptList.isEmpty()) {
            condition.wait(&mutex);
            if(m_exit) {
                mutex.unlock();
                return;
            }
        }
        QString scriptFile = runScriptList.dequeue();
        mutex.unlock();
        m_running = true;
        emit runScriptStarted();
        runScriptInternal(scriptFile);
        m_running = false;
        emit runScriptFinished();
    }
}

void PyRun::runScriptInternal(QString scriptFile) {
    QFile file(scriptFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open script file";
        return;
    }
    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lines.append(line);
    }
    file.close();
    QString script = lines.join("\n");
    QByteArray scriptBytes = script.toUtf8();
    const char* scriptStr = scriptBytes.constData();

    PyRun_SimpleString(scriptStr);
}

QString PyRun::getActivePrinter(void) {
    CentralWidget *centralWidget = static_cast<CentralWidget*>(parent());
    return centralWidget->se_getActivePrinter();
}

void PyRun::setActivePrinter(const QString &name) {
    CentralWidget *centralWidget = static_cast<CentralWidget*>(parent());
    centralWidget->se_setActivePrinter(name);
}

QString PyRun::getScriptFullName(void) {
    CentralWidget *centralWidget = static_cast<CentralWidget*>(parent());
    return centralWidget->se_getScriptFullName();
}

QString PyRun::getVersion(void) {
    return VERSION;
}

int PyRun::getActiveTabId(void) {
    CentralWidget *centralWidget = static_cast<CentralWidget*>(parent());
    return centralWidget->se_getActiveTabId();
}

void PyRun::messageBox(const QString &messageStr, const QString &titleStr, int buttons) {
    QStringList cmdList;
    cmdList.append("messageBox");
    cmdList.append(messageStr);
    cmdList.append(titleStr);
    cmdList.append(QString::number(buttons));
    emit requestCmd(cmdList,nullptr);
}

QString PyRun::prompt(const QString &promptStr, const QString &nameStr, const QString &inputStr, bool password) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("prompt");
    cmdList.append(promptStr);
    cmdList.append(nameStr);
    cmdList.append(inputStr);
    cmdList.append(password ? "True" : "False");
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

QString PyRun::fileOpenDialog(const QString &titleStr, const QString &buttonLabelStr, const QString &directoryStr, const QString &filterStr) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("fileOpenDialog");
    cmdList.append(titleStr);
    cmdList.append(buttonLabelStr);
    cmdList.append(directoryStr);
    cmdList.append(filterStr);
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

QString PyRun::fileSaveDialog(const QString &titleStr, const QString &buttonLabelStr, const QString &directoryStr, const QString &filterStr) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("fileSaveDialog");
    cmdList.append(titleStr);
    cmdList.append(buttonLabelStr);
    cmdList.append(directoryStr);
    cmdList.append(filterStr);
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

int PyRun::screenSend(const QString &str, bool synchronous) {
    QStringList cmdList;
    cmdList.append("screenSend");
    cmdList.append(str);
    cmdList.append(synchronous ? "True" : "False");
    emit requestCmd(cmdList,nullptr);
    return 0;
}

QString PyRun::screenWaitForString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex) {
    QStringList cmdList;
    cmdList.append("installWaitString");
    cmdList.append(QString::number(timeout));
    cmdList.append(bcaseInsensitive ? "True" : "False");
    cmdList.append(QString::number(strList.size()));
    for (int i = 0; i < strList.size(); i++) {
        cmdList.append(strList.at(i));
    }
    emit requestCmd(cmdList,nullptr);

    QMutexLocker locker(&waitForStringMutex);
    waitForStringCondition.wait(&waitForStringMutex);
    if(m_userStop) {
        matchIndex = -1;
        return "";
    }
    matchIndex = waitForStringMatchIndex;
    return waitForStringStrResult;
}

int PyRun::sessionConnect(const QString &cmd) {
    QStringList cmdList;
    cmdList.append("sessionConnect");
    cmdList.append(cmd);
    emit requestCmd(cmdList,nullptr);
    return 0;
}

void PyRun::windowActivate(void) {
    QStringList cmdList;
    cmdList.append("windowActivate");
    emit requestCmd(cmdList,nullptr);
}

void PyRun::windowShow(int type) {
    QStringList cmdList;
    cmdList.append("windowShow");
    cmdList.append(QString::number(type));
    emit requestCmd(cmdList,nullptr);
}

int PyRun::getWindowShowType(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("getWindowShowType");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

bool PyRun::getWindowActive(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("getWindowActive");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

QStringList PyRun::getArguments(void) {
    // get qApp arguments count
    return qApp->arguments();
}

QString PyRun::getCommandWindowText(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("getCommandWindowText");
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

void PyRun::setCommandWindowText(const QString &text) {
    QStringList cmdList;
    cmdList.append("setCommandWindowText");
    cmdList.append(text);
    emit requestCmd(cmdList,nullptr);
}

void PyRun::commandWindowSend(void) {
    QStringList cmdList;
    cmdList.append("commandWindowSend");
    emit requestCmd(cmdList,nullptr);
}

QString PyRun::getDownloadFolder(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("getDownloadFolder");
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

void PyRun::addToUploadList(const QString &file){
    QStringList cmdList;
    cmdList.append("addToUploadList");
    cmdList.append(file);
    emit requestCmd(cmdList,nullptr);
}

int PyRun::receiveKermit(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("receiveKermit");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::sendKermit(const QString &file){
    QStringList result;
    QStringList cmdList;
    cmdList.append("sendKermit");
    cmdList.append(file);
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::receiveXmodem(const QString &file){
    QStringList result;
    QStringList cmdList;
    cmdList.append("receiveXmodem");
    cmdList.append(file);
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::sendXmodem(const QString &file){
    QStringList result;
    QStringList cmdList;
    cmdList.append("sendXmodem");
    cmdList.append(file);
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::receiveYmodem(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("receiveYmodem");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::sendYmodem(const QString &file){
    QStringList result;
    QStringList cmdList;
    cmdList.append("sendYmodem");
    cmdList.append(file);
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::sendZmodem(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("sendZmodem");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetCurrentRow(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetCurrentRow");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetCurrentColumn(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetCurrentColumn");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetRows(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetRows");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetColumns(void){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetColumns");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

QString PyRun::screenGetSelection(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetSelection");
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

void PyRun::screenClear(void) {
    QStringList cmdList;
    cmdList.append("screenClear");
    emit requestCmd(cmdList,nullptr);
}

QString PyRun::screenGet(int row1, int col1, int row2, int col2) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGet");
    cmdList.append(QString::number(row1));
    cmdList.append(QString::number(col1));
    cmdList.append(QString::number(row2));
    cmdList.append(QString::number(col2));
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

QString PyRun::screenGet2(int row1, int col1, int row2, int col2) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGet2");
    cmdList.append(QString::number(row1));
    cmdList.append(QString::number(col1));
    cmdList.append(QString::number(row2));
    cmdList.append(QString::number(col2));
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

void PyRun::screenPrint(void) {
    QStringList cmdList;
    cmdList.append("screenPrint");
    emit requestCmd(cmdList,nullptr);
}

void PyRun::screenShortcut(const QString &path) {
    QStringList cmdList;
    cmdList.append("screenShortcut");
    cmdList.append(path);
    emit requestCmd(cmdList,nullptr);
}

bool PyRun::sessionGetLocked(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("sessionGetLocked");
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

void PyRun::screenSendKeys(const QStringList &keys) {
    QStringList cmdList;
    cmdList.append("screenSend");
    cmdList.append(QString::number(keys.size()));
    for (int i = 0; i < keys.size(); i++) {
        cmdList.append(keys.at(i));
    }
    emit requestCmd(cmdList,nullptr);
}

int PyRun::runSpecialCmd(const QString &cmd, const QStringList &arg) {
    if(specialSyncCmd.contains(cmd)) {
        QStringList result;
        QStringList cmdList;
        cmdList.append("specialCmd");
        cmdList.append(cmd);
        cmdList.append(QString::number(arg.size()));
        for (int i = 0; i < arg.size(); i++) {
            cmdList.append(arg.at(i));
        }
        emit requestCmd(cmdList,&result);
        return result.at(0).toInt();
    } else if(specialASyncCmd.contains(cmd)) {
        if(cmd == "AskQuard") {
            specialCmdResult = "Thank you use QuardCRT, Have a nice day!";
            return 0;
        }
        return -1;
    }
    return -1;
}

QString PyRun::getSpecialCmdResult(void) {
    return specialCmdResult;
}

