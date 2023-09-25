#include <QMenu>
#include <QTabBar>
#include <QContextMenuEvent>
#include "sessiontab.h"

SessionTab::SessionTab(QWidget *parent) 
    : QTabWidget(parent) {

}

SessionTab::~SessionTab() {

}

void SessionTab::contextMenuEvent(QContextMenuEvent *event) {
    int index = tabBar()->tabAt(event->pos());
    emit showContextMenu(index);
    Q_UNUSED(event);
}
