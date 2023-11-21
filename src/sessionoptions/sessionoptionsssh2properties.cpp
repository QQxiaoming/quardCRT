#include "sessionoptionsssh2properties.h"
#include "ui_sessionoptionsssh2properties.h"

SessionOptionsSsh2Properties::SessionOptionsSsh2Properties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsSsh2Properties)
{
    ui->setupUi(this);
}

SessionOptionsSsh2Properties::~SessionOptionsSsh2Properties()
{
    delete ui;
}
