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
#include <QContextMenuEvent>
#include <QGuiApplication>
#include <QMenu>

#include "globalsetting.h"
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
    statusBarSpeedTx = new StatusBarToolButton(this);
    statusBarSpeedRx = new StatusBarToolButton(this);
    statusBarNotifiction = new StatusBarToolButton(this);
    ui->horizontalLayout->addWidget(statusBarCursorInfo);
    ui->horizontalLayout->addWidget(statusBarType);
    ui->horizontalLayout->addWidget(statusBarTransTx);
    ui->horizontalLayout->addWidget(statusBarTransRx);
    ui->horizontalLayout->addWidget(statusBarSpeedTx);
    ui->horizontalLayout->addWidget(statusBarSpeedRx);
    ui->horizontalLayout->addWidget(statusBarNotifiction);

    statusBarCursorInfo->setVisible(false);
    statusBarType->setVisible(false);
    statusBarTransTx->setVisible(false);
    statusBarTransRx->setVisible(false);
    statusBarSpeedTx->setVisible(false);
    statusBarSpeedRx->setVisible(false);
    statusBarCursorInfo->setEnabled(false);
    statusBarType->setEnabled(false);
    statusBarTransTx->setEnabled(false);
    statusBarTransRx->setEnabled(false);
    statusBarSpeedTx->setEnabled(false);
    statusBarSpeedRx->setEnabled(false);

    retranslateUi();

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
    statusBarSpeedTx->setPopupMode(QToolButton::InstantPopup);
    statusBarSpeedTx->setAutoRaise(true);
    statusBarSpeedTx->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    statusBarSpeedRx->setPopupMode(QToolButton::InstantPopup);
    statusBarSpeedRx->setAutoRaise(true);
    statusBarSpeedRx->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    statusBarNotifiction->setPopupMode(QToolButton::InstantPopup);
    statusBarNotifiction->setAutoRaise(true);
}

StatusBarWidget::~StatusBarWidget() {
    delete ui;
}

void StatusBarWidget::setCursorPosition(int64_t x, int64_t y) {
    if(x < 0 || y < 0) {
        statusBarCursorInfo->setText("Ln /, Col /");
        statusBarCursorInfo->setEnabled(false);
        statusBarCursorInfo->setVisible(false);
        return;
    }
    statusBarCursorInfo->setText(QString("Ln %1, Col %2").arg(x).arg(y));
    if(!statusBarCursorInfo->isEnabled()) {
        statusBarCursorInfo->setEnabled(true);
        GlobalSetting settings;
        statusBarCursorInfo->setVisible(settings.value("Global/Statusbar/CurrentCursorUI", true).toBool());
    }
}

void StatusBarWidget::setType(const QString &type) {
    if(type.isEmpty()) {
        statusBarType->setText("Null");
        statusBarType->setEnabled(false);
        statusBarType->setVisible(false);
        return;
    }
    statusBarType->setText(type);
    if(!statusBarType->isEnabled()) {
        statusBarType->setEnabled(true);
        GlobalSetting settings;
        statusBarType->setVisible(settings.value("Global/Statusbar/TypeUI", true).toBool());
    }
}

