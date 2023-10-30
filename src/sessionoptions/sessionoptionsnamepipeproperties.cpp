#include "sessionoptionsnamepipeproperties.h"
#include "ui_sessionoptionsnamepipeproperties.h"

SessionOptionsNamePipeProperties::SessionOptionsNamePipeProperties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsNamePipeProperties)
{
    ui->setupUi(this);
}

SessionOptionsNamePipeProperties::~SessionOptionsNamePipeProperties()
{
    delete ui;
}
