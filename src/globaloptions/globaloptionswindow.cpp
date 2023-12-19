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
#include <QFont>
#include <QFontDatabase>
#include <QFileInfo>
#include <QComboBox>
#include <QMessageBox>
#include <QFontDialog>
#include <QDesktopServices>
#include "qtermwidget.h"

#include "filedialog.h"
#include "globalsetting.h"
#include "globaloptionswindow.h"

#include "ui_globaloptionswindow.h"
#include "ui_globaloptionsgeneralwidget.h"
#include "ui_globaloptionsappearancewidget.h"
#include "ui_globaloptionsterminalwidget.h"
#include "ui_globaloptionswindowwidget.h"
#include "ui_globaloptionsadvancedwidget.h"

const QString GlobalOptionsWindow::defaultColorScheme = "QuardCRT";

GlobalOptionsWindow::GlobalOptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalOptionsWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    ui->horizontalLayout->addWidget(splitter);
    QTreeView *treeView = new QTreeView(this);
    treeView->setHeaderHidden(true);
    model = new QStringListModel(treeView);
    treeView->setModel(model);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    splitter->addWidget(treeView);
    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    widget->setLayout(new QVBoxLayout(widget));
    widget->layout()->setContentsMargins(0,0,0,0);
    splitter->setSizes(QList<int>() << 1 << 100);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

    globalOptionsGeneralWidget = new GlobalOptionsGeneralWidget(widget);
    widget->layout()->addWidget(globalOptionsGeneralWidget);
    globalOptionsAppearanceWidget = new GlobalOptionsAppearanceWidget(widget);
    widget->layout()->addWidget(globalOptionsAppearanceWidget);
    globalOptionsTerminalWidget = new GlobalOptionsTerminalWidget(widget);
    widget->layout()->addWidget(globalOptionsTerminalWidget);
    globalOptionsWindowWidget = new GlobalOptionsWindowWidget(widget);
    widget->layout()->addWidget(globalOptionsWindowWidget);
    globalOptionsAdvancedWidget = new GlobalOptionsAdvancedWidget(widget);
    widget->layout()->addWidget(globalOptionsAdvancedWidget);

    setActiveWidget(0);

    retranslateUi();

    GlobalSetting settings;
    globalOptionsAdvancedWidget->ui->lineEditConfigFile->setText(settings.fileName());
    settings.beginGroup("Global/Options");
    font = QApplication::font();
#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
    int fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/inziu-iosevkaCC-SC-regular.ttf");
