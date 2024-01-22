!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets xml svgwidgets websockets serialport printsupport multimedia
QT += core5compat

BUILD_VERSION="$$cat(./version.txt)"
TARGET_ARCH=$${QT_ARCH}
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION="\\\"V$${BUILD_VERSION}\\\""

include(./lib/QFontIcon/QFontIcon.pri)
include(./lib/qtermwidget/qtermwidget.pro)
include(./lib/qtxyzmodem/qtxyzmodem.pro)
include(./lib/QTelnet/QTelnet.pri)
include(./lib/ptyqt/ptyqt.pri)
include(./lib/QtFancyTabWidget/QtFancyTabWidget.pri)
include(./lib/Qtftp/Qtftp.pri)
include(./lib/qtssh/qtssh.pri)
include(./lib/qcustomfilesystemmodel/qcustomfilesystemmodel.pri)
include(./lib/qtkeychain/qtkeychain.pri)
include(./lib/QVNCClient/QVNCClient.pri)
include(./lib/qhexedit/qhexedit.pri)
include(./lib/QGoodWindow/QGoodWindow/QGoodWindow.pri)
include(./lib/QGoodWindow/QGoodCentralWidget/QGoodCentralWidget.pri)

INCLUDEPATH += \
    src/util \
    src/sessionswindow \
    src/sessiontab \
    src/sessionmanagerwidget \
    src/globaloptions \
    src/sessionoptions \
    src/commandwidget \
    src/keymapmanager \
    src/locksessionwindow \
    src/quickconnectwindow \
    src/starttftpseverwindow \
    src/hexviewwindow \
    src/sftpwindow \
    src/netscanwindow \
    src/plugininfowindow \
    src/plugin \
    src

SOURCES += \
    src/plugininfowindow/plugininfowindow.cpp \
    src/sftpwindow/sftpmenubookmarkwidget.cpp \
    src/util/logger.cpp \
    src/util/globalsetting.cpp \
    src/util/keychainclass.cpp \
    src/plugin/plugininterface.cpp \
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
    src/sessionoptions/sessionoptionslocalshellstate.cpp \
    src/sessionoptions/sessionoptionsssh2properties.cpp \
    src/sessionoptions/sessionoptionsvncproperties.cpp \
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
    src/netscanwindow/netscanwindow.cpp \
    src/sftpwindow/sftpwindow.cpp \
    src/sessiontab/sessiontab.cpp \
    src/sessionswindow/sessionswindow.cpp \
    src/mainwidgetgroup.cpp \
    src/mainwindow.cpp \
    src/main.cpp

HEADERS += \
    src/plugininfowindow/plugininfowindow.h \
    src/sftpwindow/sftpmenubookmarkwidget.h \
    src/util/logger.h \
    src/util/argv_split.h \
    src/util/filedialog.h \
    src/util/globalsetting.h \
    src/util/keychainclass.h \
    src/util/passwordedit.h \
    src/plugin/plugininterface.h \
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
    src/sessionoptions/sessionoptionslocalshellstate.h \
    src/sessionoptions/sessionoptionsssh2properties.h \
    src/sessionoptions/sessionoptionsvncproperties.h \
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
    src/netscanwindow/netscanwindow.h \
    src/sftpwindow/sftpwindow.h \
    src/sessiontab/sessiontab.h \
    src/sessionswindow/sessionswindow.h \
    src/mainwidgetgroup.h \
    src/mainwindow.h

FORMS += \
    src/globaloptions/globaloptionsadvancedwidget.ui \
    src/globaloptions/globaloptionsappearancewidget.ui \
    src/globaloptions/globaloptionsterminalwidget.ui \
    src/globaloptions/globaloptionswindowwidget.ui \
    src/plugininfowindow/plugininfowindow.ui \
    src/sessionmanagerwidget/sessionmanagerwidget.ui \
    src/globaloptions/globaloptionsgeneralwidget.ui \
    src/globaloptions/globaloptionswindow.ui \
    src/sessionoptions/sessionoptionsgeneralwidget.ui \
    src/sessionoptions/sessionoptionslocalshellproperties.ui \
    src/sessionoptions/sessionoptionslocalshellstate.ui \
    src/sessionoptions/sessionoptionsnamepipeproperties.ui \
    src/sessionoptions/sessionoptionsrawproperties.ui \
    src/sessionoptions/sessionoptionsserialproperties.ui \
    src/sessionoptions/sessionoptionsssh2properties.ui \
    src/sessionoptions/sessionoptionstelnetproperties.ui \
    src/sessionoptions/sessionoptionsvncproperties.ui \
    src/sessionoptions/sessionoptionswindow.ui \
    src/commandwidget/commandwidget.ui \
    src/keymapmanager/keymapmanager.ui \
    src/locksessionwindow/locksessionwindow.ui \
    src/quickconnectwindow/quickconnectwindow.ui \
    src/hexviewwindow/hexviewwindow.ui \
    src/netscanwindow/netscanwindow.ui \
    src/sftpwindow/sftpmenubookmarkwidget.ui \
    src/sftpwindow/sftpwindow.ui \
    src/starttftpseverwindow/starttftpseverwindow.ui \
    src/mainwindow.ui

RESOURCES += \
    res/resource.qrc

TRANSLATIONS += \
    lang/quardCRT_zh_CN.ts \
    lang/quardCRT_zh_HK.ts \
    lang/quardCRT_ru_RU.ts \
    lang/quardCRT_ko_KR.ts \
    lang/quardCRT_ja_JP.ts \
    lang/quardCRT_fr_FR.ts \
    lang/quardCRT_es_ES.ts \
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
    win32-g++ {
        QMAKE_CXXFLAGS += -Wno-deprecated-copy
        RESOURCES += res/terminalfont.qrc
    }
    win32-msvc*{
    }
    VERSION = $${BUILD_VERSION}.000
    RC_ICONS = "icons\ico.ico"
    QMAKE_TARGET_PRODUCT = "quardCRT"
    QMAKE_TARGET_DESCRIPTION = "quardCRT based on Qt $$[QT_VERSION]"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License v3.0"

    build_info.commands = $$quote("c:/Windows/system32/WindowsPowerShell/v1.0/powershell.exe -ExecutionPolicy Bypass -NoLogo -NoProfile -File \"$$PWD/tools/generate_info.ps1\" > $$PWD/build_info.inc")
}

unix:!macx:{
    RESOURCES += res/terminalfont.qrc
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie 
    LIBS += -lutil
    
    build_info.commands = $$quote("cd $$PWD && ./tools/generate_info.sh > build_info.inc")
}

macx:{
    RESOURCES += res/terminalfont.qrc
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons\ico.icns"
    LIBS += -lutil

    build_info.commands = $$quote("cd $$PWD && ./tools/generate_info.sh > build_info.inc")
}

build_info.target = $$PWD/build_info.inc
build_info.depends = FORCE
PRE_TARGETDEPS += $$PWD/build_info.inc
QMAKE_EXTRA_TARGETS += build_info
