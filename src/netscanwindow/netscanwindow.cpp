/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include <QList>
#include <QClipboard>

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
    ui->tableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *copyAction = new QAction(tr("Copy"), ui->tableWidget);
    ui->tableWidget->addAction(copyAction);
    connect(copyAction, &QAction::triggered, [&]() {
        QItemSelectionModel *selection = ui->tableWidget->selectionModel();
        QModelIndexList indexes = selection->selectedIndexes();

        if (indexes.size() < 1)
            return;

        QString selected_text;
        for (int i = 0; i < indexes.size(); ++i) {
            QModelIndex index = indexes.at(i);
            QString text = ui->tableWidget->model()->data(index).toString();
            selected_text.append(text);
            if (i < indexes.size() - 1)
                selected_text.append("\t");
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(selected_text);
    });
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
                connect(telnet,&QTelnet::error,this,[telnet](QAbstractSocket::SocketError err){
                    telnet->deleteLater();
                    Q_UNUSED(err);
                });
                connect(telnet,&QTelnet::newData,this,[&,ip,port,telnet](const char *buff, int len){
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
