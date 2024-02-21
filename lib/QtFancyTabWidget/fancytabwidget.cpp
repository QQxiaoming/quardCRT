/*
 * Copyright 2022 Sascha Muenzberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fancytabwidget.h"
#include "addtabbutton.h"
#include "fancytabstyle.h"

#include <QStyleOptionTabBarBase>
#include <QStylePainter>
#include <QLineEdit>
#include <QApplication>

#include <QDebug>

namespace
{
    // copy from qt5/qtbase/src/widgets/widgets/qtabbar_p.h
    static void initStyleBaseOption(QStyleOptionTabBarBase *optTabBase, QTabBar *tabbar, QSize size)
    {
        QStyleOptionTab tabOverlap;
        tabOverlap.shape = tabbar->shape();
        int overlap = tabbar->style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, tabbar);
        QWidget *theParent = tabbar->parentWidget();
        optTabBase->initFrom(tabbar);
        optTabBase->shape = tabbar->shape();
        optTabBase->documentMode = tabbar->documentMode();
        if (theParent && overlap > 0) {
            QRect rect;
            switch (tabOverlap.shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect.setRect(0, size.height()-overlap, size.width(), overlap);
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect.setRect(0, 0, size.width(), overlap);
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                rect.setRect(0, 0, overlap, size.height());
                break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                rect.setRect(size.width() - overlap, 0, overlap, size.height());
                break;
            }
            optTabBase->rect = rect;
        }
    }
}

FancyTabWidget::FancyTabWidget(QWidget *parent)
    : QTabWidget{parent}
{
    setStyle( new FancyTabStyle(this) );
#if defined(Q_OS_MACOS)
    setStyleSheet("QTabWidget::right-corner {alignment: left;}");
#endif

    int addButtonSize = tabBar()->height() + 4;

    addButton = new AddTabButton(addButtonSize, addButtonSize, this);
    connect(addButton, &AddTabButton::clicked, this, &FancyTabWidget::addTabButtonClicked);

    connect(tabBar(), &QTabBar::tabBarDoubleClicked, this, &FancyTabWidget::onTabBarDoubleClick);

    if(qApp->isLeftToRight()) {
        setCornerWidget(addButton,Qt::TopRightCorner);
        setCornerWidget(nullptr,Qt::TopLeftCorner);
    } else {
        setCornerWidget(addButton,Qt::TopLeftCorner);
        setCornerWidget(nullptr,Qt::TopRightCorner);
    }

    setFocusPolicy(Qt::StrongFocus);
}

void FancyTabWidget::addTabButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit tabAddRequested();
}

void FancyTabWidget::paintEvent(QPaintEvent *event)
{
    // copy from qt5/qtbase/src/widgets/widgets/qtabwidget.cpp
    if ( documentMode() )
    {
        QStylePainter p(this, tabBar());
        if (QWidget *w = cornerWidget(Qt::TopLeftCorner)) {
            QStyleOptionTabBarBase opt;
            initStyleBaseOption(&opt, tabBar(), w->size());
            opt.rect.moveLeft(w->x() + opt.rect.x());
            opt.rect.moveTop(w->y() + opt.rect.y());
            p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
        }
        if (QWidget *w = cornerWidget(Qt::TopRightCorner)) {
            QStyleOptionTabBarBase opt;
            initStyleBaseOption(&opt, tabBar(), w->size());
            opt.rect.moveLeft(w->x() + opt.rect.x());
            opt.rect.moveTop(w->y() + opt.rect.y() + 3);            // correct y (3)
            p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
        }
        return;
    }

    // we need to adapt painting only in documentMode
    // if documentMode is not active, we call the original method

    QTabWidget::paintEvent(event);
}

void FancyTabWidget::onTabBarDoubleClick(int tabIndex)
{
    if(!m_tabTextEditable) {
        return;
    }

    // only start editing, if we're not already in edit mode
    if ( m_editingTabIndex == -1 )
    {
        startTabNameEditing(tabIndex);
    }
}

void FancyTabWidget::startTabNameEditing(int tabIndex)
{
    m_editingTabIndex = tabIndex;

    QLineEdit *tabLineEdit = new QLineEdit();

    tabLineEdit->setStyleSheet("border: none; background-color: rgba(0,0,0,0);");

    connect(tabLineEdit, &QLineEdit::editingFinished, this, &FancyTabWidget::finishTabNameEditing);
    connect(tabLineEdit, &QLineEdit::textChanged, this, &FancyTabWidget::adjustTabLineEdit);

    tabLineEdit->setText( tabBar()->tabText(tabIndex) );
    tabLineEdit->selectAll();

    tabBar()->setTabText(tabIndex, "");
    tabBar()->setTabButton(tabIndex, QTabBar::LeftSide, tabLineEdit);
    tabLineEdit->setFocus();
}

void FancyTabWidget::finishTabNameEditing()
{
    QLineEdit *tabLineEdit = qobject_cast<QLineEdit*>( sender() );

    // by removing the QLineEdit from the tab bar, it loses focus which triggers editingFinished again
    // we disconnect the slot first so it won't be called again
    disconnect(tabLineEdit, &QLineEdit::editingFinished, this, &FancyTabWidget::finishTabNameEditing);
    disconnect(tabLineEdit, &QLineEdit::textEdited, this, &FancyTabWidget::adjustTabLineEdit);

    tabBar()->setTabButton(m_editingTabIndex, QTabBar::LeftSide, 0);
    tabBar()->setTabText(m_editingTabIndex, tabLineEdit->text() );

    emit tabTextChanged(m_editingTabIndex, tabLineEdit->text() );

    m_editingTabIndex = -1;
    tabLineEdit->deleteLater();
}

void FancyTabWidget::adjustTabLineEdit()
{
    QLineEdit *tabLineEdit = qobject_cast<QLineEdit*>( sender() );

    QFontMetrics fm( tabLineEdit->font() );
    int pixelsWide = fm.horizontalAdvance( tabLineEdit->text() ) + 3;
    tabLineEdit->setFixedWidth(pixelsWide);
    tabLineEdit->setFixedHeight( fm.height() );

    // Reset an empty tab text to force a geometry update of the tab bar
    tabBar()->setTabText(m_editingTabIndex, "");
}

void FancyTabWidget::retranslateUi(void)
{
    if(qApp->isLeftToRight()) {
        setCornerWidget(addButton,Qt::TopRightCorner);
        setCornerWidget(nullptr,Qt::TopLeftCorner);
    } else {
        setCornerWidget(addButton,Qt::TopLeftCorner);
        setCornerWidget(nullptr,Qt::TopRightCorner);
    }
    addButton->retranslateUi();
}
