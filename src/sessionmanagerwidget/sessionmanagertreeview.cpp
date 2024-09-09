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
#include <QMessageBox>
#include <QInputDialog>

#include "qfonticon.h"

#include "sessionmanagertreeview.h"

SessionManagerTreeView::SessionManagerTreeView(QWidget *parent) 
        : QTreeView(parent) {
    mode = new SessionManagerTreeModel(this);
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

    connect(this, &SessionManagerTreeView::doubleClicked, this, [&](const QModelIndex &index){
        int type; 
        QString name;
        mode->info(index, type, name);
        if(type != -1) {
            emit sessionConnect(name);
        }
    });
}

SessionManagerTreeView::~SessionManagerTreeView() {
    delete mode;
}

QModelIndex SessionManagerTreeView::path2Group(const QString &path) {
    QModelIndex index = rootIndex;
    if((!path.isEmpty()) && (path != "/") && (path.startsWith("/"))) {
        QStringList pathList = path.split("/");
        for(int i = 1; i < pathList.size(); i++) {
            index = mode->findItems(pathList[i], index);
            if(!index.isValid()) {
                return index;
            }
            int type; QString name;
            mode->info(index, type, name);
            if(type != -1) {
                return index;
            }
        }
    }
    return index;
}

QModelIndex SessionManagerTreeView::findItemsRecursion(const QString &str, QModelIndex index) {
    QModelIndex findex = mode->findItems(str, index);
    if (findex.isValid()) {
        return findex;
    }
    for (int i = 0; i < mode->rowCount(index); i++) {
        QModelIndex cindex = mode->index(i, 0, index);
        if (mode->hasChildren(cindex)) {
            findex = findItemsRecursion(str, cindex);
            if (findex.isValid()) {
                return findex;
            }
        }
    }
    return QModelIndex();
}

bool SessionManagerTreeView::addSession(QString str, int type, QString path) {
    QModelIndex index = path2Group(path);
    mode->addTree(str, type, index);
    return true;
}

bool SessionManagerTreeView::addGroup(QString str, QString path) {
    if(!checkSession(str)){
        return addSession(str, -1, path);
    }
    return false;
}

void SessionManagerTreeView::removeSession(QString str) {
    QModelIndex index = findItemsRecursion(str, rootIndex);
    mode->removeTree(index);
}

void SessionManagerTreeView::removeGroup(QString str) {
    removeSession(str);
}

void SessionManagerTreeView::setCurrentSession(QString str) {
    setCurrentIndex(findItemsRecursion(str, rootIndex));
}

bool SessionManagerTreeView::checkSession(QString str) {
    QModelIndex index = findItemsRecursion(str, rootIndex);
    if (index.isValid()) {
        return true;
    }
    return false;
}

void SessionManagerTreeView::retranslateUi() {
    mode->setData(rootIndex, tr("Session"), -1, Qt::DisplayRole);
}

void SessionManagerTreeView::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex tIndex = indexAt(viewport()->mapFromGlobal(event->globalPos()));
    if (tIndex.isValid()) {
        int type; QString name;
        mode->info(tIndex, type, name);
        QString path = mode->path(tIndex,rootIndex);
        QMenu *contextMenu = new QMenu(this); 
        if(type != -1) {
            QAction *connectTerminalAction = new QAction(tr("Connect Terminal"), contextMenu);
            contextMenu->addAction(connectTerminalAction);
            connect(connectTerminalAction, &QAction::triggered, this, [&,name](){
                emit sessionConnect(name);
            });
            QAction *connectInNewWindowAction = new QAction(tr("Connect in New Window"), contextMenu);
            contextMenu->addAction(connectInNewWindowAction);
            connect(connectInNewWindowAction, &QAction::triggered, this, [&,name](){
                QStringList args;
                args << "--start_know_session" << name;
                QProcess::startDetached(QApplication::applicationFilePath(),args);
            });
            QAction *connectInNewTabGroupAction = new QAction(tr("Connect in New Tab Group"), contextMenu);
            contextMenu->addAction(connectInNewTabGroupAction);
            connect(connectInNewTabGroupAction, &QAction::triggered, this, [&,name](){
                //TODO:Connect in another tab group
                emit sessionConnect(name);
            });
            contextMenu->addSeparator();
            QAction *deleteAction = new QAction(tr("Delete"), contextMenu);
            contextMenu->addAction(deleteAction);
            connect(deleteAction, &QAction::triggered, this, [&,name](){
                if(QMessageBox::question(this, tr("Delete Session"), tr("Are you sure you want to delete the session?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    emit sessionRemove(name);
                }
            });
            contextMenu->addSeparator();
            QAction *propertiesAction = new QAction(tr("Properties"), contextMenu);
            contextMenu->addAction(propertiesAction);
            connect(propertiesAction, &QAction::triggered, this, [&,name](){
                emit sessionShowProperties(name);
            });
        } else {
            QAction *newGroupAction = new QAction(tr("New Group"), contextMenu);
            contextMenu->addAction(newGroupAction);
            connect(newGroupAction, &QAction::triggered, this, [&,path,name](){
                QString newGroup = QInputDialog::getText(this, tr("New Group"), tr("Enter the name of the new group:"));
                if(!newGroup.isEmpty()) {
                    bool ret = false;
                    if(path.isEmpty()) {
                        ret = addGroup(newGroup, "/");
                    } else {
                        if(path == "/") {
                            ret = addGroup(newGroup, path + name);
                        } else {
                            ret = addGroup(newGroup, path + "/" + name);
                        }
                    }
                    if(!ret) {
                        QMessageBox::warning(this, tr("New Group"), tr("The group already exists!"));
                    }
                }
            });
            if(!path.isEmpty()) {
                QAction *deleteGroupAction = new QAction(tr("Delete"), contextMenu);
                contextMenu->addAction(deleteGroupAction);
                connect(deleteGroupAction, &QAction::triggered, this, [&,path,name](){
                    if(QMessageBox::question(this, tr("Delete Group"), tr("Are you sure you want to delete the group?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                        removeGroup(name);
                    }
                });
            }
        }

        if(!contextMenu->isEmpty()) {
            QRect screenGeometry = QGuiApplication::screenAt(cursor().pos())->geometry();
            QPoint pos = cursor().pos() + QPoint(5,5);
            if (pos.x() + contextMenu->width() > screenGeometry.right()) {
                pos.setX(screenGeometry.right() - contextMenu->width());
            }
            if (pos.y() + contextMenu->height() > screenGeometry.bottom()) {
                pos.setY(screenGeometry.bottom() - contextMenu->height());
            }
            contextMenu->popup(pos);
        }
    }
}

void SessionManagerTreeView::setFilter(QString str) {
    mode->setFilter(str);
}
