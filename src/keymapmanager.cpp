#include "keymapmanager.h"
#include "ui_keymapmanager.h"

keyMapManager::keyMapManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyMapManager)
{
    ui->setupUi(this);
    
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

keyMapManager::~keyMapManager()
{
    delete ui;
}

void keyMapManager::setAvailableKeyBindings(QStringList keyBindings)
{
    ui->comboBoxKeyBinding->clear();
    ui->comboBoxKeyBinding->addItems(keyBindings);
    ui->comboBoxKeyBinding->setCurrentText("linux");
}

QString keyMapManager::getCurrentKeyBinding(void)
{
    return ui->comboBoxKeyBinding->currentText();
}

void keyMapManager::buttonBoxAccepted(void)
{
    emit keyBindingChanged(ui->comboBoxKeyBinding->currentText());
    emit this->accepted();
}

void keyMapManager::buttonBoxRejected(void)
{
    emit this->rejected();
}

void keyMapManager::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}
