#ifndef GLOBALOPTIONSWINDOW_H
#define GLOBALOPTIONSWINDOW_H

#include <QDialog>
#include <QStringListModel>
#include "globaloptionsgeneralwidget.h"
#include "globaloptionsappearancewidget.h"
#include "globaloptionsterminalwidget.h"
#include "globaloptionswindowwidget.h"
#include "globaloptionsadvancedwidget.h"

namespace Ui {
class GlobalOptionsWindow;
}

class GlobalOptionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalOptionsWindow(QWidget *parent = nullptr);
    ~GlobalOptionsWindow();

    void retranslateUi();
    void setAvailableColorSchemes(QStringList colorSchemes);
    QString getCurrentColorScheme(void);
    QFont getCurrentFont(void);
    int getTransparency(void);
    QString getBackgroundImage(void);
    int getBackgroundImageMode(void);
    qreal getBackgroundImageOpacity(void);
    QString getNewTabWorkPath(void);
    uint32_t getScrollbackLines(void);
    bool getEnableTerminalBackgroundAnimation(void);
    uint32_t getCursorShape(void);
    bool getCursorBlink(void);

    const static QString defaultColorScheme;

private:
    void setActiveWidget(int index);

signals:
    void colorSchemeChanged(QString colorScheme);
    void transparencyChanged(int transparency);

private slots:
    void fontSizeChanged(int size);
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::GlobalOptionsWindow *ui;
    GlobalOptionsGeneralWidget *globalOptionsGeneralWidget;
    GlobalOptionsAppearanceWidget *globalOptionsAppearanceWidget;
    GlobalOptionsTerminalWidget *globalOptionsTerminalWidget;
    GlobalOptionsWindowWidget *globalOptionsWindowWidget;
    GlobalOptionsAdvancedWidget *globalOptionsAdvancedWidget;
    QStringListModel *model;
    QFont font;
};

#endif //GLOBALOPTIONSWINDOW_H
