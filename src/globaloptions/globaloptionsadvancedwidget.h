#ifndef GLOBALOPTIONSADVANCEDWIDGET_H
#define GLOBALOPTIONSADVANCEDWIDGET_H

#include <QWidget>

namespace Ui {
class GlobalOptionsAdvancedWidget;
}

class GlobalOptionsAdvancedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalOptionsAdvancedWidget(QWidget *parent = nullptr);
    ~GlobalOptionsAdvancedWidget();

    Ui::GlobalOptionsAdvancedWidget *ui;
};

#endif // GLOBALOPTIONSADVANCEDWIDGET_H
