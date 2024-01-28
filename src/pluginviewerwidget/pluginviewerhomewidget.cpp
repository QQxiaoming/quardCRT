#include "pluginviewerhomewidget.h"
#include "ui_pluginviewerhomewidget.h"

PluginViewerHomeWidget::PluginViewerHomeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PluginViewerHomeWidget)
{
    ui->setupUi(this);
    retranslateUi();
}

PluginViewerHomeWidget::~PluginViewerHomeWidget() {
    delete ui;
}

void PluginViewerHomeWidget::retranslateUi(void) {
    ui->label->setText(QString("<html><head/><body><p align=\"center\">")+
        tr("Welcome to use")+
        QString("</p><p align=\"center\">")+
        tr("QuardCRT plugin system!")+
        QString("</p></body></html>"));
    ui->retranslateUi(this);
}
