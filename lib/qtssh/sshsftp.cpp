#include "sshsftp.h"
#include "sshclient.h"

#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include "sshsftpcommandsend.h"
#include "sshsftpcommandget.h"
#include "sshsftpcommandreaddir.h"
#include "sshsftpcommandmkdir.h"
#include "sshsftpcommandunlink.h"
#include "sshsftpcommandfileinfo.h"

Q_LOGGING_CATEGORY(logsshsftp, "ssh.sftp", QtWarningMsg)

#define DEBUGCH qCDebug(logsshsftp) << m_name

SshSFtp::SshSFtp(const QString &name, SshClient *client):
    SshChannel(name, client)

{
    QObject::connect(client, &SshClient::sshDataReceived, this, &SshSFtp::sshDataReceived, Qt::QueuedConnection);
    QObject::connect(this, &SshSFtp::sendEvent, this, &SshSFtp::_eventLoop, Qt::QueuedConnection);
}

SshSFtp::~SshSFtp()
{
    DEBUGCH << "Free SshSFtp (destructor)";
}

void SshSFtp::close()
{
    DEBUGCH << "Close SshSFtp asked";
    if(channelState() != ChannelState::Error)
    {
        setChannelState(ChannelState::Close);
    }
    emit sendEvent();
}


QString SshSFtp::send(const QString &source, QString dest)
{
    DEBUGCH << "send(" << source << ", " << dest << ")";
    QFileInfo src(source);
    if(dest.endsWith("/"))
    {
        if(!isDir(dest))
        {
            mkpath(dest);
        }
        dest += src.fileName();
    }

    QString s(source);
    s.replace("qrc:/", ":/");

    SshSftpCommandSend cmd(s,dest,*this);
    if(processCmd(&cmd))
    {
        return dest;
    }
    return QString();
}

bool SshSFtp::get(const QString &source, QString dest, bool override)
{
    DEBUGCH << "get(" << source << ", " << dest << ", " << override << ")";
    QFileInfo src(source);
    if(dest.endsWith("/"))
    {
        dest += src.fileName();
    }


    QString original(dest);
    QFile fout(dest);
    if(!override)
    {
        if(fout.exists())
        {
            QString newpath;
            int i = 1;
            do {
                newpath = QString("%1.%2").arg(dest).arg(i);
                fout.setFileName(newpath);
                ++i;
            } while(fout.exists());
            dest = newpath;
        }
    }

    SshSftpCommandGet cmd(fout, source, *this);
    bool ret = processCmd(&cmd);

    if(!ret)
        return false;

    /* Remove file if is the same that original */
    if(dest != original)
    {
        QCryptographicHash hash1( QCryptographicHash::Md5 );
        QCryptographicHash hash2( QCryptographicHash::Md5 );
        QFile f1( original );
        if ( f1.open( QIODevice::ReadOnly ) ) {
            hash1.addData( f1.readAll() );
        }
        QByteArray sig1 = hash1.result();
        QFile f2( dest );
        if ( f2.open( QIODevice::ReadOnly ) ) {
            hash2.addData( f2.readAll() );
        }
        QByteArray sig2 = hash2.result();
        if(sig1 == sig2)
        {
            f2.remove();
        }
    }
    return true;
}

int SshSFtp::mkdir(const QString &dest, int mode)
{
    SshSftpCommandMkdir cmd(dest, mode, *this);
    DEBUGCH << "mkdir(" << dest << "," << mode << ")";
    processCmd(&cmd);
    DEBUGCH << "mkdir(" << dest << ") = " << ((cmd.error())?("FAIL"):("OK"));
    if(cmd.error()) return -1;
    return 0;
}

QStringList SshSFtp::readdir(const QString &d)
{
    SshSftpCommandReadDir cmd(d, *this);
    DEBUGCH << "readdir(" << d << ")";
    processCmd(&cmd);
    DEBUGCH << "readdir(" << d << ") = " << cmd.result();
    return cmd.result();
}

bool SshSFtp::isDir(const QString &d)
{
    DEBUGCH << "isDir(" << d << ")";
    LIBSSH2_SFTP_ATTRIBUTES fileinfo = getFileInfo(d);
    qCDebug(logsshsftp) << "isDir(" << d << ") = " << LIBSSH2_SFTP_S_ISDIR(fileinfo.permissions);
    return LIBSSH2_SFTP_S_ISDIR(fileinfo.permissions);
}

bool SshSFtp::isFile(const QString &d)
{
    DEBUGCH << "isFile(" << d << ")";
    LIBSSH2_SFTP_ATTRIBUTES fileinfo = getFileInfo(d);
    qCDebug(logsshsftp) << "isFile(" << d << ") = " << LIBSSH2_SFTP_S_ISREG(fileinfo.permissions);
    return LIBSSH2_SFTP_S_ISREG(fileinfo.permissions);
}

int SshSFtp::mkpath(const QString &dest)
{
    DEBUGCH << "mkpath(" << dest << ")";
    if(isDir(dest)) return true;
    QStringList d = dest.split("/");
    d.pop_back();
    if(mkpath(d.join("/")))
    {
        mkdir(dest);
    }
    if(isDir(dest)) return true;
    return false;
}

