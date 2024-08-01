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
#ifndef INTERNALCOMMANDWINDOW_H
#define INTERNALCOMMANDWINDOW_H

#include <QDialog>
#include <QContextMenuEvent>
#include "qtermwidget.h"
#include "internalcommandprocess.h"

namespace Ui {
class InternalCommandWindow;
}

class InternalCommandWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InternalCommandWindow(QWidget *parent = nullptr);
    ~InternalCommandWindow();
#ifdef ENABLE_PYTHON
    void setPyRun(PyRun *pyRun);
#endif

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    Ui::InternalCommandWindow *ui;
    QTermWidget *term;
    InternalCommandProcess *process;
};

#endif // INTERNALCOMMANDWINDOW_H
