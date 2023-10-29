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
#include "commandwidget.h"
#include "ui_commandwidget.h"

CommandWidget::CommandWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandWidget)
{
    ui->setupUi(this);

    autoSendTimer = new QTimer(this);
    ui->asciiRadioButton->setChecked(true);

    connect(ui->sendPushButton, &QPushButton::clicked, this, [=]() {
        sendCurrentData();
    });

    connect(ui->autoSendCheckBox, &QCheckBox::stateChanged, this, [=](int state) {
        if(state == Qt::Checked) {
            ui->sendPushButton->setEnabled(false);
            ui->commandPlainEdit->setEnabled(false);
            ui->asciiRadioButton->setEnabled(false);
            ui->hexRadioButton->setEnabled(false);
            ui->autoSendIntervalSpinBox->setEnabled(false);
            autoSendTimer->start(ui->autoSendIntervalSpinBox->value());
        } else {
            ui->sendPushButton->setEnabled(true);
            ui->commandPlainEdit->setEnabled(true);
            ui->asciiRadioButton->setEnabled(true);
            ui->hexRadioButton->setEnabled(true);
            ui->autoSendIntervalSpinBox->setEnabled(true);
            autoSendTimer->stop();
        }
    });

    connect(autoSendTimer, &QTimer::timeout, this, [=]() {
        sendCurrentData();
    });
}

CommandWidget::~CommandWidget()
{
    delete ui;
}

void CommandWidget::setCmd(QString cmd) {
    ui->commandPlainEdit->setPlainText(cmd);
}

void CommandWidget::sendCurrentData(void) {
    if(ui->asciiRadioButton->isChecked()) {
    #if defined(Q_OS_WIN)
        emit sendData(ui->commandPlainEdit->toPlainText().replace("\n","\r\n").toLatin1());
    #else
        emit sendData(ui->commandPlainEdit->toPlainText().toLatin1());
    #endif
    } else if(ui->hexRadioButton->isChecked()) {
        QString input = ui->commandPlainEdit->toPlainText();
        QByteArray array = QByteArray::fromHex(input.toLatin1());
        emit sendData(array);
        QString output = array.toHex(' ');
        ui->commandPlainEdit->setPlainText(output);
    }
}

void CommandWidget::retranslateUi(void) {
    ui->retranslateUi(this);
}
