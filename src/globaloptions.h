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
#ifndef GLOBALOPTIONS_H
#define GLOBALOPTIONS_H

#include <QDialog>

namespace Ui {
class GlobalOptions;
}

class GlobalOptions : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalOptions(QWidget *parent = nullptr);
    ~GlobalOptions();
    void setAvailableColorSchemes(QStringList colorSchemes);
    QString getCurrentColorScheme(void);
    QFont getCurrentFont(void);
    int getTransparency(void);
    QString getBackgroundImage(void);
    int getBackgroundImageMode(void);
    qreal getBackgroundImageOpacity(void);
    QString getNewTabWorkPath(void);

    const static QString defaultColorScheme;

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
    Ui::GlobalOptions *ui;
    QFont font;
};

#endif // GLOBALOPTIONS_H
