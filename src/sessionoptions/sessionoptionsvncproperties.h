#ifndef SESSIONOPTIONSVNCPROPERTIES_H
#define SESSIONOPTIONSVNCPROPERTIES_H

#include <QWidget>
#include "passwordedit.h"

namespace Ui {
class SessionOptionsVNCProperties;
}

class SessionOptionsVNCProperties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsVNCProperties(QWidget *parent = nullptr);
    ~SessionOptionsVNCProperties();

    Ui::SessionOptionsVNCProperties *ui;
    PasswordEdit *lineEditPassword;
};

#endif // SESSIONOPTIONSVNCPROPERTIES_H
