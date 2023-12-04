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
#include <QSplitter>
#include <QTreeView>
#include <QLabel>
#include <QStringListModel>
#include <QSerialPortInfo>
#include <QFileInfo>
#include <QStandardItemModel>

#include "sessionoptionswindow.h"

#include "ui_sessionoptionswindow.h"
#include "ui_sessionoptionsgeneralwidget.h"

#include "ui_sessionoptionstelnetproperties.h"
#include "ui_sessionoptionsserialproperties.h"
#include "ui_sessionoptionslocalshellproperties.h"
#include "ui_sessionoptionsnamepipeproperties.h"
#include "ui_sessionoptionsrawproperties.h"
#include "ui_sessionoptionsssh2properties.h"
#include "ui_sessionoptionsvncproperties.h"

#include "ui_sessionoptionslocalshellstate.h"

SessionOptionsWindow::SessionOptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionOptionsWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    ui->horizontalLayout->addWidget(splitter);
    QTreeView *treeView = new QTreeView(this);
    treeView->setMinimumWidth(140);
    treeView->setHeaderHidden(true);
    model = new QStringListModel(treeView);
    treeView->setModel(model);
    splitter->addWidget(treeView);
    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    widget->setLayout(new QVBoxLayout(widget));
    widget->layout()->setContentsMargins(0,0,0,0);
    splitter->setSizes(QList<int>() << 140 << 500);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

    sessionOptionsGeneralWidget = new SessionOptionsGeneralWidget(widget);
    widget->layout()->addWidget(sessionOptionsGeneralWidget);
    sessionOptionsTelnetProperties = new SessionOptionsTelnetProperties(widget);
    widget->layout()->addWidget(sessionOptionsTelnetProperties);
    sessionOptionsSerialProperties = new SessionOptionsSerialProperties(widget);
    widget->layout()->addWidget(sessionOptionsSerialProperties);
    sessionOptionsLocalShellProperties = new SessionOptionsLocalShellProperties(widget);
    widget->layout()->addWidget(sessionOptionsLocalShellProperties);
    sessionOptionsNamePipeProperties = new SessionOptionsNamePipeProperties(widget);
    widget->layout()->addWidget(sessionOptionsNamePipeProperties);
    sessionOptionsRawProperties = new SessionOptionsRawProperties(widget);
    widget->layout()->addWidget(sessionOptionsRawProperties);
    sessionOptionsSSH2Properties = new SessionOptionsSsh2Properties(widget);
    widget->layout()->addWidget(sessionOptionsSSH2Properties);
    sessionOptionsVNCProperties = new SessionOptionsVNCProperties(widget);
    widget->layout()->addWidget(sessionOptionsVNCProperties);

    sessionOptionsLocalShellState = new SessionOptionsLocalShellState(widget);
    widget->layout()->addWidget(sessionOptionsLocalShellState);

    sessionOptionsGeneralWidget->setVisible(true);
    setactiveProperties(-1);
    setactiveState(-1);

    retranslateUi();

    connect(treeView, &QTreeView::clicked, [=](const QModelIndex &index) {
        if (index.row() == 0) {
            sessionOptionsGeneralWidget->setVisible(true);
            setactiveProperties(-1);
            setactiveState(-1);
        } else if (index.row() == 1) {
            sessionOptionsGeneralWidget->setVisible(false);
            setactiveState(-1);
            setactiveProperties(sessionOptionsGeneralWidget->ui->comboBoxProtocol->currentIndex());
        } else if (index.row() == 2) {
            sessionOptionsGeneralWidget->setVisible(false);
            setactiveProperties(-1);
            setactiveState(sessionOptionsGeneralWidget->ui->comboBoxProtocol->currentIndex());
        }
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SessionOptionsWindow::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SessionOptionsWindow::buttonBoxRejected);
}

SessionOptionsWindow::~SessionOptionsWindow()
{
    delete ui;
}

void SessionOptionsWindow::retranslateUi()
{
    model->setStringList(QStringList() << tr("General") << tr("Properties") << tr("State"));
    ui->retranslateUi(this);
    sessionOptionsGeneralWidget->ui->retranslateUi(this);
    sessionOptionsTelnetProperties->ui->retranslateUi(this);
    sessionOptionsSerialProperties->ui->retranslateUi(this);
    sessionOptionsLocalShellProperties->ui->retranslateUi(this);
    sessionOptionsNamePipeProperties->ui->retranslateUi(this);
    sessionOptionsRawProperties->ui->retranslateUi(this);
    sessionOptionsSSH2Properties->ui->retranslateUi(this);
    sessionOptionsVNCProperties->ui->retranslateUi(this);
    sessionOptionsLocalShellState->ui->retranslateUi(this);
    sessionOptionsLocalShellState->ui->treeViewInfo->model()->setHeaderData(1, Qt::Horizontal, tr("Name"));
}

