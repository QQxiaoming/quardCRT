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
#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include "qfonticon.h"

#include <QLineEdit>
#include <QAction>
#include <QIcon>

class PasswordEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PasswordEdit(bool show_visibility = true, QWidget *parent = nullptr) 
        : QLineEdit(parent) {
        if (show_visibility) {
            // Add the password hide/shown toggle at the end of the edit box.
            togglepasswordAction = addAction(QFontIcon::icon(QChar(0xf06e)), QLineEdit::TrailingPosition);
            connect(togglepasswordAction, &QAction::triggered, this, &PasswordEdit::on_toggle_password_Action);
        }
        password_shown = false;
        setEchoMode(QLineEdit::Password);
    }

    void setPasswordShown(bool show) {
        if (show) {
            setEchoMode(QLineEdit::Normal);
            password_shown = true;
            togglepasswordAction->setIcon(QFontIcon::icon(QChar(0xf070)));
        } else {
            setEchoMode(QLineEdit::Password);
            password_shown = false;
            togglepasswordAction->setIcon(QFontIcon::icon(QChar(0xf06e)));
        }
    }

private slots:
    void on_toggle_password_Action() {
        if (!password_shown) {
            setEchoMode(QLineEdit::Normal);
            password_shown = true;
            togglepasswordAction->setIcon(QFontIcon::icon(QChar(0xf070)));
        } else {
            setEchoMode(QLineEdit::Password);
            password_shown = false;
            togglepasswordAction->setIcon(QFontIcon::icon(QChar(0xf06e)));
        }
    }
    
private:
    QAction *togglepasswordAction;
    bool password_shown;
};
    

#endif // PASSWORDEDIT_H
