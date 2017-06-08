cmake_minimum_required (VERSION 2.8)
project (leechcraft_netstoremanager)
include (InitLCPlugin OPTIONAL)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (INOTIFY_DEFAULT TRUE)
else ()
	set (INOTIFY_DEFAULT FALSE)
endif ()

option (ENABLE_NETSTOREMANAGER_INOTIFY "Enable inotify file watcher backend for NetStoreManager" ${INOTIFY_DEFAULT})

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	netstoremanager.cpp
	managertab.cpp
	xmlsettingsmanager.cpp
	accountsmanager.cpp
	accountslistwidget.cpp
	addaccountdialog.cpp
	upmanager.cpp
	filesproxymodel.cpp
	filestreemodel.cpp
	filesview.cpp
	remotedirectoryselectdialog.cpp
	syncer.cpp
	syncmanager.cpp
	syncwidget.cpp
	syncitemdelegate.cpp
	directorywidget.cpp
	fileswatcherbase.cpp
	utils.cpp
	downmanager.cpp
	)

set (FORMS
	managertab.ui
	accountslistwidget.ui
	addaccountdialog.ui
	directorywidget.ui
	managertab.ui
	remotedirectoryselectdialog.ui
	syncwidget.ui
	)

if (ENABLE_NETSTOREMANAGER_INOTIFY)
	add_definitions (-DENABLE_INOTIFY)
	set (SRCS ${SRCS} fileswatcher_inotify.cpp)
else ()
	set (SRCS ${SRCS} fileswatcher_dummy.cpp)
endif ()

set (RESOURCES netstoremanagerresources.qrc)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})
CreateTrs ("netstoremanager" "en;es;ro;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("netstoremanager" "en;ru_RU" "${SRCS}" "${FORMS}" "netstoremanagersettings.xml")

add_library (leechcraft_netstoremanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_netstoremanager
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_netstoremanager DESTINATION ${LC_PLUGINS_DEST})
install (FILES netstoremanagersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

FindQtLibs (leechcraft_netstoremanager Network Widgets)

option (ENABLE_NETSTOREMANAGER_GOOGLEDRIVE "Build support for Google Drive" ON)
option (ENABLE_NETSTOREMANAGER_DROPBOX "Build support for DropBox" ON)

if (ENABLE_NETSTOREMANAGER_GOOGLEDRIVE)
	add_subdirectory (plugins/googledrive)
endif ()

if (ENABLE_NETSTOREMANAGER_DROPBOX)
	add_subdirectory (plugins/dropbox)
endif ()
