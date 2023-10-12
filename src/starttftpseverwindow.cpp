#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>

#include "starttftpseverwindow.h"
#include "ui_starttftpseverwindow.h"

StartTftpSeverWindow::StartTftpSeverWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartTftpSeverWindow)
{
    ui->setupUi(this);
    ui->uploadLineEdit->setText(QDir::homePath());
    ui->downloadLineEdit->setText(QDir::homePath());

    connect(ui->uploadToolButton, &QToolButton::clicked, this, [=](){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                QDir::homePath(),
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        ui->uploadLineEdit->setText(dir);
    });
    connect(ui->downloadToolButton, &QToolButton::clicked, this, [=](){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                QDir::homePath(),
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        ui->downloadLineEdit->setText(dir);
    });

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

StartTftpSeverWindow::~StartTftpSeverWindow()
{
    delete ui;
}

void StartTftpSeverWindow::retranslateUi(void)
{
    ui->retranslateUi(this);
}

void StartTftpSeverWindow::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}

void StartTftpSeverWindow::buttonBoxAccepted(void)
{
    QFileInfo upDirInfo(ui->uploadLineEdit->text());
    QFileInfo downDirInfo(ui->downloadLineEdit->text());
    if(!upDirInfo.isDir() || !downDirInfo.isDir()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a valid directory!"));
        return;
    }
    emit setTftpInfo(ui->spinBox->value(), ui->uploadLineEdit->text(), ui->downloadLineEdit->text());
    
    emit this->accepted();
}

void StartTftpSeverWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}
