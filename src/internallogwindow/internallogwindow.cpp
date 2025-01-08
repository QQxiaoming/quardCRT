#include <QDateTime>
#include <QMessageBox>
#include <QFontDatabase>

#include "internallogwindow.h"
#include "ui_internallogwindow.h"
#include "filedialog.h"
#include "qspdlogger.h"

InternalLogWindow::InternalLogWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InternalLogWindow)
{
    ui->setupUi(this);

    QFont font = QApplication::font();
#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
    int fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() + "/inziu-iosevkaCC-SC-regular.ttf");
#else
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/font/font/inziu-iosevkaCC-SC-regular.ttf"));
#endif
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.size() > 0) {
        font.setFamily(fontFamilies[0]);
    }
    font.setFixedPitch(true);
    font.setPointSize(12);
    ui->textEdit->setFont(font);

    QSPDLogger->addQTextEditSink(ui->textEdit, 4096, true, true);
    QSPDLogger->setQTextEditSinkLogLevel(ui->textEdit, QtDebugMsg);

    connect(ui->comboBox, &QComboBox::currentTextChanged, [&](const QString &str) {
        static const QMap<QString, QtMsgType> level_map = {
            {"QtDebugMsg"   , QtDebugMsg},
            {"QtInfoMsg"    , QtInfoMsg},
            {"QtWarningMsg" , QtWarningMsg},
            {"QtCriticalMsg", QtCriticalMsg},
            {"QtFatalMsg"   , QtFatalMsg},
        };
        QSPDLogger->setQTextEditSinkLogLevel(ui->textEdit, level_map[str]);
    });
    connect(ui->pushButtonSave, &QPushButton::released, [&](){
        static QString willsaveDir = QDir::homePath();
        QString willsaveName = willsaveDir + QString("/log-%1.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
        QString fileName = FileDialog::getSaveFileName(this,tr("Save Log"),willsaveName,tr("Text Files (*.txt)"));
        if(!fileName.isEmpty()) {
            willsaveDir = QFileInfo(fileName).absolutePath();
            QFile file(fileName);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << ui->textEdit->toPlainText();
                file.close();
                QMessageBox::information(this,tr("Save Log"),tr("Save Log File Success!"));
            } else {
                QMessageBox::warning(this,tr("Save Log"),tr("Save Log File Failed!"));
            }
        }
    });
    connect(ui->pushButtonClear, &QPushButton::released, [&](){
        ui->textEdit->clear();
    });
    connect(ui->pushButtonOk, &QPushButton::released, [&](){
        hide();
    });
}

InternalLogWindow::~InternalLogWindow()
{
    delete ui;
}
