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

#include "globalsetting.h"
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

    ui->comboBoxPluginsPath->addItem(pathShorten(QApplication::applicationDirPath() + "/plugins/QuardCRT"),QApplication::applicationDirPath() + "/plugins/QuardCRT");
    ui->comboBoxPluginsPath->setItemData(0, QApplication::applicationDirPath() + "/plugins/QuardCRT", Qt::ToolTipRole);
    GlobalSetting settings;
    QString pluginsPath = settings.value("Global/Options/UserPluginsPath", "").toString();
    if(!pluginsPath.isEmpty()) {
        ui->comboBoxPluginsPath->addItem(pathShorten(pluginsPath),pluginsPath);
        ui->comboBoxPluginsPath->setItemData(1, pluginsPath, Qt::ToolTipRole);
        ui->comboBoxPluginsPath->setCurrentText(pathShorten(pluginsPath));
    }

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("API Version") << tr("Enable") << tr("Check Update"));
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 80);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setColumnWidth(3, 80);
    ui->tableWidget->setColumnWidth(4, 180);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->pushButtonInstallPlugin, &QPushButton::clicked, this, [&](){
        QString pluginDir = ui->comboBoxPluginsPath->itemData(ui->comboBoxPluginsPath->currentIndex(), Qt::UserRole).toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(pluginDir));
    });
}

PluginInfoWindow::~PluginInfoWindow()
{
    delete ui;
}

void PluginInfoWindow::addPluginInfo(PluginInterface *plugin, const QString &tooltip, uint32_t apiVersion, bool enable, bool readOnly, const QString &website)
{
    addPluginInfo(plugin->name(), plugin->version(), tooltip, apiVersion, enable, readOnly, website);
}

void PluginInfoWindow::addPluginInfo(const QString &name, const QString &version, const QString &tooltip, uint32_t apiVersion, bool enable, bool readOnly, const QString &website)
{
    uint32_t i = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(i + 1);
    ui->tableWidget->setItem(i, 0, new QTableWidgetItem(name));
    ui->tableWidget->item(i, 0)->setToolTip(tooltip);
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(version));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(apiVersion?QString::number(apiVersion):""));
    QTableWidgetItem *itemEnable = new QTableWidgetItem();
    itemEnable->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
    itemEnable->setFlags(itemEnable->flags() & ~Qt::ItemIsEditable);
    if(readOnly) itemEnable->setFlags(itemEnable->flags() & ~Qt::ItemIsUserCheckable);
    ui->tableWidget->setItem(i, 3, itemEnable);
    if(!readOnly) {
        connect(ui->tableWidget, &QTableWidget::itemChanged, this, [&,name,itemEnable](QTableWidgetItem * in) {
            if(in == itemEnable)
                emit pluginEnableStateChanged(name, in->checkState() == Qt::Checked);
        });
    }
    if(!website.isEmpty()) {
        QPushButton *itemWebsite = new QPushButton(tr("Website"));
        if(website.isEmpty()) {
            itemWebsite->setEnabled(false);
        } else {
            itemWebsite->setToolTip(website);
            connect(itemWebsite, &QPushButton::clicked, this, [website](){
                QDesktopServices::openUrl(QUrl(website));
            });
        }
        ui->tableWidget->setCellWidget(i, 4, itemWebsite);
    } else {
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(tr("Unknown")));
    }
}

void PluginInfoWindow::retranslateUi(void) {
    ui->retranslateUi(this);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("API Version") << tr("Enable") << tr("Check Update"));
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

QString PluginInfoWindow::pathShorten(const QString &path) {
    QString ret = path;
    if(ret.size() > 40) {
        ret = "..." + ret.mid(ret.size() - 40);
    }
    return ret;
}

void PluginInfoWindow::showEvent(QShowEvent *event)
{
    GlobalSetting settings;
    QString pluginsPath = settings.value("Global/Options/UserPluginsPath", "").toString();
    if(!pluginsPath.isEmpty()) {
        int ret = ui->comboBoxPluginsPath->findText(pathShorten(pluginsPath));
        if(ret != -1) {
            if(pluginsPath != ui->comboBoxPluginsPath->itemData(ret, Qt::UserRole).toString()) {
                ui->comboBoxPluginsPath->clear();
                ui->comboBoxPluginsPath->addItem(pathShorten(QApplication::applicationDirPath() + "/plugins/QuardCRT"),QApplication::applicationDirPath() + "/plugins/QuardCRT");
                ui->comboBoxPluginsPath->setItemData(0, QApplication::applicationDirPath() + "/plugins/QuardCRT", Qt::ToolTipRole);
                ui->comboBoxPluginsPath->addItem(pathShorten(pluginsPath),pluginsPath);
                ui->comboBoxPluginsPath->setItemData(1, pluginsPath, Qt::ToolTipRole);
                ui->comboBoxPluginsPath->setCurrentText(pathShorten(pluginsPath));
            }
        }
    }
    QDialog::showEvent(event);
}
