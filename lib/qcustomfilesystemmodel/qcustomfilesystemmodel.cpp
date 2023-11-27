/*
 * This file is part of the https://github.com/QQxiaoming/QCustomFileSystemModel.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */ 
#include <QDir>
#include <QFileInfo>
#include <QIcon>

#include "qfonticon.h"
#include "qcustomfilesystemmodel.h"

QCustomFileSystemItem::QCustomFileSystemItem(const QString &path, QCustomFileSystemItem *parent) {
    m_path = path;
    m_parentItem = parent;
}

QCustomFileSystemItem::~QCustomFileSystemItem() {
    qDeleteAll(m_childItems);
}

void QCustomFileSystemItem::appendChild(QCustomFileSystemItem *child) {
    m_childItems.append(child);
}

void QCustomFileSystemItem::removeChild(int row) {
    m_childItems.removeAt(row);
}

QCustomFileSystemItem *QCustomFileSystemItem::child(int row) {
    return m_childItems.value(row);
}

int QCustomFileSystemItem::childCount() const {
    return m_childItems.count();
}

int QCustomFileSystemItem::columnCount() const {
    return 4;
}

QVariant QCustomFileSystemItem::data() const {
    return m_path;
}

int QCustomFileSystemItem::row() const {
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<QCustomFileSystemItem*>(this));
    return 0;
}

QCustomFileSystemItem *QCustomFileSystemItem::parent() {
    return m_parentItem;
}


QCustomFileSystemModel::QCustomFileSystemModel(QObject *parent) 
    : QAbstractItemModel(parent) {
    m_rootPath = "";
}

QCustomFileSystemModel::~QCustomFileSystemModel() {
    delete m_rootItem;
}

QModelIndex QCustomFileSystemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    QCustomFileSystemItem *parentItem;
    if (!parent.isValid() || !indexValid(parent))
        parentItem = m_rootItem;
    else
        parentItem = static_cast<QCustomFileSystemItem*>(parent.internalPointer());
    if(parentItem == nullptr)
        return QModelIndex();
    QCustomFileSystemItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex QCustomFileSystemModel::parent(const QModelIndex &child) const {
    if(m_rootItem == nullptr)
        return QModelIndex();
    if (!child.isValid())
        return QModelIndex();
    if(!indexValid(child))
        return QModelIndex();
    QCustomFileSystemItem *childItem = static_cast<QCustomFileSystemItem*>(child.internalPointer());
    QCustomFileSystemItem *parentItem = childItem->parent();
    if(parentItem == nullptr)
        return QModelIndex();
    if (parentItem == m_rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int QCustomFileSystemModel::rowCount(const QModelIndex &parent) const {
    QCustomFileSystemItem *parentItem;
    if (parent.column() > 0)
        return 0;
    if (!parent.isValid() || !indexValid(parent))
        parentItem = m_rootItem;
    else
        parentItem = static_cast<QCustomFileSystemItem*>(parent.internalPointer());
    if(parentItem == nullptr)
        return 0;
    return parentItem->childCount();
}

int QCustomFileSystemModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid() && indexValid(parent))
        return static_cast<QCustomFileSystemItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}

QVariant QCustomFileSystemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    if (!indexValid(index))
        return QVariant();
    if (role != Qt::DisplayRole && role != Qt::DecorationRole)
        return QVariant();
    QCustomFileSystemItem *item = static_cast<QCustomFileSystemItem*>(index.internalPointer());
    if(item->data().toString() == "")
        return QVariant();
    if (role == Qt::DecorationRole && index.column() == 0) {
        if(item->isDir())
            return QFontIcon::icon(QChar(0xf07b));
        else
            return QFontIcon::icon(QChar(0xf15b));
    }
    if (role == Qt::DisplayRole) {
        switch (index.column())
        {
        case 0:
            return item->data().toString().split(separator()).last();
        case 1:
            return item->isDir() ? tr("Directory") : tr("File");
        case 2:
            if(item->isDir()) {
                if(item->childCount() == 1 && item->child(0)->data().toString() == "")
                    return tr("Loading...");
                else
                    return QString::number(item->childCount());
            } else {
                if( item->size() <= 1024) {
                    return QString("%1 B").arg(item->size());
                } else if ( item->size() <= 1024 * 1024 ) {
                    return QString::number(item->size() / 1024.0, 'f', 2) + QString(" KB");
                } else if ( item->size() <= 1024 * 1024 * 1024 ) {
                    return QString::number(item->size() / (1024.0 * 1024.0), 'f', 2) + QString(" MB");
                } else {
                    return QString::number(item->size() / (1024.0 * 1024.0 * 1024.0), 'f', 2) + QString(" GB");
                }
            }
        case 3:
            return item->lastModified();
        }
    }
    return QVariant();
}

QVariant QCustomFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Type");
        case 2:
            return tr("Size");
        case 3:
            return tr("Last Modified");
        }
    }
    return QVariant();
}

