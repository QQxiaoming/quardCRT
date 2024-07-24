HEADERS += \
    $$PWD/iptyprocess.h \
    $$PWD/ptyqt.h

SOURCES += \
    $$PWD/ptyqt.cpp


INCLUDEPATH += $$PWD

win32:{
    win32-g++ {
        HEADERS += $$PWD/winptyprocess.h
        SOURCES += $$PWD/winptyprocess.cpp

        WINPTY_DIR=D:\quardCRT\depend\winpty
        INCLUDEPATH += $${WINPTY_DIR}\include\winpty
        LIBS += $${WINPTY_DIR}\lib\winpty.lib
    }
    win32-msvc*{
        HEADERS += $$PWD/conptyprocess.h
        SOURCES += $$PWD/conptyprocess.cpp
        LIBS += -lAdvapi32
    }

    LIBS += -lwsock32 -lws2_32 -lcrypt32 -liphlpapi -lnetapi32 -lversion -lwinmm -luserenv
}

unix:{
    HEADERS += \
        $$PWD/unixptyprocess.h

    SOURCES += \
        $$PWD/unixptyprocess.cpp

    LIBS += -lpthread -ldl
}
