#include "qsendkermit.h"

QSendKermit::QSendKermit(int32_t timeout, QObject *parent) : QThread(parent) {
    this->timeout = timeout;
}

void QSendKermit::setFilePathList(QStringList filePathList) {
    this->filePathList = filePathList;
}

void QSendKermit::run(void) {
    int seq = 0;
    int count_packages, current_package, rest, error_counter = 0, result;
	msg *receive_message = NULL;
	while (receive_message == NULL && error_counter < 3) {
        msg init;
        send_init(&init,seq);
	    send_message(&init);

		/* Receive ACK for init package. */
		receive_message = receive_message_timeout(timeout * 1000);
		if (receive_message == NULL) {
			qDebug("[ksender] Timeout error.");
			error_counter++;
		} else if (receive_message->payload[3] == 'Y') {
        	qDebug("[ksender] Got reply with payload: %d", receive_message->payload[2]);
    	} else {
            qDebug("[ksender] NACK received with seq: %d", receive_message->payload[2]);
			seq = (seq + 1) % 64;
			error_counter++;
			receive_message = NULL;
        }
    }

	if (receive_message == NULL) {
        qDebug("[ksender] Too many errors. End of transmission.");
		return;
	}

	seq = (seq + 1) % 64;

	foreach (QString filePath, filePathList) {
        QString fileName = QFileInfo(filePath).fileName();
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file: " << filePath;
            return;
        }
        QByteArray filedata = file.readAll();
        int file_size = filedata.size();
        char *data = filedata.data();
        file.close();

        QByteArray fileNameData(fileName.toUtf8());
		result = send_to_receiver(seq, fileNameData.data(), fileNameData.size(), 'F');
        if(result == -1) {
            return;
		}
		seq = (result + 1) % 64;
        count_packages = file_size / MAXL;
		current_package = 0;	

		while (current_package < count_packages) {
			result = send_to_receiver(seq, data + current_package * MAXL, MAXL, 'D');
		
			if(result == -1) {
                return;
			}
			
			seq = (result + 1) % 64;
			current_package++;
		}
        rest = file_size - count_packages * MAXL;
		result = send_to_receiver(seq, data + count_packages * MAXL, rest, 'D');
		
		if (result == -1) {
            return;
		}
			
		seq = (result + 1) % 64;
		
		/* Send EOF package. */
        QByteArray dummy("");
        result = send_to_receiver(seq, dummy.data(), 0, 'Z');
		
		if (result == -1) {
            return;
		}

		seq = (result + 1) % 64;
	}

    /* Send EOT package. */
    QByteArray dummy("");
    result = send_to_receiver(seq, dummy.data(), 0, 'B');
		
	if(result == -1) {
		qDebug("[ksender] Error sending EOT package.");
	}

	qDebug("[ksender] End of transmission.");
}

int QSendKermit::send_to_receiver(int seq, char *data, int size, char type) {
	int error_counter = 0, expected_seq = 0;
	msg *receive_message = NULL;

	while (error_counter < 3 && (receive_message == NULL || seq != expected_seq)) {
        msg init;
		send_package(&init, data, size, seq, type);
	    send_message(&init);

		receive_message = receive_message_timeout(TIME * 1000);

		if (receive_message == NULL) {
			qDebug("[ksender] Didn't receive the message. Timeout error.");
			error_counter++;

		} else if (receive_message->payload[3] == 'Y') {
			expected_seq = receive_message->payload[2];

			if (expected_seq != seq) {
				qDebug("[ksender] Didn't receive the correct message. Send again.");
				error_counter++;
			} else {
				qDebug("[ksender] ACK received with seq: %d", seq);
			}	
		} else {
			qDebug("[ksender] NACK received with seq: %d", receive_message->payload[2]);
			seq = (seq + 1) % 64;

			error_counter++;
			receive_message = NULL;
		}
	}
			
	if (receive_message == NULL) {
		qDebug("[ksender] Too many errors. End of transmission.");
		return -1;
	}
			
	return seq;
}

