CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_netstoremanager)
INCLUDE (InitLCPlugin OPTIONAL)

IF (CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
	SET (INOTIFY_DEFAULT TRUE)
ELSE (CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
	SET (INOTIFY_DEFAULT FALSE)
ENDIF (CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)

OPTION (ENABLE_NETSTOREMANAGER_INOTIFY "Enable inotify file watcher backend for NetStoreManager" ${INOTIFY_DEFAULT})

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
	filesmodel.cpp
	filesview.cpp
	syncmanager.cpp
	syncwidget.cpp
	syncitemdelegate.cpp
	directorywidget.cpp
	fileswatcherbase.cpp
	utils.cpp
	)
SET (HEADERS
	netstoremanager.h
	managertab.h
	xmlsettingsmanager.h
	accountsmanager.h
	accountslistwidget.h
	addaccountdialog.h
	upmanager.h
	filesmodel.h
	filesview.h
	syncmanager.h
	syncwidget.h
	syncitemdelegate.h
	directorywidget.h
	fileswatcher.h
	fileswatcherbase.h
	utils.h
	)
SET (FORMS
	managertab.ui
	accountslistwidget.ui
	addaccountdialog.ui
	syncwidget.ui
	directorywidget.ui
	)

IF (ENABLE_NETSTOREMANAGER_INOTIFY)
	ADD_DEFINITIONS (-DENABLE_INOTIFY)
	SET (SRCS ${SRCS} fileswatcher_inotify.cpp)
	SET (HEADERS ${HEADERS} fileswatcher_inotify.h)
ELSE (ENABLE_NETSTOREMANAGER_INOTIFY)
	SET (SRCS ${SRCS} fileswatcher_dummy.cpp)
	SET (HEADERS ${HEADERS} fileswatcher_dummy.h)
ENDIF (ENABLE_NETSTOREMANAGER_INOTIFY)

SET (RESOURCES netstoremanagerresources.qrc)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrs ("netstoremanager" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

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

OPTION (ENABLE_NETSTOREMANAGER_GOOGLEDRIVE "Build support for Google Drive" ON)

IF (ENABLE_NETSTOREMANAGER_GOOGLEDRIVE)
	ADD_SUBDIRECTORY (plugins/googledrive)
ENDIF (ENABLE_NETSTOREMANAGER_GOOGLEDRIVE)
