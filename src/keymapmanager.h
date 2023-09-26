#ifndef KEYMAPMANAGER_H
#define KEYMAPMANAGER_H

#include <QDialog>

namespace Ui {
class keyMapManager;
}

class keyMapManager : public QDialog
{
    Q_OBJECT

public:
    explicit keyMapManager(QWidget *parent = nullptr);
    ~keyMapManager();
    void setAvailableKeyBindings(QStringList keyBindings);
    QString getCurrentKeyBinding(void);

signals:
    void keyBindingChanged(QString keyBinding);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::keyMapManager *ui;
    QStringList availableKeyBindings;
};

#endif // KEYMAPMANAGER_H
