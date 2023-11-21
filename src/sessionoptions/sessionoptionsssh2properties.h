#ifndef SESSIONOPTIONSSSH2PROPERTIES_H
#define SESSIONOPTIONSSSH2PROPERTIES_H

#include <QWidget>

namespace Ui {
class SessionOptionsSsh2Properties;
}

class SessionOptionsSsh2Properties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsSsh2Properties(QWidget *parent = nullptr);
    ~SessionOptionsSsh2Properties();

    Ui::SessionOptionsSsh2Properties *ui;
};

#endif // SESSIONOPTIONSSSH2PROPERTIES_H
