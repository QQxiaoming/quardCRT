TEMPLATE = lib
CONFIG += plugin
QT += core widgets

INCLUDEPATH += ../
HEADERS = ../plugininterface.h \
        quickcomplete.h
SOURCES = ../plugininterface.cpp \
        quickcomplete.cpp
TRANSLATIONS = quickcomplete_zh_CN.ts \
        quickcomplete_en_US.ts
RESOURCES = quickcomplete.qrc

TARGET = $$qtLibraryTarget(quickcomplete)

unix:{
    QMAKE_RPATHDIR=$ORIGIN
}
