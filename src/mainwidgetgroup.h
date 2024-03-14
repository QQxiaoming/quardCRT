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
#ifndef MAINWIDGETGROUP_H
#define MAINWIDGETGROUP_H

#include <QObject>
#include <QSplitter>
#include "commandwidget.h"
#include "sessiontab.h"

class MainWidgetGroup: public QObject
{
    Q_OBJECT
public:
    enum Type {
        EMBEDDED,
        FLOATING,
    };
    MainWidgetGroup(Type type, QWidget *parent = nullptr);
    ~MainWidgetGroup();

    Type type() const { return m_type; }

public:
    QSplitter *splitter;
    SessionTab *sessionTab;
    CommandWidget *commandWidget;
    Type m_type;
};

#endif // MAINWIDGETGROUP_H
