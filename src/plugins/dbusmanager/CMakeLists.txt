IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_dbusmanager)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem system thread signals)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

IF (NOT (QT_QTDBUS_FOUND AND UNIX))
	MESSAGE (ERROR "DBusManager could not be built")
ENDIF (NOT (QT_QTDBUS_FOUND AND UNIX))

SET (QT_USE_QTDBUS TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
	dbusmanager.cpp
	core.cpp
	iinfoadaptor.cpp
	notificationmanager.cpp
	xmlsettingsmanager.cpp
	general.cpp
	generaladaptor.cpp
	tasks.cpp
	tasksadaptor.cpp
	)
SET (HEADERS
	dbusmanager.h
	core.h
	iinfoadaptor.h
	notificationmanager.h
	xmlsettingsmanager.h
	general.h
	generaladaptor.h
	tasks.h
	tasksadaptor.h
	)
SET (FORMS
	)
SET (RESOURCES
	dbusmanagerresources.qrc
	)
SET (TRANSLATIONS
	leechcraft_dbusmanager_ru_RU.ts
	leechcraft_dbusmanager_uk_UA.ts
	leechcraft_dbusmanager_eo.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_dbusmanager_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_dbusmanager_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_dbusmanager_eo.qm
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
ADD_LIBRARY (leechcraft_dbusmanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_dbusmanager
		${QT_LIBRARIES}
		${LEECHCRAFT_LIBRARIES}
		)
INSTALL (TARGETS leechcraft_dbusmanager DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES dbusmanagersettings.xml DESTINATION ${LC_SETTINGS_DEST})
