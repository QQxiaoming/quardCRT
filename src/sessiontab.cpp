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
#include <QMenu>
#include <QTabBar>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPen>
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include "sessiontab.h"

EmptyTabWidget::EmptyTabWidget(QWidget *parent) 
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setEnabled(false);
    setLayout(new QVBoxLayout);
    layout()->addWidget(label);

    easterEggs = new QLabel(this);
    easterEggs->setAlignment(Qt::AlignCenter);
    layout()->addWidget(easterEggs);
    easterEggs->setVisible(false);
    easterEggs->setEnabled(false);
    QFont font = QApplication::font();
#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
    int fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/inziu-iosevkaCC-SC-regular.ttf");
#else
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
#endif
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    font.setPointSize(12);
    easterEggs->setFont(font);
}

EmptyTabWidget::~EmptyTabWidget() {
}

void EmptyTabWidget::retranslateUi(void) {
    label->setText(tr("No session"));
}

void EmptyTabWidget::mousePressEvent(QMouseEvent *event) {
    static int count = 0;
    static int hitCount = 0;
    if(event->button() == Qt::LeftButton) {
        count++;
        if(count == 7) {
            label->setVisible(false);
            easterEggs->setVisible(true);
            hitCount++;
            switch(hitCount) {
                case 1:
                    easterEggs->setText(
"+--------------------------------------------------+\n"
"|    ___                      _  ____ ____ _____   |\n"
"|   / _ \\ _   _  __ _ _ __ __| |/ ___|  _ \\_   _|  |\n"
"|  | | | | | | |/ _` | '__/ _` | |   | |_) || |    |\n"
"|  | |_| | |_| | (_| | | | (_| | |___|  _ < | |    |\n"
"|   \\__\\_\\\\__,_|\\__,_|_|  \\__,_|\\____|_| \\_\\|_|    |\n"
"|       Please enjoy coding! Quard(乔). 2023.10.24 |\n"
"+--------------------------------------------------+\n"
);
                    hitCount = 0;
                    break;                    
            }
            count = 0;
        } else {
            label->setVisible(true);
            easterEggs->setVisible(false);
        }
    }
}


SessionTab::SessionTab(QWidget *parent) 
    : FancyTabWidget(parent) {

    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);
    setTabBarAutoHide(true);
    setTabTextEditable(false);
    setAddTabButtonVisible(false);

    emptyTab = new EmptyTabWidget(this);
    retranslateUi();
    addTab(emptyTab,"");
    tabBar()->setTabVisible(0,false);
    tabBar()->setTabEnabled(0,false);

    connect(this,&SessionTab::currentChanged,this,[&](int index) {
        setAddTabButtonVisible(count() != 0);
        QWidget *w = widget(index);
        if(w) w->setFocus();
    });
}

SessionTab::~SessionTab() {
}

int SessionTab::count(void) {
    int count = FancyTabWidget::count() - 1;
    return count < 0 ? 0 : count;
} 

void SessionTab::setCurrentIndex(int index) {
    FancyTabWidget::setCurrentIndex(index+1);
}

void SessionTab::contextMenuEvent(QContextMenuEvent *event) {
    int index = tabBar()->tabAt(event->pos());
    index = index == 0 ? -1 : index;
    if(index == -1) {
        if(event->pos().x() > tabBar()->width() &&
            event->pos().y() < tabBar()->height()) {
            index = -2;
        }
    }
    emit showContextMenu(index,event->globalPos());
    Q_UNUSED(event);
}

void SessionTab::retranslateUi(void) {
    emptyTab->retranslateUi();
}
