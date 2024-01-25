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
#include <QDesktopServices>
#include <QUrl>

#include "plugininfowindow.h"
#include "ui_plugininfowindow.h"

PluginInfoWindow::PluginInfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginInfoWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    ui->lineEditAPIVersion->setText(QString::number(PLUGIN_API_VERSION));
    ui->lineEditAPIVersion->setReadOnly(true);
    ui->lineEditAPIVersion->setFocusPolicy(Qt::NoFocus);

    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("API Version") << tr("Enable"));
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 80);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setColumnWidth(3, 80);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->pushButtonInstallPlugin, &QPushButton::clicked, this, [&](){
        QString pluginDir = QApplication::applicationDirPath() + "/plugins/QuardCRT";
        QDesktopServices::openUrl(QUrl::fromLocalFile(pluginDir));
    });
}

PluginInfoWindow::~PluginInfoWindow()
{
    delete ui;
}

void PluginInfoWindow::addPluginInfo(PluginInterface *plugin, uint32_t apiVersion, bool enable, bool readOnly)
{
    addPluginInfo(plugin->name(), plugin->version(), apiVersion, enable, readOnly);
}

void PluginInfoWindow::addPluginInfo(QString name, QString version, uint32_t apiVersion, bool enable, bool readOnly)
{
    uint32_t i = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(i + 1);
    ui->tableWidget->setItem(i, 0, new QTableWidgetItem(name));
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(version));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(apiVersion?QString::number(apiVersion):""));
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    if(readOnly) item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    ui->tableWidget->setItem(i, 3, item);
    if(!readOnly) {
        connect(ui->tableWidget, &QTableWidget::itemChanged, this, [&,name,item](QTableWidgetItem * in) {
            if(in == item)
                emit pluginEnableStateChanged(name, in->checkState() == Qt::Checked);
        });
    }
}

void PluginInfoWindow::retranslateUi(void) {
    ui->retranslateUi(this);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("API Version") << tr("Enable"));
}

QList<uint32_t> PluginInfoWindow::supportAPIVersionList(void) {
    QList<uint32_t> list;
    switch(PLUGIN_API_VERSION) {
        case 1:
            list.append(1);
            break;
        case 2:
            list.append(2);
            break;
        default:
            break;
    }
    return list;
}
