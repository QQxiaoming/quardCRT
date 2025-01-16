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
#include "pycore.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include <QSystemTrayIcon>
#include <QDebug>

#include "mainwindow.h"
#include "filedialog.h"
#include "pyrun.h"

PyRun::PyRun(QObject *parent)
    : QThread{parent} {
    QString pythonHome;
    if(PyRun::overridePythonLibPath.isEmpty()) {
    #if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
        pythonHome = QApplication::applicationDirPath() + "/pythonlib";
    #elif defined(Q_OS_MACOS)
        pythonHome = QApplication::applicationDirPath() + "/../Frameworks/pythonlib";
    #endif
    } else {
        pythonHome = PyRun::overridePythonLibPath;
    }
    if (QDir(pythonHome).exists()) {
        std::wstring  pythonHomeWStr = pythonHome.toStdWString();
        const wchar_t* pythonHomeStr = pythonHomeWStr.c_str();
        Py_SetPythonHome(pythonHomeStr);
    } else {
        qDebug() << "Python home not overriden, Maybe in debug mode, using system python";
    }
    Py_Initialize();

    // Register the trace function
    PyEval_SetProfile(reinterpret_cast<Py_tracefunc>(PyCore::stop_script), NULL);

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
        } else if (cmd == "messageNotifications") {
            QString messageStr = cmdList.at(1);
            centralWidget->se_messageNotifications(messageStr);
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
        } else if (cmd == "getCommandWindowVisibled") {
            if(resultList != nullptr)
                resultList->append(centralWidget->se_getCommandWindowVisibled()? "True" : "False");
        } else if (cmd == "setCommandWindowVisibled") {
            bool enable = cmdList.at(1) == "True";
            centralWidget->se_getCommandWindowVisibled(enable);
        } else if (cmd == "getDownloadFolder") {
            if(resultList != nullptr)
                resultList->append(centralWidget->se_getDownloadFolder());
        } else if (cmd == "addToUploadList") {
            QString file = cmdList.at(1);
            centralWidget->se_addToUploadList(file);
        } else if (cmd == "clearUploadList") {
            centralWidget->se_clearUploadList();
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
        } else if (cmd == "screenSend") {
            int id = cmdList.at(1).toInt();
            QString str = cmdList.at(2);
            bool synchronous = cmdList.at(3) == "True";
            centralWidget->se_screenSend(str,synchronous,id);
        } else if (cmd == "installWaitString") {
            int id = cmdList.at(1).toInt();
            QString timeout = cmdList.at(2);
            bool bcaseInsensitive = cmdList.at(3) == "True";
            int mode = cmdList.at(4) == "Wait string" ? 0 : 1;
            int size = cmdList.at(5).toInt();
            QStringList strList;
            for (int i = 0; i < size; i++) {
                strList.append(cmdList.at(6 + i));
            }
            centralWidget->se_installWaitString(strList,timeout.toInt(),bcaseInsensitive,mode,id);
        } else if (cmd == "screenGetCurrentRow") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetCurrentRow(id)));
        } else if (cmd == "screenGetCurrentColumn") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetCurrentColumn(id)));
        } else if (cmd == "screenGetRows") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetRows(id)));
        } else if (cmd == "screenGetColumns") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_screenGetColumns(id)));
        } else if (cmd == "screenGetSelection") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(centralWidget->se_screenGetSelection(id));
        } else if (cmd == "screenClear") {
            int id = cmdList.at(1).toInt();
            centralWidget->se_screenClear(id);
        } else if (cmd == "screenGet") {
            int id = cmdList.at(1).toInt();
            int row1 = cmdList.at(2).toInt();
            int col1 = cmdList.at(3).toInt();
            int row2 = cmdList.at(4).toInt();
            int col2 = cmdList.at(5).toInt();
            if(resultList != nullptr)
                resultList->append(centralWidget->se_screenGet(row1,col1,row2,col2,id));
        } else if (cmd == "screenGet2") {
            int id = cmdList.at(1).toInt();
            int row1 = cmdList.at(2).toInt();
            int col1 = cmdList.at(3).toInt();
            int row2 = cmdList.at(4).toInt();
            int col2 = cmdList.at(5).toInt();
            if(resultList != nullptr)
                resultList->append(centralWidget->se_screenGet2(row1,col1,row2,col2,id));
        } else if (cmd == "screenPrint") {
            int id = cmdList.at(1).toInt();
            centralWidget->se_screenPrint(id);
        } else if (cmd == "screenShortcut") {
            int id = cmdList.at(1).toInt();
            QString path = cmdList.at(2);
            centralWidget->se_screenShortcut(path,id);
        }  else if (cmd == "screenSendKeys") {
            QStringList keys;
            int id = cmdList.at(1).toInt();
            int size = cmdList.at(2).toInt();
            for (int i = 0; i < size; i++) {
                keys.append(cmdList.at(3 + i));
            }
            QList<Qt::Key> keyList;
            for (int i = 0; i < keys.size(); i++) {
                keyList.append(keyMap.value(keys.at(i), Qt::Key_unknown));
            }
            centralWidget->se_screenSendKeys(keyList,id);
        } else if (cmd == "sessionGetLocked") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sessionGetLocked(id)));
        } else if (cmd == "sessionGetConnected") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sessionGetConnected(id)));
        } else if (cmd == "sessionGetLogging") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sessionGetLogging(id)));
        } else if (cmd == "sessionConnect") {
            int id = cmdList.at(1).toInt();
            QString ccmd = cmdList.at(2);
            centralWidget->se_sessionConnect(ccmd,id);
        } else if (cmd == "sessionDisconnect") {
            int id = cmdList.at(1).toInt();
            centralWidget->se_sessionDisconnect(id);
        } else if (cmd == "sessionLog") {
            int id = cmdList.at(1).toInt();
            int enable = cmdList.at(2).toInt();
            centralWidget->se_sessionLog(enable,id);
        } else if (cmd == "sessionLock") {
            int id = cmdList.at(1).toInt();
            QString password = cmdList.at(3);
            int lockallsessions = cmdList.at(4).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sessionLock(password,lockallsessions,id)));
        } else if (cmd == "sessionUnlock") {
            int id = cmdList.at(1).toInt();
            QString password = cmdList.at(3);
            int lockallsessions = cmdList.at(4).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_sessionUnlock(password,lockallsessions,id)));
        } else if (cmd == "tabGetnumber") {
            int id = cmdList.at(1).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_tabGetnumber(id)));
        } else if(cmd == "tabCheckScreenId") {
            int tabid = cmdList.at(1).toInt();
            int id = cmdList.at(2).toInt();
            if(resultList != nullptr)
                resultList->append(QString::number(centralWidget->se_tabCheckScreenId(tabid,id)));
        } else if(cmd == "tabActivate") {
            int tabid = cmdList.at(1).toInt();
            int id = cmdList.at(2).toInt();
            centralWidget->se_tabActivate(tabid,id);
        } else if (cmd == "specialCmd") {
            int ret = -1;
            QString specialCmd = cmdList.at(1);
            if(specialCmd == "AskQuardShow") {
                specialCmdResult = "Thank you use QuardCRT, Have a nice day!";
                QMessageBox::information(centralWidget, "QuardCRT", specialCmdResult);
                ret = 0;
            } else if(specialCmd == "QuardSOnly") {
                class MyDialog : public QDialog {
                public:
                    MyDialog(QWidget *parent = nullptr) : QDialog(parent) {
                        setAttribute(Qt::WA_StyledBackground, true);
                        setStyleSheet("QWidget#MyDialog {background-color: transparent;}");
                    #if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
                        setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint);
                    #else
                        setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
                    #endif
                        QPixmap pix;
                        pix.load(":/icons/icons/about2.png",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
                        resize(pix.size());
                        setMask(QBitmap(pix.mask()));
                        QVBoxLayout *layout = new QVBoxLayout(this);
                        QLabel *label = new QLabel("<html><head/><body><p align=\"center\">I love Tangtang!</p></body></html>", this);
                        layout->addWidget(label);
                    }
                
                protected:
                    void paintEvent(QPaintEvent *event) {
                        QString paths = ":/icons/icons/about2.png";
                        QPainter painter(this);
                        painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));
                        event->accept();
                    }
                    void mouseReleaseEvent(QMouseEvent *event) {
                        if( event->button() == Qt::LeftButton) {
                            accept();
                        }
                    }
                };
                MyDialog show;
                show.setObjectName("MyDialog");
                show.exec();
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

