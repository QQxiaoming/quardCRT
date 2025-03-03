#ifndef QTFTP_H
#define QTFTP_H

#include <QUdpSocket>
#include <QThread>

class QTftp : public QThread
{
	Q_OBJECT

public:
	explicit QTftp(QObject *parent = 0);

	void startServer();
	void stopServer();
	bool isServerRunning();
	QString getUpDir() { return upPath; }
	QString getDownDir() { return downPath; }
	void setUpDir(QString path) { upPath = path; }
	void setDownDir(QString path) { downPath = path; }
	int getPort() { return m_port; }
	void setPort(int port) { m_port = port; }
	int getRetries() { return m_retries; }
	void setRetries(int retries) { m_retries = retries; }

	void client_get(QString path, QString server);
	void client_put(QString path, QString server);

	enum Error {
		Ok,
		Timeout,
		BindError,
		FileError,
		NetworkError
	};

protected:
    void run();

signals:
	void serverRuning(bool);
	void progress(int);
	void error(int);

private:
	bool sever_running;
	QUdpSocket *sock = nullptr;
	QHostAddress rhost;
	quint16 rport;
	QString upPath;
	QString downPath;
	int m_port;
	int m_timeout;
	int m_retries;
    quint64 m_segsize;

	enum Block : quint16 {
		RRQ	= 1,	// read request
		WRQ	= 2,	// write request
		DATA	= 3,	// data packet
		ACK	= 4,	// acknowledgement
		ERR	= 5,	// error code
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
		{ EUNDEF,	 "Undefined error code" },
		{ ENOTFOUND, "File not found" },
		{ EACCESS,	 "Access violation" },
		{ ENOSPACE,	 "Disk full or allocation exceeded" },
		{ EBADOP,	 "Illegal TFTP operation" },
		{ EBADID,	 "Unknown transfer ID" },
		{ EEXISTS,	 "File already exists" },
		{ ENOUSER,	 "No such user" },
		{ -1,		 0 }
	};
#if defined(Q_CC_MSVC)
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif
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
#if defined(Q_CC_MSVC)
#pragma warning( pop )
#endif

	char buffer[9000 + sizeof(tftp_header)] = {0};

	void server();
	void server_get();
	void server_put();

	void nak(TftpError);
	void sendAck(quint16);
	bool waitForAck(quint16);
};

#endif
