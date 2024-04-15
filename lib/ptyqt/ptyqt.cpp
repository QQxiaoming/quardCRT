#include "ptyqt.h"
#include <utility>

#ifdef Q_OS_WIN
#if defined(Q_CC_GNU)

#include "winptyprocess.h"

IPtyProcess *PtyQt::createPtyProcess(IPtyProcess::PtyType ptyType) {
    Q_UNUSED(ptyType);
    return new WinPtyProcess();
}

#endif

#if defined(Q_CC_MSVC)

#include "conptyprocess.h"

IPtyProcess *PtyQt::createPtyProcess(IPtyProcess::PtyType ptyType) {
    Q_UNUSED(ptyType);
    return new ConPtyProcess();
}

#endif
#endif

#ifdef Q_OS_UNIX

#include "unixptyprocess.h"

IPtyProcess *PtyQt::createPtyProcess(IPtyProcess::PtyType ptyType) {
    Q_UNUSED(ptyType);
    return new UnixPtyProcess();
}

#endif
