cmake_minimum_required (VERSION 2.8)
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

set (HESTIA_HEADERS
	accountconfigurationdialog.h
	accountstorage.h
	core.h
	hestia.h
	accountconfigurationwidget.h
	localblogaccount.h
	localbloggingplatform.h
	xmlsettingsmanager.h
	)

set (HESTIA_FORMS
	accountconfigurationdialog.ui
	accountconfigurationwidget.ui
 	)

# set (HESTIA_RESOURCES blogiquehestiaresources.qrc)

# CreateTrVars ("blogique_hestia" "de;en;es;fr;it;ru_RU;uk_UA" HESTIA_TRANSLATIONS HESTIA_COMPILED_TRANSLATIONS)

QtWrapUi (HESTIA_UIS_H ${HESTIA_FORMS})
QtAddResources (HESTIA_RCCS ${HESTIA_RESOURCES})

add_library (leechcraft_blogique_hestia SHARED
	${HESTIA_COMPILED_TRANSLATIONS}
	${HESTIA_SRCS}
	${HESTIA_UIS_H}
	${HESTIA_RCCS}
	)

FindQtLibs (leechcraft_blogique_hestia Sql)

target_link_libraries (leechcraft_blogique_hestia
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blogique_hestia
		DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${HESTIA_COMPILED_TRANSLATIONS}
		DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES blogiquehestiasettings.xml
		DESTINATION ${LC_SETTINGS_DEST})
