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
#ifndef SESSIONMANAGERTREEVIEW_H
#define SESSIONMANAGERTREEVIEW_H

#include <QStringList>
#include <QTreeView>
#include <QContextMenuEvent>

#include "sessionmanagertreemodel.h"

class SessionManagerTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit SessionManagerTreeView(QWidget *parent = 0);
    ~SessionManagerTreeView();

    bool addSession(QString str, int type, QString path = QString());
    bool addGroup(QString str, QString path = QString());
    void removeSession(QString str);
    void removeGroup(QString str);
    void setCurrentSession(QString str);
    bool checkSession(QString str);
    void retranslateUi();

public slots:
    void setFilter(QString str);

signals:
    void sessionConnect(QString str);
    void sessionRemove(QString str);
    void sessionShowProperties(QString str);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QModelIndex path2Group(const QString &path);
    QModelIndex findItemsRecursion(const QString &str, QModelIndex index);

    SessionManagerTreeModel *mode;
    QModelIndex rootIndex;
};

#endif // SESSIONMANAGERTREEVIEW_H