void StatusBarWidget::setTransInfo(bool enable, int64_t tx, int64_t rx) {
    if(!enable) {
        statusBarTransTx->setText("/");
        statusBarTransRx->setText("/");
        statusBarTransTx->setEnabled(false);
        statusBarTransTx->setVisible(false);
        statusBarTransRx->setEnabled(false);
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
    if(!statusBarTransTx->isEnabled()) {
        statusBarTransTx->setEnabled(true);
        statusBarTransRx->setEnabled(true);
        GlobalSetting settings;
        bool enable = settings.value("Global/Statusbar/TransferUI", true).toBool();
        statusBarTransTx->setVisible(enable);
        statusBarTransRx->setVisible(enable);
    }
}

void StatusBarWidget::setSpeedInfo(bool enable, qreal tx, qreal rx) {
    if(!enable) {
        statusBarSpeedTx->setText("/");
        statusBarSpeedRx->setText("/");
        statusBarSpeedTx->setEnabled(false);
        statusBarSpeedTx->setVisible(false);
        statusBarSpeedRx->setEnabled(false);
        statusBarSpeedRx->setVisible(false);
        return;
    }
    auto getSize = [](qreal size) -> QString {
        if( size <= 1024.0) {
            return QString::number(size, 'f', 2) + QString(" B/s");
        } else if ( size <= 1024.0 * 1024.0 ) {
            return QString::number(size / 1024.0, 'f', 2) + QString(" KB/s");
        } else if ( size <= 1024.0 * 1024.0 * 1024.0 ) {
            return QString::number(size / (1024.0 * 1024.0), 'f', 2) + QString(" MB/s");
        } else {
            return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + QString(" GB/s");
        }
    };
    statusBarSpeedTx->setText(QString("%1").arg(getSize(tx)));
    statusBarSpeedRx->setText(QString("%1").arg(getSize(rx)));
    if(!statusBarSpeedTx->isEnabled()) {
        statusBarSpeedTx->setEnabled(true);
        statusBarSpeedRx->setEnabled(true);
        GlobalSetting settings;
        bool enable = settings.value("Global/Statusbar/TransferUI", true).toBool();
        statusBarSpeedTx->setVisible(enable);
        statusBarSpeedRx->setVisible(enable);
    }
}

void StatusBarWidget::setNotifiction(bool enable) {
    if(enable) {
        statusBarNotifiction->setIcon(QFontIcon::icon(QChar(0xf0f3)));
    } else {
        statusBarNotifiction->setIcon(QFontIcon::icon(QChar(0xf0a2)));
    }
}

void StatusBarWidget::contextMenuEvent(QContextMenuEvent *event) {
    Q_UNUSED(event);
    QMenu *menu = new QMenu(this);

    if(statusBarCursorInfo->isEnabled()) {
        QAction *actionCurrentCursor = new QAction(tr("Current Cursor"), this);
        actionCurrentCursor->setCheckable(true);
        actionCurrentCursor->setChecked(statusBarCursorInfo->isVisible());
        connect(actionCurrentCursor, &QAction::triggered, [this](bool checked) {
            GlobalSetting settings;
            settings.setValue("Global/Statusbar/CurrentCursorUI", checked);
            statusBarCursorInfo->setVisible(checked);
        });
        menu->addAction(actionCurrentCursor);
    }

    if(statusBarType->isEnabled()) {
        QAction *actionType = new QAction(tr("Session Type"), this);
        actionType->setCheckable(true);
        actionType->setChecked(statusBarType->isVisible());
        connect(actionType, &QAction::triggered, [this](bool checked) {
            GlobalSetting settings;
            settings.setValue("Global/Statusbar/TypeUI", checked);
            statusBarType->setVisible(checked);
        });
        menu->addAction(actionType);
    }

    if(statusBarTransTx->isEnabled()) {
        QAction *actionTrans = new QAction(tr("Transfer Info"), this);
        actionTrans->setCheckable(true);
        actionTrans->setChecked(statusBarTransTx->isVisible());
        connect(actionTrans, &QAction::triggered, [this](bool checked) {
            GlobalSetting settings;
            settings.setValue("Global/Statusbar/TransferUI", checked);
            statusBarTransTx->setVisible(checked);
            statusBarTransRx->setVisible(checked);
            statusBarSpeedTx->setVisible(checked);
            statusBarSpeedRx->setVisible(checked);
        });
        menu->addAction(actionTrans);
    }

    if(menu->isEmpty()) {
        delete menu;
        return;
    }

    QRect screenGeometry = QGuiApplication::screenAt(cursor().pos())->geometry();
    QPoint pos = cursor().pos() + QPoint(5,5);
    if (pos.x() + menu->width() > screenGeometry.right()) {
        pos.setX(screenGeometry.right() - menu->width());
    }
    if (pos.y() + menu->height() > screenGeometry.bottom()) {
        pos.setY(screenGeometry.bottom() - menu->height());
    }
    menu->popup(pos);
}

void StatusBarWidget::retranslateUi()
{
    statusBarCursorInfo->setToolTip(tr("Current Cursor"));
    statusBarType->setToolTip(tr("Session Type"));
    statusBarTransTx->setToolTip(tr("Upload Total"));
    statusBarTransRx->setToolTip(tr("Download Total"));
    statusBarSpeedTx->setToolTip(tr("Upload Speed"));
    statusBarSpeedRx->setToolTip(tr("Download Speed"));

    statusBarTransTx->setIcon(QFontIcon::icon(QChar(0xf0ee)));
    statusBarTransRx->setIcon(QFontIcon::icon(QChar(0xf0ed)));
    statusBarSpeedTx->setIcon(QFontIcon::icon(QChar(0xf0aa)));
    statusBarSpeedRx->setIcon(QFontIcon::icon(QChar(0xf0ab)));
    statusBarNotifiction->setIcon(QFontIcon::icon(QChar(0xf0a2)));

    ui->retranslateUi(this);
}
