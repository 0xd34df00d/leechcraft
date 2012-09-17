CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_deadlyrics)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	deadlyrics.cpp
	searcher.cpp
	lyricscache.cpp
	xmlsettingsmanager.cpp
	sitessearcher.cpp
	concretesite.cpp
	)
SET (HEADERS
	deadlyrics.h
	searcher.h
	lyricscache.h
	xmlsettingsmanager.h
	sitessearcher.h
	concretesite.h
	)
CreateTrs ("deadlyrics" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
SET (RESOURCES deadlyricsresources.qrc)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_deadlyrics SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_deadlyrics
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_deadlyrics DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES deadlyricssettings.xml DESTINATION ${LC_SETTINGS_DEST})
