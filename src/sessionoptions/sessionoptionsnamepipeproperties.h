#ifndef SESSIONOPTIONSNAMEPIPEPROPERTIES_H
#define SESSIONOPTIONSNAMEPIPEPROPERTIES_H

#include <QWidget>

namespace Ui {
class SessionOptionsNamePipeProperties;
}

class SessionOptionsNamePipeProperties : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsNamePipeProperties(QWidget *parent = nullptr);
    ~SessionOptionsNamePipeProperties();

    Ui::SessionOptionsNamePipeProperties *ui;
};

#endif // SESSIONOPTIONSNAMEPIPEPROPERTIES_H
