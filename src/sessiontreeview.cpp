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
#include <QTreeView>
#include <QMenu>

#include "qfonticon.h"

#include "sessiontreeview.h"

SessionTreeView::SessionTreeView(QWidget *parent) 
        : QTreeView(parent) {
    mode = new TreeModel(this);
    setModel(mode);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAnimated(true);
    setUniformRowHeights(true);
    setWordWrap(false);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setColumnWidth(0,170);
    setColumnWidth(1,60);
    rootIndex = mode->addTree(tr("Session"), -1, QModelIndex());
    expand(rootIndex);
}

SessionTreeView::~SessionTreeView() {
    delete mode;
}

void SessionTreeView::addSession(QString str, int type) {
    mode->addTree(str, type, rootIndex);
}

void SessionTreeView::removeSession(QString str) {
    QModelIndex index = mode->findItems(str, rootIndex);
    mode->removeTree(index);
}

bool SessionTreeView::checkSession(QString str) {
    QModelIndex index = mode->findItems(str, rootIndex);
    if (index.isValid()) {
        return true;
    }
    return false;
}

void SessionTreeView::retranslateUi() {
    mode->setData(rootIndex, tr("Session"), -1, Qt::DisplayRole);
}

void SessionTreeView::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex tIndex = indexAt(viewport()->mapFromGlobal(event->globalPos()));
    if (tIndex.isValid()) {
        QMenu *contextMenu = new QMenu(this); 
        QAction *connectTerminalAction = new QAction(tr("Connect Terminal"), contextMenu);
        contextMenu->addAction(connectTerminalAction);
        QAction *connectInNewWindowAction = new QAction(tr("Connect in New Window"), contextMenu);
        contextMenu->addAction(connectInNewWindowAction);
        QAction *connectInNewTabGroupAction = new QAction(tr("Connect in New Tab Group"), contextMenu);
        contextMenu->addAction(connectInNewTabGroupAction);
        contextMenu->addSeparator();
        QAction *propertiesAction = new QAction(tr("Properties"), contextMenu);
        contextMenu->addAction(propertiesAction);
        if(!contextMenu->isEmpty()) {
            contextMenu->move(cursor().pos());
            contextMenu->show();
        }
    }
}
