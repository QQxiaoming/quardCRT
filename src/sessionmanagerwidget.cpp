#include <QToolBar>
#include <QLabel>

#include "qfonticon.h"
#include "treemodel.h"

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
    toolBar->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    tree = new SessionTreeWindow(this);
    ui->verticalLayout->addWidget(tree);

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
    ui->lineEdit->setPlaceholderText(tr("Filter by folder/session name"));
    ui->retranslateUi(this);
    tree->retranslateUi();
}

void SessionManagerWidget::addSession(QString str, int type)
{
    tree->addSession(str, type);
}

void SessionManagerWidget::removeSession(QString str)
{
    tree->removeSession(str);
}

bool SessionManagerWidget::checkSession(QString str)
{
    return tree->checkSession(str);
}