void PyRun::runScript(struct ScriptInfo *scriptInfo) {
    QMutexLocker locker(&mutex);
    runScriptList.enqueue(*scriptInfo);
    condition.wakeOne();
}

void PyRun::runScriptFile(const QString &scriptFile, QString *result, int *ret) {
    if(result != nullptr) {
        volatile bool finished = false;
        struct ScriptInfo scriptInfo = {0,scriptFile,result,ret,&finished};
        runScript(&scriptInfo);
        while(!finished) {
            qApp->processEvents();
        }
    } else {
        struct ScriptInfo scriptInfo = {0,scriptFile,nullptr,nullptr,nullptr};
        runScript(&scriptInfo);
    }
}

void PyRun::runScriptStr(const QString &scriptStr, QString *result, int *ret) {
    if(result != nullptr) {
        volatile bool finished = false;
        struct ScriptInfo scriptInfo = {1,scriptStr,result,ret,&finished};
        runScript(&scriptInfo);
        while(!finished) {
            qApp->processEvents();
        }
    } else {
        struct ScriptInfo scriptInfo = {1,scriptStr,nullptr,nullptr,nullptr};
        runScript(&scriptInfo);
    }
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
        //QStringList script = runScriptList.dequeue();
        struct ScriptInfo scriptInfo = runScriptList.dequeue();
        mutex.unlock();
        m_running = true;
        emit runScriptStarted();
        QString *result = scriptInfo.result;
        if(scriptInfo.type == 0) {
            if(result != nullptr)
                *result = runScriptFileInternal(scriptInfo.script, scriptInfo.ret);
            else
                runScriptFileInternal(scriptInfo.script,nullptr);
        } else if(scriptInfo.type == 1) {
            if(result != nullptr)
                *result = runScriptStrInternal(scriptInfo.script, scriptInfo.ret);
            else
                runScriptStrInternal(scriptInfo.script,nullptr);
        }
        if(scriptInfo.finished != nullptr)
            *scriptInfo.finished = true;
        m_running = false;
        emit runScriptFinished();
    }
}

