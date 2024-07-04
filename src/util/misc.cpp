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
#include "misc.h"

#if defined(Q_OS_WIN)
#include <windows.h>

#include <QString>

namespace MiscWin32 {

QString getUserName(void) {
    DWORD usernameSize = 0;
    GetUserNameW(NULL, &usernameSize);

    WCHAR *username = new WCHAR[usernameSize+1];
    if (GetUserNameW(username, &usernameSize) == 0) {
        return "UnknowUserName";
    }

    QString qsUsername = QString::fromWCharArray(username, usernameSize-1);
    delete[] username;
    if(qsUsername.isEmpty()) {
        return "UnknowUserName";
    }
    return qsUsername;
}

QString getComputerName(void) {
    DWORD computernameSize = 0;
    GetComputerNameW(NULL, &computernameSize);

    WCHAR *computername = new WCHAR[computernameSize+1];
    if (GetComputerNameW(computername, &computernameSize) == 0) {
        return "UnknowComputerName";
    }

    QString qsComputername = QString::fromWCharArray(computername, computernameSize);
    delete[] computername;
    if(qsComputername.isEmpty()) {
        return "UnknowComputerName";
    }
    return qsComputername;
}

}

#endif
