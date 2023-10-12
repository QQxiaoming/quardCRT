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
