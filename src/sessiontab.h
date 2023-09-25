#ifndef SESSIONTAB_H
#define SESSIONTAB_H

#include <QTabWidget>

class SessionTab : public QTabWidget {
    Q_OBJECT
public:
    explicit SessionTab(QWidget *parent = nullptr);
    ~SessionTab();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // SESSIONTAB_H
