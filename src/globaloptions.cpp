#include <QFont>
#include <QFontDatabase>
#include "globaloptions.h"
#include "ui_globaloptions.h"

GlobalOptions::GlobalOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalOptions)
{
    ui->setupUi(this);
    
    font = QApplication::font();
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    font.setPointSize(12);
    ui->spinBoxFontSize->setValue(font.pointSize());

    ui->comBoxColorSchemes->setEditable(true);
    ui->comBoxColorSchemes->setInsertPolicy(QComboBox::NoInsert);
  
    connect(ui->spinBoxFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

GlobalOptions::~GlobalOptions()
{
    delete ui;
}

void GlobalOptions::setAvailableColorSchemes(QStringList colorSchemes)
{
    colorSchemes.sort();
    ui->comBoxColorSchemes->clear();
    ui->comBoxColorSchemes->addItems(colorSchemes);
#if defined(Q_OS_WIN)
    ui->comBoxColorSchemes->setCurrentText("PowerShell");
#else
    ui->comBoxColorSchemes->setCurrentText("WhiteOnBlack");
#endif
}

QString GlobalOptions::getCurrentColorScheme(void)
{
    return ui->comBoxColorSchemes->currentText();
}

QFont GlobalOptions::getCurrentFont(void)
{
    return font;
}

void GlobalOptions::fontSizeChanged(int size)
{
    font.setPointSize(size);
}

void GlobalOptions::buttonBoxAccepted(void)
{
    emit colorSchemeChanged(ui->comBoxColorSchemes->currentText());
    emit this->accepted();
}

void GlobalOptions::buttonBoxRejected(void)
{
    emit this->rejected();
}

void GlobalOptions::showEvent(QShowEvent *event)
{
    ui->retranslateUi(this);
    QDialog::showEvent(event);
}
