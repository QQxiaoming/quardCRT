#include <QtEndian>
#include <QFile>
#include <QFileInfo>
#include <cstdio>

#include "qtftp.h"

QTftp::QTftp(QObject *parent) :
	QThread(parent),sever_running(false),m_port(69),m_timeout(1000),m_retries(3),m_segsize(512)
{
}

void QTftp::startServer()
{
	sever_running = true;
	start();
}

void QTftp::stopServer()
{
	sever_running = false;
	wait();
}

bool QTftp::isServerRunning()
{
	return sever_running;
}

void QTftp::run() {
	server();
}

void QTftp::server()
{
	sock = new QUdpSocket();
	if(!sock->bind(m_port)) {
		sock->close();
		delete sock;
		sever_running = false;
		emit error(BindError);
		emit serverRuning(false);
		return;
	}
	while(sever_running) {
		emit serverRuning(true);
		m_segsize = 512;
		m_timeout = 1000;
		memset(buffer, 0, m_segsize + sizeof(struct tftp_header));
		sock->waitForReadyRead(1000);
		qint64 readed = sock->readDatagram(buffer, m_segsize + sizeof(struct tftp_header), &rhost, &rport);
		if(readed < 0)
			continue;

		struct tftp_header *th = (struct tftp_header *)buffer;
		switch(qFromBigEndian(th->opcode)) {
		case RRQ:
			server_get();
			break;
		case WRQ:
			server_put();
			break;
		default: 
			nak(EBADOP);
			break;
		}
	}
	sock->close();
	delete sock;
	emit serverRuning(false);
}

void QTftp::server_get()
{
	int mode = Octet;
	struct tftp_header *th = (struct tftp_header *)buffer;
	QStringList info;
    int offset = 0;
	do {
        QString text = QString(&th->path[offset]);
		info.append(text);
        if(th->path[text.length() + 1 + offset] == 0)
			break;
		else 
            offset += text.length() + 1;
	} while (true);
	if(info.contains("octect")) mode = Octet;
	else if(info.contains("netascii")) mode = NetAscii;
	else if(info.contains("mail")) mode = Mail;
	if(info.contains("timeout")) {
		m_timeout = info.at(info.indexOf("timeout") + 1).toInt()*1000;
	}
	if(info.contains("blksize")) {
		m_segsize = info.at(info.indexOf("blksize") + 1).toInt();
	}
    qDebug("mode: %d, timeout: %d, blksize: %llu", mode, m_timeout, m_segsize);
	QString path = getUpDir() + "/" + info.at(0);
    qDebug() << "sending: " << path;
	QFileInfo fileInfo(path);
	if(!fileInfo.exists()) {
		nak(ENOTFOUND);
		return;
	}
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly))
		switch (file.error()) {
		case QFile::OpenError:
			nak(ENOTFOUND);
			return;
		case QFile::PermissionsError:
			nak(EACCESS);
			return;
		default:
			nak(EUNDEF);
			return;
		}

	quint16 block = 0;
	th->opcode = qToBigEndian((quint16)OPTACK);
    int pktnum = snprintf(th->path,512,"timeout%c%d%cblksize%c%llu%c",0,m_timeout/1000,0,0,m_segsize,0);
	int i;
	for(i = 0; i < m_retries; i++) {
		sock->writeDatagram(buffer, sizeof(struct tftp_header) + pktnum-2, rhost, rport);
		if(waitForAck(block)) {
			block++;
			break;
		}
	}
	if(i == m_retries)
		return;

	quint64 readed;
	do {
		qint64 blocks = file.size() / m_segsize;
		int percent = -1;
		
		th->opcode = qToBigEndian((quint16)DATA);
		th->data.block = qToBigEndian((quint16)block);
		readed = file.read(buffer + sizeof(struct tftp_header), m_segsize);

		for(i = 0; i < m_retries; i++) {
			sock->writeDatagram(buffer, readed + sizeof(struct tftp_header), rhost, rport);
			if(waitForAck(block)) {
				block++;
				break;
			}
		}
		if(i == m_retries)
			return;

		int newp = block * 100 / blocks;
		if(newp > percent) {
			percent = newp;
			emit progress(newp);
		}
	} while(readed == m_segsize);
	waitForAck(block-1);
	qDebug("sent %d blocks, %llu bytes", (block - 1), (block - 2) * m_segsize + readed);
}

void QTftp::server_put()
{
	struct tftp_header *th = (struct tftp_header *)buffer;
	QStringList info;
    int offset = 0;
	do {
        QString text = QString(&th->path[offset]);
		info.append(text);
        if(th->path[text.length() + 1 + offset] == 0)
			break;
		else 
            offset += text.length() + 1;
	} while (true);
	QString path = getDownDir() + "/" + info.at(0);
    qDebug() << "receiving: " << path;
	QFile file(path);
	if(!file.open(QIODevice::WriteOnly))
		switch (file.error()) {
		case QFile::PermissionsError:
			nak(EACCESS);
			return;
		default:
			nak(EUNDEF);
			return;
		}

	sendAck(0);
    quint64 received = 0;
	quint16 block = 1;
	int i;
	do {
		for(i = 0; i < m_retries; i++) {
			QHostAddress h;
			quint16 p;
			if(!sock->waitForReadyRead(m_timeout))
				return;
			received = sock->readDatagram(buffer, m_segsize + sizeof(struct tftp_header), &h, &p);

			if(h != rhost || p != rport)
				continue;

			if(th->opcode == qToBigEndian((quint16)DATA) && qFromBigEndian(th->data.block) == block)
				break;
		}
		if(i == m_retries)
			return;
		file.write(buffer + sizeof(struct tftp_header), received - sizeof(struct tftp_header));
		sendAck(block++);
	} while (received == m_segsize + sizeof(struct tftp_header));
	qDebug("received %d blocks, %llu bytes", block - 1, (block - 2) * m_segsize + received);
}

