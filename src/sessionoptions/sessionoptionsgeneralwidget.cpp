#include "sessionoptionsgeneralwidget.h"
#include "ui_sessionoptionsgeneralwidget.h"

SessionOptionsGeneralWidget::SessionOptionsGeneralWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsGeneralWidget)
{
    ui->setupUi(this);
}

SessionOptionsGeneralWidget::~SessionOptionsGeneralWidget()
{
    delete ui;
}
