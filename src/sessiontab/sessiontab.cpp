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
#include <QToolTip>

#include "sessiontab.h"
#include "qtermwidget.h"
#include "CharWidth.h"
#include "sessionswindow.h"

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

SessionTabBarPreviewWidget::SessionTabBarPreviewWidget(QWidget *parent) 
    : QWidget(parent) {
    viewWidth = 200;
    setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip|Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::NonModal);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(1,1,1,1);
    layout->setSpacing(0);
    setLayout(layout);
    viewLabel = new QLabel(this);
    viewLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    viewLabel->setScaledContents(true);
    layout->addWidget(viewLabel);
    tipLabel = new QLabel(this);
    tipLabel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    tipLabel->setWordWrap(true);
    layout->addWidget(tipLabel);
}

SessionTabBarPreviewWidget::~SessionTabBarPreviewWidget() {
}

void SessionTabBarPreviewWidget::updateInfo(const QPixmap &view, const QString &tip) {
    viewLabel->setPixmap(view);
    tipLabel->setText(tip);
    tipLabel->setFixedWidth(view.width()/previewHighDpi);
    setFixedWidth(view.width()/previewHighDpi);
    setFixedHeight(view.height()/previewHighDpi + tipLabel->height());
}

QList<SessionTabBar*> SessionTabBar::tabBarInstances;

SessionTabBar::SessionTabBar(QWidget *parent) 
    : QTabBar(parent) {
    tabBarInstances << this;
    setAttribute(Qt::WA_Hover, true);
    preview = new SessionTabBarPreviewWidget;
    preview->hide();
    preview->window()->lower();
}

SessionTabBar::~SessionTabBar() {
    delete preview;
    tabBarInstances.removeOne(this);
}

bool SessionTabBar::event(QEvent * event) {
    if(event->type() == QEvent::HoverMove) {
        QPoint pos = mapFromGlobal(QCursor::pos());
        int index = tabAt(pos);
        if(index > 0) {
            if(previewEnabled) {
                SessionTab *tab = (SessionTab *)parentWidget();
                QWidget *widget = (QWidget *)tab->widget(index);
                SessionsWindow *sessionsWindow = widget->property("session").value<SessionsWindow *>();
                QPixmap viewPixmap(preview->viewWidthHighDpi(),preview->viewWidthHighDpi());
                sessionsWindow->screenShot(&viewPixmap);
                preview->updateInfo(viewPixmap,tab->tabToolTip(index));
                QPoint viewPos = mapToGlobal(QPoint(tabRect(index).center().x(),
                    tabRect(index).center().y() + tabRect(index).height()/2 + 5) - QPoint(preview->width()/2,0));
                preview->move(viewPos);
                preview->setParent(nullptr);
                if(previewIndex != index) {
                    previewIndex = index;
                    preview->update();
                }
                preview->show();
                preview->window()->lower();
                emit tabPreviewShow(index);
            }
        } else {
            preview->hide();
        }
    } else if(event->type() == QEvent::HoverLeave) {
        preview->hide();
    } else if(event->type() == QEvent::ToolTip) {
        if(previewEnabled) {
            return true;
        }
    }
    return QTabBar::event(event);
}

void SessionTabBar::changeEvent(QEvent* event) {
    if(event->type() == QEvent::ActivationChange) {
        if(!this->isActiveWindow()) {
            preview->hide();
        }
    }
    QTabBar::changeEvent(event);
}

void SessionTabBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int index = tabAt(event->pos());
        if(index > 0) {
            QRect rect = tabRect(index);
            QPixmap pixmap = grab(rect);
            dragLabel = new QLabel;
            dragLabel->setPixmap(pixmap);
            dragTabindex = index;
            SessionTab *tab = (SessionTab *)parentWidget();
            dragTabWidget = tab->widget(dragTabindex);
            dragTabFrom = this;
            dragStartPosition = event->pos();
            grabMouse();
            setCursor(Qt::ClosedHandCursor);            
        }
    }

    QTabBar::mousePressEvent(event);
}

void SessionTabBar::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        bool moved_length = (event->pos() - dragStartPosition).manhattanLength() > qApp->startDragDistance();
        if(moved_length) {
            if (dragLabel) {
                if(!initializing_drag) {
                    dragLabel->setParent(nullptr);
                    dragLabel->setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip|Qt::WindowStaysOnTopHint);
                    dragLabel->setWindowModality(Qt::NonModal);
                    dragLabel->window()->raise();
                    initializing_drag = true;
                }
            }
        }
    }
    if (dragLabel) {
        dragLabel->move(event->globalPosition().toPoint() + QPoint(3, 3));
        dragLabel->show();
    }
    QTabBar::mouseMoveEvent(event);
}

