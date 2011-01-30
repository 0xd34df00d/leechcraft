IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_kinotify)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	kinotify.cpp
	kinotifywidget.cpp
	notificationaction.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	kinotify.h
	kinotifywidget.h
	notificationaction.h
	xmlsettingsmanager.h
	)
SET (TRANSLATIONS
	leechcraft_kinotify_ru_RU.ts
	leechcraft_kinotify_uk_UA.ts
	leechcraft_kinotify_ar.ts
	leechcraft_kinotify_de.ts
	)
SET (RESOURCES kinotifyresources.qrc)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_kinotify_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_kinotify_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_kinotify_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_kinotify_de.qm
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_kinotify SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_kinotify
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_kinotify DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES kinotifysettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY themes DESTINATION share/leechcraft/kinotify)