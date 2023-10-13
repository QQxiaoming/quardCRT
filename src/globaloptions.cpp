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
#include <QFont>
#include <QFontDatabase>
#include "globaloptions.h"
#include "ui_globaloptions.h"

const QString GlobalOptions::defaultColorScheme = "QuardCRT";

GlobalOptions::GlobalOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalOptions)
{
    ui->setupUi(this);
    
    font = QApplication::font();
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    font.setPointSize(12);
    ui->spinBoxFontSize->setValue(font.pointSize());

    ui->comBoxColorSchemes->setEditable(true);
    ui->comBoxColorSchemes->setInsertPolicy(QComboBox::NoInsert);
  
    connect(ui->spinBoxFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)));
    connect(ui->horizontalSliderTransparent, SIGNAL(valueChanged(int)), this, SIGNAL(transparencyChanged(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

GlobalOptions::~GlobalOptions()
{
    delete ui;
}

void GlobalOptions::setAvailableColorSchemes(QStringList colorSchemes)
{
    colorSchemes.sort();
    ui->comBoxColorSchemes->clear();
    ui->comBoxColorSchemes->addItems(colorSchemes);
    ui->comBoxColorSchemes->setCurrentText(defaultColorScheme);
}

QString GlobalOptions::getCurrentColorScheme(void)
{
    return ui->comBoxColorSchemes->currentText();
}

QFont GlobalOptions::getCurrentFont(void)
{
    return font;
}

void GlobalOptions::fontSizeChanged(int size)
{
    font.setPointSize(size);
}

int GlobalOptions::getTransparency(void)
{
    return ui->horizontalSliderTransparent->value();
}

void GlobalOptions::buttonBoxAccepted(void)
{
    emit colorSchemeChanged(ui->comBoxColorSchemes->currentText());
    emit this->accepted();
}

void GlobalOptions::buttonBoxRejected(void)
{
    emit this->rejected();
}

void GlobalOptions::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}
