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
#include <QToolBar>
#include <QLabel>

#include "qfonticon.h"
#include "sessionmanagertreemodel.h"

#include "sessionmanagerwidget.h"
#include "ui_sessionmanagerwidget.h"

SessionManagerWidget::SessionManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionManagerWidget)
{
    ui->setupUi(this);

    toolBar = new QToolBar(this);
    ui->verticalLayout->insertWidget(1,toolBar);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(16,16));
    toolBar->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    tree = new SessionManagerTreeView(this);
    ui->verticalLayout->addWidget(tree);

    connect(ui->lineEditFilter, &QLineEdit::textChanged, tree, &SessionManagerTreeView::setFilter);
    connect(tree, &SessionManagerTreeView::sessionConnect, this, &SessionManagerWidget::sessionConnect);
    connect(tree, &SessionManagerTreeView::sessionRemove, this, &SessionManagerWidget::sessionRemove);
    connect(tree, &SessionManagerTreeView::sessionShowProperties, this, &SessionManagerWidget::sessionShowProperties);
    connect(ui->toolButtonClose, &QToolButton::clicked, this, &SessionManagerWidget::sessionManagerHide);

    retranslateUi();
}

SessionManagerWidget::~SessionManagerWidget()
{
    delete ui;
}

void SessionManagerWidget::addActionOnToolBar(QAction *action)
{
    toolBar->addAction(action);
}

void SessionManagerWidget::retranslateUi()
{
    ui->lineEditFilter->setPlaceholderText(tr("Filter by folder/session name"));
    ui->retranslateUi(this);
    tree->retranslateUi();
}

void SessionManagerWidget::addSession(QString str, int type)
{
    tree->addSession(str, type);
}

void SessionManagerWidget::removeSession(QString str)
{
    tree->removeSession(str);
}

void SessionManagerWidget::setCurrentSession(QString str)
{
    tree->setCurrentSession(str);
}

bool SessionManagerWidget::checkSession(QString str)
{
    return tree->checkSession(str);
}
