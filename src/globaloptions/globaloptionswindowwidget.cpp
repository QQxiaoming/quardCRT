#include "globaloptionswindowwidget.h"
#include "ui_globaloptionswindowwidget.h"

GlobalOptionsWindowWidget::GlobalOptionsWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsWindowWidget)
{
    ui->setupUi(this);
}

GlobalOptionsWindowWidget::~GlobalOptionsWindowWidget()
{
    delete ui;
}
