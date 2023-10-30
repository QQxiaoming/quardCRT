#include <QSplitter>
#include <QTreeView>
#include <QLabel>
#include <QStandardItemModel>

#include "sessionoptionswindow.h"
#include "ui_sessionoptionswindow.h"

SessionOptionsWindow::SessionOptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionOptionsWindow)
{
    ui->setupUi(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    ui->horizontalLayout->addWidget(splitter);
    QTreeView *treeView = new QTreeView(this);
    treeView->setHeaderHidden(true);
    QStandardItemModel *model = new QStandardItemModel(treeView);
    treeView->setModel(model);
    QStandardItem *item = new QStandardItem("Global");
    model->appendRow(item);
    splitter->addWidget(treeView);
    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    widget->setLayout(new QVBoxLayout(widget));
    splitter->setSizes(QList<int>() << 1 << 100);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);
}

SessionOptionsWindow::~SessionOptionsWindow()
{
    delete ui;
}
