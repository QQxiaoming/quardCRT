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
#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QFileDialog>
#include <QStandardPaths>

class FileDialog
{
private:
    static QList<QUrl> getSiderbarUrls(void) {
        QList<QUrl> sidebarUrls;
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MusicLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)));
    #if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        sidebarUrls.append(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PublicShareLocation)));
    #endif
        QFileInfoList drivesList = QDir::drives();
        foreach (QFileInfo drive, drivesList) {
            sidebarUrls.append(QUrl::fromLocalFile(drive.absoluteFilePath()));
        }
        return sidebarUrls;
    }
public:
    static QString getOpenFileName(QWidget *parent = nullptr,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = nullptr,
                                   QFileDialog::Options options = QFileDialog::Options()) {
        QFileDialog dialog(parent, caption, dir, filter);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        dialog.setDefaultSuffix("txt");
        dialog.setSidebarUrls(FileDialog::getSiderbarUrls());
        dialog.setOptions(options);
        if (selectedFilter && !selectedFilter->isEmpty())
            dialog.selectNameFilter(*selectedFilter);
        if (dialog.exec() == QDialog::Accepted) {
            if (selectedFilter)
                *selectedFilter = dialog.selectedNameFilter();
            return dialog.selectedFiles().value(0);
        }
        return QString();
    }

    static QStringList getOpenFileNames(QWidget *parent = nullptr,
                                    const QString &caption = QString(),
                                    const QString &dir = QString(),
                                    const QString &filter = QString(),
                                    QString *selectedFilter = nullptr,
                                    QFileDialog::Options options = QFileDialog::Options()) {
        QFileDialog dialog(parent, caption, dir, filter);
        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        dialog.setDefaultSuffix("txt");
        dialog.setSidebarUrls(FileDialog::getSiderbarUrls());
        dialog.setOptions(options);
        if (selectedFilter && !selectedFilter->isEmpty())
            dialog.selectNameFilter(*selectedFilter);
        if (dialog.exec() == QDialog::Accepted) {
            if (selectedFilter)
                *selectedFilter = dialog.selectedNameFilter();
            return dialog.selectedFiles();
        }
        return QStringList();
    }

    static QString getSaveFileName(QWidget *parent = nullptr,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = nullptr,
                                   QFileDialog::Options options = QFileDialog::Options()) {
        QFileDialog dialog(parent, caption, dir, filter);
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setSidebarUrls(FileDialog::getSiderbarUrls());
        dialog.setOptions(options);
        if (selectedFilter && !selectedFilter->isEmpty())
            dialog.selectNameFilter(*selectedFilter);
        if (dialog.exec() == QDialog::Accepted) {
            if (selectedFilter)
                *selectedFilter = dialog.selectedNameFilter();
            return dialog.selectedFiles().value(0);
        }
        return QString();
    }
    static QString getExistingDirectory(QWidget *parent = nullptr,
                                        const QString &caption = QString(),
                                        const QString &dir = QString(),
                                        QFileDialog::Options options = QFileDialog::ShowDirsOnly) {
        QFileDialog dialog(parent, caption, dir, QString());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setSidebarUrls(FileDialog::getSiderbarUrls());
        dialog.setOptions(options);
        if (dialog.exec() == QDialog::Accepted)
            return dialog.selectedFiles().value(0);
        return QString();
    }
};

#endif // FILEDIALOG_H
