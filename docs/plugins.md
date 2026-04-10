<div style="text-align: right"><a href="../../en/latest/plugins.html">🇺🇸 English</a> | <a href="../../zh-cn/latest/plugins.html">🇨🇳 简体中文</a> | <a href="../../zh-tw/latest/plugins.html">🇭🇰 繁體中文</a> | <a href="../../ja/latest/plugins.html">🇯🇵 日本語</a></div>

# Plugins

quardCRT supports plugins from version V0.4.0. Plugins are implemented as Qt plugins and loaded as dynamic libraries.

This page covers both sides of the plugin system:

- what plugin support means for regular users
- how developers can build plugins against the quardCRT plugin API

## For users

Plugins can extend quardCRT in several ways, depending on what the plugin provides:

- add actions or menus to the main interface
- add items to the terminal context menu
- provide custom plugin widgets inside the application sidebar area
- trigger connection-related actions such as opening SSH, serial, raw socket, or VNC sessions

quardCRT also includes a plugin information window that shows plugin state, compatibility information, and initialization errors.

### Plugin loading behavior

At startup, quardCRT tries to load bundled plugins from its plugin directory. It can also load plugins from an optional user-defined plugin directory if one is configured in the application settings.

If a plugin cannot be loaded, quardCRT records the failure in the plugin information dialog instead of silently treating it as a working plugin.

Typical reasons a plugin may not load include:

- missing plugin metadata
- unsupported plugin API version
- initialization failure
- missing menu or widget entry points expected by quardCRT

### Enabling and disabling plugins

Plugin enable state is persisted in application settings. Supported plugins can be enabled or disabled from the plugin information dialog.

This is useful when:

- you want to temporarily disable a plugin without removing its files
- you are testing a new plugin and want a simple rollback path
- you want different plugin combinations on different machines

## For developers

## Plugin Development

### API

The plugin API specification is maintained in the [plugininterface](https://github.com/QuardCRT-platform/plugininterface) repository.

To develop a custom plugin, the usual flow is:

- Include plugininterface.h header file
- Define your plugin class and inherit
- Implement the relevant methods required by PluginInterface
- Build a native plugin library for your platform, such as `dll`, `so`, or `dylib`
- Place the built plugin where quardCRT can load it

In practice, a useful plugin usually provides one or more of the following:

- a main menu action or menu
- a sidebar widget
- terminal context menu entries
- settings storage via the plugin read or write settings hooks
- localization support through `setLanguage()` and `retranslateUi()`

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

For new plugin development, the recommended starting point is the GitHub template project [plugin-template](https://github.com/QuardCRT-platform/plugin-template).

The template helps you get started faster because it already includes:

- a project structure for plugin development
- build script and project configuration
- CI setup through GitHub Actions

This lets you focus on plugin logic instead of building the project skeleton from scratch.

### Compatibility notes

When building plugins, keep these points in mind:

- plugins must target the quardCRT plugin API version expected by the running application
- plugins are native binaries, so compiler, Qt version, architecture, and platform compatibility matter
- if a plugin depends on extra shared libraries, those libraries must also be available at runtime

If a plugin looks correct but still fails to load, the plugin information window is the first place to check.

## Example

quardCRT already has several open source plugins that can be used as references:

- [plugin-SearchOnWeb](https://github.com/QuardCRT-platform/plugin-SearchOnWeb)

Adds terminal context menu actions that search selected text on Google, Bing, Baidu, GitHub, and similar services.

- [plugin-quickcomplete](https://github.com/QuardCRT-platform/plugin-quickcomplete)

Provides user-defined commands for quick sending to the current session.

- [plugin-onestep](https://github.com/QuardCRT-platform/plugin-onestep)

Provides pre-filled SSH connection information and faster host selection workflows, which can be useful in embedded development or device initialization scenarios.

## More

For more information, templates, and related repositories, see the quardCRT plugin platform:

- [https://github.com/QuardCRT-platform](https://github.com/QuardCRT-platform)

The plugin system is still evolving. If you have ideas for plugin APIs, extension points, or developer tooling, open an issue or discussion on GitHub or Gitee.
