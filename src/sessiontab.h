#ifndef SESSIONTAB_H
#define SESSIONTAB_H

#include <QWidget>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QLabel>
#include <QVBoxLayout>
#include "fancytabwidget.h"

class EmptyTabWidget : public QWidget {
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

class SessionTab : public FancyTabWidget {
    Q_OBJECT
public:
    explicit SessionTab(QWidget *parent = nullptr);
    ~SessionTab();
    int count(void);
    void setCurrentIndex(int index);
    void retranslateUi(void);

signals:
    void showContextMenu(int index, const QPoint& position);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    EmptyTabWidget *emptyTab;
};

#endif // SESSIONTAB_H
