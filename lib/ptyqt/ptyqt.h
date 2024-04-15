#ifndef PTYQT_H
#define PTYQT_H

#include "iptyprocess.h"

class PtyQt
{
public:
    static IPtyProcess *createPtyProcess(IPtyProcess::PtyType ptyType = IPtyProcess::AutoPty);
};

#endif // PTYQT_H
