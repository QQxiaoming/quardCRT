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
#include <QToolTip>
#include <QColorDialog>
#include "qtermwidget.h"
#include "ColorScheme.h"
#include "CharacterColor.h"

#include "filedialog.h"
#include "globalsetting.h"
#include "globaloptionswindow.h"

#include "ui_globaloptionswindow.h"
#include "ui_globaloptionsgeneralwidget.h"
#include "ui_globaloptionsappearancewidget.h"
#include "ui_globaloptionsterminalwidget.h"
#include "ui_globaloptionswindowwidget.h"
#include "ui_globaloptionstransferwidget.h"
#include "ui_globaloptionsadvancedwidget.h"

const QString GlobalOptionsWindow::defaultColorScheme = "QuardCRT";
const QString GlobalOptionsWindow::defaultColorSchemeBak = "QuardCRT Light";

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
    globalOptionsTransferWidget = new GlobalOptionsTransferWidget(widget);
    widget->layout()->addWidget(globalOptionsTransferWidget);
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
    }
    globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText("Built-in");
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
    globalOptionsTerminalWidget->ui->horizontalSliderSelectedTextAccentColorTransparency->setValue(settings.value("selectedTextAccentColorTransparency", 50).toInt());
    globalOptionsGeneralWidget->ui->comboBoxNewTabMode->setCurrentIndex(settings.value("newTabMode", 1).toInt());
    globalOptionsTransferWidget->ui->lineEditDownload->setText(settings.value("modemDownloadPath", QDir::homePath()).toString());
    globalOptionsTransferWidget->ui->lineEditUpload->setText(settings.value("modemUploadPath", QDir::homePath()).toString());
    globalOptionsTransferWidget->ui->checkBoxZmodemOnline->setChecked(settings.value("disableZmodemOnline", false).toBool());
    globalOptionsAppearanceWidget->ui->spinBoxPreeditColorIndex->setValue(settings.value("preeditColorIndex", 16).toInt());
    globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->setChecked(settings.value("enableColorSchemeBak", true).toBool());
    globalOptionsGeneralWidget->ui->lineEditWSLUserName->setText(settings.value("WSLUserName", "root").toString());
    globalOptionsGeneralWidget->ui->lineEditWSLDistroName->setText(settings.value("WSLDistroName", "Ubuntu").toString());
    globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->setText(settings.value("UserPluginsPath", "").toString());
    globalOptionsTerminalWidget->ui->checkBoxConfirmMultilinePaste->setChecked(settings.value("ConfirmMultilinePaste", true).toBool());
    globalOptionsTerminalWidget->ui->checkBoxTrimPastedTrailingNewlines->setChecked(settings.value("TrimPastedTrailingNewlines", true).toBool());
    globalOptionsTerminalWidget->ui->checkBoxEcho->setChecked(settings.value("Echo", false).toBool());
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    globalOptionsAdvancedWidget->ui->checkBoxEnableCtrlC->setChecked(settings.value("EnableCtrlC", false).toBool());
#endif
#if defined(Q_OS_WIN)
    globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->setText(settings.value("PowerShellProfile", QApplication::applicationDirPath() + "/Profile.ps1").toString());
