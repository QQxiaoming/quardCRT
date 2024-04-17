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
#include "globaloptionsappearancewidget.h"
#include "ui_globaloptionsappearancewidget.h"

#include <QColorDialog>

GlobalOptionsAppearanceWidget::GlobalOptionsAppearanceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalOptionsAppearanceWidget)
{
    ui->setupUi(this);

    ui->comBoxColorSchemesBak->setDisabled(false);
    ui->comBoxColorSchemesBak->setEditable(true);
    ui->comBoxColorSchemesBak->setInsertPolicy(QComboBox::NoInsert);

    colorButtons.append(ui->pushButtonColor1);
    colorButtons.append(ui->pushButtonColor2);
    colorButtons.append(ui->pushButtonColor3);
    colorButtons.append(ui->pushButtonColor4);
    colorButtons.append(ui->pushButtonColor5);
    colorButtons.append(ui->pushButtonColor6);
    colorButtons.append(ui->pushButtonColor7);
    colorButtons.append(ui->pushButtonColor8);
    colorButtons.append(ui->pushButtonColor9);
    colorButtons.append(ui->pushButtonColor10);
    colorButtons.append(ui->pushButtonColor11);
    colorButtons.append(ui->pushButtonColor12);
    colorButtons.append(ui->pushButtonColor13);
    colorButtons.append(ui->pushButtonColor14);
    colorButtons.append(ui->pushButtonColor15);
    colorButtons.append(ui->pushButtonColor16);
    colorButtons.append(ui->pushButtonColor17);
    colorButtons.append(ui->pushButtonColor18);
    colorButtons.append(ui->pushButtonColor19);
    colorButtons.append(ui->pushButtonColor20);

    foreach (QPushButton *button, colorButtons) {
        button->setAutoFillBackground(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFlat(true);
        connect(button, &QPushButton::clicked, this, [&, button] {
            QColor color = QColorDialog::getColor(button->palette().color(QPalette::Button), this, tr("Select color"));
            if (color.isValid()) {
                QPalette palette = button->palette();
                palette.setColor(QPalette::Button, color);
                button->setPalette(palette);
                int index = colorButtons.indexOf(button);
                button->setToolTip(QString("%1-#%2%3%4").arg(index).arg(color.red(), 2, 16, QChar('0')).arg(color.green(), 2, 16, QChar('0')).arg(color.blue(), 2, 16, QChar('0')));
                emit colorChanged(index,color);
            }
        });
    }
    connect(ui->checkBoxColorSchemesBak,&QCheckBox::stateChanged,this,[&]{
        ui->comBoxColorSchemesBak->setDisabled(!ui->checkBoxColorSchemesBak->isChecked());
    });
}

GlobalOptionsAppearanceWidget::~GlobalOptionsAppearanceWidget()
{
    delete ui;
}
