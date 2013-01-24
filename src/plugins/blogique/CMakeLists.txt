CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_blogique)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTSQL TRUE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
	accountslistwidget.cpp
	accountsselectdialog.cpp
	addaccountwizardfirstpage.cpp
	backupmanager.cpp
	blogique.cpp
	blogiquewidget.cpp
	calendarwidget.cpp
	core.cpp
	localstorage.cpp
	pluginproxy.cpp
	profiledialog.cpp
	updateentriesdialog.cpp
	xmlsettingsmanager.cpp
	)

SET (HEADERS
	accountslistwidget.h
	accountsselectdialog.h
	addaccountwizardfirstpage.h
	backupmanager.h
	blogique.h
	blogiquewidget.h
	calendarwidget.h
	core.h
	localstorage.h
	pluginproxy.h
	profiledialog.h
	updateentriesdialog.h
	xmlsettingsmanager.h
	)

SET (FORMS accountslistwidget.ui
	accountsselectdialog.ui
	addaccountwizardfirstpage.ui
	blogiquewidget.ui
	profiledialog.ui
	updateentriesdialog.ui
	)

SET (RESOURCES blogiqueresources.qrc)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

QT4_WRAP_UI (FORMS_HEADERS ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_blogique SHARED
	${SRCS}
	${HEADERS}
	${MOC_SRCS}
	${FORMS_HEADERS}
	${RCCS}
	)

TARGET_LINK_LIBRARIES (leechcraft_blogique
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_blogique DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES blogiquesettings.xml DESTINATION ${LC_SETTINGS_DEST})

SET (BLOGIQUE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

OPTION (ENABLE_BLOGIQUE_METIDA "Build Metida, LiveJournal blogging platform support for Blogique" ON)
OPTION (ENABLE_BLOGIQUE_HESTIA "Build Hestia, platform support local blogging" ON)

IF (ENABLE_BLOGIQUE_METIDA)
	ADD_SUBDIRECTORY (plugins/metida)
ENDIF (ENABLE_BLOGIQUE_METIDA)

IF (ENABLE_BLOGIQUE_HESTIA)
	ADD_SUBDIRECTORY (plugins/hestia)
ENDIF (ENABLE_BLOGIQUE_HESTIA)
