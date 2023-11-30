#ifndef KEYCHAINCLASS_H
#define KEYCHAINCLASS_H

#include <QObject>

#include <keychain.h>

class KeyChainClass: public QObject
{
    Q_OBJECT
public:
    KeyChainClass(QObject* parent = nullptr);

    bool readKey(const QString &key, QString &value);
    bool writeKey(const QString& key, const QString& value);
    bool deleteKey(const QString& key);

private:
    QKeychain::ReadPasswordJob   m_readCredentialJob;
    QKeychain::WritePasswordJob  m_writeCredentialJob;
    QKeychain::DeletePasswordJob m_deleteCredentialJob;
};

#endif // KEYCHAINCLASS_H
