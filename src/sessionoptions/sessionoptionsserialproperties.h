#ifndef SESSIONOPTIONSSERIALPROPERTIES_H
#define SESSIONOPTIONSSERIALPROPERTIES_H

#include <QWidget>

namespace Ui {
class SessionOptionsSerialProperties;
}

class SessionOptionsSerialProperties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsSerialProperties(QWidget *parent = nullptr);
    ~SessionOptionsSerialProperties();

    Ui::SessionOptionsSerialProperties *ui;
};

#endif // SESSIONOPTIONSSERIALPROPERTIES_H
