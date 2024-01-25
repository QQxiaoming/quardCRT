#include "pluginviewerhomewidget.h"
#include "ui_pluginviewerhomewidget.h"

PluginViewerHomeWidget::PluginViewerHomeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PluginViewerHomeWidget)
{
    ui->setupUi(this);
}

PluginViewerHomeWidget::~PluginViewerHomeWidget()
{
    delete ui;
}
