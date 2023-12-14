#ifndef QUICKCOMPLETE_H_
#define QUICKCOMPLETE_H_

#include "plugininterface.h"

#define PLUGIN_NAME    "Quick Complete"
#define PLUGIN_VERSION "0.0.1"

class QuickComplete : public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.quardCRT.PluginInterface" FILE "quickcomplete.json")
    Q_INTERFACES(PluginInterface)

public:
    QuickComplete() : PluginInterface(), m_menu(nullptr), addQuickCompleteAction(nullptr), removeQuickCompleteAction(nullptr), clearQuickCompleteAction(nullptr) {}
    virtual ~QuickComplete() {}

    int init(QMap<QString, QString> params, QWidget *parent);
    void setLanguage(const QLocale &language,QApplication *app);
    void retranslateUi();

    QString name() { return PLUGIN_NAME; }
    QString version() { return PLUGIN_VERSION; }

    QMenu *mainMenu() { return m_menu; }
    QAction *mainAction() { return nullptr; }

    QMenu *terminalContextMenu(QString selectedText, QString workingDirectory, QMenu *parentMenu) {Q_UNUSED(selectedText);Q_UNUSED(workingDirectory);Q_UNUSED(parentMenu); return nullptr;}
    QList<QAction *> terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu) {Q_UNUSED(selectedText);Q_UNUSED(workingDirectory);Q_UNUSED(parentMenu); return QList<QAction *>();}
    
private:
    void quickCompleteListChanged();
    QMenu *m_menu;
    QAction *addQuickCompleteAction;
    QAction *removeQuickCompleteAction;
    QAction *clearQuickCompleteAction;
    QStringList m_quickCompleteList;
};

#endif /* QUICKCOMPLETE_H_ */
