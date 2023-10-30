#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QDialog>
#include "globaloptionswindow.h"

namespace Ui {
class SessionOptionsWindow;
}

class SessionOptionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SessionOptionsWindow(QWidget *parent = nullptr);
    ~SessionOptionsWindow();

private:
    Ui::SessionOptionsWindow *ui;
};

#endif // OPTIONSWINDOW_H
