/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
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
#include <QFontDatabase>
#include <QLabel>
#include <QPainter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QMenu>
#include <QAction>
#include <QList>

#include "globalsetting.h"
#include "globaloptionswindow.h"
#include "internalcommandwindow.h"
#include "qfonticon.h"
#include "mainwindow.h"
#include "ui_internalcommandwindow.h"

InternalCommandWindow::InternalCommandWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InternalCommandWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    process = new InternalCommandProcess(static_cast<CentralWidget*>(parent),this);
    term = new QTermWidget(this,this);
    term->setUrlFilterEnabled(false);
    term->setScrollBarPosition(QTermWidget::ScrollBarRight);
    term->setBlinkingCursor(true);
    term->setMargin(0);
    term->setDrawLineChars(false);
    term->setSelectionOpacity(0.5);
    term->setEnableHandleCtrlC(true);

    QFont font = QApplication::font();
#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
    int fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/inziu-iosevkaCC-SC-regular.ttf");
#else
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
#endif
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    GlobalSetting settings;
    int fontsize = settings.value("Global/InternalCommand/fontsize", 12).toInt();
    font.setPointSize(fontsize);
    term->setTerminalFont(font);
    
    QStringList availableColorSchemes = term->availableColorSchemes();
    availableColorSchemes.sort();
    QString currentColorScheme = availableColorSchemes.first();
    foreach(QString colorScheme, availableColorSchemes) {
        if(colorScheme == GlobalOptionsWindow::defaultColorScheme) {
            term->setColorScheme(GlobalOptionsWindow::defaultColorScheme);
            currentColorScheme = GlobalOptionsWindow::defaultColorScheme;
        }
    }

    QStringList availableKeyBindings = term->availableKeyBindings();
    availableKeyBindings.sort();
    QString currentAvailableKeyBindings = availableKeyBindings.first();
    foreach(QString keyBinding, availableKeyBindings) {
        if(keyBinding == "linux") {
            term->setKeyBindings("linux");
            currentAvailableKeyBindings = "linux";
        }
    }

    ui->verticalLayout->addWidget(term);

    connect(term, &QTermWidget::sendData,this,
        [&](const char *data, int size){
        QByteArray sendData(data, size);
        process->recvData(sendData);
    });
    connect(process, &InternalCommandProcess::sendData, this,
        [&](const QByteArray &data){
        term->recvData(data.data(), data.size());
    });

    bool supportSelection = QApplication::clipboard()->supportsSelection();
    if(!supportSelection) {
        connect(term, &QTermWidget::mousePressEventForwarded, this, [&](QMouseEvent *event) {
            if(event->button() == Qt::MiddleButton) {
                term->copyClipboard();
                term->pasteClipboard();
            }
        });
    }
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    connect(term, &QTermWidget::handleCtrlC, this, [&](void){
        QString text = term->selectedText();
        if(!text.isEmpty()) {
            QApplication::clipboard()->setText(text, QClipboard::Clipboard);
        }
    });
#endif

    connect(process, &InternalCommandProcess::showString, this,
        [&](const QString &name, const QString &str){
        QMessageBox::information(this->parentWidget(), name, str);
    });
    connect(process, &InternalCommandProcess::showIconFont, this,
        [&](QChar code){
        QIcon icon = QFontIcon::icon(code);
        QDialog dialog(this->parentWidget());
        QLabel *label = new QLabel(&dialog);
        label->setPixmap(icon.pixmap(512,512));
        dialog.resize(512,512);
        dialog.exec();
    });
    connect(process, &InternalCommandProcess::showEasterEggs, this, [&](void){
        class MyDialog : public QDialog {
        public:
            MyDialog(QWidget *parent = nullptr) : QDialog(parent) {
                setAttribute(Qt::WA_StyledBackground, true);
                setStyleSheet("QWidget#MyDialog {background-color: transparent;}");
            #if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
                setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint);
            #else
                setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
            #endif
                QPixmap pix;
                pix.load(":/icons/icons/about2.png",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
                resize(pix.size());
                setMask(QBitmap(pix.mask()));
                QVBoxLayout *layout = new QVBoxLayout(this);
                QLabel *label = new QLabel("<html><head/><body><p align=\"center\">I love Tangtang!</p></body></html>", this);
                layout->addWidget(label);
            }
    
        protected:
            void paintEvent(QPaintEvent *event) {
                QString paths = ":/icons/icons/about2.png";
                QPainter painter(this);
                painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));
                event->accept();
            }
            void mouseReleaseEvent(QMouseEvent *event) {
                if( event->button() == Qt::LeftButton) {
                    accept();
                }
            }
        };
        MyDialog show;
        show.setObjectName("MyDialog");
        show.exec();
    });

    term->startTerminalTeletype();
    process->start();
}

InternalCommandWindow::~InternalCommandWindow() {
    delete process;
    delete term;
    delete ui;
}

#ifdef ENABLE_PYTHON
void InternalCommandWindow::setPyRun(PyRun *pyRun) {
    process->setPyRun(pyRun);
}
#endif

void InternalCommandWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = new QMenu(this);
    QList<QAction*> ftActions = term->filterActions(event->pos());
    if(!ftActions.isEmpty()) {
        menu->addActions(ftActions);
        menu->addSeparator();
    }
    QAction *copyAction = new QAction(tr("Copy"),this);
    connect(copyAction,&QAction::triggered,this,[&](){
        term->copyClipboard();
    });
    menu->addAction(copyAction);

    QAction *pasteAction = new QAction(tr("Paste"),this);
    connect(pasteAction,&QAction::triggered,this,[&](){
        term->pasteClipboard();
    });
    menu->addAction(pasteAction);

    QAction *selectAllAction = new QAction(tr("Select All"),this);
    connect(selectAllAction,&QAction::triggered,this,[&](){
        term->selectAll();
    });
    menu->addAction(selectAllAction);
    
    QAction *findAction = new QAction(tr("Find"),this);
    connect(findAction,&QAction::triggered,this,[&](){
        term->toggleShowSearchBar();
    });
    menu->addAction(findAction);
    menu->addSeparator();

    QAction *zoomInAction = new QAction(tr("Zoom In"),this);
    connect(zoomInAction,&QAction::triggered,this,[&](){
        int fontsize = term->zoomIn();
        GlobalSetting settings;
        settings.setValue("Global/InternalCommand/fontsize", fontsize);
    });
    menu->addAction(zoomInAction);

    QAction *zoomOutAction = new QAction(tr("Zoom Out"),this);
    connect(zoomOutAction,&QAction::triggered,this,[&](){
        int fontsize = term->zoomOut();
        GlobalSetting settings;
        settings.setValue("Global/InternalCommand/fontsize", fontsize);
    });
    menu->addAction(zoomOutAction);

    QRect screenGeometry = QGuiApplication::screenAt(cursor().pos())->geometry();
    QPoint pos = cursor().pos() + QPoint(5,5);
    if (pos.x() + menu->width() > screenGeometry.right()) {
        pos.setX(screenGeometry.right() - menu->width());
    }
    if (pos.y() + menu->height() > screenGeometry.bottom()) {
        pos.setY(screenGeometry.bottom() - menu->height());
    }
    menu->popup(pos);
}
