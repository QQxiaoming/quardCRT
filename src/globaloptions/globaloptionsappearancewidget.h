#ifndef GLOBALOPTIONSAPPEARANCEWIDGET_H
#define GLOBALOPTIONSAPPEARANCEWIDGET_H

#include <QWidget>

namespace Ui {
class GlobalOptionsAppearanceWidget;
}

class GlobalOptionsAppearanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalOptionsAppearanceWidget(QWidget *parent = nullptr);
    ~GlobalOptionsAppearanceWidget();

    Ui::GlobalOptionsAppearanceWidget *ui;
};

#endif // GLOBALOPTIONSAPPEARANCEWIDGET_H
