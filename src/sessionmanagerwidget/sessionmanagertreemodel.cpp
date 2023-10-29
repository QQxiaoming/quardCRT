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
#include <QDebug>
#include <QIODevice>
#include <QTreeView>
#include <QDateTime>
#include "sessionmanagertreemodel.h"
#include "qfonticon.h"
#include "sessionswindow.h"

class TreeItem
{
public:
	TreeItem() {}
	TreeItem(QString str, int type, TreeItem *parent) :
        m_str(str),m_type(type),m_pParent(parent)
	{
	}
	~TreeItem()
	{
		qDeleteAll(m_children) ;
	}
	void appendChild(TreeItem *pChild) { m_children.append(pChild) ; }
	void insertChild(int idx, TreeItem *pChild) { m_children.insert(idx, pChild) ; }
	void removeChild(TreeItem *p)
	{
		int index = m_children.indexOf(p) ;
		if ( index < 0 ) { return ; }
		m_children.removeAt(index) ;
		delete p ;
	}

	QString data() { return m_str ; }
	int type() { return m_type ; }
	void setData(QString str) { m_str = str ; }
	void setType(int type) { m_type = type ; }
	int childCount() { return m_children.size() ; }
	QList<TreeItem *> &children() { return m_children ; }
	TreeItem *parent() { return m_pParent ; }
	TreeItem *child(int row)
	{
		if ( row < 0 || row >= childCount() ) { return NULL ; }
		return m_children[row] ;
	}
	int row()
	{
		if ( m_pParent ) {
			return m_pParent->children().indexOf(this) ;
		}
		return 0 ;
	}

	void copy(TreeItem *p)
	{
		m_str = p->m_str ;
		for ( int i = 0 ; i < p->m_children.size() ; i ++ ) {
            insertChild(i, new TreeItem(p->m_children[i]->m_str,p->m_children[i]->m_type, this)) ;
			this->m_children[i]->copy(p->m_children[i]) ;
		}
	}

private:
    QString				m_str;
    int				    m_type;
	TreeItem			*m_pParent ;
	QList<TreeItem *>	m_children ;
} ;

SessionManagerTreeModel::SessionManagerTreeModel(QTreeView *parent) :
	QAbstractItemModel(parent),m_roottimestamp(0),m_parent(parent)
{
	m_pRootItem = new TreeItem(" ", 0, NULL) ;
}

SessionManagerTreeModel::~SessionManagerTreeModel()
{
	delete m_pRootItem ;
}

void SessionManagerTreeModel::set_root_timestamp(uint32_t timestamp)
{
	m_roottimestamp = timestamp;
}

QVariant SessionManagerTreeModel::data(const QModelIndex &index, int role) const
{
    if ( role != Qt::DisplayRole && role != Qt::EditRole  && role != Qt::DecorationRole && role != (Qt::UserRole + 1)) { return QVariant() ; }
	if ( !index.isValid() ) { return QVariant() ; }

    TreeItem *p = static_cast<TreeItem *>(index.internalPointer()) ;
	if(role == Qt::DecorationRole) {
		switch (index.column())
		{
		case 0: // name
			if(p->type() == -1) {
				if(m_parent->isExpanded(index) && p->childCount()) {
					return QIcon(QFontIcon::icon(QChar(0xf07c)));
				} else {
					return QIcon(QFontIcon::icon(QChar(0xf07b)));
				}
			} else {
				return QIcon(QFontIcon::icon(QChar(0xf0c1)));
			}
		case 1: // kind
			break;
		}
	} else {
		switch (index.column())
		{
		case 0: // name
			return p->data();
		case 1: // kind
			switch (p->type())
			{
			case SessionsWindow::Telnet:
				return tr("Telnet");
			case SessionsWindow::Serial:
				return tr("Serial");
			case SessionsWindow::LocalShell:
				return tr("Shell");
			case SessionsWindow::RawSocket:
				return tr("Raw");
			default:
				break;
			}
		}
	}

	return QVariant();
}

void SessionManagerTreeModel::info(const QModelIndex &index, int &type, QString &name)
{
	if ( !index.isValid() ) { return; }

    TreeItem *p = static_cast<TreeItem *>(index.internalPointer()) ;

	type = p->type();
	name = p->data();
}

int SessionManagerTreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

	return p->childCount() ;
}

int SessionManagerTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
	return 2 ;	// カラムは常に4つ
}

Qt::ItemFlags SessionManagerTreeModel::flags(const QModelIndex &index) const
{
	if ( !index.isValid() ) {
		return Qt::ItemIsEnabled ;
	}
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
		 | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled ;		// drag and drop処理入れる時は追加
}

bool SessionManagerTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return setData(index, value.toList()[0].toString(),value.toList()[1].toInt(),role);
}

