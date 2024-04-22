#ifndef MISC_H
#define MISC_H

#include <QApplication>

#if defined(Q_OS_WIN)

namespace MiscWin32 {

QString getUserName(void);
QString getComputerName(void);

}

#endif

#endif // MISC_H
