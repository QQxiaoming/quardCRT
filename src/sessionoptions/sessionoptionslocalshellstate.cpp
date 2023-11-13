#include <QStandardItemModel>
#include <QHeaderView>
#include "sessionoptionslocalshellstate.h"
#include "ui_sessionoptionslocalshellstate.h"

SessionOptionsLocalShellState::SessionOptionsLocalShellState(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsLocalShellState)
{
    ui->setupUi(this);

    QStandardItemModel *model = new QStandardItemModel(0, 2, this);
    model->setHeaderData(0, Qt::Horizontal, "PID");
    model->setHeaderData(1, Qt::Horizontal, tr("Name"));
    ui->treeViewInfo->setModel(model);
    ui->treeViewInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeViewInfo->setAnimated(true);
    ui->treeViewInfo->setSortingEnabled(true);
    ui->treeViewInfo->setAllColumnsShowFocus(true);
    ui->treeViewInfo->setUniformRowHeights(true);
    ui->treeViewInfo->setWordWrap(false);
    ui->treeViewInfo->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

SessionOptionsLocalShellState::~SessionOptionsLocalShellState()
{
    delete ui;
}
