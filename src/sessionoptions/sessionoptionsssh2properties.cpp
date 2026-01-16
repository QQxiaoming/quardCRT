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
#include "sessionoptionsssh2properties.h"
#include "ui_sessionoptionsssh2properties.h"
#include <QFileDialog>

SessionOptionsSsh2Properties::SessionOptionsSsh2Properties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsSsh2Properties)
{
    ui->setupUi(this);
    lineEditPassword = new PasswordEdit(true, this);
    ui->horizontalLayoutPassword->addWidget(lineEditPassword);
    lineEditPassphrase = new PasswordEdit(true, this);
    ui->horizontalLayoutPassphrase->addWidget(lineEditPassphrase);

    auto updateAuthFields = [this]() {
        bool usePublicKey = (ui->comboBoxAuthMethod->currentIndex() == 1);
        ui->label_4->setVisible(!usePublicKey);
        lineEditPassword->setVisible(!usePublicKey);
        ui->labelPrivateKey->setVisible(usePublicKey);
        ui->lineEditPrivateKey->setVisible(usePublicKey);
        ui->toolButtonBrowsePrivateKey->setVisible(usePublicKey);
        ui->labelPublicKey->setVisible(usePublicKey);
        ui->lineEditPublicKey->setVisible(usePublicKey);
        ui->toolButtonBrowsePublicKey->setVisible(usePublicKey);
        ui->labelPassphrase->setVisible(usePublicKey);
        lineEditPassphrase->setVisible(usePublicKey);
    };

    connect(ui->comboBoxAuthMethod, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int) { updateAuthFields(); });
    connect(ui->toolButtonBrowsePrivateKey, &QToolButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(this, tr("Select Private Key"), ui->lineEditPrivateKey->text());
        if(!path.isEmpty()) {
            ui->lineEditPrivateKey->setText(path);
        }
    });
    connect(ui->toolButtonBrowsePublicKey, &QToolButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(this, tr("Select Public Key"), ui->lineEditPublicKey->text());
        if(!path.isEmpty()) {
            ui->lineEditPublicKey->setText(path);
        }
    });

    updateAuthFields();
}

SessionOptionsSsh2Properties::~SessionOptionsSsh2Properties()
{
    delete ui;
}
