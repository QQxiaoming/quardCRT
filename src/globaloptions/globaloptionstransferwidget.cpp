#include <QButtonGroup>
#include <QDir>
#include "filedialog.h"
#include "globaloptionstransferwidget.h"
#include "ui_globaloptionstransferwidget.h"

GlobalOptionsTransferWidget::GlobalOptionsTransferWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GlobalOptionsTransferWidget)
{
    ui->setupUi(this);

    //set radioButton128Bytes and radioButton1KBytes to be exclusive
    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(ui->radioButton128Bytes);
    group->addButton(ui->radioButton1KBytes);
    ui->radioButton128Bytes->setChecked(true);

    connect(ui->toolButtonUpload, &QToolButton::clicked, [this](){
        QString currentDir = ui->lineEditUpload->text();
        if (currentDir.isEmpty()) {
            currentDir = QDir::homePath();
        }
        QString dir = FileDialog::getExistingDirectory(this, tr("Select Directory"), currentDir);
        if (!dir.isEmpty()) {
            ui->lineEditUpload->setText(dir);
        }
    });
    connect(ui->toolButtonDownload, &QToolButton::clicked, [this](){
        QString currentDir = ui->lineEditDownload->text();
        if (currentDir.isEmpty()) {
            currentDir = QDir::homePath();
        }
        QString dir = FileDialog::getExistingDirectory(this, tr("Select Directory"), currentDir);
        if (!dir.isEmpty()) {
            ui->lineEditDownload->setText(dir);
        }
    });
}

GlobalOptionsTransferWidget::~GlobalOptionsTransferWidget()
{
    delete ui;
}
