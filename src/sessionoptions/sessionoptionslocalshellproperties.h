#ifndef SESSIONOPTIONSLOCALSHELLPROPERTIES_H
#define SESSIONOPTIONSLOCALSHELLPROPERTIES_H

#include <QWidget>

namespace Ui {
class SessionOptionsLocalShellProperties;
}

class SessionOptionsLocalShellProperties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsLocalShellProperties(QWidget *parent = nullptr);
    ~SessionOptionsLocalShellProperties();

    Ui::SessionOptionsLocalShellProperties *ui;
};

#endif // SESSIONOPTIONSLOCALSHELLPROPERTIES_H
