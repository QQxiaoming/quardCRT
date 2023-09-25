#ifndef WINPTYPROCESS_H
#define WINPTYPROCESS_H

#include "iptyprocess.h"
#include "winpty.h"

class WinPtyProcess : public IPtyProcess
{
public:
    WinPtyProcess();
    ~WinPtyProcess();

    bool startProcess(const QString &shellPath, QStringList environment, qint16 cols, qint16 rows);
    bool resize(qint16 cols, qint16 rows);
    bool kill();
    PtyType type();
    QString dumpDebugInfo();
    QIODevice *notifier();
    QByteArray readAll();
    qint64 write(const QByteArray &byteArray);
    bool isAvailable();
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
