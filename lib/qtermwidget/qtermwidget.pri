INCLUDEPATH += \
        -I $$PWD/utf8proc \
        -I $$PWD 

SOURCES += \
    $$PWD/utf8proc/utf8proc.c \
    $$PWD/utf8proc/utf8proc_data.c \
    $$PWD/BlockArray.cpp \
    $$PWD/ColorScheme.cpp \
    $$PWD/Emulation.cpp \
    $$PWD/Filter.cpp \
    $$PWD/History.cpp \
    $$PWD/HistorySearch.cpp \
    $$PWD/KeyboardTranslator.cpp \
    $$PWD/CharWidth.cpp \
    $$PWD/qtermwidget.cpp \
    $$PWD/Screen.cpp \
    $$PWD/ScreenWindow.cpp \
    $$PWD/SearchBar.cpp \
    $$PWD/Session.cpp \
    $$PWD/TerminalCharacterDecoder.cpp \
    $$PWD/TerminalDisplay.cpp \
    $$PWD/tools.cpp \
    $$PWD/Vt102Emulation.cpp

HEADERS += \
    $$PWD/utf8proc/utf8proc.h \
    $$PWD/BlockArray.h \
    $$PWD/CharacterColor.h \
    $$PWD/Character.h \
    $$PWD/ColorScheme.h \
    $$PWD/ColorTables.h \
    $$PWD/DefaultTranslatorText.h \
    $$PWD/Emulation.h \
    $$PWD/ExtendedDefaultTranslator.h \
    $$PWD/Filter.h \
    $$PWD/History.h \
    $$PWD/HistorySearch.h \
    $$PWD/KeyboardTranslator.h \
    $$PWD/CharWidth.h \
    $$PWD/LineFont.h \
    $$PWD/qtermwidget.h \
    $$PWD/qtermwidget_version.h \
    $$PWD/Screen.h \
    $$PWD/ScreenWindow.h \
    $$PWD/SearchBar.h \
    $$PWD/Session.h \
    $$PWD/TerminalCharacterDecoder.h \
    $$PWD/TerminalDisplay.h \
    $$PWD/tools.h \
    $$PWD/Vt102Emulation.h

FORMS += \
    $$PWD/SearchBar.ui
    
RESOURCES += \
    $$PWD/res.qrc
