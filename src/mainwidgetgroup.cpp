#include "mainwidgetgroup.h"
#include "sessionswindow.h"

MainWidgetGroup::MainWidgetGroup(QWidget *parent)
    : QObject(parent)
{
    splitter = new QSplitter(Qt::Vertical, parent);
    sessionTab = new SessionTab(parent);
    commandWindow = new CommandWindow(parent);

    splitter->setHandleWidth(1);
    splitter->addWidget(sessionTab);
    splitter->addWidget(commandWindow);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,true);
    splitter->setSizes(QList<int>() << 1 << 0);
}

MainWidgetGroup::~MainWidgetGroup()
{
}

