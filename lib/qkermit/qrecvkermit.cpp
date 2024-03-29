#include <QDir>
#include "qrecvkermit.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
#include <utime.h>
#include <sys/stat.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#if defined(Q_CC_MSVC)
#define PATH_MAX _MAX_PATH
#endif
#endif

QRecvKermit::QRecvKermit(int32_t timeout, QObject *parent) : QThread(parent) {
    this->timeout = timeout;
	m_fileDirPath = QDir::homePath();
}

void QRecvKermit::run(void) {
    msg *received_message = NULL;
	int data_size, time;
	unsigned char error_counter = 0, seq = 0;
	unsigned short crc, receive_crc;
	char last_message;
	char file_name[PATH_MAX];
    QFile *file = nullptr;

	while (error_counter < 3 && received_message == NULL) {
        received_message =  receive_message_timeout(timeout*1000);

        if (received_message == NULL) {
			if(getStopFlag()) {
				return;
			}
        	qDebug("Didn't receive init message. Timeout error. End of transmission.");
			error_counter++;
		} else if (seq != received_message->payload[2]) {
			error_counter++;
		}else {
			crc = crc16_ccitt(received_message->payload, received_message->len - 3);
			memcpy(&receive_crc, &received_message->payload[received_message->len - 3], 2);
	
			if (crc == receive_crc) {
                msg init;
                send_package(&init, nullptr, 0, seq, 'Y');
	            send_message(&init);
				last_message = 'Y';
			} else {
                msg init;
                send_package(&init, nullptr, 0, seq, 'N');
	            send_message(&init);
				last_message = 'N';

				seq = (seq + 1) % 64;
				error_counter++;
				received_message = NULL;
			}
		}
	}

	if (received_message == NULL) {
		qDebug("[kreceiver] Too many errors. End of transmission.");
		return;
	}

	time = received_message->payload[5] * 1000;

    /* Receive file data */
	while (1) {
		seq = (seq + 1) % 64;
		error_counter = 0;
		received_message = NULL;

		while (received_message == NULL) {
			received_message = receive_message_timeout(time);

			if (received_message == NULL) {
				if(getStopFlag()) {
					if(file != nullptr) {
						file->close();
						delete file;
						file = nullptr;
					}
					return;
				}
				error_counter++;
			
				if (error_counter == 3) {
					qDebug("[kreceiver] Too many errors. End of transmission.");
					if(file != nullptr) {
						file->close();
						delete file;
						file = nullptr;
					}
					return;
				}
				
				/* Send last message */
				if (last_message == 'N') {
                    msg init;
                    send_package(&init, nullptr, 0, seq - 1, last_message);
	                send_message(&init);
					seq = (seq + 1) % 64;
				} else {
                    msg init;
                    send_package(&init, nullptr, 0, seq - 1, last_message);
	                send_message(&init);
				}
    			
			} else if (received_message->payload[2] != seq) {
				error_counter++;

				if (error_counter == 3) {
					qDebug("[kreceiver] Too many errors. End of transmission.");
                    if(file != nullptr) {
                        file->close();
                        delete file;
                        file = nullptr;
                    }
					return;
				}
				
				/* Send last message */
				if (last_message == 'N') {
                    msg init;
                    send_package(&init,nullptr, 0, seq, last_message);
	                send_message(&init);
					seq = (seq + 1) % 64;
				} else {
                    msg init;
                    send_package(&init,nullptr, 0, seq - 1, last_message);
	                send_message(&init);
				}
			
				received_message = NULL;

			} else {
				/* Check the message */
				crc = crc16_ccitt(received_message->payload, received_message->len - 3);
				memcpy(&receive_crc, &received_message->payload[received_message->len - 3], 2);

				if (crc == receive_crc) {
					qDebug("[kreceiver] Received message with seq: %d", received_message->payload[2]);
					/* Send ACK */
					msg init;
                    send_package(&init, nullptr, 0, seq, 'Y');
	                send_message(&init);
					last_message = 'Y';
					/* Open file if packege is header */
					if (received_message->payload[3] == 'F') {
						memcpy(file_name , &received_message->payload[4], received_message->payload[1] - 5);
						file_name[received_message->payload[4] + 1] = '\0';
					
                        if(file != nullptr) {
                            file->close();
                            delete file;
                            file = nullptr;
                        }
                        file = new QFile(m_fileDirPath+QDir::separator()+QString(file_name));
                        if (!file->open(QIODevice::WriteOnly)) {
                            qDebug("Error opening file");
                            return;
                        }
					}
					
					/* Copy data in file */
					if (received_message->payload[3] == 'D') {
						data_size = (unsigned char) received_message->payload[1] - 5;
                        file->write(&received_message->payload[4], data_size);
					}

					/* Close the file */
					if (received_message->payload[3] == 'Z') {
                        file->close();
                        delete file;
                        file = nullptr;
					}

					/* End transmission */
					if (received_message->payload[3] == 'B') {
						qDebug("[kreceiver] End of transmission.");
                        if(file != nullptr) {
                            file->close();
                            delete file;
                            file = nullptr;
                        }
						return;
					}

				} else {
					error_counter++;
					received_message = NULL;

					if (error_counter == 3) {
						qDebug("[kreceiver] Too many errors. End of transmission.");
                        if(file != nullptr) {
                            file->close();
                            delete file;
                            file = nullptr;
                        }
						return;
					}

					/* Send NACK */
					msg init;
                    send_package(&init, nullptr, 0, seq, 'N');
	                send_message(&init);
					last_message = 'N';
					seq = (seq + 1) % 64;
				}
			}
		}
	}
}
