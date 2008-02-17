######################################################################
# Automatically generated by qmake (2.01a) Wed Dec 26 09:53:08 2007
######################################################################

TEMPLATE = lib
CONFIG += plugin release threads
TARGET = leechcraft_torrent
DESTDIR += ../bin
DEPENDPATH += .
INCLUDEPATH += ../../
INCLUDEPATH += /usr/include/libtorrent 
INCLUDEPATH += /usr/local/include/libtorrent 
INCLUDEPATH += .
INCLUDEPATH += libtorrent/
INCLUDEPATH += zlib/

# Input
HEADERS += torrentplugin.h \
		   core.h \
		   addtorrent.h \
		   settingsmanager.h \
		   torrentinfo.h \
		   contextabletree.h \
		   newtorrentwizard.h \
		   intropage.h \
		   firststep.h \
		   secondstep.h \
		   thirdstep.h \
		   fileinfo.h \
		   peerinfo.h \
		   overallstats.h \
		   addmultipletorrens.h \
		   newtorrentparams.h
SOURCES += torrentplugin.cpp \
		   core.cpp \
		   addtorrent.cpp \
		   contextabletree.cpp \
		   newtorrentwizard.cpp \
		   intropage.cpp \
		   firststep.cpp \
		   secondstep.cpp \
		   thirdstep.cpp \
		   addmultipletorrens.cpp \
		   settingsmanager.cpp
FORMS += mainwindow.ui addtorrent.ui newtorrentfirststep.ui \
	 	 newtorrentsecondstep.ui newtorrentthirdstep.ui \
		 addmultipletorrents.ui
RESOURCES += resources.qrc
TRANSLATIONS += leechcraft_torrent_ru.ts
win32:LIBS += -L../.. -lplugininterface -lexceptions -lsettingsdialog -Llibs -lws2_32 -lboost_date_time-mgw42-mt-1_34_1 -lboost_filesystem-mgw42-mt-1_34_1 -lboost_thread-mgw42-mt-1_34_1 -lzdll
LIBS += -ltorrent
