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
#include <QMessageBox>
#include <QScrollBar>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QFile>

#include "filedialog.h"
#include "hexviewwindow.h"
#include "ui_hexviewwindow.h"

HexViewWindow::HexViewWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HexViewWindow)
{
    ui->setupUi(this);
    hexEdit = new QHexEdit(this);
    ui->verticalLayout->insertWidget(0,hexEdit);
    hexEdit->setReadOnly(true);

    setWindowFlags(Qt::Window);
}

HexViewWindow::~HexViewWindow()
{
    delete ui;
}

void HexViewWindow::setFont(const QFont &font)
{
    hexEdit->setFont(font);
}

void HexViewWindow::recvData(const char *data,int size)
{
    if(size > 0) {
        QByteArray ba(data,size);
        hexEdit->insert(ba);
        hexEdit->ensureVisible();
    }
}

void HexViewWindow::hideEvent(QHideEvent *event)
{
    emit hideOrClose();
    Q_UNUSED(event);
}

void HexViewWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    QAction *copyAction = new QAction(tr("Copy"),this);
    connect(copyAction,&QAction::triggered,this,[=](){
        QByteArray ba = hexEdit->getSelection();
        if(ba.isEmpty()) return;
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(ba);
    });
    menu->addAction(copyAction);
    QAction *copyHexAction = new QAction(tr("Copy Hex"),this);
    connect(copyHexAction,&QAction::triggered,this,[=](){
        QByteArray ba = hexEdit->getSelection();
        if(ba.isEmpty()) return;
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(ba.toHex(' '));
    });
    menu->addAction(copyHexAction);
    QAction *dumpAction = new QAction(tr("Dump"),this);
    connect(dumpAction,&QAction::triggered,this,[=](){
        QByteArray ba = hexEdit->getSelection();
        if(ba.isEmpty()) return;
        QString fileName = FileDialog::getSaveFileName(this,tr("Save As"),QString(),tr("Binary File (*.bin)"));
        if(!fileName.isEmpty()) {
            if(!fileName.endsWith(".bin")) fileName.append(".bin");
            QFile file(fileName);
            if(file.open(QIODevice::WriteOnly)) {
                file.write(ba);
                file.close();
            } else {
                QMessageBox::critical(this,tr("Error"),tr("Failed to save file!"));
            }
        }
    });
    menu->addAction(dumpAction);
    QAction *clearAction = new QAction(tr("Clear"),this);
    connect(clearAction,&QAction::triggered,this,[=](){
        hexEdit->setData(QByteArray());
    });
    menu->addAction(clearAction);
    menu->move(cursor().pos()+QPoint(5,5));
    menu->show();
    Q_UNUSED(event);
}

