#include "pluginviewerwidget.h"
#include "ui_pluginviewerwidget.h"

PluginViewerWidget::PluginViewerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PluginViewerWidget)
{
    ui->setupUi(this);

    m_homeWidget = new PluginViewerHomeWidget(this);
    addPlugin(qobject_cast<QWidget *>(m_homeWidget), "Home");
    ui->stackedWidget->setCurrentWidget(m_homeWidget);
    retranslateUi();

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [this](const QString &pluginName) {
        if (m_plugins.contains(pluginName)) {
            ui->stackedWidget->setCurrentWidget(m_plugins.value(pluginName));
        } else if (ui->comboBox->currentIndex() == 0) {
            ui->stackedWidget->setCurrentWidget(m_homeWidget);
        }
    });
}

PluginViewerWidget::~PluginViewerWidget()
{
    delete ui;
}

void PluginViewerWidget::addPlugin(QWidget *pluginWidget, const QString &pluginName)
{
    m_plugins.insert(pluginName, pluginWidget);
    ui->stackedWidget->addWidget(pluginWidget);
    ui->comboBox->addItem(pluginName);
}

void PluginViewerWidget::retranslateUi(void) {
    ui->comboBox->setItemText(0, tr("Home"));
    m_homeWidget->retranslateUi();
    ui->retranslateUi(this);
}
