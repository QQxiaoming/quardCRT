#ifndef QSENDKERMIT_H
#define QSENDKERMIT_H

#include <QDebug>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include "libkermit.h"

class QSendKermit : public QThread {
    Q_OBJECT

public:
    explicit QSendKermit(int32_t timeout = -1, QObject *parent = nullptr);
    ~QSendKermit(){
        requestStop();
        wait();
    };
    void setFilePathList(QStringList filePathList);
    void requestStop(void) {
        m_abort = true;
        condition.wakeOne();
    }
    bool getStopFlag(void) {
        return m_abort;
    }

signals:
    void sendData(QByteArray data);

public slots:
    void onRecvData(const QByteArray &data) {
        QMutexLocker locker(&mutex);
        dataRecv.append(data);
        condition.wakeOne();
    }

protected:
    void run(void);

private:
    msg* receive_message_timeout(int timeout) {
        bool ok = false;
        mutex.lock();
        size_t size = dataRecv.size();
        if(size >= sizeof(msg)) {
            ok = true;
        } else {
            if(timeout<0) {
            retry:
                condition.wait(&mutex);
                if(getStopFlag()) {
                    mutex.unlock();
                    return nullptr;
                }
                size = dataRecv.size();
                if(size < sizeof(msg)) {
                    goto retry;
                }
            } else {
                QDeadlineTimer timer(timeout);
                do {
                    condition.wait(&mutex, timer.remainingTime());
                    if(getStopFlag()) {
                        mutex.unlock();
                        return nullptr;
                    }
                    size = dataRecv.size();
                } while(size < sizeof(msg) && timer.remainingTime() > 0);
                if(size >= sizeof(msg)) {
                    ok = true;
                }
            }
        }
        if(ok) {
            msg* ret = (msg*) malloc(sizeof (msg));
            memcpy(ret,dataRecv.data(),sizeof (msg));
            dataRecv.remove(0,sizeof (msg));
            mutex.unlock();
            return ret;
        }
        mutex.unlock();
        return nullptr;
    }
    int send_message(const msg* m) {
        emit sendData(QByteArray((const char*)m,sizeof (msg)));
        return sizeof (msg);
    }
    int send_to_receiver(int seq, char *data, int size, char type);

private:
    QByteArray dataRecv;
    QMutex mutex;
    QWaitCondition condition;

    int32_t timeout;
    QStringList filePathList;
    bool m_abort = false;
};

#endif // QSENDKERMIT_H
