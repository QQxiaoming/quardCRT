
#include "helloworld.h"

#include <QMap>
#include <QMessageBox>
#include <QDebug>

int HelloWorld::init(QMap<QString, QString> params, QWidget *parent)
{
    foreach (QString key, params.keys()) {
        qDebug() << key << " : " << params[key];
    }
    qDebug() << "Hello World init";
    m_action = new QAction("Hello World", parent);
    connect(m_action, &QAction::triggered, [parent](){
        QMessageBox::information(parent, "Hello World", "Hello World");
    });

    return 0;
}
