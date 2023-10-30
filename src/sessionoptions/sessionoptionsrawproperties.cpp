#include "sessionoptionsrawproperties.h"
#include "ui_sessionoptionsrawproperties.h"

SessionOptionsRawProperties::SessionOptionsRawProperties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsRawProperties)
{
    ui->setupUi(this);
}

SessionOptionsRawProperties::~SessionOptionsRawProperties()
{
    delete ui;
}
