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
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QToolButton>

#include "filedialog.h"
#include "starttftpseverwindow.h"
#include "ui_starttftpseverwindow.h"

StartTftpSeverWindow::StartTftpSeverWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartTftpSeverWindow)
{
    ui->setupUi(this);
    ui->uploadLineEdit->setText(QDir::homePath());
    ui->downloadLineEdit->setText(QDir::homePath());

    connect(ui->uploadToolButton, &QToolButton::clicked, this, [&](){
        QString dir = FileDialog::getExistingDirectory(this, tr("Open Directory"),
                                QDir::homePath(),
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        ui->uploadLineEdit->setText(dir);
    });
    connect(ui->downloadToolButton, &QToolButton::clicked, this, [&](){
        QString dir = FileDialog::getExistingDirectory(this, tr("Open Directory"),
                                QDir::homePath(),
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        ui->downloadLineEdit->setText(dir);
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &StartTftpSeverWindow::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &StartTftpSeverWindow::buttonBoxRejected);
}

StartTftpSeverWindow::~StartTftpSeverWindow()
{
    delete ui;
}

void StartTftpSeverWindow::retranslateUi(void)
{
    ui->retranslateUi(this);
}

void StartTftpSeverWindow::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}

void StartTftpSeverWindow::buttonBoxAccepted(void)
{
    QFileInfo upDirInfo(ui->uploadLineEdit->text());
    QFileInfo downDirInfo(ui->downloadLineEdit->text());
    if(!upDirInfo.isDir() || !downDirInfo.isDir()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a valid directory!"));
        return;
    }
    emit setTftpInfo(ui->spinBox->value(), ui->uploadLineEdit->text(), ui->downloadLineEdit->text());
    
    emit this->accepted();
}

void StartTftpSeverWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}
