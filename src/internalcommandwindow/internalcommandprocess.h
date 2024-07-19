#ifndef INTERNALCOMMANDPROCESS_H
#define INTERNALCOMMANDPROCESS_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class InternalCommandProcess : public QThread
{
    Q_OBJECT
public:
    explicit InternalCommandProcess(QObject *parent = nullptr);
    ~InternalCommandProcess();

    void recvData(const QByteArray &data);

signals:
    void sendData(const QByteArray &data);
    void showString(const QString &name, const QString &str);
    void showEasterEggs(void);

protected:
    void run(void);

private:
    void sendString(const QString &str);
    void sendPrompt(void);
    void processLine(const QString &line);
    QString promptLine = ">>> ";
    QStringList historyCmdList;
    int32_t historyCmdIndex = 0;

    QByteArray buffer;
    QMutex mutex;
    QWaitCondition condition;
    bool exit;
};

#endif // INTERNALCOMMANDPROCESS_H
