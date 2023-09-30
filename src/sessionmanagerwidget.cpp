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

    label = new QLabel(this);
    ui->verticalLayout->insertWidget(0,label);

    toolBar = new QToolBar(this);
    ui->verticalLayout->insertWidget(1,toolBar);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->setIconSize(QSize(16,16));

    retranslateUi();
}

SessionManagerWidget::~SessionManagerWidget()
{
    delete ui;
}

void SessionManagerWidget::addActionOnToolBar(QAction *action)
{
    toolBar->addAction(action);
}

void SessionManagerWidget::retranslateUi()
{
    label->setText(tr("Session Manager"));
    ui->retranslateUi(this);
}
