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
    if(index != -1) {
        emit showContextMenu(index);
        return;
    }

    Q_UNUSED(event);
}
