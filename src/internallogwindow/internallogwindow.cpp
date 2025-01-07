#include <QDateTime>
#include <QMessageBox>
#include "internallogwindow.h"
#include "ui_internallogwindow.h"
#include "filedialog.h"
#include "qspdlogger.h"

InternalLogWindow::InternalLogWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InternalLogWindow)
{
    ui->setupUi(this);
    QSpdLogger::Instance()->addQTextEditSink(ui->textEdit, 4096, true, true);
    QSpdLogger::Instance()->setQTextEditSinkLogLevel(ui->textEdit, QtDebugMsg);

    connect(ui->comboBox, &QComboBox::currentTextChanged, [&](const QString &str) {
        static const QMap<QString, QtMsgType> level_map = {
            {"QtDebugMsg"   , QtDebugMsg},
            {"QtInfoMsg"    , QtInfoMsg},
            {"QtWarningMsg" , QtWarningMsg},
            {"QtCriticalMsg", QtCriticalMsg},
            {"QtFatalMsg"   , QtFatalMsg},
        };
        QSpdLogger::Instance()->setQTextEditSinkLogLevel(ui->textEdit, level_map[str]);
    });
    connect(ui->pushButtonSave, &QPushButton::released, [&](){
        static QString willsaveDir = QDir::homePath();
        QString willsaveName = willsaveDir + QString("/log-%1.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
        QString fileName = FileDialog::getSaveFileName(this,tr("Save Screenshot"),willsaveName,tr("Text Files (*.txt)"));
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