void QCustomFileSystemModel::fetchMore(const QModelIndex &parent) {
    if (!parent.isValid())
        return;
    if (!indexValid(parent))
        return;
    QCustomFileSystemItem *parentItem = static_cast<QCustomFileSystemItem*>(parent.internalPointer());
    if(parentItem->childCount() != 1)
        return;
    if(parentItem->child(0)->data().toString() != "")
        return;
    QStringList entries = pathEntryList(parentItem->data().toString());
    if(entries.count() == 0)
        return;
    //remove dummy item
    parentItem->removeChild(0);
    QList<QCustomFileSystemItem*> dirItems;
    QList<QCustomFileSystemItem*> fileItems;
    for (int i = 0; i < entries.count(); ++i) {
        QString childPath = parentItem->data().toString() + separator() + entries.at(i);
        QCustomFileSystemItem *childItem = new QCustomFileSystemItem(childPath, parentItem);
        bool isDir = false;
        uint64_t size = 0;
        QDateTime lastModified = QDateTime();
        pathInfo(childPath, isDir, size, lastModified);
        childItem->setIsDir(isDir);
        childItem->setLastModified(lastModified);
        if (isDir) {
            //QStringList childEntries = pathEntryList(childPath);
            //childItem->setSize(childEntries.count());
            dirItems.append(childItem);
            //add dummy item
            QCustomFileSystemItem *dummyItem = new QCustomFileSystemItem("", childItem);
            childItem->appendChild(dummyItem);
        } else {
            childItem->setSize(size);
            fileItems.append(childItem);
        }
    }
    foreach(QCustomFileSystemItem *item, dirItems) {
        parentItem->appendChild(item);
    }
    foreach(QCustomFileSystemItem *item, fileItems) {
        parentItem->appendChild(item);
    }
}

bool QCustomFileSystemModel::canFetchMore(const QModelIndex &parent) const {
    if (!parent.isValid())
        return false;
    if (!indexValid(parent))
        return false;
    QCustomFileSystemItem *parentItem = static_cast<QCustomFileSystemItem*>(parent.internalPointer());
    if(parentItem->childCount() != 1)
        return false;
    if(parentItem->child(0)->data().toString() != "")
        return false;
    return true;
}

QModelIndex QCustomFileSystemModel::setRootPath(const QString &path) {
    beginResetModel();
    delete m_rootItem;
    m_rootItem = nullptr;
    QCustomFileSystemItem *rootItem = new QCustomFileSystemItem(path);
    m_rootPath = path;
    QStringList rootEntries = pathEntryList(m_rootPath);
    QList<QCustomFileSystemItem*> dirItems;
    QList<QCustomFileSystemItem*> fileItems;
    for (int i = 0; i < rootEntries.count(); ++i) {
        QString childPath = separator() + rootEntries.at(i);
        if(path != separator())
            childPath = path + childPath;
        QCustomFileSystemItem *childItem = new QCustomFileSystemItem(childPath, rootItem);
        bool isDir = false;
        uint64_t size = 0;
        QDateTime lastModified = QDateTime();
        pathInfo(childPath, isDir, size, lastModified);
        childItem->setIsDir(isDir);
        childItem->setLastModified(lastModified);
        if (isDir) {
            //QStringList childEntries = pathEntryList(childPath);
            //childItem->setSize(childEntries.count());
            dirItems.append(childItem);
            //add dummy item
            QCustomFileSystemItem *dummyItem = new QCustomFileSystemItem("", childItem);
            childItem->appendChild(dummyItem);
        } else {
            childItem->setSize(size);
            fileItems.append(childItem);
        }
    }
    foreach(QCustomFileSystemItem *item, dirItems) {
        rootItem->appendChild(item);
    }
    foreach(QCustomFileSystemItem *item, fileItems) {
        rootItem->appendChild(item);
    }
    QModelIndex index = createIndex(0, 0, rootItem);
    m_rootItem = rootItem;
    endResetModel();
    return index;
}

QString QCustomFileSystemModel::rootPath() {
    return m_rootPath;
}

QString QCustomFileSystemModel::filePath(const QModelIndex &index) {
    if (!index.isValid())
        return "";
    if (!indexValid(index))
        return "";
    QCustomFileSystemItem *item = static_cast<QCustomFileSystemItem*>(index.internalPointer());
    return item->data().toString();
}

QNativeFileSystemModel::QNativeFileSystemModel(QObject *parent) 
    : QCustomFileSystemModel(parent) {
}

QNativeFileSystemModel::~QNativeFileSystemModel() {
}

QString QNativeFileSystemModel::separator() const {
    return QDir::separator();
}

QStringList QNativeFileSystemModel::pathEntryList(const QString &path) {
    QDir dir(path);
    if(m_hideFiles)
        return dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
    else
        return dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
}

void QNativeFileSystemModel::pathInfo(QString path, bool &isDir, uint64_t &size, QDateTime &lastModified) {
    QFileInfo info(path);
    isDir = info.isDir();
    size = info.size();
    lastModified = info.lastModified();
}