#endif
    QString defaultLocalShell = settings.value("DefaultLocalShell",
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            "ENV:SHELL"
        #else
            "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe"
        #endif
    ).toString();
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    if(defaultLocalShell != "ENV:SHELL") 
    #endif
    {
        QFileInfo shellInfo(defaultLocalShell);
        if(!shellInfo.exists() || !shellInfo.isFile() || !shellInfo.isExecutable()) {
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            defaultLocalShell = "ENV:SHELL";
    #else
            defaultLocalShell = "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
    #endif
        }
    }
    globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->setText(defaultLocalShell);
    cursorColorStr = settings.value("CursorColor", "None").toString();
    QRegularExpression hexColorPattern(QLatin1String("^#[0-9a-f]{6}$"),
                                        QRegularExpression::CaseInsensitiveOption);
    if (hexColorPattern.match(cursorColorStr).hasMatch()) {
        uint8_t r = QStringView{cursorColorStr}.mid(1,2).toUInt(nullptr,16);
        uint8_t g = QStringView{cursorColorStr}.mid(3,2).toUInt(nullptr,16);
        uint8_t b = QStringView{cursorColorStr}.mid(5,2).toUInt(nullptr,16);
        QPushButton *button = globalOptionsTerminalWidget->ui->pushButtonCursorColor;
        QPalette palette = button->palette();
        palette.setColor(QPalette::Button, QColor(r,g,b));
        button->setPalette(palette);
        button->setToolTip(cursorColorStr);
    } else {
        cursorColorStr = "None";
        QPushButton testButton;
        QPalette palette = testButton.palette();
        globalOptionsTerminalWidget->ui->pushButtonCursorColor->setPalette(palette);
        globalOptionsTerminalWidget->ui->pushButtonCursorColor->setToolTip(cursorColorStr);
    }
    if(settings.value("xyModem1K", false).toBool()) {
        globalOptionsTransferWidget->ui->radioButton1KBytes->setChecked(true);
    } else {
        globalOptionsTransferWidget->ui->radioButton128Bytes->setChecked(true);
    }
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

    connect(globalOptionsAppearanceWidget, &GlobalOptionsAppearanceWidget::colorChanged, this, [&](int index, const QColor &color){
        if(index < TABLE_COLORS) {
            globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText("Custom");
            table[index].color = color;
        }
    });
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
    connect(globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont, &QPushButton::clicked, this, [&](){
        bool ok;
        QFont sfont = QFontDialog::getFont(&ok, this->font, this);
        if (ok) {
            sfont.setPointSize((sfont.pointSize()/3)*3);
            font = sfont;
            font.setFixedPitch(true);
            globalOptionsAppearanceWidget->ui->spinBoxFontSize->setValue(font.pointSize());
            globalOptionsAppearanceWidget->ui->pushButtonSelectSeriesFont->setText(font.family());
        } else {
            font = QApplication::font();
        #if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
            int fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/inziu-iosevkaCC-SC-regular.ttf");
        #else
            int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
        #endif
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
    connect(globalOptionsAdvancedWidget->ui->toolButtonUserPluginsPath, &QToolButton::clicked, this, [&](){
        QString path = FileDialog::getExistingDirectory(this, tr("Select User Plugins Path"), globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->text());
        if (!path.isEmpty()) {
            QFileInfo info(path);
            if(!info.isDir()) {
                QMessageBox::warning(this, tr("Warning"), tr("The path is not a directory!"));
                return;
            }
            globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->setText(path);
        }
    });
    connect(globalOptionsAdvancedWidget->ui->pushButtonUserPluginsPathClear, &QPushButton::clicked, this, [&](){
        globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->setText("");
    });
    connect(globalOptionsTerminalWidget->ui->pushButtonCursorColor, &QPushButton::clicked, this, [&](){
        QPushButton *button = globalOptionsTerminalWidget->ui->pushButtonCursorColor;
        QColor color = QColorDialog::getColor(button->palette().color(QPalette::Button), this, tr("Select color"));
        if (color.isValid()) {
            QPalette palette = button->palette();
            palette.setColor(QPalette::Button, color);
            button->setPalette(palette);
            cursorColorStr = QString("#%1%2%3").arg(color.red(), 2, 16, QChar('0')).arg(color.green(), 2, 16, QChar('0')).arg(color.blue(), 2, 16, QChar('0'));
            button->setToolTip(cursorColorStr);
        } else {
            QPushButton testButton;
            QPalette palette = testButton.palette();
            button->setPalette(palette);
            cursorColorStr = "None";
            button->setToolTip(cursorColorStr);
        }
    });
    connect(globalOptionsAppearanceWidget->ui->comBoxColorSchemes, &QComboBox::currentTextChanged, this, [&](const QString &text){
        if(text == "Custom") {
            globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->setEnabled(false);
            return;
        } 
        updateColorButtons(text);
    });
    connect(globalOptionsAdvancedWidget->ui->toolButtonDefaultLocalShell, &QToolButton::clicked, this, [&](){
        #if defined(Q_OS_WIN)
        QMessageBox::warning(this, tr("Warning"), tr("This feature is used to set the default PowerShell version.\nPlease use the PowerShell 5 or later!\nAnd do not support other shells!\nIf you need to use another shell, please create a session through [quick-connect > localshell > specific-command]."));
        #endif
        QString shell = FileDialog::getOpenFileName(this, tr("Select Default Local Shell"), globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->text(), tr("Executable Files (*)"));
        if (!shell.isEmpty()) {
            QFileInfo shellInfo(shell);
            if(!shellInfo.exists() || !shellInfo.isFile() || !shellInfo.isExecutable()) {
                QMessageBox::warning(this, tr("Warning"), tr("The Default Local Shell is not a valid file!"));
                return;
            }
            globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->setText(shell);
        } else {
            #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->setText("ENV:SHELL");
            #else
            globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->setText("c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe");
            #endif
        }
    });
#if defined(Q_OS_WIN)
    connect(globalOptionsAdvancedWidget->ui->toolButtonPowerShellProfile, &QToolButton::clicked, this, [&](){
        QString profile = FileDialog::getOpenFileName(this, tr("Select PowerShell Profile"), globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->text(), tr("PowerShell Files (*.ps1)"));
        if (!profile.isEmpty()) {
            QFileInfo profileInfo(profile);
            if(!profileInfo.exists() || !profileInfo.isFile()) {
                QMessageBox::warning(this, tr("Warning"), tr("The PowerShell Profile is not a valid file!"));
                return;
            }
            globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->setText(profile);
        } else {
            globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->setText(QApplication::applicationDirPath() + "/Profile.ps1");
        }
    });
    connect(globalOptionsAdvancedWidget->ui->pushButtonPowerShellProfile, &QPushButton::clicked, this, [&](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->text()));
    });
#endif
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GlobalOptionsWindow::buttonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GlobalOptionsWindow::buttonBoxRejected);

    connect(treeView, &QTreeView::clicked, [&](const QModelIndex &index) {
        setActiveWidget(index.row());
    });
}

