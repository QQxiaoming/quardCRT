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

#include <QDateTime>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QDesktopServices>
#include <QMessageBox>

#include <chrono>
#include <string>

#ifdef MICROSOFT_STORE_BUILD
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Services.Store.h>

namespace {

void ensureWinrtApartment(void)
{
    static bool initialized = false;
    if (initialized) {
        return;
    }

    try {
        winrt::init_apartment();
    } catch (const winrt::hresult_error &error) {
        if (error.code() != winrt::hresult_error(RPC_E_CHANGED_MODE).code()) {
            throw;
        }
    }
    initialized = true;
}

QString qStringFromHString(const winrt::hstring &value)
{
    return QString::fromWCharArray(value.c_str(), static_cast<int>(value.size()));
}

QString formatLicenseDateTime(const winrt::Windows::Foundation::DateTime &dateTime)
{
    if (dateTime.time_since_epoch().count() == 0) {
        return QObject::tr("Never");
    }

    const auto sysTime = winrt::clock::to_sys(dateTime);
    const auto msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(sysTime.time_since_epoch()).count();
    return QDateTime::fromMSecsSinceEpoch(msSinceEpoch, Qt::UTC).toLocalTime().toString(Qt::ISODate);
}

}
#endif


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

QString getAppLicenseMessage(void)
{
#ifdef MICROSOFT_STORE_BUILD
    if (!isMicrosoftStoreBuild()) {
        return QObject::tr("Microsoft Store APIs are only enabled in the dedicated Store build.");
    }

    if (!isRunningWithPackageIdentity()) {
        return QObject::tr("The current process has no package identity. Launch the installed MSIX package to query Microsoft Store license information.");
    }

    try {
        ensureWinrtApartment();

        using namespace winrt::Windows::Services::Store;

        const StoreContext storeContext = StoreContext::GetDefault();
        const StoreAppLicense appLicense = storeContext.GetAppLicenseAsync().get();

        QStringList lines;
        lines << QObject::tr("License Active: %0").arg(appLicense.IsActive() ? QObject::tr("Yes") : QObject::tr("No"));
        lines << QObject::tr("Trial License: %0").arg(appLicense.IsTrial() ? QObject::tr("Yes") : QObject::tr("No"));
        lines << QObject::tr("Expiration Date: %0").arg(formatLicenseDateTime(appLicense.ExpirationDate()));

        const QString skuStoreId = qStringFromHString(appLicense.SkuStoreId());
        if (!skuStoreId.isEmpty()) {
            lines << QObject::tr("SKU Store ID: %0").arg(skuStoreId);
        }

        lines << QObject::tr("Add-on Licenses: %0").arg(appLicense.AddOnLicenses().Size());

        const QString extendedJson = qStringFromHString(appLicense.ExtendedJsonData());
        if (!extendedJson.isEmpty()) {
            lines << QObject::tr("Extended Data Available: Yes");
        }

        return lines.join('\n');
    } catch (const winrt::hresult_error &error) {
        return QObject::tr("Failed to query Microsoft Store license information: %0").arg(qStringFromHString(error.message()));
    } catch (const std::exception &error) {
        return QObject::tr("Failed to query Microsoft Store license information: %0").arg(QString::fromLocal8Bit(error.what()));
    }
#else
    return QObject::tr("Microsoft Store APIs are only enabled in the dedicated Store build.");
#endif
}

QString getAppLicenseSummary(void)
{
    const QString message = getAppLicenseMessage();
    const QStringList lines = message.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return message;
    }
    return lines.first();
}

bool showAppLicenseDialog(QWidget *parent)
{
    QMessageBox::information(parent,
                             QObject::tr("Microsoft Store License"),
                             getAppLicenseMessage());
    return true;
}

}
#endif
