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
#ifndef SESSIONTAB_H
#define SESSIONTAB_H

#include <QWidget>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QLabel>
#include <QVBoxLayout>
#include "fancytabwidget.h"

class EmptyTabWidget : public QWidget {
    Q_OBJECT
public:
    explicit EmptyTabWidget(QWidget *parent = nullptr);
    ~EmptyTabWidget();
    void retranslateUi(void);
    
protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel *label;
    QLabel *easterEggs;
};

class SessionTab : public FancyTabWidget {
    Q_OBJECT
public:
    explicit SessionTab(QWidget *parent = nullptr);
    ~SessionTab();
    int count(void);
    void setCurrentIndex(int index);
    void retranslateUi(void);

signals:
    void showContextMenu(int index, const QPoint& position);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    EmptyTabWidget *emptyTab;
};

#endif // SESSIONTAB_H
