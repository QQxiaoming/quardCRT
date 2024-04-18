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
#include <QSerialPortInfo>

#include "quickconnectwindow.h"
#include "ui_quickconnectwindow.h"

QuickConnectWindow::QuickConnectWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickConnectWindow)
{
    ui->setupUi(this);
    lineEditPassword = new PasswordEdit(true,this);
    ui->horizontalLayout_5->addWidget(lineEditPassword);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    qRegisterMetaType<QuickConnectData>("QuickConnectData");
    
    ui->comboBoxProtocol->setCurrentIndex(0);
    comboBoxProtocolChanged(0);

    connect(ui->comboBoxProtocol, &QComboBox::currentIndexChanged, this, &QuickConnectWindow::comboBoxProtocolChanged);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QuickConnectWindow::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QuickConnectWindow::buttonBoxRejected);

    retranslateUi();
}

QuickConnectWindow::~QuickConnectWindow()
{
    delete ui;
}

void QuickConnectWindow::setProtocol(QuickConnectType index) {
    ui->comboBoxProtocol->setCurrentIndex((int)index);
}

void QuickConnectWindow::setSaveSession(bool enable) {
    ui->checkBoxSaveSession->setChecked(enable);
}

void QuickConnectWindow::setOpenInTab(bool enable) {
    ui->checkBoxOpenInTab->setChecked(enable);
}

void QuickConnectWindow::comboBoxProtocolChanged(int index)
{
    lineEditPassword->setPasswordShown(false);
    switch(index) {
        case Telnet: {
            ui->labelHostname->setText(tr("Hostname"));
            ui->labelPort->setText(tr("Port"));
            ui->comboBoxHostname->setVisible(false);
            ui->lineEditHostname->setVisible(true);
            ui->labelPort->setVisible(true);
            ui->spinBoxPort->setVisible(true);
            ui->spinBoxPort->setToolTip("");
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
            ui->labelUsername->setVisible(false);
            ui->lineEditUsername->setVisible(false);
            ui->labelPassword->setVisible(false);
            lineEditPassword->setVisible(false);
            ui->lineEditHostname->setText("");
            ui->lineEditHostname->setPlaceholderText(tr("e.g. 127.0.0.1"));
            ui->spinBoxPort->setMinimum(0);
            ui->spinBoxPort->setMaximum(65535);
            ui->spinBoxPort->setValue(23);
            ui->comboBoxWebSocket->setCurrentIndex(0);
            break;
        }
        case Serial: {
            ui->labelHostname->setText(tr("Port Name"));
            ui->labelPort->setText(tr("Baud Rate"));
            ui->comboBoxHostname->setVisible(true);
            ui->lineEditHostname->setVisible(false);
            ui->labelUsername->setVisible(false);
            ui->lineEditUsername->setVisible(false);
            ui->labelPassword->setVisible(false);
            lineEditPassword->setVisible(false);
            ui->labelPort->setVisible(true);
            ui->spinBoxPort->setVisible(true);
            ui->spinBoxPort->setToolTip(tr("e.g. 110, 300, 600, 1200, 2400, \n4800, 9600, 14400, 19200, 38400, \n56000, 57600, 115200, 128000, 256000, \n460800, 921600"));
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
            break;
        }
        case LocalShell: {
            ui->labelHostname->setText(tr("Command"));
            ui->comboBoxHostname->setVisible(false);
            ui->lineEditHostname->setVisible(true);
            ui->labelUsername->setVisible(false);
            ui->lineEditUsername->setVisible(false);
            ui->labelPassword->setVisible(false);
            lineEditPassword->setVisible(false);
            ui->labelPort->setVisible(false);
            ui->spinBoxPort->setVisible(false);
            ui->spinBoxPort->setToolTip("");
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
            break;
        }
        case Raw: {
            ui->labelHostname->setText(tr("Hostname"));
            ui->labelPort->setText(tr("Port"));
            ui->comboBoxHostname->setVisible(false);
            ui->lineEditHostname->setVisible(true);
            ui->labelUsername->setVisible(false);
            ui->lineEditUsername->setVisible(false);
            ui->labelPassword->setVisible(false);
            lineEditPassword->setVisible(false);
            ui->labelPort->setVisible(true);
            ui->spinBoxPort->setVisible(true);
            ui->spinBoxPort->setToolTip("");
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
            break;
        }
        case NamePipe:{
            ui->labelHostname->setText(tr("Pipe Name"));
            ui->comboBoxHostname->setVisible(false);
            ui->lineEditHostname->setVisible(true);
            ui->labelUsername->setVisible(false);
            ui->lineEditUsername->setVisible(false);
            ui->labelPassword->setVisible(false);
            lineEditPassword->setVisible(false);
            ui->labelPort->setVisible(false);
            ui->spinBoxPort->setVisible(false);
            ui->spinBoxPort->setToolTip("");
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
        #if defined(Q_OS_WIN)
            ui->lineEditHostname->setPlaceholderText(tr("e.g. \\\\\\.\\pipe\\namedpipe"));
        #else 
            ui->lineEditHostname->setPlaceholderText(tr("e.g. /tmp/socket"));
        #endif
            break;
        }
        case SSH2: {
            ui->labelHostname->setText(tr("Hostname"));
            ui->labelPort->setText(tr("Port"));
            ui->comboBoxHostname->setVisible(false);
            ui->lineEditHostname->setVisible(true);
            ui->labelUsername->setVisible(true);
            ui->lineEditUsername->setVisible(true);
            ui->labelPassword->setVisible(true);
            lineEditPassword->setVisible(true);
            ui->labelPort->setVisible(true);
            ui->spinBoxPort->setVisible(true);
            ui->spinBoxPort->setToolTip("");
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
            ui->spinBoxPort->setValue(22);
            break;
        }
        case VNC: {
            ui->labelHostname->setText(tr("Hostname"));
            ui->labelPort->setText(tr("Port"));
            ui->comboBoxHostname->setVisible(false);
            ui->lineEditHostname->setVisible(true);
            ui->labelPassword->setVisible(true);
            lineEditPassword->setVisible(true);
            ui->labelPort->setVisible(true);
            ui->spinBoxPort->setVisible(true);
            ui->spinBoxPort->setToolTip("");
            ui->labelWebSocket->setVisible(false);
            ui->labelUsername->setVisible(false);
            ui->lineEditUsername->setVisible(false);
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
            ui->spinBoxPort->setValue(5900);
            break;
        }
        default:
            break;
    }
}