#else
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
#endif
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
        globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText("Built-in");
    }
    if(settings.contains("fontFamily")) {
        if(settings.value("fontFamily").toString() != "Built-in") {
            font = QFont(settings.value("fontFamily").toString());
            globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText(font.family());
        }
    }
    font.setFixedPitch(true);
    if(settings.contains("fontPointSize")) {
        font.setPointSize(settings.value("fontPointSize").toInt());
    } else {
        font.setPointSize(12);
        settings.setValue("fontPointSize", font.pointSize());
    }
    globalOptionsAppearanceWidget->ui->spinBoxFontSize->setValue(font.pointSize());

    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setEditable(true);
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setInsertPolicy(QComboBox::NoInsert);

    if(settings.contains("transparency"))
        globalOptionsWindowWidget->ui->horizontalSliderTransparent->setValue(settings.value("transparency").toInt());
    if(settings.contains("backgroundImage"))
        globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->setText(settings.value("backgroundImage").toString());
    if(settings.contains("backgroundImageMode"))
        globalOptionsAppearanceWidget->ui->comboBoxBackgroundMode->setCurrentIndex(settings.value("backgroundImageMode").toInt());
    if(settings.contains("backgroundImageOpacity"))
        globalOptionsAppearanceWidget->ui->horizontalSliderBackgroundImageOpacity->setValue(settings.value("backgroundImageOpacity").toInt());
    globalOptionsTerminalWidget->ui->spinBoxScrollbackLines->setValue(settings.value("scrollbackLines", 1000).toInt());
    globalOptionsAdvancedWidget->ui->checkBoxTerminalBackgroundAnimation->setChecked(settings.value("enableTerminalBackgroundAnimation", true).toBool());
    globalOptionsTerminalWidget->ui->comboBoxCursorShape->setCurrentIndex(settings.value("cursorShape", 0).toInt());
    globalOptionsTerminalWidget->ui->checkBoxCursorBlink->setChecked(settings.value("cursorBlink", true).toBool());
    globalOptionsAdvancedWidget->ui->checkBoxNativeUI->setChecked(settings.value("enableNativeUI", false).toBool());
    globalOptionsGeneralWidget->ui->comboBoxTabTitleMode->setCurrentIndex(settings.value("tabTitleMode", 0).toInt());
    globalOptionsGeneralWidget->ui->spinBoxTabTitleWidth->setValue(settings.value("tabTitleWidth", 20).toInt());
    globalOptionsGeneralWidget->ui->checkBoxTabPreview->setChecked(settings.value("enableTabPreview", true).toBool());
    globalOptionsGeneralWidget->ui->spinBoxTabPreviewWidth->setValue(settings.value("tabPreviewWidth", 300).toInt());
    globalOptionsTerminalWidget->ui->lineEditWordCharacters->setText(settings.value("wordCharacters", ":@-./_~").toString());
    settings.endGroup();

    if(settings.contains("Global/Options/translateService")) {
        globalOptionsAdvancedWidget->ui->comboBoxTranslateService->setCurrentIndex(settings.value("Global/Options/translateService").toInt());
    } else {
        QString locale = settings.value("Global/Startup/language", "en_US").toString();
        if(locale == "zh_CN")
            globalOptionsAdvancedWidget->ui->comboBoxTranslateService->setCurrentIndex(1);
        else
            globalOptionsAdvancedWidget->ui->comboBoxTranslateService->setCurrentIndex(0);
    }

    globalOptionsGeneralWidget->ui->comboBoxNewTabWorkPath->addItem(QDir::homePath());
    int size = settings.beginReadArray("Global/Bookmark");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        globalOptionsGeneralWidget->ui->comboBoxNewTabWorkPath->addItem(settings.value("path").toString());
    }
    settings.endArray();
    globalOptionsGeneralWidget->ui->comboBoxNewTabWorkPath->setCurrentText(settings.value("Global/Options/NewTabWorkPath",QDir::homePath()).toString());

    setAvailableColorSchemes(QTermWidget::availableColorSchemes());
    buttonBoxAccepted();

    connect(globalOptionsAppearanceWidget->ui->spinBoxFontSize, &QSpinBox::valueChanged, this, &GlobalOptionsWindow::fontSizeChanged);
    connect(globalOptionsAppearanceWidget->ui->toolButtonBackgroundImage, &QToolButton::clicked, this, [&](){
        QString imgPath = FileDialog::getOpenFileName(this, tr("Select Background Image"), globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->text(), tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;Video Files (*.mp4 *.avi *.mkv *.mov)"));
        if (!imgPath.isEmpty()) {
            globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->setText(imgPath);
        }
    });
    connect(globalOptionsAppearanceWidget->ui->toolButtonClearBackgroundImage, &QToolButton::clicked, this, [&](){
        globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->setText("");
    });
    connect(globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont, &QPushButton::clicked, this, [=](){
        bool ok;
        QFont sfont = QFontDialog::getFont(&ok, this->font, this);
        if (ok) {
            sfont.setPointSize((sfont.pointSize()/3)*3);
            this->font = sfont;
            font.setFixedPitch(true);
            globalOptionsAppearanceWidget->ui->spinBoxFontSize->setValue(font.pointSize());
            globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText(font.family());
        } else {
            font = QApplication::font();
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            if (fontFamilies.size() > 0) {
                font.setFamily(fontFamilies[0]);
            }
            GlobalSetting settings;
            if(settings.contains("fontPointSize")) {
                font.setPointSize(settings.value("fontPointSize").toInt());
            } else {
                font.setPointSize(12);
                settings.setValue("fontPointSize", font.pointSize());
            }
            font.setFixedPitch(true);
            globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText("Built-in");
            globalOptionsAppearanceWidget->ui->spinBoxFontSize->setValue(font.pointSize());
        }
    });
    connect(globalOptionsWindowWidget->ui->horizontalSliderTransparent, &QSlider::valueChanged, this, &GlobalOptionsWindow::transparencyChanged);
    connect(globalOptionsAdvancedWidget->ui->checkBoxTerminalBackgroundAnimation, &QCheckBox::toggled, this, [&](){
        if (globalOptionsAdvancedWidget->ui->checkBoxTerminalBackgroundAnimation->isChecked()) {
            QMessageBox::information(this, tr("Information"), tr("This feature needs more system resources, please use it carefully!"));
        }
    });
    connect(globalOptionsAdvancedWidget->ui->checkBoxGithubCopilot, &QCheckBox::toggled, this, [&](){
        if (globalOptionsAdvancedWidget->ui->checkBoxGithubCopilot->isChecked()) {
            QMessageBox::information(this, tr("Information"), tr("This feature is not implemented yet!"));
            globalOptionsAdvancedWidget->ui->checkBoxGithubCopilot->setChecked(false);
        }
    });
    connect(globalOptionsAdvancedWidget->ui->toolButtonOpenConfigFile,&QToolButton::clicked, this, [&](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(globalOptionsAdvancedWidget->ui->lineEditConfigFile->text()));
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GlobalOptionsWindow::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GlobalOptionsWindow::buttonBoxRejected);

    connect(treeView, &QTreeView::clicked, [=](const QModelIndex &index) {
        setActiveWidget(index.row());
    });
}

