IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_laure)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

SET (QT_USE_QTGUI TRUE)
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
FIND_PACKAGE (VLC REQUIRED)
FIND_PACKAGE (LastFM REQUIRED)

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${VLC_INCLUDE_DIR}
	${LastFM_INCLUDE_DIR}
	)
SET (SRCS
	separateplayerwidget.cpp
	playlistaddmenu.cpp
	lastfmsubmitter.cpp
	playlistmodel.cpp
	playlistview.cpp
	xmlsettingsmanager.cpp
	playlistwidget.cpp
	chooseurldialog.cpp
	playpauseaction.cpp
	player.cpp
	laurewidget.cpp
	laure.cpp
	)
SET (HEADERS
	separateplayerwidget.h
	playlistaddmenu.h
	lastfmsubmitter.h
	playlistmodel.h
	playlistview.h
	xmlsettingsmanager.h
	playlistwidget.h
	chooseurldialog.h
	playpauseaction.h
	player.h
	laurewidget.h
	laure.h
	)
SET (FORMS
	laurewidget.ui
	chooseurldialog.ui
	playlistwidget.ui
)
SET (RESOURCES laureresources.qrc)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (FORMS_HEADERS ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_laure SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${FORMS_HEADERS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_laure
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${VLC_LIBRARIES}
	${LastFM_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_laure DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES lauresettings.xml DESTINATION ${LC_SETTINGS_DEST})
