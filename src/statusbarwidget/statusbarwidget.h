#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QEvent>

namespace Ui {
class StatusBarWidget;
}

class StatusBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarWidget(QWidget *parent = nullptr);
    ~StatusBarWidget();
    void setCursorPosition(int64_t x, int64_t y);
    void setType(const QString &type);
    void setTransInfo(int64_t tx, int64_t rx);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);

private:
    Ui::StatusBarWidget *ui;
};

#endif // STATUSBARWIDGET_H
