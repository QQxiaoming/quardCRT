#include "quickcomplete.h"

#include <QMap>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QLineEdit>
#include <QStringList>
#include <QDebug>

int QuickComplete::init(QMap<QString, QString> params, QWidget *parent)
{
    m_menu = new QMenu(parent);
    m_menu->setTitle("Quick Complete");

    addQuickCompleteAction = new QAction("Add Quick Complete", m_menu);
    m_menu->addAction(addQuickCompleteAction);
    QObject::connect(addQuickCompleteAction,&QAction::triggered,this,[=](){
        bool ok;
        QString inputCmd = QInputDialog::getText(parent,"Add Quick Complete","Input Command",QLineEdit::Normal,"",&ok);
        if(inputCmd.isEmpty() || !ok)
            return;
        m_quickCompleteList.append(inputCmd);
        quickCompleteListChanged();
    });

    removeQuickCompleteAction = new QAction("Remove Quick Complete", m_menu);
    m_menu->addAction(removeQuickCompleteAction);
    QObject::connect(removeQuickCompleteAction,&QAction::triggered,this,[=](){
        bool ok;
        QString inputCmd = QInputDialog::getItem(parent,"Remove Quick Complete","Select Command",m_quickCompleteList,0,false,&ok);
        if(inputCmd.isEmpty() || !ok)
            return;
        m_quickCompleteList.removeOne(inputCmd);
        quickCompleteListChanged();
    });

    clearQuickCompleteAction = new QAction("Clear Quick Complete", m_menu);
    m_menu->addAction(clearQuickCompleteAction);
    QObject::connect(clearQuickCompleteAction,&QAction::triggered,this,[=](){
        m_quickCompleteList.clear();
        quickCompleteListChanged();
    });
    m_menu->addSeparator();

    return 0;
}

void QuickComplete::quickCompleteListChanged(void) {
    foreach (QAction *action, m_menu->actions()) {
        if(action == addQuickCompleteAction || action == removeQuickCompleteAction || action == clearQuickCompleteAction)
            continue;
        m_menu->removeAction(action);
    }
    foreach (QString cmd, m_quickCompleteList) {
        QAction *action = new QAction(cmd,m_menu);
        m_menu->addAction(action);
        QObject::connect(action,&QAction::triggered,this,[=](){
            emit sendCommand(cmd);
        });
    }
}
