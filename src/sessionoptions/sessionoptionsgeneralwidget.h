#ifndef SESSIONOPTIONSGENERALWIDGET_H
#define SESSIONOPTIONSGENERALWIDGET_H

#include <QWidget>

namespace Ui {
class SessionOptionsGeneralWidget;
}

class SessionOptionsGeneralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionOptionsGeneralWidget(QWidget *parent = nullptr);
    ~SessionOptionsGeneralWidget();

    Ui::SessionOptionsGeneralWidget *ui;
};

#endif // SESSIONOPTIONSGENERALWIDGET_H
