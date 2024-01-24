#include "sftpmenubookmarkwidget.h"
#include "filedialog.h"
#include "ui_sftpmenubookmarkwidget.h"

SFTPmenuBookmarkWidget::SFTPmenuBookmarkWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SFTPmenuBookmarkWidget)
{
    ui->setupUi(this);

    connect(ui->toolButton, &QToolButton::clicked, this, [&](){
        QString dir = FileDialog::getExistingDirectory(this, tr("Open Directory"),
                 ui->lineEditLocalPath->text().isEmpty() ? QDir::homePath() : ui->lineEditLocalPath->text());
        if (!dir.isEmpty())
            ui->lineEditLocalPath->setText(dir);
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [&](){
        emit accepted();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, [&](){
        emit rejected();
    });
}

SFTPmenuBookmarkWidget::~SFTPmenuBookmarkWidget()
{
    delete ui;
}

void SFTPmenuBookmarkWidget::setConfig(const QString &bookmarkName,const QString &localPath, const QString &remotePath) {
    ui->lineEditBookmarkName->setText(bookmarkName);
    ui->lineEditLocalPath->setText(localPath);
    ui->lineEditRemotePath->setText(remotePath);
    bookmarkInitName = bookmarkName;
}

void SFTPmenuBookmarkWidget::getConfig(QString &bookmarkName, QString &localPath, QString &remotePath) {
    bookmarkName = ui->lineEditBookmarkName->text();
    localPath = ui->lineEditLocalPath->text();
    remotePath = ui->lineEditRemotePath->text();
}

QDataStream &operator<<(QDataStream &out, const SFTPmenuBookmarkWidget::Config &config) {
    out << config.bookmarkName << config.localPath << config.remotePath;
    return out;
}

QDataStream &operator>>(QDataStream &in, SFTPmenuBookmarkWidget::Config &config) {
    in >> config.bookmarkName >> config.localPath >> config.remotePath;
    return in;
}
