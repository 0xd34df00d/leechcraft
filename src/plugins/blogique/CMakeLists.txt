CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_blogique)
INCLUDE (InitLCPlugin OPTIONAL)

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
	blogentrieswidget.cpp
	blogiquewidget.cpp
	calendarwidget.cpp
	core.cpp
	draftentrieswidget.cpp
	entriesfilterproxymodel.cpp
	pluginproxy.cpp
	profiledialog.cpp
	selecttargetdelegate.cpp
	storagemanager.cpp
	submittodialog.cpp
	updateentriesdialog.cpp
	utils.cpp
	xmlsettingsmanager.cpp
	)

SET (FORMS
	accountslistwidget.ui
	accountsselectdialog.ui
	addaccountwizardfirstpage.ui
	blogentrieswidget.ui
	blogiquewidget.ui
	draftentrieswidget.ui
	profiledialog.ui
	submittodialog.ui
	updateentriesdialog.ui
	)

SET (RESOURCES blogiqueresources.qrc)

CreateTrs ("blogique" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blogique" "en;ru_RU" "${SRCS}" "${FORMS}" "blogiquesettings.xml")

QtWrapUi (FORMS_HEADERS ${FORMS})
QtAddResources (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_blogique SHARED
	${SRCS}
	${FORMS_HEADERS}
	${RCCS}
	${COMPILED_TRANSLATIONS}
	)

FindQtLibs (leechcraft_blogique Sql)

TARGET_LINK_LIBRARIES (leechcraft_blogique
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_blogique DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES blogiquesettings.xml DESTINATION ${LC_SETTINGS_DEST})

SET (BLOGIQUE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

OPTION (ENABLE_BLOGIQUE_METIDA "Build Metida, LiveJournal blogging platform" ON)
OPTION (ENABLE_BLOGIQUE_HESTIA "Build Hestia, local blogging platform" ON)

IF (ENABLE_BLOGIQUE_METIDA)
	ADD_SUBDIRECTORY (plugins/metida)
ENDIF (ENABLE_BLOGIQUE_METIDA)

IF (ENABLE_BLOGIQUE_HESTIA)
	ADD_SUBDIRECTORY (plugins/hestia)
ENDIF (ENABLE_BLOGIQUE_HESTIA)
