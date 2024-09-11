/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <QDialog>
#include <QShowEvent>
#include <QStringListModel>
#include <QStackedWidget>

#include "ptyqt.h"
#include "quickconnectwindow.h"
#include "sessionswindow.h"

#include "sessionoptionsgeneralwidget.h"

#include "sessionoptionstelnetproperties.h"
#include "sessionoptionsserialproperties.h"
#include "sessionoptionslocalshellproperties.h"
#include "sessionoptionsnamepipeproperties.h"
#include "sessionoptionsrawproperties.h"
#include "sessionoptionsssh2properties.h"
#include "sessionoptionsvncproperties.h"

#include "sessionoptionslocalshellstate.h"
#include "sessionoptionsserialstate.h"

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
    void setSessionState(SessionsWindow::StateInfo state);
    void setReadOnly(bool enable);

    void retranslateUi();

signals:
    void sessionPropertiesChanged(QString name, QuickConnectWindow::QuickConnectData data, QString newName);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    void setactiveProperties(int index);
    void setactiveState(int index);

private:
    Ui::SessionOptionsWindow *ui;
    QWidget *emptyWidget;
    SessionOptionsGeneralWidget *sessionOptionsGeneralWidget;
    SessionOptionsTelnetProperties *sessionOptionsTelnetProperties;
    SessionOptionsSerialProperties *sessionOptionsSerialProperties;
    SessionOptionsLocalShellProperties *sessionOptionsLocalShellProperties;
    SessionOptionsNamePipeProperties *sessionOptionsNamePipeProperties;
    SessionOptionsRawProperties *sessionOptionsRawProperties;
    SessionOptionsSsh2Properties *sessionOptionsSSH2Properties;
    SessionOptionsVNCProperties *sessionOptionsVNCProperties;
    SessionOptionsLocalShellState *sessionOptionsLocalShellState;
    SessionOptionsSerialState *sessionOptionsSerialState;
    QStringListModel *model;
    QString currentSessionName;
};

#endif // OPTIONSWINDOW_H
