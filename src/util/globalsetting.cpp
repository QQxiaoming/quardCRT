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
#include <QApplication>
#include "globalsetting.h"

GlobalSetting::GlobalSetting(QObject *parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope,
        QApplication::applicationName(),QApplication::applicationName(),parent)
{
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
void GlobalSetting::setValue(QAnyStringView key, const QVariant &value)
#else
void GlobalSetting::setValue(const QString &key, const QVariant &value)
#endif
{
    QSettings::setValue(key, value);
    if(realtime)
        sync();
}

void GlobalSetting::setRealtimeSave(bool realtime)
{
    this->realtime = realtime;
}
