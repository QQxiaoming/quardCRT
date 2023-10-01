#ifndef SESSIONTREEVIEW_H
#define SESSIONTREEVIEW_H

#include <QStringList>
#include <QTreeView>
#include <QContextMenuEvent>

#include "treemodel.h"

class SessionTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit SessionTreeView(QWidget *parent = 0);
    ~SessionTreeView();

    void addSession(QString str, int type);
    void removeSession(QString str);
    bool checkSession(QString str);
    void retranslateUi();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    TreeModel *mode;
    QModelIndex rootIndex;
};

#endif // SESSIONTREEVIEW_H