void SessionOptionsWindow::setactiveProperties(int index)
{
    sessionOptionsTelnetProperties->setVisible(false);
    sessionOptionsSerialProperties->setVisible(false);
    sessionOptionsLocalShellProperties->setVisible(false);
    sessionOptionsNamePipeProperties->setVisible(false);
    sessionOptionsRawProperties->setVisible(false);
    sessionOptionsSSH2Properties->setVisible(false);
    sessionOptionsVNCProperties->setVisible(false);

    if(index == -1) {
        return;
    }
    switch(index) {
    case 0:
        sessionOptionsTelnetProperties->setVisible(true);
        break;
    case 1:
        sessionOptionsSerialProperties->setVisible(true);
        break;
    case 2:
        sessionOptionsLocalShellProperties->setVisible(true);
        break;
    case 3:
        sessionOptionsRawProperties->setVisible(true);
        break;
    case 4:
        sessionOptionsNamePipeProperties->setVisible(true);
        break;
    case 5:
        sessionOptionsSSH2Properties->setVisible(true);
        break;
    case 6:
        sessionOptionsVNCProperties->setVisible(true);
        break;
    }
}

void SessionOptionsWindow::setactiveState(int index)
{
    sessionOptionsLocalShellState->setVisible(false);

    if(index == -1) {
        return;
    }
    switch(index) {
    case 2:
        sessionOptionsLocalShellState->setVisible(true);
        break;
    }
}

