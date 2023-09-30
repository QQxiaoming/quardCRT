#include <QToolBar>
#include <QLabel>

#include "qfonticon.h"

#include "sessionmanagerwidget.h"
#include "ui_sessionmanagerwidget.h"

SessionManagerWidget::SessionManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionManagerWidget)
{
    ui->setupUi(this);

    QLabel *label = new QLabel(this);
    ui->verticalLayout->insertWidget(0,label);
    label->setText("Session Manager");

    toolBar = new QToolBar(this);
    ui->verticalLayout->insertWidget(1,toolBar);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(16,16));
}

SessionManagerWidget::~SessionManagerWidget()
{
    delete ui;
}

void SessionManagerWidget::addActionOnToolBar(QAction *action)
{
    toolBar->addAction(action);
}