void SessionTabBar::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (dragLabel) {
            unsetCursor();
            dragLabel->hide();
            dragLabel->deleteLater();
            dragLabel = nullptr;
            initializing_drag = false;
            if(dragTabindex != -1) {
                SessionTab *tab = (SessionTab *)parentWidget();
                int realIndex = tab->indexOf(dragTabWidget);
                dragTabindex = realIndex == -1 ? dragTabindex : realIndex;
                bool match = false;
                foreach(SessionTabBar *bar, tabBarInstances) {
                    if(bar != this) {
                        QPoint pos = bar->mapFromGlobal(event->globalPosition().toPoint());
                        int index = bar->tabAt(pos);
                        if(index > 0) {
                            emit dragTabMoved(dragTabindex,index,bar);
                            match = true;
                        } else {
                            SessionTab *tab = (SessionTab *)bar->parentWidget();
                            if(tab->count() == 0) {
                                QPoint pos = tab->mapFromGlobal(event->globalPosition().toPoint());
                                if(pos.x() < tab->width() && pos.y() < tab->height()
                                    && pos.x() > 0 && pos.y() > 0) {
                                    emit dragTabMoved(dragTabindex,-2,bar);
                                    match = true;
                                }
                            } else {
                                if(pos.x() > bar->width() && pos.x() < bar->parentWidget()->width() &&
                                    pos.y() < bar->height() && pos.y() > 0) {
                                    emit dragTabMoved(dragTabindex,-2,bar);
                                    match = true;
                                }
                            }
                        }
                    }
                }
                if(!match) {
                    int index = tabAt(event->pos());
                    index = index == 0 ? -1 : index;
                    if(index == -1) {
                        if(event->pos().x() > width() && event->pos().x() < parentWidget()->width() &&
                            event->pos().y() < height() && event->pos().y() > 0) {
                            index = -2;
                        }
                    }
                    emit dragTabMoved(dragTabindex,index,this);
                }
            }
            dragTabindex = -1;
        }
        releaseMouse();
    }
    QTabBar::mouseReleaseEvent(event);
}

SessionTab::SessionTab(QWidget *parent) 
    : FancyTabWidget(parent) {
    SessionTabBar *sTabBar = new SessionTabBar(this);
    setTabBar(sTabBar);
    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);
    setTabBarAutoHide(true);
    setTabTextEditable(false);
    setAddTabButtonVisible(false);
    setAcceptDrops(true);

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

    connect(sTabBar,&SessionTabBar::tabMoved,this,[&](int from, int to) {
        QString text = tabTexts.at(from);
        int pos = titleScrollPos.at(from);
        tabTexts.replace(from,tabTexts.at(to));
        titleScrollPos.replace(from,titleScrollPos.at(to));
        tabTexts.replace(to,text);
        titleScrollPos.replace(to,pos);
    });

    connect(sTabBar,&SessionTabBar::dragTabMoved,this,[&](int from, int to, SessionTabBar* toBar) {
        if(toBar == tabBar()) {
            emit dragTabMoved(from,to,this);
        } else {
            emit dragTabMoved(from,to,(SessionTab *)toBar->parentWidget());
        }
    });

    connect(sTabBar,&SessionTabBar::tabPreviewShow,this,&SessionTab::tabPreviewShow);
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
    ((SessionTabBar*)tabBar())->setPreviewHide();
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

void SessionTab::refreshTabText(void) {
    for(int i = 1; i < FancyTabWidget::count(); i++) {
        QString title = tabTexts.at(i);
        if(CharWidth::string_unicode_width(title) > titleWidth) {
            titleScrollPos[i] = (titleScrollPos.at(i) + 2) % (CharWidth::string_unicode_width(title) - titleWidth + 2);
            QString txt;
            for(int j = 0; j < title.length()-titleScrollPos[i]; j++) {
                if(CharWidth::string_unicode_width(txt) + CharWidth::string_unicode_width(QString(title.at(titleScrollPos[i]+j))) <= titleWidth) {
                    txt = txt + title.at(titleScrollPos[i]+j);
                } else {
                    break;
                }
            }
            title = txt + QString(titleWidth - CharWidth::string_unicode_width(txt),' ');
        } else {
            titleScrollPos[i] = 0;
            title = title + QString(titleWidth - CharWidth::string_unicode_width(title),' ');
        }
        FancyTabWidget::setTabText(i,title);
    }
}

void SessionTab::setTabStaticText(TitleScrollMode mode, int index, QString title) {
    if(mode == TITLE_FULL) {
        FancyTabWidget::setTabText(index,title);
        return;
    }
    if(CharWidth::string_unicode_width(title) > titleWidth) {
        QString txt;
        for(int j = 0; j < title.length(); j++) {
            if(CharWidth::string_unicode_width(txt) + CharWidth::string_unicode_width(QString(title.at(j))) <= (titleWidth-3)) {
                txt = txt + title.at(j);
            } else {
                break;
            }
        }
        txt = txt + "...";
        title = txt + QString(titleWidth - CharWidth::string_unicode_width(txt),' ');
    } else {
        title = title + QString(titleWidth - CharWidth::string_unicode_width(title),' ');
    }
    FancyTabWidget::setTabText(index,title);
}

void SessionTab::setTabText(int index, const QString &text) {
    QString title = text;
    tabTexts.replace(index,text);
    titleScrollPos.replace(index,0);
    FancyTabWidget::setTabToolTip(index,text);
    emit tabTextSet(index, title);
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
