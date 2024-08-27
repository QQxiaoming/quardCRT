#include "unixptyprocess.h"
#include <QStandardPaths>

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

UnixPtyProcess::UnixPtyProcess()
    : IPtyProcess()
    , m_readMasterNotify(0)
{
    m_shellProcess.setWorkingDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

UnixPtyProcess::~UnixPtyProcess()
{
    kill();
}

bool UnixPtyProcess::startProcess(const QString &executable,
                                 const QStringList &arguments,
                                 const QString &workingDir,
                                 QStringList environment,
                                 qint16 cols,
                                 qint16 rows)
{
    if (!isAvailable())
    {
        m_lastError = QString("UnixPty Error: unavailable");
        return false;
    }

    if (m_shellProcess.state() == QProcess::Running)
        return false;

    QFileInfo fi(executable);
    if (fi.isRelative() || !QFile::exists(executable))
    {
        //todo add auto-find executable in PATH env var
        m_lastError = QString("UnixPty Error: shell file path must be absolute");
        return false;
    }

    m_shellPath = executable;
    m_size = QPair<qint16, qint16>(cols, rows);

    int rc = 0;

    m_shellProcess.m_handleMaster = ::posix_openpt(O_RDWR | O_NOCTTY);
    if (m_shellProcess.m_handleMaster <= 0)
    {
        m_lastError = QString("UnixPty Error: unable to open master -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    m_shellProcess.m_handleSlaveName = ptsname(m_shellProcess.m_handleMaster);
    if ( m_shellProcess.m_handleSlaveName.isEmpty())
    {
        m_lastError = QString("UnixPty Error: unable to get slave name -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    rc = grantpt(m_shellProcess.m_handleMaster);
    if (rc != 0)
    {
        m_lastError = QString("UnixPty Error: unable to change perms for slave -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    rc = unlockpt(m_shellProcess.m_handleMaster);
    if (rc != 0)
    {
        m_lastError = QString("UnixPty Error: unable to unlock slave -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    m_shellProcess.m_handleSlave = ::open(m_shellProcess.m_handleSlaveName.toLatin1().data(), O_RDWR | O_NOCTTY);
    if (m_shellProcess.m_handleSlave < 0)
    {
        m_lastError = QString("UnixPty Error: unable to open slave -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    rc = fcntl(m_shellProcess.m_handleMaster, F_SETFD, FD_CLOEXEC);
    if (rc == -1)
    {
        m_lastError = QString("UnixPty Error: unable to set flags for master -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    rc = fcntl(m_shellProcess.m_handleSlave, F_SETFD, FD_CLOEXEC);
    if (rc == -1)
    {
        m_lastError = QString("UnixPty Error: unable to set flags for slave -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    struct ::termios ttmode;
    rc = tcgetattr(m_shellProcess.m_handleMaster, &ttmode);
    if (rc != 0)
    {
        m_lastError = QString("UnixPty Error: termios fail -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    ttmode.c_iflag = ICRNL | IXON | IXANY | IMAXBEL | BRKINT;
#if defined(IUTF8)
    ttmode.c_iflag |= IUTF8;
#endif

    ttmode.c_oflag = OPOST | ONLCR;
    ttmode.c_cflag = CREAD | CS8 | HUPCL;
    ttmode.c_lflag = ICANON | ISIG | IEXTEN | ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL;

    ttmode.c_cc[VEOF] = 4;
    ttmode.c_cc[VEOL] = -1;
    ttmode.c_cc[VEOL2] = -1;
    ttmode.c_cc[VERASE] = 0x7f;
    ttmode.c_cc[VWERASE] = 23;
    ttmode.c_cc[VKILL] = 21;
    ttmode.c_cc[VREPRINT] = 18;
    ttmode.c_cc[VINTR] = 3;
    ttmode.c_cc[VQUIT] = 0x1c;
    ttmode.c_cc[VSUSP] = 26;
    ttmode.c_cc[VSTART] = 17;
    ttmode.c_cc[VSTOP] = 19;
    ttmode.c_cc[VLNEXT] = 22;
    ttmode.c_cc[VDISCARD] = 15;
    ttmode.c_cc[VMIN] = 1;
    ttmode.c_cc[VTIME] = 0;

#if (__APPLE__)
    ttmode.c_cc[VDSUSP] = 25;
    ttmode.c_cc[VSTATUS] = 20;
#endif

    cfsetispeed(&ttmode, B38400);
    cfsetospeed(&ttmode, B38400);

    rc = tcsetattr(m_shellProcess.m_handleMaster, TCSANOW, &ttmode);
    if (rc != 0)
    {
        m_lastError = QString("UnixPty Error: unabble to set associated params -> %1").arg(strerror(errno));
        kill();
        return false;
    }

    m_readMasterNotify = new QSocketNotifier(m_shellProcess.m_handleMaster, QSocketNotifier::Read, &m_shellProcess);
    m_readMasterNotify->setEnabled(true);
    m_readMasterNotify->moveToThread(m_shellProcess.thread());
    QObject::connect(m_readMasterNotify, &QSocketNotifier::activated, [this](int socket)
    {
        Q_UNUSED(socket)

        QByteArray buffer;
        int size = 1025;
        int readSize = 1024;
        QByteArray data;
        do
        {
            char nativeBuffer[size];
            int len = ::read(m_shellProcess.m_handleMaster, nativeBuffer, readSize);
            data = QByteArray(nativeBuffer, len);
            buffer.append(data);
        } while (data.size() == readSize); //last data block always < readSize

        m_shellReadBuffer.append(buffer);
        m_shellProcess.emitReadyRead();
    });

    QStringList defaultVars;

    defaultVars.append("TERM=xterm-256color");
    defaultVars.append("ITERM_PROFILE=Default");
    defaultVars.append("XPC_FLAGS=0x0");
    defaultVars.append("XPC_SERVICE_NAME=0");
    defaultVars.append("INIT_CWD=" + workingDir);
    defaultVars.append("COMMAND_MODE=unix2003");
    defaultVars.append("COLORTERM=truecolor");

    QStringList varNames;
    foreach (QString line, environment)
    {
        varNames.append(line.split("=").first());
    }

    //append default env vars only if they don't exists in current env
    foreach (QString defVar, defaultVars)
    {
        if (!varNames.contains(defVar.split("=").first()))
            environment.append(defVar);
    }

    QProcessEnvironment envFormat = QProcessEnvironment::systemEnvironment();
    foreach (QString line, environment)
    {
        if(envFormat.contains(line.split("=").first())) {
            envFormat.remove(line.split("=").first());
            envFormat.insert(line.split("=").first(), line.split("=").last());
        } else {
            envFormat.insert(line.split("=").first(), line.split("=").last());
        }
    }
    m_shellProcess.setWorkingDirectory(workingDir);
    m_shellProcess.setProcessEnvironment(envFormat);
    m_shellProcess.setReadChannel(QProcess::StandardOutput);


    m_shellProcess.start(m_shellPath, arguments);
    m_shellProcess.waitForStarted();

    m_pid = m_shellProcess.processId();

    resize(cols, rows);

    return true;
}

bool UnixPtyProcess::resize(qint16 cols, qint16 rows)
{
    struct winsize winp;
    winp.ws_col = cols;
    winp.ws_row = rows;
    winp.ws_xpixel = 0;
    winp.ws_ypixel = 0;

    bool res =  ( (ioctl(m_shellProcess.m_handleMaster, TIOCSWINSZ, &winp) != -1) && (ioctl(m_shellProcess.m_handleSlave, TIOCSWINSZ, &winp) != -1) );

    if (res)
    {
        m_size = QPair<qint16, qint16>(cols, rows);
    }

    return res;
}

bool UnixPtyProcess::kill()
{
    m_shellProcess.m_handleSlaveName = QString();
    if (m_shellProcess.m_handleSlave >= 0)
    {
        ::close(m_shellProcess.m_handleSlave);
        m_shellProcess.m_handleSlave = -1;
    }
    if (m_shellProcess.m_handleMaster >= 0)
    {
        ::close(m_shellProcess.m_handleMaster);
        m_shellProcess.m_handleMaster = -1;
    }

    if (m_shellProcess.state() == QProcess::Running)
    {
        m_readMasterNotify->disconnect();
        m_readMasterNotify->deleteLater();

        m_shellProcess.terminate();
        m_shellProcess.waitForFinished(1000);

        if (m_shellProcess.state() == QProcess::Running)
        {
            QProcess::startDetached( QString("kill -9 %1").arg( pid() ) );
            m_shellProcess.kill();
            m_shellProcess.waitForFinished(1000);
        }

        return (m_shellProcess.state() == QProcess::NotRunning);
    }
    return false;
}

IPtyProcess::PtyType UnixPtyProcess::type()
{
    return IPtyProcess::UnixPty;
}

QString UnixPtyProcess::dumpDebugInfo()
{
#ifdef PTYQT_DEBUG
    return QString("PID: %1, In: %2, Out: %3, Type: %4, Cols: %5, Rows: %6, IsRunning: %7, Shell: %8, SlaveName: %9")
            .arg(m_pid).arg(m_shellProcess.m_handleMaster).arg(m_shellProcess.m_handleSlave).arg(type())
            .arg(m_size.first).arg(m_size.second).arg(m_shellProcess.state() == QProcess::Running)
            .arg(m_shellPath).arg(m_shellProcess.m_handleSlaveName);
#else
    return QString("Nothing...");
#endif
}

QIODevice *UnixPtyProcess::notifier()
{
    return &m_shellProcess;
}

QByteArray UnixPtyProcess::readAll()
{
    QByteArray tmpBuffer =  m_shellReadBuffer;
    m_shellReadBuffer.clear();
    return tmpBuffer;
}

qint64 UnixPtyProcess::write(const QByteArray &byteArray)
{
    int result = ::write(m_shellProcess.m_handleMaster, byteArray.constData(), byteArray.size());
    Q_UNUSED(result)

    return byteArray.size();
}

QString UnixPtyProcess::currentDir()
{
    return QDir::currentPath();
}

bool UnixPtyProcess::hasChildProcess()
{
    pidTree_t pidTree = processInfoTree();
    return (pidTree.children.size() > 0);
}

UnixPtyProcess::pidTree_t UnixPtyProcess::processInfoTree()
{
    QList<psInfo_t> psInfoList;
    QString cmd("ps");
    QStringList args = { "-o", "pid,ppid,command", "-ax" };
    QProcess ps;
    ps.start(cmd, args);
    bool isOk = ps.waitForFinished();
    QString psResult = ps.readAllStandardOutput();
    if((!isOk) || psResult.isEmpty()) {
        pidTree_t tree = { { m_pid, 0, m_shellPath, QStringList() }, QList<pidTree_t>() };
        return tree;
    }
    QStringList psLines = psResult.split("\n");
    foreach (QString line, psLines)
    {
        if (line.contains("PID"))
            continue;

        QStringList lineParts = line.split(" ");
        QStringList linePartsFiltered;
        foreach (QString part, lineParts)
        {
            if (!part.isEmpty())
                linePartsFiltered.append(part);
        }
        if (linePartsFiltered.size() < 3)
            continue;

        struct psInfo_t info;
        info.pid = linePartsFiltered.at(0).toLongLong();
        info.ppid = linePartsFiltered.at(1).toLongLong();
        info.command = linePartsFiltered.at(2);
        info.args = linePartsFiltered.mid(3);
        psInfoList.append(info);
    }

    std::function<QList<pidTree_t>(qint64)> findChild = [&](qint64 pid) -> QList<pidTree_t> {
            QList<pidTree_t> result;
            foreach (psInfo_t info, psInfoList)
            {
                if (info.ppid == pid)
                {
                    pidTree_t tree;
                    tree.pidInfo.pid = info.pid;
                    tree.pidInfo.ppid = info.ppid;
                    tree.pidInfo.command = info.command;
                    tree.children = findChild(info.pid);
                    result.append(tree);
                }
            }
            return result;
        };
    pidTree_t tree = { { m_pid, 0, m_shellPath, QStringList() }, findChild(m_pid) };
    return tree;
}

bool UnixPtyProcess::isAvailable()
{
	//todo check something more if required
    return true;
}

void UnixPtyProcess::moveToThread(QThread *targetThread)
{
    m_shellProcess.moveToThread(targetThread);
}

