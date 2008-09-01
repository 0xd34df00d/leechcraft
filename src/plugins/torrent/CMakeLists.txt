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
					 zlib/)
LINK_DIRECTORIES (${Boost_LINK_DIRS} /usr/local/lib ../..)
FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem system thread)

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
	torrentfilesmodel.cpp
	peersmodel.cpp
	filesviewdelegate.cpp
	movetorrentfiles.cpp
	threadedpd.cpp
	representationmodel.cpp
	)
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
	torrentfilesmodel.h
	peersmodel.h
	filesviewdelegate.h
	movetorrentfiles.h
	threadedpd.h
	representationmodel.h
	newtorrentparams.h
	overallstats.h
	torrentinfo.h 
	fileinfo.h
	peerinfo.h
	../../xmlsettingsdialog/basesettingsmanager.h
	)
SET (FORMS
    tabwidget.ui
	addtorrent.ui
	addmultipletorrents.ui
	newtorrentfirststep.ui
	newtorrentsecondstep.ui
	newtorrentthirdstep.ui
	trackerschanger.ui
	movetorrentfiles.ui)
SET (RESOURCES resources.qrc)

SET (TRANSLATIONS
	leechcraft_torrent_ru_RU.qm
	leechcraft_torrent_uk_UA.qm
	)

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

ADD_CUSTOM_COMMAND (OUTPUT ${TRANSLATIONS}
	COMMAND "lrelease" ARGS "leechcraft_torrent_ru_RU.ts" "leechcraft_torrent_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_LIBRARY (leechcraft_torrent SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	${TRANSLATIONS}
	)

IF (WIN32)
TARGET_LINK_LIBRARIES (leechcraft_torrent
${QT_LIBRARIES}
plugininterface
exceptions
xmlsettingsdialog
${Boost_LIBRARIES}
torrent
)
ELSE (WIN32)
TARGET_LINK_LIBRARIES (leechcraft_torrent
${QT_LIBRARIES}
plugininterface
exceptions
xmlsettingsdialog
torrent-rasterbar
${Boost_LIBRARIES}
)
ENDIF (WIN32)

INSTALL (TARGETS leechcraft_torrent DESTINATION ${LC_PLUGINS_DEST})
