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
#include "hexviewwindow.h"
#include "ui_hexviewwindow.h"

HexViewWindow::HexViewWindow(int type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HexViewWindow),
    m_type(type)
{
    ui->setupUi(this);

    QPalette p = ui->textEditHEX->palette();
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Base, Qt::black);
    ui->textEditHEX->setPalette(p);
    ui->textEditASCII->setPalette(p);

    setWindowTitle(tr("Hex View"));

    if(type == SEND) {
        ui->textEditHEX->setReadOnly(false);
        ui->buttonBox->setVisible(true);
        ui->textEditASCII->setVisible(false);
    } else if(type == RECV){
        ui->buttonBox->setEnabled(false);
        ui->buttonBox->setVisible(false);
        ui->textEditHEX->setReadOnly(true);
        ui->textEditHEX->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->textEditHEX->setFocusPolicy(Qt::NoFocus);
        ui->textEditHEX->setLineWrapMode(QTextEdit::FixedColumnWidth);
        ui->textEditHEX->setLineWrapColumnOrWidth(3*16+2);
        ui->textEditASCII->setReadOnly(true);
        ui->textEditASCII->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->textEditASCII->setFocusPolicy(Qt::NoFocus);
        ui->textEditASCII->setLineWrapMode(QTextEdit::FixedColumnWidth);
        ui->textEditASCII->setLineWrapColumnOrWidth(16);
    }

    connect(ui->textEditHEX->verticalScrollBar(), &QScrollBar::valueChanged, ui->textEditASCII->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->textEditASCII->verticalScrollBar(), &QScrollBar::valueChanged, ui->textEditHEX->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

HexViewWindow::~HexViewWindow()
{
    delete ui;
}

void HexViewWindow::setFont(const QFont &font)
{
    ui->textEditHEX->setFont(font);
}

void HexViewWindow::buttonBoxAccepted(void)
{
    if(m_type == SEND) {
        QString input = ui->textEditHEX->toPlainText();
        QByteArray array = QByteArray::fromHex(input.toLatin1());
        if(!array.isEmpty()) {
            QMessageBox::information(this, tr("Information"), tr("Will send Hex:\n")+array.toHex(' '));
            emit sendData(array.constData(),array.size());
            ui->textEditHEX->setPlainText(array.toHex(' '));
        }
    }
    emit this->accepted();
}

void HexViewWindow::buttonBoxRejected(void)
{
    emit this->rejected();
}

void HexViewWindow::recvData(const char *data,int size)
{
    auto insertPlainText = [&](const char *data,int size){
        if(size > 0) {
            QByteArray ba(data,size);
            ui->textEditHEX->insertPlainText(ba.toHex(' ') + " ");
            for(int j=0;j<size;j++) {
                uint8_t ch = ba.at(j);
                const static QString ascii_table[256] = {
                    ".",".",".",".",".",".",".",".",
                    ".",".",".",".",".",".",".",".",
                    ".",".",".",".",".",".",".",".",
                    ".",".",".",".",".",".",".",".",
                    ".","!","\"","#","$","%","&","\'",
                    "(",")","*","+",",","-",".","/",
                    "0","1","2","3","4","5","6","7",
                    "8","9",":",";","<","=",">","?",
                    "@","A","B","C","D","E","F","G",
                    "H","I","J","K","L","M","N","O",
                    "P","Q","R","S","T","U","V","W",
                    "X","Y","Z","[","\\","]","^",".",
                    "`","a","b","c","d","e","f","g",
                    "h","i","j","k","l","m","n","o",
                    "p","q","r","s","t","u","v","w",
                    "x","y","z","{","|","}","~",".",
                    "€",".",".","ƒ",".",".","†","‡",
                    "ˆ","‰","Š","‹","Œ",".","Ž",".",
                    ".",".",".",".",".",".","–",".",
                    "˜","™","š","›","œ",".","ž","Ÿ",
                    ".","¡","¢","£",".","¥","¦",".",
                    "¨",".","ª","«","¬",".",".","¯",
                    "°",".","²","³","´","µ","¶",".",
                    "¸","¹","º","»","¼","½","¾","¿",
                    "À","Á","Â","Ã","Ä","Å","Æ","Ç",
                    "È","É","Ê","Ë","Ì","Í","Î","Ï",
                    "Ð","Ñ","Ò","Ó","Ô","Õ","Ö",".",
                    "Ø","Ù","Ú","Û","Ü","Ý","Þ","ß",
                    "à","á","â","ã","ä","å","æ","ç",
                    "è","é","ê","ë","ì","í","î","ï",
                    "ð","ñ","ò","ó","ô","õ","ö",".",
                    "ø","ù","ú","û","ü","ý","þ","ÿ"
                };
                ui->textEditASCII->insertPlainText(ascii_table[ch]);
            }
        }
    };
    if(size > 0) {
        int firstSize = ((8*3+1) - ui->textEditHEX->toPlainText().size()%(8*3+1))/3;
        if(size >= firstSize) {
            insertPlainText(data,firstSize);
            ui->textEditHEX->insertPlainText(" ");

            int i=firstSize;
            for(i=firstSize;i+8<size;i+=8) {
                insertPlainText(data+i,8);
                ui->textEditHEX->insertPlainText(" ");
            }
            if(i < size) {
                insertPlainText(data+i,size-i);
            }
        } else {
            insertPlainText(data,size);
        }
        ui->textEditHEX->ensureCursorVisible();
        ui->textEditASCII->ensureCursorVisible();
    }
}

void HexViewWindow::hideEvent(QHideEvent *event)
{
    emit hideOrClose();
    Q_UNUSED(event);
}

void HexViewWindow::on_pushButton_clicked()
{
    ui->textEditHEX->clear();
}

