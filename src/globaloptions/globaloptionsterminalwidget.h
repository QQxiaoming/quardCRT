#ifndef GLOBALOPTIONSTERMINALWIDGET_H
#define GLOBALOPTIONSTERMINALWIDGET_H

#include <QWidget>

namespace Ui {
class GlobalOptionsTerminalWidget;
}

class GlobalOptionsTerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalOptionsTerminalWidget(QWidget *parent = nullptr);
    ~GlobalOptionsTerminalWidget();

    Ui::GlobalOptionsTerminalWidget *ui;
};

#endif // GLOBALOPTIONSTERMINALWIDGET_H
