CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_liznoo)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (Qwt6)

IF (NOT WIN32)
	SET (QT_USE_QTDBUS TRUE)
ENDIF (NOT WIN32)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${Qwt6_INCLUDE_DIR}
	)
SET (SRCS
	liznoo.cpp
	batteryinfo.cpp
	batteryhistory.cpp
	batteryhistorydialog.cpp
	platformlayer.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	liznoo.h
	batteryinfo.h
	batteryhistory.h
	batteryhistorydialog.h
	platformlayer.h
	xmlsettingsmanager.h
	)
# Default implementation using UPower
IF (NOT WIN32)
	SET (SRCS
		${SRCS}
		platformupower.cpp
		dbusthread.cpp
		dbusconnector.cpp
		)
	SET (HEADERS
		${HEADERS}
		platformupower.h
		dbusthread.h
		dbusconnector.h
		)
# Windows implementation using WinAPI
ELSE (NOT WIN32)
	SET (SRCS
		${SRCS}
		platformwinapi.cpp
		fakeqwidgetwinapi.cpp
		)
	SET (HEADERS
		${HEADERS}
		platformwinapi.h
		fakeqwidgetwinapi.h
		)
ENDIF (NOT WIN32)

SET (FORMS
	batteryhistorydialog.ui
	)
SET (RESOURCES liznooresources.qrc)
CreateTrVars ("liznoo" "en;es;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)
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
ADD_LIBRARY (leechcraft_liznoo SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_liznoo
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${Qwt6_Qt4_LIBRARY}
	)
INSTALL (TARGETS leechcraft_liznoo DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES liznoosettings.xml DESTINATION ${LC_SETTINGS_DEST})
