HEADERS += \
    $$PWD/sshchannel.h \
    $$PWD/sshclient.h \
    $$PWD/sshprocess.h \
    $$PWD/sshshell.h \
    $$PWD/sshkey.h \
    $$PWD/sshscpget.h \
    $$PWD/sshscpsend.h \
    $$PWD/sshsftpcommandget.h \
    $$PWD/sshsftpcommandfileinfo.h \
    $$PWD/sshsftpcommand.h \
    $$PWD/sshsftpcommandmkdir.h \
    $$PWD/sshsftpcommandsend.h \
    $$PWD/sshsftpcommandreaddir.h \
    $$PWD/sshsftpcommandunlink.h \
    $$PWD/sshsftp.h \
    $$PWD/sshtunneldataconnector.h \
    $$PWD/sshtunnelinconnection.h \
    $$PWD/sshtunneloutconnection.h \
    $$PWD/sshtunnelout.h \
    $$PWD/sshtunnelin.h

SOURCES += \
    $$PWD/sshchannel.cpp \
    $$PWD/sshclient.cpp \
    $$PWD/sshkey.cpp \
    $$PWD/sshprocess.cpp \
    $$PWD/sshshell.cpp \
    $$PWD/sshscpget.cpp \
    $$PWD/sshscpsend.cpp \
    $$PWD/sshsftpcommand.cpp \
    $$PWD/sshsftpcommandfileinfo.cpp \
    $$PWD/sshsftpcommandget.cpp \
    $$PWD/sshsftpcommandmkdir.cpp \
    $$PWD/sshsftpcommandreaddir.cpp \
    $$PWD/sshsftpcommandsend.cpp \
    $$PWD/sshsftpcommandunlink.cpp \
    $$PWD/sshsftp.cpp \
    $$PWD/sshtunneldataconnector.cpp \
    $$PWD/sshtunnelinconnection.cpp \
    $$PWD/sshtunnelin.cpp \
    $$PWD/sshtunneloutconnection.cpp \
    $$PWD/sshtunnelout.cpp

INCLUDEPATH += \
    $$PWD

LIBSSH2_DIR=/home/qqm/Downloads/work/quardCRT/quardCRT/depend/output




win32:{
    win32-g++ {
        INCLUDEPATH += $${LIBSSH2_DIR}/include
        LIBS += -L$${LIBSSH2_DIR}/lib -lssh2
    }
    win32-msvc*{
        INCLUDEPATH += $${LIBSSH2_DIR}/include
        LIBS += $${LIBSSH2_DIR}\lib\libssh2.lib
    }
}

unix:{
    INCLUDEPATH += $${LIBSSH2_DIR}/include
    LIBS += -L$${LIBSSH2_DIR}/lib -lssh2
}
