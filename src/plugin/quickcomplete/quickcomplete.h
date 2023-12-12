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

    QMenu *menu() { return m_menu; }
    QAction *action() { return nullptr; }

private:
    void quickCompleteListChanged();
    QMenu *m_menu;
    QAction *addQuickCompleteAction;
    QAction *removeQuickCompleteAction;
    QAction *clearQuickCompleteAction;
    QStringList m_quickCompleteList;
};

#endif /* QUICKCOMPLETE_H_ */
