#ifndef PLUGINVIEWERWIDGET_H
#define PLUGINVIEWERWIDGET_H

#include <QWidget>
#include <QMap>

#include "pluginviewerhomewidget.h"

namespace Ui {
class PluginViewerWidget;
}

class PluginViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginViewerWidget(QWidget *parent = nullptr);
    ~PluginViewerWidget();
    void addPlugin(QWidget *pluginWidget, const QString &pluginName);

private:
    Ui::PluginViewerWidget *ui;
    PluginViewerHomeWidget *m_homeWidget;
    QMap<QString, QWidget*> m_plugins;
};

#endif // PLUGINVIEWERWIDGET_H
