/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
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

bool KeyChainClass::readKey(const QString &key, QString &value)
{
    bool ret = false;
    bool waitKeyChain = true;
    m_readCredentialJob.setKey(key);
    QObject::connect(&m_readCredentialJob, &QKeychain::ReadPasswordJob::finished,this,[&](){
        if (m_readCredentialJob.error()) {
            value = "";
            ret = false;
            qDebug() << "Read key failed: " << m_readCredentialJob.errorString();
        } else {
            value = m_readCredentialJob.textData();
            ret = true;
        }
        waitKeyChain = false;
    });
    m_readCredentialJob.start();
    while(waitKeyChain) {
        QApplication::processEvents();
    }
    QObject::disconnect(&m_readCredentialJob,&QKeychain::ReadPasswordJob::finished,this,nullptr);
    return ret;
}

bool KeyChainClass::writeKey(const QString &key, const QString &value)
{
    bool ret = false;
    bool waitKeyChain = true;
    m_writeCredentialJob.setKey(key);
    QObject::connect(&m_writeCredentialJob, &QKeychain::WritePasswordJob::finished, [&](){
        if (m_writeCredentialJob.error()) {
            ret = false;
            qDebug() << "Write key failed: " << m_writeCredentialJob.errorString();
        } else {
            ret = true;
        }
        waitKeyChain = false;
    });
    m_writeCredentialJob.setTextData(value);
    m_writeCredentialJob.start();
    while(waitKeyChain) {
        QApplication::processEvents();
    }
    QObject::disconnect(&m_writeCredentialJob,&QKeychain::WritePasswordJob::finished,this,nullptr);
    return ret;
}

bool KeyChainClass::deleteKey(const QString &key)
{
    bool ret = false;
    bool waitKeyChain = true;
    m_deleteCredentialJob.setKey(key);
    QObject::connect(&m_deleteCredentialJob, &QKeychain::DeletePasswordJob::finished, [&](){
        if (m_deleteCredentialJob.error()) {
            ret = false;
            qDebug() << "Delete key failed: " << m_deleteCredentialJob.errorString();
        } else {
            ret = true;
        }
        waitKeyChain = false;
    });
    m_deleteCredentialJob.start();
    while(waitKeyChain) {
        QApplication::processEvents();
    }
    QObject::disconnect(&m_deleteCredentialJob,&QKeychain::DeletePasswordJob::finished,this,nullptr);
    return ret;
}
