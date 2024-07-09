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
#include "qfonticon.h"
#include "statusbarwidget.h"
#include "ui_statusbarwidget.h"

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StatusBarWidget) {
    ui->setupUi(this);
    setMouseTracking(true);

    statusBarCursorInfo = new StatusBarToolButton(this);
    statusBarType = new StatusBarToolButton(this);
    statusBarTransTx = new StatusBarToolButton(this);
    statusBarTransRx = new StatusBarToolButton(this);
    statusBarNotifiction = new StatusBarToolButton(this);
    ui->horizontalLayout->addWidget(statusBarCursorInfo);
    ui->horizontalLayout->addWidget(statusBarType);
    ui->horizontalLayout->addWidget(statusBarTransTx);
    ui->horizontalLayout->addWidget(statusBarTransRx);
    ui->horizontalLayout->addWidget(statusBarNotifiction);

    statusBarCursorInfo->setVisible(false);
    statusBarType->setVisible(false);
    statusBarTransTx->setVisible(false);
    statusBarTransRx->setVisible(false);
    statusBarTransTx->setIcon(QFontIcon::icon(QChar(0xf0ee)));
    statusBarTransRx->setIcon(QFontIcon::icon(QChar(0xf0ed)));
    statusBarNotifiction->setIcon(QFontIcon::icon(QChar(0xf0a2)));

    statusBarCursorInfo->setPopupMode(QToolButton::InstantPopup);
    statusBarCursorInfo->setAutoRaise(true);
    statusBarType->setPopupMode(QToolButton::InstantPopup);
    statusBarType->setAutoRaise(true);
    statusBarTransTx->setPopupMode(QToolButton::InstantPopup);
    statusBarTransTx->setAutoRaise(true);
    statusBarTransTx->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    statusBarTransRx->setPopupMode(QToolButton::InstantPopup);
    statusBarTransRx->setAutoRaise(true);
    statusBarTransRx->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

StatusBarWidget::~StatusBarWidget() {
    delete ui;
}

void StatusBarWidget::setCursorPosition(int64_t x, int64_t y) {
    if(x < 0 || y < 0) {
        statusBarCursorInfo->setText("Ln /, Col /");
        statusBarCursorInfo->setVisible(false);
        return;
    }
    statusBarCursorInfo->setText(QString("Ln %1, Col %2").arg(x).arg(y));
    statusBarCursorInfo->setVisible(true);
}

void StatusBarWidget::setType(const QString &type) {
    if(type.isEmpty()) {
        statusBarType->setText("Null");
        statusBarType->setVisible(false);
        return;
    }
    statusBarType->setText(type);
    statusBarType->setVisible(true);
}

void StatusBarWidget::setTransInfo(int64_t tx, int64_t rx) {
    if(tx < 0 || rx < 0) {
        statusBarTransTx->setText("/");
        statusBarTransRx->setText("/");
        statusBarTransTx->setVisible(false);
        statusBarTransRx->setVisible(false);
        return;
    }
    auto getSize = [](int64_t size) -> QString {
        if( size <= 1024) {
            return QString("%1 B").arg(size);
        } else if ( size <= 1024 * 1024 ) {
            return QString::number(size / 1024.0, 'f', 2) + QString(" KB");
        } else if ( size <= 1024 * 1024 * 1024 ) {
            return QString::number(size / (1024.0 * 1024.0), 'f', 2) + QString(" MB");
        } else {
            return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + QString(" GB");
        }
    };
    statusBarTransTx->setText(QString("%1").arg(getSize(tx)));
    statusBarTransRx->setText(QString("%1").arg(getSize(rx)));
    statusBarTransTx->setVisible(true);
    statusBarTransRx->setVisible(true);
}

void StatusBarWidget::setNotifiction(bool enable) {
    if(enable) {
        statusBarNotifiction->setIcon(QFontIcon::icon(QChar(0xf0f3)));
    } else {
        statusBarNotifiction->setIcon(QFontIcon::icon(QChar(0xf0a2)));
    }
}

