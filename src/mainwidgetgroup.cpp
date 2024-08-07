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
#include <QWidget>
#include <QPainter>

#include "mainwidgetgroup.h"
#include "sessionswindow.h"

MainWidgetGroup::MainWidgetGroup(Type type, QWidget *parent)
    : QWidget(parent), m_type(type)
{
    splitter = new QSplitter(Qt::Vertical,parent);
    sessionTab = new SessionTab(parent);
    QWidget *widget = new QWidget(parent);
    widget->setLayout(new QVBoxLayout(widget));
    commandWidget = new CommandWidget(parent);
    widget->layout()->addWidget(commandWidget);
    widget->layout()->setContentsMargins(0,0,0,0);

    splitter->setHandleWidth(1);
    splitter->addWidget(sessionTab);
    splitter->addWidget(widget);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,true);
    splitter->setSizes(QList<int>() << 1 << 0);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(0);
    layout->addWidget(splitter);
}

MainWidgetGroup::~MainWidgetGroup()
{
}

void MainWidgetGroup::setActive(bool enable) {
    hasActive = enable;
    update();
}

void MainWidgetGroup::paintEvent(QPaintEvent *event) {
    if (hasActive) {
        QPalette palette;
        QPainter painter(this);
        painter.setPen(palette.color(QPalette::Active, QPalette::Highlight));
        QRect rect = contentsRect();
        rect.adjust(1, 1, -1, -1);
        painter.drawRect(rect);
    }
    QWidget::paintEvent(event);
}
