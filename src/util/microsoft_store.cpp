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
#include "microsoft_store.h"

#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
#include <windows.h>
#include <appmodel.h>

#include <QUrl>
#include <QString>
#include <QDesktopServices>
#include <QMessageBox>

#include <string>

namespace MicrosoftStoreApi {

bool isMicrosoftStoreBuild(void) {
#ifdef MICROSOFT_STORE_BUILD
    return true;
#else
    return false;
#endif
}

QString getCurrentPackageFamilyName(void) {
    UINT32 packageFamilyNameLength = 0;
    LONG result = GetCurrentPackageFamilyName(&packageFamilyNameLength, nullptr);
    if (result == APPMODEL_ERROR_NO_PACKAGE) {
        return QString();
    }
    if (result != ERROR_INSUFFICIENT_BUFFER || packageFamilyNameLength == 0) {
        return QString();
    }

    std::wstring packageFamilyName(packageFamilyNameLength, L'\0');
    result = GetCurrentPackageFamilyName(&packageFamilyNameLength, packageFamilyName.data());
    if (result != ERROR_SUCCESS || packageFamilyNameLength == 0) {
        return QString();
    }

    if (!packageFamilyName.empty() && packageFamilyName.back() == L'\0') {
        packageFamilyName.pop_back();
    }
    return QString::fromStdWString(packageFamilyName);
}

bool isRunningWithPackageIdentity(void) {
    return !getCurrentPackageFamilyName().isEmpty();
}

QUrl getMicrosoftStoreProductUrl(void) {
    const QString packageFamilyName = getCurrentPackageFamilyName();
    if (packageFamilyName.isEmpty()) {
        return QUrl();
    }
    return QUrl(QString("ms-windows-store://pdp/?PFN=%1")
                    .arg(QString::fromLatin1(QUrl::toPercentEncoding(packageFamilyName))));
}

QUrl getMicrosoftStoreReviewUrl(void) {
    const QString packageFamilyName = getCurrentPackageFamilyName();
    if (packageFamilyName.isEmpty()) {
        return QUrl();
    }
    return QUrl(QString("ms-windows-store://review/?PFN=%1")
                    .arg(QString::fromLatin1(QUrl::toPercentEncoding(packageFamilyName))));
}

bool openMicrosoftStorePage(QWidget *parent, const QUrl &url)
{
    if (url.isValid() && QDesktopServices::openUrl(url)) {
        return true;
    }

    if (MicrosoftStoreApi::isMicrosoftStoreBuild()) {
        QMessageBox::information(parent,
                                 QObject::tr("Microsoft Store"),
                                 QObject::tr("This feature requires the packaged MSIX application with package identity. Please install and launch the Microsoft Store build."));
    }
    return false;
}

}
#endif