void QTftp::client_get(QString path, QString server)
{
	qDebug("receiving %s", path.toUtf8().constData());
	QFile file(path);
	if(!file.open(QIODevice::WriteOnly))
		return;

	QFileInfo name(path);
	m_segsize = 512;
	m_timeout = 1000;
	struct tftp_header *th = (struct tftp_header *)buffer;
	strcpy(th->path, name.fileName().toUtf8().constData());
	strcpy(th->path + name.fileName().length() + 1, "octect");

	sock = new QUdpSocket(this);
	sock->bind();

	th->opcode = qToBigEndian((quint16)RRQ);

	if(sock->writeDatagram(buffer, sizeof(struct tftp_header) + name.fileName().length() + sizeof("octect") - 1, QHostAddress(server), m_port) <= 0) {
		emit error(NetworkError);
		return;
	}

    quint64 readed;
	quint16 block = 1;
	do {
		if(!sock->waitForReadyRead(m_timeout)) {
			emit error(Timeout);
			return;
		}
		readed = sock->readDatagram(buffer, m_segsize + (sizeof(struct tftp_header)), &rhost, &rport);

		file.write(th->data.data, readed - sizeof(struct tftp_header));
		sendAck(block++);
	} while (readed == m_segsize + sizeof(tftp_header));
	qDebug("received %d blocks, %llu bytes", block - 1, (block - 2) * m_segsize + readed);
	sock->close();
	delete sock;
}

void QTftp::client_put(QString path, QString server)
{
	qDebug("sending %s", path.toUtf8().constData());
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly))
		return;

	QFileInfo name(path);
	m_segsize = 512;
	m_timeout = 1000;
	struct tftp_header *th = (struct tftp_header *)buffer;
	strcpy(th->path, name.fileName().toUtf8().constData());
	strcpy(th->path + name.fileName().length() + 1, "octect");

	sock = new QUdpSocket(this);
	sock->bind();

	th->opcode = qToBigEndian((quint16)WRQ);

	int i;
	for(i = 0; i < m_retries; i++) {
		if(sock->writeDatagram(buffer, sizeof(struct tftp_header) + name.fileName().length() + sizeof("octect") - 1, QHostAddress(server), m_port) <= 0) {
			emit error(NetworkError);
			return;
		}
		rhost.clear();
		rport = 0;
		if(waitForAck(0))
			break;
	}
	if(i == m_retries) {
		emit error(Timeout);
		return;
	}

	quint64 readed;
	quint16 block = 1;
	do {
		qint64 blocks = file.size() / 512;
		int percent = -1;

		th->opcode = qToBigEndian((quint16)DATA);
		th->data.block = qToBigEndian((quint16)block);
		readed = file.read(buffer + sizeof(struct tftp_header), m_segsize);

		for(i = 0; i < m_retries; i++) {
			if(sock->writeDatagram(buffer, readed + sizeof(struct tftp_header), rhost, rport) <= 0) {
				emit error(NetworkError);
				return;
			}
			if(waitForAck(block)) {
				block++;
				break;
			}
		}
		if(i == m_retries) {
			emit error(Timeout);
			return;
		}
		int newp = block * 100 / blocks;
		if(newp > percent) {
			percent = newp;
			emit progress(newp);
		}
	} while(readed == m_segsize);
	qDebug("sent %d blocks, %llu bytes", (block - 1), (block - 2) * m_segsize + readed);
	sock->close();
	delete sock;
}


bool QTftp::waitForAck(quint16 block)
{
	for(int i = 0; i < m_retries; i++) {
		struct tftp_header th;
		QHostAddress h;
		quint16 p;

		if(!sock->waitForReadyRead(m_timeout))
			continue;

		sock->readDatagram((char *)&th, sizeof(struct tftp_header), &h, &p);
		
		if(rhost.isNull() && rport == 0) {
			rhost = h;
			rport = p;
		} else if(h != rhost || p != rport)
			continue;
		
		if(th.opcode == qToBigEndian((quint16)ACK) && qFromBigEndian(th.data.block) == block)
			return true;
	}
	return false;
}

void QTftp::sendAck(quint16 block)
{
	struct tftp_header ack;
	ack.opcode = qToBigEndian((quint16)ACK);
	ack.data.block = qToBigEndian(block);
	sock->writeDatagram((char*)&ack, sizeof(struct tftp_header), rhost, rport);
}

void QTftp::nak(TftpError error)
{
	struct tftp_header *th = (struct tftp_header *)buffer;
	th->opcode = qToBigEndian((quint16)ERR);
	th->data.block = qToBigEndian((quint16)error);

	struct errmsg *pe;
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = strerror(error - 100);
		th->data.block = EUNDEF;   /* set 'undef' errorcode */
	}

	strcpy(th->data.data, pe->e_msg);
	int length = strlen(pe->e_msg);
	th->data.data[length] = 0;
	length += 5;
	sock->writeDatagram(buffer, length, rhost, rport);
}
