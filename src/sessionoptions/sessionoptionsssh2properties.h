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
#ifndef SESSIONOPTIONSSSH2PROPERTIES_H
#define SESSIONOPTIONSSSH2PROPERTIES_H

#include "passwordedit.h"
#include <QWidget>

namespace Ui {
class SessionOptionsSsh2Properties;
}

class SessionOptionsSsh2Properties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsSsh2Properties(QWidget *parent = nullptr);
    ~SessionOptionsSsh2Properties();

    Ui::SessionOptionsSsh2Properties *ui;
    PasswordEdit *lineEditPassword;
    PasswordEdit *lineEditPassphrase;
};

#endif // SESSIONOPTIONSSSH2PROPERTIES_H
