#ifndef SESSIONTAB_H
#define SESSIONTAB_H

#include <QTabWidget>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QLabel>

class SessionTab : public QTabWidget {
    Q_OBJECT
public:
    explicit SessionTab(QWidget *parent = nullptr);
    ~SessionTab();
    int count(void);
    void setCurrentIndex(int index);
    void retranslateUi(void);

signals:
    void showContextMenu(int index);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QLabel *label;
};

#endif // SESSIONTAB_H