bool SessionManagerTreeModel::setData(const QModelIndex &index, QString data, int type, int role)
{
    if ( role != Qt::DisplayRole && role != Qt::EditRole ) {
		return false ;
	}

	TreeItem *p = m_pRootItem ;
	if ( index.isValid() ) {
		p = static_cast<TreeItem *>(index.internalPointer()) ;
	}

    p->setData(data) ;
    p->setType(type) ;
    QList<int> lRole = {role} ;
	emit dataChanged(index, index, lRole);
	return true ;
}

bool SessionManagerTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row+count-1) ;
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

    p->insertChild(row, new TreeItem(QString(), 0, p)) ;
	endInsertRows();
	return true ;
}

bool SessionManagerTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row+count-1) ;
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

	p->removeChild(p->child(row)) ;
	endRemoveRows();
	return true ;
}

QModelIndex SessionManagerTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if ( !hasIndex(row, column, parent) ) { return QModelIndex() ; }

	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

	TreeItem *pChild = p->child(row) ;
	if ( pChild ) {
		return createIndex(row, column, pChild) ;
	}
	return QModelIndex() ;
}

QModelIndex SessionManagerTreeModel::parent(const QModelIndex &child) const
{
	if ( !child.isValid() ) { return QModelIndex() ; }
	TreeItem *c = static_cast<TreeItem *>(child.internalPointer()) ;
	TreeItem *p = c->parent() ;
	if ( p == m_pRootItem ) { return QModelIndex() ; }
	return createIndex(p->row(), 0, p) ;
}

QVariant SessionManagerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        switch(section) {
			case 0:
				return tr("Name");
			case 1:
				return tr("Kind");
		}
	}
	return QVariant() ;
}

// drag and drop 処理 ----------------------------------------
Qt::DropActions SessionManagerTreeModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction ;
}

QStringList SessionManagerTreeModel::mimeTypes() const
{
	QStringList types ;
	types << "application/tree.item.list" ;
	return types ;
}

QMimeData *SessionManagerTreeModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData() ;
	QByteArray encodeData ;

	QDataStream stream(&encodeData, QIODevice::WriteOnly) ;
	foreach ( const QModelIndex &index, indexes ) {
		if ( index.isValid() ) {
			stream << reinterpret_cast<quint64>(index.internalPointer()) ;
		}
	}
	mimeData->setData("application/tree.item.list", encodeData) ;
	return mimeData ;
}

bool SessionManagerTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);

	if ( action == Qt::IgnoreAction ) { return true ; }
	if ( !data->hasFormat("application/tree.item.list") ) { return false ; }
	if ( column > 0 ) { return false ; }

	QByteArray encodeData = data->data("application/tree.item.list") ;
	QDataStream stream(&encodeData, QIODevice::ReadOnly) ;

	while ( !stream.atEnd() ) {
		quint64 val ;
		TreeItem *p ;
		stream >> val ;
		p = reinterpret_cast<TreeItem *>(val) ;

		QString text = p->data() ;
        QModelIndex index = addTree(text, 0, parent) ;
		TreeItem *newItem = static_cast<TreeItem *>(index.internalPointer()) ;
		newItem->copy(p) ;
	}
	return true;
}
// drag and drop 処理 ここまで ----------------------------------

QModelIndex SessionManagerTreeModel::addTree(QString str, int type, const QModelIndex &parent)
{
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}
	int row = p->childCount() ;

	insertRows(row, 1, parent) ;	// row 追加

    QModelIndex index = this->index(row, 0, parent) ;
    setData(index, str, type, Qt::DisplayRole) ;
	return index ;
}

void SessionManagerTreeModel::removeTree(QModelIndex &index)
{
	if ( !index.isValid() ) { return ; }

	removeRows(index.row(), 1, index.parent()) ;
}

QModelIndex SessionManagerTreeModel::findItems(QString str, QModelIndex &index)
{
	TreeItem *p = m_pRootItem ;
	if ( index.isValid() ) {
		p = static_cast<TreeItem *>(index.internalPointer()) ;
	}
	for ( int i = 0 ; i < p->childCount() ; i ++ ) {
		TreeItem *c = p->child(i);
		if(c->data() == str) {
			return createIndex(c->row(), 0, c) ;
		}
	}
	return QModelIndex();
}

void SessionManagerTreeModel::dumpTreeItems()
{
	TreeItem *p = m_pRootItem ;
	int tab = 0 ;
	qDebug() << p->data() ;
	for ( int i = 0 ; i < p->childCount() ; i ++ ) {
		_dump(p->child(i), tab + 1) ;
	}
	qDebug() << "dump end---------" ;
}

void SessionManagerTreeModel::_dump(TreeItem *p, int tab)
{
	QString t ;
	for ( int i = 0 ; i < tab ; i ++ ) {
		t += " " ;
	}
	t += p->data() ;
	qDebug() << t ;

	for ( int i = 0 ; i < p->childCount() ; i ++ ) {
		_dump(p->child(i), tab + 1) ;
	}
}