void SessionOptionsWindow::setSessionProperties(QString name, QuickConnectWindow::QuickConnectData data)
{
    currentSessionName = name;
    sessionOptionsLocalShellState->ui->treeViewInfo->model()->removeRows(0, sessionOptionsLocalShellState->ui->treeViewInfo->model()->rowCount());
    sessionOptionsGeneralWidget->ui->comboBoxProtocol->setCurrentIndex(data.type);
    sessionOptionsGeneralWidget->ui->lineEditName->setText(name);
    switch(data.type) {
    case QuickConnectWindow::Telnet:
        sessionOptionsTelnetProperties->ui->lineEditHostname->setText(data.TelnetData.hostname);
        sessionOptionsTelnetProperties->ui->spinBoxPort->setValue(data.TelnetData.port);
        sessionOptionsTelnetProperties->ui->comboBoxWebSocket->setCurrentText(data.TelnetData.webSocket);
        break;
    case QuickConnectWindow::Serial: {
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
    case QuickConnectWindow::LocalShell:
        sessionOptionsLocalShellProperties->ui->lineEditCommand->setText(data.LocalShellData.command);
        break;
    case QuickConnectWindow::Raw:
        sessionOptionsRawProperties->ui->lineEditHostname->setText(data.RawData.hostname);
        sessionOptionsRawProperties->ui->spinBoxPort->setValue(data.RawData.port);
        break;
    case QuickConnectWindow::NamePipe:
        sessionOptionsNamePipeProperties->ui->lineEditPipeName->setText(data.NamePipeData.pipeName);
        break;
    case QuickConnectWindow::SSH2:
        sessionOptionsSSH2Properties->ui->lineEditHostname->setText(data.SSH2Data.hostname);
        sessionOptionsSSH2Properties->ui->spinBoxPort->setValue(data.SSH2Data.port);
        sessionOptionsSSH2Properties->ui->lineEditUserName->setText(data.SSH2Data.username);
        sessionOptionsSSH2Properties->lineEditPassword->setText(data.SSH2Data.password);
        break;
    case QuickConnectWindow::VNC:
        sessionOptionsVNCProperties->ui->lineEditHostname->setText(data.VNCData.hostname);
        sessionOptionsVNCProperties->ui->spinBoxPort->setValue(data.VNCData.port);
        sessionOptionsVNCProperties->lineEditPassword->setText(data.VNCData.password);
        break;
    }
}

void SessionOptionsWindow::setSessionState(SessionsWindow::StateInfo state)
{
    switch(state.type) {
        case SessionsWindow::LocalShell: {
            QStandardItemModel *model = (QStandardItemModel *)sessionOptionsLocalShellState->ui->treeViewInfo->model();
            std::function<void(IPtyProcess::pidTree_t, QStandardItem *)> addNode = [&](IPtyProcess::pidTree_t tree, QStandardItem *parent) {
                qint64 pid = tree.pidInfo.pid;
                QString path = tree.pidInfo.command;
                QFileInfo fileInfo(path);
                QString name = fileInfo.fileName();
                QList<QStandardItem *> items = { new QStandardItem(), new QStandardItem() };
                items[0]->setData(pid, Qt::DisplayRole);
                items[1]->setData(name, Qt::DisplayRole);
                items[1]->setData(path, Qt::ToolTipRole);
                parent->appendRow(items);
                foreach(IPtyProcess::pidTree_t child, tree.children) {
                    addNode(child, items[0]);
                }
            };
            addNode(state.localShell.tree, model->invisibleRootItem());
            sessionOptionsLocalShellState->ui->treeViewInfo->expandToDepth(2);
            switch(state.state) {
                case SessionsWindow::Connected:
                    sessionOptionsLocalShellState->ui->labelState->setPixmap(QPixmap(QFontIcon::icon(QChar(0xf0c1), Qt::green).pixmap(16,16)));
                    break;
                case SessionsWindow::Disconnected:
                case SessionsWindow::Error:
                    sessionOptionsLocalShellState->ui->labelState->setPixmap(QPixmap(QFontIcon::icon(QChar(0xf127), Qt::red).pixmap(16,16)));
                    break;
                case SessionsWindow::Locked:
                    sessionOptionsLocalShellState->ui->labelState->setPixmap(QPixmap(QFontIcon::icon(QChar(0xf084), Qt::yellow).pixmap(16,16)));
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void SessionOptionsWindow::buttonBoxAccepted(void)
{
    if(currentSessionName.isEmpty()) {
        return;
    }
    QuickConnectWindow::QuickConnectData data;
    data.type = (QuickConnectWindow::QuickConnectType)sessionOptionsGeneralWidget->ui->comboBoxProtocol->currentIndex();
    switch(data.type) {
    case QuickConnectWindow::Telnet:
        data.TelnetData.hostname = sessionOptionsTelnetProperties->ui->lineEditHostname->text();
        data.TelnetData.port = sessionOptionsTelnetProperties->ui->spinBoxPort->value();
        data.TelnetData.webSocket = sessionOptionsTelnetProperties->ui->comboBoxWebSocket->currentText();
        break;
    case QuickConnectWindow::Serial:
        data.SerialData.portName = sessionOptionsSerialProperties->ui->comboBoxPortName->currentText().split(" - ")[0];
        data.SerialData.baudRate = sessionOptionsSerialProperties->ui->spinBoxBaudRate->value();
        data.SerialData.dataBits = sessionOptionsSerialProperties->ui->comboBoxDataBits->currentText().toInt();
        data.SerialData.parity = sessionOptionsSerialProperties->ui->comboBoxParity->currentText().toInt();
        data.SerialData.stopBits = sessionOptionsSerialProperties->ui->comboBoxStopBits->currentText().toInt();
        data.SerialData.flowControl = sessionOptionsSerialProperties->ui->checkBoxFlowControl->isChecked();
        data.SerialData.xEnable = sessionOptionsSerialProperties->ui->checkBoxXEnable->isChecked();
        break;
    case QuickConnectWindow::LocalShell:
        data.LocalShellData.command = sessionOptionsLocalShellProperties->ui->lineEditCommand->text();
        break;
    case QuickConnectWindow::Raw:
        data.RawData.hostname = sessionOptionsRawProperties->ui->lineEditHostname->text();
        data.RawData.port = sessionOptionsRawProperties->ui->spinBoxPort->value();
        break;
    case QuickConnectWindow::NamePipe:
        data.NamePipeData.pipeName = sessionOptionsNamePipeProperties->ui->lineEditPipeName->text();
        break;
    case QuickConnectWindow::SSH2:
        data.SSH2Data.hostname = sessionOptionsSSH2Properties->ui->lineEditHostname->text();
        data.SSH2Data.port = sessionOptionsSSH2Properties->ui->spinBoxPort->value();
        data.SSH2Data.username = sessionOptionsSSH2Properties->ui->lineEditUserName->text();
        data.SSH2Data.password = sessionOptionsSSH2Properties->lineEditPassword->text();
        break;
    case QuickConnectWindow::VNC:
        data.VNCData.hostname = sessionOptionsVNCProperties->ui->lineEditHostname->text();
        data.VNCData.port = sessionOptionsVNCProperties->ui->spinBoxPort->value();
        data.VNCData.password = sessionOptionsVNCProperties->lineEditPassword->text();
        break;
    }
    emit sessionPropertiesChanged(currentSessionName, data, sessionOptionsGeneralWidget->ui->lineEditName->text());
    emit this->accepted();
}

void SessionOptionsWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}

void SessionOptionsWindow::showEvent(QShowEvent *event)
{
    retranslateUi();
    QDialog::showEvent(event);
}
