#include "misc.h"

#if defined(Q_OS_WIN)
#include <windows.h>

#include <QString>

namespace MiscWin32 {

QString getUserName(void) {
    DWORD usernameSize = 0;
    GetUserNameW(NULL, &usernameSize);

    WCHAR *username = new WCHAR[usernameSize+1];
    if (GetUserNameW(username, &usernameSize) == 0) {
        return "UnknowUserName";
    }

    QString qsUsername = QString::fromWCharArray(username, usernameSize-1);
    delete[] username;
    if(qsUsername.isEmpty()) {
        return "UnknowUserName";
    }
    return qsUsername;
}

QString getComputerName(void) {
    DWORD computernameSize = 0;
    GetComputerNameW(NULL, &computernameSize);

    WCHAR *computername = new WCHAR[computernameSize+1];
    if (GetComputerNameW(computername, &computernameSize) == 0) {
        return "UnknowComputerName";
    }

    QString qsComputername = QString::fromWCharArray(computername, computernameSize);
    delete[] computername;
    if(qsComputername.isEmpty()) {
        return "UnknowComputerName";
    }
    return qsComputername;
}

}

#endif
