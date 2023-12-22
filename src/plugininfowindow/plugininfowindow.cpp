#include <QDesktopServices>
#include <QUrl>

#include "plugininfowindow.h"
#include "ui_plugininfowindow.h"

PluginInfoWindow::PluginInfoWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PluginInfoWindow)
{
    ui->setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);

    ui->lineEditAPIVersion->setText(QString::number(PLUGIN_API_VERSION));
    ui->lineEditAPIVersion->setReadOnly(true);
    ui->lineEditAPIVersion->setFocusPolicy(Qt::NoFocus);

    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("API Version") << tr("Enable"));
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 80);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setColumnWidth(3, 80);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->pushButtonInstallPlugin, &QPushButton::clicked, this, [=](){
        QString pluginDir = QApplication::applicationDirPath() + "/plugins/QuardCRT";
        QDesktopServices::openUrl(QUrl::fromLocalFile(pluginDir));
    });
}

PluginInfoWindow::~PluginInfoWindow()
{
    delete ui;
}

void PluginInfoWindow::addPluginInfo(PluginInterface *plugin, uint32_t apiVersion, bool enable)
{
    addPluginInfo(plugin->name(), plugin->version(), apiVersion, enable);
}

void PluginInfoWindow::addPluginInfo(QString name, QString version, uint32_t apiVersion, bool enable)
{
    uint32_t i = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(i + 1);
    ui->tableWidget->setItem(i, 0, new QTableWidgetItem(name));
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(version));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(apiVersion?QString::number(apiVersion):""));
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    ui->tableWidget->setItem(i, 3, item);
}

void PluginInfoWindow::retranslateUi(void) {
    ui->retranslateUi(this);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Version") << tr("API Version") << tr("Enable"));
}
