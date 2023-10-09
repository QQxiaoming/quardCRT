#ifndef MAINWIDGETGROUP_H
#define MAINWIDGETGROUP_H

#include <QObject>
#include <QSplitter>
#include "commandwindow.h"
#include "sessiontab.h"

class MainWidgetGroup: public QObject
{
    Q_OBJECT
public:
    MainWidgetGroup(QWidget *parent = nullptr);
    ~MainWidgetGroup();

public:
    QSplitter *splitter;
    SessionTab *sessionTab;
    CommandWindow *commandWindow;
};

#endif // MAINWIDGETGROUP_H
