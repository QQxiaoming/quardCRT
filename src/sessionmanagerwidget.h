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
#ifndef SESSIONMANAGERWIDGET_H
#define SESSIONMANAGERWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QLabel>

#include "sessiontreeview.h"

namespace Ui {
class SessionManagerWidget;
}

class SessionManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionManagerWidget(QWidget *parent = nullptr);
    ~SessionManagerWidget();

    void addActionOnToolBar(QAction *action);
    void retranslateUi();
    void addSession(QString str, int type);
    void removeSession(QString str);
    bool checkSession(QString str);

signals:
    void sessionConnect(QString str);
    void sessionRemove(QString str);

private:
    Ui::SessionManagerWidget *ui;
    QToolBar *toolBar;
    QLabel *label;
    SessionTreeView *tree;
};

#endif // SESSIONMANAGERWIDGET_H
