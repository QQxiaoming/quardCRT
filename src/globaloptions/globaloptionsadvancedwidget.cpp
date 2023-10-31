#include "globaloptionsadvancedwidget.h"
#include "ui_globaloptionsadvancedwidget.h"

GlobalOptionsAdvancedWidget::GlobalOptionsAdvancedWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsAdvancedWidget)
{
    ui->setupUi(this);
}

GlobalOptionsAdvancedWidget::~GlobalOptionsAdvancedWidget()
{
    delete ui;
}
