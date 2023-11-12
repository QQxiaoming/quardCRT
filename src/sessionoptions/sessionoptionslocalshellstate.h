#ifndef SESSIONOPTIONSLOCALSHELLSTATE_H
#define SESSIONOPTIONSLOCALSHELLSTATE_H

#include <QWidget>

namespace Ui {
class SessionOptionsLocalShellState;
}

class SessionOptionsLocalShellState : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsLocalShellState(QWidget *parent = nullptr);
    ~SessionOptionsLocalShellState();

    Ui::SessionOptionsLocalShellState *ui;
};

#endif // SESSIONOPTIONSLOCALSHELLSTATE_H
