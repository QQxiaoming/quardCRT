#include <QSplitter>
#include <QTreeView>
#include <QLabel>
#include <QStandardItemModel>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "optionswindow.h"
#include "ui_optionswindow.h"

OptionsWindow::OptionsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("Session Options"));

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    ui->horizontalLayout->addWidget(splitter);
    QTreeView *treeView = new QTreeView(this);
    treeView->setHeaderHidden(true);
    QStandardItemModel *model = new QStandardItemModel(treeView);
    treeView->setModel(model);
    QStandardItem *item = new QStandardItem("Global");
    model->appendRow(item);
    splitter->addWidget(treeView);
    QWidget *widget = new QWidget(this);
    splitter->addWidget(widget);
    widget->setLayout(new QVBoxLayout(widget));
    splitter->setSizes(QList<int>() << 1 << 100);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

    QLabel *labelColorSchemes = new QLabel(tr("Color Schemes"), widget);
    widget->layout()->addWidget(labelColorSchemes);
    QComboBox *comBoxColorSchemes = new QComboBox(widget);
    widget->layout()->addWidget(comBoxColorSchemes);
    QLabel *labelFontSize = new QLabel(tr("Font Size"), widget);
    widget->layout()->addWidget(labelFontSize);
    QSpinBox *spinBoxFontSize = new QSpinBox(widget);
    widget->layout()->addWidget(spinBoxFontSize);
    QLabel *labelTransparentWindow = new QLabel(tr("Transparent Window"), widget);
    widget->layout()->addWidget(labelTransparentWindow);
    QSlider *sliderTransparentWindow = new QSlider(Qt::Horizontal, widget);
    widget->layout()->addWidget(sliderTransparentWindow);
    QLabel *lableBackgroundImage = new QLabel(tr("Background Image"), widget);
    widget->layout()->addWidget(lableBackgroundImage);
    QHBoxLayout *hBoxLayoutBackgroundImage = new QHBoxLayout(widget);
    hBoxLayoutBackgroundImage->setContentsMargins(0,0,0,0);
    QLineEdit *lineEditBackgroundImage = new QLineEdit(widget);
    hBoxLayoutBackgroundImage->addWidget(lineEditBackgroundImage);
    QToolButton *toolButtonBackgroundImage = new QToolButton(widget);
    toolButtonBackgroundImage->setText("...");
    hBoxLayoutBackgroundImage->addWidget(toolButtonBackgroundImage);
    QWidget *widgetBackgroundImage = new QWidget(widget);
    widgetBackgroundImage->setLayout(hBoxLayoutBackgroundImage);
    widget->layout()->addWidget(widgetBackgroundImage);
    QLabel *labelBackgroundImageMode = new QLabel(tr("Background Image Mode"), widget);
    widget->layout()->addWidget(labelBackgroundImageMode);
    QComboBox *comBoxBackgroundImageMode = new QComboBox(widget);
    widget->layout()->addWidget(comBoxBackgroundImageMode);
    QLabel *labelBackgroundImageOpacity = new QLabel(tr("Background Image Opacity"), widget);
    widget->layout()->addWidget(labelBackgroundImageOpacity);
    QSlider *sliderBackgroundImageOpacity = new QSlider(Qt::Horizontal, widget);
    widget->layout()->addWidget(sliderBackgroundImageOpacity);
    QLabel *labelNewTabWorkingPath = new QLabel(tr("New Tab Working Path"), widget);
    widget->layout()->addWidget(labelNewTabWorkingPath);
    QComboBox *comBoxNewTabWorkingPath = new QComboBox(widget);
    widget->layout()->addWidget(comBoxNewTabWorkingPath);
    QLabel *labelScrollbackLines = new QLabel(tr("Scrollback Lines"), widget);
    widget->layout()->addWidget(labelScrollbackLines);
    QSpinBox *spinBoxScrollbackLines = new QSpinBox(widget);
    widget->layout()->addWidget(spinBoxScrollbackLines);
}

OptionsWindow::~OptionsWindow()
{
    delete ui;
}
