#include <QSplitter>
#include <QTreeView>
#include <QLabel>
#include <QStringListModel>
#include <QFont>
#include <QFontDatabase>
#include <QFileInfo>
#include <QComboBox>
#include "filedialog.h"
#include "globalsetting.h"

#include "globaloptionswindow.h"
#include "ui_globaloptionswindow.h"
#include "ui_globaloptionswidget.h"

const QString GlobalOptionsWindow::defaultColorScheme = "QuardCRT";

GlobalOptionsWindow::GlobalOptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalOptionsWindow)
{
    ui->setupUi(this);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    ui->horizontalLayout->addWidget(splitter);
    QTreeView *treeView = new QTreeView(this);
    treeView->setHeaderHidden(true);
    QStringListModel *model = new QStringListModel(treeView);
    treeView->setModel(model);
    model->setStringList(QStringList() << tr("General") << tr("Appearance") << tr("Terminal") << tr("Keyboard") << tr("Mouse") << tr("Window") << tr("Advanced"));
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    splitter->addWidget(treeView);
    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    widget->setLayout(new QVBoxLayout(widget));
    globalOptionsWidget = new GlobalOptionsWidget(widget);
    widget->layout()->addWidget(globalOptionsWidget);
    splitter->setSizes(QList<int>() << 1 << 100);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

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
    globalOptionsWidget->ui->spinBoxFontSize->setValue(font.pointSize());

    globalOptionsWidget->ui->comBoxColorSchemes->setEditable(true);
    globalOptionsWidget->ui->comBoxColorSchemes->setInsertPolicy(QComboBox::NoInsert);

    if(settings.contains("transparency"))
        globalOptionsWidget->ui->horizontalSliderTransparent->setValue(settings.value("transparency").toInt());
    if(settings.contains("backgroundImage"))
        globalOptionsWidget->ui->lineEditBackgroundImage->setText(settings.value("backgroundImage").toString());
    if(settings.contains("backgroundImageMode"))
        globalOptionsWidget->ui->comboBoxBackgroundMode->setCurrentIndex(settings.value("backgroundImageMode").toInt());
    if(settings.contains("backgroundImageOpacity"))
        globalOptionsWidget->ui->horizontalSliderBackgroundImageOpacity->setValue(settings.value("backgroundImageOpacity").toInt());
    settings.endGroup();

    globalOptionsWidget->ui->comboBoxNewTabWorkPath->addItem(QDir::homePath());
    int size = settings.beginReadArray("Global/Bookmark");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        globalOptionsWidget->ui->comboBoxNewTabWorkPath->addItem(settings.value("path").toString());
    }
    settings.endArray();
    globalOptionsWidget->ui->comboBoxNewTabWorkPath->setCurrentText(settings.value("Global/Options/NewTabWorkPath",QDir::homePath()).toString());
  
    globalOptionsWidget->ui->spinBoxScrollbackLines->setValue(settings.value("scrollbackLines", 1000).toInt());

    connect(globalOptionsWidget->ui->spinBoxFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)));
    connect(globalOptionsWidget->ui->toolButtonBackgroundImage, &QToolButton::clicked, this, [&](){
        QString imgPath = FileDialog::getOpenFileName(this, tr("Select Background Image"), globalOptionsWidget->ui->lineEditBackgroundImage->text(), tr("Image Files (*.png *.jpg *.bmp)"));
        if (!imgPath.isEmpty()) {
            globalOptionsWidget->ui->lineEditBackgroundImage->setText(imgPath);
        }
    });
    connect(globalOptionsWidget->ui->horizontalSliderTransparent, SIGNAL(valueChanged(int)), this, SIGNAL(transparencyChanged(int)));
    
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

GlobalOptionsWindow::~GlobalOptionsWindow()
{
    delete ui;
}

void GlobalOptionsWindow::setAvailableColorSchemes(QStringList colorSchemes)
{
    colorSchemes.sort();
    globalOptionsWidget->ui->comBoxColorSchemes->clear();
    globalOptionsWidget->ui->comBoxColorSchemes->addItems(colorSchemes);
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    if((settings.contains("colorScheme")) &&(colorSchemes.contains(settings.value("colorScheme").toString()))) {
        globalOptionsWidget->ui->comBoxColorSchemes->setCurrentText(settings.value("colorScheme").toString());
    } else {
        globalOptionsWidget->ui->comBoxColorSchemes->setCurrentText(defaultColorScheme);
        settings.setValue("colorScheme", defaultColorScheme);
    }
    settings.endGroup();
}

QString GlobalOptionsWindow::getCurrentColorScheme(void)
{
    return globalOptionsWidget->ui->comBoxColorSchemes->currentText();
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
    return globalOptionsWidget->ui->horizontalSliderTransparent->value();
}

QString GlobalOptionsWindow::getBackgroundImage(void)
{
    QString imgPath = globalOptionsWidget->ui->lineEditBackgroundImage->text();
    QFileInfo imgInfo(imgPath);
    if (imgInfo.exists() && imgInfo.isFile()) {
        return imgPath;
    }
    return QString();
}

int GlobalOptionsWindow::getBackgroundImageMode(void)
{
    return globalOptionsWidget->ui->comboBoxBackgroundMode->currentIndex();
}

qreal GlobalOptionsWindow::getBackgroundImageOpacity(void)
{
    return globalOptionsWidget->ui->horizontalSliderBackgroundImageOpacity->value() / 100.0;
}

QString GlobalOptionsWindow::getNewTabWorkPath(void)
{
    return globalOptionsWidget->ui->comboBoxNewTabWorkPath->currentText();
}

uint32_t GlobalOptionsWindow::getScrollbackLines(void)
{
    return globalOptionsWidget->ui->spinBoxScrollbackLines->value();
}

void GlobalOptionsWindow::buttonBoxAccepted(void)
{
    GlobalSetting settings;
    settings.beginGroup("Global/Options");
    settings.setValue("colorScheme", globalOptionsWidget->ui->comBoxColorSchemes->currentText());
    settings.setValue("fontPointSize", font.pointSize());
    settings.setValue("transparency", globalOptionsWidget->ui->horizontalSliderTransparent->value());
    settings.setValue("backgroundImage", globalOptionsWidget->ui->lineEditBackgroundImage->text());
    settings.setValue("backgroundImageMode", globalOptionsWidget->ui->comboBoxBackgroundMode->currentIndex());
    settings.setValue("backgroundImageOpacity", globalOptionsWidget->ui->horizontalSliderBackgroundImageOpacity->value());
    settings.setValue("NewTabWorkPath", globalOptionsWidget->ui->comboBoxNewTabWorkPath->currentText());
    settings.setValue("scrollbackLines", globalOptionsWidget->ui->spinBoxScrollbackLines->value());
    settings.endGroup();
    emit colorSchemeChanged(globalOptionsWidget->ui->comBoxColorSchemes->currentText());
    emit this->accepted();
}

void GlobalOptionsWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}

void GlobalOptionsWindow::showEvent(QShowEvent *event)
{
    int index = globalOptionsWidget->ui->comboBoxBackgroundMode->currentIndex();
    globalOptionsWidget->ui->retranslateUi(this);
    globalOptionsWidget->ui->comboBoxBackgroundMode->setCurrentIndex(index);
    QDialog::showEvent(event);
}
