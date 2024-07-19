#ifndef INTERNALCOMMANDWINDOW_H
#define INTERNALCOMMANDWINDOW_H

#include <QDialog>
#include <QContextMenuEvent>
#include "qtermwidget.h"
#include "internalcommandprocess.h"

namespace Ui {
class InternalCommandWindow;
}

class InternalCommandWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InternalCommandWindow(QWidget *parent = nullptr);
    ~InternalCommandWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    Ui::InternalCommandWindow *ui;
    QTermWidget *term;
    InternalCommandProcess *process;
};

#endif // INTERNALCOMMANDWINDOW_H
