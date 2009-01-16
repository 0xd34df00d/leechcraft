INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
					 ${QT_QTCORE_INCLUDE_DIR}
					 ${QT_QTGUI_INCLUDE_DIR}
					 ${QT_QTNETWORK_INCLUDE_DIR}
					 ${CMAKE_CURRENT_BINARY_DIR}
					 ${Boost_INCLUDE_DIR}
					 ../..
					 .
					 zlib/
					 ${RBTorrent_INCLUDE_DIR})
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
	torrentfilesmodel.cpp
	peersmodel.cpp
	filesviewdelegate.cpp
	movetorrentfiles.cpp
	representationmodel.cpp
	trackerschanger.cpp
	historymodel.cpp
	exportdialog.cpp
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
	torrentfilesmodel.h
	peersmodel.h
	filesviewdelegate.h
	movetorrentfiles.h
	representationmodel.h
	trackerschanger.h
	historymodel.h
	exportdialog.h
	newtorrentparams.h
	torrentinfo.h
	fileinfo.h
	peerinfo.h
	)
SET (FORMS
    tabwidget.ui
	addtorrent.ui
	addmultipletorrents.ui
	newtorrentfirststep.ui
	newtorrentsecondstep.ui
	newtorrentthirdstep.ui
	movetorrentfiles.ui
	trackerschanger.ui
	exportdialog.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_torrent_ru_RU.ts
	leechcraft_torrent_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_torrent_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_torrent_uk_UA.qm
	)

IF (UNIX)
	FIND_PACKAGE (PkgConfig)
	pkg_check_modules (RB_LT REQUIRED libtorrent-rasterbar>=0.15-svn)
	ADD_DEFINITIONS (${RB_LT_CFLAGS})
ENDIF (UNIX)

ADD_DEFINITIONS(-DBOOST_DYN_LINK)
ADD_DEFINITIONS(-DTORRENT_LINKING_SHARED)
#ADD_DEFINITIONS(-D_DLL)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_torrent SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)

TARGET_LINK_LIBRARIES (leechcraft_torrent
	${Boost_SYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	${RBTorrent_LIBRARY}
)
INSTALL (TARGETS leechcraft_torrent DESTINATION ${LC_PLUGINS_DEST})
