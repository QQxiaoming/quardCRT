#ifndef PLUGINVIEWERHOMEWIDGET_H
#define PLUGINVIEWERHOMEWIDGET_H

#include <QWidget>

namespace Ui {
class PluginViewerHomeWidget;
}

class PluginViewerHomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginViewerHomeWidget(QWidget *parent = nullptr);
    ~PluginViewerHomeWidget();
    void retranslateUi(void);

private:
    Ui::PluginViewerHomeWidget *ui;
};

#endif // PLUGINVIEWERHOMEWIDGET_H
