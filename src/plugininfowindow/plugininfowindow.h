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
#ifndef PLUGININFOWINDOW_H
#define PLUGININFOWINDOW_H

#include <QDialog>
#include <QList>
#include "plugininterface.h"

namespace Ui {
class PluginInfoWindow;
}

class PluginInfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PluginInfoWindow(QWidget *parent = nullptr);
    ~PluginInfoWindow();
    void addPluginInfo(PluginInterface *plugin, const QString &tooltip, uint32_t apiVersion, bool enable, bool readOnly, const QString &website = QString());
    void addPluginInfo(const QString &name, const QString &version, const QString &tooltip, uint32_t apiVersion, bool enable, bool readOnly, const QString &website = QString());
    void retranslateUi(void);

    static QList<uint32_t> supportAPIVersionList(void);

signals:
    void pluginEnableStateChanged(QString name, bool enable);

private:
    Ui::PluginInfoWindow *ui;
};

#endif // PLUGININFOWINDOW_H
