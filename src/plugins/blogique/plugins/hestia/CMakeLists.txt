cmake_minimum_required (VERSION 3.3)
project (leechcraft_blogique_hestia)
include (InitLCPlugin OPTIONAL)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${BLOGIQUE_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/blogique
	)

set (HESTIA_SRCS
	accountconfigurationdialog.cpp
	accountconfigurationwidget.cpp
	accountstorage.cpp
	core.cpp
	hestia.cpp
	localblogaccount.cpp
	localbloggingplatform.cpp
	xmlsettingsmanager.cpp
	)

set (HESTIA_FORMS
	accountconfigurationdialog.ui
	accountconfigurationwidget.ui
	)

QtWrapUi (HESTIA_UIS_H ${HESTIA_FORMS})
QtAddResources (HESTIA_RCCS ${HESTIA_RESOURCES})

CreateTrs ("blogique_hestia" "en;ru_RU" HESTIA_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("blogique_hestia" "en;ru_RU" "${HESTIA_SRCS}" "${HESTIA_FORMS}" "blogiquehestiasettings.xml")

add_library (leechcraft_blogique_hestia SHARED
	${HESTIA_COMPILED_TRANSLATIONS}
	${HESTIA_SRCS}
	${HESTIA_UIS_H}
	${HESTIA_RCCS}
	)

target_link_libraries (leechcraft_blogique_hestia
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blogique_hestia DESTINATION ${LC_PLUGINS_DEST})
install (FILES blogiquehestiasettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_blogique_hestia Sql Widgets)
