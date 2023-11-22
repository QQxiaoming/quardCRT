#include "sshchannel.h"
#include "sshclient.h"
#include <QCoreApplication>

Q_LOGGING_CATEGORY(sshchannel, "ssh.channel", QtWarningMsg)

SshChannel::SshChannel(QString name, SshClient *client)
    : QObject(client)
    , m_sshClient(client)
    , m_name(name)
{
    qCDebug(sshchannel) << "createChannel:" << m_name;
    QObject::connect(m_sshClient, &SshClient::sshDataReceived, this, &SshChannel::sshDataReceived, Qt::QueuedConnection);
}

SshChannel::~SshChannel()
{
    qCDebug(sshchannel) << "destroyChannel:" << this;
    setChannelState(ChannelState::Free);
}

SshClient *SshChannel::sshClient() const
{
    return m_sshClient;
}

SshChannel::ChannelState SshChannel::channelState() const
{
    return m_channelState;
}

void SshChannel::setChannelState(const ChannelState &channelState)
{
    if(m_channelState != channelState)
    {
        qCDebug(sshchannel)  << m_name << "Change State:" << m_channelState << "->" << channelState;
        m_channelState = channelState;
        emit stateChanged(m_channelState);
    }
}

bool SshChannel::waitForState(SshChannel::ChannelState state)
{
    QEventLoop wait(this);
    QObject::connect(this, &SshChannel::stateChanged, &wait, &QEventLoop::quit);
    while(channelState() != ChannelState::Error && channelState() != ChannelState::Free && channelState() !=state)
    {
        wait.exec();
    }
    return channelState() == state;
}


QString SshChannel::name() const
{
    return m_name;
}
