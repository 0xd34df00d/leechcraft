SET (QT_USE_QTNETWORK true)
INCLUDE (${CMAKE_ROOT}/Modules/FindBoost.cmake)
INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
					 ${QT_QTCORE_INCLUDE_DIR}
					 ${QT_QTGUI_INCLUDE_DIR}
					 ${QT_QTNETWORK_INCLUDE_DIR}
					 ${CMAKE_CURRENT_BINARY_DIR}
					 ${Boost_INCLUDE_DIRS}
					 ../..
					 .
					 /usr/include/libtorrent/
					 /usr/local/include/libtorrent/
					 libtorrent/
					 zlib/)
LINK_DIRECTORIES (${Boost_LINK_DIRS} /usr/local/lib ../..)

SET (SRCS
	torrentplugin.cpp 
	core.cpp 
	addtorrent.cpp 
	newtorrentwizard.cpp
	intropage.cpp
	firststep.cpp
	secondstep.cpp
	thirdstep.cpp
	addmultipletorrents.cpp
	pieceswidget.cpp
	xmlsettingsmanager.cpp
	piecesmodel.cpp
	trackerschanger.cpp
	channelsfiltermodel.cpp
	torrentfilesmodel.cpp
	peersmodel.cpp)
SET (HEADERS
	torrentplugin.h 
	core.h 
	addtorrent.h 
	newtorrentwizard.h
	intropage.h
	firststep.h
	secondstep.h
	thirdstep.h
	addmultipletorrents.h
	pieceswidget.h
	xmlsettingsmanager.h
	piecesmodel.h
	trackerschanger.h
	channelsfiltermodel.h
	torrentfilesmodel.h
	peersmodel.h
	newtorrentparams.h
	overallstats.h
	torrentinfo.h 
	fileinfo.h
	peerinfo.h
	../../plugininterface/basesettingsmanager.h)
SET (FORMS
    mainwindow.ui
	addtorrent.ui
	addmultipletorrents.ui
	newtorrentfirststep.ui
	newtorrentsecondstep.ui
	newtorrentthirdstep.ui
	trackerschanger.ui)
SET (RESOURCES resources.qrc)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
ADD_DEFINITIONS(-DWIN32_WINNT=0x0501)
ADD_DEFINITIONS(-DWIN32_WINDOWS=0x0500)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
ADD_LIBRARY (leechcraft_torrent SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS})
IF (WIN32)
TARGET_LINK_LIBRARIES (leechcraft_torrent
${QT_LIBRARIES}
plugininterface
exceptions
xmlsettingsdialog
torrent
boost_date_time-mgw42-mt-1_34_1
boost_filesystem-mgw42-mt-1_34_1
boost_thread-mgw42-mt-1_34_1)
ELSE (WIN32)
TARGET_LINK_LIBRARIES (leechcraft_torrent
${QT_LIBRARIES}
plugininterface
exceptions
xmlsettingsdialog
torrent)
ENDIF (WIN32)
INSTALL (TARGETS leechcraft_torrent DESTINATION share/leechcraft/plugins)
