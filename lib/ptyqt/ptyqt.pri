HEADERS += \
    $$PWD/iptyprocess.h \
    $$PWD/ptyqt.h

SOURCES += \
    $$PWD/ptyqt.cpp


INCLUDEPATH += $$PWD

win32:{
    HEADERS += \
        $$PWD/conptyprocess.h \
        $$PWD/winptyprocess.h

    SOURCES += \
        $$PWD/conptyprocess.cpp \
        $$PWD/winptyprocess.cpp

    WINPTY_DIR=D:\quardCRT\depend\winpty
    INCLUDEPATH += $${WINPTY_DIR}\include
    LIBS += $${WINPTY_DIR}\lib\winpty.lib

    LIBS += -lwsock32 -lws2_32 -lcrypt32 -liphlpapi -lnetapi32 -lversion -lwinmm -luserenv
}

unix:{
    HEADERS += \
        $$PWD/unixptyprocess.h

    SOURCES += \
        $$PWD/unixptyprocess.cpp

    LIBS += -lpthread -ldl
}
