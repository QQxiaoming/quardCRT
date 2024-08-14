#ifndef CREATEPUBLICKEY_H
#define CREATEPUBLICKEY_H

#include <QDialog>

namespace Ui {
class CreatePublicKey;
}

class CreatePublicKey : public QDialog
{
    Q_OBJECT

public:
    explicit CreatePublicKey(QWidget *parent = nullptr);
    ~CreatePublicKey();

private:
    Ui::CreatePublicKey *ui;
};

#endif // CREATEPUBLICKEY_H
