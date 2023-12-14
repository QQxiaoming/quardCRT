#ifndef HELLOWORLD_H_
#define HELLOWORLD_H_

#include "plugininterface.h"

#define PLUGIN_NAME    "Hello World"
#define PLUGIN_VERSION "0.0.1"

class HelloWorld : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.quardCRT.PluginInterface" FILE "../plugininterface.json")
    Q_INTERFACES(PluginInterface)

public:
    HelloWorld() : m_action(nullptr) {}
    virtual ~HelloWorld() {}

    int init(QMap<QString, QString> params, QWidget *parent);

    void setLanguage(const QLocale &language,QApplication *app) {Q_UNUSED(language);Q_UNUSED(app);}
    void retranslateUi() {}
    QString name() { return PLUGIN_NAME; }
    QString version() { return PLUGIN_VERSION; }

    QMenu *mainMenu() { return nullptr; }
    QAction *mainAction() { return m_action; }

    QMenu *terminalContextMenu(QString selectedText, QString workingDirectory, QMenu *parentMenu) {Q_UNUSED(selectedText);Q_UNUSED(workingDirectory);Q_UNUSED(parentMenu); return nullptr;}
    QList<QAction *> terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu) {Q_UNUSED(selectedText);Q_UNUSED(workingDirectory);Q_UNUSED(parentMenu); return QList<QAction *>();}
    
private:
    QAction *m_action;
};

#endif /* HELLOWORLD_H_ */
