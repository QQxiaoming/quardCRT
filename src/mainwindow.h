#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLocale>
#include <QSocketNotifier>

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
#include <pty.h>
#include <unistd.h>
#include <signal.h>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static void setAppLangeuage(QLocale::Language lang);

private:
    Ui::MainWindow *ui;
    QSocketNotifier *localShell = nullptr;
    pid_t localShellPid = 0;
};
#endif // MAINWINDOW_H
