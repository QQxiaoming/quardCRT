#ifndef INTERNALLOGWINDOW_H
#define INTERNALLOGWINDOW_H

#include <QDialog>

namespace Ui {
class InternalLogWindow;
}

class InternalLogWindow : public QDialog
{
    Q_OBJECT

public:
    explicit InternalLogWindow(QWidget *parent = nullptr);
    ~InternalLogWindow();

private:
    Ui::InternalLogWindow *ui;
};

#endif // INTERNALLOGWINDOW_H
