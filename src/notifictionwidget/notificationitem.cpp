#include <QDateTime>
#include <QListWidgetItem>

#include "qfonticon.h"
#include "notificationitem.h"
#include "ui_notificationitem.h"

NotificationItem::NotificationItem(QDateTime time, QString msg, QListWidgetItem *item, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotificationItem)
{
    m_item = item;
    ui->setupUi(this);

    ui->labelDataTime->setText(time.toString("yyyy-MM-dd hh:mm:ss"));
    QFont font = ui->labelDataTime->font();
    font.setPointSize(8);
    ui->labelDataTime->setFont(font);

    ui->labelMsg->setText(msg);

    ui->toolButtonClose->setIcon(QFontIcon::icon(QChar(0xf00d)));
    ui->toolButtonClose->setToolButtonStyle(Qt::ToolButtonIconOnly);
    ui->toolButtonClose->setAutoRaise(true);
    connect(ui->toolButtonClose, &QToolButton::clicked, [this](){
        emit removeItem(m_item);
    });
}

NotificationItem::~NotificationItem()
{
    delete ui;
}
