#ifndef SESSIONMANAGERWIDGET_H
#define SESSIONMANAGERWIDGET_H

#include <QWidget>
#include <QToolBar>

namespace Ui {
class SessionManagerWidget;
}

class SessionManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionManagerWidget(QWidget *parent = nullptr);
    ~SessionManagerWidget();

    void addActionOnToolBar(QAction *action);

private:
    Ui::SessionManagerWidget *ui;
    QToolBar *toolBar;
};

#endif // SESSIONMANAGERWIDGET_H
