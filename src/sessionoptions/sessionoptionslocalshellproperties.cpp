#include "sessionoptionslocalshellproperties.h"
#include "ui_sessionoptionslocalshellproperties.h"

SessionOptionsLocalShellProperties::SessionOptionsLocalShellProperties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsLocalShellProperties)
{
    ui->setupUi(this);
}

SessionOptionsLocalShellProperties::~SessionOptionsLocalShellProperties()
{
    delete ui;
}
