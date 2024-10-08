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
#include <QMessageBox>
#include "locksessionwindow.h"
#include "ui_locksessionwindow.h"

LockSessionWindow::LockSessionWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LockSessionWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LockSessionWindow::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LockSessionWindow::buttonBoxRejected);
}

LockSessionWindow::~LockSessionWindow()
{
    delete ui;
}

void LockSessionWindow::buttonBoxAccepted(void)
{
    if(islock) {
        QString password = ui->passwordLineEdit->text();
        QString reenterPassword = ui->reenterPasswordLineEdit->text();
        if (password != reenterPassword)
        {
            QMessageBox::warning(this, tr("Error"), tr("Passwords do not match!"));
            this->show();
            return;
        } 
        if (password.isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Password cannot be empty!"));
            this->show();
            return;
        }
        ui->passwordLineEdit->clear();
        ui->reenterPasswordLineEdit->clear();
        emit this->sendLockSessionData(password, ui->lockAllSessionCheckBox->isChecked(), ui->lockAllSessionGroupCheckBox->isChecked());
    } else {
        QString password = ui->passwordLineEdit->text();
        if (password.isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Password cannot be empty!"));
            this->show();
            return;
        }
        ui->passwordLineEdit->clear();
        emit this->sendUnLockSessionData(password, ui->lockAllSessionCheckBox->isChecked(), ui->lockAllSessionGroupCheckBox->isChecked());
    }

    emit this->accepted();
}

void LockSessionWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}

void LockSessionWindow::showLock(void)
{
    ui->headerLabel->setText(tr("Enter the password that will be used to unlock the session:"));
    ui->lockAllSessionCheckBox->setText(tr("Lock all sessions"));
    ui->lockAllSessionGroupCheckBox->setText(tr("Lock all sessions in tab group"));
    ui->lockAllSessionCheckBox->setVisible(true);
    ui->lockAllSessionGroupCheckBox->setVisible(true);
    ui->passwordLineEdit->setVisible(true);
    ui->passwordLabel->setVisible(true);
    ui->reenterPasswordLineEdit->setVisible(true);
    ui->reenterPasswordLabel->setVisible(true);
    islock = true;
    this->show();
}

void LockSessionWindow::showUnlock(void)
{
    ui->headerLabel->setText(tr("Enter the password that was used to lock the session:"));
    ui->lockAllSessionCheckBox->setText(tr("Unlock all sessions"));
    ui->lockAllSessionGroupCheckBox->setText(tr("Unlock all sessions in tab group"));
    ui->lockAllSessionCheckBox->setVisible(true);
    ui->lockAllSessionGroupCheckBox->setVisible(true);
    ui->passwordLineEdit->setVisible(true);
    ui->passwordLabel->setVisible(true);
    ui->reenterPasswordLineEdit->setVisible(false);
    ui->reenterPasswordLabel->setVisible(false);
    islock = false;
    this->show();
}
