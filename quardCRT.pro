!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets xml svg websockets serialport
QT += core5compat

BUILD_VERSION=0.0.1
TARGET_ARCH=$${QT_ARCH}
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION="\\\"V$${BUILD_VERSION}\\\""
QMAKE_CXXFLAGS += -Wno-deprecated-copy

include(./lib/QFontIcon/QFontIcon.pri)
include(./lib/qtermwidget/qtermwidget.pro)
include(./lib/qtxyzmodem/qtxyzmodem.pro)
include(./lib/QTelnet/QTelnet.pri)
include(./lib/ptyqt/ptyqt.pri)


SOURCES += \
    src/quickconnectwindow.cpp \
    src/sessionswindow.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/quickconnectwindow.h \
    src/sessionswindow.h \
    src/mainwindow.h

FORMS += \
    src/quickconnectwindow.ui \
    src/mainwindow.ui

RESOURCES += \
    res/resource.qrc \
    res/terminalfont.qrc \
    theme/dark/darkstyle.qrc \
    theme/light/lightstyle.qrc

TRANSLATIONS += \
    lang/quardCRT_zh_CN.ts \
    lang/quardCRT_ja_JP.ts \
    lang/quardCRT_en_US.ts

build_type =
CONFIG(debug, debug|release) {
    build_type = build_debug
} else {
    build_type = build_release
}

DESTDIR     = $$build_type/out
OBJECTS_DIR = $$build_type/obj
MOC_DIR     = $$build_type/moc
RCC_DIR     = $$build_type/rcc
UI_DIR      = $$build_type/ui


# 平台配置
win32:{
    VERSION = $${BUILD_VERSION}.000
    RC_ICONS = "icons\ico.ico"
    QMAKE_TARGET_PRODUCT = "quardCRT"
    QMAKE_TARGET_DESCRIPTION = "quardCRT based on Qt $$[QT_VERSION]"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License v3.0"

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --tags | $$PWD/tools/awk/awk.exe \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.inc")
}

unix:!macx:{
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie 
    LIBS += -lutil
    
    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --tags | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.inc")
}

macx:{
    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons\ico.icns"
    LIBS += -lutil

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --tags | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.inc")
}

git_tag.target = $$PWD/git_tag.inc
git_tag.depends = FORCE
PRE_TARGETDEPS += $$PWD/git_tag.inc
QMAKE_EXTRA_TARGETS += git_tag