GlobalOptionsWindow::~GlobalOptionsWindow()
{
    delete ui;
}

void GlobalOptionsWindow::retranslateUi()
{
    model->setStringList(QStringList() << tr("General") << tr("Appearance") << tr("Terminal") << tr("Window") << tr("Advanced"));
    ui->retranslateUi(this);
    globalOptionsGeneralWidget->ui->retranslateUi(this);
    globalOptionsGeneralWidget->ui->retranslateUi(this);
    globalOptionsAppearanceWidget->ui->retranslateUi(this);
    globalOptionsTerminalWidget->ui->retranslateUi(this);
    globalOptionsWindowWidget->ui->retranslateUi(this);
    globalOptionsAdvancedWidget->ui->retranslateUi(this);
}

void GlobalOptionsWindow::setActiveWidget(int index)
{
    globalOptionsGeneralWidget->setVisible(false);
    globalOptionsAppearanceWidget->setVisible(false);
    globalOptionsTerminalWidget->setVisible(false);
    globalOptionsWindowWidget->setVisible(false);
    globalOptionsAdvancedWidget->setVisible(false);

    switch(index) {
    case 0:
        globalOptionsGeneralWidget->setVisible(true);
        break;
    case 1:
        globalOptionsAppearanceWidget->setVisible(true);
        break;
    case 2:
        globalOptionsTerminalWidget->setVisible(true);
        break;
    case 3:
        globalOptionsWindowWidget->setVisible(true);
        break;
    case 4:
        globalOptionsAdvancedWidget->setVisible(true);
        break;
    }
}

void GlobalOptionsWindow::setAvailableColorSchemes(QStringList colorSchemes)
{
    colorSchemes.sort();
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->clear();
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->addItems(colorSchemes);
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    if((settings.contains("colorScheme")) &&(colorSchemes.contains(settings.value("colorScheme").toString()))) {
        globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(settings.value("colorScheme").toString());
    } else {
        globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(defaultColorScheme);
        settings.setValue("colorScheme", defaultColorScheme);
    }
    settings.endGroup();
}

QString GlobalOptionsWindow::getCurrentColorScheme(void)
{
    return globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText();
}

QFont GlobalOptionsWindow::getCurrentFont(void)
{
    return font;
}

void GlobalOptionsWindow::fontSizeChanged(int size)
{
    font.setPointSize(size);
}