GlobalOptionsWindow::~GlobalOptionsWindow()
{
    delete ui;
}

void GlobalOptionsWindow::retranslateUi()
{
    model->setStringList(QStringList() << tr("General") << tr("Appearance") << tr("Terminal") << tr("Window") << tr("Transfer") << tr("Advanced"));
    ui->retranslateUi(this);
    globalOptionsGeneralWidget->ui->retranslateUi(this);
    globalOptionsGeneralWidget->ui->retranslateUi(this);
    globalOptionsAppearanceWidget->ui->retranslateUi(this);
    globalOptionsTerminalWidget->ui->retranslateUi(this);
    globalOptionsWindowWidget->ui->retranslateUi(this);
    globalOptionsTransferWidget->ui->retranslateUi(this);
    globalOptionsAdvancedWidget->ui->retranslateUi(this);
}

void GlobalOptionsWindow::setActiveWidget(int index)
{
    globalOptionsGeneralWidget->setVisible(false);
    globalOptionsAppearanceWidget->setVisible(false);
    globalOptionsTerminalWidget->setVisible(false);
    globalOptionsWindowWidget->setVisible(false);
    globalOptionsTransferWidget->setVisible(false);
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
        globalOptionsTransferWidget->setVisible(true);
        break;
    case 5:
        globalOptionsAdvancedWidget->setVisible(true);
        break;
    }
}

