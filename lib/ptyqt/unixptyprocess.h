#ifndef UNIXPTYPROCESS_H
#define UNIXPTYPROCESS_H

#include "iptyprocess.h"
#include <QProcess>
#include <QSocketNotifier>

#include <termios.h>
#include <errno.h>
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_FREEBSD)
#include <utmpx.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

// support for build with MUSL on Alpine Linux
#ifndef _PATH_UTMPX
#include <sys/time.h>
# define _PATH_UTMPX	"/var/log/utmp"
#endif

class ShellProcess : public QProcess
{
    friend class UnixPtyProcess;
    Q_OBJECT
public:
    ShellProcess( )
        : QProcess( )
        , m_handleMaster(-1)
        , m_handleSlave(-1)
    {
        setProcessChannelMode(QProcess::SeparateChannels);

        setChildProcessModifier([this] {
            dup2(m_handleSlave, STDIN_FILENO);
            dup2(m_handleSlave, STDOUT_FILENO);
            dup2(m_handleSlave, STDERR_FILENO);

            pid_t sid = setsid();
            ioctl(m_handleSlave, TIOCSCTTY, 0);
            tcsetpgrp(m_handleSlave, sid);

        #if !defined(Q_OS_ANDROID) && !defined(Q_OS_FREEBSD)
            // on Android imposible to put record to the 'utmp' file
            struct utmpx utmpxInfo;
            memset(&utmpxInfo, 0, sizeof(utmpxInfo));

            strncpy(utmpxInfo.ut_user, qgetenv("USER"), sizeof(utmpxInfo.ut_user));

            QString device(m_handleSlaveName);
            if (device.startsWith("/dev/"))
                device = device.mid(5);

            QByteArray ba = device.toLatin1();
            const char *d = ba.constData();

            strncpy(utmpxInfo.ut_line, d, sizeof(utmpxInfo.ut_line));

            strncpy(utmpxInfo.ut_id, d + strlen(d) - sizeof(utmpxInfo.ut_id), sizeof(utmpxInfo.ut_id));

            struct timeval tv;
            gettimeofday(&tv, 0);
            utmpxInfo.ut_tv.tv_sec = tv.tv_sec;
            utmpxInfo.ut_tv.tv_usec = tv.tv_usec;

            utmpxInfo.ut_type = USER_PROCESS;
            utmpxInfo.ut_pid = getpid();

            utmpxname(_PATH_UTMPX);
            setutxent();
            pututxline(&utmpxInfo);
            endutxent();

        #if !defined(Q_OS_UNIX)
            updwtmpx(_PATH_UTMPX, &loginInfo);
        #endif

        #endif
        } );
    }

    void emitReadyRead()
    {
        emit readyRead();
    }

private:
    int m_handleMaster, m_handleSlave;
    QString m_handleSlaveName;
};

class UnixPtyProcess : public IPtyProcess
{
public:
    UnixPtyProcess();
    virtual ~UnixPtyProcess();

    virtual bool startProcess(const QString &executable,
                      const QStringList &arguments,
                      const QString &workingDir,
                      QStringList environment,
                      qint16 cols,
                      qint16 rows);
    virtual bool resize(qint16 cols, qint16 rows);
    virtual bool kill();
    virtual PtyType type();
    virtual QString dumpDebugInfo();
    virtual QIODevice *notifier();
    virtual QByteArray readAll();
    virtual qint64 write(const QByteArray &byteArray);
    virtual QString currentDir();
    virtual bool hasChildProcess();
    virtual pidTree_t processInfoTree();
    static bool isAvailable();
    void moveToThread(QThread *targetThread);

private:
    ShellProcess m_shellProcess;
    QSocketNotifier *m_readMasterNotify;
    QByteArray m_shellReadBuffer;
};

#endif // UNIXPTYPROCESS_H
