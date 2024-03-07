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
#include "filedialog.h"
#include "ui_filedialog.h"

FileDialog::FileDialog(QWidget *parent, const QString &caption,
                        const QString &dir, const QString &filter)
    : QDialog(parent)
    , ui(new Ui::FileDialog)
    , m_dir(dir)
    , m_filter(filter)
{
    ui->setupUi(this);

    setWindowTitle(caption);
}

FileDialog::~FileDialog()
{
    delete ui;
}

QStringList FileDialog::selectedFiles() const
{
    return QStringList();
}

void FileDialog::setFileMode(QFileDialog::FileMode mode) {
    m_mode = mode;
}

void FileDialog::setAcceptMode(QFileDialog::AcceptMode mode) {
    m_acceptMode = mode;
}

void FileDialog::setDefaultSuffix(const QString &suffix) {
    m_defaultSuffix = suffix;
}

void FileDialog::setSidebarUrls(const QList<QUrl> &urls) {
    m_sidebarUrls = urls;
}

void FileDialog::setOptions(QFileDialog::Options options) {
    m_options = options;
}

void FileDialog::selectNameFilter(const QString &filter) {
    ui->fileTypeCombo->setCurrentText(filter);
}

QString FileDialog::selectedNameFilter() const {
    return ui->fileTypeCombo->currentText();
}
