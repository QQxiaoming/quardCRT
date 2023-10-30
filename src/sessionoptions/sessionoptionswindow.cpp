#include <QSplitter>
#include <QTreeView>
#include <QLabel>
#include <QStringListModel>
#include <QSerialPortInfo>

#include "sessionoptionswindow.h"
#include "ui_sessionoptionswindow.h"
#include "ui_sessionoptionsgeneralwidget.h"
#include "ui_sessionoptionstelnetproperties.h"
#include "ui_sessionoptionsserialproperties.h"

SessionOptionsWindow::SessionOptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionOptionsWindow)
{
    ui->setupUi(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    ui->horizontalLayout->addWidget(splitter);
    QTreeView *treeView = new QTreeView(this);
    treeView->setHeaderHidden(true);
    QStringListModel *model = new QStringListModel(treeView);
    treeView->setModel(model);
    splitter->addWidget(treeView);
    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    widget->setLayout(new QVBoxLayout(widget));
    widget->layout()->setContentsMargins(0,0,0,0);
    splitter->setSizes(QList<int>() << 1 << 100);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

    model->setStringList(QStringList() << tr("General") << tr("Properties"));
    sessionOptionsGeneralWidget = new SessionOptionsGeneralWidget(widget);
    widget->layout()->addWidget(sessionOptionsGeneralWidget);
    sessionOptionsTelnetProperties = new SessionOptionsTelnetProperties(widget);
    widget->layout()->addWidget(sessionOptionsTelnetProperties);
    sessionOptionsSerialProperties = new SessionOptionsSerialProperties(widget);
    widget->layout()->addWidget(sessionOptionsSerialProperties);

    sessionOptionsGeneralWidget->setVisible(true);
    sessionOptionsTelnetProperties->setVisible(false);
    sessionOptionsSerialProperties->setVisible(false);

    connect(treeView, &QTreeView::clicked, [=](const QModelIndex &index) {
        if (index.row() == 0) {
            sessionOptionsGeneralWidget->setVisible(true);
            sessionOptionsTelnetProperties->setVisible(false);
            sessionOptionsSerialProperties->setVisible(false);
        } else if (index.row() == 1) {
            sessionOptionsGeneralWidget->setVisible(false);
            switch(sessionOptionsGeneralWidget->ui->comboBoxProtocol->currentIndex()) {
            case 0:
                sessionOptionsTelnetProperties->setVisible(true);
                sessionOptionsSerialProperties->setVisible(false);
                break;
            case 1:
                sessionOptionsTelnetProperties->setVisible(false);
                sessionOptionsSerialProperties->setVisible(true);
                break;
            }
        }
    });
}

SessionOptionsWindow::~SessionOptionsWindow()
{
    delete ui;
}

void SessionOptionsWindow::setSessionProperties(QString name, QuickConnectWindow::QuickConnectData data)
{
    sessionOptionsGeneralWidget->ui->comboBoxProtocol->setCurrentIndex(data.type);
    switch(data.type) {
    case QuickConnectWindow::Telnet:
        sessionOptionsTelnetProperties->ui->lineEditHostname->setText(data.TelnetData.hostname);
        sessionOptionsTelnetProperties->ui->spinBoxPort->setValue(data.TelnetData.port);
        sessionOptionsTelnetProperties->ui->comboBoxWebSocket->setCurrentText(data.TelnetData.webSocket);
        break;
    case QuickConnectWindow::Serial:
        sessionOptionsSerialProperties->ui->comboBoxPortName->clear();
        QSerialPortInfo serialPortInfo;
        bool match = false;
        foreach(serialPortInfo, QSerialPortInfo::availablePorts()) {
            if(serialPortInfo.description().isEmpty()) {
                sessionOptionsSerialProperties->ui->comboBoxPortName->addItem(serialPortInfo.portName());
            } else {
                sessionOptionsSerialProperties->ui->comboBoxPortName->addItem(serialPortInfo.portName()+" - "+serialPortInfo.description());
            }
            if(serialPortInfo.portName() == data.SerialData.portName) {
                sessionOptionsSerialProperties->ui->comboBoxPortName->setCurrentIndex(sessionOptionsSerialProperties->ui->comboBoxPortName->count()-1);
                match = true;
            }
        }
        if(!match) {
            sessionOptionsSerialProperties->ui->comboBoxPortName->addItem(data.SerialData.portName);
            sessionOptionsSerialProperties->ui->comboBoxPortName->setCurrentText(data.SerialData.portName);
        }
        sessionOptionsSerialProperties->ui->spinBoxBaudRate->setValue(data.SerialData.baudRate);
        sessionOptionsSerialProperties->ui->comboBoxDataBits->setCurrentText(QString::number(data.SerialData.dataBits));
        sessionOptionsSerialProperties->ui->comboBoxParity->setCurrentText(QString::number(data.SerialData.parity));
        sessionOptionsSerialProperties->ui->comboBoxStopBits->setCurrentText(QString::number(data.SerialData.stopBits));
        sessionOptionsSerialProperties->ui->checkBoxFlowControl->setChecked(data.SerialData.flowControl);
        sessionOptionsSerialProperties->ui->checkBoxXEnable->setChecked(data.SerialData.xEnable);
        break;
    }
}
