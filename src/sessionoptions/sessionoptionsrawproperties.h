#ifndef SESSIONOPTIONSRAWPROPERTIES_H
#define SESSIONOPTIONSRAWPROPERTIES_H

#include <QWidget>

namespace Ui {
class SessionOptionsRawProperties;
}

class SessionOptionsRawProperties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsRawProperties(QWidget *parent = nullptr);
    ~SessionOptionsRawProperties();

    Ui::SessionOptionsRawProperties *ui;
};

#endif // SESSIONOPTIONSRAWPROPERTIES_H