void QuickConnectWindow::buttonBoxAccepted(void)
{
    QuickConnectData data;
    data.type = (QuickConnectType)ui->comboBoxProtocol->currentIndex();
    data.saveSession = ui->checkBoxSaveSession->isChecked();
    data.openInTab = ui->checkBoxOpenInTab->isChecked();
    switch(ui->comboBoxProtocol->currentIndex()) {
        case Telnet:
            data.TelnetData.hostname = ui->lineEditHostname->text();
            data.TelnetData.port = ui->spinBoxPort->value();
            data.TelnetData.webSocket = ui->comboBoxWebSocket->currentText();
            emit this->sendQuickConnectData(data);
            break;
        case Serial:
            data.SerialData.portName = ui->comboBoxHostname->currentText().split(" - ")[0];
            data.SerialData.baudRate = ui->spinBoxPort->value();
            data.SerialData.dataBits = ui->comboBoxDataBits->currentText().toInt();
            data.SerialData.parity = ui->comboBoxParity->currentIndex();
            data.SerialData.stopBits = ui->comboBoxStopBits->currentText().toInt();
            data.SerialData.flowControl = ui->checkBoxFlowCtrl->isChecked();
            data.SerialData.xEnable = ui->checkBoxXEnable->isChecked();
            emit this->sendQuickConnectData(data);
            break;
        case LocalShell:
            data.LocalShellData.command = ui->lineEditHostname->text();
            emit this->sendQuickConnectData(data);
            break;
        case Raw:
            data.RawData.hostname = ui->lineEditHostname->text();
            data.RawData.port = ui->spinBoxPort->value();
            emit this->sendQuickConnectData(data);
            break;
        case NamePipe:
            data.NamePipeData.pipeName = ui->lineEditHostname->text();
            emit this->sendQuickConnectData(data);
            break;
        case SSH2:
            data.SSH2Data.hostname = ui->lineEditHostname->text();
            data.SSH2Data.port = ui->spinBoxPort->value();
            data.SSH2Data.username = ui->lineEditUsername->text();
            data.SSH2Data.password = lineEditPassword->text();
            emit this->sendQuickConnectData(data);
            break;
        case VNC:
            data.VNCData.hostname = ui->lineEditHostname->text();
            data.VNCData.port = ui->spinBoxPort->value();
            data.VNCData.password = lineEditPassword->text();
            emit this->sendQuickConnectData(data);
            break;
        default:
            break;
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
    QDialog::showEvent(event);
}

void QuickConnectWindow::reset(void)
{
    comboBoxProtocolChanged(ui->comboBoxProtocol->currentIndex());
}

void QuickConnectWindow::setQuickConnectData(QuickConnectData data) 
{
    ui->comboBoxProtocol->setCurrentIndex((int)data.type);
    comboBoxProtocolChanged((int)data.type);
    switch(data.type) {
        case Telnet:
            ui->lineEditHostname->setText(data.TelnetData.hostname);
            ui->spinBoxPort->setValue(data.TelnetData.port);
            ui->comboBoxWebSocket->setCurrentText(data.TelnetData.webSocket);
            break;
        case Serial:
            ui->comboBoxHostname->setCurrentText(data.SerialData.portName);
            ui->spinBoxPort->setValue(data.SerialData.baudRate);
            ui->comboBoxDataBits->setCurrentText(QString::number(data.SerialData.dataBits));
            ui->comboBoxParity->setCurrentIndex(data.SerialData.parity);
            ui->comboBoxStopBits->setCurrentText(QString::number(data.SerialData.stopBits));
            ui->checkBoxFlowCtrl->setChecked(data.SerialData.flowControl);
            ui->checkBoxXEnable->setChecked(data.SerialData.xEnable);
            break;
        case LocalShell:
            ui->lineEditHostname->setText(data.LocalShellData.command);
            break;
        case Raw:
            ui->lineEditHostname->setText(data.RawData.hostname);
            ui->spinBoxPort->setValue(data.RawData.port);
            break;
        case NamePipe:
            ui->lineEditHostname->setText(data.NamePipeData.pipeName);
            break;
        case SSH2:
            ui->lineEditHostname->setText(data.SSH2Data.hostname);
            ui->spinBoxPort->setValue(data.SSH2Data.port);
            ui->lineEditUsername->setText(data.SSH2Data.username);
            lineEditPassword->setText(data.SSH2Data.password);
            break;
        case VNC:
            ui->lineEditHostname->setText(data.VNCData.hostname);
            ui->spinBoxPort->setValue(data.VNCData.port);
            lineEditPassword->setText(data.VNCData.password);
            break;
        default:
            break;
    }
}

void QuickConnectWindow::retranslateUi()
{
    setWindowTitle(tr("Quick Connect"));
    ui->retranslateUi(this);
}
