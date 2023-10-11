#include "commandwindow.h"
#include "ui_commandwindow.h"

CommandWindow::CommandWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandWindow)
{
    ui->setupUi(this);

    autoSendTimer = new QTimer(this);
    ui->asciiRadioButton->setChecked(true);

    connect(ui->sendPushButton, &QPushButton::clicked, this, [=]() {
        if(ui->asciiRadioButton->isChecked()) {
            emit sendData(ui->commandPlainEdit->toPlainText().toLatin1());
        } else if(ui->hexRadioButton->isChecked()) {
            QString input = ui->commandPlainEdit->toPlainText();
            QByteArray array = QByteArray::fromHex(input.toLatin1());
            emit sendData(array);
            QString output = array.toHex(' ');
            ui->commandPlainEdit->setPlainText(output);
        }
    });

    connect(ui->autoSendCheckBox, &QCheckBox::stateChanged, this, [=](int state) {
        if(state == Qt::Checked) {
            ui->sendPushButton->setEnabled(false);
            ui->commandPlainEdit->setEnabled(false);
            ui->asciiRadioButton->setEnabled(false);
            ui->hexRadioButton->setEnabled(false);
            ui->autoSendIntervalSpinBox->setEnabled(false);
            autoSendTimer->start(ui->autoSendIntervalSpinBox->value());
        } else {
            ui->sendPushButton->setEnabled(true);
            ui->commandPlainEdit->setEnabled(true);
            ui->asciiRadioButton->setEnabled(true);
            ui->hexRadioButton->setEnabled(true);
            ui->autoSendIntervalSpinBox->setEnabled(true);
            autoSendTimer->stop();
        }
    });

    connect(autoSendTimer, &QTimer::timeout, this, [=]() {
        if(ui->asciiRadioButton->isChecked()) {
            emit sendData(ui->commandPlainEdit->toPlainText().toLatin1());
        } else if(ui->hexRadioButton->isChecked()) {
            QString input = ui->commandPlainEdit->toPlainText();
            QByteArray array = QByteArray::fromHex(input.toLatin1());
            emit sendData(array);
            QString output = array.toHex(' ');
            ui->commandPlainEdit->setPlainText(output);
        }
    });
}

CommandWindow::~CommandWindow()
{
    delete ui;
}

void CommandWindow::setCmd(QString cmd) {
    ui->commandPlainEdit->setPlainText(cmd);
}

void CommandWindow::retranslateUi(void) {
    ui->retranslateUi(this);
}
