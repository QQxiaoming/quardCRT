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
#ifndef GLOBALOPTIONSWINDOW_H
#define GLOBALOPTIONSWINDOW_H

#include <QDialog>
#include <QStringListModel>
#include <QColor>
#include <QTreeView>
#include <QList>
#include <QStackedWidget>

#include "globaloptionsgeneralwidget.h"
#include "globaloptionsappearancewidget.h"
#include "globaloptionsterminalwidget.h"
#include "globaloptionslogfilewidget.h"
#include "globaloptionswindowwidget.h"
#include "globaloptionstransferwidget.h"
#include "globaloptionsadvancedwidget.h"
#include "CharacterColor.h"
#include "qcustomfilesystemmodel.h"

namespace Ui {
class GlobalOptionsWindow;
}

class GlobalOptionsModel : public QCustomFileSystemModel
{
    Q_OBJECT
public:
    explicit GlobalOptionsModel(QObject *parent = 0);
    ~GlobalOptionsModel();

    QString separator() const override;
    QStringList pathEntryList(const QString &path) override;
    void pathInfo(QString path, bool &isDir, uint64_t &size, QDateTime &lastModified) override;

    class TreeNode {
    public:
        TreeNode(const QString &name = QString(), QWidget *widget = nullptr, const QList<TreeNode> &children = QList<TreeNode>())
            : name(name), widget(widget), children(children) {}
        TreeNode(const TreeNode &node) {
            name = node.name;
            widget = node.widget;
            children = node.children;
        }
        TreeNode& operator=(const TreeNode &node) {
            if (this != &node) {
                name = node.name;
                widget = node.widget;
                children = node.children;
            }
            return *this;
        }
        QString name;
        QWidget *widget;
        QList<TreeNode> children;
    };
    void setTree(const TreeNode &tree);
    TreeNode rootInfo;
    
    static TreeNode findNode(const QString &name, const TreeNode &fnode);
};

class GlobalOptionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalOptionsWindow(QWidget *parent = nullptr);
    ~GlobalOptionsWindow();

    void retranslateUi();
    void setAvailableColorSchemes(QStringList colorSchemes);
    QString getCurrentColorScheme(void);
    QList<QColor> getColorTable(void);
    QFont getCurrentFont(void);
    bool isCurrentFontBuiltIn(void);
    int getTransparency(void);
    QString getBackgroundImage(void);
    int getBackgroundImageMode(void);
    qreal getBackgroundImageOpacity(void);
    QString getNewTabWorkPath(void);
    uint32_t getScrollbackLines(void);
    bool getEnableTerminalBackgroundAnimation(void);
    uint32_t getCursorShape(void);
    bool getCursorBlink(void);
    int getTabTitleMode(void);
    int getTabTitleWidth(void);
    bool getEnableTabPreview(void);
    int getTabPreviewWidth(void);
    QString getWordCharacters(void);
    int getSelectedTextAccentColorTransparency(void);
    int getTranslateService(void);
    int getNewTabMode(void);
    QString getModemDownloadPath(void);
    QString getModemUploadPath(void);
    bool getDisableZmodemOnline(void);
    bool getXYModem1K(void);
    int getPreeditColorIndex(void);
    bool getConfirmMultilinePaste(void);
    bool getTrimPastedTrailingNewlines(void);
    bool getEcho(void);
    QColor getCursorColor(void);
    QString getLogOnEachLine(void);
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    bool getEnableCtrlC(void);
#endif
#if defined(Q_OS_WIN)
    QString getPowerShellProfile(void);
#endif
    bool updateColorButtons(const QString &text);
    void switchTheme(void);

    const static QString defaultColorScheme;
    const static QString defaultColorSchemeBak;

private:
    void setActiveWidget(QWidget *widget);
    void expandAllItems();
    void expandRecursively(const QModelIndex &index);

signals:
    void colorSchemeChanged(QString colorScheme);
    void transparencyChanged(int transparency);
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    void enableCtrlCChanged(bool enable);
#endif

private slots:
    void fontSizeChanged(int size);
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::GlobalOptionsWindow *ui;
    QWidget *emptyWidget;
    GlobalOptionsGeneralWidget *globalOptionsGeneralWidget;
    GlobalOptionsAppearanceWidget *globalOptionsAppearanceWidget;
    GlobalOptionsTerminalWidget *globalOptionsTerminalWidget;
    GlobalOptionsLogFileWidget *globalOptionsLogFileWidget;
    GlobalOptionsWindowWidget *globalOptionsWindowWidget;
    GlobalOptionsTransferWidget *globalOptionsTransferWidget;
    GlobalOptionsAdvancedWidget *globalOptionsAdvancedWidget;
    GlobalOptionsModel *model;
    QFont font;
    QString cursorColorStr;
    ColorEntry table[TABLE_COLORS];
    GlobalOptionsModel::TreeNode rootInfo;
};

#endif //GLOBALOPTIONSWINDOW_H
