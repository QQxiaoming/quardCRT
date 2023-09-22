#include "quickconnectwindow.h"
#include "ui_quickconnectwindow.h"

QuickConnectWindow::QuickConnectWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickConnectWindow)
{
    ui->setupUi(this);

    ui->comboBoxProtocol->addItem("Telnet");
    ui->comboBoxProtocol->addItem("Serial");
    ui->comboBoxProtocol->addItem("Local Shell");
    ui->comboBoxProtocol->addItem("Raw");
    ui->comboBoxProtocol->setCurrentIndex(0);

    ui->lineEditHostname->setText("");

    ui->spinBoxPort->setMinimum(0);
    ui->spinBoxPort->setMaximum(65535);
    ui->spinBoxPort->setValue(23);

    ui->comboBoxWebSocket->addItem("None");
    ui->comboBoxWebSocket->addItem("Insecure");
    ui->comboBoxWebSocket->addItem("Secure");
    ui->comboBoxWebSocket->setCurrentIndex(0);

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

QuickConnectWindow::~QuickConnectWindow()
{
    delete ui;
}

void QuickConnectWindow::buttonBoxAccepted(void)
{
    emit sendQuickConnectData(ui->lineEditHostname->text(),
                              ui->spinBoxPort->value(),
                              ui->comboBoxProtocol->currentText(),
                              ui->comboBoxWebSocket->currentText());
    emit this->accepted();
}

void QuickConnectWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}
