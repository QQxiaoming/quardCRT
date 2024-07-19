#ifndef NOTIFICTIONWIDGET_H
#define NOTIFICTIONWIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QMouseEvent>

namespace Ui {
class NotifictionWidget;
}

class NotifictionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NotifictionWidget(QWidget *parent = nullptr);
    ~NotifictionWidget();

    void addNotifiction(QString msg);
    uint32_t getNotifictionCount();
    bool removeNotifiction(int32_t index);
    bool isEmpty(void);
    void clearNotifiction(void);

signals:
    void notifictionChanged(uint32_t count);

private:
    Ui::NotifictionWidget *ui;
};

#endif // NOTIFICTIONWIDGET_H
