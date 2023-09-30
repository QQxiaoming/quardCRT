#ifndef SESSIONMANAGERWIDGET_H
#define SESSIONMANAGERWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QLabel>

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
    void retranslateUi();

private:
    Ui::SessionManagerWidget *ui;
    QToolBar *toolBar;
    QLabel *label;
};

#endif // SESSIONMANAGERWIDGET_H
