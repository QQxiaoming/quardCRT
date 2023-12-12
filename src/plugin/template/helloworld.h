#ifndef HELLOWORLD_H_
#define HELLOWORLD_H_

#include "plugininterface.h"

class HelloWorld : public QObject, PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "org.quardCRT.PluginInterface" FILE "helloworld.json")

public:
    HelloWorld();
    virtual ~HelloWorld();

    int init(QMap<QString, QString> params, QWidget *parent);
    QString name();

    QMenu *menu();
    QAction *action();

private:
    QAction *m_action;
};

#endif /* HELLOWORLD_H_ */
