#include "globaloptionsterminalwidget.h"
#include "ui_globaloptionsterminalwidget.h"

GlobalOptionsTerminalWidget::GlobalOptionsTerminalWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsTerminalWidget)
{
    ui->setupUi(this);
}

GlobalOptionsTerminalWidget::~GlobalOptionsTerminalWidget()
{
    delete ui;
}
