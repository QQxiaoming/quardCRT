#include "onestepwindow.h"
#include "ui_onestepwindow.h"

OneStepWindow::OneStepWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OneStepWindow)
{
    ui->setupUi(this);
    lineEditPassword = new PasswordEdit(true, this);
    ui->horizontalLayoutPassword->addWidget(lineEditPassword);
}

OneStepWindow::~OneStepWindow()
{
    delete ui;
}

void OneStepWindow::setConfig(const QString &stepName, const QString &userName, const QString &password, const int &port) {
    ui->lineEditStepName->setText(stepName);
    ui->lineEditUserName->setText(userName);
    lineEditPassword->setText(password);
    ui->spinBoxPort->setValue(port);
}

void OneStepWindow::getConfig(QString &stepName, QString &userName, QString &password, int &port) {
    stepName = ui->lineEditStepName->text();
    userName = ui->lineEditUserName->text();
    password = lineEditPassword->text();
    port = ui->spinBoxPort->value();
}
