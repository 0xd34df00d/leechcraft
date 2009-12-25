SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${RBTorrent_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

IF (WIN32)
	INCLUDE_DIRECTORIES (${RBTorrent_INCLUDE_DIR}/../zlib)
ENDIF (WIN32)

SET (SRCS
	torrentplugin.cpp
	tabwidget.cpp
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
	exportdialog.cpp
	peerstablinker.cpp
	addpeerdialog.cpp
	ipvalidators.cpp
	addwebseeddialog.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	fastspeedcontrolwidget.cpp
	banpeersdialog.cpp
	ipfilterdialog.cpp
	livestreammanager.cpp
	livestreamplugin.cpp
	livestreamdevice.cpp
	)
SET (HEADERS
	torrentplugin.h
	tabwidget.h
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
	exportdialog.h
	peerstablinker.h
	addpeerdialog.h
	ipvalidators.h
	addwebseeddialog.h
	wizardgenerator.h
	startupfirstpage.h
	fastspeedcontrolwidget.h
	banpeersdialog.h
	ipfilterdialog.h
	livestreammanager.h
	livestreamplugin.h
	livestreamdevice.h
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
	addpeerdialog.ui
	addwebseeddialog.ui
	startupfirstpage.ui
	fastspeedcontrolwidget.ui
	banpeersdialog.ui
	ipfilterdialog.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_bittorrent_ru_RU.ts
	leechcraft_bittorrent_uk_UA.ts
	leechcraft_bittorrent_ar.ts
	leechcraft_bittorrent_pl.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_bittorrent_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_bittorrent_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_bittorrent_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_bittorrent_pl.qm
	)

ADD_DEFINITIONS (-DWIN32_LEAN_AND_MEAN -DBOOST_ALL_NO_LIB)

IF (WIN32)
	ADD_DEFINITIONS (-DTORRENT_DISABLE_GEO_IP)
ENDIF (WIN32)

IF (UNIX)
	FIND_PROGRAM (LPKGC_CMD pkg-config)
	EXECUTE_PROCESS (COMMAND "${LPKGC_CMD}" libtorrent-rasterbar --cflags
		OUTPUT_VARIABLE RB_LT_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	MESSAGE (STATUS "Fetched from libtorrent-rasterbar.pc: ${RB_LT_CFLAGS}")
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RB_LT_CFLAGS}")
ENDIF (UNIX)

IF (WIN32)
	ADD_DEFINITIONS(-DWIN32)
ENDIF (WIN32)
#ADD_DEFINITIONS(-DBOOST_DYN_LINK)
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
ADD_LIBRARY (leechcraft_bittorrent SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)

TARGET_LINK_LIBRARIES (leechcraft_bittorrent
	${Boost_SYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	${RBTorrent_LIBRARY}
)
INSTALL (TARGETS leechcraft_bittorrent DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES torrentsettings.xml DESTINATION ${LC_SETTINGS_DEST})
