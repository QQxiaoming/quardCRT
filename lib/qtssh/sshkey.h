#ifndef SSHKEY_H
#define SSHKEY_H

#include <QObject>

class SshKey : public QObject
{
    Q_OBJECT

public:
    explicit SshKey(QObject *parent = nullptr);
    virtual ~SshKey() = default;

    enum Type {
        UnknownType,
        Rsa,
        Dss
    };
    Q_ENUM(Type)
    QByteArray hash;
    QByteArray key;
    Type       type {UnknownType};
};

#endif // SSHKEY_H
