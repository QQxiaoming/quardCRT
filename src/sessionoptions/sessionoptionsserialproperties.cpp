#include "sessionoptionsserialproperties.h"
#include "ui_sessionoptionsserialproperties.h"

SessionOptionsSerialProperties::SessionOptionsSerialProperties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsSerialProperties)
{
    ui->setupUi(this);
}

SessionOptionsSerialProperties::~SessionOptionsSerialProperties()
{
    delete ui;
}
