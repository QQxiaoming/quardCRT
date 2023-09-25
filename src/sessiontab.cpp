#include <QMenu>
#include "sessiontab.h"

SessionTab::SessionTab(QWidget *parent) 
    : QTabWidget(parent) {

}

SessionTab::~SessionTab() {

}

void SessionTab::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = new QMenu(this);
    menu->addAction("New Session");
    menu->addAction("Duplicate Session");
    menu->addAction("Rename Session");
    menu->addAction("Change Session Icon");
    menu->addAction("Change Session Color");
    menu->addAction("Delete Session");

    menu->move(cursor().pos());
    menu->show();

    Q_UNUSED(event);
}
