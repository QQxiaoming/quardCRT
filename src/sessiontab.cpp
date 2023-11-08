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
#include <QMenu>
#include <QTabBar>
#include <QContextMenuEvent>
#include <QPainter>
#include <QPen>
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QTimer>
#include "sessiontab.h"
#include "utf8proc.h"

EmptyTabWidget::EmptyTabWidget(QWidget *parent) 
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setEnabled(false);
    setLayout(new QVBoxLayout);
    layout()->addWidget(label);

    easterEggs = new QLabel(this);
    easterEggs->setAlignment(Qt::AlignCenter);
    layout()->addWidget(easterEggs);
    easterEggs->setVisible(false);
    easterEggs->setEnabled(false);
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
    font.setPointSize(12);
    easterEggs->setFont(font);
}

EmptyTabWidget::~EmptyTabWidget() {
}

void EmptyTabWidget::retranslateUi(void) {
    label->setText(tr("No session"));
}

void EmptyTabWidget::mousePressEvent(QMouseEvent *event) {
    static int count = 0;
    static int hitCount = 0;
    if(event->button() == Qt::LeftButton) {
        count++;
        if(count == 7) {
            label->setVisible(false);
            easterEggs->setVisible(true);
            hitCount++;
            switch(hitCount) {
                case 1:
                    easterEggs->setText(
"+--------------------------------------------------+\n"
"|    ___                      _  ____ ____ _____   |\n"
"|   / _ \\ _   _  __ _ _ __ __| |/ ___|  _ \\_   _|  |\n"
"|  | | | | | | |/ _` | '__/ _` | |   | |_) || |    |\n"
"|  | |_| | |_| | (_| | | | (_| | |___|  _ < | |    |\n"
"|   \\__\\_\\\\__,_|\\__,_|_|  \\__,_|\\____|_| \\_\\|_|    |\n"
"|       Please enjoy coding! Quard(乔). 2023.10.24 |\n"
"+--------------------------------------------------+\n"
);
                    hitCount = 0;
                    break;                    
            }
            count = 0;
        } else {
            label->setVisible(true);
            easterEggs->setVisible(false);
        }
    }
}


SessionTab::SessionTab(QWidget *parent) 
    : FancyTabWidget(parent) {

    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);
    setTabBarAutoHide(true);
    setTabTextEditable(false);
    setAddTabButtonVisible(false);

    titleScrollTimer = new QTimer(this);
    titleScrollTimer->setInterval(750);
    titleScrollTimer->setSingleShot(false);
    connect(titleScrollTimer,&QTimer::timeout,this,[&]() {
        refreshTabText();
    });

    emptyTab = new EmptyTabWidget(this);
    retranslateUi();
    addTab(emptyTab,"");
    tabBar()->setTabVisible(0,false);
    tabBar()->setTabEnabled(0,false);

    connect(this,&SessionTab::currentChanged,this,[&](int index) {
        setAddTabButtonVisible(count() != 0);
        QWidget *w = widget(index);
        if(w) w->setFocus();
    });

    connect(tabBar(),&QTabBar::tabMoved,this,[&](int from, int to) {
        QString text = tabTexts.at(from);
        int pos = titleScrollPos.at(from);
        tabTexts.replace(from,tabTexts.at(to));
        titleScrollPos.replace(from,titleScrollPos.at(to));
        tabTexts.replace(to,text);
        titleScrollPos.replace(to,pos);
    });
}

SessionTab::~SessionTab() {
}

int SessionTab::count(void) {
    int count = FancyTabWidget::count() - 1;
    return count < 0 ? 0 : count;
} 

void SessionTab::setCurrentIndex(int index) {
    FancyTabWidget::setCurrentIndex(index+1);
}

