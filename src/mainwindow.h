#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLocale>
#include <QSocketNotifier>

#include "ptyqt.h"

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
    IPtyProcess *localShell = nullptr;
    pid_t localShellPid = 0;
};
#endif // MAINWINDOW_H
