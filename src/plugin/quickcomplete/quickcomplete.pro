TEMPLATE = lib
CONFIG += plugin
QT += core widgets

INCLUDEPATH += ../
HEADERS = ../plugininterface.h \
        quickcomplete.h
SOURCES = ../plugininterface.cpp \
        quickcomplete.cpp

TARGET = $$qtLibraryTarget(quickcomplete)
