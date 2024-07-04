/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
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
#include "pluginviewerwidget.h"
#include "ui_pluginviewerwidget.h"

PluginViewerWidget::PluginViewerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PluginViewerWidget)
{
    ui->setupUi(this);

    m_homeWidget = new PluginViewerHomeWidget(this);
    addPlugin(qobject_cast<QWidget *>(m_homeWidget), "Home");
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
    retranslateUi();

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [this](const QString &pluginName) {
        if (m_plugins.contains(pluginName)) {
            ui->stackedWidget->setCurrentWidget(m_plugins.value(pluginName));
        } else if (ui->comboBox->currentIndex() == 0) {
            ui->stackedWidget->setCurrentWidget(m_homeWidget);
        }
    });
}

PluginViewerWidget::~PluginViewerWidget()
{
    delete ui;
}

void PluginViewerWidget::addPlugin(QWidget *pluginWidget, const QString &pluginName)
{
    m_plugins.insert(pluginName, pluginWidget);
    ui->stackedWidget->addWidget(pluginWidget);
    ui->comboBox->addItem(pluginName);
}

void PluginViewerWidget::retranslateUi(void) {
    ui->comboBox->setItemText(0, tr("Home"));
    m_homeWidget->retranslateUi();
    ui->retranslateUi(this);
}
