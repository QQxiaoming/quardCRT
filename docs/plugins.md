<div style="text-align: right"><a href="../../en/latest/plugins.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/plugins.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/plugins.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/plugins.html">🇯🇵 日本語</a></div>

# Plugins

quardCRT supports plugins from version V0.4.0, plugins will be provided in the form of Qt plugins, loaded in the form of dynamic libraries.

## Plugin Development

### API

quardCRT's plugin standard API specification is located at [plugininterface](https://github.com/QuardCRT-platform/plugininterface), if you want to develop your custom plugin, the steps are as follows:

- Include plugininterface.h header file
- Define your plugin class and inherit
- Implement the relevant methods required by PluginInterface
- Build DLL/so/dylib according to your native platform and load it using the quardCRT main program

Here is a simple example:

```c++
#include "plugininterface.h"

#include <QMap>
#include <QMessageBox>
#include <QDebug>

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

    int init(QMap<QString, QString> params, QWidget *parent) {
        foreach (QString key, params.keys()) {
            qDebug() << key << " : " << params[key];
        }
        qDebug() << "Hello World init";
        m_action = new QAction("Hello World", parent);
        connect(m_action, &QAction::triggered, [parent](){
            QMessageBox::information(parent, "Hello World", "Hello World");
        });

        return 0;
    }

    void setLanguage(const QLocale &language,QApplication *app) {Q_UNUSED(language);Q_UNUSED(app);}
    void retranslateUi() {}
    QString name() { return PLUGIN_NAME; }
    QString version() { return PLUGIN_VERSION; }

    QMap<QString,void *> metaObject() {
        QMap<QString,void *> ret;
        ret.insert("QAction", (void *)m_action);
        return ret;
    }

    QMenu *terminalContextMenu(QString selectedText, QString workingDirectory, QMenu *parentMenu) {Q_UNUSED(selectedText);Q_UNUSED(workingDirectory);Q_UNUSED(parentMenu); return nullptr;}
    QList<QAction *> terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu) {Q_UNUSED(selectedText);Q_UNUSED(workingDirectory);Q_UNUSED(parentMenu); return QList<QAction *>();}
    
private:
    QAction *m_action;
};
```

### Template Project

For developing plugins, we recommend using the Github template project [plugin-template](https://github.com/QuardCRT-platform/plugin-template), enter the git repository, click use this template in the upper right corner, and you can immediately create your new plugin repository. We have configured the build script, project (pro) file, and enabled github action as CI/CD in the template repository. You only need to focus on your C++ code development and submit it, you can get your plugin build immediately through github action.

## Example

Currently quardCRT already has some plugins and is packaged in the binary installation program, these plugins are still open source, and provided as plugin examples for reference here:

- [plugin-SearchOnWeb](https://github.com/QuardCRT-platform/plugin-SearchOnWeb)

Add right-click context menu to quickly jump the selected terminal content to the search engine, support Google, Bing, Baidu, Github, Stack Overflower.

- [plugin-quickcomplete](https://github.com/QuardCRT-platform/plugin-quickcomplete)

Provide user-defined commands for quick sending to the session.

- [plugin-onestep](https://github.com/QuardCRT-platform/plugin-onestep)

Provide custom pre-filled ssh information, quickly select the host address in the context to quickly connect. (Common operation for initializing production devices in embedded development).

## More

For more information about plugin development, please refer to the plugin open platform [https://github.com/QuardCRT-platform](https://github.com/QuardCRT-platform), the plugin function is still in the early development stage. If you have good ideas or suggestions, please submit issues or discussions on GitHub or Gitee.
