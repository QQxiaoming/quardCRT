/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2026 Quard <qiaoqm@aliyun.com>
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
#ifndef MICROSOFT_STORE_H
#define MICROSOFT_STORE_H

#include <QApplication>

#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)

#include <QUrl>
#include <QWidget>

namespace MicrosoftStoreApi {

bool isMicrosoftStoreBuild(void);
QString getCurrentPackageFamilyName(void);
bool isRunningWithPackageIdentity(void);
QUrl getMicrosoftStoreProductUrl(void);
QUrl getMicrosoftStoreReviewUrl(void);
bool openMicrosoftStorePage(QWidget *parent, const QUrl &url);

}

#endif

#endif // MICROSOFT_STORE_H
