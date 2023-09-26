#include <QMenu>
#include <QTabBar>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPen>
#include <QApplication>
#include <QLabel>
#include "sessiontab.h"

SessionTab::SessionTab(QWidget *parent) 
    : QTabWidget(parent) {

    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);
    setTabBarAutoHide(true);

    label = new QLabel(this);
    retranslateUi();
    label->setAlignment(Qt::AlignCenter);
    addTab(label,"");
    tabBar()->setTabVisible(0,false);
    setTabEnabled(0, false);
}

SessionTab::~SessionTab() {

}

int SessionTab::count(void) {
    int count = QTabWidget::count() - 1;
    return count < 0 ? 0 : count;
} 

void SessionTab::setCurrentIndex(int index) {
    QTabWidget::setCurrentIndex(index+1);
}

void SessionTab::contextMenuEvent(QContextMenuEvent *event) {
    int index = tabBar()->tabAt(event->pos());
    index = index == 0 ? -1 : index;
    emit showContextMenu(index);
    Q_UNUSED(event);
}

void SessionTab::retranslateUi(void) {
    label->setText(tr("No session"));
}