void GlobalOptionsWindow::setAvailableColorSchemes(QStringList colorSchemes)
{
    colorSchemes.sort();
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->clear();
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->addItem("Custom");
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->addItems(colorSchemes);
    globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->clear();
    globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->addItems(colorSchemes);
    const ColorScheme *cs = ColorSchemeManager::instance()->findColorScheme(defaultColorScheme);
    if(cs) cs->getColorTable(table);

    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    if(settings.contains("colorScheme")) {
        QString colorScheme = settings.value("colorScheme").toString();
        if(colorScheme == "Custom") {
            globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText("Custom");
        } else if(colorSchemes.contains(colorScheme)) {
            globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(colorScheme);
        } else {
            globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(defaultColorScheme);
            settings.setValue("colorScheme", defaultColorScheme);
        }
    } else {
        globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(defaultColorScheme);
        settings.setValue("colorScheme", defaultColorScheme);
    }
    if(settings.contains("colorSchemeBak")) {
        QString colorScheme = settings.value("colorSchemeBak").toString();
        if(colorSchemes.contains(colorScheme)) {
            globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->setCurrentText(colorScheme);
        } else {
            globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->setCurrentText(defaultColorSchemeBak);
            settings.setValue("colorSchemeBak", defaultColorSchemeBak);
        }
    } else {
        globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->setCurrentText(defaultColorSchemeBak);
        settings.setValue("colorSchemeBak", defaultColorSchemeBak);
    }
    settings.endGroup();

    updateColorButtons(globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText());
}

QString GlobalOptionsWindow::getCurrentColorScheme(void)
{
    return globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText();
}

QList<QColor> GlobalOptionsWindow::getColorTable(void)
{
    QList<QColor> colors;
    for(int i = 0; i < TABLE_COLORS; i++) {
        colors.append(table[i].color);
    }
    return colors;
}

QFont GlobalOptionsWindow::getCurrentFont(void)
{
    return font;
}

