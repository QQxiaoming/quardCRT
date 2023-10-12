#include <QMenu>
#include <QTabBar>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPen>
#include <QApplication>
#include "sessiontab.h"

EmptyTabWidget::EmptyTabWidget(QWidget *parent) 
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setEnabled(false);
    setLayout(new QVBoxLayout);
    layout()->addWidget(label);

    easterEggs = new QLabel(this);
    easterEggs->setAlignment(Qt::AlignCenter);
    layout()->addWidget(easterEggs);
    easterEggs->setVisible(false);
    easterEggs->setEnabled(false);
}

EmptyTabWidget::~EmptyTabWidget() {
}

void EmptyTabWidget::retranslateUi(void) {
    label->setText(tr("No session"));
}

void EmptyTabWidget::mousePressEvent(QMouseEvent *event) {
    static int count = 0;
    static int hitCount = 0;
    if(event->button() == Qt::LeftButton) {
        count++;
        if(count == 5) {
            label->setVisible(false);
            easterEggs->setVisible(true);
            hitCount++;
            switch(hitCount) {
                case 1:
                    easterEggs->setText("This is a boring game!");
                    break;
                case 2:
                    easterEggs->setText("You are so boring!");
                    break;
                case 3:
                    easterEggs->setText("I'm tired of you!");
                    break;
                case 4:
                    easterEggs->setText("I'm going to sleep!");
                    break;
                case 5:
                    easterEggs->setText("Goodbye!");
                    hitCount = 0;
                    break;                    
            }
            count = 0;
        } else {
            label->setVisible(true);
            easterEggs->setVisible(false);
        }
    }
}


SessionTab::SessionTab(QWidget *parent) 
    : FancyTabWidget(parent) {

    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);
    setTabBarAutoHide(true);
    setTabTextEditable(false);
    setAddTabButtonVisible(false);

    emptyTab = new EmptyTabWidget(this);
    retranslateUi();
    addTab(emptyTab,"");
    tabBar()->setTabVisible(0,false);
    tabBar()->setTabEnabled(0,false);

    connect(this,&SessionTab::currentChanged,this,[&](int index) {
        setAddTabButtonVisible(count() != 0);
        QWidget *w = widget(index);
        if(w) w->setFocus();
    });
}

SessionTab::~SessionTab() {
}

int SessionTab::count(void) {
    int count = FancyTabWidget::count() - 1;
    return count < 0 ? 0 : count;
} 

void SessionTab::setCurrentIndex(int index) {
    FancyTabWidget::setCurrentIndex(index+1);
}

void SessionTab::contextMenuEvent(QContextMenuEvent *event) {
    int index = tabBar()->tabAt(event->pos());
    index = index == 0 ? -1 : index;
    emit showContextMenu(index,event->globalPos());
    Q_UNUSED(event);
}

void SessionTab::retranslateUi(void) {
    emptyTab->retranslateUi();
}
