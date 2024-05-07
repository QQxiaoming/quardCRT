#include "statusbarwidget.h"
#include "ui_statusbarwidget.h"

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StatusBarWidget) {
    ui->setupUi(this);
    setMouseTracking(true);

    ui->statusBarCursorInfo->setVisible(false);
    ui->statusBarType->setVisible(false);
    ui->statusBarTrans->setVisible(false);
}

StatusBarWidget::~StatusBarWidget() {
    delete ui;
}

void StatusBarWidget::setCursorPosition(int64_t x, int64_t y) {
    if(x < 0 || y < 0) {
        ui->statusBarCursorInfo->setText("Ln /, Col /");
        ui->statusBarCursorInfo->setVisible(false);
        return;
    }
    ui->statusBarCursorInfo->setText(QString("Ln %1, Col %2").arg(x).arg(y));
    ui->statusBarCursorInfo->setVisible(true);
}

void StatusBarWidget::setType(const QString &type) {
    if(type.isEmpty()) {
        ui->statusBarType->setText("Null");
        ui->statusBarType->setVisible(false);
        return;
    }
    ui->statusBarType->setText(type);
    ui->statusBarType->setVisible(true);
}

void StatusBarWidget::setTransInfo(int64_t tx, int64_t rx) {
    if(tx < 0 || rx < 0) {
        ui->statusBarTrans->setText("Tx /, Rx /");
        ui->statusBarTrans->setVisible(false);
        return;
    }
    auto getSize = [](int64_t size) -> QString {
        if( size <= 1024) {
            return QString("%1 B").arg(size);
        } else if ( size <= 1024 * 1024 ) {
            return QString::number(size / 1024.0, 'f', 2) + QString(" KB");
        } else if ( size <= 1024 * 1024 * 1024 ) {
            return QString::number(size / (1024.0 * 1024.0), 'f', 2) + QString(" MB");
        } else {
            return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + QString(" GB");
        }
    };
    ui->statusBarTrans->setText(QString("Tx %1, Rx %2").arg(getSize(tx)).arg(getSize(rx)));
    ui->statusBarTrans->setVisible(true);
}

void StatusBarWidget::mouseMoveEvent(QMouseEvent *event) {
    if(ui->statusBarCursorInfo->geometry().contains(event->pos())) {
        ui->statusBarCursorInfo->setEnabled(true);
        ui->statusBarType->setEnabled(false);
        ui->statusBarTrans->setEnabled(false);
    } else if(ui->statusBarType->geometry().contains(event->pos())) {
        ui->statusBarCursorInfo->setEnabled(false);
        ui->statusBarType->setEnabled(true);
        ui->statusBarTrans->setEnabled(false);
    } else if(ui->statusBarTrans->geometry().contains(event->pos())) {
        ui->statusBarCursorInfo->setEnabled(false);
        ui->statusBarType->setEnabled(false);
        ui->statusBarTrans->setEnabled(true);
    } else {
        ui->statusBarCursorInfo->setEnabled(false);
        ui->statusBarType->setEnabled(false);
        ui->statusBarTrans->setEnabled(false);
    }
    QWidget::mouseMoveEvent(event);
}

void StatusBarWidget::leaveEvent(QEvent *event) {
    ui->statusBarCursorInfo->setEnabled(false);
    ui->statusBarType->setEnabled(false);
    ui->statusBarTrans->setEnabled(false);
    QWidget::leaveEvent(event);
}
