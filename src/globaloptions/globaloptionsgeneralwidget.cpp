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
#include "globaloptionsgeneralwidget.h"
#include "ui_globaloptionsgeneralwidget.h"

GlobalOptionsGeneralWidget::GlobalOptionsGeneralWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsGeneralWidget)
{
    ui->setupUi(this);
#ifndef Q_OS_WIN
    ui->labelWSLUserName->setVisible(false);
    ui->labelWSLDistroName->setVisible(false);
    ui->lineEditWSLUserName->setVisible(false);
    ui->lineEditWSLDistroName->setVisible(false);
#else
    ui->labelWSLUserName->setVisible(true);
    ui->labelWSLDistroName->setVisible(true);
    ui->lineEditWSLUserName->setVisible(true);
    ui->lineEditWSLDistroName->setVisible(true);
#endif
}

GlobalOptionsGeneralWidget::~GlobalOptionsGeneralWidget()
{
    delete ui;
}
