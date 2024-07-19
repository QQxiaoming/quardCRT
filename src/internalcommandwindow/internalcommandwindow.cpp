#include <QMessageBox>
#include <QFontDatabase>
#include <QLabel>
#include <QPainter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QMenu>
#include <QAction>
#include <QList>
#include "globaloptionswindow.h"
#include "internalcommandwindow.h"
#include "ui_internalcommandwindow.h"

InternalCommandWindow::InternalCommandWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InternalCommandWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    process = new InternalCommandProcess(this);
    term = new QTermWidget(this,this);
    term->setScrollBarPosition(QTermWidget::ScrollBarRight);
    term->setBlinkingCursor(true);
    term->setMargin(0);
    term->setDrawLineChars(false);
    term->setSelectionOpacity(0.5);
    term->setEnableHandleCtrlC(true);

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
    term->setTerminalFont(font);
    
    QStringList availableColorSchemes = term->availableColorSchemes();
    availableColorSchemes.sort();
    QString currentColorScheme = availableColorSchemes.first();
    foreach(QString colorScheme, availableColorSchemes) {
        if(colorScheme == GlobalOptionsWindow::defaultColorScheme) {
            term->setColorScheme(GlobalOptionsWindow::defaultColorScheme);
            currentColorScheme = GlobalOptionsWindow::defaultColorScheme;
        }
    }

    QStringList availableKeyBindings = term->availableKeyBindings();
    availableKeyBindings.sort();
    QString currentAvailableKeyBindings = availableKeyBindings.first();
    foreach(QString keyBinding, availableKeyBindings) {
        if(keyBinding == "linux") {
            term->setKeyBindings("linux");
            currentAvailableKeyBindings = "linux";
        }
    }

    ui->verticalLayout->addWidget(term);

    connect(term, &QTermWidget::sendData,this,
        [&](const char *data, int size){
        QByteArray sendData(data, size);
        process->recvData(sendData);
    });
    connect(process, &InternalCommandProcess::sendData, this,
        [&](const QByteArray &data){
        term->recvData(data.data(), data.size());
    });

    bool supportSelection = QApplication::clipboard()->supportsSelection();
    if(!supportSelection) {
        connect(term, &QTermWidget::mousePressEventForwarded, this, [&](QMouseEvent *event) {
            if(event->button() == Qt::MiddleButton) {
                term->copyClipboard();
                term->pasteClipboard();
            }
        });
    }
    connect(term, &QTermWidget::urlActivated, this, [&](const QUrl& url, uint32_t opcode){
        QUrl u = url;
        QString path = u.toString();
        if(path.startsWith("relative:") ) {
            return;
        } 
        QString target = u.toString();
        if(target.startsWith("file://")) {
            target.remove("file://");
            // check endsWith '$' '#'
            if(target.endsWith('$') || target.endsWith('#')) {
                // check file exists
                // FIXME: That's not a good way to fix filePath match issue
                if(!QFile::exists(target)) {
                    target.chop(1);
                    u = QUrl::fromLocalFile(target);
                }
            }
        }
        switch(opcode) {
            case QTermWidget::OpenFromContextMenu:
            case QTermWidget::OpenFromClick: {
                bool ret = QDesktopServices::openUrl(u);
                if(!ret) {
                    QMessageBox::warning(this, tr("Open URL"), tr("Cannot open URL %1.").arg(u.toString()));
                }
                break;
            }
            case QTermWidget::OpenContainingFromContextMenu: {
                QFileInfo fileInfo(u.toLocalFile());
                QDir dir = fileInfo.dir();
                if(dir.exists()) {
                    bool ret = QDesktopServices::openUrl(QUrl::fromLocalFile(dir.path()));
                    if(!ret) {
                        QMessageBox::warning(this, tr("Open URL"), tr("Cannot open URL %1.").arg(u.toString()));
                    }
                }
                break;
            }
            default:
                break;
        }
    });
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    connect(term, &QTermWidget::handleCtrlC, this, [&](void){
        QString text = term->selectedText();
        if(!text.isEmpty()) {
            QApplication::clipboard()->setText(text, QClipboard::Clipboard);
        }
    });
#endif

    connect(process, &InternalCommandProcess::showString, this,
        [&](const QString &name, const QString &str){
        QMessageBox::information(this->parentWidget(), name, str);
    });
    connect(process, &InternalCommandProcess::showEasterEggs, this, [&](void){
        class MyDialog : public QDialog {
        public:
            MyDialog(QWidget *parent = nullptr) : QDialog(parent) {
                setAttribute(Qt::WA_StyledBackground, true);
                setStyleSheet("QWidget#MyDialog {background-color: transparent;}");
            #if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
                setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint);
            #else
                setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
            #endif
                QPixmap pix;
                pix.load(":/icons/icons/about2.png",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
                resize(pix.size());
                setMask(QBitmap(pix.mask()));
                QVBoxLayout *layout = new QVBoxLayout(this);
                QLabel *label = new QLabel("<html><head/><body><p align=\"center\">I love Tangtang!</p></body></html>", this);
                layout->addWidget(label);
            }
    
        protected:
            void paintEvent(QPaintEvent *event) {
                QString paths = ":/icons/icons/about2.png";
                QPainter painter(this);
                painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));
                event->accept();
            }
            void mouseReleaseEvent(QMouseEvent *event) {
                if( event->button() == Qt::LeftButton) {
                    accept();
                }
            }
        };
        MyDialog show;
        show.setObjectName("MyDialog");
        show.exec();
    });

    term->startTerminalTeletype();
    process->start();
}

InternalCommandWindow::~InternalCommandWindow() {
    delete process;
    delete term;
    delete ui;
}

void InternalCommandWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = new QMenu(this);
    QList<QAction*> ftActions = term->filterActions(event->pos());
    if(!ftActions.isEmpty()) {
        menu->addActions(ftActions);
        menu->addSeparator();
    }
    QAction *copyAction = new QAction(tr("Copy"),this);
    connect(copyAction,&QAction::triggered,this,[&](){
        term->copyClipboard();
    });
    menu->addAction(copyAction);

    QAction *pasteAction = new QAction(tr("Paste"),this);
    connect(pasteAction,&QAction::triggered,this,[&](){
        term->pasteClipboard();
    });
    menu->addAction(pasteAction);

    QAction *selectAllAction = new QAction(tr("Select All"),this);
    connect(selectAllAction,&QAction::triggered,this,[&](){
        term->selectAll();
    });
    menu->addAction(selectAllAction);
    
    QAction *findAction = new QAction(tr("Find"),this);
    connect(findAction,&QAction::triggered,this,[&](){
        term->toggleShowSearchBar();
    });
    menu->addAction(findAction);

    QRect screenGeometry = QGuiApplication::screenAt(cursor().pos())->geometry();
    QPoint pos = cursor().pos() + QPoint(5,5);
    if (pos.x() + menu->width() > screenGeometry.right()) {
        pos.setX(screenGeometry.right() - menu->width());
    }
    if (pos.y() + menu->height() > screenGeometry.bottom()) {
        pos.setY(screenGeometry.bottom() - menu->height());
    }
    menu->popup(pos);
}