bool GlobalOptionsWindow::isCurrentFontBuiltIn(void) {
    GlobalSetting settings;
    if(settings.contains("Global/Options/fontFamily"))
        return settings.value("Global/Options/fontFamily").toString() == "Built-in";

    return true;
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

int GlobalOptionsWindow::getSelectedTextAccentColorTransparency(void) 
{
    return globalOptionsTerminalWidget->ui->horizontalSliderSelectedTextAccentColorTransparency->value();
}

int GlobalOptionsWindow::getTranslateService(void) 
{
    return globalOptionsAdvancedWidget->ui->comboBoxTranslateService->currentIndex();
}

int GlobalOptionsWindow::getNewTabMode(void) 
{
    return globalOptionsGeneralWidget->ui->comboBoxNewTabMode->currentIndex();
}

QString GlobalOptionsWindow::getModemDownloadPath(void) 
{
    return globalOptionsTransferWidget->ui->lineEditDownload->text();
}

QString GlobalOptionsWindow::getModemUploadPath(void) 
{
    return globalOptionsTransferWidget->ui->lineEditUpload->text();
}

bool GlobalOptionsWindow::getDisableZmodemOnline(void) 
{
    return globalOptionsTransferWidget->ui->checkBoxZmodemOnline->isChecked();
}

bool GlobalOptionsWindow::getXYModem1K(void) 
{
    return globalOptionsTransferWidget->ui->radioButton1KBytes->isChecked();
}

int GlobalOptionsWindow::getPreeditColorIndex(void) 
{
    return globalOptionsAppearanceWidget->ui->spinBoxPreeditColorIndex->value();
}

bool GlobalOptionsWindow::getConfirmMultilinePaste(void) 
{
    return globalOptionsTerminalWidget->ui->checkBoxConfirmMultilinePaste->isChecked();
}

bool GlobalOptionsWindow::getTrimPastedTrailingNewlines(void) 
{
    return globalOptionsTerminalWidget->ui->checkBoxTrimPastedTrailingNewlines->isChecked();
}

bool GlobalOptionsWindow::getEcho(void) 
{
    return globalOptionsTerminalWidget->ui->checkBoxEcho->isChecked();
}

QColor GlobalOptionsWindow::getCursorColor(void)
{
    if(cursorColorStr == "None") {
        return QColor();
    } else {
        return globalOptionsTerminalWidget->ui->pushButtonCursorColor->palette().color(QPalette::Button);
    }
}

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
bool GlobalOptionsWindow::getEnableCtrlC(void) 
{
    return globalOptionsAdvancedWidget->ui->checkBoxEnableCtrlC->isChecked();
}
#endif

#if defined(Q_OS_WIN)
QString GlobalOptionsWindow::getPowerShellProfile(void) 
{
    GlobalSetting settings;
    QString profile = settings.value("Global/Options/PowerShellProfile", QApplication::applicationDirPath() + "/Profile.ps1").toString();
    if (!profile.isEmpty()) {
        QFileInfo profileInfo(profile);
        if(!profileInfo.exists() || !profileInfo.isFile()) {
            globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->setText(QApplication::applicationDirPath() + "/Profile.ps1");
        }
    } else {
        globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->setText(QApplication::applicationDirPath() + "/Profile.ps1");
    }
    settings.setValue("Global/Options/PowerShellProfile", globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->text());
    return globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->text();
}
#endif

void GlobalOptionsWindow::buttonBoxAccepted(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    if(globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText() == "Custom") {
        for(int i = 0; i < TABLE_COLORS; i++) {
            QColor color = table[i].color;
            int colorv = color.red()<<16 | color.green()<<8 | color.blue();
            settings.setValue(QString("CustomColor%1").arg(i), colorv);
        }
        globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText("Custom");
        globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->setEnabled(false);
        settings.setValue("colorScheme", "Custom");
    } else {
        settings.setValue("colorScheme", globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText());
    }
    settings.setValue("colorSchemeBak", globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->currentText());
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
    settings.setValue("selectedTextAccentColorTransparency", globalOptionsTerminalWidget->ui->horizontalSliderSelectedTextAccentColorTransparency->value());
    settings.setValue("translateService", globalOptionsAdvancedWidget->ui->comboBoxTranslateService->currentIndex());
    settings.setValue("newTabMode", globalOptionsGeneralWidget->ui->comboBoxNewTabMode->currentIndex());
    settings.setValue("modemDownloadPath", globalOptionsTransferWidget->ui->lineEditDownload->text());
    settings.setValue("modemUploadPath", globalOptionsTransferWidget->ui->lineEditUpload->text());
    settings.setValue("disableZmodemOnline", globalOptionsTransferWidget->ui->checkBoxZmodemOnline->isChecked());
    settings.setValue("xyModem1K", globalOptionsTransferWidget->ui->radioButton1KBytes->isChecked());
    settings.setValue("preeditColorIndex", globalOptionsAppearanceWidget->ui->spinBoxPreeditColorIndex->value());
    settings.setValue("enableColorSchemeBak", globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->isChecked());
    settings.setValue("WSLUserName", globalOptionsGeneralWidget->ui->lineEditWSLUserName->text());
    settings.setValue("WSLDistroName", globalOptionsGeneralWidget->ui->lineEditWSLDistroName->text());
    QFileInfo info(globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->text());
    if(info.isDir() || globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->text().isEmpty()) {
        settings.setValue("UserPluginsPath", globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->text());
    } else {
        globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->setText(settings.value("UserPluginsPath", "").toString());
        QMessageBox::warning(this, tr("Warning"), tr("The User Plugins Path is not a directory!"));
    }
    settings.setValue("ConfirmMultilinePaste", globalOptionsTerminalWidget->ui->checkBoxConfirmMultilinePaste->isChecked());
    settings.setValue("TrimPastedTrailingNewlines", globalOptionsTerminalWidget->ui->checkBoxTrimPastedTrailingNewlines->isChecked());
    settings.setValue("Echo", globalOptionsTerminalWidget->ui->checkBoxEcho->isChecked());
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    settings.setValue("EnableCtrlC", globalOptionsAdvancedWidget->ui->checkBoxEnableCtrlC->isChecked());
#endif
#if defined(Q_OS_WIN)
    settings.setValue("PowerShellProfile", globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->text());
#endif
    QString defaultLocalShell = globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->text();
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    if(defaultLocalShell != "ENV:SHELL") 
    #endif
    {
        QFileInfo shellInfo(defaultLocalShell);
        if(!shellInfo.exists() || !shellInfo.isFile() || !shellInfo.isExecutable()) {
            QMessageBox::warning(this, tr("Warning"), tr("The Default Local Shell is not a valid file!"));
            globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->setText(settings.value("DefaultLocalShell",
                #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
                    "ENV:SHELL"
                #else
                    "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe"
                #endif
            ).toString());
        } else {
            settings.setValue("DefaultLocalShell", defaultLocalShell);
        }
    } 
    #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    else {
        settings.setValue("DefaultLocalShell", defaultLocalShell);
    }
    #endif
    settings.setValue("CursorColor",cursorColorStr);
    settings.endGroup();
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    emit enableCtrlCChanged(globalOptionsAdvancedWidget->ui->checkBoxEnableCtrlC->isChecked());
#endif
    emit colorSchemeChanged(globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText());
    emit this->accepted();
}

