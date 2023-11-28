#include <QDebug>
#include <QApplication>

#include "keychainclass.h"

KeyChainClass::KeyChainClass(QObject* parent) :
    QObject(parent),
    m_readCredentialJob(QLatin1String("keychain.quardCRT.org")),
    m_writeCredentialJob(QLatin1String("keychain.quardCRT.org")),
    m_deleteCredentialJob(QLatin1String("keychain.quardCRT.org"))
{
    m_readCredentialJob.setAutoDelete(false);
    m_writeCredentialJob.setAutoDelete(false);
    m_deleteCredentialJob.setAutoDelete(false);
}

void KeyChainClass::readKey(const QString &key, QString &value)
{
    bool waitKeyChain = true;
    m_readCredentialJob.setKey(key);
    QObject::connect(&m_readCredentialJob, &QKeychain::ReadPasswordJob::finished,this,[&](){
        if (m_readCredentialJob.error()) {
            value = "";
            qDebug() << "Read key failed: " << m_readCredentialJob.errorString();
        } else {
            value = m_readCredentialJob.textData();
        }
        waitKeyChain = false;
    });
    m_readCredentialJob.start();
    while(waitKeyChain) {
        QApplication::processEvents();
    }
    QObject::disconnect(&m_readCredentialJob,&QKeychain::ReadPasswordJob::finished,this,nullptr);
}

void KeyChainClass::writeKey(const QString &key, const QString &value)
{
    bool waitKeyChain = true;
    m_writeCredentialJob.setKey(key);
    QObject::connect(&m_writeCredentialJob, &QKeychain::WritePasswordJob::finished, [&](){
        if (m_writeCredentialJob.error()) {
            qDebug() << "Write key failed: " << m_writeCredentialJob.errorString();
        }
        waitKeyChain = false;
    });
    m_writeCredentialJob.setTextData(value);
    m_writeCredentialJob.start();
    while(waitKeyChain) {
        QApplication::processEvents();
    }
    QObject::disconnect(&m_writeCredentialJob,&QKeychain::WritePasswordJob::finished,this,nullptr);
}

void KeyChainClass::deleteKey(const QString &key)
{
    bool waitKeyChain = true;
    m_deleteCredentialJob.setKey(key);
    QObject::connect(&m_deleteCredentialJob, &QKeychain::DeletePasswordJob::finished, [&](){
        if (m_deleteCredentialJob.error()) {
            qDebug() << "Delete key failed: " << m_deleteCredentialJob.errorString();
        }
        waitKeyChain = false;
    });
    m_deleteCredentialJob.start();
    while(waitKeyChain) {
        QApplication::processEvents();
    }
    QObject::disconnect(&m_deleteCredentialJob,&QKeychain::DeletePasswordJob::finished,this,nullptr);
}
