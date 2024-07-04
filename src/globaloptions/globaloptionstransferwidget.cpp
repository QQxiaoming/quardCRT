/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
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
#include <QButtonGroup>
#include <QDir>
#include "filedialog.h"
#include "globaloptionstransferwidget.h"
#include "ui_globaloptionstransferwidget.h"

GlobalOptionsTransferWidget::GlobalOptionsTransferWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GlobalOptionsTransferWidget)
{
    ui->setupUi(this);

    //set radioButton128Bytes and radioButton1KBytes to be exclusive
    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(ui->radioButton128Bytes);
    group->addButton(ui->radioButton1KBytes);
    ui->radioButton128Bytes->setChecked(true);

    connect(ui->toolButtonUpload, &QToolButton::clicked, [this](){
        QString currentDir = ui->lineEditUpload->text();
        if (currentDir.isEmpty()) {
            currentDir = QDir::homePath();
        }
        QString dir = FileDialog::getExistingDirectory(this, tr("Select Directory"), currentDir);
        if (!dir.isEmpty()) {
            ui->lineEditUpload->setText(dir);
        }
    });
    connect(ui->toolButtonDownload, &QToolButton::clicked, [this](){
        QString currentDir = ui->lineEditDownload->text();
        if (currentDir.isEmpty()) {
            currentDir = QDir::homePath();
        }
        QString dir = FileDialog::getExistingDirectory(this, tr("Select Directory"), currentDir);
        if (!dir.isEmpty()) {
            ui->lineEditDownload->setText(dir);
        }
    });
}

GlobalOptionsTransferWidget::~GlobalOptionsTransferWidget()
{
    delete ui;
}
