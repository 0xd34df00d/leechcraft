CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_dbusmanager)
INCLUDE (InitLCPlugin OPTIONAL)

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
CreateTrVars ("dbusmanager" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
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
