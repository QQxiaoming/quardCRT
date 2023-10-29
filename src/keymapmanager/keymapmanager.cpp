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
#include "keymapmanager.h"
#include "globalsetting.h"
#include "ui_keymapmanager.h"

keyMapManager::keyMapManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyMapManager)
{
    ui->setupUi(this);
    
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
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
        QString defaultKeyBinding = "default";
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
    emit this->rejected();
}

void keyMapManager::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}