bool SshSFtp::unlink(const QString &d)
{
    SshSftpCommandUnlink cmd(d, *this);
    DEBUGCH << "unlink(" << d << "," << d << ")";
    processCmd(&cmd);
    DEBUGCH << "unlink(" << d << ") = " << ((cmd.error())?("FAIL"):("OK"));
    if(cmd.error()) return -1;
    return 0;
}

quint64 SshSFtp::filesize(const QString &d)
{
    DEBUGCH << "filesize(" << d << ")";
    return getFileInfo(d).filesize;
}

void SshSFtp::sshDataReceived()
{
    // Nothing to do
    _eventLoop();
}

void SshSFtp::_eventLoop()
{
    switch(channelState())
    {
        case Openning:
        {
            if ( ! m_sshClient->takeChannelCreationMutex(this) )
            {
                return;
            }
            m_sftpSession = libssh2_sftp_init(m_sshClient->session());
            m_sshClient->releaseChannelCreationMutex(this);
            if(m_sftpSession == nullptr)
            {
                char *emsg;
                int size;
                int ret = libssh2_session_last_error(m_sshClient->session(), &emsg, &size, 0);
                if(ret == LIBSSH2_ERROR_EAGAIN)
                {
                    return;
                }
                if(!m_error)
                {
                    qCWarning(logsshsftp) << "Create sftp session failed " << QString(emsg);
                    m_error = true;
                }
                m_errMsg << QString::fromUtf8(emsg, size);
                setChannelState(ChannelState::Error);
                qCWarning(logsshsftp) << "Channel session open failed";
                return;
            }
            DEBUGCH << "Channel session opened";
            setChannelState(ChannelState::Exec);
        }

        FALLTHROUGH; case Exec:
        {
            setChannelState(ChannelState::Ready);
        }


        FALLTHROUGH; case Ready:
        {
            if(m_currentCmd == nullptr && m_cmd.size() > 0)
            {
                m_currentCmd = m_cmd.first();
                DEBUGCH << "Start process next command:" << m_currentCmd->name();
                m_cmd.pop_front();
            }
            if(m_currentCmd)
            {
                DEBUGCH << "Continue process current command:" << m_currentCmd->name();
                m_currentCmd->process();
                if(m_currentCmd->state() == SshSftpCommand::CommandState::Terminate || m_currentCmd->state() == SshSftpCommand::CommandState::Error)
                {
                    DEBUGCH << "Finish process command:" << m_currentCmd->name();
                    if (m_currentCmd->state() == SshSftpCommand::CommandState::Error)
                    {
                        if (m_currentCmd->errMsg().size() > 0)
                            m_errMsg.append(m_currentCmd->errMsg());
                        m_error = true;
                    }
                    m_currentCmd = nullptr;
                    emit cmdEvent();
                    emit sendEvent();
                }
            }
            break;
        }

        case Close:
        {
            DEBUGCH << "closeChannel";
            if(libssh2_sftp_shutdown(m_sftpSession) != 0)
            {
                return;
            }
            setChannelState(ChannelState::WaitClose);
        }

        FALLTHROUGH; case WaitClose:
        {
            setChannelState(ChannelState::Freeing);
        }

        FALLTHROUGH; case Freeing:
        {
            DEBUGCH << "free Channel";
            setChannelState(ChannelState::Free);
            QObject::disconnect(m_sshClient, &SshClient::sshDataReceived, this, &SshSFtp::sshDataReceived);
            return;
        }

        case Free:
        {
            DEBUGCH << "Channel is free";
            return;
        }

        case Error:
        {
            DEBUGCH << "Channel is in error state";
            setChannelState(Free);
            return;
        }
    }
}

LIBSSH2_SFTP *SshSFtp::getSftpSession() const
{
    return m_sftpSession;
}

bool SshSFtp::processCmd(SshSftpCommand *cmd)
{
    QEventLoop wait(this);
    QObject::connect(this, &SshSFtp::stateChanged, &wait, &QEventLoop::quit);
    QObject::connect(this, &SshSFtp::cmdEvent, &wait, &QEventLoop::quit);

    m_cmd.push_back(cmd);
    emit sendEvent();
    while(channelState() <= ChannelState::Ready && cmd->state() != SshSftpCommand::CommandState::Terminate && cmd->state() != SshSftpCommand::CommandState::Error)
    {
        wait.exec();
    }
    return (cmd->state() == SshSftpCommand::CommandState::Terminate);
}

bool SshSFtp::isError()
{
    return m_error;
}

QStringList SshSFtp::errMsg()
{
    return m_errMsg;
}

LIBSSH2_SFTP_ATTRIBUTES SshSFtp::getFileInfo(const QString &path)
{
    if(!m_fileinfo.contains(path))
    {
        SshSftpCommandFileInfo cmd(path, *this);
        DEBUGCH << "fileinfo(" << path << ")";
        processCmd(&cmd);
        DEBUGCH << "fileinfo(" << path << ") = " << ((cmd.error())?("FAIL"):("OK"));
        LIBSSH2_SFTP_ATTRIBUTES fileinfo = cmd.fileinfo();
        m_fileinfo[path] = fileinfo;
    }
    return m_fileinfo[path];
}
