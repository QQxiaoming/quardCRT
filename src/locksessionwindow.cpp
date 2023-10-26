#include <QMessageBox>
#include "locksessionwindow.h"
#include "ui_locksessionwindow.h"

LockSessionWindow::LockSessionWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LockSessionWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

LockSessionWindow::~LockSessionWindow()
{
    delete ui;
}

void LockSessionWindow::buttonBoxAccepted(void)
{
    if(islock) {
        QString password = ui->passwordLineEdit->text();
        QString reenterPassword = ui->reenterPasswordLineEdit->text();
        if (password != reenterPassword)
        {
            QMessageBox::warning(this, tr("Error"), tr("Passwords do not match!"));
            this->show();
            return;
        } 
        if (password.isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Password cannot be empty!"));
            this->show();
            return;
        }
        ui->passwordLineEdit->clear();
        ui->reenterPasswordLineEdit->clear();
        emit this->sendLockSessionData(password, ui->lockAllSessionCheckBox->isChecked(), ui->lockAllSessionGroupCheckBox->isChecked());
    } else {
        QString password = ui->passwordLineEdit->text();
        if (password.isEmpty())
        {
            QMessageBox::warning(this, tr("Error"), tr("Password cannot be empty!"));
            this->show();
            return;
        }
        ui->passwordLineEdit->clear();
        emit this->sendUnLockSessionData(password, ui->lockAllSessionCheckBox->isChecked());
    }

    emit this->accepted();
}

void LockSessionWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}

void LockSessionWindow::showLock(void)
{
    ui->headerLabel->setText(tr("Enter the password that will be used to unlock the session:"));
    ui->lockAllSessionCheckBox->setText(tr("Lock all sessions"));
    ui->lockAllSessionCheckBox->setVisible(true);
    ui->lockAllSessionGroupCheckBox->setVisible(true);
    ui->passwordLineEdit->setVisible(true);
    ui->passwordLabel->setVisible(true);
    ui->reenterPasswordLineEdit->setVisible(true);
    ui->reenterPasswordLabel->setVisible(true);
    islock = true;
    this->show();
}

void LockSessionWindow::showUnlock(void)
{
    ui->headerLabel->setText(tr("Enter the password that was used to lock the session:"));
    ui->lockAllSessionCheckBox->setText(tr("Unlock all sessions"));
    ui->lockAllSessionCheckBox->setVisible(true);
    ui->lockAllSessionGroupCheckBox->setVisible(false);
    ui->passwordLineEdit->setVisible(true);
    ui->passwordLabel->setVisible(true);
    ui->reenterPasswordLineEdit->setVisible(false);
    ui->reenterPasswordLabel->setVisible(false);
    islock = false;
    this->show();
}
