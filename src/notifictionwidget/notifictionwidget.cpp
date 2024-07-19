#include <QFrame>

#include "notifictionwidget.h"
#include "notificationitem.h"
#include "ui_notifictionwidget.h"

NotifictionWidget::NotifictionWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotifictionWidget)
{
    ui->setupUi(this);
    ui->label->setEnabled(false);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->label->setVisible(true);
    ui->listWidget->setVisible(false);
}

NotifictionWidget::~NotifictionWidget()
{
    delete ui;
}

void NotifictionWidget::addNotifiction(QString msg) {
    QDateTime current = QDateTime::currentDateTime();
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    NotificationItem *notificationItem = new NotificationItem(current, msg, item);
    item->setSizeHint(QSize(0, 50));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, notificationItem);
    connect(notificationItem, &NotificationItem::removeItem, this, [=](QListWidgetItem *item) {
        ui->listWidget->removeItemWidget(item);
        delete item;
        if (ui->listWidget->count() > 0) {
            ui->label->setVisible(false);
            ui->listWidget->setVisible(true);
        } else {
            ui->label->setVisible(true);
            ui->listWidget->setVisible(false);
        }
        emit notifictionChanged(ui->listWidget->count());
    });
    emit notifictionChanged(ui->listWidget->count());
    if (ui->listWidget->count() > 0) {
        ui->label->setVisible(false);
        ui->listWidget->setVisible(true);
    } else {
        ui->label->setVisible(true);
        ui->listWidget->setVisible(false);
    }
}

uint32_t NotifictionWidget::getNotifictionCount(){
    return ui->listWidget->count();
}

bool NotifictionWidget::removeNotifiction(int32_t index){
    if (index < ui->listWidget->count()) {
        QListWidgetItem *item = ui->listWidget->takeItem(index);
        if (item) {
            delete item;
            if (ui->listWidget->count() > 0) {
                ui->label->setVisible(false);
                ui->listWidget->setVisible(true);
            } else {
                ui->label->setVisible(true);
                ui->listWidget->setVisible(false);
            }
            emit notifictionChanged(ui->listWidget->count());
            return true;
        }
    }
    return false;
}

bool NotifictionWidget::isEmpty(void){
    return ui->listWidget->count() == 0;
}

void NotifictionWidget::clearNotifiction(void){
    ui->listWidget->clear();    
    emit notifictionChanged(ui->listWidget->count());
    if (ui->listWidget->count() > 0) {
        ui->label->setVisible(false);
        ui->listWidget->setVisible(true);
    } else {
        ui->label->setVisible(true);
        ui->listWidget->setVisible(false);
    }
}
