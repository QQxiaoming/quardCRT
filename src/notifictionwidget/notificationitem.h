#ifndef NOTIFICATIONITEM_H
#define NOTIFICATIONITEM_H

#include <QWidget>
#include <QDateTime>
#include <QListWidgetItem>

namespace Ui {
class NotificationItem;
}

class NotificationItem : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationItem(QDateTime time, QString msg, QListWidgetItem *m_item,  QWidget *parent = nullptr);
    ~NotificationItem();

signals:
    void removeItem(QListWidgetItem *item);

private:
    Ui::NotificationItem *ui;
    QListWidgetItem *m_item;
};

#endif // NOTIFICATIONITEM_H
