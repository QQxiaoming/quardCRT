#include <QSerialPortInfo>

#include "quickconnectwindow.h"
#include "ui_quickconnectwindow.h"

QuickConnectWindow::QuickConnectWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickConnectWindow)
{
    ui->setupUi(this);

    setWindowTitle("Quick Connect");

    qRegisterMetaType<QuickConnectData>("QuickConnectData");
    
    ui->comboBoxProtocol->addItem("Telnet");
    ui->comboBoxProtocol->addItem("Serial");
    ui->comboBoxProtocol->addItem("Local Shell");
    ui->comboBoxProtocol->addItem("Raw");
    ui->comboBoxProtocol->setCurrentIndex(0);

    ui->lineEditHostname->setText("");
    ui->comboBoxHostname->setVisible(false);

    ui->spinBoxPort->setMinimum(0);
    ui->spinBoxPort->setMaximum(65535);
    ui->spinBoxPort->setValue(23);

    ui->comboBoxWebSocket->addItem("None");
    ui->comboBoxWebSocket->addItem("Insecure");
    ui->comboBoxWebSocket->addItem("Secure");
    ui->comboBoxWebSocket->setCurrentIndex(0);

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
        ui->labelHostname->setText("Hostname");
        ui->labelPort->setText("Port");
        ui->comboBoxHostname->setVisible(false);
        ui->lineEditHostname->setVisible(true);
        ui->labelPort->setVisible(true);
        ui->spinBoxPort->setVisible(true);
        ui->labelWebSocket->setVisible(true);
        ui->comboBoxWebSocket->setVisible(true);
        ui->lineEditHostname->setText("");
        ui->spinBoxPort->setMinimum(0);
        ui->spinBoxPort->setMaximum(65535);
        ui->spinBoxPort->setValue(23);
        ui->comboBoxWebSocket->setCurrentIndex(0);
    } else if(index == 1) {
        ui->labelHostname->setText("Port Name");
        ui->labelPort->setText("Baud Rate");
        ui->comboBoxHostname->setVisible(true);
        ui->lineEditHostname->setVisible(false);
        ui->labelPort->setVisible(true);
        ui->spinBoxPort->setVisible(true);
        ui->labelWebSocket->setVisible(false);
        ui->comboBoxWebSocket->setVisible(false);
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
        ui->labelHostname->setText("Command");
        ui->comboBoxHostname->setVisible(false);
        ui->lineEditHostname->setVisible(true);
        ui->labelPort->setVisible(false);
        ui->spinBoxPort->setVisible(false);
        ui->labelWebSocket->setVisible(false);
        ui->comboBoxWebSocket->setVisible(false);
        ui->lineEditHostname->setText("");
    } else if(index == 3) {
        ui->labelHostname->setText("Hostname");
        ui->labelPort->setText("Port");
        ui->comboBoxHostname->setVisible(false);
        ui->lineEditHostname->setVisible(true);
        ui->labelPort->setVisible(true);
        ui->spinBoxPort->setVisible(true);
        ui->labelWebSocket->setVisible(false);
        ui->comboBoxWebSocket->setVisible(false);
        ui->lineEditHostname->setText("");
        ui->spinBoxPort->setMinimum(0);
        ui->spinBoxPort->setMaximum(65535);
        ui->spinBoxPort->setValue(8080);
    }
}


void QuickConnectWindow::buttonBoxAccepted(void)
{
    QuickConnectData data;
    if(ui->comboBoxProtocol->currentText() == "Telnet")
    {
        data.type = Telnet;
        data.TelnetData.hostname = ui->lineEditHostname->text();
        data.TelnetData.port = ui->spinBoxPort->value();
        data.TelnetData.webSocket = ui->comboBoxWebSocket->currentText();
        emit this->sendQuickConnectData(data);
    } else if(ui->comboBoxProtocol->currentText() == "Serial") {
        data.type = Serial;
        data.SerialData.portName = ui->comboBoxHostname->currentText().split(" - ")[0];
        data.SerialData.baudRate = ui->spinBoxPort->value();
        //data.SerialData.dataBits = ui->comboBoxDataBits->currentText().toInt();
        //data.SerialData.parity = ui->comboBoxParity->currentIndex();
        //data.SerialData.stopBits = ui->comboBoxStopBits->currentIndex();
        //data.SerialData.flowControl = ui->comboBoxFlowControl->currentIndex();
        emit this->sendQuickConnectData(data);
    } else if(ui->comboBoxProtocol->currentText() == "Local Shell") {
        data.type = LocalShell;
        data.LocalShellData.command = ui->lineEditHostname->text();
        emit this->sendQuickConnectData(data);
    } else if(ui->comboBoxProtocol->currentText() == "Raw") {
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
