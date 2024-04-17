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
#ifndef GLOBALOPTIONSWINDOW_H
#define GLOBALOPTIONSWINDOW_H

#include <QDialog>
#include <QStringListModel>
#include <QColor>
#include <QList>
#include "globaloptionsgeneralwidget.h"
#include "globaloptionsappearancewidget.h"
#include "globaloptionsterminalwidget.h"
#include "globaloptionswindowwidget.h"
#include "globaloptionstransferwidget.h"
#include "globaloptionsadvancedwidget.h"
#include "ColorTables.h"

namespace Ui {
class GlobalOptionsWindow;
}

class GlobalOptionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalOptionsWindow(QWidget *parent = nullptr);
    ~GlobalOptionsWindow();

    void retranslateUi();
    void setAvailableColorSchemes(QStringList colorSchemes);
    QString getCurrentColorScheme(void);
    QList<QColor> getColorTable(void);
    QFont getCurrentFont(void);
    int getTransparency(void);
    QString getBackgroundImage(void);
    int getBackgroundImageMode(void);
    qreal getBackgroundImageOpacity(void);
    QString getNewTabWorkPath(void);
    uint32_t getScrollbackLines(void);
    bool getEnableTerminalBackgroundAnimation(void);
    uint32_t getCursorShape(void);
    bool getCursorBlink(void);
    int getTabTitleMode(void);
    int getTabTitleWidth(void);
    bool getEnableTabPreview(void);
    int getTabPreviewWidth(void);
    QString getWordCharacters(void);
    int getSelectedTextAccentColorTransparency(void);
    int getTranslateService(void);
    int getNewTabMode(void);
    QString getModemDownloadPath(void);
    QString getModemUploadPath(void);
    bool getDisableZmodemOnline(void);
    bool getXYModem1K(void);
    int getPreeditColorIndex(void);

    bool updateColorButtons(const QString &text);

    const static QString defaultColorScheme;

private:
    void setActiveWidget(int index);

signals:
    void colorSchemeChanged(QString colorScheme);
    void transparencyChanged(int transparency);

private slots:
    void fontSizeChanged(int size);
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::GlobalOptionsWindow *ui;
    GlobalOptionsGeneralWidget *globalOptionsGeneralWidget;
    GlobalOptionsAppearanceWidget *globalOptionsAppearanceWidget;
    GlobalOptionsTerminalWidget *globalOptionsTerminalWidget;
    GlobalOptionsWindowWidget *globalOptionsWindowWidget;
    GlobalOptionsTransferWidget *globalOptionsTransferWidget;
    GlobalOptionsAdvancedWidget *globalOptionsAdvancedWidget;
    QStringListModel *model;
    QFont font;
    Konsole::ColorEntry table[TABLE_COLORS];
};

#endif //GLOBALOPTIONSWINDOW_H
