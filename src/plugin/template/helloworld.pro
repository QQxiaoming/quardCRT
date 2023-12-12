TEMPLATE = lib
CONFIG += plugin
QT += core widgets

INCLUDEPATH += ../
HEADERS = ../plugininterface.h \
        helloworld.h
SOURCES = ../plugininterface.cpp \
        helloworld.cpp

TARGET = $$qtLibraryTarget(helloworld)
