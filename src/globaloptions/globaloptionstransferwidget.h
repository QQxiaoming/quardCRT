#ifndef GLOBALOPTIONSTRANSFERWIDGET_H
#define GLOBALOPTIONSTRANSFERWIDGET_H

#include <QWidget>

namespace Ui {
class GlobalOptionsTransferWidget;
}

class GlobalOptionsTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalOptionsTransferWidget(QWidget *parent = nullptr);
    ~GlobalOptionsTransferWidget();

    Ui::GlobalOptionsTransferWidget *ui;
};

#endif // GLOBALOPTIONSTRANSFERWIDGET_H
