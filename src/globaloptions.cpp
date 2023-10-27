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
#include <QFileInfo>
#include "filedialog.h"
#include "globaloptions.h"
#include "globalsetting.h"
#include "ui_globaloptions.h"

const QString GlobalOptions::defaultColorScheme = "QuardCRT";

GlobalOptions::GlobalOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalOptions)
{
    ui->setupUi(this);

    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    
    font = QApplication::font();
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    if(settings.contains("fontPointSize")) {
        font.setPointSize(settings.value("fontPointSize").toInt());
    } else {
        font.setPointSize(12);
        settings.setValue("fontPointSize", font.pointSize());
    }
    ui->spinBoxFontSize->setValue(font.pointSize());

    ui->comBoxColorSchemes->setEditable(true);
    ui->comBoxColorSchemes->setInsertPolicy(QComboBox::NoInsert);

    if(settings.contains("transparency"))
        ui->horizontalSliderTransparent->setValue(settings.value("transparency").toInt());
    if(settings.contains("backgroundImage"))
        ui->lineEditBackgroundImage->setText(settings.value("backgroundImage").toString());
    if(settings.contains("backgroundImageMode"))
        ui->comboBoxBackgroundMode->setCurrentIndex(settings.value("backgroundImageMode").toInt());
    if(settings.contains("backgroundImageOpacity"))
        ui->horizontalSliderBackgroundImageOpacity->setValue(settings.value("backgroundImageOpacity").toInt());
    settings.endGroup();

    ui->comboBoxNewTabWorkPath->addItem(QDir::homePath());
    int size = settings.beginReadArray("Global/Bookmark");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ui->comboBoxNewTabWorkPath->addItem(settings.value("path").toString());
    }
    settings.endArray();
    ui->comboBoxNewTabWorkPath->setCurrentText(settings.value("Global/Options/NewTabWorkPath",QDir::homePath()).toString());
  
    ui->spinBoxScrollbackLines->setValue(settings.value("scrollbackLines", 1000).toInt());

    connect(ui->spinBoxFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)));
    connect(ui->toolButtonBackgroundImage, &QToolButton::clicked, this, [&](){
        QString imgPath = FileDialog::getOpenFileName(this, tr("Select Background Image"), ui->lineEditBackgroundImage->text(), tr("Image Files (*.png *.jpg *.bmp)"));
        if (!imgPath.isEmpty()) {
            ui->lineEditBackgroundImage->setText(imgPath);
        }
    });
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
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    if((settings.contains("colorScheme")) &&(colorSchemes.contains(settings.value("colorScheme").toString()))) {
        ui->comBoxColorSchemes->setCurrentText(settings.value("colorScheme").toString());
    } else {
        ui->comBoxColorSchemes->setCurrentText(defaultColorScheme);
        settings.setValue("colorScheme", defaultColorScheme);
    }
    settings.endGroup();
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

QString GlobalOptions::getBackgroundImage(void)
{
    QString imgPath = ui->lineEditBackgroundImage->text();
    QFileInfo imgInfo(imgPath);
    if (imgInfo.exists() && imgInfo.isFile()) {
        return imgPath;
    }
    return QString(); 
}

int GlobalOptions::getBackgroundImageMode(void)
{
    return ui->comboBoxBackgroundMode->currentIndex();
}

qreal GlobalOptions::getBackgroundImageOpacity(void)
{
    return ui->horizontalSliderBackgroundImageOpacity->value() / 100.0;
}

QString GlobalOptions::getNewTabWorkPath(void)
{
    return ui->comboBoxNewTabWorkPath->currentText();
}

uint32_t GlobalOptions::getScrollbackLines(void)
{
    return ui->spinBoxScrollbackLines->value();
}

void GlobalOptions::buttonBoxAccepted(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    settings.setValue("colorScheme", ui->comBoxColorSchemes->currentText());
    settings.setValue("fontPointSize", font.pointSize());
    settings.setValue("transparency", ui->horizontalSliderTransparent->value());
    settings.setValue("backgroundImage", ui->lineEditBackgroundImage->text());
    settings.setValue("backgroundImageMode", ui->comboBoxBackgroundMode->currentIndex());
    settings.setValue("backgroundImageOpacity", ui->horizontalSliderBackgroundImageOpacity->value());
    settings.setValue("NewTabWorkPath", ui->comboBoxNewTabWorkPath->currentText());
    settings.setValue("scrollbackLines", ui->spinBoxScrollbackLines->value());
    settings.endGroup();
    emit colorSchemeChanged(ui->comBoxColorSchemes->currentText());
    emit this->accepted();
}

void GlobalOptions::buttonBoxRejected(void)
{
    emit this->rejected();
}

void GlobalOptions::showEvent(QShowEvent *event)
{
    int index = ui->comboBoxBackgroundMode->currentIndex();
    ui->retranslateUi(this);
    ui->comboBoxBackgroundMode->setCurrentIndex(index);
    QDialog::showEvent(event);
}
