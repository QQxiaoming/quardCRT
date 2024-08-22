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
#include "globaloptionsadvancedwidget.h"
#include "ui_globaloptionsadvancedwidget.h"

GlobalOptionsAdvancedWidget::GlobalOptionsAdvancedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsAdvancedWidget)
{
    ui->setupUi(this);

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    ui->checkBoxEnableCtrlC->setVisible(true);
#else
    ui->checkBoxEnableCtrlC->setVisible(false);
#endif
#if defined(Q_OS_WIN)
    ui->labelPowerShellProfile->setVisible(true);
    ui->lineEditPowerShellProfile->setVisible(true);
    ui->toolButtonPowerShellProfile->setVisible(true);
    ui->pushButtonPowerShellProfile->setVisible(true);
#else
    ui->labelPowerShellProfile->setVisible(false);
    ui->lineEditPowerShellProfile->setVisible(false);
    ui->toolButtonPowerShellProfile->setVisible(false);
    ui->pushButtonPowerShellProfile->setVisible(false);
#endif
}

GlobalOptionsAdvancedWidget::~GlobalOptionsAdvancedWidget()
{
    delete ui;
}
