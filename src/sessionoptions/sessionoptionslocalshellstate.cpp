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
#include <QStandardItemModel>
#include <QHeaderView>
#include "sessionoptionslocalshellstate.h"
#include "ui_sessionoptionslocalshellstate.h"

SessionOptionsLocalShellState::SessionOptionsLocalShellState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsLocalShellState)
{
    ui->setupUi(this);

    QStandardItemModel *model = new QStandardItemModel(0, 2, this);
    model->setHeaderData(0, Qt::Horizontal, "PID");
    model->setHeaderData(1, Qt::Horizontal, tr("Name"));
    ui->treeViewInfo->setModel(model);
    ui->treeViewInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeViewInfo->setAnimated(true);
    ui->treeViewInfo->setSortingEnabled(true);
    ui->treeViewInfo->setAllColumnsShowFocus(true);
    ui->treeViewInfo->setUniformRowHeights(true);
    ui->treeViewInfo->setWordWrap(false);
    ui->treeViewInfo->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

SessionOptionsLocalShellState::~SessionOptionsLocalShellState()
{
    delete ui;
}