void GlobalOptionsWindow::buttonBoxRejected(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    if(settings.value("colorScheme").toString() == "Custom") {
        for(int i = 0; i < TABLE_COLORS; i++) {
            int color = settings.value(QString("CustomColor%1").arg(i),0x0).toInt();
            table[i].color = QColor(color>>16, (color>>8)&0xFF, color&0xFF);
        }
        globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText("Custom");
        globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->setEnabled(false);
    } else {
        globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(settings.value("colorScheme").toString());
    }
    globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->setCurrentText(settings.value("colorSchemeBak").toString());
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
    globalOptionsTerminalWidget->ui->horizontalSliderSelectedTextAccentColorTransparency->setValue(settings.value("selectedTextAccentColorTransparency", 50).toInt());
    globalOptionsAdvancedWidget->ui->comboBoxTranslateService->setCurrentIndex(settings.value("translateService", 0).toInt());
    globalOptionsGeneralWidget->ui->comboBoxNewTabMode->setCurrentIndex(settings.value("newTabMode", 1).toInt());
    globalOptionsTransferWidget->ui->lineEditDownload->setText(settings.value("modemDownloadPath", QDir::homePath()).toString());
    globalOptionsTransferWidget->ui->lineEditUpload->setText(settings.value("modemUploadPath", QDir::homePath()).toString());
    globalOptionsTransferWidget->ui->checkBoxZmodemOnline->setChecked(settings.value("disableZmodemOnline", false).toBool());
    globalOptionsAppearanceWidget->ui->spinBoxPreeditColorIndex->setValue(settings.value("preeditColorIndex", 16).toInt());
    globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->setChecked(settings.value("enableColorSchemeBak", true).toBool());
    globalOptionsGeneralWidget->ui->lineEditWSLUserName->setText(settings.value("WSLUserName", "root").toString());
    globalOptionsGeneralWidget->ui->lineEditWSLDistroName->setText(settings.value("WSLDistroName", "Ubuntu").toString());
    globalOptionsAdvancedWidget->ui->lineEditUserPluginsPath->setText(settings.value("UserPluginsPath", "").toString());
    globalOptionsTerminalWidget->ui->checkBoxConfirmMultilinePaste->setChecked(settings.value("ConfirmMultilinePaste", true).toBool());
    globalOptionsTerminalWidget->ui->checkBoxTrimPastedTrailingNewlines->setChecked(settings.value("TrimPastedTrailingNewlines", true).toBool());
    globalOptionsTerminalWidget->ui->checkBoxEcho->setChecked(settings.value("Echo", false).toBool());
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    globalOptionsAdvancedWidget->ui->checkBoxEnableCtrlC->setChecked(settings.value("EnableCtrlC", false).toBool());
#endif
#if defined(Q_OS_WIN)
    globalOptionsAdvancedWidget->ui->lineEditPowerShellProfile->setText(settings.value("PowerShellProfile", QApplication::applicationDirPath() + "/Profile.ps1").toString());
#endif
    globalOptionsAdvancedWidget->ui->lineEditDefaultLocalShell->setText(settings.value("DefaultLocalShell",
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            "ENV:SHELL"
        #else
            "c:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe"
        #endif
    ).toString());
    cursorColorStr = settings.value("CursorColor", "None").toString();
    QRegularExpression hexColorPattern(QLatin1String("^#[0-9a-f]{6}$"),
                                        QRegularExpression::CaseInsensitiveOption);
    if (hexColorPattern.match(cursorColorStr).hasMatch()) {
        uint8_t r = QStringView{cursorColorStr}.mid(1,2).toUInt(nullptr,16);
        uint8_t g = QStringView{cursorColorStr}.mid(3,2).toUInt(nullptr,16);
        uint8_t b = QStringView{cursorColorStr}.mid(5,2).toUInt(nullptr,16);
        QPushButton *button = globalOptionsTerminalWidget->ui->pushButtonCursorColor;
        QPalette palette = button->palette();
        palette.setColor(QPalette::Button, QColor(r,g,b));
        button->setPalette(palette);
        button->setToolTip(cursorColorStr);
    } else {
        cursorColorStr = "None";
        QPushButton testButton;
        QPalette palette = testButton.palette();
        globalOptionsTerminalWidget->ui->pushButtonCursorColor->setPalette(palette);
        globalOptionsTerminalWidget->ui->pushButtonCursorColor->setToolTip(cursorColorStr);
    }
    if(settings.value("xyModem1K", false).toBool()) {
        globalOptionsTransferWidget->ui->radioButton1KBytes->setChecked(true);
    } else {
        globalOptionsTransferWidget->ui->radioButton128Bytes->setChecked(true);
    }
    settings.endGroup();
    emit this->rejected();
}

