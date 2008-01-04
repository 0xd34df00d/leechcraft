TEMPLATE = lib
CONFIG += plugin release threads
TARGET = leechcraft_spacecraft
DESTDIR = ../bin
DEPENDPATH += .
INCLUDEPATH += ../../

# Input
HEADERS += spacecraft.h gamewidget.h particle.h
SOURCES += spacecraft.cpp gamewidget.cpp particle.cpp
RESOURCES = resources.qrc
win32:LIBS += -L../.. -lplugininterface -lexceptions -lsettingsdialog
