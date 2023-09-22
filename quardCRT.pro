!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets xml svg websockets
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


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/mainwindow.h

FORMS += \
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
    build_type = debug
} else {
    build_type = release
}

DESTDIR     = $$build_type/out
OBJECTS_DIR = $$build_type/obj
MOC_DIR     = $$build_type/moc
RCC_DIR     = $$build_type/rcc
UI_DIR      = $$build_type/ui


win32:!wasm {
    VERSION = $${BUILD_VERSION}.000
    RC_LANG = 0x0004
    RC_ICONS = "icons\icon.ico"

    contains(TARGET_ARCH, x86_64) {
        CONFIG(release, debug|release) {
            AFTER_LINK_CMD_LINE = $$PWD/tools/upx-3.96-win64/upx.exe --best -f $$DESTDIR/$${TARGET}.exe
            QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
        }
    } else {
        QMAKE_LFLAGS += -Wl,--large-address-aware
        CONFIG(release, debug|release) {
            AFTER_LINK_CMD_LINE = $$PWD/tools/upx-3.96-win32/upx.exe --best -f $$DESTDIR/$${TARGET}.exe
            QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
        }
    }
}

unix:!macx:!android:!ios:!wasm {
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie

    CONFIG(release, debug|release) {
        AFTER_LINK_CMD_LINE = upx-ucl --best -f $$DESTDIR/$$TARGET
        QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
    }

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

macx:!ios:!wasm {
    DEFINES += DESKTOP_INTERACTION_MODE
    DEFINES += BUILT_IN_QEMU_MODE

    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons/icon.icns"

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

git_tag.target = $$PWD/git_tag.inc
git_tag.depends = FORCE
PRE_TARGETDEPS += $$PWD/git_tag.inc
QMAKE_EXTRA_TARGETS += git_tag
