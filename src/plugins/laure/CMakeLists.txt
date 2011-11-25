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

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

OPTION (TESTS_LAURE "Enable Laure tests" OFF)
OPTION (ENABLE_LASTFM "Enable Last.fm submitter" ON)

SET (QT_USE_QTGUI TRUE)
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
IF (TESTS_LAURE)
	ENABLE_TESTING ()
	SET (QT_USE_QTTEST TRUE)
ENDIF (TESTS_LAURE)

FIND_PACKAGE (VLC REQUIRED)
FIND_PACKAGE (LastFM REQUIRED)
FIND_PACKAGE (MAGIC REQUIRED)

IF (LastFM_FOUND)
	ADD_DEFINITIONS (-DHAVE_LASTFM)
ENDIF (LastFM_FOUND)

IF (MAGIC_FOUND)
	ADD_DEFINITIONS (-DHAVE_MAGIC)
ENDIF (MAGIC_FOUND)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${VLC_INCLUDE_DIR}
	${LastFM_INCLUDE_DIR}
	${MAGIC_INCLUDE_DIR}
	)

SET (SRCS
	separateplayer.cpp
	playbackmodemenu.cpp
	nowplayingdelegate.cpp
	volumeslider.cpp
	vlcwrapper.cpp
	playlistaddmenu.cpp
	lastfmsubmitter.cpp
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
	separateplayer.h
	playbackmodemenu.h
	nowplayingdelegate.h
	volumeslider.h
	vlcwrapper.h
	playlistaddmenu.h
	lastfmsubmitter.h
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
)
SET (RESOURCES laureresources.qrc)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (FORMS_HEADERS ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrVars ("laure" "en;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
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
	${MAGIC_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_laure DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES lauresettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

SET (LAURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

IF (TESTS_LAURE)
	ADD_SUBDIRECTORY (tests)
ENDIF (TESTS_LAURE)
