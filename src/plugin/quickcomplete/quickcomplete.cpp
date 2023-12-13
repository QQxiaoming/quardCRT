#include "quickcomplete.h"

#include <QMap>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QLineEdit>
#include <QStringList>
#include <QTranslator>
#include <QApplication>
#include <QTimer>
#include <QDebug>

int QuickComplete::init(QMap<QString, QString> params, QWidget *parent)
{
    m_menu = new QMenu(parent);
    m_menu->setTitle(tr("Quick Complete"));

    addQuickCompleteAction = new QAction(tr("Add Quick Complete"), m_menu);
    m_menu->addAction(addQuickCompleteAction);
    QObject::connect(addQuickCompleteAction,&QAction::triggered,this,[=](){
        bool ok;
        QString inputCmd = QInputDialog::getText(parent,tr("Add Quick Complete"),tr("Input Command"),QLineEdit::Normal,"",&ok);
        if(inputCmd.isEmpty() || !ok)
            return;
        m_quickCompleteList.append(inputCmd);
        quickCompleteListChanged();
    });

    removeQuickCompleteAction = new QAction(tr("Remove Quick Complete"), m_menu);
    m_menu->addAction(removeQuickCompleteAction);
    QObject::connect(removeQuickCompleteAction,&QAction::triggered,this,[=](){
        bool ok;
        QString inputCmd = QInputDialog::getItem(parent,tr("Remove Quick Complete"),tr("Select Command"),m_quickCompleteList,0,false,&ok);
        if(inputCmd.isEmpty() || !ok)
            return;
        m_quickCompleteList.removeOne(inputCmd);
        quickCompleteListChanged();
    });

    clearQuickCompleteAction = new QAction(tr("Clear Quick Complete"), m_menu);
    m_menu->addAction(clearQuickCompleteAction);
    QObject::connect(clearQuickCompleteAction,&QAction::triggered,this,[=](){
        m_quickCompleteList.clear();
        quickCompleteListChanged();
    });
    m_menu->addSeparator();

    QTimer::singleShot(100, this, [=](){
        QVariant quickCompleteListVariant(m_quickCompleteList);
        emit readSettings("settings","quickCompleteList",quickCompleteListVariant);
        m_quickCompleteList = quickCompleteListVariant.toStringList();
        foreach (QString cmd, m_quickCompleteList) {
            QAction *action = new QAction(cmd,m_menu);
            m_menu->addAction(action);
            QObject::connect(action,&QAction::triggered,this,[=](){
                emit sendCommand(cmd);
            });
        }
    });
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
    QVariant quickCompleteListVariant(m_quickCompleteList);
    emit writeSettings("settings","quickCompleteList",quickCompleteListVariant);
}

void QuickComplete::setLanguage(const QLocale &language,QApplication *app) {
    static QTranslator *qtTranslator = nullptr;
    if(qtTranslator == nullptr) {
        qtTranslator = new QTranslator(app);
    } else {
        app->removeTranslator(qtTranslator);
        delete qtTranslator;
        qtTranslator = new QTranslator(app);
    }
    switch(language.language()) {
    case QLocale::Chinese:
        if(qtTranslator->load(":/lang/quickcomplete_zh_CN.qm"))
            app->installTranslator(qtTranslator);
        break;
    default:
    case QLocale::English:
        if(qtTranslator->load(":/lang/quickcomplete_en_US.qm"))
            app->installTranslator(qtTranslator);
        break;
    }
}

void QuickComplete::retranslateUi() {
    m_menu->setTitle(tr("Quick Complete"));
    addQuickCompleteAction->setText(tr("Add Quick Complete"));
    removeQuickCompleteAction->setText(tr("Remove Quick Complete"));
    clearQuickCompleteAction->setText(tr("Clear Quick Complete"));
}
