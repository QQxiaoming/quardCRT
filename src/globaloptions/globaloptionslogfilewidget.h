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
#ifndef GLOBALOPTIONSLOGFILEWIDGET_H
#define GLOBALOPTIONSLOGFILEWIDGET_H

#include <QWidget>

namespace Ui {
class GlobalOptionsLogFileWidget;
}

class GlobalOptionsLogFileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalOptionsLogFileWidget(QWidget *parent = nullptr);
    ~GlobalOptionsLogFileWidget();

    Ui::GlobalOptionsLogFileWidget *ui;
};

#endif // GLOBALOPTIONSLOGFILEWIDGET_H
