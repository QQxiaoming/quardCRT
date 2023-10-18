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
#ifndef HEXVIEWWINDOWS_H
#define HEXVIEWWINDOWS_H

#include <QDialog>
#include <QHideEvent>
#include <QFont>
#include <QMutex>

namespace Ui {
class HexViewWindow;
}

class HexViewWindow : public QDialog
{
    Q_OBJECT

public:
    explicit HexViewWindow(int type, QWidget *parent = nullptr);
    ~HexViewWindow();
    void setFont(const QFont &font);

    enum {
        SEND = 0,
        RECV = 1,
    };

signals:
    void sendData(const char *,int);
    void hideOrClose();

protected:
    void hideEvent(QHideEvent *event);

public slots:
    void recvData(const char *data,int size);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);
    void on_pushButton_clicked();

private:
    Ui::HexViewWindow *ui;
    int m_type;
    QMutex m_mutex;
};

#endif // HEXVIEWWINDOWS_H