void SessionTab::contextMenuEvent(QContextMenuEvent *event) {
    int index = tabBar()->tabAt(event->pos());
    index = index == 0 ? -1 : index;
    if(index == -1) {
        if(event->pos().x() > tabBar()->width() &&
            event->pos().y() < tabBar()->height()) {
            index = -2;
        }
    }
    emit showContextMenu(index,event->globalPos());
    Q_UNUSED(event);
}

void SessionTab::retranslateUi(void) {
    emptyTab->retranslateUi();
}

int SessionTab::addTab(QWidget *widget, const QString &text) {
    int index = FancyTabWidget::addTab(widget,QString(20,' '));
    tabTexts.insert(index,text);
    titleScrollPos.insert(index,0);
    setTabText(index,text);
    return index;
}

void SessionTab::removeTab(int index) {
    FancyTabWidget::removeTab(index);
    tabTexts.removeAt(index);
    titleScrollPos.removeAt(index);
}

int SessionTab::stringWidth( const QString &string)
{
    auto wcharwidth = [](wchar_t ucs) {
        utf8proc_category_t cat = utf8proc_category( ucs );
        if (cat == UTF8PROC_CATEGORY_CO) {
            // Co: Private use area. libutf8proc makes them zero width, while tmux
            // assumes them to be width 1, and glibc's default width is also 1
            return 1;
        }
        return utf8proc_charwidth( ucs );
    };
    std::wstring wstr = string.toStdWString();
    int w = 0;
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += wcharwidth( wstr[ i ] );
    }
    return w;
}

void SessionTab::refreshTabText(void) {
    for(int i = 1; i < FancyTabWidget::count(); i++) {
        QString title = tabTexts.at(i);
        if(stringWidth(title) > titleWidth) {
            titleScrollPos[i] = (titleScrollPos.at(i) + 2) % (stringWidth(title) - titleWidth + 2);
            QString txt;
            for(int j = 0; j < title.length()-titleScrollPos[i]; j++) {
                if(stringWidth(txt) + stringWidth(QString(title.at(titleScrollPos[i]+j))) <= titleWidth) {
                    txt = txt + title.at(titleScrollPos[i]+j);
                } else {
                    break;
                }
            }
            title = txt + QString(titleWidth - stringWidth(txt),' ');
        } else {
            titleScrollPos[i] = 0;
            title = title + QString(titleWidth - stringWidth(title),' ');
        }
        FancyTabWidget::setTabText(i,title);
    }
}

void SessionTab::setTabStaticText(TitleScrollMode mode, int index, QString title) {
    if(mode == TITLE_FULL) {
        FancyTabWidget::setTabText(index,title);
        return;
    }
    if(stringWidth(title) > titleWidth) {
        QString txt;
        for(int j = 0; j < title.length(); j++) {
            if(stringWidth(txt) + stringWidth(QString(title.at(j))) <= (titleWidth-3)) {
                txt = txt + title.at(j);
            } else {
                break;
            }
        }
        txt = txt + "...";
        title = txt + QString(titleWidth - stringWidth(txt),' ');
    } else {
        title = title + QString(titleWidth - stringWidth(title),' ');
    }
    FancyTabWidget::setTabText(index,title);
}

void SessionTab::setTabText(int index, const QString &text) {
    QString title = text;
    tabTexts.replace(index,text);
    titleScrollPos.replace(index,0);
    FancyTabWidget::setTabToolTip(index,text);
    if(titleScrollTimer->isActive()) {
        refreshTabText();
    } else {
        setTabStaticText(scrollTitleMode,index,text);
    }
}

void SessionTab::setScrollTitleMode(TitleScrollMode mode) {
    scrollTitleMode = mode;
    switch(mode) {
    case TITLE_BRIEF:
    case TITLE_FULL:
        titleScrollTimer->stop();
        for(int i = 1; i < FancyTabWidget::count(); i++) {
            setTabStaticText(mode,i,tabTexts.at(i));
        }
        break;
    case TITLE_SCROLL:
        titleScrollTimer->start();
        break;
    }
}
