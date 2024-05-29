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
#ifndef SESSIONTAB_H
#define SESSIONTAB_H

#include <QWidget>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QList>
#include <QTabBar>
#include <QMouseEvent>
#include <QPoint>
#include <QPainter>
#include "fancytabwidget.h"

class EmptyTabWidget : public QWidget 
{
    Q_OBJECT
public:
    explicit EmptyTabWidget(QWidget *parent = nullptr);
    ~EmptyTabWidget();
    void retranslateUi(void);
    
protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel *label;
    QLabel *easterEggs;
};

class SessionTabBarPreviewWidget : public QWidget 
{
    Q_OBJECT
public:
    explicit SessionTabBarPreviewWidget(QWidget *parent = nullptr);
    ~SessionTabBarPreviewWidget();

    QPixmap *getViewPixmap(void);
    void updateInfo(const QPixmap &view, const QString &tip);
    void setViewWidth(int width) { viewWidth = width; }
    int getViewWidth(void) { return viewWidth; }

    int viewWidthHighDpi(void) { return viewWidth*previewHighDpi; }

private:
    QLabel *viewLabel;
    QLabel *tipLabel;
    int viewWidth;
#if defined(Q_OS_WIN)
    const int previewHighDpi = 1; // Windows can't handle high dpi pixmap
#else
    const int previewHighDpi = 4;
#endif
};

class SessionTabBar : public QTabBar 
{
    Q_OBJECT
public:
    explicit SessionTabBar(QWidget *parent = nullptr);
    ~SessionTabBar();

    void setPreviewEnabled(bool enabled) { previewEnabled = enabled; }
    bool getPreviewEnabled(void) { return previewEnabled; }
    void setPreviewWidth(int width) { preview->setViewWidth(width); }
    int getPreviewWidth(void) { return preview->width(); }
    void setPreviewHide(void) { preview->hide(); }
    void setChangeBackground(int index, bool change, QColor color = QColor()) {
        foreach (int i, backgroundColors.keys()) {
            if (i == index) {
                if (change) {
                    backgroundColors[i] = color;
                } else {
                    backgroundColors.remove(i);
                }
                return;
            }
        }
        backgroundColors.insert(index, color);
    }
    void clearAllBackground(void){
        backgroundColors.clear();
    }
    
signals:
    void dragTabMoved(int from, int to, SessionTabBar* toBar);
    void tabPreviewShow(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent * event) override;

private:
    QPoint dragStartPosition;
    QLabel *dragLabel = nullptr;
    bool initializing_drag = false;
    int dragTabindex = -1;
    QWidget *dragTabWidget = nullptr;
    SessionTabBar* dragTabFrom = nullptr;
    SessionTabBarPreviewWidget *preview;
    bool previewEnabled = true;
    bool mousePressDoing = false;
    int previewIndex = -1;
    QMap<int, QColor> backgroundColors;
    static QList<SessionTabBar*> tabBarInstances;
};

class SessionTab : public FancyTabWidget 
{
    Q_OBJECT
public:
    enum TitleScrollMode{
        TITLE_BRIEF = 0,
        TITLE_FULL = 1,
        TITLE_SCROLL = 2
    };
    explicit SessionTab(QWidget *parent = nullptr);
    ~SessionTab();
    int count(void);
    void setCurrentIndex(int index);
    void retranslateUi(void);
    int addTab(QWidget *widget, const QString &text);
    void setTabText(int index, const QString &text);
    void removeTab(int index);
    void setScrollTitleMode(TitleScrollMode mode);
    int getScrollTitleMode(void) { return scrollTitleMode; }
    void setTitleWidth(int width) { titleWidth = width; setScrollTitleMode(scrollTitleMode); }
    int getTitleWidth(void) { return titleWidth; }
    void setTabBarHidden(bool hide) { tabBar()->setHidden(hide); }
    QString tabTitle(int index) { return tabTexts[index]; }
    void setPreviewEnabled(bool enabled) { ((SessionTabBar*)tabBar())->setPreviewEnabled(enabled); }
    bool getPreviewEnabled(void) { return ((SessionTabBar*)tabBar())->getPreviewEnabled(); }
    void setPreviewWidth(int width) { ((SessionTabBar*)tabBar())->setPreviewWidth(width); }
    int getPreviewWidth(void) { return ((SessionTabBar*)tabBar())->getPreviewWidth(); }
    void setTagColor(int index, bool change, QColor color = QColor()) { 
        ((SessionTabBar*)tabBar())->setChangeBackground(index, change, color); 
    }
    void clearAllTagColor(void){
        ((SessionTabBar*)tabBar())->clearAllBackground(); 
    }

signals:
    void showContextMenu(int index, const QPoint& position);
    void tabTextSet(int index, const QString &text);
    void dragTabMoved(int from, int to, SessionTab *toTab);
    void tabPreviewShow(int index);
    void tabMoved(int from, int to);

private:
    void refreshTabText(void);
    void setTabStaticText(TitleScrollMode mode, int index, QString title);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    EmptyTabWidget *emptyTab;
    QStringList tabTexts;
    QList<int> titleScrollPos;
    QTimer *titleScrollTimer;
    TitleScrollMode scrollTitleMode = TITLE_BRIEF;
    int titleWidth = 20;
};

#endif // SESSIONTAB_H
