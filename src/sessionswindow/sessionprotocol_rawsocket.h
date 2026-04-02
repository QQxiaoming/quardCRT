/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */
#ifndef SESSIONPROTOCOL_RAWSOCKET_H
#define SESSIONPROTOCOL_RAWSOCKET_H

#include "sessionprotocol.h"
#include "sessionswindow.h"

class QRawSocket : public QObject
{
    Q_OBJECT
public:
    QRawSocket(QObject *parent = nullptr)
        : QObject(parent){}
    ~QRawSocket() {
        disconnectFromHost();
        switch (m_rawMode) {
        case 0:
            delete tcpSocket;
            break;
        case 1:
            if(tcpSocket) {
                delete tcpSocket;
            }
            delete tcpServer;
            break;
        case 2:
        case 3:
            delete udpSocket;
            break;
        default:
            break;
        }
    }
    int setRawMode(int mode) {
        if(m_rawMode == -1) {
            switch (mode) {
            case 0:
                tcpSocket = new QTcpSocket(this);
                connect(tcpSocket,&QTcpSocket::readyRead,this,&QRawSocket::readyRead);
                connect(tcpSocket,&QTcpSocket::errorOccurred,this,&QRawSocket::errorOccurred);
                connect(tcpSocket,&QTcpSocket::stateChanged,this,&QRawSocket::stateChanged);
                break;
            case 1:
                tcpServer = new QTcpServer(this);
                connect(tcpServer, &QTcpServer::newConnection, [&](){
                    if (tcpSocket) {
                        // If there is already a connection, reject the new one
                        QTcpSocket *newSocket = tcpServer->nextPendingConnection();
                        newSocket->disconnectFromHost();
                        newSocket->deleteLater();
                        return;
                    }
                    tcpSocket = tcpServer->nextPendingConnection();
                    connect(tcpSocket,&QTcpSocket::readyRead,this,&QRawSocket::readyRead);
                    connect(tcpSocket,&QTcpSocket::errorOccurred,this,&QRawSocket::errorOccurred);
                    connect(tcpSocket,&QTcpSocket::stateChanged,this,&QRawSocket::stateChanged);
                    emit stateChanged(tcpSocket->state());
                });
                break;
            case 2:
                udpSocket = new QUdpSocket(this);
                connect(udpSocket,&QUdpSocket::errorOccurred,this,&QRawSocket::errorOccurred);
                break;
            case 3:
                udpSocket = new QUdpSocket(this);
                connect(udpSocket,&QUdpSocket::readyRead,this,&QRawSocket::readyRead);
                connect(udpSocket,&QUdpSocket::errorOccurred,this,&QRawSocket::errorOccurred);
                break;
            default:
                return -1;
            }
            m_rawMode = mode;
            return 0;
        }
        return -1;
    }
    int getRawMode() const { return m_rawMode; }
    void connectToHost(const QString &hostName, quint16 port) {
        switch(m_rawMode) {
        case 0:
            tcpSocket->connectToHost(hostName,port);
            return;
        case 1:
            tcpServer->listen(QHostAddress(hostName),port);
            return;
        case 2:
            udpSocket->connectToHost(hostName,port);
            udpstate = QAbstractSocket::ConnectedState;
            emit stateChanged(udpstate);
            return;
        case 3:
            udpSocket->bind(QHostAddress(hostName),port);
            udpstate = QAbstractSocket::ConnectedState;
            emit stateChanged(udpstate);
            return;
        }
    }
    void disconnectFromHost() {
        switch(m_rawMode) {
        case 0:
            tcpSocket->disconnectFromHost();
            return;
        case 1:
            if(tcpSocket)
                tcpSocket->close();
            tcpServer->close();
            return;
        case 2:
            udpSocket->disconnectFromHost();
            udpstate = QAbstractSocket::ClosingState;
            emit stateChanged(udpstate);
            return;
        case 3:
            udpSocket->close();
            udpstate = QAbstractSocket::ClosingState;
            emit stateChanged(udpstate);
            return;
        }
    }
    QByteArray readAll() {
        switch(m_rawMode) {
        case 0:
            return tcpSocket->readAll();
        case 1:
            if(tcpSocket)
                return tcpSocket->readAll();
            return QByteArray();
        case 3:
            QByteArray data;
            while (udpSocket->hasPendingDatagrams()) {
                QNetworkDatagram datagram = udpSocket->receiveDatagram();
                data.append(datagram.data());
            }
            return data;
        }
        return QByteArray();
    }
    qint64 write(const QByteArray &data) {
        switch(m_rawMode) {
        case 0:
            return tcpSocket->write(data);
        case 1:
            if(tcpSocket)
                return tcpSocket->write(data);
            return 0;
        case 2:
            return udpSocket->write(data);
        }
        return 0;
    }
    qint64 write(const char *data, qint64 len) {
        return this->write(QByteArray(data,len));
    }
    QAbstractSocket::SocketState state() const {
        switch(m_rawMode) {
        case 0:
            return tcpSocket->state();
        case 1:
            if(tcpSocket)
                return tcpSocket->state();
            return QAbstractSocket::UnconnectedState;
        case 2:
        case 3:
            return udpstate;
        }
        return QAbstractSocket::UnconnectedState;
    }
    QString errorString() const {
        switch(m_rawMode) {
        case 0:
            return tcpSocket->errorString();
        case 1:
            if(tcpSocket)
                return tcpSocket->errorString();
            return QString();
        case 2:
        case 3:
            return udpSocket->errorString();
        }
        return QString();
    }

signals:
    void readyRead();
    void errorOccurred(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState);

private:
    QTcpSocket *tcpSocket = nullptr;
    QTcpServer *tcpServer = nullptr;
    QUdpSocket *udpSocket = nullptr;
    int m_rawMode = -1;
    QAbstractSocket::SocketState udpstate = QAbstractSocket::ClosingState;
};

#endif // SESSIONPROTOCOL_RAWSOCKET_H