int GlobalOptionsWindow::getTransparency(void)
{
    return globalOptionsWindowWidget->ui->horizontalSliderTransparent->value();
}

QString GlobalOptionsWindow::getBackgroundImage(void)
{
    QString imgPath = globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->text();
    QFileInfo imgInfo(imgPath);
    if (imgInfo.exists() && imgInfo.isFile()) {
        return imgPath;
    }
    return QString();
}

int GlobalOptionsWindow::getBackgroundImageMode(void)
{
    return globalOptionsAppearanceWidget->ui->comboBoxBackgroundMode->currentIndex();
}

qreal GlobalOptionsWindow::getBackgroundImageOpacity(void)
{
    return globalOptionsAppearanceWidget->ui->horizontalSliderBackgroundImageOpacity->value() / 100.0;
}

QString GlobalOptionsWindow::getNewTabWorkPath(void)
{
    return globalOptionsGeneralWidget->ui->comboBoxNewTabWorkPath->currentText();
}

uint32_t GlobalOptionsWindow::getScrollbackLines(void)
{
    return globalOptionsTerminalWidget->ui->spinBoxScrollbackLines->value();
}

bool GlobalOptionsWindow::getEnableTerminalBackgroundAnimation(void)
{
    return globalOptionsAdvancedWidget->ui->checkBoxTerminalBackgroundAnimation->isChecked();
}

uint32_t GlobalOptionsWindow::getCursorShape(void)
{
    return globalOptionsTerminalWidget->ui->comboBoxCursorShape->currentIndex();
}

bool GlobalOptionsWindow::getCursorBlink(void)
{
    return globalOptionsTerminalWidget->ui->checkBoxCursorBlink->isChecked();
}

int GlobalOptionsWindow::getTabTitleMode(void) 
{
    return globalOptionsGeneralWidget->ui->comboBoxTabTitleMode->currentIndex();
}

int GlobalOptionsWindow::getTabTitleWidth(void) 
{
    return globalOptionsGeneralWidget->ui->spinBoxTabTitleWidth->value();
}

bool GlobalOptionsWindow::getEnableTabPreview(void) 
{
    return globalOptionsGeneralWidget->ui->checkBoxTabPreview->isChecked();
}

int GlobalOptionsWindow::getTabPreviewWidth(void) 
{
    return globalOptionsGeneralWidget->ui->spinBoxTabPreviewWidth->value();
}

QString GlobalOptionsWindow::getWordCharacters(void) 
{
    return globalOptionsTerminalWidget->ui->lineEditWordCharacters->text();
}

int GlobalOptionsWindow::getTranslateService(void) 
{
    return globalOptionsAdvancedWidget->ui->comboBoxTranslateService->currentIndex();
}

void GlobalOptionsWindow::buttonBoxAccepted(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    settings.setValue("colorScheme", globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText());
    settings.setValue("fontFamily", globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->text());
    settings.setValue("fontPointSize", font.pointSize());
    settings.setValue("transparency", globalOptionsWindowWidget->ui->horizontalSliderTransparent->value());
    settings.setValue("backgroundImage", globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->text());
    settings.setValue("backgroundImageMode", globalOptionsAppearanceWidget->ui->comboBoxBackgroundMode->currentIndex());
    settings.setValue("backgroundImageOpacity", globalOptionsAppearanceWidget->ui->horizontalSliderBackgroundImageOpacity->value());
    settings.setValue("NewTabWorkPath", globalOptionsGeneralWidget->ui->comboBoxNewTabWorkPath->currentText());
    settings.setValue("scrollbackLines", globalOptionsTerminalWidget->ui->spinBoxScrollbackLines->value());
    settings.setValue("enableTerminalBackgroundAnimation", globalOptionsAdvancedWidget->ui->checkBoxTerminalBackgroundAnimation->isChecked());
    settings.setValue("cursorShape", globalOptionsTerminalWidget->ui->comboBoxCursorShape->currentIndex());
    settings.setValue("cursorBlink", globalOptionsTerminalWidget->ui->checkBoxCursorBlink->isChecked());
    settings.setValue("enableNativeUI", globalOptionsAdvancedWidget->ui->checkBoxNativeUI->isChecked());
    settings.setValue("tabTitleMode", globalOptionsGeneralWidget->ui->comboBoxTabTitleMode->currentIndex());
    settings.setValue("tabTitleWidth", globalOptionsGeneralWidget->ui->spinBoxTabTitleWidth->value());
    settings.setValue("enableTabPreview", globalOptionsGeneralWidget->ui->checkBoxTabPreview->isChecked());
    settings.setValue("tabPreviewWidth", globalOptionsGeneralWidget->ui->spinBoxTabPreviewWidth->value());
    settings.setValue("wordCharacters", globalOptionsTerminalWidget->ui->lineEditWordCharacters->text());
    settings.setValue("translateService", globalOptionsAdvancedWidget->ui->comboBoxTranslateService->currentIndex());
    settings.endGroup();
    emit colorSchemeChanged(globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText());
    emit this->accepted();
}

