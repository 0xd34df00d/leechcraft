IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_seekthru)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
)
SET (SRCS
	seekthru.cpp
	core.cpp
	description.cpp
	xmlsettingsmanager.cpp
	searcherslist.cpp
	findproxy.cpp
	searchhandler.cpp
	selectablebrowser.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	tagsasker.cpp
	)
SET (HEADERS
	seekthru.h
	core.h
	description.h
	xmlsettingsmanager.h
	searcherslist.h
	findproxy.h
	searchhandler.h
	selectablebrowser.h
	wizardgenerator.h
	startupfirstpage.h
	tagsasker.h
	)
SET (TRANSLATIONS
	leechcraft_seekthru_ru_RU.ts
	leechcraft_seekthru_uk_UA.ts
	)
SET (FORMS
	searcherslist.ui
	startupfirstpage.ui
	tagsasker.ui
	)
SET (RESOURCES
	seekthruresources.qrc
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_uk_UA.qm
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
ADD_LIBRARY (leechcraft_seekthru SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_seekthru
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_seekthru DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES seekthrusettings.xml DESTINATION ${LC_SETTINGS_DEST})
