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

#ifndef FANCYTABWIDGET_H
#define FANCYTABWIDGET_H

#include <QTabWidget>
#include "addtabbutton.h"

class FancyTabWidget : public QTabWidget
{
    Q_OBJECT
    public:

        explicit    FancyTabWidget(QWidget *parent = nullptr);

        void       setTabTextEditable(bool editable) {
            m_tabTextEditable = editable;
        }
        void       setAddTabButtonVisible(bool visible) {
            addButton->setVisible(visible);
        }

    signals:

        void        tabAddRequested();
        void        tabTextChanged(int index, const QString &text);

    protected slots:

        void        addTabButtonClicked(bool checked);
        void        paintEvent(QPaintEvent *event) override;
        void        onTabBarDoubleClick(int tabIndex);

        void        startTabNameEditing(int tabIndex);
        void        finishTabNameEditing();
        void        adjustTabLineEdit();

    protected:

        int         m_editingTabIndex = { -1 };

    private:

        bool       m_tabTextEditable = false;
        AddTabButton *addButton;
};

#endif // FANCYTABWIDGET_H
