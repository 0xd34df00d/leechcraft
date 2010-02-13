IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_lmp)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

FIND_PACKAGE (Phonon)

IF (NOT QT_PHONON_FOUND)
	MESSAGE (ERROR "Phonon not found, LMP could not be built")
ENDIF (NOT QT_PHONON_FOUND)

SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_PHONON_INCLUDES}
	${LEECHCRAFT_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	)
SET (SRCS
	lmp.cpp
	core.cpp
	xmlsettingsmanager.cpp
	keyinterceptor.cpp
	videosettings.cpp
	entitychecker.cpp
	player.cpp
	playerwidget.cpp
	defaultwidget.cpp
	)
SET (HEADERS
	lmp.h
	core.h
	xmlsettingsmanager.h
	keyinterceptor.h
	videosettings.h
	entitychecker.h
	player.h
	playerwidget.h
	defaultwidget.h
	phonon.h
	)
SET (FORMS
    player.ui
	playerwidget.ui
	videosettings.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_lmp_ru_RU.ts
	leechcraft_lmp_uk_UA.ts
	leechcraft_lmp_ar.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lmp_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lmp_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lmp_ar.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_lmp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_lmp
	${QT_LIBRARIES}
	${QT_PHONON_LIBS}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_lmp DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES lmpsettings.xml DESTINATION ${LC_SETTINGS_DEST})
