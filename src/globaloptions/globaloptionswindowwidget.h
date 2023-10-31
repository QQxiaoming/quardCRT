#ifndef GLOBALOPTIONSWINDOWWIDGET_H
#define GLOBALOPTIONSWINDOWWIDGET_H

#include <QWidget>

namespace Ui {
class GlobalOptionsWindowWidget;
}

class GlobalOptionsWindowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalOptionsWindowWidget(QWidget *parent = nullptr);
    ~GlobalOptionsWindowWidget();

    Ui::GlobalOptionsWindowWidget *ui;
};

#endif // GLOBALOPTIONSWINDOWWIDGET_H
