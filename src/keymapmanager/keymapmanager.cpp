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
#include <QFile>
#include <QFileInfo>
#include "keymapmanager.h"
#include "globalsetting.h"
#include "ui_keymapmanager.h"

#if defined(Q_OS_WIN)
#if defined(Q_CC_MSVC)
QString keyMapManager::defaultKeyBinding = "windows_conpty";
#else
QString keyMapManager::defaultKeyBinding = "windows_winpty";
#endif
#elif defined(Q_OS_MAC)
QString keyMapManager::defaultKeyBinding = "macos_default";
#else
QString keyMapManager::defaultKeyBinding = "linux_default";
#endif

keyMapManager::keyMapManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyMapManager)
{
    ui->setupUi(this);
    
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &keyMapManager::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &keyMapManager::buttonBoxRejected);

    connect(ui->comboBoxKeyBinding, &QComboBox::currentTextChanged, this, [this](const QString &keyBinding) {
        QString path = ":/lib/qtermwidget/kb-layouts/"+keyBinding+".keytab";
        QFileInfo fileInfo(path);
        if(fileInfo.exists()) {
            QString content;
            QFile file(path);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                content = in.readAll();
                file.close();
                ui->plainTextEditKeyBinding->setPlainText(content);
            }
        }
    });
}

keyMapManager::~keyMapManager()
{
    delete ui;
}

void keyMapManager::setAvailableKeyBindings(QStringList keyBindings)
{
    ui->comboBoxKeyBinding->clear();
    ui->comboBoxKeyBinding->addItems(keyBindings);

    GlobalSetting settings;
    settings.beginGroup("Global/keyMapManager");
    if((settings.contains("keyBinding")) &&(keyBindings.contains(settings.value("keyBinding").toString()))) {
        ui->comboBoxKeyBinding->setCurrentText(settings.value("keyBinding").toString());
    } else {
        ui->comboBoxKeyBinding->setCurrentText(defaultKeyBinding);
        settings.setValue("keyBinding", defaultKeyBinding);
    }
    settings.endGroup();
}

QString keyMapManager::getCurrentKeyBinding(void)
{
    return ui->comboBoxKeyBinding->currentText();
}

void keyMapManager::buttonBoxAccepted(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/keyMapManager");
    settings.setValue("keyBinding", ui->comboBoxKeyBinding->currentText());
    settings.endGroup();
    emit keyBindingChanged(ui->comboBoxKeyBinding->currentText());
    emit this->accepted();
}

void keyMapManager::buttonBoxRejected(void)
{
    GlobalSetting settings;
    ui->comboBoxKeyBinding->setCurrentText(settings.value("Global/keyMapManager/keyBinding", defaultKeyBinding).toString());
    emit this->rejected();
}

void keyMapManager::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}
