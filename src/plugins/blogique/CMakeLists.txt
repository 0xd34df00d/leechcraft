cmake_minimum_required (VERSION 3.3)
project (leechcraft_blogique)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	accountslistwidget.cpp
	accountsselectdialog.cpp
	addaccountwizardfirstpage.cpp
	addcommentdialog.cpp
	blogique.cpp
	blogentrieswidget.cpp
	blogiquewidget.cpp
	calendarwidget.cpp
	commentsmanager.cpp
	commentsmodel.cpp
	commentswidget.cpp
	core.cpp
	draftentrieswidget.cpp
	dummytexteditor.cpp
	entriesfilterproxymodel.cpp
	exportwizard.cpp
	pluginproxy.cpp
	profiledialog.cpp
	selecttargetdelegate.cpp
	sortcommentsproxymodel.cpp
	storagemanager.cpp
	submittodialog.cpp
	tagsproxymodel.cpp
	updateentriesdialog.cpp
	utils.cpp
	xmlsettingsmanager.cpp
	)

set (FORMS
	accountslistwidget.ui
	accountsselectdialog.ui
	addaccountwizardfirstpage.ui
	addcommentdialog.ui
	blogentrieswidget.ui
	blogiquewidget.ui
	commentswidget.ui
	draftentrieswidget.ui
	exportwizard.ui
	profiledialog.ui
	submittodialog.ui
	updateentriesdialog.ui
	)

set (RESOURCES blogiqueresources.qrc)

CreateTrs ("blogique" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("blogique" "en;ru_RU" "${SRCS}" "${FORMS}" "blogiquesettings.xml")

QtWrapUi (FORMS_HEADERS ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_blogique SHARED
	${SRCS}
	${MOC_SRCS}
	${FORMS_HEADERS}
	${RCCS}
	${COMPILED_TRANSLATIONS}
	)

target_link_libraries (leechcraft_blogique
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blogique DESTINATION ${LC_PLUGINS_DEST})
install (FILES blogiquesettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml5/blogique DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_blogique PrintSupport QuickWidgets Sql WebKitWidgets Xml)

set (BLOGIQUE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

option (ENABLE_BLOGIQUE_METIDA "Build Metida, LiveJournal blogging platform" ON)
option (ENABLE_BLOGIQUE_HESTIA "Build Hestia, local blogging platform" ON)

if (ENABLE_BLOGIQUE_METIDA)
	add_subdirectory (plugins/metida)
endif ()

if (ENABLE_BLOGIQUE_HESTIA)
	add_subdirectory (plugins/hestia)
endif ()
