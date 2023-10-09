#include <QSerialPortInfo>

#include "quickconnectwindow.h"
#include "ui_quickconnectwindow.h"

QuickConnectWindow::QuickConnectWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickConnectWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("Quick Connect"));
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    qRegisterMetaType<QuickConnectData>("QuickConnectData");
    
    ui->comboBoxProtocol->setCurrentIndex(0);
    comboBoxProtocolChanged(0);

    connect(ui->comboBoxProtocol, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxProtocolChanged(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

QuickConnectWindow::~QuickConnectWindow()
{
    delete ui;
}

void QuickConnectWindow::comboBoxProtocolChanged(int index)
{
    if(index == 0)
    {
        ui->labelHostname->setText(tr("Hostname"));
        ui->labelPort->setText(tr("Port"));
        ui->comboBoxHostname->setVisible(false);
        ui->lineEditHostname->setVisible(true);
        ui->labelPort->setVisible(true);
        ui->spinBoxPort->setVisible(true);
        ui->labelWebSocket->setVisible(true);
        ui->comboBoxWebSocket->setVisible(true);
        ui->labelDataBits->setVisible(false);
        ui->comboBoxDataBits->setVisible(false);
        ui->labelParity->setVisible(false);
        ui->comboBoxParity->setVisible(false);
        ui->labelStopBits->setVisible(false);
        ui->comboBoxStopBits->setVisible(false);
        ui->checkBoxFlowCtrl->setVisible(false);
        ui->checkBoxXEnable->setVisible(false);
        ui->lineEditHostname->setText("");
        ui->lineEditHostname->setPlaceholderText(tr("e.g. 127.0.0.1"));
        ui->spinBoxPort->setMinimum(0);
        ui->spinBoxPort->setMaximum(65535);
        ui->spinBoxPort->setValue(23);
        ui->comboBoxWebSocket->setCurrentIndex(0);
    } else if(index == 1) {
        ui->labelHostname->setText(tr("Port Name"));
        ui->labelPort->setText(tr("Baud Rate"));
        ui->comboBoxHostname->setVisible(true);
        ui->lineEditHostname->setVisible(false);
        ui->labelPort->setVisible(true);
        ui->spinBoxPort->setVisible(true);
        ui->labelWebSocket->setVisible(false);
        ui->comboBoxWebSocket->setVisible(false);
        ui->labelDataBits->setVisible(true);
        ui->comboBoxDataBits->setVisible(true);
        ui->labelParity->setVisible(true);
        ui->comboBoxParity->setVisible(true);
        ui->labelStopBits->setVisible(true);
        ui->comboBoxStopBits->setVisible(true);
        ui->checkBoxFlowCtrl->setVisible(true);
        ui->checkBoxXEnable->setVisible(true);
        ui->comboBoxDataBits->setCurrentIndex(3);
        ui->comboBoxParity->setCurrentIndex(0);
        ui->comboBoxStopBits->setCurrentIndex(0);
        ui->checkBoxFlowCtrl->setChecked(false);
        ui->checkBoxXEnable->setChecked(false);
        ui->comboBoxHostname->clear();
        QSerialPortInfo serialPortInfo;
        foreach(serialPortInfo, QSerialPortInfo::availablePorts()) {
            if(serialPortInfo.description().isEmpty()) {
                ui->comboBoxHostname->addItem(serialPortInfo.portName());
            } else {
                ui->comboBoxHostname->addItem(serialPortInfo.portName()+" - "+serialPortInfo.description());
            }
        }
        ui->comboBoxHostname->setCurrentIndex(0);
        ui->spinBoxPort->setMinimum(0);
        ui->spinBoxPort->setMaximum(INT_MAX);
        ui->spinBoxPort->setValue(115200);
    } else if(index == 2) {
        ui->labelHostname->setText(tr("Command"));
        ui->comboBoxHostname->setVisible(false);
        ui->lineEditHostname->setVisible(true);
        ui->labelPort->setVisible(false);
        ui->spinBoxPort->setVisible(false);
        ui->labelWebSocket->setVisible(false);
        ui->comboBoxWebSocket->setVisible(false);
        ui->labelDataBits->setVisible(false);
        ui->comboBoxDataBits->setVisible(false);
        ui->labelParity->setVisible(false);
        ui->comboBoxParity->setVisible(false);
        ui->labelStopBits->setVisible(false);
        ui->comboBoxStopBits->setVisible(false);
        ui->checkBoxFlowCtrl->setVisible(false);
        ui->checkBoxXEnable->setVisible(false);
        ui->lineEditHostname->setText("");
        ui->lineEditHostname->setPlaceholderText(tr("e.g. /bin/bash"));
    } else if(index == 3) {
        ui->labelHostname->setText(tr("Hostname"));
        ui->labelPort->setText(tr("Port"));
        ui->comboBoxHostname->setVisible(false);
        ui->lineEditHostname->setVisible(true);
        ui->labelPort->setVisible(true);
        ui->spinBoxPort->setVisible(true);
        ui->labelWebSocket->setVisible(false);
        ui->comboBoxWebSocket->setVisible(false);
        ui->labelDataBits->setVisible(false);
        ui->comboBoxDataBits->setVisible(false);
        ui->labelParity->setVisible(false);
        ui->comboBoxParity->setVisible(false);
        ui->labelStopBits->setVisible(false);
        ui->comboBoxStopBits->setVisible(false);
        ui->checkBoxFlowCtrl->setVisible(false);
        ui->checkBoxXEnable->setVisible(false);
        ui->lineEditHostname->setText("");
        ui->lineEditHostname->setPlaceholderText(tr("e.g. 127.0.0.1"));
        ui->spinBoxPort->setMinimum(0);
        ui->spinBoxPort->setMaximum(65535);
        ui->spinBoxPort->setValue(8080);
    }
}

void QuickConnectWindow::buttonBoxAccepted(void)
{
    QuickConnectData data;
    if(ui->comboBoxProtocol->currentIndex() == 0)
    {
        data.type = Telnet;
        data.TelnetData.hostname = ui->lineEditHostname->text();
        data.TelnetData.port = ui->spinBoxPort->value();
        data.TelnetData.webSocket = ui->comboBoxWebSocket->currentText();
        emit this->sendQuickConnectData(data);
    } else if(ui->comboBoxProtocol->currentIndex() == 1) {
        data.type = Serial;
        data.SerialData.portName = ui->comboBoxHostname->currentText().split(" - ")[0];
        data.SerialData.baudRate = ui->spinBoxPort->value();
        data.SerialData.dataBits = ui->comboBoxDataBits->currentText().toInt();
        data.SerialData.parity = ui->comboBoxParity->currentIndex();
        data.SerialData.stopBits = ui->comboBoxStopBits->currentText().toInt();
        data.SerialData.flowControl = ui->checkBoxFlowCtrl->isChecked();
        data.SerialData.xEnable = ui->checkBoxXEnable->isChecked();
        emit this->sendQuickConnectData(data);
    } else if(ui->comboBoxProtocol->currentIndex() == 2) {
        data.type = LocalShell;
        data.LocalShellData.command = ui->lineEditHostname->text();
        emit this->sendQuickConnectData(data);
    } else if(ui->comboBoxProtocol->currentIndex() == 3) {
        data.type = Raw;
        data.RawData.hostname = ui->lineEditHostname->text();
        data.RawData.port = ui->spinBoxPort->value();
        emit this->sendQuickConnectData(data);
    }

    emit this->accepted();
}

void QuickConnectWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}

void QuickConnectWindow::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    comboBoxProtocolChanged(ui->comboBoxProtocol->currentIndex());
    QDialog::showEvent(event);
}
