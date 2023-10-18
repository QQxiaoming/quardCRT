!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets xml svgwidgets websockets serialport
QT += core5compat

BUILD_VERSION=0.2.0
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
include(./lib/QtFancyTabWidget/QtFancyTabWidget.pri)
include(./lib/Qtftp/Qtftp.pri)

SOURCES += \
    src/commandwindow.cpp \
    src/globaloptions.cpp \
    src/globalsetting.cpp \
    src/keymapmanager.cpp \
    src/quickconnectwindow.cpp \
    src/sessionmanagerwidget.cpp \
    src/sessiontab.cpp \
    src/sessionswindow.cpp \
    src/sessiontreeview.cpp \
    src/hexviewwindow.cpp \
    src/starttftpseverwindow.cpp \
    src/treemodel.cpp \
    src/mainwidgetgroup.cpp \
    src/mainwindow.cpp \
    src/main.cpp

HEADERS += \
    src/argv_split.h \
    src/commandwindow.h \
    src/globaloptions.h \
    src/globalsetting.h \
    src/keymapmanager.h \
    src/quickconnectwindow.h \
    src/sessionmanagerwidget.h \
    src/sessiontab.h \
    src/sessionswindow.h \
    src/sessiontreeview.h \
    src/hexviewwindow.h \
    src/starttftpseverwindow.h \
    src/treemodel.h \
    src/mainwidgetgroup.h \
    src/mainwindow.h

FORMS += \
    src/commandwindow.ui \
    src/globaloptions.ui \
    src/keymapmanager.ui \
    src/quickconnectwindow.ui \
    src/hexviewwindow.ui \
    src/mainwindow.ui \
    src/sessionmanagerwidget.ui \
    src/starttftpseverwindow.ui

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
