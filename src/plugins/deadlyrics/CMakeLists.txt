IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_deadlyrics)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem system thread signals)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

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
	core.cpp
	searcher.cpp
	lyricwikisearcher.cpp
	findproxy.cpp
	lyricscache.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	deadlyrics.h
	core.h
	searcher.h
	lyricwikisearcher.h
	findproxy.h
	lyricscache.h
	xmlsettingsmanager.h
	)
SET (TRANSLATIONS
	leechcraft_deadlyrics_ru_RU.ts
	leechcraft_deadlyrics_uk_UA.ts
	leechcraft_deadlyrics_ar.ts
	leechcraft_deadlyrics_eo.ts
	)
SET (RESOURCES deadlyricsresources.qrc)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_deadlyrics_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_deadlyrics_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_deadlyrics_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_deadlyrics_eo.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
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
