#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QMap>
#include <QMenu>
#include <QAction>

class PluginInterface
{
public:
    virtual ~PluginInterface() {}
    virtual int init(QMap<QString, QString> params, QWidget *parent) = 0;
    virtual QString name() = 0;
    virtual QMenu *menu() = 0;
    virtual QAction *action() = 0;
};

#define PluginInterface_iid "org.quardCRT.PluginInterface"

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
