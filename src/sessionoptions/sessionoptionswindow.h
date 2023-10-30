#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QDialog>
#include "quickconnectwindow.h"
#include "sessionoptionsgeneralwidget.h"
#include "sessionoptionstelnetproperties.h"
#include "sessionoptionsserialproperties.h"

namespace Ui {
class SessionOptionsWindow;
}

class SessionOptionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SessionOptionsWindow(QWidget *parent = nullptr);
    ~SessionOptionsWindow();

    void setSessionProperties(QString name, QuickConnectWindow::QuickConnectData data);

private:
    Ui::SessionOptionsWindow *ui;
    SessionOptionsGeneralWidget *sessionOptionsGeneralWidget;
    SessionOptionsTelnetProperties *sessionOptionsTelnetProperties;
    SessionOptionsSerialProperties *sessionOptionsSerialProperties;
};

#endif // OPTIONSWINDOW_H
