TEMPLATE = lib
CONFIG += plugin release threads
TARGET = leechcraft_torrent
DESTDIR = ../bin
DEPENDPATH += .
INCLUDEPATH += ../../
INCLUDEPATH += .
QT += network

# Input
RESOURCES = resources.qrc
TRANSLATIONS = leechcraft_torrent_ru.ts
HEADERS = torrentplugin.h \
		  torrentfileparser.h \
		  bencodeparser.h \
		  bencodegenerator.h \
		  peerconnection.h \
		  addtorrentdialog.h \
		  addtorrentwidgetitem.h \
		  block.h \
		  torrentpeer.h \
		  metainfo.h \
		  settingsmanager.h \
		  torrentclient.h \
		  trackerclient.h \
		  torrentserver.h \
		  connectionmanager.h \
		  ratecontroller.h \
		  filemanager.h \
		  torrentviewdelegate.h \
		  sha1/sha1.h \
		  globals.h
SOURCES = torrentplugin.cpp \
		  torrentfileparser.cpp \
		  bencodeparser.cpp \
		  bencodegenerator.cpp \
		  peerconnection.cpp \
		  addtorrentdialog.cpp \
		  addtorrentwidgetitem.cpp \
		  block.cpp \
		  torrentpeer.cpp \
		  metainfo.cpp \
		  settingsmanager.cpp \
		  torrentclient.cpp \
		  trackerclient.cpp \
		  torrentserver.cpp \
		  connectionmanager.cpp \
		  ratecontroller.cpp \
		  filemanager.cpp \
		  torrentviewdelegate.cpp \
		  sha1/sha1.c
FORMS = addtorrentdialog.ui
win32:LIBS += -L../.. -lplugininterface -lexceptions -lsettingsdialog
