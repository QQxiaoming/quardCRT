#ifndef SESSIONMANAGERWIDGET_H
#define SESSIONMANAGERWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QLabel>

#include "treemodel.h"

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
    void addSession(QString str, int type);
    void removeSession(QString str);
    bool checkSession(QString str);
    
private:
    Ui::SessionManagerWidget *ui;
    QToolBar *toolBar;
    QLabel *label;
    SessionTreeWindow *tree;
};

#endif // SESSIONMANAGERWIDGET_H
