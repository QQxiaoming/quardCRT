#ifndef QTFTP_H
#define QTFTP_H

#include <QUdpSocket>
#include <QThread>

class QTftp : public QObject
{
Q_OBJECT

private:
	QThread worker;
	QUdpSocket *sock;
	QHostAddress rhost;
	quint16 rport;
	QString upPath;
	QString downPath;

	enum Block : quint16 {
		RRQ	= 1,	// read request
		WRQ	= 2,	// write request
		DATA	= 3,	// data packet
		ACK	= 4,	// acknowledgement
		ERROR	= 5,	// error code
		OPTACK	= 6	// option acknowledgement
	};

	enum TftpError : quint16 {
		EUNDEF		= 0,	/* not defined */
		ENOTFOUND	= 1,	/* file not found */
		EACCESS		= 2,	/* access violation */
		ENOSPACE	= 3,	/* disk full or allocation exceeded */
		EBADOP		= 4,	/* illegal TFTP operation */
		EBADID		= 5,	/* unknown transfer ID */
		EEXISTS		= 6,	/* file already exists */
		ENOUSER		= 7	/* no such user */
	};

	enum Mode
	{
		ModeInvalid,	//!< Invalid mode
		NetAscii,	    //!< ASCII-mode transfer
		Octet,		    //!< Binary-mode transfer
		Mail		    //!< Mail transfer (not supported)
	};

	struct errmsg {
		int e_code;
		const char *e_msg;
	} errmsgs[9] = {
		{ EUNDEF,	"Undefined error code" },
		{ ENOTFOUND,	"File not found" },
		{ EACCESS,	"Access violation" },
		{ ENOSPACE,	"Disk full or allocation exceeded" },
		{ EBADOP,	"Illegal TFTP operation" },
		{ EBADID,	"Unknown transfer ID" },
		{ EEXISTS,	"File already exists" },
		{ ENOUSER,	"No such user" },
		{ -1,			0 }
	};

	struct tftp_header {
		quint16 opcode;
		union {
			struct {
				quint16 block;
				char data[0];
			} data;
			char path[0];
		};
	};

	char buffer[9000 + sizeof(tftp_header)];

	void server_get();
	void server_put();

	void nak(TftpError);
	void sendAck(quint16);
	bool waitForAck(quint16);

public:
	QTftp();
	void put(QString, QString);
	void get(QString, QString);
	void startServer();
	void stopServer();
	bool isRunning();
	QString getUpDir() { return upPath; }
	QString getDownDir() { return downPath; }
	void setUpDir(QString path) { upPath = path; }
	void setDownDir(QString path) { downPath = path; }
	int PORT = 69;
	int TIMEOUT = 1000;
	int RETRIES = 3;
    quint64 SEGSIZE = 512;

	enum Error {
		Ok,
		Timeout,
		BindError,
		FileError,
		NetworkError
	};

private slots:
	void client_get(QString, QString);
	void client_put(QString, QString);
	void server();

signals:
	void fileSent(QString);
	void fileReceived(QString);
	void doGet(QString, QString);
	void doPut(QString, QString);
	void doServer();
	void error(int);
	void progress(int);
	void send(bool);
};

#endif