void GlobalOptionsWindow::buttonBoxRejected(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(settings.value("colorScheme").toString());
    globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText(settings.value("fontFamily").toString());
    font.setFamily(settings.value("fontFamily").toString());
    font.setPointSize(settings.value("fontPointSize").toInt());
    globalOptionsAppearanceWidget->ui->spinBoxFontSize->setValue(font.pointSize());
    globalOptionsWindowWidget->ui->horizontalSliderTransparent->setValue(settings.value("transparency").toInt());
    globalOptionsAppearanceWidget->ui->lineEditBackgroundImage->setText(settings.value("backgroundImage").toString());
    globalOptionsAppearanceWidget->ui->comboBoxBackgroundMode->setCurrentIndex(settings.value("backgroundImageMode").toInt());
    globalOptionsAppearanceWidget->ui->horizontalSliderBackgroundImageOpacity->setValue(settings.value("backgroundImageOpacity").toInt());
    globalOptionsGeneralWidget->ui->comboBoxNewTabWorkPath->setCurrentText(settings.value("NewTabWorkPath",QDir::homePath()).toString());
    globalOptionsTerminalWidget->ui->spinBoxScrollbackLines->setValue(settings.value("scrollbackLines", 1000).toInt());
    globalOptionsAdvancedWidget->ui->checkBoxTerminalBackgroundAnimation->setChecked(settings.value("enableTerminalBackgroundAnimation", true).toBool());
    globalOptionsTerminalWidget->ui->comboBoxCursorShape->setCurrentIndex(settings.value("cursorShape", 0).toInt());
    globalOptionsTerminalWidget->ui->checkBoxCursorBlink->setChecked(settings.value("cursorBlink", true).toBool());
    globalOptionsAdvancedWidget->ui->checkBoxNativeUI->setChecked(settings.value("enableNativeUI", false).toBool());
    globalOptionsGeneralWidget->ui->comboBoxTabTitleMode->setCurrentIndex(settings.value("tabTitleMode", 0).toInt());
    globalOptionsGeneralWidget->ui->spinBoxTabTitleWidth->setValue(settings.value("tabTitleWidth", 20).toInt());
    globalOptionsGeneralWidget->ui->checkBoxTabPreview->setChecked(settings.value("enableTabPreview", true).toBool());
    globalOptionsGeneralWidget->ui->spinBoxTabPreviewWidth->setValue(settings.value("tabPreviewWidth", 300).toInt());
    globalOptionsTerminalWidget->ui->lineEditWordCharacters->setText(settings.value("wordCharacters", ":@-./_~").toString());
    globalOptionsAdvancedWidget->ui->comboBoxTranslateService->setCurrentIndex(settings.value("translateService", 0).toInt());
    settings.endGroup();
    emit this->rejected();
}

void GlobalOptionsWindow::showEvent(QShowEvent *event)
{
    retranslateUi();
    GlobalSetting settings;
    globalOptionsAppearanceWidget->ui->comboBoxBackgroundMode->setCurrentIndex(settings.value("Global/Options/backgroundImageMode", 1).toInt());
    QDialog::showEvent(event);
}
