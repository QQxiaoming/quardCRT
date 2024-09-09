/*
 * This file is part of the https://github.com/QQxiaoming/QFSViewer.git
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
#ifndef SESSIONMANAGERTREEMODEL_H
#define SESSIONMANAGERTREEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QMimeData>
#include <QTreeView>

class TreeItem ;

class SessionManagerTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SessionManagerTreeModel(QTreeView *parent = 0);
	~SessionManagerTreeModel() ;

	void set_root_timestamp(uint32_t timestamp) ;
	QVariant data(const QModelIndex &index, int role) const ;
	int rowCount(const QModelIndex &parent) const ;
	int columnCount(const QModelIndex &parent) const ;
	Qt::ItemFlags flags(const QModelIndex &index) const ;
    bool setData(const QModelIndex &index, const QVariant &value, int role) ;
    bool setData(const QModelIndex &index, QString data, int type, int role);
	bool insertRows(int row, int count, const QModelIndex &parent) ;
	bool removeRows(int row, int count, const QModelIndex &parent) ;
	QModelIndex index(int row, int column, const QModelIndex &parent) const ;
	QModelIndex parent(const QModelIndex &child) const ;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const ;

    QModelIndex addTree(QString str, int type, const QModelIndex &parent) ;
    void removeTree(QModelIndex &parent);
	void info(const QModelIndex &index, int &type, QString &name);
    QString path(const QModelIndex &index, const QModelIndex &rootIndex);
	QModelIndex findItems(QString str, QModelIndex &index);

	void setFilter(QString str) ;

	void dumpTreeItems() ;

private:
	void _dump(TreeItem *p, int tab) ;

signals:

public slots:

private:
	TreeItem	*m_pRootItem ;
	uint32_t     m_roottimestamp;
    QTreeView   *m_parent;
};

#endif // SESSIONMANAGERTREEMODEL_H
