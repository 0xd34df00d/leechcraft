CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_netstoremanager)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	netstoremanager.cpp
	managertab.cpp
	xmlsettingsmanager.cpp
	accountsmanager.cpp
	accountslistwidget.cpp
	addaccountdialog.cpp
	upmanager.cpp
	)
SET (HEADERS
	netstoremanager.h
	managertab.h
	xmlsettingsmanager.h
	accountsmanager.h
	accountslistwidget.h
	addaccountdialog.h
	upmanager.h
	)
SET (FORMS
	managertab.ui
	accountslistwidget.ui
	addaccountdialog.ui
	)
SET (RESOURCES netstoremanagerresources.qrc)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrVars ("netstoremanager" "en;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_netstoremanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_netstoremanager
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_netstoremanager DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES netstoremanagersettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

OPTION (ENABLE_NETSTOREMANAGER_YANDEXDISK "Build support for Yandex.Disk" ON)

IF (ENABLE_NETSTOREMANAGER_YANDEXDISK)
	ADD_SUBDIRECTORY (plugins/yandexdisk)
ENDIF (ENABLE_NETSTOREMANAGER_YANDEXDISK)
