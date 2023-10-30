#include "sessionoptionstelnetproperties.h"
#include "ui_sessionoptionstelnetproperties.h"

SessionOptionsTelnetProperties::SessionOptionsTelnetProperties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsTelnetProperties)
{
    ui->setupUi(this);
}

SessionOptionsTelnetProperties::~SessionOptionsTelnetProperties()
{
    delete ui;
}