QString PyRun::runScriptFileInternal(const QString &scriptFile, int *sret) {
    QFile file(scriptFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open script file";
        return QString();
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

    PyObject* ioModule = PyImport_ImportModule("io");
    PyObject* stringIO = PyObject_CallMethod(ioModule, "StringIO", nullptr);
    PyObject* sysModule = PyImport_ImportModule("sys");
    PyObject_SetAttrString(sysModule, "stdout", stringIO);

    int ret = PyRun_SimpleString(scriptStr);
    if(sret) {
        *sret = ret;
    }

    PyObject* output = PyObject_CallMethod(stringIO, "getvalue", nullptr);
    const char* outputStr = PyUnicode_AsUTF8(output);
    QString outputQString = QString::fromUtf8(outputStr);
    Py_DECREF(output);
    Py_DECREF(ioModule);
    Py_DECREF(stringIO);
    Py_DECREF(sysModule);

    if(!sret) {
        messageNotifications(tr("Script finished with return code: ") + QString::number(ret));
    }

    return outputQString;
}

QString PyRun::runScriptStrInternal(const QString &scriptStr, int *sret) {
    QByteArray scriptBytes = scriptStr.toUtf8();
    const char* scriptString = scriptBytes.constData();

    PyObject* ioModule = PyImport_ImportModule("io");
    PyObject* stringIO = PyObject_CallMethod(ioModule, "StringIO", nullptr);
    PyObject* sysModule = PyImport_ImportModule("sys");
    PyObject_SetAttrString(sysModule, "stdout", stringIO);

    int ret = PyRun_SimpleString(scriptString);
    if(sret) {
        *sret = ret;
    }

    PyObject* output = PyObject_CallMethod(stringIO, "getvalue", nullptr);
    const char* outputStr = PyUnicode_AsUTF8(output);
    QString outputQString = QString::fromUtf8(outputStr);
    Py_DECREF(output);
    Py_DECREF(ioModule);
    Py_DECREF(stringIO);
    Py_DECREF(sysModule);

    if(!sret) {
        messageNotifications(tr("Script finished with return code: ") + QString::number(ret));
    }
    
    return outputQString;
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

int PyRun::getActiveSessionId(void) {
    CentralWidget *centralWidget = static_cast<CentralWidget*>(parent());
    return centralWidget->se_getActiveSessionId();
}

void PyRun::messageBox(const QString &messageStr, const QString &titleStr, int buttons) {
    QStringList cmdList;
    cmdList.append("messageBox");
    cmdList.append(messageStr);
    cmdList.append(titleStr);
    cmdList.append(QString::number(buttons));
    emit requestCmd(cmdList,nullptr);
}

void PyRun::messageNotifications(const QString &messageStr) {
    QStringList cmdList;
    cmdList.append("messageNotifications");
    cmdList.append(messageStr);
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

bool PyRun::getCommandWindowVisibled(void) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("getCommandWindowVisibled");
    emit requestCmd(cmdList,&result);
    return result.at(0) == "True";
}

void PyRun::setCommandWindowVisibled(bool enable) {
    QStringList cmdList;
    cmdList.append("setCommandWindowVisibled");
    cmdList.append(enable ? "True" : "False");
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

void PyRun::clearUploadList(void){
    QStringList cmdList;
    cmdList.append("clearUploadList");
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

int PyRun::screenGetCurrentRow(int id){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetCurrentRow");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetCurrentColumn(int id){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetCurrentColumn");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetRows(int id){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetRows");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::screenGetColumns(int id){
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetColumns");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

QString PyRun::screenGetSelection(int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGetSelection");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

void PyRun::screenClear(int id) {
    QStringList cmdList;
    cmdList.append("screenClear");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,nullptr);
}

QString PyRun::screenGet(int row1, int col1, int row2, int col2, int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGet");
    cmdList.append(QString::number(id));
    cmdList.append(QString::number(row1));
    cmdList.append(QString::number(col1));
    cmdList.append(QString::number(row2));
    cmdList.append(QString::number(col2));
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

QString PyRun::screenGet2(int row1, int col1, int row2, int col2, int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("screenGet2");
    cmdList.append(QString::number(id));
    cmdList.append(QString::number(row1));
    cmdList.append(QString::number(col1));
    cmdList.append(QString::number(row2));
    cmdList.append(QString::number(col2));
    emit requestCmd(cmdList,&result);
    return result.at(0);
}

void PyRun::screenPrint(int id) {
    QStringList cmdList;
    cmdList.append("screenPrint");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,nullptr);
}

void PyRun::screenShortcut(const QString &path, int id) {
    QStringList cmdList;
    cmdList.append("screenShortcut");
    cmdList.append(QString::number(id));
    cmdList.append(path);
    emit requestCmd(cmdList,nullptr);
}

int PyRun::screenSend(const QString &str, bool synchronous, int id) {
    QStringList cmdList;
    cmdList.append("screenSend");
    cmdList.append(QString::number(id));
    cmdList.append(str);
    cmdList.append(synchronous ? "True" : "False");
    emit requestCmd(cmdList,nullptr);
    return 0;
}

QString PyRun::screenWaitForString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex, int id) {
    QStringList cmdList;
    cmdList.append("installWaitString");
    cmdList.append(QString::number(id));
    cmdList.append(QString::number(timeout));
    cmdList.append(bcaseInsensitive ? "True" : "False");
    cmdList.append("Wait string");
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

QString PyRun::screenReadString(const QStringList &strList, int timeout, bool bcaseInsensitive, int &matchIndex, int id) {
    QStringList cmdList;
    cmdList.append("installWaitString");
    cmdList.append(QString::number(id));
    cmdList.append(QString::number(timeout));
    cmdList.append(bcaseInsensitive ? "True" : "False");
    cmdList.append("Read string");
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

void PyRun::screenSendKeys(const QStringList &keys, int id) {
    QStringList cmdList;
    cmdList.append("screenSend");
    cmdList.append(QString::number(id));
    cmdList.append(QString::number(keys.size()));
    for (int i = 0; i < keys.size(); i++) {
        cmdList.append(keys.at(i));
    }
    emit requestCmd(cmdList,nullptr);
}

int PyRun::sessionLock(const QString &prompt, const QString &password, int lockallsessions, int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("sessionLock");
    cmdList.append(QString::number(id));
    cmdList.append(prompt);
    cmdList.append(password);
    cmdList.append(QString::number(lockallsessions));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::sessionUnlock(const QString &prompt, const QString &password, int lockallsessions, int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("sessionUnlock");
    cmdList.append(QString::number(id));
    cmdList.append(prompt);
    cmdList.append(password);
    cmdList.append(QString::number(lockallsessions));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::sessionConnect(const QString &cmd, int id) {
    QStringList cmdList;
    cmdList.append("sessionConnect");
    cmdList.append(QString::number(id));
    cmdList.append(cmd);
    emit requestCmd(cmdList,nullptr);
    return 0;
}

void PyRun::sessionDisconnect(int id) {
    QStringList cmdList;
    cmdList.append("sessionDisconnect");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,nullptr);
}

void PyRun::sessionLog(int enable, int id) {
    QStringList cmdList;
    cmdList.append("sessionLog");
    cmdList.append(QString::number(id));
    cmdList.append(QString::number(enable));
    emit requestCmd(cmdList,nullptr);
}

bool PyRun::sessionGetLocked(int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("sessionGetLocked");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

bool PyRun::sessionGetConnected(int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("sessionGetConnected");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

bool PyRun::sessionGetLogging(int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("sessionGetLogging");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::tabGetnumber(int id) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("tabGetnumber");
    cmdList.append(QString::number(id));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

int PyRun::tabCheckScreenId(int tabId, int screenId) {
    QStringList result;
    QStringList cmdList;
    cmdList.append("tabCheckScreenId");
    cmdList.append(QString::number(tabId));
    cmdList.append(QString::number(screenId));
    emit requestCmd(cmdList,&result);
    return result.at(0).toInt();
}

void PyRun::tabActivate(int tabId, int screenId) {
    QStringList cmdList;
    cmdList.append("tabActivate");
    cmdList.append(QString::number(tabId));
    cmdList.append(QString::number(screenId));
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

QString PyRun::overridePythonLibPath = QString();
void PyRun::setOverridePythonLib(const QString &path) {
    PyRun::overridePythonLibPath = path;
}
