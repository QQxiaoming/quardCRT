#include "createpublickey.h"
#include "ui_createpublickey.h"

CreatePublicKey::CreatePublicKey(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreatePublicKey)
{
    ui->setupUi(this);
}

CreatePublicKey::~CreatePublicKey()
{
    delete ui;
}
