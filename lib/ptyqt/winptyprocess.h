#ifndef WINPTYPROCESS_H
#define WINPTYPROCESS_H

#include "iptyprocess.h"
#include "winpty.h"

class WinPtyProcess : public IPtyProcess
{
public:
    WinPtyProcess();
    ~WinPtyProcess();

    virtual bool startProcess(const QString &executable,
                    const QStringList &arguments,
                    const QString &workingDir,
                    QStringList environment,
                    qint16 cols,
                    qint16 rows);
    bool resize(qint16 cols, qint16 rows);
    bool kill();
    PtyType type();
    QString dumpDebugInfo();
    QIODevice *notifier();
    QByteArray readAll();
    qint64 write(const QByteArray &byteArray);
    QString currentDir();
    virtual bool hasChildProcess();
    virtual pidTree_t processInfoTree();
    static bool isAvailable();
    void moveToThread(QThread *targetThread);

private:
    winpty_t *m_ptyHandler;
    HANDLE m_innerHandle;
    QString m_conInName;
    QString m_conOutName;
    QLocalSocket *m_inSocket;
    QLocalSocket *m_outSocket;
};

#endif // WINPTYPROCESS_H
