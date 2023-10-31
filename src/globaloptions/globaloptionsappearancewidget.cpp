#include "globaloptionsappearancewidget.h"
#include "ui_globaloptionsappearancewidget.h"

GlobalOptionsAppearanceWidget::GlobalOptionsAppearanceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsAppearanceWidget)
{
    ui->setupUi(this);
}

GlobalOptionsAppearanceWidget::~GlobalOptionsAppearanceWidget()
{
    delete ui;
}