void GlobalOptionsWindow::showEvent(QShowEvent *event)
{
    retranslateUi();
    GlobalSetting settings;
    globalOptionsAppearanceWidget->ui->comboBoxBackgroundMode->setCurrentIndex(settings.value("Global/Options/backgroundImageMode", 1).toInt());
    updateColorButtons(globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText());
    QDialog::showEvent(event);
}

bool GlobalOptionsWindow::updateColorButtons(const QString &text) {
    if (text == "Custom") {
        GlobalSetting settings;
        settings.beginGroup("Global/Options");
        for(int i = 0; i < TABLE_COLORS; i++) {
            if(settings.contains(QString("CustomColor%1").arg(i))) {
                int colorv = settings.value(QString("CustomColor%1").arg(i)).toInt();
                table[i].color = QColor(colorv>>16, (colorv>>8)&0xFF, colorv&0xFF);
                QPushButton *button = globalOptionsAppearanceWidget->colorButtons.at(i);
                QColor color = table[i].color;
                QPalette palette = button->palette();
                palette.setColor(QPalette::Button, color);
                button->setPalette(palette);
                button->setToolTip(QString("#%1%2%3").arg(color.red(), 2, 16, QLatin1Char('0')).arg(color.green(), 2, 16, QLatin1Char('0')).arg(color.blue(), 2, 16, QLatin1Char('0')));
            } else {
                QColor color = table[i].color;
                int colorv = color.red()<<16 | color.green()<<8 | color.blue();
                settings.setValue(QString("CustomColor%1").arg(i), colorv);
            }
        }
        settings.endGroup();
        return true;
    }

    if (QTermWidget::availableColorSchemes().contains(text)) {
        const ColorScheme *cs = ColorSchemeManager::instance()->findColorScheme(text);
        if (cs) {
            cs->getColorTable(table);
            for(int i = 0; i < TABLE_COLORS; i++) {
                QPushButton *button = globalOptionsAppearanceWidget->colorButtons.at(i);
                QColor color = table[i].color;
                QPalette palette = button->palette();
                palette.setColor(QPalette::Button, color);
                button->setPalette(palette);
                button->setToolTip(QString("%1-#%2%3%4").arg(i).arg(color.red(), 2, 16, QLatin1Char('0')).arg(color.green(), 2, 16, QLatin1Char('0')).arg(color.blue(), 2, 16, QLatin1Char('0')));
            }
            return true;
        }
    }
    return false;
}

void GlobalOptionsWindow::switchTheme(void) {
    if(!globalOptionsAppearanceWidget->ui->checkBoxColorSchemesBak->isChecked()) {
        return;
    }
    if(globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText() == "Custom") {
        return;
    }
    QString colorScheme = globalOptionsAppearanceWidget->ui->comBoxColorSchemes->currentText();
    QString colorSchemeBak = globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->currentText();
    globalOptionsAppearanceWidget->ui->comBoxColorSchemesBak->setCurrentText(colorScheme);
    globalOptionsAppearanceWidget->ui->comBoxColorSchemes->setCurrentText(colorSchemeBak);
    buttonBoxAccepted();
}
