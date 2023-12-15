#ifndef PLUGININFOWINDOW_H
#define PLUGININFOWINDOW_H

#include <QDialog>
#include <QList>
#include "plugininterface.h"

namespace Ui {
class PluginInfoWindow;
}

class PluginInfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PluginInfoWindow(QWidget *parent = nullptr);
    ~PluginInfoWindow();
    void addPluginInfo(PluginInterface *plugin, uint32_t apiVersion, bool enable);
    void addPluginInfo(QString name, QString version, uint32_t apiVersion, bool enable);

private:
    Ui::PluginInfoWindow *ui;
};

#endif // PLUGININFOWINDOW_H
