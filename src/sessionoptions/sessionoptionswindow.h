#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QDialog>
#include <QShowEvent>
#include <QStringListModel>
#include "quickconnectwindow.h"
#include "sessionoptionsgeneralwidget.h"
#include "sessionoptionstelnetproperties.h"
#include "sessionoptionsserialproperties.h"
#include "sessionoptionslocalshellproperties.h"
#include "sessionoptionsnamepipeproperties.h"
#include "sessionoptionsrawproperties.h"

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
    void retranslateUi();

private:
    void setactiveProperties(int index);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::SessionOptionsWindow *ui;
    SessionOptionsGeneralWidget *sessionOptionsGeneralWidget;
    SessionOptionsTelnetProperties *sessionOptionsTelnetProperties;
    SessionOptionsSerialProperties *sessionOptionsSerialProperties;
    SessionOptionsLocalShellProperties *sessionOptionsLocalShellProperties;
    SessionOptionsNamePipeProperties *sessionOptionsNamePipeProperties;
    SessionOptionsRawProperties *sessionOptionsRawProperties;
    QStringListModel *model;
};

#endif // OPTIONSWINDOW_H
