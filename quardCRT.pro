!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets xml svgwidgets websockets serialport printsupport multimedia
QT += core5compat

BUILD_VERSION=0.2.3
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

INCLUDEPATH += \
    src/util \
    src/sessionmanagerwidget \
    src/globaloptions \
    src/sessionoptions \
    src/commandwidget \
    src/keymapmanager \
    src/locksessionwindow \
    src/quickconnectwindow \
    src/starttftpseverwindow \
    src/hexviewwindow \
    src

SOURCES += \
    src/util/logger.cpp \
    src/util/globalsetting.cpp \
    src/globaloptions/globaloptionsadvancedwidget.cpp \
    src/globaloptions/globaloptionsappearancewidget.cpp \
    src/globaloptions/globaloptionsterminalwidget.cpp \
    src/globaloptions/globaloptionswindowwidget.cpp \
    src/sessionoptions/sessionoptionsgeneralwidget.cpp \
    src/sessionoptions/sessionoptionslocalshellproperties.cpp \
    src/sessionoptions/sessionoptionsnamepipeproperties.cpp \
    src/sessionoptions/sessionoptionsrawproperties.cpp \
    src/sessionoptions/sessionoptionsserialproperties.cpp \
    src/sessionoptions/sessionoptionstelnetproperties.cpp \
    src/sessionmanagerwidget/sessionmanagerwidget.cpp \
    src/sessionmanagerwidget/sessionmanagertreeview.cpp \
    src/sessionmanagerwidget/sessionmanagertreemodel.cpp \
    src/globaloptions/globaloptionsgeneralwidget.cpp \
    src/globaloptions/globaloptionswindow.cpp \
    src/sessionoptions/sessionoptionswindow.cpp \
    src/commandwidget/commandwidget.cpp \
    src/keymapmanager/keymapmanager.cpp \
    src/locksessionwindow/locksessionwindow.cpp \
    src/quickconnectwindow/quickconnectwindow.cpp \
    src/starttftpseverwindow/starttftpseverwindow.cpp \
    src/hexviewwindow/hexviewwindow.cpp \
    src/sessiontab.cpp \
    src/sessionswindow.cpp \
    src/mainwidgetgroup.cpp \
    src/mainwindow.cpp \
    src/main.cpp

HEADERS += \
    src/util/logger.h \
    src/util/argv_split.h \
    src/util/filedialog.h \
    src/util/globalsetting.h \
    src/globaloptions/globaloptionsadvancedwidget.h \
    src/globaloptions/globaloptionsappearancewidget.h \
    src/globaloptions/globaloptionsterminalwidget.h \
    src/globaloptions/globaloptionswindowwidget.h \
    src/sessionoptions/sessionoptionsgeneralwidget.h \
    src/sessionoptions/sessionoptionslocalshellproperties.h \
    src/sessionoptions/sessionoptionsnamepipeproperties.h \
    src/sessionoptions/sessionoptionsrawproperties.h \
    src/sessionoptions/sessionoptionsserialproperties.h \
    src/sessionoptions/sessionoptionstelnetproperties.h \
    src/sessionmanagerwidget/sessionmanagerwidget.h \
    src/sessionmanagerwidget/sessionmanagertreeview.h \
    src/sessionmanagerwidget/sessionmanagertreemodel.h \
    src/globaloptions/globaloptionsgeneralwidget.h \
    src/globaloptions/globaloptionswindow.h \
    src/sessionoptions/sessionoptionswindow.h \
    src/commandwidget/commandwidget.h \
    src/keymapmanager/keymapmanager.h \
    src/locksessionwindow/locksessionwindow.h \
    src/quickconnectwindow/quickconnectwindow.h \
    src/hexviewwindow/hexviewwindow.h \
    src/starttftpseverwindow/starttftpseverwindow.h \
    src/sessiontab.h \
    src/sessionswindow.h \
    src/mainwidgetgroup.h \
    src/mainwindow.h

FORMS += \
    src/globaloptions/globaloptionsadvancedwidget.ui \
    src/globaloptions/globaloptionsappearancewidget.ui \
    src/globaloptions/globaloptionsterminalwidget.ui \
    src/globaloptions/globaloptionswindowwidget.ui \
    src/sessionmanagerwidget/sessionmanagerwidget.ui \
    src/globaloptions/globaloptionsgeneralwidget.ui \
    src/globaloptions/globaloptionswindow.ui \
    src/sessionoptions/sessionoptionsgeneralwidget.ui \
    src/sessionoptions/sessionoptionslocalshellproperties.ui \
    src/sessionoptions/sessionoptionsnamepipeproperties.ui \
    src/sessionoptions/sessionoptionsrawproperties.ui \
    src/sessionoptions/sessionoptionsserialproperties.ui \
    src/sessionoptions/sessionoptionstelnetproperties.ui \
    src/sessionoptions/sessionoptionswindow.ui \
    src/commandwidget/commandwidget.ui \
    src/keymapmanager/keymapmanager.ui \
    src/locksessionwindow/locksessionwindow.ui \
    src/quickconnectwindow/quickconnectwindow.ui \
    src/hexviewwindow/hexviewwindow.ui \
    src/starttftpseverwindow/starttftpseverwindow.ui \
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
