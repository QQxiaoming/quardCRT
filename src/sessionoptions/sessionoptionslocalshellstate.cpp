#include <QStandardItemModel>
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
    ui->tableViewInfo->setModel(model);
    ui->tableViewInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewInfo->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewInfo->verticalHeader()->hide();
}

SessionOptionsLocalShellState::~SessionOptionsLocalShellState()
{
    delete ui;
}
