#ifndef SESSIONOPTIONSTELNETPROPERTIES_H
#define SESSIONOPTIONSTELNETPROPERTIES_H

#include <QWidget>

namespace Ui {
class SessionOptionsTelnetProperties;
}

class SessionOptionsTelnetProperties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsTelnetProperties(QWidget *parent = nullptr);
    ~SessionOptionsTelnetProperties();

    Ui::SessionOptionsTelnetProperties *ui;
};

#endif // SESSIONOPTIONSTELNETPROPERTIES_H
