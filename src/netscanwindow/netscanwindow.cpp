#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include <QList>

#include "QTelnet.h"
#include "netscanwindow.h"
#include "ui_netscanwindow.h"

NetScanWindow::NetScanWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetScanWindow)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "IP" << "Port" << "Status");
    ui->tableWidget->setColumnWidth(0,150);
    ui->tableWidget->setColumnWidth(1,50);
    ui->tableWidget->setColumnWidth(2,150);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

NetScanWindow::~NetScanWindow()
{
    delete ui;
}

void NetScanWindow::addRow(QString ip, QString port, QString status)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row,0,new QTableWidgetItem(ip));
    ui->tableWidget->setItem(row,1,new QTableWidgetItem(port));
    ui->tableWidget->setItem(row,2,new QTableWidgetItem(status));
}

void NetScanWindow::setScanPort(int port)
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    QList<QHostAddress> addressList = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, addressList) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
            QString gateway = address.toString().split(".").at(0) + "." + address.toString().split(".").at(1) + "." + address.toString().split(".").at(2) + ".";
            for(int i=1;i<255;i++) {
                QString ip = gateway + QString::number(i);
                QTelnet *telnet = new QTelnet(this);
                telnet->connectToHost(ip,port);
                connect(telnet,&QTelnet::error,this,[=](QAbstractSocket::SocketError err){
                    telnet->deleteLater();
                    Q_UNUSED(err);
                });
                connect(telnet,&QTelnet::newData,this,[=](const char *buff, int len){
                    QString data = QString::fromUtf8(buff,len);
                    if(data.contains("SSH-2.0")) {
                        addRow(ip,QString::number(port),"open");
                        telnet->disconnectFromHost();
                        telnet->deleteLater();
                    } else {
                        telnet->disconnectFromHost();
                        telnet->deleteLater();
                    }
                });
            }
        }
    }
}
