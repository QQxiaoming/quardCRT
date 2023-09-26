#ifndef GLOBALOPTIONS_H
#define GLOBALOPTIONS_H

#include <QDialog>

namespace Ui {
class GlobalOptions;
}

class GlobalOptions : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalOptions(QWidget *parent = nullptr);
    ~GlobalOptions();
    void setAvailableColorSchemes(QStringList colorSchemes);
    QString getCurrentColorScheme(void);
    QFont getCurrentFont(void);

signals:
    void colorSchemeChanged(QString colorScheme);

private slots:
    void fontSizeChanged(int size);
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::GlobalOptions *ui;
    QFont font;
};

#endif // GLOBALOPTIONS_H
