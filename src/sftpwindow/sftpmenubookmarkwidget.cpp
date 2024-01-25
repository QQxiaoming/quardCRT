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
#include "sftpmenubookmarkwidget.h"
#include "filedialog.h"
#include "ui_sftpmenubookmarkwidget.h"

SFTPmenuBookmarkWidget::SFTPmenuBookmarkWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SFTPmenuBookmarkWidget)
{
    ui->setupUi(this);

    connect(ui->toolButton, &QToolButton::clicked, this, [&](){
        QString dir = FileDialog::getExistingDirectory(this, tr("Open Directory"),
                 ui->lineEditLocalPath->text().isEmpty() ? QDir::homePath() : ui->lineEditLocalPath->text());
        if (!dir.isEmpty())
            ui->lineEditLocalPath->setText(dir);
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [&](){
        emit accepted();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [&](){
        emit rejected();
    });
}

SFTPmenuBookmarkWidget::~SFTPmenuBookmarkWidget()
{
    delete ui;
}

void SFTPmenuBookmarkWidget::setConfig(const QString &bookmarkName,const QString &localPath, const QString &remotePath) {
    ui->lineEditBookmarkName->setText(bookmarkName);
    ui->lineEditLocalPath->setText(localPath);
    ui->lineEditRemotePath->setText(remotePath);
    bookmarkInitName = bookmarkName;
}

void SFTPmenuBookmarkWidget::getConfig(QString &bookmarkName, QString &localPath, QString &remotePath) {
    bookmarkName = ui->lineEditBookmarkName->text();
    localPath = ui->lineEditLocalPath->text();
    remotePath = ui->lineEditRemotePath->text();
}

QDataStream &operator<<(QDataStream &out, const SFTPmenuBookmarkWidget::Config &config) {
    out << config.bookmarkName << config.localPath << config.remotePath;
    return out;
}

QDataStream &operator>>(QDataStream &in, SFTPmenuBookmarkWidget::Config &config) {
    in >> config.bookmarkName >> config.localPath >> config.remotePath;
    return in;
}
