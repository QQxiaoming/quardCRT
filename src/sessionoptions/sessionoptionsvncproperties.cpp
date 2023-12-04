#include "sessionoptionsvncproperties.h"
#include "ui_sessionoptionsvncproperties.h"

SessionOptionsVNCProperties::SessionOptionsVNCProperties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionOptionsVNCProperties)
{
    ui->setupUi(this);
    lineEditPassword = new PasswordEdit(true, this);
    ui->horizontalLayout_2->addWidget(lineEditPassword);
}

SessionOptionsVNCProperties::~SessionOptionsVNCProperties()
{
    delete ui;
}
